#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Traffic/TMOPTrafficSignalTypes.h"
#include "TMOPTrafficSignalComponent.generated.h"

/** Optional visual signal head. Blueprint can update lamps from CurrentState. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPTrafficSignalComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPTrafficSignalComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    FName SignalGroupId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic Signal")
    ETMOPTrafficSignalState CurrentState = ETMOPTrafficSignalState::Red;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    void ApplySignalState(ETMOPTrafficSignalState NewState);
};
