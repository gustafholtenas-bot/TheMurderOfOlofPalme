#include "Venues/TMOPGrandFilmDirector.h"

#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "MediaPlayer.h"
#include "MediaSoundComponent.h"
#include "MediaSource.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Venues/TMOPGrandFilmBehaviorComponent.h"

ATMOPGrandFilmDirector::ATMOPGrandFilmDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;
    FilmSoundComponent = CreateDefaultSubobject<UMediaSoundComponent>(
        TEXT("FilmSound"));
    FilmSoundComponent->SetupAttachment(SceneRoot);
}

void ATMOPGrandFilmDirector::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetGameInstance();
    Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 LoopNumber = Clock != nullptr ? Clock->GetLoopNumber() : 1;
    ResolveFilmTimes(LoopNumber);
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.AddDynamic(this, &ATMOPGrandFilmDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.AddDynamic(this, &ATMOPGrandFilmDirector::HandleLoopRestarted);
        EvaluateAtTime(Clock->GetCurrentTime());
    }
    if (FilmSoundComponent != nullptr)
    {
        FilmSoundComponent->SetMediaPlayer(FilmMediaPlayer);
        FilmSoundComponent->SetVolumeMultiplier(FilmVolume);
    }
    if (FilmMediaPlayer != nullptr)
    {
        FilmMediaPlayer->OnMediaOpened.AddUniqueDynamic(
            this, &ATMOPGrandFilmDirector::HandleMediaOpened);
        FilmMediaPlayer->OnMediaOpenFailed.AddUniqueDynamic(
            this, &ATMOPGrandFilmDirector::HandleMediaOpenFailed);
        FilmMediaPlayer->SetLooping(false);
    }
    if (bAutoOpenFilm) OpenFilm();
}

void ATMOPGrandFilmDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopFilm();
    if (FilmMediaPlayer != nullptr)
    {
        FilmMediaPlayer->OnMediaOpened.RemoveDynamic(
            this, &ATMOPGrandFilmDirector::HandleMediaOpened);
        FilmMediaPlayer->OnMediaOpenFailed.RemoveDynamic(
            this, &ATMOPGrandFilmDirector::HandleMediaOpenFailed);
    }
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.RemoveDynamic(this, &ATMOPGrandFilmDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.RemoveDynamic(this, &ATMOPGrandFilmDirector::HandleLoopRestarted);
    }
    Super::EndPlay(EndPlayReason);
}

void ATMOPGrandFilmDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    SynchronizeFilmToClock();
}

bool ATMOPGrandFilmDirector::OpenFilm()
{
    bFilmMediaOpen = false;
    LastSynchronizedClockSecond = INDEX_NONE;
    if (FilmMediaPlayer == nullptr || FilmMediaSource == nullptr)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP Grand film playback needs both Film Media Player and Film Media Source."));
        return false;
    }
    if (FilmSoundComponent != nullptr)
    {
        FilmSoundComponent->SetMediaPlayer(FilmMediaPlayer);
        FilmSoundComponent->SetVolumeMultiplier(FilmVolume);
    }
    FilmMediaPlayer->SetLooping(false);
    return FilmMediaPlayer->OpenSource(FilmMediaSource);
}

void ATMOPGrandFilmDirector::StopFilm()
{
    bFilmMediaOpen = false;
    LastSynchronizedClockSecond = INDEX_NONE;
    if (FilmMediaPlayer != nullptr) FilmMediaPlayer->Close();
}

double ATMOPGrandFilmDirector::ResolveMediaStartSecond() const
{
    if (bAlignMediaEndToResolvedFilmEnd && FilmMediaPlayer != nullptr)
    {
        const double Duration = FilmMediaPlayer->GetDuration().GetTotalSeconds();
        if (Duration > 0.0)
            return ResolvedFilmEnd.ToSecondsFromMidnight() - Duration;
    }
    return MediaStartGameTime.ToSecondsFromMidnight();
}

void ATMOPGrandFilmDirector::SynchronizeFilmToClock()
{
    if (!bFilmMediaOpen || FilmMediaPlayer == nullptr || Clock == nullptr)
        return;

    const int32 ClockSecond =
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    const double Now = ClockSecond;
    const double Start = ResolveMediaStartSecond();
    const double End = ResolvedFilmEnd.ToSecondsFromMidnight();
    const double Duration = FilmMediaPlayer->GetDuration().GetTotalSeconds();
    const bool bBeforeFilm = Now < Start;
    const bool bAfterFilm = Now >= End;
    double ExpectedMediaSecond = FMath::Max(0.0, Now - Start);
    if (Duration > 0.0)
        ExpectedMediaSecond = FMath::Min(ExpectedMediaSecond, Duration);

    // The TMOP clock has whole-second resolution. Seek only when its second
    // changes; seeking every render tick would repeatedly rewind the movie
    // within the same clock second.
    if (ClockSecond != LastSynchronizedClockSecond)
    {
        const double CurrentMediaSecond =
            FilmMediaPlayer->GetTime().GetTotalSeconds();
        if (FMath::Abs(CurrentMediaSecond - ExpectedMediaSecond) >
            FMath::Max(0.1f, ResyncThresholdSeconds))
        {
            FilmMediaPlayer->Seek(
                FTimespan::FromSeconds(ExpectedMediaSecond));
        }
        LastSynchronizedClockSecond = ClockSecond;
    }

    const float TimeScale = Clock->GetTimeScale();
    const bool bPause = bBeforeFilm || bAfterFilm ||
        !Clock->IsClockRunning() || TimeScale <= 0.0f ||
        TimeScale > MaximumRealtimePlaybackScale;
    if (bPause)
    {
        FilmMediaPlayer->Pause();
        return;
    }

    if (!FilmMediaPlayer->SetRate(TimeScale))
    {
        if (FMath::IsNearlyEqual(TimeScale, 1.0f))
            FilmMediaPlayer->Play();
        else
            FilmMediaPlayer->Pause();
    }
}

void ATMOPGrandFilmDirector::HandleMediaOpened(FString OpenedUrl)
{
    bFilmMediaOpen = true;
    LastSynchronizedClockSecond = INDEX_NONE;
    SynchronizeFilmToClock();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP Grand film media opened: %s"), *OpenedUrl);
}

void ATMOPGrandFilmDirector::HandleMediaOpenFailed(FString FailedUrl)
{
    bFilmMediaOpen = false;
    UE_LOG(LogTemp, Error,
        TEXT("TMOP Grand could not open film media: %s"), *FailedUrl);
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
    SynchronizeFilmToClock();
}

void ATMOPGrandFilmDirector::HandleLoopRestarted(const int32 NewLoopNumber, const FTMOPTime RestartTime)
{
    ResetBehaviors();
    if (bResolveAgainOnLoopRestart) ResolveFilmTimes(NewLoopNumber);
    else { bCreditsTriggered = false; bFilmEndTriggered = false; }
    EvaluateAtTime(RestartTime);
    SynchronizeFilmToClock();
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
