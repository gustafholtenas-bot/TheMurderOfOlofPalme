#include "Animation/TMOPMannyAnimInstance.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"

void UTMOPMannyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    CharacterOwner = Cast<ACharacter>(TryGetPawnOwner());
    HistoricalAgent = Cast<ATMOPHistoricalAgent>(CharacterOwner);
    AnimationState = IsValid(CharacterOwner)
        ? CharacterOwner->FindComponentByClass<UTMOPAnimationStateComponent>() : nullptr;
}

void UTMOPMannyAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!IsValid(CharacterOwner)) NativeInitializeAnimation();
    if (!IsValid(CharacterOwner))
    {
        Speed = 0.0f; Direction = 0.0f; bIsMoving = false; bIsInAir = false;
        bIsAccelerating = false; bIsSeated = false; bIsStandingStill = true;
        return;
    }
    if (!IsValid(AnimationState))
        AnimationState = CharacterOwner->FindComponentByClass<UTMOPAnimationStateComponent>();

    const FVector Velocity = CharacterOwner->GetVelocity();
    Speed = Velocity.Size2D();
    const FVector LocalVelocity = CharacterOwner->GetActorTransform().InverseTransformVectorNoScale(Velocity);
    Direction = Speed > KINDA_SMALL_NUMBER
        ? FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X)) : 0.0f;
    bIsMoving = Speed > 3.0f;
    if (IsValid(HistoricalAgent)) ActivityState = HistoricalAgent->ActivityState;
    if (const UCharacterMovementComponent* Movement = CharacterOwner->GetCharacterMovement())
    {
        bIsInAir = Movement->IsFalling();
        bIsAccelerating = Movement->GetCurrentAcceleration().SizeSquared2D() > 1.0f;
    }
    if (IsValid(AnimationState))
    {
        Posture = AnimationState->Posture;
        LocomotionStyle = AnimationState->LocomotionStyle;
        Overlay = AnimationState->Overlay;
        ActiveReaction = AnimationState->ActiveReaction;
        WeaponPose = AnimationState->WeaponPose;
        bIsDeadOnGround = AnimationState->bIsDeadOnGround;
    }
    else
    {
        Posture = ActivityState == ETMOPAgentActivityState::Seated
            ? ETMOPAnimPosture::Sitting : ETMOPAnimPosture::Standing;
        bIsDeadOnGround = IsValid(HistoricalAgent) &&
            HistoricalAgent->LifeState == ETMOPAgentLifeState::Dead;
    }
    bIsSeated = Posture == ETMOPAnimPosture::Sitting || Posture == ETMOPAnimPosture::SittingInCar;
    if (!IsValid(HistoricalAgent))
    {
        if (bIsSeated) ActivityState = ETMOPAgentActivityState::Seated;
        else if (bIsMoving)
            ActivityState = Speed > 350.0f ? ETMOPAgentActivityState::Running : ETMOPAgentActivityState::Walking;
        else ActivityState = ETMOPAgentActivityState::Idle;
    }
    bIsGrounded = Posture == ETMOPAnimPosture::Grounded;
    bIsStandingStill = !bIsMoving && Posture == ETMOPAnimPosture::Standing;
}
