#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPHistoricalEventTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPEventTimingMode : uint8
{
    Absolute,
    Window,
    Relative
};

UENUM(BlueprintType)
enum class ETMOPHistoricalLock : uint8
{
    HardLocked,
    SoftLocked,
    Free
};

UENUM(BlueprintType)
enum class ETMOPEventRuntimeState : uint8
{
    Pending,
    Triggered,
    Cancelled
};

/** A central historical event definition; also usable directly as a DataTable row. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalEventDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    FName EventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    ETMOPHistoricalLock HistoricalLock = ETMOPHistoricalLock::Free;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Absolute")
    FTMOPTime AbsoluteTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Window")
    FTMOPTime EarliestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Window")
    FTMOPTime PreferredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Window")
    FTMOPTime LatestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Relative")
    FName TriggerEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Relative")
    int32 MinimumDelaySeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Relative")
    int32 PreferredDelaySeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event|Relative")
    int32 MaximumDelaySeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    FString SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Event")
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalEventRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Event")
    FName EventId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Event")
    ETMOPEventRuntimeState State = ETMOPEventRuntimeState::Pending;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Event")
    FTMOPTime ResolvedTime;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Event")
    int32 TriggeredLoopNumber = 0;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Event")
    bool bHasResolvedTime = false;
};
