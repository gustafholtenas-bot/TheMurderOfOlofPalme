#pragma once

#include "CoreMinimal.h"
#include "Anchors/TMOPAnchorTypes.h"
#include "Entities/TMOPWorldEntity.h"
#include "TMOPHistoricalAnchor.generated.h"

class UBillboardComponent;
class UTextRenderComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPHistoricalAnchor : public ATMOPWorldEntity
{
    GENERATED_BODY()

public:
    ATMOPHistoricalAnchor();

    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    ETMOPAnchorCategory AnchorCategory = ETMOPAnchorCategory::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    FName ParentPlaceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    FName ParentAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    int32 FloorLevel = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    TArray<FTMOPSourceReference> Sources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor")
    FString Notes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Debug")
    bool bShowDebugLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Navigation")
    bool bCanBeUsedForRouting = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Navigation")
    bool bHardHistoricalAnchor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Navigation")
    ETMOPRouteSurfacePreference SurfacePreference =
        ETMOPRouteSurfacePreference::SidewalkPreferred;

    /** Zero keeps point behaviour. Positive values spread arrivals deterministically in a circle. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Area",
        meta=(ClampMin="0.0"))
    float PlacementRadiusCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Area",
        meta=(ClampMin="0.0"))
    float MinimumSpacingCm = 80.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Anchor|Area")
    bool bProjectPlacementToNavMesh = true;

    /** Stable point for an entity/group. The same key always gets the same location. */
    UFUNCTION(BlueprintCallable, Category = "TMOP|Anchor|Area")
    FVector GetPlacementLocation(FName StableKey);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Anchor|Debug")
    TObjectPtr<UBillboardComponent> Billboard;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Anchor|Debug")
    TObjectPtr<UTextRenderComponent> DebugLabel;

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchor")
    FName GetAnchorId() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchor")
    FVector GetAnchorLocation() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Anchor")
    FRotator GetAnchorRotation() const;

private:
    UPROPERTY(Transient)
    TMap<FName, FVector> ReservedPlacementLocations;
};
