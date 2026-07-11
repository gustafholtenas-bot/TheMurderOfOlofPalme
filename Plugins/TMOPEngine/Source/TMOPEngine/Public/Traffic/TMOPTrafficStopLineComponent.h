#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPTrafficStopLineComponent.generated.h"

/** Stop position on one directed lane, controlled by a signal group. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPTrafficStopLineComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPTrafficStopLineComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Stop Line")
    FName StopLineId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Stop Line")
    FName LaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Stop Line")
    FName SignalGroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Stop Line",
        meta=(ClampMin="0.0"))
    float DistanceAlongLane = 0.0f;

    /** Vehicle center stops this far before the painted line. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Stop Line",
        meta=(ClampMin="0.0"))
    float StopBufferCm = 100.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Stop Line")
    bool ValidateStopLine(TArray<FString>& OutErrors) const;
};
