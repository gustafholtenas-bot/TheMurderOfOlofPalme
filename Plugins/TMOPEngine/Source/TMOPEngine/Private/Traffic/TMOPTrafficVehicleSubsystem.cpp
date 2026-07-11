#include "Traffic/TMOPTrafficVehicleSubsystem.h"

#include "Traffic/TMOPTrafficVehicleMovementComponent.h"

void UTMOPTrafficVehicleSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Vehicles.Reset();
}

void UTMOPTrafficVehicleSubsystem::Deinitialize()
{
    Vehicles.Reset();
    Super::Deinitialize();
}

bool UTMOPTrafficVehicleSubsystem::RegisterVehicle(UTMOPTrafficVehicleMovementComponent* Vehicle)
{
    if (!IsValid(Vehicle)) return false;
    for (const TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent>& Existing : Vehicles)
        if (Existing.Get() == Vehicle) return true;
    Vehicles.Add(Vehicle);
    return true;
}

bool UTMOPTrafficVehicleSubsystem::UnregisterVehicle(UTMOPTrafficVehicleMovementComponent* Vehicle)
{
    for (int32 Index = Vehicles.Num() - 1; Index >= 0; --Index)
    {
        if (!Vehicles[Index].IsValid() || Vehicles[Index].Get() == Vehicle)
        {
            const bool bMatched = Vehicles[Index].Get() == Vehicle;
            Vehicles.RemoveAtSwap(Index);
            if (bMatched) return true;
        }
    }
    return false;
}

UTMOPTrafficVehicleMovementComponent* UTMOPTrafficVehicleSubsystem::FindLeadVehicle(
    const UTMOPTrafficVehicleMovementComponent* Vehicle, float& OutCenterDistance) const
{
    OutCenterDistance = TNumericLimits<float>::Max();
    if (!IsValid(Vehicle) || Vehicle->CurrentLaneId.IsNone()) return nullptr;
    UTMOPTrafficVehicleMovementComponent* Best = nullptr;
    for (const TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent>& CandidatePointer : Vehicles)
    {
        UTMOPTrafficVehicleMovementComponent* Candidate = CandidatePointer.Get();
        if (!IsValid(Candidate) || Candidate == Vehicle ||
            Candidate->CurrentLaneId != Vehicle->CurrentLaneId) continue;
        const float Delta = Candidate->DistanceAlongLane - Vehicle->DistanceAlongLane;
        if (Delta > 0.0f && Delta < OutCenterDistance)
        {
            OutCenterDistance = Delta;
            Best = Candidate;
        }
    }
    return Best;
}

void UTMOPTrafficVehicleSubsystem::FindNearestVehiclesInLane(const FName LaneId,
    const float ReferenceDistance, const UTMOPTrafficVehicleMovementComponent* IgnoredVehicle,
    float& OutAheadDistance, float& OutBehindDistance) const
{
    OutAheadDistance = TNumericLimits<float>::Max();
    OutBehindDistance = TNumericLimits<float>::Max();
    for (const TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent>& Pointer : Vehicles)
    {
        UTMOPTrafficVehicleMovementComponent* Candidate = Pointer.Get();
        if (!IsValid(Candidate) || Candidate == IgnoredVehicle || Candidate->CurrentLaneId != LaneId) continue;
        const float Delta = Candidate->DistanceAlongLane - ReferenceDistance;
        if (Delta >= 0.0f) OutAheadDistance = FMath::Min(OutAheadDistance, Delta);
        else OutBehindDistance = FMath::Min(OutBehindDistance, -Delta);
    }
}

int32 UTMOPTrafficVehicleSubsystem::GetActiveTrafficVehicleCount() const
{
    int32 Count = 0;
    for (const TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent>& Vehicle : Vehicles)
        if (Vehicle.IsValid()) ++Count;
    return Count;
}
