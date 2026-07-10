#include "Agents/TMOPHistoricalAgent.h"

#include "Actions/TMOPActionExecutorComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Routes/TMOPRouteFollowerComponent.h"

ATMOPHistoricalAgent::ATMOPHistoricalAgent()
{
    PrimaryActorTick.bCanEverTick = false;

    EntityIdentity =
        CreateDefaultSubobject<UTMOPWorldEntityComponent>(
            TEXT("EntityIdentity"));

    if (EntityIdentity != nullptr)
    {
        EntityIdentity->EntityType = TEXT("Agent");
    }

    ActionExecutor =
        CreateDefaultSubobject<UTMOPActionExecutorComponent>(
            TEXT("ActionExecutor"));

    RouteFollower =
        CreateDefaultSubobject<UTMOPRouteFollowerComponent>(
            TEXT("RouteFollower"));
}

void ATMOPHistoricalAgent::BeginPlay()
{
    Super::BeginPlay();
    ApplyMovementSpeedForActivity();
}

bool ATMOPHistoricalAgent::SetLifeState(
    const ETMOPAgentLifeState NewState)
{
    if (LifeState == NewState)
    {
        return false;
    }

    const ETMOPAgentLifeState OldState = LifeState;
    LifeState = NewState;

    HandleLifeStateChanged(OldState, NewState);
    OnLifeStateChanged.Broadcast(OldState, NewState);
    return true;
}

bool ATMOPHistoricalAgent::SetActivityState(
    const ETMOPAgentActivityState NewActivity)
{
    if (ActivityState == NewActivity)
    {
        return false;
    }

    const ETMOPAgentActivityState OldActivity = ActivityState;
    ActivityState = NewActivity;

    ApplyMovementSpeedForActivity();
    HandleActivityStateChanged(OldActivity, NewActivity);
    OnActivityStateChanged.Broadcast(OldActivity, NewActivity);
    return true;
}

void ATMOPHistoricalAgent::ApplyMovementSpeedForActivity()
{
    if (UCharacterMovementComponent* Movement =
        GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = GetDesiredMovementSpeed();
        Movement->SetMovementMode(
            CanMove() ? MOVE_Walking : MOVE_None);
    }
}

float ATMOPHistoricalAgent::GetDesiredMovementSpeed() const
{
    float BaseSpeed = 0.0f;

    switch (ActivityState)
    {
    case ETMOPAgentActivityState::Walking:
        BaseSpeed = MovementProfile.NormalWalkSpeed;
        break;
    case ETMOPAgentActivityState::FastWalking:
        BaseSpeed = MovementProfile.FastWalkSpeed;
        break;
    case ETMOPAgentActivityState::Jogging:
        BaseSpeed = MovementProfile.JogSpeed;
        break;
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Fleeing:
        BaseSpeed = MovementProfile.RunSpeed;
        break;
    case ETMOPAgentActivityState::Sprinting:
        BaseSpeed = MovementProfile.SprintSpeed;
        break;
    default:
        BaseSpeed = 0.0f;
        break;
    }

    return FMath::Max(
        0.0f,
        BaseSpeed * MovementProfile.PersonalSpeedMultiplier);
}

bool ATMOPHistoricalAgent::CanMove() const
{
    if (LifeState != ETMOPAgentLifeState::Alive)
    {
        return false;
    }

    switch (ActivityState)
    {
    case ETMOPAgentActivityState::Walking:
    case ETMOPAgentActivityState::FastWalking:
    case ETMOPAgentActivityState::Jogging:
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Sprinting:
    case ETMOPAgentActivityState::Fleeing:
        return true;
    default:
        return false;
    }
}

bool ATMOPHistoricalAgent::HasInitialSeatAssignment() const
{
    return !InitialSeatAssignment.VenueId.IsNone() &&
        !InitialSeatAssignment.SeatId.IsNone();
}

void ATMOPHistoricalAgent::HandleLifeStateChanged(
    const ETMOPAgentLifeState OldState,
    const ETMOPAgentLifeState NewState)
{
    ApplyMovementSpeedForActivity();
}

void ATMOPHistoricalAgent::HandleActivityStateChanged(
    const ETMOPAgentActivityState OldActivity,
    const ETMOPAgentActivityState NewActivity)
{
}
