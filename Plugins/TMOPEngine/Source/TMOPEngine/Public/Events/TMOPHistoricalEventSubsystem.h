#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "TMOPHistoricalEventSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPHistoricalEventTriggeredSignature,
    FName,
    EventId,
    FTMOPTime,
    TriggerTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPHistoricalEventsResetSignature,
    int32,
    LoopNumber);

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPHistoricalEventSubsystem final
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Events")
    FTMOPHistoricalEventTriggeredSignature OnHistoricalEventTriggered;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Events")
    FTMOPHistoricalEventsResetSignature OnHistoricalEventsReset;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Events")
    bool RegisterEventDefinition(
        const FTMOPHistoricalEventDefinition& Definition);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Events")
    bool RemoveEventDefinition(FName EventId);

    UFUNCTION(BlueprintPure, Category = "TMOP|Events")
    bool HasEventDefinition(FName EventId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Events")
    bool TryGetEventDefinition(
        FName EventId,
        FTMOPHistoricalEventDefinition& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Events")
    bool TryGetEventRuntime(
        FName EventId,
        FTMOPHistoricalEventRuntime& OutRuntime) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Events")
    bool HasEventTriggered(FName EventId) const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Events")
    bool TriggerEventNow(FName EventId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Events")
    bool CancelEvent(FName EventId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Events")
    void ResetEventsForLoop(int32 LoopNumber);

    UFUNCTION(BlueprintPure, Category = "TMOP|Events")
    TArray<FName> GetRegisteredEventIds() const;

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool ResolveEventTime(FName EventId);
    bool TriggerEventAtTime(FName EventId, const FTMOPTime& TriggerTime);
    int32 ChooseWindowSecond(
        int32 EarliestSecond,
        int32 PreferredSecond,
        int32 LatestSecond,
        ETMOPHistoricalLock LockMode,
        FName EventId) const;

    UPROPERTY(Transient)
    TMap<FName, FTMOPHistoricalEventDefinition> Definitions;

    UPROPERTY(Transient)
    TMap<FName, FTMOPHistoricalEventRuntime> RuntimeEvents;

    int32 CurrentLoopNumber = 1;
};
