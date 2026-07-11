#include "Venues/TMOPGrandFilmDirector.h"

#include "EngineUtils.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Venues/TMOPGrandFilmBehaviorComponent.h"

ATMOPGrandFilmDirector::ATMOPGrandFilmDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPGrandFilmDirector::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPClockSubsystem* Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 LoopNumber = Clock != nullptr ? Clock->GetLoopNumber() : 1;
    ResolveFilmTimes(LoopNumber);
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.AddDynamic(this, &ATMOPGrandFilmDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.AddDynamic(this, &ATMOPGrandFilmDirector::HandleLoopRestarted);
        EvaluateAtTime(Clock->GetCurrentTime());
    }
}

void ATMOPGrandFilmDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPClockSubsystem* Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.RemoveDynamic(this, &ATMOPGrandFilmDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.RemoveDynamic(this, &ATMOPGrandFilmDirector::HandleLoopRestarted);
    }
    Super::EndPlay(EndPlayReason);
}

bool ATMOPGrandFilmDirector::ResolveFilmTimes(const int32 LoopNumber)
{
    const int32 Earliest = EarliestFilmEnd.ToSecondsFromMidnight();
    const int32 Latest = LatestFilmEnd.ToSecondsFromMidnight();
    if (Latest < Earliest)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP Grand film end window is invalid."));
        return false;
    }
    FRandomStream Random(SimulationSeed + FMath::Max(1, LoopNumber));
    const int32 EndSeconds = Random.RandRange(Earliest, Latest);
    ResolvedFilmEnd = FTMOPTime::FromSecondsFromMidnight(EndSeconds);
    ResolvedCreditsStart = FTMOPTime::FromSecondsFromMidnight(
        EndSeconds - FMath::Max(0, CreditsDurationSeconds));
    bCreditsTriggered = false;
    bFilmEndTriggered = false;
    UE_LOG(LogTemp, Log, TEXT("TMOP Grand credits %s, film end %s."),
        *ResolvedCreditsStart.ToDisplayString(), *ResolvedFilmEnd.ToDisplayString());
    return true;
}

void ATMOPGrandFilmDirector::EvaluateAtTime(const FTMOPTime CurrentTime)
{
    const int32 Now = CurrentTime.ToSecondsFromMidnight();
    if (!bCreditsTriggered && Now >= ResolvedCreditsStart.ToSecondsFromMidnight())
    {
        bCreditsTriggered = true;
        TriggerStandingGroup(false);
        OnCreditsStarted.Broadcast(ResolvedCreditsStart);
    }
    if (!bFilmEndTriggered && Now >= ResolvedFilmEnd.ToSecondsFromMidnight())
    {
        bFilmEndTriggered = true;
        TriggerStandingGroup(true);
        OnFilmEnded.Broadcast(ResolvedFilmEnd);
    }
}

void ATMOPGrandFilmDirector::HandleSecondChanged(const FTMOPTime NewTime)
{
    EvaluateAtTime(NewTime);
}

void ATMOPGrandFilmDirector::HandleLoopRestarted(const int32 NewLoopNumber, const FTMOPTime RestartTime)
{
    ResetBehaviors();
    if (bResolveAgainOnLoopRestart) ResolveFilmTimes(NewLoopNumber);
    else { bCreditsTriggered = false; bFilmEndTriggered = false; }
    EvaluateAtTime(RestartTime);
}

void ATMOPGrandFilmDirector::TriggerStandingGroup(const bool bAtFilmEnd)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    int32 Stood = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPGrandFilmBehaviorComponent*> Behaviors;
        It->GetComponents<UTMOPGrandFilmBehaviorComponent>(Behaviors);
        for (UTMOPGrandFilmBehaviorComponent* Behavior : Behaviors)
        {
            if (!IsValid(Behavior)) continue;
            bool bPersonStandsAtFilmEnd = Behavior->bStandAtFilmEnd;
            const ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(Behavior->GetOwner());
            const FName EntityId = Agent != nullptr && Agent->EntityIdentity != nullptr
                ? Agent->EntityIdentity->EntityId : NAME_None;
            for (const FTMOPGrandStandingRule& Rule : StandingRules)
            {
                if (!Rule.EntityId.IsNone() && Rule.EntityId == EntityId)
                {
                    bPersonStandsAtFilmEnd = Rule.bStandAtFilmEnd;
                    break;
                }
            }
            if (bPersonStandsAtFilmEnd == bAtFilmEnd)
                Stood += Behavior->StandFromAssignedSeat() ? 1 : 0;
        }
    }
    UE_LOG(LogTemp, Log, TEXT("TMOP Grand %s standing group: %d."),
        bAtFilmEnd ? TEXT("film-end") : TEXT("credits"), Stood);
}

void ATMOPGrandFilmDirector::ResetBehaviors()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPGrandFilmBehaviorComponent*> Behaviors;
        It->GetComponents<UTMOPGrandFilmBehaviorComponent>(Behaviors);
        for (UTMOPGrandFilmBehaviorComponent* Behavior : Behaviors)
            if (IsValid(Behavior)) Behavior->ResetForLoop();
    }
}
