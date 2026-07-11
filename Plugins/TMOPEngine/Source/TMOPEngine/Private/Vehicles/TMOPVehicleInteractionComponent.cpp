#include "Vehicles/TMOPVehicleInteractionComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Vehicles/TMOPVehicleBase.h"

UTMOPVehicleInteractionComponent::UTMOPVehicleInteractionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPVehicleInteractionComponent::EnterVehicle(
    ATMOPVehicleBase* Vehicle,
    const FName PreferredSeatId)
{
    ATMOPHistoricalAgent* Agent = GetAgentOwner();

    if (!IsValid(Agent) || !IsValid(Vehicle) ||
        IsValid(CurrentVehicle))
    {
        return false;
    }

    if (!Vehicle->EnterVehicle(Agent, PreferredSeatId))
    {
        return false;
    }

    CurrentVehicle = Vehicle;
    return true;
}

bool UTMOPVehicleInteractionComponent::EnterDriverSeat(
    ATMOPVehicleBase* Vehicle)
{
    ATMOPHistoricalAgent* Agent = GetAgentOwner();

    if (!IsValid(Agent) || !IsValid(Vehicle) ||
        IsValid(CurrentVehicle))
    {
        return false;
    }

    if (!Vehicle->EnterDriverSeat(Agent))
    {
        return false;
    }

    CurrentVehicle = Vehicle;
    return true;
}

bool UTMOPVehicleInteractionComponent::ExitCurrentVehicle()
{
    ATMOPHistoricalAgent* Agent = GetAgentOwner();

    if (!IsValid(Agent) || !IsValid(CurrentVehicle))
    {
        return false;
    }

    if (!CurrentVehicle->ExitVehicle(Agent))
    {
        return false;
    }

    CurrentVehicle = nullptr;
    return true;
}

ATMOPHistoricalAgent*
UTMOPVehicleInteractionComponent::GetAgentOwner() const
{
    return Cast<ATMOPHistoricalAgent>(GetOwner());
}
