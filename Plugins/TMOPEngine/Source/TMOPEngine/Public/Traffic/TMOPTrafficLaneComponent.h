#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "TMOPTrafficLaneComponent.generated.h"

/** One directed road lane. Spline direction is always the legal travel direction. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPTrafficLaneComponent : public USplineComponent
{
    GENERATED_BODY()

public:
    UTMOPTrafficLaneComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Identity")
    FName LaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Identity")
    FName RoadId = NAME_None;

    /** Same value for lanes travelling in the same direction on this road. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Identity")
    FName DirectionId = NAME_None;

    /** 1 is the rightmost lane in the legal travel direction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Layout",
        meta=(ClampMin="1"))
    int32 LaneIndexFromRight = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Layout",
        meta=(ClampMin="1"))
    int32 LaneCountSameDirection = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Rules")
    bool bRightHandTraffic = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Rules",
        meta=(ClampMin="1.0"))
    float SpeedLimitKmh = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Rules")
    ETMOPTrafficLaneType LaneType = ETMOPTrafficLaneType::General;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Connections")
    TArray<FTMOPLaneConnection> NextLanes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Connections")
    FName LeftNeighborLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Connections")
    FName RightNeighborLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Signals")
    FName StopLineId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Signals")
    FName TrafficSignalGroupId = NAME_None;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    float GetSpeedLimitCentimetersPerSecond() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    FTransform GetLaneTransformAtDistance(float Distance) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    FVector GetLaneLocationAtDistance(float Distance) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    bool AllowsBus() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool ValidateLane(TArray<FString>& OutErrors) const;
};
