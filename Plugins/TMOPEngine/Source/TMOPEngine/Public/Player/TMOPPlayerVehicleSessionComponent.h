#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/TMOPVehicleTakeoverComponent.h"
#include "TMOPPlayerVehicleSessionComponent.generated.h"

class ATMOPVehicleBase;
class UTMOPPlayerVehicleDrivingComponent;
class ACameraActor;
class USpringArmComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTMOPVehicleSessionSignature,
    ATMOPVehicleBase*, Vehicle, bool, bDriver);

/** Coordinates entering, driving, camera view and exiting while the player Pawn remains possessed. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerVehicleSessionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerVehicleSessionComponent();
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Session|Camera")
    float EnterCameraBlendSeconds = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Session|Camera")
    float ExitCameraBlendSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Session|Camera")
    bool bUseVehicleAsCameraTarget = true;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Vehicle Session|Events")
    FTMOPVehicleSessionSignature OnVehicleSessionStarted;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Vehicle Session|Events")
    FTMOPVehicleSessionSignature OnVehicleSessionEnded;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session")
    ETMOPVehicleTakeoverResult EnterNearestVehicle(bool bPreferDriverSeat = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session")
    ETMOPVehicleTakeoverResult EnterVehicle(ATMOPVehicleBase* Vehicle,
        bool bPreferDriverSeat = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session")
    bool ExitVehicle();

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Session")
    bool IsInVehicle() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Session")
    bool IsDrivingVehicle() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session|Input")
    void VehicleThrottle(float Value);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session|Input")
    void VehicleSteering(float Value);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session|Input")
    void VehicleBrake(float Value);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session|Input")
    void VehicleHandbrake(bool bPressed);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Session|Input")
    void VehicleHighSpeedMode(bool bEnabled);

private:
    UPROPERTY(Transient) TObjectPtr<ACameraActor> LocalVehicleCamera;
    UPROPERTY(Transient) TObjectPtr<USpringArmComponent> LocalVehicleBoom;
    FRotator LastLocalControlRotation = FRotator::ZeroRotator;
    float SecondsSinceLocalCameraInput = 0.0f;
    void UpdateLocalVehicleCamera(float DeltaTime);
    UTMOPVehicleTakeoverComponent* GetTakeover() const;
    UTMOPPlayerVehicleDrivingComponent* GetDriving() const;
    void SetCameraTarget(AActor* Target, float BlendSeconds);
};
