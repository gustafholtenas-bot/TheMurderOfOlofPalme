#include "World/TMOPAerialVehicleDirector.h"

#include "Components/SplineComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Time/TMOPClockSubsystem.h"

ATMOPAerialVehicleDirector::ATMOPAerialVehicleDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPAerialVehicleDirector::BeginPlay()
{
    Super::BeginPlay();
    RestartScheduleAtCurrentTime();
}

void ATMOPAerialVehicleDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    DestroyAllAircraft();
    Super::EndPlay(EndPlayReason);
}

void ATMOPAerialVehicleDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!IsValid(Clock)) return;

    const int32 CurrentSecond =
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    const float SimulationDelta = Clock->IsClockRunning()
        ? DeltaSeconds * Clock->GetTimeScale() : 0.0f;
    const int32 LargestExpectedStep =
        FMath::Max(2, FMath::CeilToInt(SimulationDelta) + 1);
    if (LastEvaluatedSecond != INDEX_NONE &&
        (CurrentSecond < LastEvaluatedSecond ||
         CurrentSecond - LastEvaluatedSecond > LargestExpectedStep))
    {
        RestartScheduleAtCurrentTime();
        return;
    }

    if (CurrentSecond != LastEvaluatedSecond)
    {
        EvaluateSchedule(CurrentSecond, false);
        LastEvaluatedSecond = CurrentSecond;
    }

    if (SimulationDelta > 0.0f)
        UpdateFlights(SimulationDelta);
}

void ATMOPAerialVehicleDirector::RestartScheduleAtCurrentTime()
{
    DestroyAllAircraft();
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

AActor* ATMOPAerialVehicleDirector::FindAircraft(
    const FName InstanceId) const
{
    const FActiveFlight* Flight = ActiveFlights.Find(InstanceId);
    return Flight != nullptr ? Flight->Aircraft.Get() : nullptr;
}

void ATMOPAerialVehicleDirector::EvaluateSchedule(
    const int32 CurrentSecond, const bool bCatchUp)
{
    while (ScheduledFlights.IsValidIndex(NextEntryIndex))
    {
        const FTMOPAerialScheduleEntry& Entry =
            ScheduledFlights[NextEntryIndex];
        int32 ResolvedSecond = INDEX_NONE;
        if (!ResolveEntrySecond(Entry, ResolvedSecond) ||
            ResolvedSecond > CurrentSecond)
            break;

        if (!ApplyEntry(
            Entry, ResolvedSecond,
            bCatchUp ? CurrentSecond : ResolvedSecond))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Aerial entry '%s' could not be applied (Instance '%s')."),
                *Entry.EntryId.ToString(), *Entry.InstanceId.ToString());
        }

        LastResolvedEntrySecond = ResolvedSecond;
        ++NextEntryIndex;
    }
}

bool ATMOPAerialVehicleDirector::ResolveEntrySecond(
    const FTMOPAerialScheduleEntry& Entry, int32& OutSecond) const
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

bool ATMOPAerialVehicleDirector::ApplyEntry(
    const FTMOPAerialScheduleEntry& Entry,
    const int32 ResolvedSecond,
    const int32 CurrentSecond)
{
    if (Entry.InstanceId.IsNone()) return false;
    if (Entry.Action == ETMOPAerialScheduleAction::Despawn)
    {
        DespawnFlight(Entry.InstanceId);
        return true;
    }
    return SpawnFlight(Entry, ResolvedSecond, CurrentSecond);
}

bool ATMOPAerialVehicleDirector::SpawnFlight(
    const FTMOPAerialScheduleEntry& Entry,
    const int32 ResolvedSecond,
    const int32 CurrentSecond)
{
    if (GetWorld() == nullptr)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Aerial '%s': no valid world."),
            *Entry.EntryId.ToString());
        return false;
    }
    if (!Entry.AircraftClass)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Aerial '%s': Aircraft Class is not assigned."),
            *Entry.EntryId.ToString());
        return false;
    }

    AActor* SplineOwner = Entry.SplineActor.Get();
    if (!IsValid(SplineOwner))
        SplineOwner = Entry.SplineActor.LoadSynchronous();
    if (!IsValid(SplineOwner))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Aerial '%s': Spline Actor is not assigned or could not be loaded."),
            *Entry.EntryId.ToString());
        return false;
    }
    USplineComponent* Spline = IsValid(SplineOwner)
        ? SplineOwner->FindComponentByClass<USplineComponent>() : nullptr;
    if (!IsValid(Spline))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Aerial '%s': actor '%s' has no Spline Component."),
            *Entry.EntryId.ToString(), *GetNameSafe(SplineOwner));
        return false;
    }
    if (Spline->GetSplineLength() <= KINDA_SMALL_NUMBER)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Aerial '%s': spline on '%s' has zero length."),
            *Entry.EntryId.ToString(), *GetNameSafe(SplineOwner));
        return false;
    }

    DespawnFlight(Entry.InstanceId);

    FActiveFlight Flight;
    Flight.Spline = Spline;
    Flight.SpeedCmPerSecond =
        FMath::Max(0.0f, Entry.SpeedKmh) * (100000.0f / 3600.0f);
    Flight.bReverseDirection = Entry.bReverseDirection;
    Flight.RotationOffset = Entry.RotationOffset;
    Flight.EndBehavior = Entry.EndBehavior;
    const float RequestedStartDistance =
        Entry.bReverseDirection && Entry.StartDistanceCm <= 0.0f
        ? Spline->GetSplineLength()
        : Entry.StartDistanceCm;
    Flight.DistanceCm = FMath::Clamp(
        RequestedStartDistance, 0.0f, Spline->GetSplineLength());

    const float CatchUpSeconds =
        static_cast<float>(FMath::Max(0, CurrentSecond - ResolvedSecond));
    Flight.DistanceCm += (Flight.bReverseDirection ? -1.0f : 1.0f) *
        Flight.SpeedCmPerSecond * CatchUpSeconds;

    AActor* Aircraft = GetWorld()->SpawnActor<AActor>(
        Entry.AircraftClass, FTransform::Identity);
    if (!IsValid(Aircraft))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP Aerial '%s': failed to spawn Aircraft Class '%s'."),
            *Entry.EntryId.ToString(), *GetNameSafe(Entry.AircraftClass.Get()));
        return false;
    }
    Flight.Aircraft = Aircraft;

    ActiveFlights.Add(Entry.InstanceId, Flight);
    FActiveFlight& AddedFlight = ActiveFlights.FindChecked(Entry.InstanceId);
    if (!ApplyFlightTransform(AddedFlight))
    {
        DespawnFlight(Entry.InstanceId);
        return Entry.EndBehavior == ETMOPAerialEndBehavior::DespawnAtEnd;
    }
    return true;
}

void ATMOPAerialVehicleDirector::DespawnFlight(const FName InstanceId)
{
    if (FActiveFlight* Flight = ActiveFlights.Find(InstanceId))
        if (AActor* Aircraft = Flight->Aircraft.Get())
            Aircraft->Destroy();
    ActiveFlights.Remove(InstanceId);
}

void ATMOPAerialVehicleDirector::UpdateFlights(
    const float SimulationDeltaSeconds)
{
    TArray<FName> FinishedFlights;
    for (TPair<FName, FActiveFlight>& Pair : ActiveFlights)
    {
        FActiveFlight& Flight = Pair.Value;
        Flight.DistanceCm += (Flight.bReverseDirection ? -1.0f : 1.0f) *
            Flight.SpeedCmPerSecond * SimulationDeltaSeconds;
        if (!ApplyFlightTransform(Flight))
            FinishedFlights.Add(Pair.Key);
    }
    for (const FName InstanceId : FinishedFlights)
        DespawnFlight(InstanceId);
}

bool ATMOPAerialVehicleDirector::ApplyFlightTransform(
    FActiveFlight& Flight)
{
    AActor* Aircraft = Flight.Aircraft.Get();
    USplineComponent* Spline = Flight.Spline.Get();
    if (!IsValid(Aircraft) || !IsValid(Spline)) return false;

    const float SplineLength = Spline->GetSplineLength();
    const bool bPastStart = Flight.DistanceCm < 0.0f;
    const bool bPastEnd = Flight.DistanceCm > SplineLength;
    if (bPastStart || bPastEnd)
    {
        if (Flight.EndBehavior == ETMOPAerialEndBehavior::Loop)
        {
            Flight.DistanceCm = FMath::Fmod(
                Flight.DistanceCm, SplineLength);
            if (Flight.DistanceCm < 0.0f)
                Flight.DistanceCm += SplineLength;
        }
        else if (Flight.EndBehavior ==
            ETMOPAerialEndBehavior::HoldAtEnd)
        {
            Flight.DistanceCm = FMath::Clamp(
                Flight.DistanceCm, 0.0f, SplineLength);
        }
        else
        {
            return false;
        }
    }

    const FVector Location = Spline->GetLocationAtDistanceAlongSpline(
        Flight.DistanceCm, ESplineCoordinateSpace::World);
    FRotator SplineRotation = Spline->GetRotationAtDistanceAlongSpline(
        Flight.DistanceCm, ESplineCoordinateSpace::World);
    if (Flight.bReverseDirection)
        SplineRotation.Yaw += 180.0f;
    const FQuat FinalRotation =
        SplineRotation.Quaternion() * Flight.RotationOffset.Quaternion();
    Aircraft->SetActorLocationAndRotation(
        Location, FinalRotation, false, nullptr,
        ETeleportType::TeleportPhysics);
    return true;
}

void ATMOPAerialVehicleDirector::DestroyAllAircraft()
{
    for (TPair<FName, FActiveFlight>& Pair : ActiveFlights)
        if (AActor* Aircraft = Pair.Value.Aircraft.Get())
            Aircraft->Destroy();
    ActiveFlights.Empty();
}
