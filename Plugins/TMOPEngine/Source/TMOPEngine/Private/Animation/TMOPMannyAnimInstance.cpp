#include "Animation/TMOPMannyAnimInstance.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void UTMOPMannyAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    HistoricalAgent = Cast<ATMOPHistoricalAgent>(TryGetPawnOwner());
    AnimationState = IsValid(HistoricalAgent)
        ? HistoricalAgent->FindComponentByClass<UTMOPAnimationStateComponent>() : nullptr;
}

void UTMOPMannyAnimInstance::NativeUpdateAnimation(const float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);
    if (!IsValid(HistoricalAgent)) NativeInitializeAnimation();
    if (!IsValid(HistoricalAgent))
    {
        Speed = 0.0f; Direction = 0.0f; bIsMoving = false; bIsInAir = false;
        bIsAccelerating = false; bIsSeated = false; bIsStandingStill = true;
        return;
    }
    if (!IsValid(AnimationState))
        AnimationState = HistoricalAgent->FindComponentByClass<UTMOPAnimationStateComponent>();

    const FVector Velocity = HistoricalAgent->GetVelocity();
    Speed = Velocity.Size2D();
    const FVector LocalVelocity = HistoricalAgent->GetActorTransform().InverseTransformVectorNoScale(Velocity);
    Direction = Speed > KINDA_SMALL_NUMBER
        ? FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X)) : 0.0f;
    bIsMoving = Speed > 3.0f;
    ActivityState = HistoricalAgent->ActivityState;
    if (const UCharacterMovementComponent* Movement = HistoricalAgent->GetCharacterMovement())
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
        bIsDeadOnGround = HistoricalAgent->LifeState == ETMOPAgentLifeState::Dead;
    }
    bIsSeated = Posture == ETMOPAnimPosture::Sitting || Posture == ETMOPAnimPosture::SittingInCar;
    bIsGrounded = Posture == ETMOPAnimPosture::Grounded;
    bIsStandingStill = !bIsMoving && Posture == ETMOPAnimPosture::Standing;
}
