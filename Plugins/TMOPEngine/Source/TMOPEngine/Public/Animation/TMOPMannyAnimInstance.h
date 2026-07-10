#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Agents/TMOPAgentTypes.h"
#include "TMOPMannyAnimInstance.generated.h"

/**
 * Animation-state bridge for Manny-based Historical Agents.
 *
 * Create an Animation Blueprint with this class as Parent Class. Use:
 * - Speed for idle/walk/run locomotion
 * - bIsMoving for transitions
 * - bIsSeated for the sitting state
 * - ActivityState for more detailed transitions
 */
UCLASS(Blueprintable, BlueprintType)
class TMOPENGINE_API UTMOPMannyAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaSeconds) override;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Animation")
    float Speed = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Animation")
    bool bIsMoving = false;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Animation")
    bool bIsSeated = false;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Animation")
    bool bIsStandingStill = true;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Animation")
    ETMOPAgentActivityState ActivityState =
        ETMOPAgentActivityState::Idle;

private:
    UPROPERTY(Transient)
    TObjectPtr<class ATMOPHistoricalAgent> HistoricalAgent;
};
