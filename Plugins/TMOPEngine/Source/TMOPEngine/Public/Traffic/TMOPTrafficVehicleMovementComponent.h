#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPTrafficVehicleMovementComponent.generated.h"

class UTMOPTrafficLaneComponent;

UENUM(BlueprintType)
enum class ETMOPTrafficVehicleState : uint8
{
    Uninitialized,
    Driving,
    FollowingVehicle,
    ChangingLane,
    BrakingForConstraint,
    Stopped,
    RouteComplete,
    InvalidLane
};

/** Deterministic lane-following movement shared by cars and buses. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPTrafficVehicleMovementComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPTrafficVehicleMovementComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Route")
    FName InitialLaneId = NAME_None;

    /** Optional explicit route. First item should normally equal InitialLaneId. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Route")
    TArray<FName> PlannedLaneIds;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Runtime")
    FName CurrentLaneId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Runtime")
    float DistanceAlongLane = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Runtime")
    float CurrentSpeedCmPerSecond = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Runtime")
    ETMOPTrafficVehicleState TrafficState = ETMOPTrafficVehicleState::Uninitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Driving",
        meta=(ClampMin="0.1"))
    float SpeedLimitMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Driving",
        meta=(ClampMin="1.0"))
    float AccelerationCmPerSecondSquared = 220.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Driving",
        meta=(ClampMin="1.0"))
    float ServiceBrakeCmPerSecondSquared = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Following",
        meta=(ClampMin="0.0"))
    float MinimumGapCm = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Following",
        meta=(ClampMin="0.1"))
    float DesiredTimeHeadwaySeconds = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Dimensions",
        meta=(ClampMin="50.0"))
    float VehicleLengthCm = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Detection")
    bool bDetectPhysicalObstacles = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Detection",
        meta=(ClampMin="100.0"))
    float MinimumObstacleLookAheadCm = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Detection",
        meta=(ClampMin="200.0"))
    float MaximumObstacleLookAheadCm = 3500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Detection",
        meta=(ClampMin="10.0"))
    float ObstacleSensorHalfWidthCm = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Detection",
        meta=(ClampMin="10.0"))
    float ObstacleSensorHalfHeightCm = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Placement")
    FVector VehicleLocalOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Placement")
    FRotator VehicleRotationOffset = FRotator::ZeroRotator;

    /** Runtime local-right offset used for bus-bay pull-in and similar manoeuvres. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Placement")
    float AdditionalLateralOffsetCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic")
    bool bStartDrivingAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change")
    bool bAllowLaneChanges = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change")
    bool bKeepRightWhenPossible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change",
        meta=(ClampMin="0.1"))
    float LaneChangeDurationSeconds = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change",
        meta=(ClampMin="0.1"))
    float LaneChangeCheckIntervalSeconds = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change",
        meta=(ClampMin="0.0"))
    float MinimumTargetLaneFrontGapCm = 700.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change",
        meta=(ClampMin="0.0"))
    float MinimumTargetLaneRearGapCm = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Lane Change",
        meta=(ClampMin="0.0"))
    float MinimumLaneChangeDistanceFromLaneEndCm = 3000.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool InitializeOnLane(FName LaneId, float StartDistance = 0.0f);

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    void StartDriving();

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    void StopDriving();

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic|Placement")
    void SetAdditionalLateralOffset(float OffsetCm);

    /** Used by future signals/stops. Negative clears the constraint. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    void SetExternalStopDistance(float StopDistanceAlongCurrentLane);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    bool HasExternalStopConstraint() const { return StopConstraints.Num() > 0; }

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    void SetNamedStopConstraint(FName SourceId, float StopDistanceAlongCurrentLane);

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    void ClearNamedStopConstraint(FName SourceId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    void ClearAllStopConstraints();

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    float GetNearestActiveStopDistance() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    float GetCurrentSpeedKmh() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    UTMOPTrafficLaneComponent* GetCurrentLane() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic|Lane Change")
    bool RequestLaneChange(FName TargetLaneId);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic|Lane Change")
    bool IsChangingLane() const { return !TargetLaneId.IsNone(); }

private:
    float CalculateTargetSpeed(UTMOPTrafficLaneComponent* Lane);
    bool AdvanceToNextLane(UTMOPTrafficLaneComponent* CurrentLane);
    FName ChooseNextLaneId(const UTMOPTrafficLaneComponent* CurrentLane) const;
    void ApplyVehicleTransform(UTMOPTrafficLaneComponent* Lane);
    void EvaluateAutomaticLaneChange(UTMOPTrafficLaneComponent* Lane);
    bool IsTargetLaneSafe(UTMOPTrafficLaneComponent* TargetLane, float TargetDistance) const;
    void UpdateLaneChange(float DeltaTime, UTMOPTrafficLaneComponent* SourceLane);
    float GetPhysicalObstacleDistance() const;

    int32 PlannedLaneIndex = INDEX_NONE;
    TMap<FName, float> StopConstraints;
    bool bDrivingEnabled = false;
    FName TargetLaneId = NAME_None;
    float TargetLaneDistance = 0.0f;
    float LaneChangeElapsedSeconds = 0.0f;
    float LaneChangeCheckAccumulator = 0.0f;
    float LaneChangeCooldownSeconds = 0.0f;
};
