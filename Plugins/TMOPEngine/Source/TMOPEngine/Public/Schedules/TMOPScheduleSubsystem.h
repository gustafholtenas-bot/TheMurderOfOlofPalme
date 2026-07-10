#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Schedules/TMOPScheduleTypes.h"
#include "TMOPScheduleSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTMOPScheduleEntryReadySignature,
    FName,
    AgentId,
    FTMOPScheduleEntry,
    Entry,
    FTMOPTime,
    TriggerTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPSchedulesResetSignature,
    int32,
    LoopNumber);

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPScheduleSubsystem final
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Schedules")
    FTMOPScheduleEntryReadySignature OnScheduleEntryReady;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Schedules")
    FTMOPSchedulesResetSignature OnSchedulesReset;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Schedules")
    bool RegisterAgentSchedule(const FTMOPAgentSchedule& Schedule);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Schedules")
    bool RemoveAgentSchedule(FName AgentId);

    UFUNCTION(BlueprintPure, Category = "TMOP|Schedules")
    bool HasAgentSchedule(FName AgentId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Schedules")
    bool TryGetAgentSchedule(
        FName AgentId,
        FTMOPAgentSchedule& OutSchedule) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Schedules")
    bool TryGetEntryRuntime(
        FName AgentId,
        FName EntryId,
        FTMOPScheduleEntryRuntime& OutRuntime) const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Schedules")
    bool MarkEntryExecuted(FName AgentId, FName EntryId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Schedules")
    bool CancelEntry(FName AgentId, FName EntryId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Schedules")
    void ResetSchedulesForLoop(int32 LoopNumber);

    UFUNCTION(BlueprintPure, Category = "TMOP|Schedules")
    TArray<FName> GetScheduledAgentIds() const;

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleHistoricalEventTriggered(
        FName EventId,
        FTMOPTime TriggerTime);

    UFUNCTION()
    void HandleLoopRestarted(
        int32 NewLoopNumber,
        FTMOPTime RestartTime);

    void ResolveSchedule(FName AgentId);
    bool ResolveEntryTime(
        FName AgentId,
        const FTMOPScheduleEntry& Entry,
        FTMOPScheduleEntryRuntime& Runtime);

    int32 ChooseWindowSecond(
        int32 EarliestSecond,
        int32 PreferredSecond,
        int32 LatestSecond,
        ETMOPHistoricalLock LockMode,
        FName AgentId,
        FName EntryId) const;

    FString MakeRuntimeKey(FName AgentId, FName EntryId) const;

    UPROPERTY(Transient)
    TMap<FName, FTMOPAgentSchedule> Schedules;

    UPROPERTY(Transient)
    TMap<FString, FTMOPScheduleEntryRuntime> RuntimeEntries;

    UPROPERTY(Transient)
    TMap<FName, FTMOPTime> TriggeredEventTimes;

    int32 CurrentLoopNumber = 1;
};
