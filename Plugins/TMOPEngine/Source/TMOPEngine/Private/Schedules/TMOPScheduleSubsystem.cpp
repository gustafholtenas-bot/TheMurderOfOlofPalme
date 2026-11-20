#include "Schedules/TMOPScheduleSubsystem.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "ProfilingDebugging/CpuProfilerTrace.h"
#include "Misc/Crc.h"

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
    DueEntries.Reset();
    RelativeEventEntries.Reset();
    RegisteredExecutors.Reset();
    ExecutorsByAgentId.Reset();
    MissingExecutorIds.Reset();

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

    // A replacement must discard old runtime rows and event dependencies.
    if (Schedules.Contains(Schedule.AgentId))
        RemoveAgentSchedule(Schedule.AgentId);

    Schedules.Add(Schedule.AgentId, Schedule);

    for (const FTMOPScheduleEntry& Entry : Schedule.Entries)
    {
        FTMOPScheduleEntryRuntime Runtime;
        Runtime.EntryId = Entry.EntryId;

        RuntimeEntries.Add(
            MakeRuntimeKey(Schedule.AgentId, Entry.EntryId),
            Runtime);
        if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
            RelativeEventEntries.FindOrAdd(Entry.TriggerEventId).Add(
                FEntryRef{Schedule.AgentId, Entry.EntryId});
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
        RuntimeEntries.Remove(MakeRuntimeKey(AgentId, Entry.EntryId));
        if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
        {
            const FName EntryId = Entry.EntryId;
            if (TArray<FEntryRef>* References =
                RelativeEventEntries.Find(Entry.TriggerEventId))
            {
                References->RemoveAll([AgentId, EntryId](const FEntryRef& Ref)
                {
                    return Ref.AgentId == AgentId && Ref.EntryId == EntryId;
                });
                if (References->IsEmpty())
                    RelativeEventEntries.Remove(Entry.TriggerEventId);
            }
        }
    }

    // Already queued references are safely discarded when their time arrives.
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
    DueEntries.Reset();

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

void UTMOPScheduleSubsystem::RegisterActionExecutor(
    UTMOPActionExecutorComponent* Executor)
{
    if (!IsValid(Executor)) return;
    RegisteredExecutors.AddUnique(TWeakObjectPtr<UTMOPActionExecutorComponent>(Executor));
    bExecutorIndexDirty = true;
    MissingExecutorIds.Reset();
}

void UTMOPScheduleSubsystem::UnregisterActionExecutor(
    UTMOPActionExecutorComponent* Executor)
{
    RegisteredExecutors.RemoveAll([Executor](
        const TWeakObjectPtr<UTMOPActionExecutorComponent>& Candidate)
    {
        return !Candidate.IsValid() || Candidate.Get() == Executor;
    });
    bExecutorIndexDirty = true;
    MissingExecutorIds.Reset();
}

void UTMOPScheduleSubsystem::RebuildExecutorIndex()
{
    ExecutorsByAgentId.Reset();
    RegisteredExecutors.RemoveAll([](
        const TWeakObjectPtr<UTMOPActionExecutorComponent>& Candidate)
    {
        return !Candidate.IsValid();
    });
    for (const TWeakObjectPtr<UTMOPActionExecutorComponent>& Weak : RegisteredExecutors)
    {
        UTMOPActionExecutorComponent* Executor = Weak.Get();
        if (!IsValid(Executor)) continue;
        const FName AgentId = Executor->GetOwnerEntityId();
        if (!AgentId.IsNone())
            ExecutorsByAgentId.FindOrAdd(AgentId).Add(Weak);
    }
    bExecutorIndexDirty = false;
    MissingExecutorIds.Reset();
}

void UTMOPScheduleSubsystem::EnqueueResolvedEntry(
    const FName AgentId,
    const FName EntryId,
    const FTMOPScheduleEntryRuntime& Runtime)
{
    if (Runtime.State != ETMOPScheduleEntryState::Pending ||
        !Runtime.bHasResolvedTime)
        return;

    FDueEntry Due;
    Due.Ref.AgentId = AgentId;
    Due.Ref.EntryId = EntryId;
    Due.Second = Runtime.ResolvedTime.ToSecondsFromMidnight();
    DueEntries.HeapPush(Due);
}

void UTMOPScheduleSubsystem::DispatchReadyEntry(
    const FName AgentId,
    const FTMOPScheduleEntry& Entry,
    const FTMOPTime TriggerTime)
{
    if (bExecutorIndexDirty) RebuildExecutorIndex();

    // Entity IDs may be assigned after the component's BeginPlay.
    if (!ExecutorsByAgentId.Contains(AgentId) &&
        !MissingExecutorIds.Contains(AgentId))
    {
        RebuildExecutorIndex();
        if (!ExecutorsByAgentId.Contains(AgentId))
            MissingExecutorIds.Add(AgentId);
    }

    if (const TArray<TWeakObjectPtr<UTMOPActionExecutorComponent>>* Matches =
        ExecutorsByAgentId.Find(AgentId))
    {
        const auto Recipients = *Matches;
        for (const TWeakObjectPtr<UTMOPActionExecutorComponent>& Weak : Recipients)
            if (UTMOPActionExecutorComponent* Executor = Weak.Get())
                if (IsValid(Executor))
                    Executor->HandleScheduleEntryReady(AgentId, Entry, TriggerTime);
    }

    // Preserve the public Blueprint event; native NPC receivers are targeted.
    OnScheduleEntryReady.Broadcast(AgentId, Entry, TriggerTime);
}

void UTMOPScheduleSubsystem::HandleSecondChanged(
    const FTMOPTime NewTime)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(TMOP_ScheduleDispatch);

    // Recorded world playback owns the actor state. A seek must not replay
    // all preceding live actions in the target frame.
    if (const UGameInstance* GameInstance = GetGameInstance())
        if (const UTMOPClockSubsystem* Clock =
            GameInstance->GetSubsystem<UTMOPClockSubsystem>())
            if (Clock->bAuthoritativePlayback) return;

    const int32 CurrentSecond = NewTime.ToSecondsFromMidnight();

    // Only wake events that have become due. No scan of every NPC's schedule.
    while (!DueEntries.IsEmpty() &&
        DueEntries.HeapTop().Second <= CurrentSecond)
    {
        FDueEntry Due;
        DueEntries.HeapPop(Due, EAllowShrinking::No);

        FTMOPScheduleEntryRuntime* Runtime =
            RuntimeEntries.Find(MakeRuntimeKey(Due.Ref.AgentId, Due.Ref.EntryId));
        if (Runtime == nullptr ||
            Runtime->State != ETMOPScheduleEntryState::Pending ||
            !Runtime->bHasResolvedTime ||
            Runtime->ResolvedTime.ToSecondsFromMidnight() != Due.Second)
            continue; // Removed, replaced, cancelled or rescheduled entry.

        const FTMOPAgentSchedule* Schedule = Schedules.Find(Due.Ref.AgentId);
        if (Schedule == nullptr) continue;
        const FTMOPScheduleEntry* Entry = Schedule->Entries.FindByPredicate(
            [&Due](const FTMOPScheduleEntry& Candidate)
            {
                return Candidate.EntryId == Due.Ref.EntryId;
            });
        if (Entry == nullptr) continue;

        // Set the state first, since callbacks can trigger more events.
        Runtime->State = ETMOPScheduleEntryState::Ready;
        const FTMOPScheduleEntry EntryCopy = *Entry;
        DispatchReadyEntry(Due.Ref.AgentId, EntryCopy, NewTime);
    }
}

void UTMOPScheduleSubsystem::HandleHistoricalEventTriggered(
    const FName EventId,
    const FTMOPTime TriggerTime)
{
    TriggeredEventTimes.Add(EventId, TriggerTime);

    // Only the entries referencing this event need their time resolved.
    const TArray<FEntryRef>* Waiting = RelativeEventEntries.Find(EventId);
    if (Waiting == nullptr) return;
    const TArray<FEntryRef> References = *Waiting; // Safe if callbacks replace schedules.

    for (const FEntryRef& Ref : References)
    {
        const FTMOPAgentSchedule* Schedule = Schedules.Find(Ref.AgentId);
        if (Schedule == nullptr) continue;
        const FTMOPScheduleEntry* Entry = Schedule->Entries.FindByPredicate(
            [&Ref](const FTMOPScheduleEntry& Candidate)
            {
                return Candidate.EntryId == Ref.EntryId;
            });
        if (Entry == nullptr ||
            Entry->TimingMode != ETMOPEventTimingMode::Relative ||
            Entry->TriggerEventId != EventId)
            continue;

        FTMOPScheduleEntryRuntime* Runtime =
            RuntimeEntries.Find(MakeRuntimeKey(Ref.AgentId, Ref.EntryId));
        if (Runtime != nullptr &&
            Runtime->State == ETMOPScheduleEntryState::Pending &&
            ResolveEntryTime(Ref.AgentId, *Entry, *Runtime))
            EnqueueResolvedEntry(Ref.AgentId, Ref.EntryId, *Runtime);
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
            if (ResolveEntryTime(AgentId, Entry, *Runtime))
                EnqueueResolvedEntry(AgentId, Entry.EntryId, *Runtime);
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
        FCrc::StrCrc32(*AgentId.ToString()) ^
        FCrc::StrCrc32(*EntryId.ToString()) ^
        static_cast<uint32>(2654435761u);

    FRandomStream RandomStream(static_cast<int32>(Seed));
    return RandomStream.RandRange(EarliestSecond, LatestSecond);
}

FString UTMOPScheduleSubsystem::MakeRuntimeKey(
    const FName AgentId,
    const FName EntryId) const
{
    return AgentId.ToString() + TEXT("::") + EntryId.ToString();
}
