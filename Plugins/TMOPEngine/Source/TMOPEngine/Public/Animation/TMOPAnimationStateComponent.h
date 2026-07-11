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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    ETMOPAnimOverlay Overlay = ETMOPAnimOverlay::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Animation")
    ETMOPAnimWeaponPose WeaponPose = ETMOPAnimWeaponPose::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation")
    ETMOPAnimReaction ActiveReaction = ETMOPAnimReaction::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Animation")
    bool bIsDeadOnGround = false;

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

private:
    void UpdateFromAgent();
    float ReactionTimeRemaining = 0.0f;
};
