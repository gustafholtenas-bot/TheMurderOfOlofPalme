#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPTrafficVehicleMovementComponent.generated.h"

class UTMOPTrafficLaneComponent;
class AActor;

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
    InvalidLane,
    FinalApproach,
    AnchorManeuver
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

    /** Visual front-wheel angle calculated from the current lane curvature. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Runtime")
    float VisualSteeringAngleDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Wheels",
        meta=(ClampMin="1.0", ClampMax="60.0"))
    float MaximumVisualSteeringDegrees = 35.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic|Runtime")
    ETMOPTrafficVehicleState TrafficState = ETMOPTrafficVehicleState::Uninitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Driving",
        meta=(ClampMin="0.1"))
    float SpeedLimitMultiplier = 1.0f;

    /** Zero follows lane speed limits; positive values set this route's cruise speed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Driving",
        meta=(ClampMin="0.0", Units="km/h"))
    float DesiredCruiseSpeedKmh = 0.0f;

    /** Allows explicitly planned connections even when marked restricted. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Rules")
    bool bIgnoreOneWayRestrictions = false;

    /** Signal controllers leave this vehicle unconstrained at red lights. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Rules")
    bool bRunRedLights = false;

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

    /** Ignore pedestrian obstacles and run them down instead of stopping. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Fleeing")
    bool bFleeingVehicle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Horn")
    bool bHonkAtBlockingPawns = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Horn",
        meta=(ClampMin="0.1"))
    float HornAfterBlockedSeconds = 1.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Horn",
        meta=(ClampMin="0.1"))
    float HornCooldownSeconds = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Fleeing",
        meta=(ClampMin="0.0"))
    float FleeingImpactDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Fleeing",
        meta=(ClampMin="0.0"))
    float FleeingImpactLaunchStrength = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Fleeing",
        meta=(ClampMin="0.0"))
    float FleeingImpactUpwardStrength = 350.0f;

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

    /** After this long behind the same stopped actor, perform a controlled bypass. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Bypass",
        meta=(ClampMin="1.0"))
    float ObstacleBypassAfterSeconds = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Bypass",
        meta=(ClampMin="50.0"))
    float ObstacleBypassLateralOffsetCm = 260.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Obstacle Bypass",
        meta=(ClampMin="0.5"))
    float ObstacleBypassDurationSeconds = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Placement")
    FVector VehicleLocalOffset = FVector::ZeroVector;

    /**
     * Destroy the vehicle when it reaches the end of its planned lane route.
     * Historical occupants attached to the vehicle are destroyed with it.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Route")
    bool bDespawnAtRouteEnd = true;

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

    /** Makes route speed follow an absolute simulation arrival deadline. */
    void ConfigureTimedArrival(int32 ExpectedArrivalSecond,
        float MaximumCatchUpSpeedKmh = 90.0f);

    /** Applies the exact final route/anchor transform when its deadline is due. */
    bool ForceCompleteTimedArrival();

    bool HasTimedArrival() const
    {
        return TimedArrivalSecond != INDEX_NONE;
    }

    /**
     * Finish the final lane at the point nearest a destination anchor, then
     * blend the remaining short parking manoeuvre to the anchor transform.
     */
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic|Route")
    void ConfigureFinalApproach(
        FName FinalLaneId,
        float FinalLaneDistanceCm,
        const FTransform& TargetTransform);

    /**
     * Updates a pending parking target without interrupting lane movement.
     * Returns false when this vehicle has no active final approach.
     */
    bool UpdateFinalApproachTarget(const FTransform& TargetTransform);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic|Route")
    bool HasFinalApproach() const { return bHasFinalApproach; }

    /** Starts a deterministic off-lane curve through two or more anchor
     * transforms. Anchor rotations control vehicle orientation and curve
     * tangents, so opposite rotations naturally form a U-turn. */
    bool StartAnchorManeuver(
        const TArray<FTransform>& AnchorTransforms,
        int32 ExpectedArrivalSecond,
        float CurveStrength = 0.5f,
        bool bReverse = false);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic|Route")
    bool IsAnchorManeuverActive() const
    {
        return bAnchorManeuverInProgress;
    }

    /** Approximate speed of the short off-lane parking manoeuvre. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Route",
        meta=(ClampMin="50.0"))
    float FinalApproachSpeedCmPerSecond = 250.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Route",
        meta=(ClampMin="0.1"))
    float MinimumFinalApproachDurationSeconds = 0.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Route",
        meta=(ClampMin="0.1"))
    float MaximumFinalApproachDurationSeconds = 3.0f;

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

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic|World Bake")
    bool IsDrivingEnabled() const { return bDrivingEnabled; }

    /** Restores lane progress and motion after a deterministic world seek. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic|World Bake")
    bool RestoreBakedTrafficState(
        FName LaneId,
        float DistanceCm,
        float SpeedCmPerSecond,
        const TArray<FName>& RouteLaneIds,
        bool bShouldDrive);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    UTMOPTrafficLaneComponent* GetCurrentLane() const;

    /**
     * Non-mutating validation diagnostic for the physical obstacle sensor.
     * Returns the nearest blocking actor and its forward distance.
     */
    bool GetPhysicalObstacleDiagnostics(
        float& OutDistanceCm,
        AActor*& OutBlockingActor) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic|Lane Change")
    bool RequestLaneChange(FName TargetLaneId);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic|Lane Change")
    bool IsChangingLane() const { return !TargetLaneId.IsNone(); }

private:
    float CalculateTargetSpeed(UTMOPTrafficLaneComponent* Lane);
    float CalculateRemainingRouteDistanceCm() const;
    double GetCurrentSimulationSecondExact() const;
    bool AdvanceToNextLane(UTMOPTrafficLaneComponent* CurrentLane);
    FName ChooseNextLaneId(const UTMOPTrafficLaneComponent* CurrentLane) const;
    void ApplyVehicleTransform(UTMOPTrafficLaneComponent* Lane);
    void EvaluateAutomaticLaneChange(UTMOPTrafficLaneComponent* Lane);
    bool IsTargetLaneSafe(UTMOPTrafficLaneComponent* TargetLane, float TargetDistance) const;
    void UpdateLaneChange(float DeltaTime, UTMOPTrafficLaneComponent* SourceLane);
    float GetPhysicalObstacleDistance() const;
    void DespawnAtCompletedRoute();
    void UpdateObstacleBypass(float DeltaTime);
    void BeginObstacleBypass(AActor* BlockingActor);
    void EndObstacleBypass();
    void BeginFinalApproach(UTMOPTrafficLaneComponent* Lane);
    void UpdateFinalApproach(float DeltaTime);
    void ClearFinalApproach();
    void UpdateAnchorManeuver(float DeltaTime);
    void ClearAnchorManeuver();
    FTransform EvaluateAnchorManeuver(float Alpha) const;
    void UpdateVisualSteeringForLane(UTMOPTrafficLaneComponent* Lane);
    void UpdateFleeingVehicleImpacts(float DeltaTime);
    void TryAutomaticHorn(AActor* BlockingActor);

    int32 PlannedLaneIndex = INDEX_NONE;
    TMap<FName, float> StopConstraints;
    bool bDrivingEnabled = false;
    FName TargetLaneId = NAME_None;
    float TargetLaneDistance = 0.0f;
    float LaneChangeElapsedSeconds = 0.0f;
    float LaneChangeCheckAccumulator = 0.0f;
    float LaneChangeCooldownSeconds = 0.0f;
    TWeakObjectPtr<AActor> PersistentBlockingActor;
    float PersistentBlockSeconds = 0.0f;
    float LastHornWorldSeconds = -1000.0f;
    bool bHornPlayedForCurrentBlock = false;
    TWeakObjectPtr<AActor> LastFleeingImpactActor;
    float LastFleeingImpactWorldSeconds = -1000.0f;
    float ObstacleBypassSecondsRemaining = 0.0f;
    float ObstacleBypassBaseLateralOffsetCm = 0.0f;
    bool bObstacleBypassActive = false;
    bool bCollisionWasEnabledBeforeBypass = true;
    bool bHasFinalApproach = false;
    bool bFinalApproachInProgress = false;
    FName FinalApproachLaneId = NAME_None;
    float FinalApproachLaneDistanceCm = 0.0f;
    FTransform FinalApproachStartTransform = FTransform::Identity;
    FTransform FinalApproachTargetTransform = FTransform::Identity;
    float FinalApproachElapsedSeconds = 0.0f;
    float FinalApproachDurationSeconds = 1.0f;
    bool bAnchorManeuverInProgress = false;
    bool bAnchorManeuverReverse = false;
    float AnchorManeuverCurveStrength = 0.5f;
    double AnchorManeuverStartSecond = 0.0;
    float AnchorManeuverElapsedSeconds = 0.0f;
    float AnchorManeuverDurationSeconds = 1.0f;
    float AnchorManeuverApproximateLengthCm = 0.0f;
    TArray<FTransform> AnchorManeuverTransforms;
    TArray<float> AnchorManeuverSegmentWeights;
    int32 TimedArrivalSecond = INDEX_NONE;
    float MaximumTimedCatchUpSpeedCmPerSecond = 2500.0f;
};
