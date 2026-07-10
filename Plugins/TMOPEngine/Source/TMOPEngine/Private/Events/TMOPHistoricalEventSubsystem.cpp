#include "Events/TMOPHistoricalEventSubsystem.h"

#include "Engine/GameInstance.h"
#include "Time/TMOPClockSubsystem.h"

void UTMOPHistoricalEventSubsystem::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Collection.InitializeDependency<UTMOPClockSubsystem>();

    if (UTMOPClockSubsystem* Clock =
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        Clock->OnSecondChanged.AddDynamic(
            this,
            &UTMOPHistoricalEventSubsystem::HandleSecondChanged);

        Clock->OnLoopRestarted.AddDynamic(
            this,
            &UTMOPHistoricalEventSubsystem::HandleLoopRestarted);
    }
}

void UTMOPHistoricalEventSubsystem::Deinitialize()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
        {
            Clock->OnSecondChanged.RemoveDynamic(
                this,
                &UTMOPHistoricalEventSubsystem::HandleSecondChanged);

            Clock->OnLoopRestarted.RemoveDynamic(
                this,
                &UTMOPHistoricalEventSubsystem::HandleLoopRestarted);
        }
    }

    Definitions.Reset();
    RuntimeEvents.Reset();

    Super::Deinitialize();
}

bool UTMOPHistoricalEventSubsystem::RegisterEventDefinition(
    const FTMOPHistoricalEventDefinition& Definition)
{
    if (Definition.EventId.IsNone())
    {
        return false;
    }

    Definitions.Add(Definition.EventId, Definition);

    FTMOPHistoricalEventRuntime Runtime;
    Runtime.EventId = Definition.EventId;
    Runtime.State = ETMOPEventRuntimeState::Pending;
    RuntimeEvents.Add(Definition.EventId, Runtime);

    ResolveEventTime(Definition.EventId);
    return true;
}

bool UTMOPHistoricalEventSubsystem::RemoveEventDefinition(
    const FName EventId)
{
    RuntimeEvents.Remove(EventId);
    return Definitions.Remove(EventId) > 0;
}

bool UTMOPHistoricalEventSubsystem::HasEventDefinition(
    const FName EventId) const
{
    return Definitions.Contains(EventId);
}

bool UTMOPHistoricalEventSubsystem::TryGetEventDefinition(
    const FName EventId,
    FTMOPHistoricalEventDefinition& OutDefinition) const
{
    if (const FTMOPHistoricalEventDefinition* Found =
        Definitions.Find(EventId))
    {
        OutDefinition = *Found;
        return true;
    }

    OutDefinition = FTMOPHistoricalEventDefinition();
    return false;
}

bool UTMOPHistoricalEventSubsystem::TryGetEventRuntime(
    const FName EventId,
    FTMOPHistoricalEventRuntime& OutRuntime) const
{
    if (const FTMOPHistoricalEventRuntime* Found =
        RuntimeEvents.Find(EventId))
    {
        OutRuntime = *Found;
        return true;
    }

    OutRuntime = FTMOPHistoricalEventRuntime();
    return false;
}

bool UTMOPHistoricalEventSubsystem::HasEventTriggered(
    const FName EventId) const
{
    const FTMOPHistoricalEventRuntime* Runtime =
        RuntimeEvents.Find(EventId);

    return Runtime != nullptr &&
        Runtime->State == ETMOPEventRuntimeState::Triggered;
}

bool UTMOPHistoricalEventSubsystem::TriggerEventNow(
    const FName EventId)
{
    UTMOPClockSubsystem* Clock =
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();

    return Clock != nullptr &&
        TriggerEventAtTime(EventId, Clock->GetCurrentTime());
}

bool UTMOPHistoricalEventSubsystem::CancelEvent(
    const FName EventId)
{
    FTMOPHistoricalEventRuntime* Runtime =
        RuntimeEvents.Find(EventId);

    if (Runtime == nullptr ||
        Runtime->State == ETMOPEventRuntimeState::Triggered)
    {
        return false;
    }

    Runtime->State = ETMOPEventRuntimeState::Cancelled;
    return true;
}

void UTMOPHistoricalEventSubsystem::ResetEventsForLoop(
    const int32 LoopNumber)
{
    CurrentLoopNumber = LoopNumber;

    for (TPair<FName, FTMOPHistoricalEventRuntime>& Pair : RuntimeEvents)
    {
        Pair.Value.State = ETMOPEventRuntimeState::Pending;
        Pair.Value.TriggeredLoopNumber = 0;
        Pair.Value.bHasResolvedTime = false;
    }

    for (const TPair<FName, FTMOPHistoricalEventDefinition>& Pair : Definitions)
    {
        ResolveEventTime(Pair.Key);
    }

    OnHistoricalEventsReset.Broadcast(LoopNumber);
}

TArray<FName> UTMOPHistoricalEventSubsystem::GetRegisteredEventIds() const
{
    TArray<FName> EventIds;
    Definitions.GetKeys(EventIds);
    EventIds.Sort(FNameLexicalLess());
    return EventIds;
}

void UTMOPHistoricalEventSubsystem::HandleSecondChanged(
    const FTMOPTime NewTime)
{
    const int32 CurrentSecond = NewTime.ToSecondsFromMidnight();

    for (TPair<FName, FTMOPHistoricalEventRuntime>& Pair : RuntimeEvents)
    {
        FTMOPHistoricalEventRuntime& Runtime = Pair.Value;

        if (Runtime.State != ETMOPEventRuntimeState::Pending)
        {
            continue;
        }

        if (!Runtime.bHasResolvedTime)
        {
            ResolveEventTime(Pair.Key);
        }

        if (Runtime.bHasResolvedTime &&
            CurrentSecond >= Runtime.ResolvedTime.ToSecondsFromMidnight())
        {
            TriggerEventAtTime(Pair.Key, NewTime);
        }
    }
}

void UTMOPHistoricalEventSubsystem::HandleLoopRestarted(
    const int32 NewLoopNumber,
    const FTMOPTime RestartTime)
{
    ResetEventsForLoop(NewLoopNumber);
}

bool UTMOPHistoricalEventSubsystem::ResolveEventTime(
    const FName EventId)
{
    const FTMOPHistoricalEventDefinition* Definition =
        Definitions.Find(EventId);

    FTMOPHistoricalEventRuntime* Runtime =
        RuntimeEvents.Find(EventId);

    if (Definition == nullptr || Runtime == nullptr)
    {
        return false;
    }

    int32 ResolvedSecond = 0;

    switch (Definition->TimingMode)
    {
    case ETMOPEventTimingMode::Absolute:
        ResolvedSecond = Definition->AbsoluteTime.ToSecondsFromMidnight();
        break;

    case ETMOPEventTimingMode::Window:
        ResolvedSecond = ChooseWindowSecond(
            Definition->EarliestTime.ToSecondsFromMidnight(),
            Definition->PreferredTime.ToSecondsFromMidnight(),
            Definition->LatestTime.ToSecondsFromMidnight(),
            Definition->HistoricalLock,
            EventId);
        break;

    case ETMOPEventTimingMode::Relative:
    {
        const FTMOPHistoricalEventRuntime* TriggerRuntime =
            RuntimeEvents.Find(Definition->TriggerEventId);

        if (TriggerRuntime == nullptr || !TriggerRuntime->bHasResolvedTime)
        {
            return false;
        }

        const int32 TriggerSecond =
            TriggerRuntime->ResolvedTime.ToSecondsFromMidnight();

        const int32 Delay = ChooseWindowSecond(
            Definition->MinimumDelaySeconds,
            Definition->PreferredDelaySeconds,
            Definition->MaximumDelaySeconds,
            Definition->HistoricalLock,
            EventId);

        ResolvedSecond = TriggerSecond + Delay;
        break;
    }

    default:
        return false;
    }

    Runtime->ResolvedTime =
        FTMOPTime::FromSecondsFromMidnight(ResolvedSecond);
    Runtime->bHasResolvedTime = true;
    return true;
}

bool UTMOPHistoricalEventSubsystem::TriggerEventAtTime(
    const FName EventId,
    const FTMOPTime& TriggerTime)
{
    FTMOPHistoricalEventRuntime* Runtime =
        RuntimeEvents.Find(EventId);

    if (Runtime == nullptr ||
        Runtime->State != ETMOPEventRuntimeState::Pending)
    {
        return false;
    }

    Runtime->State = ETMOPEventRuntimeState::Triggered;
    Runtime->TriggeredLoopNumber = CurrentLoopNumber;

    OnHistoricalEventTriggered.Broadcast(EventId, TriggerTime);

    for (const TPair<FName, FTMOPHistoricalEventDefinition>& Pair : Definitions)
    {
        if (Pair.Value.TimingMode == ETMOPEventTimingMode::Relative &&
            Pair.Value.TriggerEventId == EventId)
        {
            ResolveEventTime(Pair.Key);
        }
    }

    return true;
}

int32 UTMOPHistoricalEventSubsystem::ChooseWindowSecond(
    int32 EarliestSecond,
    int32 PreferredSecond,
    int32 LatestSecond,
    const ETMOPHistoricalLock LockMode,
    const FName EventId) const
{
    if (LatestSecond < EarliestSecond)
    {
        Swap(EarliestSecond, LatestSecond);
    }

    PreferredSecond = FMath::Clamp(
        PreferredSecond,
        EarliestSecond,
        LatestSecond);

    if (LockMode == ETMOPHistoricalLock::HardLocked)
    {
        return PreferredSecond;
    }

    if (LockMode == ETMOPHistoricalLock::SoftLocked)
    {
        const int32 HalfRange =
            FMath::Max(1, (LatestSecond - EarliestSecond) / 4);

        EarliestSecond = FMath::Max(
            EarliestSecond,
            PreferredSecond - HalfRange);

        LatestSecond = FMath::Min(
            LatestSecond,
            PreferredSecond + HalfRange);
    }

    const uint32 Seed =
        GetTypeHash(EventId) ^
        static_cast<uint32>(CurrentLoopNumber * 2654435761u);

    FRandomStream RandomStream(static_cast<int32>(Seed));

    return RandomStream.RandRange(EarliestSecond, LatestSecond);
}
