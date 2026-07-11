#include "Vehicles/TMOPVehicleDoorComponent.h"

UTMOPVehicleDoorComponent::UTMOPVehicleDoorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

FTransform UTMOPVehicleDoorComponent::GetApproachWorldTransform() const
{
    return FTransform(FRotator::ZeroRotator, ApproachLocalOffset, FVector::OneVector) *
        GetComponentTransform();
}
