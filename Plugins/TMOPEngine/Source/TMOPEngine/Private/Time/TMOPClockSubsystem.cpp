#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationSettings.h"
#include "Time/TMOPLoopEndPolicy.h"
#include "Player/TMOPLocalSessionPolicy.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UTMOPClockSubsystem::UTMOPClockSubsystem()
{
    const UTMOPSimulationSettings* Settings = GetDefault<UTMOPSimulationSettings>();
    LoopStartSeconds = Settings->ScenarioStartTime.ToSecondsFromMidnight();
    LoopEndSeconds = Settings->ScenarioEndTime.ToSecondsFromMidnight();
    if (LoopEndSeconds <= LoopStartSeconds)
    {
        LoopStartSeconds = FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
        LoopEndSeconds = FTMOPTime(23, 45, 0).ToSecondsFromMidnight();
    }
    CurrentTimeSeconds = LoopStartSeconds;
    TimeScale = FMath::Clamp(Settings->DefaultTimeScale, 0.0f, 100.0f);
}

void UTMOPClockSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    CurrentTimeSeconds = LoopStartSeconds;
    FractionalSeconds = 0.0;
    LoopNumber = 1;
    bClockRunning = true;
    bAwaitingLoopDecision = false;

    TickerHandle = FTSTicker::GetCoreTicker().AddTicker(
        FTickerDelegate::CreateUObject(this, &UTMOPClockSubsystem::TickClock));
}

void UTMOPClockSubsystem::Deinitialize()
{
    if (TickerHandle.IsValid())
    {
        FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);
        TickerHandle.Reset();
    }

    Super::Deinitialize();
}

FTMOPTime UTMOPClockSubsystem::GetCurrentTime() const
{
    return FTMOPTime::FromSecondsFromMidnight(CurrentTimeSeconds);
}

FTMOPTime UTMOPClockSubsystem::GetLoopStartTime() const
{
    return FTMOPTime::FromSecondsFromMidnight(LoopStartSeconds);
}

FTMOPTime UTMOPClockSubsystem::GetLoopEndTime() const
{
    return FTMOPTime::FromSecondsFromMidnight(LoopEndSeconds);
}

void UTMOPClockSubsystem::StartClock()
{
    // Menus and other callers cannot accidentally run past the unresolved end state.
    if (!bAwaitingLoopDecision) bClockRunning = true;
}

void UTMOPClockSubsystem::PauseClock()
{
    bClockRunning = false;
}

void UTMOPClockSubsystem::RestartLoop()
{
    ++LoopNumber;
    CurrentTimeSeconds = LoopStartSeconds;
    FractionalSeconds = 0.0;
    bAwaitingLoopDecision = false;
    ReleasePause(this, TEXT("LoopEnd"));

    const FTMOPTime RestartTime = GetCurrentTime();
    OnLoopRestarted.Broadcast(LoopNumber, RestartTime);
    OnSecondChanged.Broadcast(RestartTime);
}

void UTMOPClockSubsystem::SetCurrentTime(const FTMOPTime NewTime)
{
    CurrentTimeSeconds = NewTime.ToSecondsFromMidnight();
    FractionalSeconds = 0.0;
    if (TMOPLoopEndPolicy::HasReachedEnd(CurrentTimeSeconds, LoopEndSeconds))
    {
        ReachLoopEnd();
        return;
    }
    bAwaitingLoopDecision = false;
    ReleasePause(this, TEXT("LoopEnd"));
    OnSecondChanged.Broadcast(GetCurrentTime());
}

bool UTMOPClockSubsystem::SetLoopRange(
    const FTMOPTime NewStartTime,
    const FTMOPTime NewEndTime)
{
    const int32 NewStartSeconds = NewStartTime.ToSecondsFromMidnight();
    const int32 NewEndSeconds = NewEndTime.ToSecondsFromMidnight();

    if (NewEndSeconds <= NewStartSeconds)
    {
        return false;
    }

    LoopStartSeconds = NewStartSeconds;
    LoopEndSeconds = NewEndSeconds;
    CurrentTimeSeconds = FMath::Clamp(
        CurrentTimeSeconds,
        LoopStartSeconds,
        LoopEndSeconds);
    bAwaitingLoopDecision = false;

    ReleasePause(this, TEXT("LoopEnd"));
    return true;
}

void UTMOPClockSubsystem::SetTimeScale(const float NewTimeScale)
{
    TimeScale = FMath::Clamp(NewTimeScale, 0.0f, 100.0f);
}

bool UTMOPClockSubsystem::TickClock(const float DeltaSeconds)
{
    UpdatePauseState();
    if (!IsClockRunning() || TimeScale <= 0.0f)
    {
        return true;
    }

    FractionalSeconds += static_cast<double>(DeltaSeconds) * TimeScale;

    while (IsClockRunning() && FractionalSeconds >= 1.0)
    {
        FractionalSeconds -= 1.0;
        AdvanceOneSecond();
    }

    return true;
}

void UTMOPClockSubsystem::AdvanceOneSecond()
{
    ++CurrentTimeSeconds;

    if (TMOPLoopEndPolicy::HasReachedEnd(CurrentTimeSeconds, LoopEndSeconds))
    {
        ReachLoopEnd();
        return;
    }

    OnSecondChanged.Broadcast(GetCurrentTime());
}

void UTMOPClockSubsystem::ReachLoopEnd()
{
    CurrentTimeSeconds = LoopEndSeconds;
    FractionalSeconds = 0.0;
    bClockRunning = false;
    if (bAwaitingLoopDecision) return;
    bAwaitingLoopDecision = true;
    RequestPause(this, TEXT("LoopEnd"));

    const FTMOPTime EndTime = GetCurrentTime();
    OnSecondChanged.Broadcast(EndTime);
    OnLoopEnded.Broadcast(LoopNumber, EndTime);
}

bool UTMOPClockSubsystem::HasPauseRequests() const
{
    return PauseRequests.ContainsByPredicate([](const FPauseRequest& Request)
    { return Request.Owner.IsValid(); });
}

bool UTMOPClockSubsystem::IsClockRunning() const
{
    // CoreTicker continues during an Unreal world pause; explicitly honour it.
    return TMOPLocalSessionPolicy::CanRunClock(bClockRunning, bAwaitingLoopDecision,
        HasPauseRequests(), GetWorld() && GetWorld()->IsPaused());
}

void UTMOPClockSubsystem::RequestPause(UObject* Owner, FName Reason, bool bPauseWorld)
{
    if (!IsValid(Owner) || Reason.IsNone()) return;
    FPauseRequest* Existing = PauseRequests.FindByPredicate(
        [Owner, Reason](const FPauseRequest& Request)
        { return Request.Owner.Get() == Owner && Request.Reason == Reason; });
    if (Existing) Existing->bPauseWorld = bPauseWorld;
    else PauseRequests.Add({Owner, Reason, bPauseWorld});
    UpdatePauseState();
}

void UTMOPClockSubsystem::ReleasePause(UObject* Owner, FName Reason)
{
    PauseRequests.RemoveAll([Owner, Reason](const FPauseRequest& Request)
    { return Request.Owner.Get() == Owner && Request.Reason == Reason; });
    UpdatePauseState();
}

void UTMOPClockSubsystem::ReleaseAllPauses(UObject* Owner)
{
    PauseRequests.RemoveAll([Owner](const FPauseRequest& Request)
    { return Request.Owner.Get() == Owner; });
    UpdatePauseState();
}

void UTMOPClockSubsystem::UpdatePauseState()
{
    PauseRequests.RemoveAll([](const FPauseRequest& Request)
    { return !Request.Owner.IsValid(); });
    UWorld* World = GetWorld();
    if (PausedWorld.Get() != World)
    {
        PausedWorld = World;
        bOwnsWorldPause = false;
    }
    if (!World || !World->IsGameWorld()) return;
    const bool bWantPause = PauseRequests.ContainsByPredicate(
        [](const FPauseRequest& Request) { return Request.bPauseWorld; });
    if (bWantPause && !World->IsPaused())
        bOwnsWorldPause = UGameplayStatics::SetGamePaused(World, true);
    else if (!bWantPause && bOwnsWorldPause)
    {
        UGameplayStatics::SetGamePaused(World, false);
        bOwnsWorldPause = false;
    }
}
