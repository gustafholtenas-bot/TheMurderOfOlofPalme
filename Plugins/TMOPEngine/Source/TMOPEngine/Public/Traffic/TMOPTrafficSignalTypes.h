#pragma once

#include "CoreMinimal.h"
#include "TMOPTrafficSignalTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPTrafficSignalState : uint8
{
    Red,
    RedYellow,
    Green,
    Yellow,
    Disabled
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPSignalGroupState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    FName SignalGroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    ETMOPTrafficSignalState State = ETMOPTrafficSignalState::Red;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTrafficSignalPhase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal",
        meta=(ClampMin="0.1"))
    float DurationSeconds = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    TArray<FTMOPSignalGroupState> GroupStates;
};
