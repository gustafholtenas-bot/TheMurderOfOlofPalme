#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPVehicleGroundAudit.generated.h"

class ATMOPVehicleBase;
class ATMOPHistoricalAnchor;

/** Read-only editor audit using the actual vehicle's support footprint. Run once per vehicle size. */
UCLASS()
class TMOPENGINE_API ATMOPVehicleGroundAudit : public AActor
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="Ground audit") TObjectPtr<ATMOPVehicleBase> ReferenceVehicle;
    /** Only vehicle parking/spawn anchors belong here; pedestrian anchors are intentionally excluded. */
    UPROPERTY(EditAnywhere, Category="Ground audit") TArray<TObjectPtr<ATMOPHistoricalAnchor>> ParkingAnchors;
    UPROPERTY(EditAnywhere, Category="Ground audit") FTransform AnchorLocalOffset;
    UPROPERTY(EditAnywhere, Category="Ground audit") bool bCheckAllLanes = true;
    UPROPERTY(EditAnywhere, Category="Ground audit", meta=(ClampMin="25")) float SampleSpacingCm = 250;
    UPROPERTY(EditAnywhere, Category="Ground audit", meta=(ClampMin="0")) float WarnCorrectionCm = 40;
    UPROPERTY(VisibleAnywhere, Category="Ground audit") int32 CheckedSamples = 0;
    UPROPERTY(VisibleAnywhere, Category="Ground audit") int32 FailedSamples = 0;
    UPROPERTY(VisibleAnywhere, Category="Ground audit") TArray<FString> Findings;
    UFUNCTION(CallInEditor, Category="Ground audit") void ValidateLanesAndAnchors();
};
