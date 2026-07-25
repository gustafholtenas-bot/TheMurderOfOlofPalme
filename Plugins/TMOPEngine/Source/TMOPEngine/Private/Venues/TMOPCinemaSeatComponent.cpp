#include "Venues/TMOPCinemaSeatComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "Animation/TMOPAnimationTypes.h"
#include "Components/CapsuleComponent.h"
#include "Engine/GameInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
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
    FRotator RotationOffset = SeatedRotationOffset;
    if (bReverseSeatedFacing)
    {
        RotationOffset.Yaw += 180.0f;
    }
    const FVector CharacterOriginOffset =
        SeatedLocalOffset + FVector(0.0f, 0.0f, SeatedCharacterOriginHeight);
    return FTransform(RotationOffset, CharacterOriginOffset, FVector::OneVector) *
        GetComponentTransform();
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
    // The initial profile may already say Seated. SetActivityState then
    // intentionally performs no transition, so explicitly refresh the
    // animation intent as part of the physical seating operation.
    Agent->SetActivityState(ETMOPAgentActivityState::Seated);
    if (UTMOPAnimationStateComponent* AnimationState =
        Agent->FindComponentByClass<UTMOPAnimationStateComponent>())
    {
        AnimationState->SetPostureOverride(ETMOPAnimPosture::Sitting);
    }
    OnAgentSeated.Broadcast(SeatId, Agent);
    return true;
}

bool UTMOPCinemaSeatComponent::StandAgent(ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent) || OccupyingAgent != Agent) return false;
    Agent->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
    const FTransform Target = GetApproachWorldTransform();
    FVector StandLocation = Target.GetLocation();
    if (bProjectApproachToNavMesh)
    {
        if (UNavigationSystemV1* Navigation =
            FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
        {
            FNavLocation ProjectedLocation;
            if (Navigation->ProjectPointToNavigation(
                StandLocation, ProjectedLocation, ApproachNavProjectionExtent))
            {
                StandLocation = ProjectedLocation.Location;
            }
            else
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("TMOP seat '%s': approach point could not be projected to NavMesh."),
                    *SeatId.ToString());
            }
        }
    }
    if (bApproachIsFootLocation && IsValid(Agent->GetCapsuleComponent()))
    {
        StandLocation.Z +=
            Agent->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    }
    Agent->SetActorLocationAndRotation(
        StandLocation, Target.Rotator(), false, nullptr,
        ETeleportType::TeleportPhysics);
    OccupyingAgent = nullptr;
    Agent->SetActivityState(ETMOPAgentActivityState::Standing);
    if (UTMOPAnimationStateComponent* AnimationState =
        Agent->FindComponentByClass<UTMOPAnimationStateComponent>())
    {
        AnimationState->SetAutomaticStateDerivation(true);
    }
    OnAgentStoodUp.Broadcast(SeatId, Agent);
    return true;
}
