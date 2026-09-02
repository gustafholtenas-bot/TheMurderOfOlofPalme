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
    Component->SetLeaderPoseComponent(Body);
    Component->SetVisibility(true, true);

    if (UMaterialInterface* Material = Part.Material.LoadSynchronous())
    {
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
    }
    return true;
}

UStaticMeshComponent* ATMOPPlayerAppearanceDirector::EnsureHeadwearComponent(
    ACharacter* Character, USkeletalMeshComponent* Body)
{
    if (!IsValid(Character) || !IsValid(Body)) return nullptr;
    if (IsValid(ManagedHeadwearComponent)) return ManagedHeadwearComponent;

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
    Component->SetRelativeTransform(Part.AttachmentTransform);
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

    if (DesiredBody != nullptr)
        Body->SetSkeletalMesh(DesiredBody);
    Body->SetVisibility(Body->GetSkeletalMeshAsset() != nullptr, true);
    return Body->GetSkeletalMeshAsset() != nullptr;
}

void ATMOPPlayerAppearanceDirector::ApplyBodyRegionMask(
    USkeletalMeshComponent* Body)
{
    if (!IsValid(Body)) return;
    const int32 Mask = ResolvedAppearance.Outerwear.HiddenBodyRegions |
        ResolvedAppearance.UpperBody.HiddenBodyRegions |
        ResolvedAppearance.Trousers.HiddenBodyRegions |
        ResolvedAppearance.Footwear.HiddenBodyRegions |
        ResolvedAppearance.Gloves.HiddenBodyRegions |
        ResolvedAppearance.Headwear.HiddenBodyRegions |
        ResolvedAppearance.Scarf.HiddenBodyRegions;

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
                (Mask & static_cast<int32>(Parameter.Region)) != 0 ? 1.0f : 0.0f);
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
    if (!ResolvedAppearance.bUsesBespokeMetaHuman)
        ApplyResolvedBody(Body, Profile);

    bool bSuccess = Body->GetSkeletalMeshAsset() != nullptr;
    if (!ResolvedAppearance.bUsesBespokeMetaHuman)
    {
        bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Face"),
            ResolvedAppearance.Face);
        bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Hair"),
            ResolvedAppearance.Hair);
        bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_FacialHair"),
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
    bSuccess &= ApplyResolvedPart(Character, Body, TEXT("TMOP_Player_Glasses"),
        ResolvedAppearance.Glasses);

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
    if (IsValid(ManagedHeadwearComponent))
    {
        ManagedHeadwearComponent->SetStaticMesh(nullptr);
        ManagedHeadwearComponent->SetVisibility(false, true);
    }
    ResolvedAppearance = FTMOPResolvedAppearance();
    bHasAppliedAppearance = false;
}

bool ATMOPPlayerAppearanceDirector::RefreshPlayerAppearance()
{
    ClearPlayerAppearance();
    return ApplyPlayerAppearance();
}

void ATMOPPlayerAppearanceDirector::TryStartupApply()
{
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
