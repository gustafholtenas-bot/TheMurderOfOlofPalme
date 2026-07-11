#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPTrafficVehicleSubsystem.generated.h"

class UTMOPTrafficVehicleMovementComponent;

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPTrafficVehicleSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    bool RegisterVehicle(UTMOPTrafficVehicleMovementComponent* Vehicle);
    bool UnregisterVehicle(UTMOPTrafficVehicleMovementComponent* Vehicle);

    /** Returns the closest vehicle ahead in the same directed lane. */
    UTMOPTrafficVehicleMovementComponent* FindLeadVehicle(
        const UTMOPTrafficVehicleMovementComponent* Vehicle,
        float& OutCenterDistance) const;

    /** Signed distances from a reference point to the nearest centers in a target lane. */
    void FindNearestVehiclesInLane(FName LaneId, float ReferenceDistance,
        const UTMOPTrafficVehicleMovementComponent* IgnoredVehicle,
        float& OutAheadDistance, float& OutBehindDistance) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    int32 GetActiveTrafficVehicleCount() const;

private:
    TArray<TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent>> Vehicles;
};
