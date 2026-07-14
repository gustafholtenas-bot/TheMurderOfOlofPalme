#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "TMOPConfiguredVehicle.generated.h"

class UStaticMeshComponent;
class UTMOPVehicleModelData;

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

    UFUNCTION(BlueprintCallable, Category="TMOP|Configured Vehicle")
    bool ApplyConfiguration();

private:
    void ApplyWheel(UStaticMeshComponent* Component, const FTransform& LocalTransform);
    void UpdateWheelAnimation(float DeltaSeconds);
    float AccumulatedWheelRollDegrees = 0.0f;
};
