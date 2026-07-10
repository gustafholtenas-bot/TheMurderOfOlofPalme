#include "Schedules/TMOPScheduleSubsystem.h"

#include "Engine/GameInstance.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Time/TMOPClockSubsystem.h"

void UTMOPScheduleSubsystem::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    Collection.InitializeDependency<UTMOPClockSubsystem>();
    Collection.InitializeDependency<UTMOPHistoricalEventSubsystem>();

    if (UTMOPClockSubsystem* Clock =
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
    {
        Clock->OnSecondChanged.AddDynamic(
            this,
            &UTMOPScheduleSubsystem::HandleSecondChanged);

        Clock->OnLoopRestarted.AddDynamic(
            this,
            &UTMOPScheduleSubsystem::HandleLoopRestarted);
    }

    if (UTMOPHistoricalEventSubsystem* Events =
        GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
    {
        Events->OnHistoricalEventTriggered.AddDynamic(
            this,
            &UTMOPScheduleSubsystem::HandleHistoricalEventTriggered);
    }
}

void UTMOPScheduleSubsystem::Deinitialize()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
        {
            Clock->OnSecondChanged.RemoveDynamic(
                this,
                &UTMOPScheduleSubsystem::HandleSecondChanged);

            Clock->OnLoopRestarted.RemoveDynamic(
                this,
                &UTMOPScheduleSubsystem::HandleLoopRestarted);
        }

        if (UTMOPHistoricalEventSubsystem* Events =
            GameInstance->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        {
            Events->OnHistoricalEventTriggered.RemoveDynamic(
                this,
                &UTMOPScheduleSubsystem::HandleHistoricalEventTriggered);
        }
    }

    Schedules.Reset();
    RuntimeEntries.Reset();
    TriggeredEventTimes.Reset();

    Super::Deinitialize();
}

bool UTMOPScheduleSubsystem::RegisterAgentSchedule(
    const FTMOPAgentSchedule& Schedule)
{
    if (Schedule.AgentId.IsNone())
    {
        return false;
    }

    TSet<FName> SeenEntryIds;

    for (const FTMOPScheduleEntry& Entry : Schedule.Entries)
    {
        if (Entry.EntryId.IsNone() ||
            SeenEntryIds.Contains(Entry.EntryId))
        {
            return false;
        }

        SeenEntryIds.Add(Entry.EntryId);
    }

    Schedules.Add(Schedule.AgentId, Schedule);

    for (const FTMOPScheduleEntry& Entry : Schedule.Entries)
    {
        FTMOPScheduleEntryRuntime Runtime;
        Runtime.EntryId = Entry.EntryId;

        RuntimeEntries.Add(
            MakeRuntimeKey(Schedule.AgentId, Entry.EntryId),
            Runtime);
    }

    ResolveSchedule(Schedule.AgentId);
    return true;
}

bool UTMOPScheduleSubsystem::RemoveAgentSchedule(
    const FName AgentId)
{
    const FTMOPAgentSchedule* Schedule = Schedules.Find(AgentId);
    if (Schedule == nullptr)
    {
        return false;
    }

    for (const FTMOPScheduleEntry& Entry : Schedule->Entries)
    {
        RuntimeEntries.Remove(
            MakeRuntimeKey(AgentId, Entry.EntryId));
    }

    return Schedules.Remove(AgentId) > 0;
}

bool UTMOPScheduleSubsystem::HasAgentSchedule(
    const FName AgentId) const
{
    return Schedules.Contains(AgentId);
}

bool UTMOPScheduleSubsystem::TryGetAgentSchedule(
    const FName AgentId,
    FTMOPAgentSchedule& OutSchedule) const
{
    if (const FTMOPAgentSchedule* Found = Schedules.Find(AgentId))
    {
        OutSchedule = *Found;
        return true;
    }

    OutSchedule = FTMOPAgentSchedule();
    return false;
}

bool UTMOPScheduleSubsystem::TryGetEntryRuntime(
    const FName AgentId,
    const FName EntryId,
    FTMOPScheduleEntryRuntime& OutRuntime) const
{
    if (const FTMOPScheduleEntryRuntime* Found =
        RuntimeEntries.Find(MakeRuntimeKey(AgentId, EntryId)))
    {
        OutRuntime = *Found;
        return true;
    }

    OutRuntime = FTMOPScheduleEntryRuntime();
    return false;
}

bool UTMOPScheduleSubsystem::MarkEntryExecuted(
    const FName AgentId,
    const FName EntryId)
{
    FTMOPScheduleEntryRuntime* Runtime =
        RuntimeEntries.Find(MakeRuntimeKey(AgentId, EntryId));

    if (Runtime == nullptr ||
        Runtime->State == ETMOPScheduleEntryState::Cancelled)
    {
        return false;
    }

    Runtime->State = ETMOPScheduleEntryState::Executed;
    Runtime->ExecutedLoopNumber = CurrentLoopNumber;
    return true;
}

bool UTMOPScheduleSubsystem::CancelEntry(
    const FName AgentId,
    const FName EntryId)
{
    FTMOPScheduleEntryRuntime* Runtime =
        RuntimeEntries.Find(MakeRuntimeKey(AgentId, EntryId));

    if (Runtime == nullptr ||
        Runtime->State == ETMOPScheduleEntryState::Executed)
    {
        return false;
    }

    Runtime->State = ETMOPScheduleEntryState::Cancelled;
    return true;
}

void UTMOPScheduleSubsystem::ResetSchedulesForLoop(
    const int32 LoopNumber)
{
    CurrentLoopNumber = LoopNumber;
    TriggeredEventTimes.Reset();

    for (TPair<FString, FTMOPScheduleEntryRuntime>& Pair : RuntimeEntries)
    {
        Pair.Value.State = ETMOPScheduleEntryState::Pending;
        Pair.Value.bHasResolvedTime = false;
        Pair.Value.ExecutedLoopNumber = 0;
    }

    for (const TPair<FName, FTMOPAgentSchedule>& Pair : Schedules)
    {
        ResolveSchedule(Pair.Key);
    }

    OnSchedulesReset.Broadcast(LoopNumber);
}

TArray<FName> UTMOPScheduleSubsystem::GetScheduledAgentIds() const
{
    TArray<FName> AgentIds;
    Schedules.GetKeys(AgentIds);
    AgentIds.Sort(FNameLexicalLess());
    return AgentIds;
}

void UTMOPScheduleSubsystem::HandleSecondChanged(
    const FTMOPTime NewTime)
{
    const int32 CurrentSecond =
        NewTime.ToSecondsFromMidnight();

    for (const TPair<FName, FTMOPAgentSchedule>& SchedulePair : Schedules)
    {
        const FName AgentId = SchedulePair.Key;

        for (const FTMOPScheduleEntry& Entry :
            SchedulePair.Value.Entries)
        {
            FTMOPScheduleEntryRuntime* Runtime =
                RuntimeEntries.Find(
                    MakeRuntimeKey(AgentId, Entry.EntryId));

            if (Runtime == nullptr ||
                Runtime->State != ETMOPScheduleEntryState::Pending)
            {
                continue;
            }

            if (!Runtime->bHasResolvedTime)
            {
                ResolveEntryTime(AgentId, Entry, *Runtime);
            }

            if (Runtime->bHasResolvedTime &&
                CurrentSecond >=
                    Runtime->ResolvedTime.ToSecondsFromMidnight())
            {
                Runtime->State = ETMOPScheduleEntryState::Ready;

                OnScheduleEntryReady.Broadcast(
                    AgentId,
                    Entry,
                    NewTime);
            }
        }
    }
}

void UTMOPScheduleSubsystem::HandleHistoricalEventTriggered(
    const FName EventId,
    const FTMOPTime TriggerTime)
{
    TriggeredEventTimes.Add(EventId, TriggerTime);

    for (const TPair<FName, FTMOPAgentSchedule>& SchedulePair : Schedules)
    {
        const FName AgentId = SchedulePair.Key;

        for (const FTMOPScheduleEntry& Entry :
            SchedulePair.Value.Entries)
        {
            if (Entry.TimingMode == ETMOPEventTimingMode::Relative &&
                Entry.TriggerEventId == EventId)
            {
                if (FTMOPScheduleEntryRuntime* Runtime =
                    RuntimeEntries.Find(
                        MakeRuntimeKey(AgentId, Entry.EntryId)))
                {
                    ResolveEntryTime(AgentId, Entry, *Runtime);
                }
            }
        }
    }
}

void UTMOPScheduleSubsystem::HandleLoopRestarted(
    const int32 NewLoopNumber,
    const FTMOPTime RestartTime)
{
    ResetSchedulesForLoop(NewLoopNumber);
}

void UTMOPScheduleSubsystem::ResolveSchedule(
    const FName AgentId)
{
    const FTMOPAgentSchedule* Schedule =
        Schedules.Find(AgentId);

    if (Schedule == nullptr)
    {
        return;
    }

    for (const FTMOPScheduleEntry& Entry : Schedule->Entries)
    {
        if (FTMOPScheduleEntryRuntime* Runtime =
            RuntimeEntries.Find(
                MakeRuntimeKey(AgentId, Entry.EntryId)))
        {
            ResolveEntryTime(AgentId, Entry, *Runtime);
        }
    }
}

bool UTMOPScheduleSubsystem::ResolveEntryTime(
    const FName AgentId,
    const FTMOPScheduleEntry& Entry,
    FTMOPScheduleEntryRuntime& Runtime)
{
    int32 ResolvedSecond = 0;

    switch (Entry.TimingMode)
    {
    case ETMOPEventTimingMode::Absolute:
        ResolvedSecond =
            Entry.AbsoluteTime.ToSecondsFromMidnight();
        break;

    case ETMOPEventTimingMode::Window:
        ResolvedSecond = ChooseWindowSecond(
            Entry.EarliestTime.ToSecondsFromMidnight(),
            Entry.PreferredTime.ToSecondsFromMidnight(),
            Entry.LatestTime.ToSecondsFromMidnight(),
            Entry.HistoricalLock,
            AgentId,
            Entry.EntryId);
        break;

    case ETMOPEventTimingMode::Relative:
    {
        const FTMOPTime* TriggerTime =
            TriggeredEventTimes.Find(Entry.TriggerEventId);

        if (TriggerTime == nullptr)
        {
            return false;
        }

        const int32 Delay = ChooseWindowSecond(
            Entry.MinimumDelaySeconds,
            Entry.PreferredDelaySeconds,
            Entry.MaximumDelaySeconds,
            Entry.HistoricalLock,
            AgentId,
            Entry.EntryId);

        ResolvedSecond =
            TriggerTime->ToSecondsFromMidnight() + Delay;
        break;
    }

    default:
        return false;
    }

    Runtime.ResolvedTime =
        FTMOPTime::FromSecondsFromMidnight(ResolvedSecond);
    Runtime.bHasResolvedTime = true;
    return true;
}

int32 UTMOPScheduleSubsystem::ChooseWindowSecond(
    int32 EarliestSecond,
    int32 PreferredSecond,
    int32 LatestSecond,
    const ETMOPHistoricalLock LockMode,
    const FName AgentId,
    const FName EntryId) const
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
        GetTypeHash(AgentId) ^
        GetTypeHash(EntryId) ^
        static_cast<uint32>(CurrentLoopNumber * 2654435761u);

    FRandomStream RandomStream(static_cast<int32>(Seed));
    return RandomStream.RandRange(EarliestSecond, LatestSecond);
}

FString UTMOPScheduleSubsystem::MakeRuntimeKey(
    const FName AgentId,
    const FName EntryId) const
{
    return AgentId.ToString() + TEXT("::") + EntryId.ToString();
}
