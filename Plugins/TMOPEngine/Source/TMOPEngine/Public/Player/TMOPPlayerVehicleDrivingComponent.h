#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPPlayerVehicleDrivingComponent.generated.h"

class ATMOPVehicleBase;
class UTMOPTrafficVehicleMovementComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTMOPPlayerDrivingStateSignature,
    ATMOPVehicleBase*, Vehicle, bool, bPlayerDriving);

/** Free kinematic player driving for TMOP vehicles. Input is supplied by the player Blueprint. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerVehicleDrivingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerVehicleDrivingComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Speed",
        meta=(ClampMin="0.0"))
    float MaximumForwardSpeedKmh = 110.0f;

    /** Normal full-throttle speed. Hold the player's sprint button for MaximumForwardSpeedKmh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Speed",
        meta=(ClampMin="0.0"))
    float NormalForwardSpeedKmh = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Speed",
        meta=(ClampMin="0.0"))
    float MaximumReverseSpeedKmh = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Response",
        meta=(ClampMin="1.0"))
    float EngineAccelerationCmPerSecondSquared = 420.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Response",
        meta=(ClampMin="1.0"))
    float BrakeDecelerationCmPerSecondSquared = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Response",
        meta=(ClampMin="1.0"))
    float CoastingDecelerationCmPerSecondSquared = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Response",
        meta=(ClampMin="1.0"))
    float HandbrakeDecelerationCmPerSecondSquared = 1500.0f;

    /** Above this speed, opposite throttle brakes before reverse engages. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Response",
        meta=(ClampMin="0.0"))
    float DirectionChangeSpeedThresholdCmPerSecond = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.0"))
    float MaximumSteeringDegrees = 32.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.0"))
    float SteeringResponse = 3.0f;

    /** How quickly the car body reaches the requested yaw rate. Lower values feel heavier. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.1"))
    float YawInertiaResponse = 2.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.0"))
    float MinimumTurnRadiusCm = 520.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Wheels",
        meta=(ClampMin="1.0"))
    float VisualWheelRadiusCm = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Collision")
    bool bSweepMovement = true;

    /** Kinematic road contact. Do not enable Simulate Physics on the body mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground")
    bool bFollowGround = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.0"))
    float GroundTraceUpCm = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.0"))
    float GroundTraceDownCm = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground")
    float GroundClearanceCm = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground")
    bool bAlignToGroundNormal = true;

    /** Maximum height treated as a driveable low kerb/step. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.0"))
    float MaximumStepUpHeightCm = 32.0f;

    /** Maximum vertical adjustment speed while climbing a slope or low kerb. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="1.0"))
    float StepUpSpeedCmPerSecond = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.1"))
    float GroundHeightResponse = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.1"))
    float GroundRotationResponse = 5.0f;

    /** Probe placement relative to the vehicle collision box. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.1", ClampMax="1.5"))
    float ProbeExtentScale = 0.85f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.0"))
    float MinimumProbeLongitudinalOffsetCm = 120.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.0"))
    float MinimumProbeLateralOffsetCm = 65.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Ground",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float MinimumGroundNormalZ = 0.55f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Driving|Runtime")
    TObjectPtr<ATMOPVehicleBase> DrivenVehicle;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Driving|Runtime")
    float CurrentSpeedCmPerSecond = 0.0f;

    /** Smoothed front-wheel steering angle for Blueprint wheel meshes. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Driving|Runtime")
    float VisualSteeringAngleDegrees = 0.0f;

    /** Accumulated wheel roll for Blueprint wheel meshes. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Driving|Runtime")
    float VisualWheelRotationDegrees = 0.0f;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Player Driving|Events")
    FTMOPPlayerDrivingStateSignature OnPlayerDrivingStateChanged;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving")
    bool BeginDriving(ATMOPVehicleBase* Vehicle);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving")
    void EndDriving();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving|Input")
    void SetThrottleInput(float Value);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving|Input")
    void SetSteeringInput(float Value);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving|Input")
    void SetBrakeInput(float Value);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving|Input")
    void SetHandbrakeInput(bool bPressed);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Driving|Input")
    void SetHighSpeedMode(bool bEnabled);

    UFUNCTION(BlueprintPure, Category="TMOP|Player Driving")
    bool IsDriving() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Player Driving")
    float GetCurrentSpeedKmh() const;

private:
    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    bool bHandbrakeInput = false;
    bool bHighSpeedMode = false;
    bool bHasGroundContact = false;
    float LastGroundHeightCm = 0.0f;
    float CurrentYawRateDegreesPerSecond = 0.0f;
    float LastCollisionSoundWorldSeconds = -1000.0f;
    TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent> SuspendedTrafficMovement;
};
