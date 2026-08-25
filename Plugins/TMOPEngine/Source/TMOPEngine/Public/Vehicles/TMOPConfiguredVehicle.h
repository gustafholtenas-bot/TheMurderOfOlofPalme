#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "TMOPConfiguredVehicle.generated.h"

class UStaticMeshComponent;
class UStaticMesh;
class USceneComponent;
class USpringArmComponent;
class UCameraComponent;
class UTMOPVehicleModelData;
class UTMOPVehicleSeatComponent;
class UTMOPVehicleDoorComponent;
class UTMOPTrafficVehicleMovementComponent;
class UMaterialInstanceDynamic;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Configured Vehicle|Appearance")
    bool bOverrideBodyColor = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Configured Vehicle|Appearance",
        meta=(EditCondition="bOverrideBodyColor"))
    FLinearColor BodyColor = FLinearColor::White;

    /** Converts side-facing imported meshes to Unreal's X-forward convention. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Orientation")
    float VisualYawCorrectionDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Configured Vehicle|Collision",
        meta=(ClampMin="10.0"))
    float CollisionHalfHeightCm = 60.0f;

    /**
     * If the selected model has no explicit vertical Body Local Transform,
     * lift its bounds so the bottom rests on the road-level Visual Root.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Configured Vehicle|Placement")
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

    /** Disabled while parked; BeginDriving in a person's timeline starts it. */
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
    bool ApplyBodyColor();
    int32 ResolveBodyMaterialSlotIndex() const;
    FName ResolveBodyColorParameterName(
        class UMaterialInterface* Material) const;
    void ApplyWheel(UStaticMeshComponent* Component, UStaticMesh* WheelMesh,
        const FTransform& LocalTransform);
    void UpdateWheelAnimation(float DeltaSeconds);
    void InitializeVehicleLightMaterials();
    void UpdateVehicleLights(bool bForce = false);
    void ApplyLightMaterialState(int32 MaterialIndex, bool bEnabled);
    float AccumulatedWheelRollDegrees = 0.0f;
    float DisplayedWheelSteeringDegrees = 0.0f;
    TMap<int32, TWeakObjectPtr<UMaterialInstanceDynamic>> VehicleLightMaterials;
    TMap<int32, FLinearColor> VehicleLightColors;
    TSet<int32> EmergencyBlueMaterialIndices;
    bool bLastDrivingLightsEnabled = false;
    bool bLastEmergencyBlueEnabled = false;
    bool bVehicleLightsInitialized = false;
};
