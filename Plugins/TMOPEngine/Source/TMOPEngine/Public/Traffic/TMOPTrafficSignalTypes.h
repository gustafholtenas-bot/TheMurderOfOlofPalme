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
    Disabled,
    // Append only: preserve serialized enum values in existing levels/bakes.
    GreenYellow
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

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTrafficSignalGroup
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal") FName GroupId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal") bool bPedestrian = false;
};

/** Movements that must never be released together (including amber transitions). */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTrafficSignalConflict
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal") FName GroupA = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal") FName GroupB = NAME_None;
};

/** Protected stages; the builder adds amber, clearance and red/amber phases. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTrafficSignalStage
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal") TArray<FName> GreenGroups;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal", meta=(ClampMin="0.1")) float GreenSeconds = 20.0f;
    /** Extra all-red time AFTER amber. Include pedestrian crossing clearance here. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal", meta=(ClampMin="0.1")) float ClearanceSeconds = 10.0f;
};
