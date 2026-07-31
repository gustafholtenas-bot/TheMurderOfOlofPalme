#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "TMOPConfiguredVehicle.generated.h"

class UStaticMeshComponent;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UMaterialInstanceDynamic;
class UTMOPVehicleModelData;
class UTMOPVehicleSeatComponent;
class UTMOPVehicleDoorComponent;
class UTMOPTrafficVehicleMovementComponent;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Appearance")
    bool bOverrideBodyColor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Appearance",
        meta=(EditCondition="bOverrideBodyColor"))
    FLinearColor BodyColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Appearance")
    bool bOverrideWindowTint = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Appearance",
        meta=(EditCondition="bOverrideWindowTint"))
    FLinearColor WindowTint = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Lights")
    bool bHeadlightsOn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Lights")
    bool bOrangeLightsOn = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Lights")
    bool bRedLightsOn = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Configured Vehicle|Lights")
    void SetExteriorLightsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="TMOP|Configured Vehicle|Lights")
    void SetHeadlightsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="TMOP|Configured Vehicle|Lights")
    void SetOrangeLightsEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="TMOP|Configured Vehicle|Lights")
    void SetRedLightsEnabled(bool bEnabled);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Orientation")
    float VisualYawCorrectionDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Collision",
        meta=(ClampMin="10.0"))
    float CollisionHalfHeightCm = 60.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Placement")
    bool bAutoAlignBodyMeshToGround = true;

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Configured Vehicle|Traffic")
    TObjectPtr<UTMOPTrafficVehicleMovementComponent> TrafficMovement;

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
    int32 FindMaterialSlot(FName SlotName) const;
    UMaterialInstanceDynamic* CreateMaterialInstanceForSlot(FName SlotName);
    void ApplyAppearanceMaterials();
    void ApplyLightMaterial(FName SlotName, bool bEnabled);
    void ApplyWheel(UStaticMeshComponent* Component, const FTransform& LocalTransform);
    void UpdateWheelAnimation(float DeltaSeconds);
    float AccumulatedWheelRollDegrees = 0.0f;
};
