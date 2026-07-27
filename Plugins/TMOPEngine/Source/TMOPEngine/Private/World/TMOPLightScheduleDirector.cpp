#include "World/TMOPLightScheduleDirector.h"

#include "Components/LightComponent.h"
#include "EngineUtils.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Time/TMOPClockSubsystem.h"

ATMOPLightScheduleDirector::ATMOPLightScheduleDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPLightScheduleDirector::BeginPlay()
{
    Super::BeginPlay();
    RestartScheduleAtCurrentTime();
}

void ATMOPLightScheduleDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateTransitions(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!IsValid(Clock)) return;
    const int32 CurrentSecond =
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE &&
        CurrentSecond < LastEvaluatedSecond)
    {
        RestartScheduleAtCurrentTime();
        return;
    }
    if (CurrentSecond != LastEvaluatedSecond)
    {
        EvaluateSchedule(CurrentSecond, false);
        LastEvaluatedSecond = CurrentSecond;
    }
}

void ATMOPLightScheduleDirector::RestartScheduleAtCurrentTime()
{
    RestoreBaseline();
    NextEntryIndex = 0;
    LastResolvedEntrySecond = INDEX_NONE;
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = IsValid(Clock)
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    EvaluateSchedule(CurrentSecond, true);
    LastEvaluatedSecond = CurrentSecond;
}

void ATMOPLightScheduleDirector::EvaluateSchedule(
    const int32 CurrentSecond, const bool bCatchUp)
{
    while (ScheduledEntries.IsValidIndex(NextEntryIndex))
    {
        const FTMOPLightScheduleEntry& Entry =
            ScheduledEntries[NextEntryIndex];
        int32 ResolvedSecond = INDEX_NONE;
        if (!ResolveEntrySecond(Entry, ResolvedSecond) ||
            ResolvedSecond > CurrentSecond)
            break;
        ApplyEntry(Entry, bCatchUp || ResolvedSecond < CurrentSecond);
        LastResolvedEntrySecond = ResolvedSecond;
        ++NextEntryIndex;
    }
}

bool ATMOPLightScheduleDirector::ResolveEntrySecond(
    const FTMOPLightScheduleEntry& Entry, int32& OutSecond) const
{
    OutSecond = INDEX_NONE;
    if (Entry.TimingMode == ETMOPEventTimingMode::Absolute)
    {
        OutSecond = Entry.Time.ToSecondsFromMidnight();
        return true;
    }
    if (Entry.TimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
    {
        if (LastResolvedEntrySecond == INDEX_NONE) return false;
        OutSecond = LastResolvedEntrySecond + Entry.OffsetSeconds;
        return true;
    }
    if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
    {
        const UTMOPHistoricalEventSubsystem* Events =
            GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<
                UTMOPHistoricalEventSubsystem>() : nullptr;
        FTMOPHistoricalEventRuntime Runtime;
        if (!IsValid(Events) || Entry.SharedEventId.IsNone() ||
            !Events->TryGetEventRuntime(Entry.SharedEventId, Runtime) ||
            !Runtime.bHasResolvedTime)
            return false;
        OutSecond = Runtime.ResolvedTime.ToSecondsFromMidnight() +
            Entry.OffsetSeconds;
        return true;
    }
    return false;
}

void ATMOPLightScheduleDirector::ApplyEntry(
    const FTMOPLightScheduleEntry& Entry, const bool bCatchUp)
{
    TArray<ULightComponent*> Lights;
    GatherTargetLights(Entry, Lights);
    if (Lights.IsEmpty())
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP Light Schedule '%s': no target Light Components found."),
            *Entry.EntryId.ToString());
    for (ULightComponent* Light : Lights)
    {
        if (!IsValid(Light)) continue;
        OriginalIntensities.FindOrAdd(Light, Light->Intensity);
        OriginalVisibility.FindOrAdd(Light, Light->IsVisible());
        ActiveTransitions.RemoveAll(
            [Light](const FLightTransition& Transition)
            {
                return Transition.Light.Get() == Light;
            });

        bool bTurnOn = Entry.Action == ETMOPLightScheduleAction::TurnOn;
        bool bTurnOff = Entry.Action == ETMOPLightScheduleAction::TurnOff;
        if (Entry.Action == ETMOPLightScheduleAction::Toggle)
        {
            bTurnOn = !Light->IsVisible();
            bTurnOff = !bTurnOn;
        }
        const float Original = OriginalIntensities.FindRef(Light);
        float Target = Light->Intensity;
        if (bTurnOn)
            Target = Entry.bOverrideTurnOnIntensity
                ? Entry.TargetIntensity : Original;
        else if (bTurnOff)
            Target = 0.0f;
        else if (Entry.Action == ETMOPLightScheduleAction::SetIntensity)
            Target = Entry.TargetIntensity;

        const float Fade = bCatchUp ? 0.0f : Entry.FadeSeconds;
        if (Fade <= KINDA_SMALL_NUMBER)
        {
            if (bTurnOff)
            {
                Light->SetVisibility(false, true);
                Light->SetIntensity(0.0f);
            }
            else
            {
                Light->SetVisibility(true, true);
                Light->SetIntensity(Target);
            }
            continue;
        }

        if (!bTurnOff) Light->SetVisibility(true, true);
        FLightTransition& Transition = ActiveTransitions.AddDefaulted_GetRef();
        Transition.Light = Light;
        Transition.StartIntensity = Light->Intensity;
        Transition.TargetIntensity = Target;
        Transition.Duration = Fade;
        Transition.bTurnOffAtEnd = bTurnOff;
    }
}

void ATMOPLightScheduleDirector::GatherTargetLights(
    const FTMOPLightScheduleEntry& Entry,
    TArray<ULightComponent*>& OutLights)
{
    OutLights.Reset();
    auto AddActorLights = [&OutLights](AActor* Actor)
    {
        if (!IsValid(Actor)) return;
        TArray<ULightComponent*> Components;
        Actor->GetComponents<ULightComponent>(Components);
        for (ULightComponent* Component : Components)
            OutLights.AddUnique(Component);
    };

    for (const TSoftObjectPtr<AActor>& Target : Entry.TargetLightActors)
        AddActorLights(Target.Get());
    if (!Entry.TargetActorTag.IsNone() && GetWorld() != nullptr)
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
            if (It->ActorHasTag(Entry.TargetActorTag))
                AddActorLights(*It);
}

void ATMOPLightScheduleDirector::UpdateTransitions(
    const float DeltaSeconds)
{
    for (int32 Index = ActiveTransitions.Num() - 1; Index >= 0; --Index)
    {
        FLightTransition& Transition = ActiveTransitions[Index];
        ULightComponent* Light = Transition.Light.Get();
        if (!IsValid(Light))
        {
            ActiveTransitions.RemoveAtSwap(Index);
            continue;
        }
        Transition.Elapsed += DeltaSeconds;
        const float Alpha = FMath::Clamp(
            Transition.Elapsed / FMath::Max(
                KINDA_SMALL_NUMBER, Transition.Duration), 0.0f, 1.0f);
        Light->SetIntensity(FMath::Lerp(
            Transition.StartIntensity,
            Transition.TargetIntensity, Alpha));
        if (Alpha >= 1.0f)
        {
            if (Transition.bTurnOffAtEnd)
                Light->SetVisibility(false, true);
            ActiveTransitions.RemoveAtSwap(Index);
        }
    }
}

void ATMOPLightScheduleDirector::RestoreBaseline()
{
    ActiveTransitions.Reset();
    for (const TPair<TWeakObjectPtr<ULightComponent>, float>& Pair :
        OriginalIntensities)
        if (ULightComponent* Light = Pair.Key.Get())
        {
            Light->SetIntensity(Pair.Value);
            Light->SetVisibility(
                OriginalVisibility.FindRef(Pair.Key), true);
        }
}
