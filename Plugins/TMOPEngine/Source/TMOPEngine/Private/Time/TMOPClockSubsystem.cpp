#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationSettings.h"

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
    bClockRunning = true;
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

    const FTMOPTime RestartTime = GetCurrentTime();
    OnLoopRestarted.Broadcast(LoopNumber, RestartTime);
    OnSecondChanged.Broadcast(RestartTime);
}

void UTMOPClockSubsystem::SetCurrentTime(const FTMOPTime NewTime)
{
    CurrentTimeSeconds = NewTime.ToSecondsFromMidnight();
    FractionalSeconds = 0.0;
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

    return true;
}

void UTMOPClockSubsystem::SetTimeScale(const float NewTimeScale)
{
    TimeScale = FMath::Clamp(NewTimeScale, 0.0f, 100.0f);
}

bool UTMOPClockSubsystem::TickClock(const float DeltaSeconds)
{
    if (!bClockRunning || TimeScale <= 0.0f)
    {
        return true;
    }

    FractionalSeconds += static_cast<double>(DeltaSeconds) * TimeScale;

    while (FractionalSeconds >= 1.0)
    {
        FractionalSeconds -= 1.0;
        AdvanceOneSecond();
    }

    return true;
}

void UTMOPClockSubsystem::AdvanceOneSecond()
{
    ++CurrentTimeSeconds;

    if (CurrentTimeSeconds >= LoopEndSeconds)
    {
        RestartLoop();
        return;
    }

    OnSecondChanged.Broadcast(GetCurrentTime());
}
