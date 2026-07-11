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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.0"))
    float MaximumSteeringDegrees = 32.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.0"))
    float SteeringResponse = 6.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Steering",
        meta=(ClampMin="0.0"))
    float MinimumTurnRadiusCm = 520.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Wheels",
        meta=(ClampMin="1.0"))
    float VisualWheelRadiusCm = 34.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player Driving|Collision")
    bool bSweepMovement = true;

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

    UFUNCTION(BlueprintPure, Category="TMOP|Player Driving")
    bool IsDriving() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Player Driving")
    float GetCurrentSpeedKmh() const;

private:
    float ThrottleInput = 0.0f;
    float SteeringInput = 0.0f;
    float BrakeInput = 0.0f;
    bool bHandbrakeInput = false;
    TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent> SuspendedTrafficMovement;
};
