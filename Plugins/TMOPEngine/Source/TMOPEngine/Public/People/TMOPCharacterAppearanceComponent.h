#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "TMOPCharacterAppearanceComponent.generated.h"

class ATMOPHistoricalAgent;
class UDataTable;
class UMaterialInterface;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|MetaHuman")
    bool bPreserveMetaHumanBodyPlacement = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|MetaHuman")
    bool bApplyPerformanceSettingsToAdditionalSkeletalMeshes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Performance",
        meta=(ClampMin="1000.0", Units="cm"))
    float CullDistanceCentimeters = 15000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Performance")
    bool bEnableAnimationUpdateRateOptimizations = true;

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
    void ApplyBodyAndProportions(ATMOPHistoricalAgent* Agent);
    void ApplyPart(USkeletalMeshComponent* Component,
        const FTMOPResolvedAppearancePart& Part, bool bPreserveMeshWhenMissing);
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
