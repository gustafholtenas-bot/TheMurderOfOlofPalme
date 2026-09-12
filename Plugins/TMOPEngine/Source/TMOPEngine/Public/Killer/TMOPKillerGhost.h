#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPKillerGhost.generated.h"

class USkeletalMeshComponent;
class UAnimSequence;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** Visual hypothesis only: no historical identity, AI, collision, dialogue or observations. */
UCLASS(NotBlueprintable, Transient, NotPlaceable)
class TMOPENGINE_API ATMOPKillerGhost : public AActor
{
    GENERATED_BODY()
public:
    ATMOPKillerGhost();
    bool InitializeVisual(AActor* Source, UAnimSequence* Animation, UMaterialInterface* Material,
        const FLinearColor& Color);
    void SampleAnimation(double ElapsedSeconds, float PlayRate, float Opacity);
    float GetGroundOffset() const { return GroundOffset; }

private:
    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> Body;
    UPROPERTY(Transient)
    TObjectPtr<UAnimSequence> RunAnimation;
    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> GhostMaterial;
    float GroundOffset = 0.0f;
};
