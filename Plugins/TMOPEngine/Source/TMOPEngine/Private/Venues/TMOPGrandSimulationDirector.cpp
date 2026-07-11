#include "Venues/TMOPGrandSimulationDirector.h"

#include "EngineUtils.h"
#include "Groups/TMOPGroupDirector.h"
#include "Time/TMOPClockSubsystem.h"
#include "TimerManager.h"
#include "Venues/TMOPGrandAudienceTestSpawner.h"
#include "Venues/TMOPGrandAuditoriumExitDirector.h"
#include "Venues/TMOPGrandFoyerDirector.h"
#include "Venues/TMOPGrandRowExitDirector.h"

ATMOPGrandSimulationDirector::ATMOPGrandSimulationDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPGrandSimulationDirector::BeginPlay()
{
    Super::BeginPlay();
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.AddDynamic(this, &ATMOPGrandSimulationDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.AddDynamic(this, &ATMOPGrandSimulationDirector::HandleLoopRestarted);
        CurrentLoopNumber = Clock->GetLoopNumber();
    }
    GetWorldTimerManager().SetTimerForNextTick(this, &ATMOPGrandSimulationDirector::InitializeSimulation);
}

void ATMOPGrandSimulationDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.RemoveDynamic(this, &ATMOPGrandSimulationDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.RemoveDynamic(this, &ATMOPGrandSimulationDirector::HandleLoopRestarted);
    }
    Super::EndPlay(EndPlayReason);
}

void ATMOPGrandSimulationDirector::InitializeSimulation()
{
    ResetFullGrandSimulation();
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr) EvaluateTimeline(Clock->GetCurrentTime());
}

void ATMOPGrandSimulationDirector::ResetFullGrandSimulation()
{
    if (bIsResetting || GetWorld() == nullptr) return;
    bIsResetting = true;

    for (TActorIterator<ATMOPGrandRowExitDirector> It(GetWorld()); It; ++It) It->ResetRowExits();
    for (TActorIterator<ATMOPGrandAuditoriumExitDirector> It(GetWorld()); It; ++It) It->ResetAuditoriumExits();
    for (TActorIterator<ATMOPGrandFoyerDirector> It(GetWorld()); It; ++It) It->ResetFoyer();
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It) It->ResetAllGroups();
    for (TActorIterator<ATMOPGrandAudienceTestSpawner> It(GetWorld()); It; ++It) It->ResetAudience();
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It) It->RecreateInitialGroups();

    ResolveTimeline(CurrentLoopNumber);
    LastEvaluatedSeconds = INDEX_NONE;
    bIsResetting = false;
}

bool ATMOPGrandSimulationDirector::ResolveTimeline(const int32 LoopNumber)
{
    ResolvedActions.Reset();
    for (int32 Index = 0; Index < TimelineActions.Num(); ++Index)
    {
        const FTMOPGrandTimelineAction& Action = TimelineActions[Index];
        int32 Seconds = Action.ExactTime.ToSecondsFromMidnight();
        if (!Action.bUseExactTime)
        {
            const int32 Earliest = Action.EarliestTime.ToSecondsFromMidnight();
            const int32 Latest = Action.LatestTime.ToSecondsFromMidnight();
            if (Latest < Earliest) return false;
            FRandomStream Random(SimulationSeed + LoopNumber * 1009 + Index * 97);
            Seconds = Random.RandRange(Earliest, Latest);
        }
        FTMOPGrandResolvedTimelineAction Resolved;
        Resolved.ActionId = Action.ActionId;
        Resolved.ResolvedTime = FTMOPTime::FromSecondsFromMidnight(Seconds);
        Resolved.SourceIndex = Index;
        ResolvedActions.Add(Resolved);
    }
    ResolvedActions.Sort([](const FTMOPGrandResolvedTimelineAction& A,
        const FTMOPGrandResolvedTimelineAction& B)
        { return A.ResolvedTime.ToSecondsFromMidnight() < B.ResolvedTime.ToSecondsFromMidnight(); });
    return true;
}

void ATMOPGrandSimulationDirector::EvaluateTimeline(const FTMOPTime CurrentTime)
{
    if (bIsResetting) return;
    const int32 Now = CurrentTime.ToSecondsFromMidnight();
    if (bResetWhenTimeMovesBackwards && LastEvaluatedSeconds != INDEX_NONE && Now < LastEvaluatedSeconds)
        ResetFullGrandSimulation();
    for (FTMOPGrandResolvedTimelineAction& Resolved : ResolvedActions)
    {
        if (Resolved.bExecuted || Resolved.SourceIndex == INDEX_NONE ||
            Resolved.ResolvedTime.ToSecondsFromMidnight() > Now) continue;
        Resolved.bExecuted = ExecuteAction(TimelineActions[Resolved.SourceIndex], Resolved.SourceIndex);
    }
    LastEvaluatedSeconds = Now;
}

void ATMOPGrandSimulationDirector::HandleSecondChanged(const FTMOPTime NewTime)
{
    EvaluateTimeline(NewTime);
}

void ATMOPGrandSimulationDirector::HandleLoopRestarted(const int32 NewLoopNumber,
    const FTMOPTime RestartTime)
{
    CurrentLoopNumber = NewLoopNumber;
    ResetFullGrandSimulation();
    EvaluateTimeline(RestartTime);
}

bool ATMOPGrandSimulationDirector::ExecuteAction(const FTMOPGrandTimelineAction& Action,
    const int32 ActionIndex)
{
    ATMOPGroupDirector* Groups = nullptr;
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It) { Groups = *It; break; }
    if (!IsValid(Groups)) return false;
    switch (Action.ActionType)
    {
    case ETMOPGrandTimelineActionType::CreateGroup:
        return Groups->CreateGroup(Action.GroupDefinition);
    case ETMOPGrandTimelineActionType::MergeGroups:
        return Groups->MergeGroups(Action.GroupId, Action.SourceGroupIds,
            Action.LeaderEntityId, Action.Formation, Action.FormationSpacing);
    case ETMOPGrandTimelineActionType::StartConversation:
        return Groups->StartConversation(Action.GroupId, Action.MinimumConversationSeconds,
            Action.MaximumConversationSeconds, SimulationSeed + CurrentLoopNumber * 1009 + ActionIndex);
    case ETMOPGrandTimelineActionType::EndConversation:
        return Groups->EndConversation(Action.GroupId);
    case ETMOPGrandTimelineActionType::MoveGroup:
        return Groups->MoveGroupToLocation(Action.GroupId, Action.TargetLocation, Action.AcceptanceRadius);
    case ETMOPGrandTimelineActionType::SplitGroup:
        return Groups->SplitGroup(Action.GroupId, Action.SplitDefinitions);
    case ETMOPGrandTimelineActionType::StopGroup:
        return Groups->StopGroup(Action.GroupId);
    default:
        return false;
    }
}

bool ATMOPGrandSimulationDirector::ValidateTimeline(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TSet<FName> ActionIds;
    for (const FTMOPGrandTimelineAction& Action : TimelineActions)
    {
        if (Action.ActionId.IsNone()) OutErrors.Add(TEXT("A timeline action has no ActionId."));
        else if (ActionIds.Contains(Action.ActionId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate ActionId '%s'."), *Action.ActionId.ToString()));
        ActionIds.Add(Action.ActionId);
        if (!Action.bUseExactTime &&
            Action.LatestTime.ToSecondsFromMidnight() < Action.EarliestTime.ToSecondsFromMidnight())
            OutErrors.Add(FString::Printf(TEXT("Action '%s' has an invalid time window."), *Action.ActionId.ToString()));
    }
    return OutErrors.IsEmpty();
}
