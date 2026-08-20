#include "People/TMOPCharacterAppearanceComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "People/TMOPAppearanceResolver.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"

UTMOPCharacterAppearanceComponent::UTMOPCharacterAppearanceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPCharacterAppearanceComponent::BeginPlay()
{
    Super::BeginPlay();
    if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner()))
        CacheBaseBodyTransform(Agent);
}

UDataTable* UTMOPCharacterAppearanceComponent::ResolveAssetCatalog() const
{
    if (IsValid(AssetCatalogOverride)) return AssetCatalogOverride;
    const UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr ? World->GetGameInstance() : nullptr;
    const UTMOPPersonRegistrySubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    return Registry != nullptr ? Registry->GetAppearanceAssetTable() : nullptr;
}

void UTMOPCharacterAppearanceComponent::CacheBaseBodyTransform(
    ATMOPHistoricalAgent* Agent)
{
    if (bBaseBodyTransformCached || !IsValid(Agent) || !IsValid(Agent->BodyMesh)) return;
    BaseBodyRelativeLocation = Agent->BodyMesh->GetRelativeLocation();
    BaseBodyRelativeScale = Agent->BodyMesh->GetRelativeScale3D();
    bBaseBodyTransformCached = true;
}

bool UTMOPCharacterAppearanceComponent::ApplyAppearance()
{
    ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    UTMOPPersonProfileComponent* ProfileComponent = IsValid(Agent)
        ? Agent->FindComponentByClass<UTMOPPersonProfileComponent>() : nullptr;
    if (!IsValid(Agent) || !IsValid(ProfileComponent)) return false;
    if (!ProfileComponent->bHasLoadedProfile)
    {
        if (!ProfileComponent->LoadProfile()) return false;
        // LoadProfile applies appearance after setting bHasLoadedProfile.
        if (bHasAppliedAppearance) return true;
    }

    CacheBaseBodyTransform(Agent);
    UTMOPAppearanceResolver::ResolveAppearance(
        ProfileComponent->Profile, ResolveAssetCatalog(), ResolvedAppearance);
    const bool bMetaHuman = ResolvedAppearance.bUsesBespokeMetaHuman;
    if (!bMetaHuman)
    {
        ApplyBodyAndProportions(Agent);
        ApplyPart(Agent->FaceMesh, ResolvedAppearance.Face, false);
        ApplyPart(Agent->HairMesh, ResolvedAppearance.Hair, false);
        ApplyPart(Agent->FacialHairMesh, ResolvedAppearance.FacialHair, false);
    }
    ApplyPart(Agent->OuterwearMesh, ResolvedAppearance.Outerwear, false);
    ApplyPart(Agent->UpperBodyMesh, ResolvedAppearance.UpperBody, false);
    ApplyPart(Agent->TrousersMesh, ResolvedAppearance.Trousers, false);
    ApplyPart(Agent->FootwearMesh, ResolvedAppearance.Footwear, false);
    ApplyPart(Agent->GlovesMesh, ResolvedAppearance.Gloves, false);
    ApplyPart(Agent->HeadwearMesh, ResolvedAppearance.Headwear, false);
    ApplyPart(Agent->ScarfMesh, ResolvedAppearance.Scarf, false);
    ApplyPart(Agent->GlassesMesh, ResolvedAppearance.Glasses, false);
    ApplyModularMorphs(Agent, ProfileComponent->Profile.AppearanceProfile,
        ResolvedAppearance.BodyBuild, !bMetaHuman);
    if (!bMetaHuman)
        ApplyFaceMorphs(Agent->FaceMesh, ResolvedAppearance.FaceMorphs);
    ApplyBodyRegionMask(Agent);
    ApplyCollisionAndPresentation(Agent,
        bMetaHuman && bPreserveMetaHumanBodyPlacement);
    ApplyPerformanceSettings(Agent);
    bHasAppliedAppearance = true;
    return true;
}

void UTMOPCharacterAppearanceComponent::ApplyFaceMorphs(
    USkeletalMeshComponent* Face, const FTMOPResolvedFaceMorphs& Morphs)
{
    if (!IsValid(Face) || Face->GetSkeletalMeshAsset() == nullptr) return;
    Face->SetMorphTarget(TEXT("TMOP_FaceWidth"), Morphs.FaceWidth);
    Face->SetMorphTarget(TEXT("TMOP_JawWidth"), Morphs.JawWidth);
    Face->SetMorphTarget(TEXT("TMOP_JawProjection"), Morphs.JawProjection);
    Face->SetMorphTarget(TEXT("TMOP_CheekboneProminence"),
        Morphs.CheekboneProminence);
    Face->SetMorphTarget(TEXT("TMOP_NoseWidth"), Morphs.NoseWidth);
    Face->SetMorphTarget(TEXT("TMOP_NoseLength"), Morphs.NoseLength);
    Face->SetMorphTarget(TEXT("TMOP_BrowHeight"), Morphs.BrowHeight);
    Face->SetMorphTarget(TEXT("TMOP_EyeSpacing"), Morphs.EyeSpacing);
    Face->SetMorphTarget(TEXT("TMOP_LipThickness"), Morphs.LipThickness);
    Face->SetMorphTarget(TEXT("TMOP_Age"), Morphs.Age);
}

void UTMOPCharacterAppearanceComponent::ApplyBodyAndProportions(
    ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || !IsValid(Agent->BodyMesh)) return;
    ApplyPart(Agent->BodyMesh, ResolvedAppearance.Body, true);
    const UTMOPPersonProfileComponent* ProfileComponent =
        Agent->FindComponentByClass<UTMOPPersonProfileComponent>();
    if (ProfileComponent == nullptr) return;
    const float HeightScale = FMath::Clamp(
        ResolvedAppearance.HeightCentimeters /
            FMath::Max(1.0f, BodyReferenceHeightCentimeters), 0.66f, 1.2f);
    FVector Scale = BaseBodyRelativeScale;
    Scale.Z *= HeightScale;
    Agent->BodyMesh->SetRelativeScale3D(Scale);
    ApplyMorphs(Agent->BodyMesh, ProfileComponent->Profile.AppearanceProfile,
        ResolvedAppearance.BodyBuild);
}

void UTMOPCharacterAppearanceComponent::ApplyMorphs(
    USkeletalMeshComponent* Body, const FTMOPAppearanceProfile& Profile,
    const ETMOPBodyBuild BodyBuild)
{
    if (!IsValid(Body) || Body->GetSkeletalMeshAsset() == nullptr) return;
    float DefaultWeight = 0.0f;
    float DefaultMuscularity = 0.0f;
    switch (BodyBuild)
    {
    case ETMOPBodyBuild::Thin:     DefaultWeight = -0.75f; break;
    case ETMOPBodyBuild::Slim:     DefaultWeight = -0.40f; break;
    case ETMOPBodyBuild::Athletic: DefaultWeight = -0.10f; DefaultMuscularity = 0.45f; break;
    case ETMOPBodyBuild::Strong:   DefaultWeight = 0.15f; DefaultMuscularity = 0.80f; break;
    case ETMOPBodyBuild::Heavy:    DefaultWeight = 0.75f; DefaultMuscularity = 0.10f; break;
    default: break;
    }
    const float Weight = FMath::IsNearlyZero(Profile.BodyWeightMorph)
        ? DefaultWeight : Profile.BodyWeightMorph;
    const float Muscularity = FMath::IsNearlyZero(Profile.MuscularityMorph)
        ? DefaultMuscularity : Profile.MuscularityMorph;
    Body->SetMorphTarget(TEXT("TMOP_BodyWeight"), Weight);
    Body->SetMorphTarget(TEXT("TMOP_Muscularity"), Muscularity);
    Body->SetMorphTarget(TEXT("TMOP_HeadScale"), Profile.HeadScale - 1.0f);
    Body->SetMorphTarget(TEXT("TMOP_ShoulderScale"), Profile.ShoulderScale - 1.0f);
    Body->SetMorphTarget(TEXT("TMOP_TorsoLength"), Profile.TorsoLengthScale - 1.0f);
    Body->SetMorphTarget(TEXT("TMOP_ArmLength"), Profile.ArmLengthScale - 1.0f);
    Body->SetMorphTarget(TEXT("TMOP_LegLength"), Profile.LegLengthScale - 1.0f);
}

void UTMOPCharacterAppearanceComponent::ApplyModularMorphs(
    ATMOPHistoricalAgent* Agent, const FTMOPAppearanceProfile& Profile,
    const ETMOPBodyBuild BodyBuild, const bool bIncludeBespokeHeadParts)
{
    if (!IsValid(Agent)) return;
    const TArray<USkeletalMeshComponent*> Parts = {
        Agent->FaceMesh.Get(), Agent->HairMesh.Get(), Agent->FacialHairMesh.Get(),
        Agent->ScarfMesh.Get(), Agent->GlassesMesh.Get(), Agent->OuterwearMesh.Get(),
        Agent->UpperBodyMesh.Get(), Agent->TrousersMesh.Get(),
        Agent->FootwearMesh.Get(), Agent->GlovesMesh.Get(),
        Agent->HeadwearMesh.Get() };
    for (USkeletalMeshComponent* Part : Parts)
        if (IsValid(Part) && Part->IsVisible() &&
            (bIncludeBespokeHeadParts ||
             (Part != Agent->FaceMesh.Get() && Part != Agent->HairMesh.Get() &&
              Part != Agent->FacialHairMesh.Get())))
            ApplyMorphs(Part, Profile, BodyBuild);
}

void UTMOPCharacterAppearanceComponent::ApplyBodyRegionMask(
    ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || !IsValid(Agent->BodyMesh)) return;
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
    for (int32 Index = 0; Index < Agent->BodyMesh->GetNumMaterials(); ++Index)
    {
        UMaterialInstanceDynamic* Dynamic = Cast<UMaterialInstanceDynamic>(
            Agent->BodyMesh->GetMaterial(Index));
        if (Dynamic == nullptr)
            Dynamic = Agent->BodyMesh->CreateAndSetMaterialInstanceDynamic(Index);
        if (Dynamic == nullptr) continue;
        for (const FMaskParameter& Parameter : Parameters)
            Dynamic->SetScalarParameterValue(Parameter.Name,
                (Mask & static_cast<int32>(Parameter.Region)) != 0 ? 1.0f : 0.0f);
    }
}

void UTMOPCharacterAppearanceComponent::ApplyPart(
    USkeletalMeshComponent* Component, const FTMOPResolvedAppearancePart& Part,
    const bool bPreserveMeshWhenMissing)
{
    if (!IsValid(Component)) return;
    if (Part.bIntentionallyEmpty)
    {
        Component->SetVisibility(false, true);
        return;
    }
    if (USkeletalMesh* Mesh = Part.Mesh.LoadSynchronous())
    {
        const ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
        const USkeletalMesh* BodyAsset = IsValid(Agent) && IsValid(Agent->BodyMesh)
            ? Agent->BodyMesh->GetSkeletalMeshAsset() : nullptr;
        if (IsValid(Agent) && Component != Agent->BodyMesh && BodyAsset != nullptr &&
            BodyAsset->GetSkeleton() != nullptr && Mesh->GetSkeleton() != nullptr &&
            BodyAsset->GetSkeleton() != Mesh->GetSkeleton())
        {
            ResolvedAppearance.Diagnostics.Add(FString::Printf(
                TEXT("Asset '%s' has an incompatible skeleton."),
                *Part.CatalogId.ToString()));
            Component->SetVisibility(false, true);
            return;
        }
        Component->SetSkeletalMesh(Mesh);
        if (IsValid(Agent) && Component != Agent->BodyMesh)
            Component->SetLeaderPoseComponent(Agent->BodyMesh);
    }
    else if (!bPreserveMeshWhenMissing)
    {
        Component->SetVisibility(false, true);
        return;
    }
    Component->SetVisibility(true, true);
    UMaterialInterface* Material = Part.Material.LoadSynchronous();
    if (Part.bUsesObscuredFallback && Material == nullptr)
        Material = ObscuredMaterialOverride.LoadSynchronous();
    if (Material != nullptr)
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

void UTMOPCharacterAppearanceComponent::ApplyCollisionAndPresentation(
    ATMOPHistoricalAgent* Agent, const bool bPreserveBespokeBodyPlacement)
{
    if (!IsValid(Agent)) return;
    const float Height = ResolvedAppearance.HeightCentimeters;
    const float Radius = FMath::Clamp(Height * 0.155f, 20.0f, 34.0f);
    const float HalfHeight = FMath::Max(Radius + 1.0f, Height * 0.5f);
    Agent->PedestrianCapsuleRadiusCm = Radius;
    Agent->GetCapsuleComponent()->SetCapsuleSize(Radius, HalfHeight, true);
    if (!bPreserveBespokeBodyPlacement && bBaseBodyTransformCached &&
        IsValid(Agent->BodyMesh))
    {
        FVector Location = BaseBodyRelativeLocation;
        Location.Z = -HalfHeight;
        Agent->BodyMesh->SetRelativeLocation(Location);
    }
    Agent->BaseEyeHeight = FMath::Clamp(Height * 0.43f, 48.0f, 90.0f);
    Agent->NameLabelHeightCm = HalfHeight + 15.0f;
    Agent->SpeechBubbleHeightCm = HalfHeight + 115.0f;
    Agent->AppearanceMovementSpeedMultiplier = bAdjustMovementSpeedForHeight
        ? FMath::Clamp(FMath::Sqrt(Height / 171.5f), 0.88f, 1.12f) : 1.0f;
    Agent->RefreshNameLabel();
    Agent->ApplyMovementSpeedForActivity();
}

void UTMOPCharacterAppearanceComponent::ApplyPerformanceSettings(
    ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent)) return;
    TArray<USkeletalMeshComponent*> Parts;
    if (bApplyPerformanceSettingsToAdditionalSkeletalMeshes)
        Agent->GetComponents<USkeletalMeshComponent>(Parts);
    else
        Parts = { Agent->BodyMesh.Get(), Agent->FaceMesh.Get(),
            Agent->HairMesh.Get(), Agent->FacialHairMesh.Get(),
            Agent->ScarfMesh.Get(), Agent->GlassesMesh.Get(),
            Agent->OuterwearMesh.Get(), Agent->UpperBodyMesh.Get(),
            Agent->TrousersMesh.Get(), Agent->FootwearMesh.Get(),
            Agent->GlovesMesh.Get(), Agent->HeadwearMesh.Get() };
    for (USkeletalMeshComponent* Part : Parts)
    {
        if (!IsValid(Part)) continue;
        Part->SetCullDistance(CullDistanceCentimeters);
        Part->bAllowCullDistanceVolume = true;
        Part->VisibilityBasedAnimTickOption =
            EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
        Part->bEnableUpdateRateOptimizations =
            bEnableAnimationUpdateRateOptimizations;
    }
}

void UTMOPCharacterAppearanceComponent::ResetAppearance()
{
    ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    if (!IsValid(Agent)) return;
    const bool bWasMetaHuman = ResolvedAppearance.bUsesBespokeMetaHuman;
    if (bBaseBodyTransformCached && IsValid(Agent->BodyMesh))
    {
        Agent->BodyMesh->SetRelativeLocation(BaseBodyRelativeLocation);
        Agent->BodyMesh->SetRelativeScale3D(BaseBodyRelativeScale);
        if (!bWasMetaHuman)
            Agent->BodyMesh->ClearMorphTargets();
    }
    TArray<USkeletalMeshComponent*> ModularParts = {
        Agent->ScarfMesh.Get(), Agent->GlassesMesh.Get(), Agent->OuterwearMesh.Get(),
        Agent->UpperBodyMesh.Get(), Agent->TrousersMesh.Get(),
        Agent->FootwearMesh.Get(), Agent->GlovesMesh.Get(), Agent->HeadwearMesh.Get() };
    if (!bWasMetaHuman)
    {
        ModularParts.Add(Agent->FaceMesh.Get());
        ModularParts.Add(Agent->HairMesh.Get());
        ModularParts.Add(Agent->FacialHairMesh.Get());
    }
    for (USkeletalMeshComponent* Part : ModularParts)
        if (IsValid(Part)) { Part->SetSkeletalMesh(nullptr); Part->SetVisibility(false, true); }
    ResolvedAppearance = FTMOPResolvedAppearance();
    ApplyBodyRegionMask(Agent);
    Agent->AppearanceMovementSpeedMultiplier = 1.0f;
    bHasAppliedAppearance = false;
}

bool UTMOPCharacterAppearanceComponent::ValidateAppearance(
    TArray<FString>& OutWarnings) const
{
    OutWarnings = ResolvedAppearance.Diagnostics;
    if (!IsValid(ResolveAssetCatalog()))
        OutWarnings.Add(TEXT("No DT_TMOP_AppearanceAssets table is configured."));
    return OutWarnings.IsEmpty();
}
