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

    /** Animation-facing speed. Use this in locomotion BlendSpaces instead of raw Speed. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement|Smoothed")
    float SmoothedSpeed = 0.0f;

    /** Animation-facing local movement direction with wrap-safe interpolation. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement|Smoothed")
    float SmoothedDirection = 0.0f;

    /** Continuous cycle-rate correction. Connect to the BlendSpace player's Play Rate. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement|Smoothed")
    float LocomotionPlayRate = 1.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category="TMOP|Animation|Movement|Tuning", meta=(ClampMin="0.1"))
    float SpeedInterpolationRate = 6.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category="TMOP|Animation|Movement|Tuning", meta=(ClampMin="1.0"))
    float DirectionInterpolationDegreesPerSecond = 540.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category="TMOP|Animation|Movement|Tuning", meta=(ClampMin="0.1"))
    float PlayRateInterpolationRate = 8.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category="TMOP|Animation|Movement|Tuning", meta=(ClampMin="0.1"))
    float MinimumLocomotionPlayRate = 0.72f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly,
        Category="TMOP|Animation|Movement|Tuning", meta=(ClampMin="0.1"))
    float MaximumLocomotionPlayRate = 1.30f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsInAir = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsAccelerating = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    float VerticalVelocity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    float AirTimeSeconds = 0.0f;

    /** True for one animation update immediately after leaving falling state. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bJustLanded = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Movement")
    bool bIsCrouching = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Actions")
    bool bIsPunching = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Actions")
    bool bIsKicking = false;

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

    /** Feed these into spine/neck Transform (Modify) Bone nodes. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Social")
    float SocialLookYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Social")
    float SocialLookPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Social")
    float SocialLookAlpha = 0.0f;

private:
    UPROPERTY(Transient)
    TObjectPtr<class ACharacter> CharacterOwner;

    UPROPERTY(Transient)
    TObjectPtr<class ATMOPHistoricalAgent> HistoricalAgent;

    UPROPERTY(Transient)
    TObjectPtr<class UTMOPAnimationStateComponent> AnimationState;

    bool bWasInAir = false;
    bool bMovementSmoothingInitialized = false;
};
