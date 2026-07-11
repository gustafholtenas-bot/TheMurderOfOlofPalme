#include "Vehicles/TMOPVehicleSeatComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "GameFramework/CharacterMovementComponent.h"

UTMOPVehicleSeatComponent::UTMOPVehicleSeatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPVehicleSeatComponent::IsOccupied() const
{
    return IsValid(Occupant);
}

ATMOPHistoricalAgent* UTMOPVehicleSeatComponent::GetOccupant() const
{
    return Occupant;
}

bool UTMOPVehicleSeatComponent::EnterSeat(
    ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || (IsOccupied() && Occupant != Agent))
    {
        return false;
    }

    Occupant = Agent;

    if (UCharacterMovementComponent* Movement =
        Agent->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }

    Agent->SetActorLocationAndRotation(
        GetComponentLocation(),
        GetComponentRotation(),
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    Agent->AttachToComponent(
        this,
        FAttachmentTransformRules::KeepWorldTransform);

    Agent->SetActivityState(
        ETMOPAgentActivityState::RidingVehicle);

    return true;
}

bool UTMOPVehicleSeatComponent::ExitSeat(
    ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || Occupant != Agent)
    {
        return false;
    }

    Agent->DetachFromActor(
        FDetachmentTransformRules::KeepWorldTransform);

    const FTransform ExitTransform(
        ExitRotationOffset,
        ExitLocalOffset,
        FVector::OneVector);

    const FTransform WorldExit =
        ExitTransform * GetComponentTransform();

    Agent->SetActorLocationAndRotation(
        WorldExit.GetLocation(),
        WorldExit.Rotator(),
        false,
        nullptr,
        ETeleportType::TeleportPhysics);

    Agent->SetActivityState(
        ETMOPAgentActivityState::Standing);

    Occupant = nullptr;
    return true;
}
