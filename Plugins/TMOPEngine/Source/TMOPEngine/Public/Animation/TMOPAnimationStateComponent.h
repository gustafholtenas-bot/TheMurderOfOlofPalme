#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Animation/TMOPAnimationTypes.h"
#include "TMOPAnimationStateComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPAnimReactionSignature, ETMOPAnimReaction, Reaction);

/** Runtime animation intent shared by every TMOP agent and Animation Blueprint. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPAnimationStateComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPAnimationStateComponent();
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    bool bDerivePostureAndMovementFromAgent = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    ETMOPAnimPosture Posture = ETMOPAnimPosture::Standing;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    ETMOPAnimLocomotionStyle LocomotionStyle = ETMOPAnimLocomotionStyle::Normal;

    /** Dead band around gait thresholds; prevents rapid back-and-forth switching. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Animation|Locomotion Stability",
        meta=(ClampMin="0.0", Units="cm/s"))
    float LocomotionStyleHysteresisCmPerSecond = 18.0f;

    /** A new automatic gait must remain requested this long before it is accepted. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Animation|Locomotion Stability",
        meta=(ClampMin="0.0", Units="s"))
    float LocomotionStyleChangeDelaySeconds = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    ETMOPAnimOverlay Overlay = ETMOPAnimOverlay::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    ETMOPAnimWeaponPose WeaponPose = ETMOPAnimWeaponPose::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation")
    ETMOPAnimReaction ActiveReaction = ETMOPAnimReaction::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation")
    bool bIsDeadOnGround = false;

    /** Procedural social-look values for the Animation Blueprint. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Social")
    float SocialLookYaw = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Social")
    float SocialLookPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation|Social")
    float SocialLookAlpha = 0.0f;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Animation|Events")
    FTMOPAnimReactionSignature OnReactionTriggered;

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void SetOverlay(ETMOPAnimOverlay NewOverlay);

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void SetWeaponPose(ETMOPAnimWeaponPose NewWeaponPose);

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void SetLocomotionStyle(ETMOPAnimLocomotionStyle NewStyle);

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void SetPostureOverride(ETMOPAnimPosture NewPosture);

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void TriggerReaction(ETMOPAnimReaction Reaction, float DurationSeconds = 0.75f);

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void ClearReaction();

    UFUNCTION(BlueprintCallable, Category="TMOP|Animation")
    void SetAutomaticStateDerivation(bool bEnabled);

private:
    void UpdateFromOwner(float DeltaTime);
    float ReactionTimeRemaining = 0.0f;
    ETMOPAnimLocomotionStyle PendingLocomotionStyle =
        ETMOPAnimLocomotionStyle::Normal;
    float PendingLocomotionStyleSeconds = 0.0f;
};
