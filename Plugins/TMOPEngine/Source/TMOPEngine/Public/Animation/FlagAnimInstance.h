#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "FlagAnimInstance.generated.h"

class UAnimSequence;

/** Deterministic, world-position-driven animation phase for city flags. */
UCLASS(Blueprintable, BlueprintType)
class TMOPENGINE_API UFlagAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    /** Assign the looping flag animation in the Anim Blueprint defaults. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TMOP|Flag")
    TObjectPtr<UAnimSequence> FlagSequence = nullptr;

    /** Direction wind travels, in world XY. (1,0,0) = +X. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TMOP|Flag|Wind")
    FVector2D WindDirection = FVector2D(1.0, 0.0);

    /** Wind propagation speed in meters/second; not the animation playback rate. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TMOP|Flag|Wind", meta=(ClampMin="0.01", UIMin="0.1"))
    float WindSpeedMetersPerSecond = 8.0f;

    /** Additional waves per meter, independent of propagation speed (0 disables). */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TMOP|Flag|Wind", meta=(ClampMin="0.0"))
    float ExtraCyclesPerMeter = 0.0f;

    /** Small stable per-flag offset, in seconds. 0 = perfectly synchronized wave. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="TMOP|Flag|Wind", meta=(ClampMin="0.0"))
    float JitterSeconds = 0.0f;

    /** Read this in the AnimGraph Sequence Player's Start Position. */
    UPROPERTY(Transient, BlueprintReadOnly, Category="TMOP|Flag")
    float StartOffset = 0.0f;

    virtual void NativeInitializeAnimation() override;
};
