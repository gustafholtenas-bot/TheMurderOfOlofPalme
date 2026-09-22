#include "People/TMOPPlayerAppearanceDirector.h"
#include "People/TMOPPlayerAppearanceSave.h"
#include "People/TMOPAppearanceResolver.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"
#include "EngineUtils.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

namespace { const TCHAR* AppearanceSaveSlot = TEXT("TMOP_PlayerAppearance_v1"); }

FTMOPAppearancePartChoice& ATMOPPlayerAppearanceDirector::Choice(FTMOPPersonProfileRow& P, ETMOPAppearancePartType Type)
{
    auto& A = P.AppearanceProfile;
    switch (Type)
    {
    case ETMOPAppearancePartType::Face: return A.Face;
    case ETMOPAppearancePartType::Hair: return A.Hair;
    case ETMOPAppearancePartType::Outerwear: return A.Outerwear;
    case ETMOPAppearancePartType::UpperBody: return A.UpperBody;
    case ETMOPAppearancePartType::Trousers: return A.Trousers;
    case ETMOPAppearancePartType::Footwear: return A.Footwear;
    case ETMOPAppearancePartType::Gloves: return A.Gloves;
    case ETMOPAppearancePartType::Headwear: return A.Headwear;
    case ETMOPAppearancePartType::FacialHair: return A.FacialHair;
    case ETMOPAppearancePartType::Scarf: return A.Scarf;
    case ETMOPAppearancePartType::Glasses: return A.Glasses;
    default: return A.Body;
    }
}

ATMOPPlayerAppearanceDirector* ATMOPPlayerAppearanceDirector::ForCharacter(ACharacter* Character)
{
    if (!IsValid(Character)) return nullptr;
    const int32 Slot = UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(Cast<ATMOPPlayerCharacter>(Character));
    if (Slot == INDEX_NONE) return nullptr;
    for (TActorIterator<ATMOPPlayerAppearanceDirector> It(Character->GetWorld()); It; ++It)
        if (It->ResolveTargetCharacter() == Character && It->PlayerIndex == Slot) return *It;
    auto* Director = Character->GetWorld()->SpawnActorDeferred<ATMOPPlayerAppearanceDirector>(
        StaticClass(), FTransform::Identity);
    if (!Director) return nullptr;
    Director->PlayerIndex = Slot;
    Director->TargetCharacterOverride = Character;
    Director->bApplyOnBeginPlay = false;
    Director->Tags.Add(TEXT("TMOP_RuntimePlayerAppearance"));
    Director->FinishSpawning(FTransform::Identity);
    return Director;
}

void ATMOPPlayerAppearanceDirector::LoadEditedProfile()
{
    bHasEditedProfile = false;
    if (auto* Save = Cast<UTMOPPlayerAppearanceSave>(UGameplayStatics::LoadGameFromSlot(AppearanceSaveSlot, 0)))
        if (const auto* Found = Save->Players.Find(PlayerIndex))
        {
            FString Error;
            if (ValidateEditableProfile(*Found, Error)) { EditedProfile = *Found; bHasEditedProfile = true; }
            else UE_LOG(LogTemp, Warning, TEXT("Saved player appearance skipped: %s"), *Error);
        }
}

bool ATMOPPlayerAppearanceDirector::SaveEditedProfile(FString& Error)
{
    if (!bHasEditedProfile) { Error = TEXT("Inget ändrat utseende att spara."); return false; }
    if (!ValidateEditableProfile(EditedProfile, Error)) return false;
    auto* Save = Cast<UTMOPPlayerAppearanceSave>(UGameplayStatics::LoadGameFromSlot(AppearanceSaveSlot, 0));
    if (!Save) Save = Cast<UTMOPPlayerAppearanceSave>(UGameplayStatics::CreateSaveGameObject(UTMOPPlayerAppearanceSave::StaticClass()));
    if (!Save) return false;
    Save->Players.Add(PlayerIndex, EditedProfile);
    if (!UGameplayStatics::SaveGameToSlot(Save, AppearanceSaveSlot, 0))
    { Error = TEXT("Kunde inte spara utseendet till disk."); return false; }
    return true;
}

void ATMOPPlayerAppearanceDirector::GetSelectableAssets(const FTMOPPersonProfileRow& Profile,
    ETMOPAppearancePartType Type, TArray<FName>& Out) const
{
    Out.Reset();
    UDataTable* Table = ResolveAssetCatalog();
    if (!Table || Table->GetRowStruct() != FTMOPAppearanceAssetRow::StaticStruct()) return;
    FTMOPResolvedAppearance Resolved;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Resolved);
    USkeletalMesh* Body = Resolved.Body.Mesh.LoadSynchronous();
    if (!Body) Body = (Profile.Gender == ETMOPPersonGender::Female ? FemaleBaseBodyMesh : MaleBaseBodyMesh).LoadSynchronous();
    if (!Body || !Body->GetSkeleton()) return;
    TArray<FTMOPAppearanceAssetRow*> Rows;
    Table->GetAllRows(TEXT("Appearance menu"), Rows);
    for (const auto* Row : Rows)
    {
        if (!Row || Row->CatalogId.IsNone() ||
            Table->FindRow<FTMOPAppearanceAssetRow>(Row->CatalogId, TEXT("Appearance menu ID"), false) != Row ||
            Row->PartType != Type ||
            (Row->Gender != ETMOPPersonGender::Unknown && Row->Gender != Profile.Gender) ||
            (!Row->CompatibleBodyBuilds.IsEmpty() && !Row->CompatibleBodyBuilds.Contains(Profile.GetResolvedBodyBuild())) ||
            Row->EarliestYear > 1986 || Row->LatestYear < 1986 ||
            (Profile.AgeAtEvent > 0 && (Profile.AgeAtEvent < Row->MinimumAge ||
                (Row->MaximumAge > 0 && Profile.AgeAtEvent > Row->MaximumAge)))) continue;
        const bool bSocket = Type == ETMOPAppearancePartType::Headwear ||
            Type == ETMOPAppearancePartType::Glasses || Type == ETMOPAppearancePartType::FacialHair;
        if (bSocket && Row->StaticMesh.LoadSynchronous()) { Out.AddUnique(Row->CatalogId); continue; }
        USkeletalMesh* Mesh = Row->Mesh.LoadSynchronous();
        if (Mesh && Mesh->GetSkeleton() == Body->GetSkeleton()) Out.AddUnique(Row->CatalogId);
    }
    Out.Sort(FNameLexicalLess());
}

bool ATMOPPlayerAppearanceDirector::ValidateEditableProfile(const FTMOPPersonProfileRow& Profile, FString& Error) const
{
    if (Profile.Gender != ETMOPPersonGender::Male && Profile.Gender != ETMOPPersonGender::Female)
    { Error = TEXT("Välj Manny/man eller Quinn/kvinna."); return false; }
    const float Height = Profile.AppearanceProfile.HeightOverrideCentimeters;
    if (!FMath::IsFinite(Height) || (Height != 0 && (Height < 120 || Height > 205)))
    { Error = TEXT("Längden måste vara 120–205 cm."); return false; }
    if (!ResolveAssetCatalog()) { Error = TEXT("DT_TMOP_AppearanceAssets saknas."); return false; }
    if (!(Profile.Gender == ETMOPPersonGender::Female ? FemaleBaseBodyMesh : MaleBaseBodyMesh).LoadSynchronous())
    { Error = TEXT("Grundkroppens mesh saknas."); return false; }
    FTMOPPersonProfileRow Copy = Profile;
    for (int32 I = 1; I <= static_cast<int32>(ETMOPAppearancePartType::Glasses); ++I)
    {
        const auto Type = static_cast<ETMOPAppearancePartType>(I);
        const auto& Part = Choice(Copy, Type);
        if (Part.bHidden || Part.CatalogId.IsNone()) continue;
        TArray<FName> Options;
        GetSelectableAssets(Profile, Type, Options);
        if (!Options.Contains(Part.CatalogId))
        { Error = FString::Printf(TEXT("%s saknas eller passar inte valt kön/kropp/skelett."), *Part.CatalogId.ToString()); return false; }
    }
    return true;
}

bool ATMOPPlayerAppearanceDirector::PreviewProfile(const FTMOPPersonProfileRow& Profile, FString& Error)
{
    Error.Reset();
    if (!ValidateEditableProfile(Profile, Error)) return false;
    const auto Before = EditedProfile;
    const bool bBefore = bHasEditedProfile;
    EditedProfile = Profile;
    bHasEditedProfile = true;
    if (ApplyPlayerAppearance())
    {
        // Report a content/shader error rather than pretending scalar writes
        // can make an opaque material cut out body regions.
        if (auto* Body = ResolveBodyMesh(ResolveTargetCharacter()))
            for (int32 I=0; I<Body->GetNumMaterials(); ++I)
            {
                auto* Material = Body->GetMaterial(I);
                float Value = 0;
                if (Material && (Material->GetBlendMode() != BLEND_Masked ||
                    !Material->GetScalarParameterValue(FMaterialParameterInfo(TEXT("TMOP_HideTorso")), Value)))
                {
                    Error = TEXT("Varning: kroppens material saknar maskstöd. Använd TMOP:s masked body-material med regionmask/UV2; annars kan huden synas genom kläderna.");
                    break;
                }
            }
        return true;
    }
    Error = FString::Join(ResolvedAppearance.Diagnostics, TEXT("\n"));
    if (Error.IsEmpty()) Error = TEXT("Utseendet kunde inte appliceras; tidigare val återställs.");
    EditedProfile = Before;
    bHasEditedProfile = bBefore;
    ApplyPlayerAppearance();
    return false;
}
