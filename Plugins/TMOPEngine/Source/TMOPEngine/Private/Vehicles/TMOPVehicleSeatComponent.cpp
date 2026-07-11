#include "Vehicles/TMOPVehicleSeatComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UTMOPVehicleSeatComponent::UTMOPVehicleSeatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPVehicleSeatComponent::IsOccupied() const { return IsValid(CharacterOccupant); }
ATMOPHistoricalAgent* UTMOPVehicleSeatComponent::GetOccupant() const
{
    return Cast<ATMOPHistoricalAgent>(CharacterOccupant);
}
ACharacter* UTMOPVehicleSeatComponent::GetOccupantCharacter() const { return CharacterOccupant; }

bool UTMOPVehicleSeatComponent::EnterSeat(ATMOPHistoricalAgent* Agent)
{
    return EnterCharacterSeat(Agent);
}

bool UTMOPVehicleSeatComponent::ExitSeat(ATMOPHistoricalAgent* Agent)
{
    return ExitCharacterSeat(Agent);
}

bool UTMOPVehicleSeatComponent::EnterCharacterSeat(ACharacter* Character)
{
    if (!IsValid(Character) || (IsOccupied() && CharacterOccupant != Character)) return false;
    CharacterOccupant = Character;
    if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }
    Character->SetActorLocationAndRotation(GetComponentLocation(), GetComponentRotation(),
        false, nullptr, ETeleportType::TeleportPhysics);
    Character->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
    Character->SetActorEnableCollision(false);
    if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(Character))
        Agent->SetActivityState(ETMOPAgentActivityState::RidingVehicle);
    else if (UTMOPAnimationStateComponent* Animation =
        Character->FindComponentByClass<UTMOPAnimationStateComponent>())
        Animation->SetPostureOverride(ETMOPAnimPosture::SittingInCar);
    return true;
}

bool UTMOPVehicleSeatComponent::ExitCharacterSeat(ACharacter* Character)
{
    if (!IsValid(Character) || CharacterOccupant != Character) return false;
    Character->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    const FTransform ExitTransform(ExitRotationOffset, ExitLocalOffset, FVector::OneVector);
    const FTransform WorldExit = ExitTransform * GetComponentTransform();
    Character->SetActorLocationAndRotation(WorldExit.GetLocation(), WorldExit.Rotator(),
        false, nullptr, ETeleportType::TeleportPhysics);
    Character->SetActorEnableCollision(true);
    if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
        Movement->SetMovementMode(MOVE_Walking);
    if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(Character))
        Agent->SetActivityState(ETMOPAgentActivityState::Standing);
    else if (UTMOPAnimationStateComponent* Animation =
        Character->FindComponentByClass<UTMOPAnimationStateComponent>())
        Animation->SetPostureOverride(ETMOPAnimPosture::Standing);
    CharacterOccupant = nullptr;
    return true;
}
