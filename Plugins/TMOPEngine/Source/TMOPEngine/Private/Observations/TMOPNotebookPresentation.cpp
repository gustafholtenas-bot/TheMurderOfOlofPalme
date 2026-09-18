#include "Observations/TMOPNotebookPresentation.h"
#include "Observations/TMOPObservationDirector.h"
#include "People/TMOPPersonRegistrySubsystem.h"
#include "People/TMOPPersonNameLibrary.h"
#include "People/TMOPPersonProfileTypes.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Components/MeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/GameInstance.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "ImageUtils.h"
#include "TextureResource.h"

namespace
{
    void AddText(TArray<FString>& Lines, const FString& Text)
    {
        const FString Trimmed = Text.TrimStartAndEnd();
        if (!Trimmed.IsEmpty()) Lines.AddUnique(Trimmed);
    }
}

void FTMOPNotebookPresentation::Populate(FTMOPNotebookObservation& Entry, UWorld* World,
    AActor* Actor, const FTMOPPersonProfileRow* SuppliedProfile, bool bCaptureModel)
{
    if (!World || Entry.EntityId.IsNone()) return;
    auto* Registry = World->GetGameInstance()
        ? World->GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    FTMOPPersonProfileRow StoredProfile;
    const FTMOPPersonProfileRow* Profile = SuppliedProfile;
    if (!Profile && Entry.Kind == ETMOPNotebookEntityKind::Person && Registry &&
        Registry->GetPersonProfile(Entry.EntityId, StoredProfile)) Profile = &StoredProfile;

    if (Entry.PresentationVersion == 0)
    {
        Entry.ObserverNames.Reset();
        Entry.EvidenceImages.Reset();
        TArray<FString> Description, Signalement;
        TSet<FName> ObserverIds;
        if (Profile)
        {
            // Use the authored appearance wording, never guessed visual asset IDs.
            const FTMOPAppearanceSlot* Slots[] = {&Profile->Hair, &Profile->Headwear,
                &Profile->BeardOrMustache, &Profile->FaceShape, &Profile->BodyBuild, &Profile->JacketOrCoat,
                &Profile->ShirtOrSweater, &Profile->Trousers, &Profile->Shoes, &Profile->Scarf};
            for (const auto* Slot : Slots) AddText(Signalement, Slot->OriginalText);
            for (const auto& Image : Profile->EvidenceImages)
            {
                if (Image.Image.IsNull()) continue;
                // Keep the right-hand gallery focused on the suspect sketches.
                if (Image.Type != ETMOPEvidenceImageType::PhantomImage &&
                    Image.Type != ETMOPEvidenceImageType::Sketch &&
                    Image.Type != ETMOPEvidenceImageType::Reconstruction &&
                    Image.Type != ETMOPEvidenceImageType::Other) continue;
                FTMOPNotebookEvidenceImage Saved;
                Saved.ImagePath = Image.Image.ToSoftObjectPath(); Saved.Caption = Image.Caption;
                if (Saved.Caption.IsEmpty()) Saved.Caption = FText::FromString(
                    Image.Type == ETMOPEvidenceImageType::PhantomImage ? TEXT("Fantombild") :
                    Image.Type == ETMOPEvidenceImageType::Sketch ? TEXT("Skiss") :
                    Image.Type == ETMOPEvidenceImageType::Reconstruction ? TEXT("Rekonstruktion") : TEXT("Bild"));
                Saved.Source = FText::FromString(Image.SourceReference);
                Entry.EvidenceImages.Add(Saved);
            }
        }
        if (const auto* Vehicle = Cast<ATMOPVehicleBase>(Actor))
        {
            Entry.VehicleSuspicion = Vehicle->NotebookSuspicion;
            AddText(Signalement, Vehicle->NotebookSignalement.ToString());
            if (!Vehicle->RegistrationNumber.IsEmpty()) AddText(Signalement,
                FString::Printf(TEXT("Registreringsnummer: %s"), *Vehicle->RegistrationNumber));
        }

        struct FTimedDefinition { int32 Second; FTMOPObservationDefinition Definition; };
        TArray<FTimedDefinition> Definitions;
        for (TActorIterator<ATMOPObservationDirector> It(World); It; ++It)
        {
            for (const auto& Definition : It->GetObservationDefinitionsForTarget(Entry.EntityId))
            {
                FTMOPObservationRuntime Runtime;
                int32 Second = INDEX_NONE;
                if (It->TryGetObservationRuntime(Definition.ObservationId, Runtime) && Runtime.bHasResolvedCanonicalTime)
                    Second = Runtime.ResolvedCanonicalStartTime.ToSecondsFromMidnight();
                else if (Definition.TimingMode == ETMOPObservationTimingMode::Absolute)
                    Second = Definition.CanonicalTime.ToSecondsFromMidnight();
                // The card must not reveal a person's future route when opened early.
                if (Second == INDEX_NONE || Second > Entry.DiscoveredSecond) continue;
                Definitions.Add({Second, Definition});
            }
        }
        Definitions.Sort([](const FTimedDefinition& A, const FTimedDefinition& B)
        { return A.Second == B.Second ? A.Definition.ObservationId.LexicalLess(B.Definition.ObservationId) : A.Second < B.Second; });
        for (const auto& Timed : Definitions)
        {
            const auto& D = Timed.Definition;
            if (!D.ObservedDescription.IsEmpty()) AddText(Description,
                FTMOPTime::FromSecondsFromMidnight(Timed.Second).ToDisplayString() + TEXT(" — ") + D.ObservedDescription);
            for (const FName Id : D.ObserverEntityIds) if (!Id.IsNone()) ObserverIds.Add(Id);
            for (const auto& Witness : D.WitnessSignalements)
            {
                if (!Witness.ObserverEntityId.IsNone()) ObserverIds.Add(Witness.ObserverEntityId);
                TArray<FString> WitnessDetails;
                AddText(WitnessDetails, Witness.OriginalSummary);
                if (WitnessDetails.IsEmpty())
                    for (const auto& Trait : Witness.Traits) AddText(WitnessDetails, Trait.OriginalText);
                if (!WitnessDetails.IsEmpty())
                {
                    FTMOPPersonProfileRow Observer;
                    const FString Name = Registry && Registry->GetPersonProfile(Witness.ObserverEntityId, Observer)
                        ? UTMOPPersonNameLibrary::FormatPersonName(Observer.FullName, Observer.FirstName, Observer.LastName).ToString()
                        : TEXT("Okänt vittne");
                    // Conflicting accounts retain their witness attribution.
                    AddText(Signalement, Name + TEXT(": ") + FString::Join(WitnessDetails, TEXT("; ")));
                }
            }
        }
        // Preserve the earlier collected summary when no timed source text exists.
        if (!Description.IsEmpty()) Entry.Summary = FText::FromString(FString::Join(Description, TEXT("\n\n")));
        Entry.Signalement = FText::FromString(FString::Join(Signalement, TEXT("; ")));
        TArray<FName> SortedObservers = ObserverIds.Array(); SortedObservers.Sort(FNameLexicalLess());
        for (const FName Id : SortedObservers)
        {
            FTMOPPersonProfileRow Observer;
            const FText Name = Registry && Registry->GetPersonProfile(Id, Observer)
                ? UTMOPPersonNameLibrary::FormatPersonName(Observer.FullName, Observer.FirstName, Observer.LastName)
                : NSLOCTEXT("TMOP", "NotebookUnknownWitness", "Okänt vittne");
            Entry.ObserverNames.Add(Name);
        }
        // A legacy vehicle needs its live actor to recover authored classification.
        Entry.PresentationVersion = Entry.Kind == ETMOPNotebookEntityKind::Vehicle && !IsValid(Actor) ? 0 : 1;
    }
    if (bCaptureModel && Entry.ModelPreviewPng.IsEmpty() && IsValid(Actor)) CaptureModel(Actor, Entry.Kind, Entry.ModelPreviewPng);
}

bool FTMOPNotebookPresentation::CaptureModel(AActor* Actor, ETMOPNotebookEntityKind Kind, TArray<uint8>& OutPng)
{
    if (!IsValid(Actor) || !Actor->GetWorld() || Actor->GetWorld()->GetNetMode() == NM_DedicatedServer) return false;
    TArray<UMeshComponent*> Meshes; Actor->GetComponents<UMeshComponent>(Meshes, true);
    FBox Bounds(ForceInit);
    TArray<UMeshComponent*> Visible;
    for (auto* Mesh : Meshes)
    {
        if (!IsValid(Mesh) || !Mesh->IsRegistered() || !Mesh->IsVisible() || Mesh->bHiddenInGame) continue;
        Bounds += Mesh->Bounds.GetBox(); Visible.Add(Mesh);
    }
    if (!Bounds.IsValid || Visible.IsEmpty()) return false;
    const int32 Width = Kind == ETMOPNotebookEntityKind::Vehicle ? 320 : 192;
    const int32 Height = Kind == ETMOPNotebookEntityKind::Vehicle ? 200 : 256;
    FActorSpawnParameters Params; Params.ObjectFlags |= RF_Transient;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* CaptureActor = Actor->GetWorld()->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, Params);
    if (!CaptureActor) return false;
    auto* Capture = NewObject<USceneCaptureComponent2D>(CaptureActor);
    auto* Target = NewObject<UTextureRenderTarget2D>(CaptureActor);
    CaptureActor->SetRootComponent(Capture); CaptureActor->AddInstanceComponent(Capture);
    Capture->bCaptureEveryFrame = false; Capture->bCaptureOnMovement = false;
    Target->ClearColor = FLinearColor(0.36f, 0.36f, 0.36f, 1.0f);
    Target->InitCustomFormat(Width, Height, PF_B8G8R8A8, false); Target->UpdateResourceImmediate(true);
    Capture->TextureTarget = Target;
    Capture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;
    Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
    // Stable unlit material view: no street/background actors, UI labels or night exposure.
    Capture->ShowFlags.SetLighting(false); Capture->ShowFlags.SetFog(false);
    Capture->ShowFlags.SetAtmosphere(false); Capture->ShowFlags.SetPostProcessing(false);
    for (auto* Mesh : Visible) Capture->ShowOnlyComponent(Mesh);
    Capture->FOVAngle = 35.0f;
    const double HalfHorizontal = FMath::DegreesToRadians(17.5);
    const double HalfVertical = FMath::Atan(FMath::Tan(HalfHorizontal) * Height / Width);
    const double Distance = Bounds.GetExtent().Size() * 1.12 / FMath::Sin(FMath::Min(HalfHorizontal, HalfVertical));
    const FVector Center = Bounds.GetCenter();
    const FVector Direction = Actor->GetActorQuat().RotateVector(Kind == ETMOPNotebookEntityKind::Vehicle
        ? FVector(1, 0.8, 0.35).GetSafeNormal() : FVector(1, 0.12, 0.03).GetSafeNormal());
    const FVector Location = Center + Direction * Distance;
    Capture->RegisterComponent(); Capture->SetWorldLocationAndRotation(Location, (Center - Location).Rotation());
    Capture->CaptureScene();
    TArray<uint8> Bytes;
    TArray<FColor> Pixels;
    if (auto* Resource = Target->GameThread_GetRenderTargetResource())
        if (Resource->ReadPixels(Pixels) && Pixels.Num() == Width * Height)
        {
            const FColor First = Pixels[0];
            const bool bHasModel = Pixels.ContainsByPredicate([First](const FColor& P)
                { return FMath::Abs(int32(P.R) - First.R) + FMath::Abs(int32(P.G) - First.G) +
                    FMath::Abs(int32(P.B) - First.B) > 8; });
            // FinalColor captures may carry an unused alpha channel. The card is opaque.
            for (auto& Pixel : Pixels) Pixel.A = 255;
            if (bHasModel) FImageUtils::CompressImageArray(Width, Height, Pixels, Bytes);
        }
    const bool bSuccess = !Bytes.IsEmpty();
    Capture->DestroyComponent(); CaptureActor->Destroy();
    if (bSuccess) OutPng = MoveTemp(Bytes);
    return bSuccess;
}
