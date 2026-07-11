#include "Player/TMOPPlayerActionComponent.h"

#include "Animation/TMOPAnimationStateComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"

UTMOPPlayerActionComponent::UTMOPPlayerActionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTMOPPlayerActionComponent::BeginPlay()
{
    Super::BeginPlay();
    CharacterOwner = Cast<ACharacter>(GetOwner());
    AnimationState = IsValid(CharacterOwner)
        ? CharacterOwner->FindComponentByClass<UTMOPAnimationStateComponent>() : nullptr;
}

void UTMOPPlayerActionComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (CurrentAction == ETMOPPlayerAction::None || RemainingActionSeconds < 0.0f) return;
    RemainingActionSeconds -= DeltaTime;
    if (RemainingActionSeconds <= 0.0f) CompleteCurrentAction();
}

bool UTMOPPlayerActionComponent::CanStartAction(const ETMOPPlayerAction Action) const
{
    return Action != ETMOPPlayerAction::None && CurrentAction == ETMOPPlayerAction::None;
}

bool UTMOPPlayerActionComponent::StartAction(const ETMOPPlayerAction Action,
    AActor* Target, const float DurationSeconds, const bool bBlockMovement)
{
    if (!CanStartAction(Action)) return false;
    CurrentAction = Action;
    CurrentTarget = Target;
    RemainingActionSeconds = DurationSeconds < 0.0f ? -1.0f : FMath::Max(0.01f, DurationSeconds);
    SetMovementBlocked(bBlockMovement);
    ApplyAnimationForAction(Action);
    OnActionStarted.Broadcast(Action, Target);
    return true;
}

bool UTMOPPlayerActionComponent::CompleteCurrentAction()
{
    if (CurrentAction == ETMOPPlayerAction::None) return false;
    const ETMOPPlayerAction Completed = CurrentAction;
    AActor* Target = CurrentTarget;
    ClearActionAnimation(Completed);
    SetMovementBlocked(false);
    CurrentAction = ETMOPPlayerAction::None;
    CurrentTarget = nullptr;
    RemainingActionSeconds = 0.0f;
    OnActionCompleted.Broadcast(Completed, Target);
    return true;
}

bool UTMOPPlayerActionComponent::CancelCurrentAction()
{
    if (CurrentAction == ETMOPPlayerAction::None) return false;
    const ETMOPPlayerAction Cancelled = CurrentAction;
    AActor* Target = CurrentTarget;
    ClearActionAnimation(Cancelled);
    SetMovementBlocked(false);
    CurrentAction = ETMOPPlayerAction::None;
    CurrentTarget = nullptr;
    RemainingActionSeconds = 0.0f;
    OnActionCancelled.Broadcast(Cancelled, Target);
    return true;
}

void UTMOPPlayerActionComponent::ApplyAnimationForAction(const ETMOPPlayerAction Action)
{
    if (!IsValid(AnimationState)) return;
    using Overlay = ETMOPAnimOverlay;
    using Reaction = ETMOPAnimReaction;
    switch (Action)
    {
    case ETMOPPlayerAction::Talk: AnimationState->SetOverlay(Overlay::Talking); break;
    case ETMOPPlayerAction::PhoneOrRadio: AnimationState->SetOverlay(Overlay::PhoneOrRadio); break;
    case ETMOPPlayerAction::Smoke: AnimationState->SetOverlay(Overlay::Smoking); break;
    case ETMOPPlayerAction::LookAround: AnimationState->SetOverlay(Overlay::LookAround); break;
    case ETMOPPlayerAction::Sit: AnimationState->SetPostureOverride(ETMOPAnimPosture::Sitting); break;
    case ETMOPPlayerAction::Stand: AnimationState->SetPostureOverride(ETMOPAnimPosture::Standing); break;
    case ETMOPPlayerAction::Punch: AnimationState->TriggerReaction(Reaction::Punch, RemainingActionSeconds); break;
    case ETMOPPlayerAction::Kick: AnimationState->TriggerReaction(Reaction::Kick, RemainingActionSeconds); break;
    case ETMOPPlayerAction::SwingWeapon: AnimationState->TriggerReaction(Reaction::SwingWeapon, RemainingActionSeconds); break;
    case ETMOPPlayerAction::AimGun: AnimationState->TriggerReaction(Reaction::AimGun, -1.0f); break;
    case ETMOPPlayerAction::ShootGun: AnimationState->TriggerReaction(Reaction::ShootGun, RemainingActionSeconds); break;
    default: break;
    }
}

void UTMOPPlayerActionComponent::ClearActionAnimation(const ETMOPPlayerAction Action)
{
    if (!IsValid(AnimationState)) return;
    switch (Action)
    {
    case ETMOPPlayerAction::Talk:
    case ETMOPPlayerAction::PhoneOrRadio:
    case ETMOPPlayerAction::Smoke:
    case ETMOPPlayerAction::LookAround:
        AnimationState->SetOverlay(ETMOPAnimOverlay::None);
        break;
    case ETMOPPlayerAction::Punch:
    case ETMOPPlayerAction::Kick:
    case ETMOPPlayerAction::SwingWeapon:
    case ETMOPPlayerAction::AimGun:
    case ETMOPPlayerAction::ShootGun:
        AnimationState->ClearReaction();
        break;
    default: break;
    }
}

void UTMOPPlayerActionComponent::SetMovementBlocked(const bool bBlocked)
{
    bMovementBlocked = bBlocked;
    if (!IsValid(CharacterOwner)) return;
    if (AController* Controller = CharacterOwner->GetController())
        Controller->SetIgnoreMoveInput(bBlocked);
}
