#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Player/TMOPVehicleTakeoverComponent.h"
#include "TMOPPlayerVehicleSessionComponent.generated.h"

class ATMOPVehicleBase;
class UTMOPPlayerVehicleDrivingComponent;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Session|Camera")
    float EnterCameraBlendSeconds = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Session|Camera")
    float ExitCameraBlendSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Session|Camera")
    bool bUseVehicleAsCameraTarget = false;

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

private:
    UTMOPVehicleTakeoverComponent* GetTakeover() const;
    UTMOPPlayerVehicleDrivingComponent* GetDriving() const;
    void SetCameraTarget(AActor* Target, float BlendSeconds) const;
};
