#include "People/TMOPPlayerAppearanceDirector.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "People/TMOPAppearanceResolver.h"
#include "People/TMOPCharacterAppearanceComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"
#include "TimerManager.h"

ATMOPPlayerAppearanceDirector::ATMOPPlayerAppearanceDirector()
{
    PrimaryActorTick.bCanEverTick = false;
    MaleBaseBodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT(
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
    FemaleBaseBodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT(
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")));
}

void ATMOPPlayerAppearanceDirector::BeginPlay()
{
    Super::BeginPlay();
    LoadEditedProfile();
    if (!bApplyOnBeginPlay) return;

    RemainingStartupRetries = MaximumStartupRetries;
    if (InitialApplyDelaySeconds <= 0.0f)
    {
        TryStartupApply();
        return;
    }
    GetWorldTimerManager().SetTimer(StartupRetryTimer, this,
        &ATMOPPlayerAppearanceDirector::TryStartupApply,
        InitialApplyDelaySeconds, false);
}

ACharacter* ATMOPPlayerAppearanceDirector::ResolveTargetCharacter() const
{
    if (IsValid(TargetCharacterOverride)) return TargetCharacterOverride;
    return UGameplayStatics::GetPlayerCharacter(this, PlayerIndex);
}

USkeletalMeshComponent* ATMOPPlayerAppearanceDirector::ResolveBodyMesh(
    ACharacter* Character) const
{
    if (IsValid(BodyMeshOverride)) return BodyMeshOverride;
    return IsValid(Character) ? Character->GetMesh() : nullptr;
}

UDataTable* ATMOPPlayerAppearanceDirector::ResolveAssetCatalog() const
{
    if (IsValid(AppearanceAssetTableOverride)) return AppearanceAssetTableOverride;
    const UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    const UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    return Registry != nullptr ? Registry->GetAppearanceAssetTable() : nullptr;
}

bool ATMOPPlayerAppearanceDirector::BuildPlayerProfile(
    FTMOPPersonProfileRow& OutProfile) const
{
    if (bHasEditedProfile) { OutProfile = EditedProfile; return true; }
    if (bUsePersonProfileRow)
    {
        const UDataTable* Table = PlayerProfileRow.DataTable;
        if (!IsValid(Table) || PlayerProfileRow.RowName.IsNone() ||
            Table->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct())
        {
            UE_LOG(LogTemp, Error, TEXT(
                "TMOP PlayerAppearanceDirector: Player Profile Row is missing or uses the wrong row struct."));
            return false;
        }
        const FTMOPPersonProfileRow* Row = Table->FindRow<FTMOPPersonProfileRow>(
            PlayerProfileRow.RowName, TEXT("PlayerAppearanceDirector"), false);
        if (Row == nullptr) return false;
        OutProfile = *Row;
        return true;
    }

    OutProfile = FTMOPPersonProfileRow();
    OutProfile.EntityId = TEXT("PLAYER_CHARACTER");
    OutProfile.Gender = PlayerGender;
    OutProfile.AppearanceProfile = InlineAppearanceProfile;
    return true;
}

USkeletalMeshComponent* ATMOPPlayerAppearanceDirector::EnsurePartComponent(
    ACharacter* Character, USkeletalMeshComponent* Body,
    const FName ComponentName)
{
    if (!IsValid(Character) || !IsValid(Body)) return nullptr;

    TArray<USkeletalMeshComponent*> ExistingComponents;
    Character->GetComponents<USkeletalMeshComponent>(ExistingComponents);
    for (USkeletalMeshComponent* Existing : ExistingComponents)
    {
        if (IsValid(Existing) && Existing->GetFName() == ComponentName)
        {
            ManagedPartComponents.AddUnique(Existing);
            return Existing;
        }
    }

    USkeletalMeshComponent* Component = NewObject<USkeletalMeshComponent>(
        Character, ComponentName);
    if (!IsValid(Component)) return nullptr;

    Character->AddInstanceComponent(Component);
    Component->AttachToComponent(Body,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Component->SetRelativeTransform(FTransform::Identity);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetOwnerNoSee(Body->bOwnerNoSee);
    Component->SetOnlyOwnerSee(Body->bOnlyOwnerSee);
    Component->SetCastShadow(Body->CastShadow);
    Component->RegisterComponent();
    ManagedPartComponents.AddUnique(Component);
    return Component;
}

bool ATMOPPlayerAppearanceDirector::ApplyResolvedPart(
    ACharacter* Character, USkeletalMeshComponent* Body,
    const FName ComponentName, const FTMOPResolvedAppearancePart& Part)
{
    USkeletalMeshComponent* Component = EnsurePartComponent(
        Character, Body, ComponentName);
    if (!IsValid(Component)) return false;

    if (Part.PartType == ETMOPAppearancePartType::Hair)
    {
        Component->SetLeaderPoseComponent(nullptr);
        Component->SetSkeletalMesh(nullptr);
        Component->SetAbsolute(false, false, false);
        Component->AttachToComponent(Body,
            FAttachmentTransformRules::SnapToTargetIncludingScale, NAME_None);
        Component->SetRelativeTransform(FTransform::Identity);
    }
    Component->EmptyOverrideMaterials();
    if (Part.bIntentionallyEmpty)
    {
        Component->SetVisibility(false, true);
        return true;
    }

    USkeletalMesh* Mesh = Part.Mesh.LoadSynchronous();
    if (Mesh == nullptr)
    {
        Component->SetVisibility(false, true);
        return Part.Mesh.IsNull();
    }

    const USkeletalMesh* BodyAsset = Body->GetSkeletalMeshAsset();
    if (BodyAsset != nullptr && BodyAsset->GetSkeleton() != nullptr &&
        Mesh->GetSkeleton() != nullptr &&
        BodyAsset->GetSkeleton() != Mesh->GetSkeleton())
    {
        const FString Message = FString::Printf(TEXT(
            "Player asset '%s' has a skeleton incompatible with body '%s'."),
            *Part.CatalogId.ToString(), *BodyAsset->GetName());
        ResolvedAppearance.Diagnostics.Add(Message);
        UE_LOG(LogTemp, Error, TEXT("TMOP PlayerAppearanceDirector: %s"), *Message);
        Component->SetVisibility(false, true);
        return false;
    }

    Component->SetSkeletalMesh(Mesh);
    Component->SetLeaderPoseComponent(Body, true);
    Component->SetVisibility(true, true);
    Component->SetHiddenInGame(false);

    if (UMaterialInterface* Material = Part.Material.LoadSynchronous())
    {
        for (int32 Index = 0; Index < Component->GetNumMaterials(); ++Index)
        {
            UMaterialInstanceDynamic* Dynamic =
                Component->CreateDynamicMaterialInstance(Index, Material);
            if (Dynamic == nullptr) continue;
            if (Part.PartType != ETMOPAppearancePartType::Hair &&
                Part.PartType != ETMOPAppearancePartType::FacialHair)
            {
                Dynamic->SetVectorParameterValue(TEXT("PrimaryColor"), Part.PrimaryColor);
                Dynamic->SetVectorParameterValue(TEXT("SecondaryColor"), Part.SecondaryColor);
            }
            Dynamic->SetScalarParameterValue(TEXT("TMOP_IsUnknown"),
                Part.bUsesObscuredFallback ? 1.0f : 0.0f);
            Dynamic->SetScalarParameterValue(TEXT("TMOP_ObscurityAmount"),
                Part.ObscurityAmount);
        }
    }
    return true;
}

bool ATMOPPlayerAppearanceDirector::ApplyResolvedFaceAccessory(
    ACharacter* Character, USkeletalMeshComponent* Body,
    const FName LegacyName, const FName StaticName, const FName DefaultSocket,
    const FTransform& FaceOffset, const FTMOPResolvedAppearancePart& Part)
{
    TArray<USkeletalMeshComponent*> OldParts;
    Character->GetComponents<USkeletalMeshComponent>(OldParts);
    for (USkeletalMeshComponent* Old : OldParts)
        if (IsValid(Old) && Old->GetFName() == LegacyName)
        {
            Old->SetSkeletalMesh(nullptr);
            Old->SetVisibility(false, true);
        }
    TArray<UStaticMeshComponent*> Existing;
    Character->GetComponents<UStaticMeshComponent>(Existing);
    UStaticMeshComponent* Component = nullptr;
    for (UStaticMeshComponent* Candidate : Existing)
        if (IsValid(Candidate) && Candidate->GetFName() == StaticName)
            Component = Candidate;
    if (Component)
    {
        Component->SetStaticMesh(nullptr);
        Component->EmptyOverrideMaterials();
        Component->SetVisibility(false, true);
        ManagedFaceAccessories.AddUnique(Component);
    }
    if (Part.bIntentionallyEmpty) return true;
    if (Part.StaticMesh.IsNull())
        return Part.Mesh.IsNull() || ApplyResolvedPart(Character, Body, LegacyName, Part);
    UStaticMesh* Mesh = Part.StaticMesh.LoadSynchronous();
    if (!Mesh)
    {
        ResolvedAppearance.Diagnostics.Add(FString::Printf(
            TEXT("Player accessory '%s' could not load StaticMesh."), *Part.CatalogId.ToString()));
        return false;
    }
    if (!Component)
    {
        Component = NewObject<UStaticMeshComponent>(Character, StaticName);
        Character->AddInstanceComponent(Component);
        Component->SetupAttachment(Body);
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->RegisterComponent();
        ManagedFaceAccessories.AddUnique(Component);
    }
    Component->SetOwnerNoSee(Body->bOwnerNoSee);
    Component->SetOnlyOwnerSee(Body->bOnlyOwnerSee);
    Component->SetCastShadow(Body->CastShadow);
    FName Socket = Part.AttachmentSocket.IsNone() || Part.AttachmentSocket == DefaultHeadwearSocket
        ? DefaultSocket : Part.AttachmentSocket;
    if (!Body->DoesSocketExist(Socket)) Socket = HeadwearFallbackBone;
    if (!Body->DoesSocketExist(Socket))
    {
        ResolvedAppearance.Diagnostics.Add(TEXT("Player face accessory socket and fallback bone are missing."));
        return false;
    }
    Component->SetStaticMesh(Mesh);
    Component->AttachToComponent(Body, FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
    Component->SetRelativeTransform(Part.AttachmentTransform * FaceOffset);
    Component->SetVisibility(true, true);
    Component->SetHiddenInGame(false);
    if (UMaterialInterface* Material = Part.Material.LoadSynchronous())
        for (int32 Index = 0; Index < Component->GetNumMaterials(); ++Index)
            if (UMaterialInstanceDynamic* Dynamic = Component->CreateDynamicMaterialInstance(Index, Material))
            {
                if (Part.PartType != ETMOPAppearancePartType::FacialHair)
                {
                    Dynamic->SetVectorParameterValue(TEXT("PrimaryColor"), Part.PrimaryColor);
                    Dynamic->SetVectorParameterValue(TEXT("SecondaryColor"), Part.SecondaryColor);
                }
                Dynamic->SetScalarParameterValue(TEXT("TMOP_IsUnknown"), Part.bUsesObscuredFallback ? 1.0f : 0.0f);
                Dynamic->SetScalarParameterValue(TEXT("TMOP_ObscurityAmount"), Part.ObscurityAmount);
            }
    return true;
}

UStaticMeshComponent* ATMOPPlayerAppearanceDirector::EnsureHeadwearComponent(
    ACharacter* Character, USkeletalMeshComponent* Body)
{
    if (!IsValid(Character) || !IsValid(Body)) return nullptr;
    if (IsValid(ManagedHeadwearComponent) && ManagedHeadwearComponent->GetOwner() == Character)
        return ManagedHeadwearComponent;
    ManagedHeadwearComponent = nullptr;

    TArray<UStaticMeshComponent*> ExistingComponents;
    Character->GetComponents<UStaticMeshComponent>(ExistingComponents);
    for (UStaticMeshComponent* Existing : ExistingComponents)
        if (IsValid(Existing) && Existing->GetFName() ==
            FName(TEXT("TMOP_Player_Headwear")))
        {
            ManagedHeadwearComponent = Existing;
            return Existing;
        }

    UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(
        Character, FName(TEXT("TMOP_Player_Headwear")));
    if (!IsValid(Component)) return nullptr;
    Character->AddInstanceComponent(Component);
    Component->SetupAttachment(Body);
    Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Component->SetGenerateOverlapEvents(false);
    Component->SetOwnerNoSee(Body->bOwnerNoSee);
    Component->SetOnlyOwnerSee(Body->bOnlyOwnerSee);
    Component->SetCastShadow(Body->CastShadow);
    Component->RegisterComponent();
    ManagedHeadwearComponent = Component;
    return Component;
}

bool ATMOPPlayerAppearanceDirector::ApplyResolvedHeadwear(
    ACharacter* Character, USkeletalMeshComponent* Body,
    const FTMOPResolvedAppearancePart& Part)
{
    UStaticMeshComponent* Component = EnsureHeadwearComponent(Character, Body);
    if (!IsValid(Component)) return false;
    Component->EmptyOverrideMaterials();
    for (USkeletalMeshComponent* Legacy : ManagedPartComponents)
        if (IsValid(Legacy) && Legacy->GetFName() == FName(TEXT("TMOP_Player_Headwear_Legacy")))
        {
            Legacy->SetSkeletalMesh(nullptr);
            Legacy->SetVisibility(false, true);
        }
    if (Part.bIntentionallyEmpty)
    {
        Component->SetStaticMesh(nullptr);
        Component->SetVisibility(false, true);
        return true;
    }

    UStaticMesh* Mesh = Part.StaticMesh.LoadSynchronous();
    if (Mesh == nullptr)
    {
        Component->SetStaticMesh(nullptr);
        Component->SetVisibility(false, true);
        if (!Part.StaticMesh.IsNull())
        {
            ResolvedAppearance.Diagnostics.Add(FString::Printf(TEXT(
                "Player headwear '%s' could not load Static Mesh '%s'."),
                *Part.CatalogId.ToString(),
                *Part.StaticMesh.ToSoftObjectPath().ToString()));
            return false;
        }
        if (!Part.Mesh.IsNull())
        {
            ResolvedAppearance.Diagnostics.Add(FString::Printf(TEXT(
                "Player headwear '%s' still uses legacy Skeletal Mesh; migrate it to StaticMesh."),
                *Part.CatalogId.ToString()));
            return ApplyResolvedPart(Character, Body,
                TEXT("TMOP_Player_Headwear_Legacy"), Part);
        }
        return false;
    }

    Component->SetStaticMesh(Mesh);
    for (USkeletalMeshComponent* Legacy : ManagedPartComponents)
        if (IsValid(Legacy) && Legacy->GetFName() ==
            FName(TEXT("TMOP_Player_Headwear_Legacy")))
        {
            Legacy->SetSkeletalMesh(nullptr);
            Legacy->SetVisibility(false, true);
        }
    FName Socket = Part.AttachmentSocket.IsNone()
        ? DefaultHeadwearSocket : Part.AttachmentSocket;
    if (!Body->DoesSocketExist(Socket))
    {
        const FName RequestedSocket = Socket;
        Socket = Body->DoesSocketExist(HeadwearFallbackBone)
            ? HeadwearFallbackBone : NAME_None;
        ResolvedAppearance.Diagnostics.Add(FString::Printf(TEXT(
            "Player headwear socket '%s' is missing; using '%s'."),
            *RequestedSocket.ToString(),
            Socket.IsNone() ? TEXT("component root") : *Socket.ToString()));
    }
    Component->AttachToComponent(Body,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
    Component->SetRelativeTransform(Part.AttachmentTransform * ResolvedAppearance.Face.HeadAccessoryFit.HeadwearOffset);
    Component->SetVisibility(true, true);

    if (UMaterialInterface* Material = Part.Material.LoadSynchronous())
        for (int32 Index = 0; Index < Component->GetNumMaterials(); ++Index)
        {
            UMaterialInstanceDynamic* Dynamic =
                Component->CreateDynamicMaterialInstance(Index, Material);
            if (Dynamic == nullptr) continue;
            Dynamic->SetVectorParameterValue(TEXT("PrimaryColor"), Part.PrimaryColor);
            Dynamic->SetVectorParameterValue(TEXT("SecondaryColor"), Part.SecondaryColor);
            Dynamic->SetScalarParameterValue(TEXT("TMOP_IsUnknown"),
                Part.bUsesObscuredFallback ? 1.0f : 0.0f);
            Dynamic->SetScalarParameterValue(TEXT("TMOP_ObscurityAmount"),
                Part.ObscurityAmount);
        }
    return true;
}

bool ATMOPPlayerAppearanceDirector::ApplyResolvedBody(
    USkeletalMeshComponent* Body, const FTMOPPersonProfileRow& Profile)
{
    if (!IsValid(Body)) return false;

    USkeletalMesh* DesiredBody = ResolvedAppearance.Body.Mesh.LoadSynchronous();
    if (DesiredBody == nullptr && bAutomaticallySelectMannyOrQuinnByGender)
    {
        switch (Profile.Gender)
        {
        case ETMOPPersonGender::Female:
            DesiredBody = FemaleBaseBodyMesh.LoadSynchronous();
            break;
        case ETMOPPersonGender::Male:
            DesiredBody = MaleBaseBodyMesh.LoadSynchronous();
            break;
        default:
            break;
        }
    }

    Body->EmptyOverrideMaterials();
    if (DesiredBody != nullptr)
        Body->SetSkeletalMesh(DesiredBody);
    Body->SetVisibility(Body->GetSkeletalMeshAsset() != nullptr, true);

    // Mirror the NPC appearance path: a catalog material override belongs to
    // the body row and must be installed before the region-mask parameters are
    // written. SetSkeletalMesh restores the mesh's default materials, so doing
    // this only in the Blueprint/player mesh asset is not sufficient.
    if (UMaterialInterface* Material =
        ResolvedAppearance.Body.Material.LoadSynchronous())
    {
        for (int32 Index = 0; Index < Body->GetNumMaterials(); ++Index)
        {
            UMaterialInstanceDynamic* Dynamic =
                Body->CreateDynamicMaterialInstance(Index, Material);
            if (Dynamic == nullptr) continue;
            Dynamic->SetVectorParameterValue(TEXT("PrimaryColor"),
                ResolvedAppearance.Body.PrimaryColor);
            Dynamic->SetVectorParameterValue(TEXT("SecondaryColor"),
                ResolvedAppearance.Body.SecondaryColor);
            Dynamic->SetScalarParameterValue(TEXT("TMOP_IsUnknown"),
                ResolvedAppearance.Body.bUsesObscuredFallback ? 1.0f : 0.0f);
            Dynamic->SetScalarParameterValue(TEXT("TMOP_ObscurityAmount"),
                ResolvedAppearance.Body.ObscurityAmount);
        }
    }
    return Body->GetSkeletalMeshAsset() != nullptr;
}

void ATMOPPlayerAppearanceDirector::ApplyBodyRegionMask(
    USkeletalMeshComponent* Body)
{
    if (!IsValid(Body)) return;
    int32 Mask = 0;
    auto IncludeVisibleSkeletalPart = [this, &Mask](
        const FName ComponentName, const FTMOPResolvedAppearancePart& Part) -> bool
    {
        if (Part.bIntentionallyEmpty) return false;
        for (USkeletalMeshComponent* Component : ManagedPartComponents)
            if (IsValid(Component) && Component->GetFName() == ComponentName &&
                Component->IsVisible() && !Component->bHiddenInGame &&
                Component->GetSkeletalMeshAsset() != nullptr)
            {
                Mask |= Part.HiddenBodyRegions;
                return true;
            }
        return false;
    };
    if (IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Face"),
        ResolvedAppearance.Face))
        Mask |= TMOPBodyRegionMask(ETMOPBodyRegion::Head) | TMOPBodyRegionMask(ETMOPBodyRegion::Neck);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Outerwear"),
        ResolvedAppearance.Outerwear);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_UpperBody"),
        ResolvedAppearance.UpperBody);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Trousers"),
        ResolvedAppearance.Trousers);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Footwear"),
        ResolvedAppearance.Footwear);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Gloves"),
        ResolvedAppearance.Gloves);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Scarf"),
        ResolvedAppearance.Scarf);
    IncludeVisibleSkeletalPart(TEXT("TMOP_Player_Headwear_Legacy"),
        ResolvedAppearance.Headwear);
    if (!ResolvedAppearance.Headwear.bIntentionallyEmpty &&
        IsValid(ManagedHeadwearComponent) &&
        ManagedHeadwearComponent->IsVisible() &&
        !ManagedHeadwearComponent->bHiddenInGame &&
        ManagedHeadwearComponent->GetStaticMesh() != nullptr)
        Mask |= ResolvedAppearance.Headwear.HiddenBodyRegions;

    struct FMaskParameter { const TCHAR* Name; ETMOPBodyRegion Region; };
    const FMaskParameter Parameters[] = {
        { TEXT("TMOP_HideHead"), ETMOPBodyRegion::Head },
        { TEXT("TMOP_HideNeck"), ETMOPBodyRegion::Neck },
        { TEXT("TMOP_HideTorso"), ETMOPBodyRegion::Torso },
        { TEXT("TMOP_HideArms"), ETMOPBodyRegion::Arms },
        { TEXT("TMOP_HideHands"), ETMOPBodyRegion::Hands },
        { TEXT("TMOP_HideHips"), ETMOPBodyRegion::Hips },
        { TEXT("TMOP_HideLegs"), ETMOPBodyRegion::Legs },
        { TEXT("TMOP_HideFeet"), ETMOPBodyRegion::Feet } };

    for (int32 Index = 0; Index < Body->GetNumMaterials(); ++Index)
    {
        UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(
            Body->GetMaterial(Index));
        if (Dynamic == nullptr)
            Dynamic = Body->CreateAndSetMaterialInstanceDynamic(Index);
        if (Dynamic == nullptr) continue;
        for (const FMaskParameter& Parameter : Parameters)
            Dynamic->SetScalarParameterValue(Parameter.Name,
                (Mask & TMOPBodyRegionMask(Parameter.Region)) != 0 ? 1.0f : 0.0f);
    }
}

bool ATMOPPlayerAppearanceDirector::ApplyPlayerAppearance()
{
    ACharacter* Character = ResolveTargetCharacter();
    USkeletalMeshComponent* Body = ResolveBodyMesh(Character);
    UDataTable* Catalog = ResolveAssetCatalog();
    FTMOPPersonProfileRow Profile;
    if (!IsValid(Character) || !IsValid(Body) || !IsValid(Catalog) ||
        !BuildPlayerProfile(Profile))
    {
        return false;
    }

    UTMOPAppearanceResolver::ResolveAppearance(Profile, Catalog,
        ResolvedAppearance);
    // Share the native colour mapping with NPCs. Explicit per-player materials win.
    const auto& HairDefaults = GetDefault<UTMOPCharacterAppearanceComponent>()->HairMaterials;
    FName HairKey = UTMOPAppearanceResolver::GetHairMaterialKey(Profile.Hair, Profile.HairColorCategory);
    if (HairKey == TEXT("Unknown"))
        HairKey = UTMOPAppearanceResolver::GetDeterministicUnknownHairMaterialKey(
            ResolvedAppearance.ResolvedSeed, Profile.AgeAtEvent);
    FName BeardKey = UTMOPAppearanceResolver::GetHairMaterialKey(Profile.BeardOrMustache, ETMOPHairColor::Unknown);
    if (BeardKey == TEXT("Unknown")) BeardKey = HairKey;
    const auto ChooseMaterial = [&HairDefaults](FTMOPResolvedAppearancePart& Part,
        const FTMOPAppearancePartChoice& Choice, const FName Key)
    {
        if (Part.bIntentionallyEmpty || !Choice.MaterialOverride.IsNull()) return;
        if (const auto* Material = HairDefaults.Find(Key))
            if (!Material->IsNull()) Part.Material = *Material;
    };
    ChooseMaterial(ResolvedAppearance.Hair, Profile.AppearanceProfile.Hair, HairKey);
    ChooseMaterial(ResolvedAppearance.FacialHair, Profile.AppearanceProfile.FacialHair, BeardKey);
    for (UStaticMeshComponent* Accessory : ManagedFaceAccessories)
        if (IsValid(Accessory))
        {
            Accessory->SetStaticMesh(nullptr);
            Accessory->SetVisibility(false, true);
        }
    if (!ResolvedAppearance.bUsesBespokeMetaHuman)
        ApplyResolvedBody(Body, Profile);

    // Visual height only: keep feet at the original mesh origin and leave the
    // gameplay capsule/camera untouched. Never multiply the previous scale.
    if (HeightTarget.Get() != Character)
    {
        HeightTarget = Character;
        InitialActorScale = Body->GetRelativeScale3D();
    }
    if (USkeletalMesh* Asset = Body->GetSkeletalMeshAsset())
    {
        const float ReferenceHeight = Asset->GetBounds().BoxExtent.Z * 2.0f;
        if (ReferenceHeight > 1.0f)
            Body->SetRelativeScale3D(InitialActorScale * (Profile.GetResolvedHeightCentimeters() / ReferenceHeight));
    }

    bool bSuccess = Body->GetSkeletalMeshAsset() != nullptr;
    if (!ResolvedAppearance.bUsesBespokeMetaHuman)
    {
        bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Face"),
            ResolvedAppearance.Face);
        bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Hair"),
            ResolvedAppearance.Hair);
        bSuccess &= ApplyResolvedFaceAccessory(Character, Body,
            TEXT("TMOP_Player_FacialHair"), TEXT("TMOP_Player_FacialHair_Static"),
            TEXT("FacialHairSocket"), ResolvedAppearance.Face.HeadAccessoryFit.FacialHairOffset,
            ResolvedAppearance.FacialHair);
    }
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Outerwear"),
        ResolvedAppearance.Outerwear);
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_UpperBody"),
        ResolvedAppearance.UpperBody);
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Trousers"),
        ResolvedAppearance.Trousers);
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Footwear"),
        ResolvedAppearance.Footwear);
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Gloves"),
        ResolvedAppearance.Gloves);
    bSuccess &= ApplyResolvedHeadwear(Character, Body,
        ResolvedAppearance.Headwear);
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Scarf"),
        ResolvedAppearance.Scarf);
    bSuccess &= ApplyResolvedFaceAccessory(Character, Body,
        TEXT("TMOP_Player_Glasses"), TEXT("TMOP_Player_Glasses_Static"), TEXT("GlassesSocket"),
        ResolvedAppearance.Face.HeadAccessoryFit.GlassesOffset, ResolvedAppearance.Glasses);

    // Leader Pose shares bone transforms, not the authored clothing shapes.
    // Apply the same policy as NPCs explicitly to each visible skeletal piece.
    const ETMOPBodyBuild BodyBuild = Profile.GetResolvedBodyBuild();
    if (!ResolvedAppearance.bUsesBespokeMetaHuman)
        UTMOPCharacterAppearanceComponent::ApplyMorphs(Body, Profile.AppearanceProfile, BodyBuild);
    for (USkeletalMeshComponent* Part : ManagedPartComponents)
    {
        if (!IsValid(Part) || !Part->IsVisible()) continue;
        if (ResolvedAppearance.bUsesBespokeMetaHuman &&
            (Part->GetFName() == TEXT("TMOP_Player_Face") ||
             Part->GetFName() == TEXT("TMOP_Player_Hair") ||
             Part->GetFName() == TEXT("TMOP_Player_FacialHair"))) continue;
        UTMOPCharacterAppearanceComponent::ApplyMorphs(Part, Profile.AppearanceProfile, BodyBuild);
    }
    ApplyBodyRegionMask(Body);
    bHasAppliedAppearance = bSuccess;
    if (bSuccess)
        UE_LOG(LogTemp, Log, TEXT(
            "TMOP PlayerAppearanceDirector applied appearance to '%s'."),
            *Character->GetName());
    return bSuccess;
}

void ATMOPPlayerAppearanceDirector::ClearPlayerAppearance()
{
    for (USkeletalMeshComponent* Component : ManagedPartComponents)
    {
        if (!IsValid(Component)) continue;
        Component->SetSkeletalMesh(nullptr);
        Component->SetVisibility(false, true);
    }
    ManagedPartComponents.Reset();
    for (UStaticMeshComponent* Accessory : ManagedFaceAccessories)
        if (IsValid(Accessory))
        {
            Accessory->SetStaticMesh(nullptr);
            Accessory->EmptyOverrideMaterials();
            Accessory->SetVisibility(false, true);
        }
    ManagedFaceAccessories.Reset();
    if (IsValid(ManagedHeadwearComponent))
    {
        ManagedHeadwearComponent->SetStaticMesh(nullptr);
        ManagedHeadwearComponent->SetVisibility(false, true);
    }
    ResolvedAppearance = FTMOPResolvedAppearance();
    bHasAppliedAppearance = false;
}

void ATMOPPlayerAppearanceDirector::ConfigureForLocalPlayer(
    const ATMOPPlayerAppearanceDirector* TemplateDirector, ACharacter* Character, int32 Slot)
{
    if (!TemplateDirector || !Character) return;
    TargetCharacterOverride = Character;
    // Preserve an explicitly selected mesh on this pawn, but never share P1's
    // mesh component with another local player's pawn.
    if (IsValid(BodyMeshOverride) && BodyMeshOverride->GetOwner() != Character)
        BodyMeshOverride = nullptr;
    PlayerIndex = Slot;
    bUsePersonProfileRow = TemplateDirector->bUsePersonProfileRow;
    PlayerProfileRow = TemplateDirector->PlayerProfileRow;
    PlayerGender = TemplateDirector->PlayerGender;
    InlineAppearanceProfile = TemplateDirector->InlineAppearanceProfile;
    AppearanceAssetTableOverride = TemplateDirector->AppearanceAssetTableOverride;
    bAutomaticallySelectMannyOrQuinnByGender = TemplateDirector->bAutomaticallySelectMannyOrQuinnByGender;
    MaleBaseBodyMesh = TemplateDirector->MaleBaseBodyMesh;
    FemaleBaseBodyMesh = TemplateDirector->FemaleBaseBodyMesh;
    DefaultHeadwearSocket = TemplateDirector->DefaultHeadwearSocket;
    HeadwearFallbackBone = TemplateDirector->HeadwearFallbackBone;
    if (TemplateDirector->LocalPlayerAppearances.IsValidIndex(Slot))
    {
        const auto& Preset = TemplateDirector->LocalPlayerAppearances[Slot];
        bUsePersonProfileRow = Preset.bUsePersonProfileRow;
        PlayerProfileRow = Preset.PlayerProfileRow;
        PlayerGender = Preset.PlayerGender;
        InlineAppearanceProfile = Preset.Appearance;
    }
    LoadEditedProfile();
}

bool ATMOPPlayerAppearanceDirector::RefreshPlayerAppearance()
{
    ClearPlayerAppearance();
    return ApplyPlayerAppearance();
}

void ATMOPPlayerAppearanceDirector::TryStartupApply()
{
    if (!bHasEditedProfile) LoadEditedProfile();
    if (ApplyPlayerAppearance())
    {
        GetWorldTimerManager().ClearTimer(StartupRetryTimer);
        return;
    }
    if (RemainingStartupRetries-- <= 0)
    {
        UE_LOG(LogTemp, Error, TEXT(
            "TMOP PlayerAppearanceDirector could not apply appearance after startup retries."));
        return;
    }
    GetWorldTimerManager().SetTimer(StartupRetryTimer, this,
        &ATMOPPlayerAppearanceDirector::TryStartupApply,
        StartupRetryIntervalSeconds, false);
}

void ATMOPPlayerAppearanceDirector::ValidatePlayerAppearance()
{
    TArray<FString> Problems;
    if (!IsValid(ResolveTargetCharacter()) && !IsValid(TargetCharacterOverride))
        Problems.Add(TEXT("No target player exists yet. This is normal outside PIE unless Target Character Override is set."));
    if (!IsValid(ResolveAssetCatalog()))
        Problems.Add(TEXT("No Appearance Asset Table is available."));
    FTMOPPersonProfileRow Profile;
    const bool bProfileValid = BuildPlayerProfile(Profile);
    if (!bProfileValid)
        Problems.Add(TEXT("Player profile is invalid."));
    UDataTable* Catalog = ResolveAssetCatalog();
    if (bProfileValid && IsValid(Catalog))
    {
        FTMOPResolvedAppearance Preview;
        UTMOPAppearanceResolver::ResolveAppearance(Profile, Catalog, Preview);
        if (Preview.Headwear.StaticMesh.IsNull() &&
            !Preview.Headwear.Mesh.IsNull())
            Problems.Add(TEXT(
                "Selected headwear still uses legacy Skeletal Mesh; assign StaticMesh for socket attachment."));
    }

    if (Problems.IsEmpty())
    {
        UE_LOG(LogTemp, Log, TEXT("TMOP PlayerAppearanceDirector validation passed."));
        return;
    }
    for (const FString& Problem : Problems)
        UE_LOG(LogTemp, Warning, TEXT("TMOP PlayerAppearanceDirector: %s"), *Problem);
}
