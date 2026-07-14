#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "TMOPConfiguredVehicle.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UTMOPVehicleModelData;
class UTMOPVehicleSeatComponent;
class UTMOPVehicleDoorComponent;

/** Select one vehicle model; all visual parts are assembled automatically. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPConfiguredVehicle : public ATMOPVehicleBase
{
    GENERATED_BODY()

public:
    ATMOPConfiguredVehicle();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle")
    TObjectPtr<UTMOPVehicleModelData> VehicleModel;

    /** Converts side-facing imported meshes to Unreal's X-forward convention. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Orientation")
    float VisualYawCorrectionDegrees = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle")
    TObjectPtr<USceneComponent> VisualRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle")
    TObjectPtr<UStaticMeshComponent> BodyMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelFrontLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelFrontRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelRearLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle")
    TObjectPtr<UStaticMeshComponent> WheelRearRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Camera")
    TObjectPtr<USpringArmComponent> VehicleCameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Camera")
    TObjectPtr<UCameraComponent> VehicleCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Seats")
    TObjectPtr<UTMOPVehicleSeatComponent> SeatDriver;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Seats")
    TObjectPtr<UTMOPVehicleSeatComponent> SeatFrontPassenger;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Seats")
    TObjectPtr<UTMOPVehicleSeatComponent> SeatRearLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Seats")
    TObjectPtr<UTMOPVehicleSeatComponent> SeatRearCenter;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Seats")
    TObjectPtr<UTMOPVehicleSeatComponent> SeatRearRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Doors")
    TObjectPtr<UTMOPVehicleDoorComponent> DoorFrontLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Doors")
    TObjectPtr<UTMOPVehicleDoorComponent> DoorFrontRight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Doors")
    TObjectPtr<UTMOPVehicleDoorComponent> DoorRearLeft;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Doors")
    TObjectPtr<UTMOPVehicleDoorComponent> DoorRearRight;

    UFUNCTION(BlueprintCallable, Category="TMOP|Configured Vehicle")
    bool ApplyConfiguration();

private:
    void ApplyWheel(UStaticMeshComponent* Component, const FTransform& LocalTransform);
    void UpdateWheelAnimation(float DeltaSeconds);
    float AccumulatedWheelRollDegrees = 0.0f;
};
