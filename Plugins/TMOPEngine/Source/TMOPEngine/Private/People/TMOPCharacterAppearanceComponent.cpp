#include "People/TMOPCharacterAppearanceComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Components/ChildActorComponent.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "People/TMOPAppearanceResolver.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"

UTMOPCharacterAppearanceComponent::UTMOPCharacterAppearanceComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    MaleBaseBodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT(
        "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple")));
    FemaleBaseBodyMesh = TSoftObjectPtr<USkeletalMesh>(FSoftObjectPath(TEXT(
        "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple.SKM_Quinn_Simple")));
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

    ClearHybridHead();
    CacheBaseBodyTransform(Agent);
    FTMOPPersonProfileRow ProfileForAppearance = ProfileComponent->Profile;
    if (bOuterwearOnlyPilotMode && bForceOuterwearOnEveryoneInPilotMode &&
        !ProfileComponent->Profile.AppearanceProfile.bUseMetaHumanHybridHead)
    {
        if (ProfileForAppearance.OuterwearCategory == ETMOPOuterwearType::None)
            ProfileForAppearance.OuterwearCategory = ETMOPOuterwearType::Unknown;
        if (ProfileForAppearance.AppearanceProfile.UnknownPartStyle ==
            ETMOPUnknownAppearanceStyle::Hidden)
            ProfileForAppearance.AppearanceProfile.UnknownPartStyle =
                ETMOPUnknownAppearanceStyle::Obscured;
    }
    UTMOPAppearanceResolver::ResolveAppearance(
        ProfileForAppearance, ResolveAssetCatalog(), ResolvedAppearance);

    // Hybrid has a normal modular TMOP body even if the old MetaHuman mode was selected.
    if (ProfileComponent->Profile.AppearanceProfile.bUseMetaHumanHybridHead)
        ResolvedAppearance.bUsesBespokeMetaHuman = false;
    const bool bMetaHuman = ResolvedAppearance.bUsesBespokeMetaHuman;
    const bool bHasExplicitResolvedBody = !ResolvedAppearance.Body.Mesh.IsNull();
    if (!bMetaHuman && bAutomaticallySelectMannyOrQuinnByGender &&
        ((bOuterwearOnlyPilotMode &&
          !ProfileComponent->Profile.AppearanceProfile.bUseMetaHumanHybridHead) ||
         !bHasExplicitResolvedBody))
    {
        ApplyAutomaticGenderBody(Agent, ProfileComponent->Profile.Gender);
    }

    if (bOuterwearOnlyPilotMode &&
        !ProfileComponent->Profile.AppearanceProfile.bUseMetaHumanHybridHead)
    {
        const bool bOuterwearApplied = ApplyPart(
            Agent->OuterwearMesh, ResolvedAppearance.Outerwear, false);
        if (bOuterwearApplied)
        {
            ApplyMorphs(Agent->OuterwearMesh,
                ProfileComponent->Profile.AppearanceProfile,
                ResolvedAppearance.BodyBuild);
            ApplyBodyRegionMask(Agent);
            ApplyPerformanceSettings(Agent);
        }
        else
        {
            const FString CatalogId =
                ResolvedAppearance.Outerwear.CatalogId.IsNone()
                ? TEXT("<none>")
                : ResolvedAppearance.Outerwear.CatalogId.ToString();
            UE_LOG(LogTemp, Error, TEXT(
                "TMOP jacket pilot: '%s' could not apply outerwear '%s'. "
                "Assign a Manny/Quinn-compatible Skeletal Mesh in "
                "DT_TMOP_AppearanceAssets."),
                *ProfileComponent->Profile.EntityId.ToString(), *CatalogId);
        }
        bHasAppliedAppearance = bOuterwearApplied;
        return bOuterwearApplied;
    }

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
    ApplyHeadwear(Agent, ResolvedAppearance.Headwear);
    ApplyPart(Agent->ScarfMesh, ResolvedAppearance.Scarf, false);
    ApplyPart(Agent->GlassesMesh, ResolvedAppearance.Glasses, false);
    ApplyModularMorphs(Agent, ProfileComponent->Profile.AppearanceProfile,
        ResolvedAppearance.BodyBuild, !bMetaHuman);
    if (!bMetaHuman)
        ApplyFaceMorphs(Agent->FaceMesh, ResolvedAppearance.FaceMorphs);
    ApplyHybridHead(Agent);
    ApplyBodyRegionMask(Agent);
    ApplyCollisionAndPresentation(Agent,
        bMetaHuman && bPreserveMetaHumanBodyPlacement);
    ApplyPerformanceSettings(Agent);
    bHasAppliedAppearance = true;
    return true;
}

bool UTMOPCharacterAppearanceComponent::ApplyAutomaticGenderBody(
    ATMOPHistoricalAgent* Agent, const ETMOPPersonGender Gender)
{
    if (!IsValid(Agent) || !IsValid(Agent->BodyMesh) ||
        !bAutomaticallySelectMannyOrQuinnByGender)
    {
        return false;
    }

    const TSoftObjectPtr<USkeletalMesh>* DesiredBody = nullptr;
    const TCHAR* BodyLabel = TEXT("existing body");
    switch (Gender)
    {
    case ETMOPPersonGender::Female:
        DesiredBody = &FemaleBaseBodyMesh;
        BodyLabel = TEXT("Quinn");
        break;
    case ETMOPPersonGender::Male:
        DesiredBody = &MaleBaseBodyMesh;
        BodyLabel = TEXT("Manny");
        break;
    default:
        // Do not invent a gender when the historical evidence is unknown.
        return false;
    }

    USkeletalMesh* Mesh = DesiredBody != nullptr
        ? DesiredBody->LoadSynchronous() : nullptr;
    if (Mesh == nullptr)
    {
        const FString Path = DesiredBody != nullptr
            ? DesiredBody->ToSoftObjectPath().ToString() : TEXT("<none>");
        const FString Message = FString::Printf(TEXT(
            "Automatic %s body could not load '%s'; keeping the existing body."),
            BodyLabel, *Path);
        ResolvedAppearance.Diagnostics.Add(Message);
        UE_LOG(LogTemp, Warning, TEXT("TMOP appearance: %s"), *Message);
        return false;
    }

    Agent->BodyMesh->SetSkeletalMesh(Mesh);
    Agent->BodyMesh->SetVisibility(true, true);
    UE_LOG(LogTemp, Verbose, TEXT(
        "TMOP appearance: '%s' automatically uses %s body '%s'."),
        *Agent->GetName(), BodyLabel, *Mesh->GetName());
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
    // Authored relative shapes use Body_workout as the Basis (all values zero).
    // ApplyMorphs is shared by the body and modular garments, so their names
    // and category weights stay in sync without overwriting unrelated morphs.
    USkeletalMesh* Mesh = Body->GetSkeletalMeshAsset();
    const FName NormalName(TEXT("Body_normal"));
    const FName HeavyName(TEXT("Body_heavy"));
    const FName ThinName(TEXT("Body_thin"));
    const bool bUsesAuthoredBodyShapes = Mesh->FindMorphTarget(NormalName) != nullptr ||
        Mesh->FindMorphTarget(HeavyName) != nullptr ||
        Mesh->FindMorphTarget(ThinName) != nullptr;
    if (bUsesAuthoredBodyShapes)
    {
        float Normal = 0.0f, Heavy = 0.0f, Thin = 0.0f;
        switch (BodyBuild)
        {
        case ETMOPBodyBuild::Thin: Thin = 1.0f; break;
        case ETMOPBodyBuild::Slim: Normal = 0.5f; Thin = 0.5f; break;
        case ETMOPBodyBuild::Heavy: Heavy = 1.0f; break;
        case ETMOPBodyBuild::Athletic:
        case ETMOPBodyBuild::Strong: break; // Strong uses workout until authored.
        default: Normal = 1.0f; break;
        }
        // Always write zeroes as well: Heavy -> Average/Workout must reset Heavy.
        if (Mesh->FindMorphTarget(NormalName)) Body->SetMorphTarget(NormalName, Normal);
        if (Mesh->FindMorphTarget(HeavyName)) Body->SetMorphTarget(HeavyName, Heavy);
        if (Mesh->FindMorphTarget(ThinName)) Body->SetMorphTarget(ThinName, Thin);
        // Do not stack the old signed weight/muscularity system on these shapes.
        if (Mesh->FindMorphTarget(TEXT("TMOP_BodyWeight")))
            Body->SetMorphTarget(TEXT("TMOP_BodyWeight"), 0.0f);
        if (Mesh->FindMorphTarget(TEXT("TMOP_Muscularity")))
            Body->SetMorphTarget(TEXT("TMOP_Muscularity"), 0.0f);
    }
    else
    {
        // Preserve existing assets that use the original TMOP morph convention.
        Body->SetMorphTarget(TEXT("TMOP_BodyWeight"), Weight);
        Body->SetMorphTarget(TEXT("TMOP_Muscularity"), Muscularity);
    }
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
        Agent->FootwearMesh.Get(), Agent->GlovesMesh.Get() };
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
    // Rebuild from zero each appearance application. A failed/hidden garment
    // must not remove skin. Face only masks regions explicitly set in its row.
    int32 Mask = bHybridHeadActive ? static_cast<int32>(ETMOPBodyRegion::Head) : 0;
    auto IncludeVisibleSkeletalPart = [&Mask](
        USkeletalMeshComponent* Component, const FTMOPResolvedAppearancePart& Part)
    {
        if (!Part.bIntentionallyEmpty && IsValid(Component) &&
            Component->IsVisible() && !Component->bHiddenInGame &&
            Component->GetSkeletalMeshAsset() != nullptr)
            Mask |= Part.HiddenBodyRegions;
    };
    IncludeVisibleSkeletalPart(Agent->FaceMesh.Get(), ResolvedAppearance.Face);
    IncludeVisibleSkeletalPart(Agent->OuterwearMesh.Get(), ResolvedAppearance.Outerwear);
    IncludeVisibleSkeletalPart(Agent->UpperBodyMesh.Get(), ResolvedAppearance.UpperBody);
    IncludeVisibleSkeletalPart(Agent->TrousersMesh.Get(), ResolvedAppearance.Trousers);
    IncludeVisibleSkeletalPart(Agent->FootwearMesh.Get(), ResolvedAppearance.Footwear);
    IncludeVisibleSkeletalPart(Agent->GlovesMesh.Get(), ResolvedAppearance.Gloves);
    IncludeVisibleSkeletalPart(Agent->ScarfMesh.Get(), ResolvedAppearance.Scarf);
    IncludeVisibleSkeletalPart(Agent->HeadwearMesh.Get(), ResolvedAppearance.Headwear);
    if (!ResolvedAppearance.Headwear.bIntentionallyEmpty &&
        IsValid(Agent->HeadwearStaticMesh) &&
        Agent->HeadwearStaticMesh->IsVisible() &&
        !Agent->HeadwearStaticMesh->bHiddenInGame &&
        Agent->HeadwearStaticMesh->GetStaticMesh() != nullptr)
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

bool UTMOPCharacterAppearanceComponent::ApplyPart(
    USkeletalMeshComponent* Component, const FTMOPResolvedAppearancePart& Part,
    const bool bPreserveMeshWhenMissing)
{
    if (!IsValid(Component)) return false;
    if (Part.bIntentionallyEmpty)
    {
        Component->SetVisibility(false, true);
        return false;
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
            return false;
        }
        Component->SetSkeletalMesh(Mesh);
        if (IsValid(Agent) && Component != Agent->BodyMesh)
            Component->SetLeaderPoseComponent(Agent->BodyMesh);
    }
    else if (!Part.Mesh.IsNull())
    {
        const FString Message = FString::Printf(
            TEXT("Asset '%s' could not load Skeletal Mesh '%s'."),
            *Part.CatalogId.ToString(), *Part.Mesh.ToSoftObjectPath().ToString());
        ResolvedAppearance.Diagnostics.Add(Message);
        UE_LOG(LogTemp, Error, TEXT("TMOP appearance: %s"), *Message);
        Component->SetVisibility(false, true);
        return false;
    }
    else if (!bPreserveMeshWhenMissing)
    {
        Component->SetVisibility(false, true);
        return false;
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
    return Component->GetSkeletalMeshAsset() != nullptr;
}

bool UTMOPCharacterAppearanceComponent::ApplyHeadwear(
    ATMOPHistoricalAgent* Agent, const FTMOPResolvedAppearancePart& Part)
{
    if (!IsValid(Agent) || !IsValid(Agent->BodyMesh)) return false;

    // A migrated static hat always wins. Keep the old component empty so it
    // contributes no leader-pose, skinning or morph evaluation cost.
    if (IsValid(Agent->HeadwearMesh))
    {
        Agent->HeadwearMesh->SetSkeletalMesh(nullptr);
        Agent->HeadwearMesh->SetVisibility(false, true);
    }
    if (Part.bIntentionallyEmpty)
    {
        if (IsValid(Agent->HeadwearStaticMesh))
        {
            Agent->HeadwearStaticMesh->SetStaticMesh(nullptr);
            Agent->HeadwearStaticMesh->SetVisibility(false, true);
        }
        return true;
    }

    UStaticMesh* Mesh = Part.StaticMesh.LoadSynchronous();
    if (Mesh == nullptr)
    {
        if (IsValid(Agent->HeadwearStaticMesh))
        {
            Agent->HeadwearStaticMesh->SetStaticMesh(nullptr);
            Agent->HeadwearStaticMesh->SetVisibility(false, true);
        }
        if (!Part.StaticMesh.IsNull())
        {
            const FString Message = FString::Printf(TEXT(
                "Headwear '%s' could not load Static Mesh '%s'."),
                *Part.CatalogId.ToString(),
                *Part.StaticMesh.ToSoftObjectPath().ToString());
            ResolvedAppearance.Diagnostics.Add(Message);
            UE_LOG(LogTemp, Error, TEXT("TMOP appearance: %s"), *Message);
            return false;
        }

        // Temporary migration fallback. Old catalog rows continue to render,
        // but the heavier skeletal component is created only for those agents.
        if (!Part.Mesh.IsNull())
        {
            ResolvedAppearance.Diagnostics.Add(FString::Printf(TEXT(
                "Headwear '%s' still uses legacy Skeletal Mesh; migrate it to StaticMesh."),
                *Part.CatalogId.ToString()));
            if (!IsValid(Agent->HeadwearMesh))
            {
                Agent->HeadwearMesh = NewObject<USkeletalMeshComponent>(
                    Agent, FName(TEXT("HeadwearMesh_Legacy")));
                if (!IsValid(Agent->HeadwearMesh)) return false;
                Agent->AddInstanceComponent(Agent->HeadwearMesh);
                Agent->HeadwearMesh->SetupAttachment(Agent->BodyMesh);
                Agent->HeadwearMesh->SetCollisionEnabled(
                    ECollisionEnabled::NoCollision);
                Agent->HeadwearMesh->SetGenerateOverlapEvents(false);
                Agent->HeadwearMesh->RegisterComponent();
            }
            return ApplyPart(Agent->HeadwearMesh, Part, false);
        }
        return false;
    }

    if (!IsValid(Agent->HeadwearStaticMesh))
    {
        Agent->HeadwearStaticMesh = NewObject<UStaticMeshComponent>(
            Agent, FName(TEXT("HeadwearStaticMesh")));
        if (!IsValid(Agent->HeadwearStaticMesh)) return false;
        Agent->AddInstanceComponent(Agent->HeadwearStaticMesh);
        Agent->HeadwearStaticMesh->SetupAttachment(Agent->BodyMesh);
        Agent->HeadwearStaticMesh->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
        Agent->HeadwearStaticMesh->SetGenerateOverlapEvents(false);
        Agent->HeadwearStaticMesh->RegisterComponent();
    }
    UStaticMeshComponent* Component = Agent->HeadwearStaticMesh;
    Component->SetStaticMesh(Mesh);
    FName Socket = Part.AttachmentSocket.IsNone()
        ? DefaultHeadwearSocket : Part.AttachmentSocket;
    if (!Agent->BodyMesh->DoesSocketExist(Socket))
    {
        const FName RequestedSocket = Socket;
        Socket = Agent->BodyMesh->DoesSocketExist(HeadwearFallbackBone)
            ? HeadwearFallbackBone : NAME_None;
        ResolvedAppearance.Diagnostics.Add(FString::Printf(TEXT(
            "Headwear socket '%s' is missing; using '%s'."),
            *RequestedSocket.ToString(),
            Socket.IsNone() ? TEXT("component root") : *Socket.ToString()));
    }
    Component->AttachToComponent(Agent->BodyMesh,
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
            Agent->GlovesMesh.Get() };
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
    if (bHybridHeadActive && IsValid(HybridSource))
    {
        HybridSource->VisibilityBasedAnimTickOption =
            EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
        HybridSource->bEnableUpdateRateOptimizations = false;
    }
    if (IsValid(Agent->HeadwearStaticMesh))
    {
        Agent->HeadwearStaticMesh->SetCullDistance(CullDistanceCentimeters);
        Agent->HeadwearStaticMesh->bAllowCullDistanceVolume = true;
    }
}

void UTMOPCharacterAppearanceComponent::ResetAppearance()
{
    ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    if (!IsValid(Agent)) return;
    ClearHybridHead();
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
    if (IsValid(Agent->HeadwearStaticMesh))
    {
        Agent->HeadwearStaticMesh->SetStaticMesh(nullptr);
        Agent->HeadwearStaticMesh->SetVisibility(false, true);
    }
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


void UTMOPCharacterAppearanceComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    ClearHybridHead();
    Super::EndPlay(Reason);
}

void UTMOPCharacterAppearanceComponent::ClearHybridHead()
{
    bHybridHeadActive = false;
    if (IsValid(HybridPresentation))
    {
        HybridPresentation->DestroyChildActor();
        HybridPresentation->DestroyComponent();
    }
    HybridPresentation = nullptr;
    if (IsValid(HybridSource))
    {
        HybridSource->VisibilityBasedAnimTickOption =
            static_cast<EVisibilityBasedAnimTickOption>(SavedHybridSourceTickPolicy);
        HybridSource->bEnableUpdateRateOptimizations = bSavedHybridSourceURO;
    }
    HybridSource = nullptr;
}

void UTMOPCharacterAppearanceComponent::ApplyHybridHead(ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || !IsValid(Agent->PersonProfile)) return;
    const FTMOPAppearanceProfile& P = Agent->PersonProfile->Profile.AppearanceProfile;
    if (!P.bUseMetaHumanHybridHead) return;
    auto Fail = [this](const FString& Message)
    {
        ClearHybridHead();
        const FString Diagnostic = TEXT("MetaHuman hybrid: ") + Message;
        ResolvedAppearance.Diagnostics.Add(Diagnostic);
        UE_LOG(LogTemp, Warning, TEXT("%s"), *Diagnostic);
    };
    if (!IsValid(Agent->BodyMesh) || !Agent->BodyMesh->GetSkeletalMeshAsset())
    {
        Fail(TEXT("No TMOP source body; using normal appearance."));
        return;
    }
    UClass* PresentationClass = P.MetaHumanPresentationClass.LoadSynchronous();
    UClass* RetargetClass = P.MetaHumanBodyRetargetAnimClass.LoadSynchronous();
    if (!PresentationClass || PresentationClass->IsChildOf(APawn::StaticClass()) ||
        PresentationClass->HasAnyClassFlags(CLASS_Abstract) || !RetargetClass ||
        RetargetClass->HasAnyClassFlags(CLASS_Abstract))
    {
        Fail(TEXT("Select an assembled Actor Blueprint (not Pawn) and a target-body retarget AnimBP."));
        return;
    }
    // Do not hide the ordinary head when the configured skin cannot mask it.
    if (Agent->BodyMesh->GetNumMaterials() == 0)
    {
        Fail(TEXT("TMOP body has no materials."));
        return;
    }
    for (int32 Index = 0; Index < Agent->BodyMesh->GetNumMaterials(); ++Index)
    {
        UMaterialInterface* Material = Agent->BodyMesh->GetMaterial(Index);
        float Value = 0.0f;
        if (!Material || !Material->GetScalarParameterValue(
            FMaterialParameterInfo(TEXT("TMOP_HideHead")), Value))
        {
            Fail(TEXT("Each TMOP body material must support R22 TMOP_HideHead. Check region UV/mask wiring too."));
            return;
        }
    }
    HybridPresentation = NewObject<UChildActorComponent>(Agent);
    Agent->AddInstanceComponent(HybridPresentation);
    HybridPresentation->SetupAttachment(Agent->BodyMesh);
    HybridPresentation->SetChildActorClass(PresentationClass);
    HybridPresentation->RegisterComponent();
    AActor* Presentation = HybridPresentation->GetChildActor();
    if (!IsValid(Presentation))
    {
        Fail(TEXT("Could not create presentation actor."));
        return;
    }
    Presentation->SetActorEnableCollision(false);
    TArray<USkeletalMeshComponent*> Meshes;
    Presentation->GetComponents<USkeletalMeshComponent>(Meshes);
    USkeletalMeshComponent* Bridge = nullptr;
    USkeletalMeshComponent* Face = nullptr;
    for (USkeletalMeshComponent* Mesh : Meshes)
    {
        if (Mesh->GetFName() == P.MetaHumanBodyComponentName) Bridge = Mesh;
        if (Mesh->GetFName() == P.MetaHumanFaceComponentName) Face = Mesh;
    }
    if (!Bridge || !Face || Bridge == Face || !Bridge->GetSkeletalMeshAsset() ||
        !Face->GetSkeletalMeshAsset() || !Face->IsAttachedTo(Bridge))
    {
        Fail(TEXT("Body/Face component names, meshes or Face-under-Body hierarchy are invalid."));
        return;
    }
    // Keep the original face hierarchy, DNA, post-process graph, bindings and LODSync.
    // The invisible MH body is the animation bridge; no socket applies head motion twice.
    Bridge->AttachToComponent(Agent->BodyMesh, FAttachmentTransformRules::KeepRelativeTransform);
    Bridge->SetRelativeTransform(P.MetaHumanBodyAlignment);
    Bridge->SetLeaderPoseComponent(nullptr);
    Bridge->SetAnimInstanceClass(RetargetClass);
    if (!Bridge->GetAnimInstance())
    {
        Fail(TEXT("Retarget AnimBP could not initialize on the MetaHuman body."));
        return;
    }
    Bridge->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Bridge->bEnableUpdateRateOptimizations = false;
    Bridge->AddTickPrerequisiteComponent(Agent->BodyMesh);
    Face->AddTickPrerequisiteComponent(Bridge);
    TArray<UPrimitiveComponent*> Primitives;
    Presentation->GetComponents<UPrimitiveComponent>(Primitives);
    for (UPrimitiveComponent* Part : Primitives)
    {
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        if (Part != Face && !Part->IsAttachedTo(Face))
        {
            Part->SetVisibility(false, false);
            Part->SetHiddenInGame(true, false);
        }
    }
    Face->SetVisibility(true, false);
    Face->SetHiddenInGame(false, false);
    Face->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    Face->bEnableUpdateRateOptimizations = false;
    HybridSource = Agent->BodyMesh;
    SavedHybridSourceTickPolicy = static_cast<uint8>(HybridSource->VisibilityBasedAnimTickOption);
    bSavedHybridSourceURO = HybridSource->bEnableUpdateRateOptimizations;
    HybridSource->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
    HybridSource->bEnableUpdateRateOptimizations = false;
    for (USkeletalMeshComponent* Part : { Agent->FaceMesh.Get(), Agent->HairMesh.Get(),
        Agent->FacialHairMesh.Get() })
        if (IsValid(Part)) Part->SetVisibility(false, true);
    bHybridHeadActive = true;
    ResolvedAppearance.Diagnostics.Add(TEXT("MetaHuman hybrid active. Verify neck seam, retargeting and groom LODs in play."));
}
