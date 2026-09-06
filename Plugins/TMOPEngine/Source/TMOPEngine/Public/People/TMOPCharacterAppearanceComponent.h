#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "TMOPCharacterAppearanceComponent.generated.h"

class UChildActorComponent;
class ATMOPHistoricalAgent;
class UDataTable;
class UMaterialInterface;
class USkeletalMesh;
class USkeletalMeshComponent;

/** Builds one historical agent from their evidence-backed appearance profile. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPCharacterAppearanceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPCharacterAppearanceComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Optional per-agent catalog. Empty uses the registry's central table. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance")
    TObjectPtr<UDataTable> AssetCatalogOverride;

    /** Shared anonymous material used until an obscured catalog asset supplies one. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Unknown")
    TSoftObjectPtr<UMaterialInterface> ObscuredMaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Body",
        meta=(ClampMin="100.0", Units="cm"))
    float BodyReferenceHeightCentimeters = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Body")
    bool bAdjustMovementSpeedForHeight = true;

    /** Preferred socket shared by Manny and Quinn for rigid hats. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Headwear")
    FName DefaultHeadwearSocket = TEXT("HeadwearSocket");

    /** Used while a Skeleton asset has not yet received DefaultHeadwearSocket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Headwear")
    FName HeadwearFallbackBone = TEXT("head");

    /** Automatically uses Manny for male profiles and Quinn for female profiles.
     *  Unknown/unspecified genders keep the actor's existing body. Explicit
     *  catalog body overrides still take precedence outside pilot mode. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Body")
    bool bAutomaticallySelectMannyOrQuinnByGender = true;

    /** Technical male base body. Defaults to the UE5 Manny Simple asset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Body",
        meta=(EditCondition="bAutomaticallySelectMannyOrQuinnByGender"))
    TSoftObjectPtr<USkeletalMesh> MaleBaseBodyMesh;

    /** Technical female base body. Defaults to the UE5 Quinn Simple asset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Body",
        meta=(EditCondition="bAutomaticallySelectMannyOrQuinnByGender"))
    TSoftObjectPtr<USkeletalMesh> FemaleBaseBodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|MetaHuman")
    bool bPreserveMetaHumanBodyPlacement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|MetaHuman")
    bool bApplyPerformanceSettingsToAdditionalSkeletalMeshes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Performance",
        meta=(ClampMin="1000.0", Units="cm"))
    float CullDistanceCentimeters = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Performance")
    bool bEnableAnimationUpdateRateOptimizations = true;

    /**
     * First integration step for the modular wardrobe.  While enabled, the
     * component only applies Outerwear and leaves the actor's existing body,
     * face, hair and other clothing untouched.  This makes it possible to
     * validate one rigged jacket on every historical/observed NPC before the
     * complete wardrobe is enabled.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Pilot")
    bool bOuterwearOnlyPilotMode = false;

    /** Force a fallback jacket even when the source profile says None/Hidden.
     *  This is a runtime-only visual test and never changes DT_TMOP_People. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Pilot",
        meta=(EditCondition="bOuterwearOnlyPilotMode"))
    bool bForceOuterwearOnEveryoneInPilotMode = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearance ResolvedAppearance;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    bool bHasAppliedAppearance = false;

    UFUNCTION(BlueprintCallable, Category="TMOP|Appearance")
    bool ApplyAppearance();

    UFUNCTION(BlueprintCallable, Category="TMOP|Appearance")
    void ResetAppearance();

    UFUNCTION(BlueprintCallable, Category="TMOP|Appearance")
    bool ValidateAppearance(TArray<FString>& OutWarnings) const;

private:
    void ApplyHybridHead(ATMOPHistoricalAgent* Agent);
    void ClearHybridHead();

    UPROPERTY(Transient)
    TObjectPtr<UChildActorComponent> HybridPresentation;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> HybridSource;

    bool bHybridHeadActive = false;
    uint8 SavedHybridSourceTickPolicy = 0;
    bool bSavedHybridSourceURO = false;

    bool ApplyAutomaticGenderBody(ATMOPHistoricalAgent* Agent,
        ETMOPPersonGender Gender);
    void ApplyBodyAndProportions(ATMOPHistoricalAgent* Agent);
    bool ApplyPart(USkeletalMeshComponent* Component,
        const FTMOPResolvedAppearancePart& Part, bool bPreserveMeshWhenMissing);
    bool ApplyHeadwear(ATMOPHistoricalAgent* Agent,
        const FTMOPResolvedAppearancePart& Part);
    void ApplyCollisionAndPresentation(ATMOPHistoricalAgent* Agent,
        bool bPreserveBespokeBodyPlacement);
    void ApplyPerformanceSettings(ATMOPHistoricalAgent* Agent);
    void ApplyMorphs(USkeletalMeshComponent* Body,
        const FTMOPAppearanceProfile& Profile, ETMOPBodyBuild BodyBuild);
    void ApplyModularMorphs(ATMOPHistoricalAgent* Agent,
        const FTMOPAppearanceProfile& Profile, ETMOPBodyBuild BodyBuild,
        bool bIncludeBespokeHeadParts);
    void ApplyBodyRegionMask(ATMOPHistoricalAgent* Agent);
    void ApplyFaceMorphs(USkeletalMeshComponent* Face,
        const FTMOPResolvedFaceMorphs& Morphs);
    UDataTable* ResolveAssetCatalog() const;
    void CacheBaseBodyTransform(ATMOPHistoricalAgent* Agent);

    bool bBaseBodyTransformCached = false;
    FVector BaseBodyRelativeLocation = FVector::ZeroVector;
    FVector BaseBodyRelativeScale = FVector::OneVector;
};
