#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPScheduleTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPScheduleActionType : uint8
{
    None,
    SetActivity,
    MoveToAnchor,
    WaitAtAnchor,
    SitAtSeat,
    StandUp,
    EnterVehicle,
    ExitVehicle,
    Interact,
    Custom
};

UENUM(BlueprintType)
enum class ETMOPScheduleEntryState : uint8
{
    Pending,
    Ready,
    Executed,
    Skipped,
    Cancelled
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule")
    FName EntryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule")
    ETMOPHistoricalLock HistoricalLock = ETMOPHistoricalLock::Free;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Absolute")
    FTMOPTime AbsoluteTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Window")
    FTMOPTime EarliestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Window")
    FTMOPTime PreferredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Window")
    FTMOPTime LatestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Relative")
    FName TriggerEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Relative")
    int32 MinimumDelaySeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Relative")
    int32 PreferredDelaySeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Relative")
    int32 MaximumDelaySeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Action")
    ETMOPScheduleActionType ActionType = ETMOPScheduleActionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Action")
    ETMOPAgentActivityState ActivityState =
        ETMOPAgentActivityState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Action")
    FName TargetAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Action")
    FName TargetEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Action")
    FString CustomActionId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Source")
    FString SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule|Source")
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAgentSchedule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule")
    FName AgentId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Schedule")
    TArray<FTMOPScheduleEntry> Entries;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPScheduleEntryRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Schedule")
    FName EntryId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Schedule")
    ETMOPScheduleEntryState State = ETMOPScheduleEntryState::Pending;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Schedule")
    FTMOPTime ResolvedTime;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Schedule")
    bool bHasResolvedTime = false;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Schedule")
    int32 ExecutedLoopNumber = 0;
};
