#include "Animation/TMOPAnimationStateComponent.h"

#include "Agents/TMOPHistoricalAgent.h"

UTMOPAnimationStateComponent::UTMOPAnimationStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTMOPAnimationStateComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bDerivePostureAndMovementFromAgent) UpdateFromAgent();
    if (ActiveReaction != ETMOPAnimReaction::None && ReactionTimeRemaining >= 0.0f)
    {
        ReactionTimeRemaining -= DeltaTime;
        if (ReactionTimeRemaining <= 0.0f) ClearReaction();
    }
}

void UTMOPAnimationStateComponent::UpdateFromAgent()
{
    const ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    if (!IsValid(Agent)) return;
    bIsDeadOnGround = Agent->LifeState == ETMOPAgentLifeState::Dead;
    if (bIsDeadOnGround) { Posture = ETMOPAnimPosture::Grounded; return; }
    switch (Agent->ActivityState)
    {
    case ETMOPAgentActivityState::Seated:
        Posture = ETMOPAnimPosture::Sitting;
        break;
    case ETMOPAgentActivityState::RidingVehicle:
        Posture = ETMOPAnimPosture::SittingInCar;
        break;
    default:
        if (Posture != ETMOPAnimPosture::Grounded && Posture != ETMOPAnimPosture::Squatting)
            Posture = ETMOPAnimPosture::Standing;
        break;
    }
    switch (Agent->ActivityState)
    {
    case ETMOPAgentActivityState::FastWalking:
        LocomotionStyle = ETMOPAnimLocomotionStyle::Fast;
        break;
    case ETMOPAgentActivityState::Jogging:
        LocomotionStyle = ETMOPAnimLocomotionStyle::MildRun;
        break;
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Sprinting:
    case ETMOPAgentActivityState::Fleeing:
        LocomotionStyle = ETMOPAnimLocomotionStyle::FastRun;
        break;
    default:
        break;
    }
}

void UTMOPAnimationStateComponent::SetOverlay(const ETMOPAnimOverlay NewOverlay) { Overlay = NewOverlay; }
void UTMOPAnimationStateComponent::SetWeaponPose(const ETMOPAnimWeaponPose NewWeaponPose) { WeaponPose = NewWeaponPose; }
void UTMOPAnimationStateComponent::SetLocomotionStyle(const ETMOPAnimLocomotionStyle NewStyle) { LocomotionStyle = NewStyle; }
void UTMOPAnimationStateComponent::SetPostureOverride(const ETMOPAnimPosture NewPosture)
{
    bDerivePostureAndMovementFromAgent = false;
    Posture = NewPosture;
}

void UTMOPAnimationStateComponent::TriggerReaction(const ETMOPAnimReaction Reaction,
    const float DurationSeconds)
{
    ActiveReaction = Reaction;
    ReactionTimeRemaining = DurationSeconds < 0.0f ? -1.0f : FMath::Max(0.01f, DurationSeconds);
    OnReactionTriggered.Broadcast(Reaction);
}

void UTMOPAnimationStateComponent::ClearReaction()
{
    ActiveReaction = ETMOPAnimReaction::None;
    ReactionTimeRemaining = 0.0f;
}
