#include "Animation/TMOPAnimationStateComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "GameFramework/CharacterMovementComponent.h"

UTMOPAnimationStateComponent::UTMOPAnimationStateComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTMOPAnimationStateComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (bDerivePostureAndMovementFromAgent) UpdateFromOwner();
    if (ActiveReaction != ETMOPAnimReaction::None && ReactionTimeRemaining >= 0.0f)
    {
        ReactionTimeRemaining -= DeltaTime;
        if (ReactionTimeRemaining <= 0.0f) ClearReaction();
    }
}

void UTMOPAnimationStateComponent::UpdateFromOwner()
{
    const ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    if (!IsValid(Agent))
    {
        bIsDeadOnGround = false;
        return;
    }
    // Unconscious casualties must use the grounded body posture as well.
    // This keeps medical simulations historically correct without falsely
    // marking a casualty Dead merely to obtain the lying animation.
    bIsDeadOnGround =
        Agent->LifeState == ETMOPAgentLifeState::Dead ||
        Agent->LifeState == ETMOPAgentLifeState::Unconscious;
    SocialLookYaw = Agent->SocialLookYaw;
    SocialLookPitch = Agent->SocialLookPitch;
    SocialLookAlpha = Agent->SocialLookAlpha;
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
    case ETMOPAgentActivityState::Walking:
    case ETMOPAgentActivityState::FastWalking:
    case ETMOPAgentActivityState::Jogging:
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Sprinting:
    case ETMOPAgentActivityState::Fleeing:
    {
        // Arrival-timed moves can use an exact runtime speed which differs
        // from the broad ActivityState. Derive the animation gait from the
        // actual MaxWalkSpeed so a 4.0 m/s deadline selects a run animation,
        // while a social 1.2 m/s move stays in the normal walking blendspace.
        const UCharacterMovementComponent* Movement =
            Agent->GetCharacterMovement();
        const float DesiredSpeed = IsValid(Movement)
            ? Movement->MaxWalkSpeed : Agent->GetDesiredMovementSpeed();
        const float Multiplier =
            Agent->MovementProfile.PersonalSpeedMultiplier *
            Agent->AppearanceMovementSpeedMultiplier;
        const float Normal = Agent->MovementProfile.NormalWalkSpeed * Multiplier;
        const float Fast = Agent->MovementProfile.FastWalkSpeed * Multiplier;
        const float Jog = Agent->MovementProfile.JogSpeed * Multiplier;
        const float Run = Agent->MovementProfile.RunSpeed * Multiplier;
        if (DesiredSpeed >= (Jog + Run) * 0.5f)
            LocomotionStyle = ETMOPAnimLocomotionStyle::FastRun;
        else if (DesiredSpeed >= (Fast + Jog) * 0.5f)
            LocomotionStyle = ETMOPAnimLocomotionStyle::MildRun;
        else if (DesiredSpeed >= (Normal + Fast) * 0.5f)
            LocomotionStyle = ETMOPAnimLocomotionStyle::Fast;
        else
            LocomotionStyle = ETMOPAnimLocomotionStyle::Normal;
        break;
    }
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

void UTMOPAnimationStateComponent::SetAutomaticStateDerivation(const bool bEnabled)
{
    bDerivePostureAndMovementFromAgent = bEnabled;
}
