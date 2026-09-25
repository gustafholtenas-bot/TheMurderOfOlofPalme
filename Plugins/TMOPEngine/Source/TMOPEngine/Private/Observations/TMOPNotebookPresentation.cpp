#include "Observations/TMOPNotebookPresentation.h"
#include "Localization/TMOPLocalization.h"
#include "Agents/TMOPHistoricalAgent.h"
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
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Addresses/TMOPAddressComponent.h"
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
    AActor* Actor, const FTMOPPersonProfileRow* SuppliedProfile, bool bCaptureModel, bool bLocalizeView)
{
    if (!World || Entry.EntityId.IsNone()) return;
    auto* Registry = World->GetGameInstance()
        ? World->GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    FTMOPPersonProfileRow StoredProfile;
    const FTMOPPersonProfileRow* Profile = SuppliedProfile;
    if (!Profile && Entry.Kind == ETMOPNotebookEntityKind::Person && Registry &&
        Registry->GetPersonProfile(Entry.EntityId, StoredProfile)) Profile = &StoredProfile;

    FTMOPPersonProfileRow ProfileView;
    if (bLocalizeView && Profile)
    {
        ProfileView = FTMOPLocalization::RowView(TEXT("DT_TMOP_People"), Entry.EntityId.ToString(), *Profile);
        Profile = &ProfileView;
    }

    const auto Display = [bLocalizeView](const FString& Text)
    { return bLocalizeView ? FTMOPLocalization::String(Text) : Text; };
    if (Entry.PresentationVersion == 0 || bLocalizeView)
    {
        Entry.ObserverNames.Reset();
        if (!bLocalizeView || Profile) Entry.EvidenceImages.Reset();
        TArray<FString> Description, Signalement;
        TSet<FName> ObserverIds;
        if (Profile)
        {
            // Use the authored appearance wording, never guessed visual asset IDs.
            const FTMOPAppearanceSlot* Slots[] = {&Profile->Hair, &Profile->Headwear,
                &Profile->BeardOrMustache, &Profile->FaceShape, &Profile->BodyBuild, &Profile->JacketOrCoat,
                &Profile->ShirtOrSweater, &Profile->Trousers, &Profile->Shoes, &Profile->Scarf};
            for (const auto* Slot : Slots) AddText(Signalement, Display(Slot->OriginalText));
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
            AddText(Signalement, Display(Vehicle->NotebookSignalement.ToString()));
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
                if (Second == INDEX_NONE || Second > FMath::Max(Entry.DiscoveredSecond, Entry.LastObservedSecond)) continue;
                Definitions.Add({Second, Definition});
            }
        }
        Definitions.Sort([](const FTimedDefinition& A, const FTimedDefinition& B)
        { return A.Second == B.Second ? A.Definition.ObservationId.LexicalLess(B.Definition.ObservationId) : A.Second < B.Second; });
        for (const auto& Timed : Definitions)
        {
            const auto D = bLocalizeView
                ? FTMOPLocalization::RowView(TEXT("DT_TMOP_Observations"), Timed.Definition.ObservationId.ToString(), Timed.Definition)
                : Timed.Definition;
            if (!D.ObservedDescription.IsEmpty()) AddText(Description,
                FTMOPTime::FromSecondsFromMidnight(Timed.Second).ToDisplayString() + TEXT(" — ") + Display(D.ObservedDescription));
            for (const FName Id : D.ObserverEntityIds) if (!Id.IsNone()) ObserverIds.Add(Id);
            for (const auto& Witness : D.WitnessSignalements)
            {
                if (!Witness.ObserverEntityId.IsNone()) ObserverIds.Add(Witness.ObserverEntityId);
                TArray<FString> WitnessDetails;
                AddText(WitnessDetails, Display(Witness.OriginalSummary));
                if (WitnessDetails.IsEmpty())
                    for (const auto& Trait : Witness.Traits) AddText(WitnessDetails, Display(Trait.OriginalText));
                if (!WitnessDetails.IsEmpty())
                {
                    FTMOPPersonProfileRow Observer;
                    const FString Name = Registry && Registry->GetPersonProfile(Witness.ObserverEntityId, Observer)
                        ? UTMOPPersonNameLibrary::FormatPersonName(Observer.FullName, Observer.FirstName, Observer.LastName).ToString()
                        : NSLOCTEXT("TMOP", "NotebookUnknownWitness", "Okänt vittne").ToString();
                    // Conflicting accounts retain their witness attribution.
                    AddText(Signalement, Name + TEXT(": ") + FString::Join(WitnessDetails, TEXT("; ")));
                }
            }
        }
        // Preserve the earlier collected summary when no timed source text exists.
        if (!Description.IsEmpty()) Entry.Summary = FText::FromString(FString::Join(Description, TEXT("\n\n")));
        if (!Signalement.IsEmpty()) Entry.Signalement = FText::FromString(FString::Join(Signalement, TEXT("; ")));
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
    CollectLocations(Entry, World);
    if (bCaptureModel && (Entry.ModelPreviewPng.IsEmpty() || Entry.ModelPreviewVersion < 2) && IsValid(Actor))
        if (CaptureModel(Actor, Entry.Kind, Entry.ModelPreviewPng)) Entry.ModelPreviewVersion = 2;
}

FTMOPNotebookObservation FTMOPNotebookPresentation::LocalizedView(
    const FTMOPNotebookObservation& Source, UWorld* World)
{
    FTMOPNotebookObservation View = Source;
    if (!World) return View;
    AActor* Actor = nullptr;
    if (Source.Kind == ETMOPNotebookEntityKind::Person && World->GetGameInstance())
    {
        if (auto* Registry = World->GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>())
            Actor = Registry->FindActiveAgent(Source.EntityId);
    }
    else if (Source.Kind == ETMOPNotebookEntityKind::Vehicle)
        for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
            if (It->VehicleId == Source.EntityId) { Actor = *It; break; }
    // Existing discovery/last-observed cutoffs still apply inside Populate.
    Populate(View, World, Actor, nullptr, false, true);
    return View;
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
    const FVector Center = Bounds.GetCenter();
    const FVector Direction = Actor->GetActorQuat().RotateVector(Kind == ETMOPNotebookEntityKind::Vehicle
        ? FVector(1, 0.8, 0.35).GetSafeNormal() : FVector(1, 0.12, 0.03).GetSafeNormal());
    const FRotationMatrix Basis((-Direction).Rotation());
    const FVector Right = Basis.GetUnitAxis(EAxis::Y), Up = Basis.GetUnitAxis(EAxis::Z);
    double Distance = 1.0;
    // Fit projected bounds rather than a sphere; a tall person now fills the portrait.
    for (int32 X = 0; X < 2; ++X) for (int32 Y = 0; Y < 2; ++Y) for (int32 Z = 0; Z < 2; ++Z)
    {
        const FVector Corner(X ? Bounds.Max.X : Bounds.Min.X,
            Y ? Bounds.Max.Y : Bounds.Min.Y, Z ? Bounds.Max.Z : Bounds.Min.Z);
        const FVector Offset = Corner - Center;
        Distance = FMath::Max(Distance, FVector::DotProduct(Offset, Direction) + 1.06 * FMath::Max(
            FMath::Abs(FVector::DotProduct(Offset, Right)) / FMath::Tan(HalfHorizontal),
            FMath::Abs(FVector::DotProduct(Offset, Up)) / FMath::Tan(HalfVertical)));
    }
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

void FTMOPNotebookPresentation::CollectLocations(FTMOPNotebookObservation& Entry, UWorld* World)
{
    if (!World) return;
    TMap<FName, ATMOPHistoricalAnchor*> Anchors;
    TArray<ATMOPHistoricalAnchor*> Addresses;
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        Anchors.Add(It->GetAnchorId(), *It);
        const auto* Address = It->FindComponentByClass<UTMOPAddressComponent>();
        if (Address && Address->HasValidAddress()) Addresses.Add(*It);
    }
    for (TActorIterator<ATMOPObservationDirector> It(World); It; ++It)
        for (const auto& D : It->GetObservationDefinitionsForTarget(Entry.EntityId))
        {
            FTMOPObservationRuntime Runtime;
            int32 Second = INDEX_NONE;
            if (It->TryGetObservationRuntime(D.ObservationId, Runtime) && Runtime.bHasResolvedCanonicalTime)
                Second = Runtime.ResolvedCanonicalStartTime.ToSecondsFromMidnight();
            else if (D.TimingMode == ETMOPObservationTimingMode::Absolute)
                Second = D.CanonicalTime.ToSecondsFromMidnight();
            if (Second == INDEX_NONE || Second > FMath::Max(Entry.DiscoveredSecond, Entry.LastObservedSecond)) continue;
            auto* const* Anchor = Anchors.Find(D.ObservationAnchorId);
            if (!Anchor) continue; // Never substitute the actor's current position for a historical sighting.
            FTMOPNotebookLocation Point;
            Point.ObservationId = D.ObservationId; Point.Second = Second;
            Point.WorldLocation = (*Anchor)->GetActorLocation();
            double Nearest = TNumericLimits<double>::Max();
            for (auto* AddressAnchor : Addresses)
            {
                const double Distance = FVector::DistSquared2D(Point.WorldLocation, AddressAnchor->GetActorLocation());
                if (Distance < Nearest)
                {
                    Nearest = Distance;
                    Point.Address = AddressAnchor->FindComponentByClass<UTMOPAddressComponent>()->GetAddressTitle();
                }
            }
            if (Point.Address.IsEmpty()) Point.Address = (*Anchor)->DisplayName;
            if (Point.Address.IsEmpty()) Point.Address = FText::FromName(D.ObservationAnchorId);
            if (auto* Existing = Entry.Locations.FindByPredicate([&](const auto& P) { return P.ObservationId == Point.ObservationId; }))
                *Existing = Point;
            else Entry.Locations.Add(Point);
        }
    Entry.Locations.Sort([](const auto& A, const auto& B) { return A.Second < B.Second; });
}

void FTMOPNotebookPresentation::RecordPlayerSighting(FTMOPNotebookObservation& Entry, UWorld* World,
    FVector Position, int32 Second)
{
    if (!World) return;
    FTMOPNotebookLocation Point;
    Point.ObservationId = FName(*FString::Printf(TEXT("PLAYER_SEEN_%d"), Second));
    Point.bPlayerObservation = true; Point.Second = Second; Point.WorldLocation = Position;
    double Nearest = TNumericLimits<double>::Max();
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        const auto* Address = It->FindComponentByClass<UTMOPAddressComponent>();
        if (!Address || !Address->HasValidAddress()) continue;
        const double Distance = FVector::DistSquared2D(Position, It->GetActorLocation());
        if (Distance < Nearest) { Nearest = Distance; Point.Address = Address->GetAddressTitle(); }
    }
    if (Point.Address.IsEmpty()) Point.Address = FText::FromString(TEXT("Adress saknas"));
    if (!Entry.Locations.ContainsByPredicate([&](const auto& P) { return P.ObservationId == Point.ObservationId; }))
        Entry.Locations.Add(Point);
    Entry.Locations.Sort([](const auto& A, const auto& B) { return A.Second < B.Second; });
}
