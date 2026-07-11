#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Agents/TMOPAgentTypes.h"
#include "Animation/TMOPAnimationTypes.h"
#include "TMOPMannyAnimInstance.generated.h"

/** Universal parent class for ABP_TMOPAgent. Works with every Historical Agent. */
UCLASS(Blueprintable, BlueprintType)
class TMOPENGINE_API UTMOPMannyAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    float Direction = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Legacy")
    bool bIsSeated = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Legacy")
    bool bIsStandingStill = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Agent")
    ETMOPAgentActivityState ActivityState = ETMOPAgentActivityState::Idle;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    ETMOPAnimPosture Posture = ETMOPAnimPosture::Standing;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    ETMOPAnimLocomotionStyle LocomotionStyle = ETMOPAnimLocomotionStyle::Normal;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    ETMOPAnimOverlay Overlay = ETMOPAnimOverlay::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    ETMOPAnimReaction ActiveReaction = ETMOPAnimReaction::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    ETMOPAnimWeaponPose WeaponPose = ETMOPAnimWeaponPose::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    bool bIsDeadOnGround = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|State Machine")
    bool bIsGrounded = false;

private:
    UPROPERTY(Transient)
    TObjectPtr<class ATMOPHistoricalAgent> HistoricalAgent;

    UPROPERTY(Transient)
    TObjectPtr<class UTMOPAnimationStateComponent> AnimationState;
};
