#include "Venues/TMOPCinemaSeatComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

UTMOPCinemaSeatComponent::UTMOPCinemaSeatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPCinemaSeatComponent::BeginPlay()
{
    Super::BeginPlay();
    if (UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (UTMOPCinemaSeatSubsystem* Seats = GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>())
        {
            Seats->RegisterSeat(this);
        }
    }
}

void UTMOPCinemaSeatComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr)
    {
        if (UTMOPCinemaSeatSubsystem* Seats = GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>())
        {
            Seats->UnregisterSeat(this);
        }
    }
    OccupyingAgent = nullptr;
    Super::EndPlay(EndPlayReason);
}

FTransform UTMOPCinemaSeatComponent::GetSeatWorldTransform() const
{
    return FTransform(SeatedRotationOffset, SeatedLocalOffset, FVector::OneVector) * GetComponentTransform();
}

FTransform UTMOPCinemaSeatComponent::GetApproachWorldTransform() const
{
    if (bUseManualApproachTransform)
    {
        return ManualApproachTransform * GetComponentTransform();
    }
    const FVector Location = GetComponentLocation() + GetForwardVector() * ApproachDistance + GetUpVector() * ApproachVerticalOffset;
    return FTransform(GetComponentRotation(), Location, FVector::OneVector);
}

bool UTMOPCinemaSeatComponent::IsOccupied() const { return IsValid(OccupyingAgent); }
ATMOPHistoricalAgent* UTMOPCinemaSeatComponent::GetOccupyingAgent() const { return IsOccupied() ? OccupyingAgent : nullptr; }

bool UTMOPCinemaSeatComponent::ReserveSeat(ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || (IsOccupied() && OccupyingAgent != Agent)) return false;
    OccupyingAgent = Agent;
    return true;
}

bool UTMOPCinemaSeatComponent::ReleaseSeat(ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || OccupyingAgent != Agent) return false;
    OccupyingAgent = nullptr;
    return true;
}

bool UTMOPCinemaSeatComponent::SeatAgent(ATMOPHistoricalAgent* Agent)
{
    if (!ReserveSeat(Agent)) return false;
    Agent->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    if (UCharacterMovementComponent* Movement = Agent->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
        Movement->DisableMovement();
    }
    const FTransform Target = GetSeatWorldTransform();
    Agent->SetActorLocationAndRotation(Target.GetLocation(), Target.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
    if (bAttachAgentWhileSeated)
    {
        Agent->AttachToComponent(this, FAttachmentTransformRules::KeepWorldTransform);
    }
    Agent->SetActivityState(ETMOPAgentActivityState::Seated);
    OnAgentSeated.Broadcast(SeatId, Agent);
    return true;
}

bool UTMOPCinemaSeatComponent::StandAgent(ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || OccupyingAgent != Agent) return false;
    Agent->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    const FTransform Target = GetApproachWorldTransform();
    Agent->SetActorLocationAndRotation(Target.GetLocation(), Target.Rotator(), false, nullptr, ETeleportType::TeleportPhysics);
    OccupyingAgent = nullptr;
    Agent->SetActivityState(ETMOPAgentActivityState::Standing);
    OnAgentStoodUp.Broadcast(SeatId, Agent);
    return true;
}
