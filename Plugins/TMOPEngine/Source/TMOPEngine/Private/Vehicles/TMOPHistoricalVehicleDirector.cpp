#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPVehicleRoutePlan.h"
#include "Vehicles/TMOPVehicleTimeline.h"
#include "Vehicles/TMOPVehicleRouteMath.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "Vehicles/TMOPVehiclePresentation.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "World/TMOPWorldSubsystem.h"

namespace
{
    const FName HistoricalVehicleObjectType(TEXT("HistoricalVehicle"));


}

bool ATMOPHistoricalVehicleDirector::ResolveTimelineEntrySecond(
    const FTMOPHistoricalVehicleRow& Profile,
    const int32 EntryIndex,
    int32& OutSecond) const
{
    auto EventTime = [this](FName Id, int32& Second)
    {
        const auto* Events = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>() : nullptr;
        FTMOPHistoricalEventRuntime Event;
        if (!Events || !Events->TryGetEventRuntime(Id, Event) || !Event.bHasResolvedTime) return false;
        Second = Event.ResolvedTime.ToSecondsFromMidnight(); return true;
    };
    return TMOPVehicleTimeline::ResolveEntry(Profile, EntryIndex, EventTime, OutSecond);
}

bool ATMOPHistoricalVehicleDirector::ResolveTimelineEntryCompletionSecond(
    const FTMOPHistoricalVehicleRow& Profile,
    const int32 EntryIndex,
    int32& OutSecond) const
{
    if (!ResolveTimelineEntrySecond(Profile, EntryIndex, OutSecond))
        return false;
    OutSecond += TMOPVehicleRoute::CompletionDelay(Profile.Timeline[EntryIndex]);
    return true;
}

bool ATMOPHistoricalVehicleDirector::ResolveDrivingDepartureSecond(
    const FTMOPHistoricalVehicleRow& Profile,
    const int32 DrivingEntryIndex,
    int32& OutSecond) const
{
    auto EventTime = [this](FName Id, int32& Second)
    {
        const auto* Events = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>() : nullptr;
        FTMOPHistoricalEventRuntime Event;
        if (!Events || !Events->TryGetEventRuntime(Id, Event) || !Event.bHasResolvedTime) return false;
        Second = Event.ResolvedTime.ToSecondsFromMidnight(); return true;
    };
    return TMOPVehicleTimeline::ResolveDeparture(Profile, DrivingEntryIndex, EventTime, OutSecond);
}

ATMOPHistoricalVehicleDirector::ATMOPHistoricalVehicleDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    DefaultVehicleClass = ATMOPConfiguredVehicle::StaticClass();
}

void ATMOPHistoricalVehicleDirector::BeginPlay()
{
    Super::BeginPlay();
    InitializeHistoricalVehicles();
}

void ATMOPHistoricalVehicleDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        if (ATMOPVehicleBase* Vehicle = Pair.Value.Vehicle.Get())
        {
            UnregisterVehicle(Vehicle);
        }
    }
    RuntimeVehicles.Reset();
    LastDrivingFailureCodes.Reset();
    LastDrivingFailureDetails.Reset();
    Super::EndPlay(EndPlayReason);
}

void ATMOPHistoricalVehicleDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateBoundaryEntryCollision(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock == nullptr)
    {
        return;
    }

    const int32 CurrentSecond =
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    const int32 ExpectedStep = FMath::CeilToInt(DeltaSeconds * FMath::Max(0.0f, Clock->GetTimeScale()));
    if (LastEvaluatedSecond != INDEX_NONE &&
        (CurrentSecond < LastEvaluatedSecond ||
         CurrentSecond - LastEvaluatedSecond > FMath::Max(3, ExpectedStep + 2)))
    {
        InitializeHistoricalVehicles();
        return;
    }
    if (CurrentSecond != LastEvaluatedSecond)
    {
        SpawnDueVehicles(CurrentSecond);
        ApplyDueVehiclePlacements(CurrentSecond);
        DespawnDueVehicles(CurrentSecond);
        LastEvaluatedSecond = CurrentSecond;
    }
    // Retry every frame after the due second. This lets same-second person
    // seating finish before a migrated vehicle-owned route starts.
    StartDueVehicleRoutes(CurrentSecond);
    ReportCompletedVehicleArrivals(CurrentSecond);
}

void ATMOPHistoricalVehicleDirector::StartDueVehicleRoutes(
    const int32 CurrentSecond)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        if (!IsValid(Vehicle) || Runtime.ActiveOffscreenTransferEntryIndex != INDEX_NONE)
            continue;
        if (Runtime.bHiddenUntilSpawn)
        {
            SetVehicleAndOccupantsHidden(Vehicle, true);
            continue;
        }

        for (int32 Index = 0; Index < Runtime.Profile.Timeline.Num(); ++Index)
        {
            const FTMOPHistoricalVehicleTimelineEntry& Entry =
                Runtime.Profile.Timeline[Index];
            const bool bDrivingAction =
                Entry.Action == ETMOPHistoricalVehicleAction::BeginDriving ||
                Entry.Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
            if (!bDrivingAction ||
                Runtime.AppliedDrivingEntryIds.Contains(Entry.EntryId))
                continue;

            int32 DepartureSecond = INDEX_NONE;
            if (!ResolveDrivingDepartureSecond(Runtime.Profile, Index, DepartureSecond))
            {
                ReportDrivingFailure(Runtime.Profile.VehicleId, TEXT("UnresolvedDeparture"),
                    FString::Printf(TEXT("Route '%s' cannot resolve its departure. Check the shared event and previous row."), *Entry.EntryId.ToString()));
                break;
            }
            if (DepartureSecond > CurrentSecond) continue;

            int32 WindowDeparture, WindowArrival;
            if (!ResolveDrivingWindow(Runtime.Profile, Index, WindowDeparture, WindowArrival))
            {
                ReportDrivingFailure(Runtime.Profile.VehicleId, TEXT("UnresolvedDrivingWindow"),
                    FString::Printf(TEXT("Route '%s' needs valid departure and arrival times."), *Entry.EntryId.ToString()));
                break;
            }
            if (CurrentSecond >= WindowArrival || Index < Runtime.LastAppliedLifecycleEntryIndex)
            {
                if (CurrentSecond >= WindowArrival && Index >= Runtime.LastAppliedLifecycleEntryIndex)
                {
                    const FString PreviousFailure = LastDrivingFailureDetails.FindRef(Runtime.Profile.VehicleId);
                    ReportDrivingFailure(Runtime.Profile.VehicleId, TEXT("RouteNeverStarted"),
                        FString::Printf(TEXT("Route '%s' expired without starting (%d -> %d). Last start failure: %s"),
                            *Entry.EntryId.ToString(), WindowDeparture, WindowArrival, *PreviousFailure));
                }
                Runtime.AppliedDrivingEntryIds.Add(Entry.EntryId);
                continue; // Expired routes never replay after late boarding or a seek.
            }
            const FName RequiredDriver = TMOPVehicleRoute::Driver(Runtime.Profile, Entry);
            ATMOPHistoricalAgent* Driver = Vehicle->GetDriverAgent();
            const FName SeatedDriverId =
                IsValid(Driver) && IsValid(Driver->EntityIdentity)
                ? Driver->EntityIdentity->EntityId : NAME_None;
            if (RequiredDriver.IsNone() || SeatedDriverId != RequiredDriver)
            {
                ReportDrivingFailure(Runtime.Profile.VehicleId, TEXT("WaitingForDriver"),
                    FString::Printf(TEXT("Route '%s' needs seated driver '%s'; seated now '%s'."),
                        *Entry.EntryId.ToString(), *RequiredDriver.ToString(), *SeatedDriverId.ToString()));
                break;
            }

            if (Entry.bWaitForListedOccupants)
            {
                TSet<FName> SeatedIds;
                for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
                {
                    ATMOPHistoricalAgent* Occupant = IsValid(Seat)
                        ? Seat->GetOccupant() : nullptr;
                    if (IsValid(Occupant) && IsValid(Occupant->EntityIdentity))
                        SeatedIds.Add(Occupant->EntityIdentity->EntityId);
                }
                bool bAllSeated = true;
                for (const FName PassengerId : Entry.PassengerEntityIds)
                    if (!PassengerId.IsNone() && !SeatedIds.Contains(PassengerId))
                    {
                        bAllSeated = false;
                        ReportDrivingFailure(Runtime.Profile.VehicleId, TEXT("WaitingForListedOccupants"),
                            FString::Printf(TEXT("Route '%s' waits for passenger '%s' to board '%s'."),
                                *Entry.EntryId.ToString(), *PassengerId.ToString(), *Runtime.Profile.VehicleId.ToString()));
                        break;
                    }
                if (!bAllSeated) break;
            }

            // A previous route may still have its movement flag set on the
            // boundary tick. Its flag must not suppress a scheduled successor.
            // Reject genuinely overlapping windows instead; StartRoutePlan
            // still checks the actual start position before replacing a plan.
            bool bOverlaps = false;
            for (int32 Earlier = 0; Earlier < Index; ++Earlier)
            {
                if (!TMOPVehicleRoute::IsDriving(Runtime.Profile.Timeline[Earlier].Action)) continue;
                int32 EarlierDeparture, EarlierArrival;
                if (ResolveDrivingWindow(Runtime.Profile, Earlier, EarlierDeparture, EarlierArrival) &&
                    EarlierDeparture < WindowArrival && EarlierArrival > WindowDeparture)
                {
                    ReportDrivingFailure(Runtime.Profile.VehicleId, TEXT("OverlappingDrivingWindows"),
                        FString::Printf(TEXT("Route '%s' overlaps '%s'. Correct their departure/arrival times."),
                            *Entry.EntryId.ToString(), *Runtime.Profile.Timeline[Earlier].EntryId.ToString()));
                    bOverlaps = true;
                    break;
                }
            }
            if (bOverlaps) break;

            RequestedDrivingEntryOverrides.Add(
                Runtime.Profile.VehicleId, Entry.EntryId);
            const bool bStarted = BeginDrivingVehicle(Runtime.Profile.VehicleId,
                RequiredDriver, Entry.OrderedLaneIds,
                Entry.RouteViaAnchorIds, Entry.VehicleRouteMode,
                Entry.RouteDestinationAnchorId,
                Entry.RouteStartDistanceAlongFirstLaneCm);
            RequestedDrivingEntryOverrides.Remove(Runtime.Profile.VehicleId);
            if (bStarted)
            {
                Runtime.AppliedDrivingEntryIds.Add(Entry.EntryId);
                UE_LOG(LogTemp, Display,
                    TEXT("TMOP vehicle-owned timeline: '%s' started '%s' at %02d:%02d:%02d."),
                    *Runtime.Profile.VehicleId.ToString(),
                    *Entry.EntryId.ToString(),
                    FTMOPTime::FromSecondsFromMidnight(DepartureSecond).Hour,
                    FTMOPTime::FromSecondsFromMidnight(DepartureSecond).Minute,
                    FTMOPTime::FromSecondsFromMidnight(DepartureSecond).Second);
            }
        }
    }
}

void ATMOPHistoricalVehicleDirector::ApplyDueVehiclePlacements(
    const int32 CurrentSecond)
{
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    if (Anchors != nullptr) Anchors->DiscoverAnchorsInWorld();
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        if (!IsValid(Vehicle)) continue;
        CompleteDueOffscreenTransfer(Runtime, CurrentSecond);
        for (int32 Index = 0; Index < Runtime.Profile.Timeline.Num(); ++Index)
        {
            const FTMOPHistoricalVehicleTimelineEntry& Entry =
                Runtime.Profile.Timeline[Index];
            // When initialization jumps directly into a later vehicle life,
            // do not replay stops/placements belonging to an earlier life.
            if (Runtime.LastAppliedLifecycleEntryIndex != INDEX_NONE &&
                Index < Runtime.LastAppliedLifecycleEntryIndex)
            {
                Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                continue;
            }
            int32 EntrySecond = INDEX_NONE;
            if (Runtime.AppliedPlacementEntryIds.Contains(Entry.EntryId) ||
                !ResolveTimelineEntrySecond(
                    Runtime.Profile, Index, EntrySecond) ||
                EntrySecond > CurrentSecond)
                continue;
            const bool bInitialEntry = Index == 0 &&
                (Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                 Entry.Action == ETMOPHistoricalVehicleAction::Spawn);
            if (bInitialEntry)
            {
                Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                continue;
            }
            const bool bEmergencySirenAction =
                Entry.Action ==
                    ETMOPHistoricalVehicleAction::EmergencySirenOn ||
                Entry.Action ==
                    ETMOPHistoricalVehicleAction::EmergencySirenOff;
            if (bEmergencySirenAction)
            {
                UTMOPVehicleAudioComponent* Audio =
                    Vehicle->FindComponentByClass<
                        UTMOPVehicleAudioComponent>();
                if (!IsValid(Audio))
                {
                    // The audio director attaches this runtime component.
                    // Leave the entry pending and retry next simulation second
                    // if the vehicle has only just spawned.
                    continue;
                }
                const bool bEnable = Entry.Action ==
                    ETMOPHistoricalVehicleAction::EmergencySirenOn;
                Audio->SetEmergencySirenEnabled(bEnable);
                Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                UE_LOG(LogTemp, Display,
                    TEXT("TMOP VehicleSiren: '%s' set emergency siren %s at %02d:%02d:%02d."),
                    *Runtime.Profile.VehicleId.ToString(),
                    bEnable ? TEXT("ON") : TEXT("OFF"),
                    FTMOPTime::FromSecondsFromMidnight(EntrySecond).Hour,
                    FTMOPTime::FromSecondsFromMidnight(EntrySecond).Minute,
                    FTMOPTime::FromSecondsFromMidnight(EntrySecond).Second);
                continue;
            }
            const bool bTimedPlacement =
                Entry.Action == ETMOPHistoricalVehicleAction::ExitTrafficRoute ||
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                Entry.Action == ETMOPHistoricalVehicleAction::Park ||
                Entry.Action ==
                    ETMOPHistoricalVehicleAction::OffscreenTransfer;
            if (!bTimedPlacement) continue;
            const bool bArrivalEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::ExitTrafficRoute ||
                Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                Entry.Action == ETMOPHistoricalVehicleAction::Park;
            FTransform Target = Entry.WorldTransform;
            if (Entry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor)
            {
                ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
                    ? Anchors->FindAnchor(Entry.PlacementAnchorId) : nullptr;
                if (!IsValid(Anchor))
                {
                    UE_LOG(LogTemp, Error,
                        TEXT("TMOP VehicleTimedPlacement: anchor '%s' for '%s' was not found."),
                        *Entry.PlacementAnchorId.ToString(),
                        *Runtime.Profile.VehicleId.ToString());
                    continue;
                }
                Target = Entry.AnchorLocalOffset * FTransform(
                    Anchor->GetAnchorRotation(), Anchor->GetAnchorLocation(),
                    FVector::OneVector);
            }
            if (Entry.Action ==
                ETMOPHistoricalVehicleAction::OffscreenTransfer)
            {
                if (UTMOPTrafficVehicleMovementComponent* Movement =
                    Vehicle->FindComponentByClass<
                        UTMOPTrafficVehicleMovementComponent>())
                {
                    Movement->StopDriving();
                    Movement->PlannedLaneIds.Reset();
                    Movement->TrafficState =
                        ETMOPTrafficVehicleState::RouteComplete;
                    Movement->bDetectPhysicalObstacles = false;
                }
                // Hide before teleporting so neither the vehicle nor an
                // attached occupant can flash across the playable area.
                SetVehicleAndOccupantsHidden(Vehicle, true);
                Vehicle->SetActorEnableCollision(false);
                Vehicle->SetActorTransform(
                    Target, false, nullptr, ETeleportType::TeleportPhysics);
                Runtime.bBoundaryCollisionSuppressed = false;
                Runtime.bBoundaryVehicleHasStartedDriving = false;
                Runtime.ActiveOffscreenTransferEntryIndex = Index;
                Runtime.OffscreenTransferRevealSecond = EntrySecond +
                    FMath::Max(0,
                        Entry.OffscreenTransferDurationSeconds);
                Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                UE_LOG(LogTemp, Display,
                    TEXT("TMOP VehicleOffscreenTransfer: '%s' moved to '%s' and remains hidden until %02d:%02d:%02d."),
                    *Runtime.Profile.VehicleId.ToString(),
                    Entry.PlacementMode ==
                            ETMOPHistoricalVehiclePlacementMode::Anchor
                        ? *Entry.PlacementAnchorId.ToString()
                        : TEXT("world transform"),
                    FTMOPTime::FromSecondsFromMidnight(
                        Runtime.OffscreenTransferRevealSecond).Hour,
                    FTMOPTime::FromSecondsFromMidnight(
                        Runtime.OffscreenTransferRevealSecond).Minute,
                    FTMOPTime::FromSecondsFromMidnight(
                        Runtime.OffscreenTransferRevealSecond).Second);
                CompleteDueOffscreenTransfer(Runtime, CurrentSecond);
                continue;
            }
            if (bArrivalEntry && Runtime.bReconstructCurrentRoute &&
                ReconstructionSecond != INDEX_NONE && EntrySecond < ReconstructionSecond)
            {
                // A seek reconstructs past state; it is not an observed arrival
                // and must not generate a passed/failed validation result.
                if (auto* Movement = Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
                    Movement->StopDriving();
                Vehicle->SetActorTransform(Target, false, nullptr, ETeleportType::TeleportPhysics);
                Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                continue;
            }
            if (UTMOPTrafficVehicleMovementComponent* Movement =
                Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
            {
                // A scheduled Stop/Park is a target time, not permission to
                // snap a delayed car away from its lane. Let an already
                // configured final approach finish naturally and update its
                // exact parking transform from this timeline entry.
                if ((Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                     Entry.Action == ETMOPHistoricalVehicleAction::Park ||
                     Entry.Action == ETMOPHistoricalVehicleAction::ExitTrafficRoute) &&
                    !Movement->HasTimedArrival() &&
                    Movement->UpdateFinalApproachTarget(Target))
                {
                    Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                    Runtime.PendingArrivalEntryId = Entry.EntryId;
                    Runtime.PendingArrivalPlannedSecond = EntrySecond;
                    UE_LOG(LogTemp, Display,
                        TEXT("TMOP VehicleTimedPlacement: '%s' deferred '%s' to its smooth final approach."),
                        *Runtime.Profile.VehicleId.ToString(),
                        *Entry.EntryId.ToString());
                    continue;
                }
                if (Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                    Entry.Action == ETMOPHistoricalVehicleAction::Park ||
                    Entry.Action == ETMOPHistoricalVehicleAction::ExitTrafficRoute)
                {
                    if (Movement->HasTimedArrival())
                    {
                        Movement->UpdateFinalApproachTarget(Target);
                        Movement->ForceCompleteTimedArrival();
                    }
                    const float CorrectionDistanceCm = FVector::Dist2D(
                        Vehicle->GetActorLocation(), Target.GetLocation());
                    if (Movement->bLastArrivalBlocked || (!bAllowDistantTimedParkingTeleport &&
                        CorrectionDistanceCm > TimedParkingAlignmentToleranceCm))
                    {
                        // A timetable is not permission to hide an invalid or
                        // blocked route. Stop where the vehicle really is and
                        // let validation expose the missed parking anchor.
                        // Keep the planned route incomplete so occupants do
                        // not disembark at the wrong location.
                        Movement->StopDriving();
                        Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                        Runtime.PendingArrivalEntryId = NAME_None;
                        Runtime.PendingArrivalPlannedSecond = INDEX_NONE;
                        OnTimelineEntryArrived.Broadcast(
                            Runtime.Profile.VehicleId, Entry,
                            EntrySecond, CurrentSecond, false);
                        UE_LOG(LogTemp, Error,
                            TEXT("TMOP VehicleTimedPlacement: '%s' missed '%s' by %.0f cm; distant teleport suppressed."),
                            *Runtime.Profile.VehicleId.ToString(),
                            *Entry.PlacementAnchorId.ToString(),
                            CorrectionDistanceCm);
                        continue;
                    }
                }
                Movement->StopDriving();
                Movement->PlannedLaneIds.Reset();
                Movement->TrafficState =
                    ETMOPTrafficVehicleState::RouteComplete;
            }
            Vehicle->SetActorEnableCollision(false);
            Vehicle->SetActorTransform(
                Target, false, nullptr, ETeleportType::TeleportPhysics);
            if (IsVehicleClearForCollisionRestore(Vehicle))
            {
                Vehicle->SetActorEnableCollision(true);
                if (UTMOPTrafficVehicleMovementComponent* Movement =
                    Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
                    Movement->bDetectPhysicalObstacles = true;
                Runtime.bBoundaryCollisionSuppressed = false;
            }
            else
            {
                Runtime.bBoundaryCollisionSuppressed = true;
                Runtime.bBoundaryVehicleHasStartedDriving = false;
            }
            Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
            if (bArrivalEntry)
            {
                Runtime.PendingArrivalEntryId = NAME_None;
                Runtime.PendingArrivalPlannedSecond = INDEX_NONE;
                OnTimelineEntryArrived.Broadcast(
                    Runtime.Profile.VehicleId, Entry,
                    EntrySecond, CurrentSecond, true);
                if (auto* Movement = Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
                    Movement->LastArrivalCorrectionCm = 0.0;
            }
            UE_LOG(LogTemp, Display,
                TEXT("TMOP VehicleTimedPlacement: '%s' applied '%s' at %02d:%02d:%02d."),
                *Runtime.Profile.VehicleId.ToString(), *Entry.EntryId.ToString(),
                FTMOPTime::FromSecondsFromMidnight(EntrySecond).Hour,
                FTMOPTime::FromSecondsFromMidnight(EntrySecond).Minute,
                FTMOPTime::FromSecondsFromMidnight(EntrySecond).Second);
        }
    }
}

void ATMOPHistoricalVehicleDirector::ReportCompletedVehicleArrivals(
    const int32 CurrentSecond)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (Runtime.PendingArrivalEntryId.IsNone()) continue;

        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        const UTMOPTrafficVehicleMovementComponent* Movement =
            IsValid(Vehicle)
            ? Vehicle->FindComponentByClass<
                UTMOPTrafficVehicleMovementComponent>()
            : nullptr;
        if (!IsValid(Movement) || Movement->IsDrivingEnabled() ||
            Movement->TrafficState != ETMOPTrafficVehicleState::RouteComplete)
            continue;

        const FName CompletedEntryId = Runtime.PendingArrivalEntryId;
        const int32 PlannedSecond = Runtime.PendingArrivalPlannedSecond;
        Runtime.PendingArrivalEntryId = NAME_None;
        Runtime.PendingArrivalPlannedSecond = INDEX_NONE;
        const FTMOPHistoricalVehicleTimelineEntry* Entry =
            Runtime.Profile.Timeline.FindByPredicate(
                [CompletedEntryId](
                    const FTMOPHistoricalVehicleTimelineEntry& Candidate)
                {
                    return Candidate.EntryId == CompletedEntryId;
                });
        if (Entry != nullptr)
        {
            OnTimelineEntryArrived.Broadcast(
                Runtime.Profile.VehicleId, *Entry,
                PlannedSecond, CurrentSecond, true);
        }
    }
}

void ATMOPHistoricalVehicleDirector::SetVehicleAndOccupantsHidden(
    ATMOPVehicleBase* Vehicle, const bool bShouldHide) const
{
    if (!IsValid(Vehicle)) return;
    Vehicle->SetActorHiddenInGame(bShouldHide);
    for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
    {
        ACharacter* Occupant = IsValid(Seat)
            ? Seat->GetOccupantCharacter() : nullptr;
        if (IsValid(Occupant)) Occupant->SetActorHiddenInGame(bShouldHide);
    }
}

void ATMOPHistoricalVehicleDirector::CompleteDueOffscreenTransfer(
    FHistoricalVehicleRuntime& Runtime, const int32 CurrentSecond)
{
    if (Runtime.ActiveOffscreenTransferEntryIndex == INDEX_NONE ||
        Runtime.OffscreenTransferRevealSecond == INDEX_NONE ||
        CurrentSecond < Runtime.OffscreenTransferRevealSecond)
        return;

    ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
    if (!IsValid(Vehicle))
    {
        Runtime.ActiveOffscreenTransferEntryIndex = INDEX_NONE;
        Runtime.OffscreenTransferRevealSecond = INDEX_NONE;
        return;
    }

    const int32 CompletedIndex =
        Runtime.ActiveOffscreenTransferEntryIndex;
    const FTMOPHistoricalVehicleTimelineEntry* TransferEntry =
        Runtime.Profile.Timeline.IsValidIndex(CompletedIndex)
            ? &Runtime.Profile.Timeline[CompletedIndex] : nullptr;
    Vehicle->SetActorTickEnabled(true);
    SetVehicleAndOccupantsHidden(Vehicle, false);

    if (IsVehicleClearForCollisionRestore(Vehicle))
    {
        Vehicle->SetActorEnableCollision(true);
        if (UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<
                UTMOPTrafficVehicleMovementComponent>())
            Movement->bDetectPhysicalObstacles = true;
        Runtime.bBoundaryCollisionSuppressed = false;
    }
    else
    {
        // Keep a transferred car ghosted until it has driven clear of an
        // occupied entry point, exactly like a normal boundary spawn.
        SuppressBoundaryEntryCollision(Runtime, true);
    }

    Runtime.ActiveOffscreenTransferEntryIndex = INDEX_NONE;
    Runtime.OffscreenTransferRevealSecond = INDEX_NONE;
    UE_LOG(LogTemp, Display,
        TEXT("TMOP VehicleOffscreenTransfer: '%s' reappeared at '%s'."),
        *Runtime.Profile.VehicleId.ToString(),
        TransferEntry != nullptr &&
                TransferEntry->PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor
            ? *TransferEntry->PlacementAnchorId.ToString()
            : TEXT("world transform"));
}

void ATMOPHistoricalVehicleDirector::DespawnDueVehicles(
    const int32 CurrentSecond)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        int32 DueLifecycleIndex = INDEX_NONE;
        int32 DueLifecycleSecond = INDEX_NONE;
        for (int32 Index = 0; Index < Runtime.Profile.Timeline.Num(); ++Index)
        {
            const FTMOPHistoricalVehicleTimelineEntry& Entry =
                Runtime.Profile.Timeline[Index];
            const bool bLifecycleEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn ||
                Entry.Action == ETMOPHistoricalVehicleAction::Despawn;
            int32 EntrySecond = INDEX_NONE;
            if (bLifecycleEntry &&
                ResolveTimelineEntrySecond(
                    Runtime.Profile, Index, EntrySecond) &&
                EntrySecond <= CurrentSecond &&
                (DueLifecycleIndex == INDEX_NONE ||
                 EntrySecond >= DueLifecycleSecond))
            {
                DueLifecycleIndex = Index;
                DueLifecycleSecond = EntrySecond;
            }
        }
        if (DueLifecycleIndex == INDEX_NONE ||
            DueLifecycleIndex == Runtime.LastAppliedLifecycleEntryIndex)
            continue;
        const FTMOPHistoricalVehicleTimelineEntry& DespawnEntry =
            Runtime.Profile.Timeline[DueLifecycleIndex];
        if (DespawnEntry.Action != ETMOPHistoricalVehicleAction::Despawn)
            continue;

        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        Runtime.LastAppliedLifecycleEntryIndex = DueLifecycleIndex;
        if (!IsValid(Vehicle)) continue;

        UE_LOG(LogTemp, Display,
            TEXT("TMOP Historical Vehicles: despawned '%s' at %02d:%02d:%02d."),
            *Runtime.Profile.VehicleId.ToString(),
            FTMOPTime::FromSecondsFromMidnight(DueLifecycleSecond).Hour,
            FTMOPTime::FromSecondsFromMidnight(DueLifecycleSecond).Minute,
            FTMOPTime::FromSecondsFromMidnight(DueLifecycleSecond).Second);
        UnregisterVehicle(Vehicle);
        Vehicle->Destroy();
        Runtime.Vehicle.Reset();
        Runtime.bSpawnedByDirector = false;
        Runtime.bDeferredPlacedVehicle = false;
        Runtime.bBoundaryCollisionSuppressed = false;
        Runtime.bBoundaryVehicleHasStartedDriving = false;
        Runtime.ActiveOffscreenTransferEntryIndex = INDEX_NONE;
        Runtime.OffscreenTransferRevealSecond = INDEX_NONE;
        Runtime.PendingArrivalEntryId = NAME_None;
        Runtime.PendingArrivalPlannedSecond = INDEX_NONE;
    }
}

int32 ATMOPHistoricalVehicleDirector::InitializeHistoricalVehicles()
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (Runtime.bSpawnedByDirector && Runtime.Vehicle.IsValid())
        {
            UnregisterVehicle(Runtime.Vehicle.Get());
            Runtime.Vehicle->Destroy();
        }
    }
    RuntimeVehicles.Reset();

    TArray<FString> Errors;
    ValidateHistoricalVehicleTable(Errors);
    for (const FString& Error : Errors)
        UE_LOG(LogTemp, Error, TEXT("TMOP Historical Vehicles: %s"), *Error);
    if (!IsValid(HistoricalVehicleTable) ||
        HistoricalVehicleTable->GetRowStruct() !=
            FTMOPHistoricalVehicleRow::StaticStruct())
        return 0;

    const TMap<FName, uint8*>& Rows = HistoricalVehicleTable->GetRowMap();
    RuntimeVehicles.Reserve(Rows.Num());
    for (const TPair<FName, uint8*>& Pair : Rows)
    {
        const FTMOPHistoricalVehicleRow* Row =
            reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
        if (Row == nullptr)
        {
            continue;
        }
        // Rows with spawning disabled may intentionally be archive-only
        // records. They do not need a timeline and must not be reported as
        // invalid runtime vehicles.
        if (!Row->bSpawnInSimulation && Row->Timeline.IsEmpty())
        {
            UE_LOG(LogTemp, Verbose,
                TEXT("TMOP Historical Vehicles: archive-only row '%s' has no timeline and will not be registered for simulation."),
                *Pair.Key.ToString());
            continue;
        }
        if (Row->VehicleId.IsNone() ||
            !Row->Timeline.ContainsByPredicate(
                [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
                {
                    return Entry.Action ==
                            ETMOPHistoricalVehicleAction::InitialPlacement ||
                        Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
                }))
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Historical Vehicles: skipped invalid row '%s'."),
                *Pair.Key.ToString());
            continue;
        }

        FHistoricalVehicleRuntime Runtime;
        Runtime.RowName = Pair.Key;
        Runtime.Profile = *Row;
        Runtime.InitialSpawnSecond = GetInitialSpawnSecond(*Row);
        // Keep a hidden staged actor available for early seat assignments.
        // An explicit Spawn before the first Despawn controls its visibility.
        for (int32 Index = 0; Index < Row->Timeline.Num(); ++Index)
        {
            if (Row->Timeline[Index].Action == ETMOPHistoricalVehicleAction::Despawn) break;
            if (Row->Timeline[Index].Action == ETMOPHistoricalVehicleAction::Spawn)
            {
                ResolveTimelineEntrySecond(*Row, Index, Runtime.FirstVisibleSpawnSecond);
                break;
            }
        }
        RuntimeVehicles.Add(Row->VehicleId, MoveTemp(Runtime));
    }

    if (bReusePlacedVehicles)
    {
        DiscoverPlacedVehicles();
    }

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    ApplyDeferredPlacedVehicleState(CurrentSecond);
    ReconstructionSecond = CurrentSecond;
    LastEvaluatedSecond = INDEX_NONE; // Apply all due lifecycle/placement rows before starting the active route.
    return bSpawnVehiclesAutomatically
        ? SpawnDueVehicles(CurrentSecond)
        : RuntimeVehicles.Num();
}

int32 ATMOPHistoricalVehicleDirector::SpawnEnabledVehicles()
{
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : FTMOPTime(23, 0, 0).ToSecondsFromMidnight();
    return SpawnDueVehicles(CurrentSecond);
}

void ATMOPHistoricalVehicleDirector::SpawnAllVehiclesForStaging()
{
    if (RuntimeVehicles.IsEmpty())
    {
        const bool bWasAutomatic = bSpawnVehiclesAutomatically;
        bSpawnVehiclesAutomatically = false;
        InitializeHistoricalVehicles();
        bSpawnVehiclesAutomatically = bWasAutomatic;
    }
    const int32 SpawnedCount = SpawnVehicles(true);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP Historical Vehicles: %d staging vehicles are available."),
        SpawnedCount);
}

int32 ATMOPHistoricalVehicleDirector::SpawnVehicles(const bool bIgnoreRowFlags)
{
    int32 AvailableCount = 0;
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (Runtime.Vehicle.IsValid())
        {
            ++AvailableCount;
            continue;
        }
        if (!ShouldSpawn(Runtime.Profile, bIgnoreRowFlags))
        {
            continue;
        }
        if (SpawnVehicle(Runtime) != nullptr)
        {
            SuppressBoundaryEntryCollision(Runtime);
            ++AvailableCount;
        }
    }
    return AvailableCount;
}

int32 ATMOPHistoricalVehicleDirector::SpawnDueVehicles(
    const int32 CurrentSecond)
{
    int32 AvailableCount = 0;
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        if (Runtime.InitialSpawnSecond == INDEX_NONE)
            Runtime.InitialSpawnSecond = GetInitialSpawnSecond(Runtime.Profile);
        if (!ShouldSpawn(Runtime.Profile, false) ||
            Runtime.InitialSpawnSecond == INDEX_NONE ||
            Runtime.InitialSpawnSecond > CurrentSecond)
        {
            continue;
        }

        int32 DueLifecycleIndex = INDEX_NONE;
        int32 DueLifecycleSecond = INDEX_NONE;
        for (int32 Index = 0; Index < Runtime.Profile.Timeline.Num(); ++Index)
        {
            const FTMOPHistoricalVehicleTimelineEntry& Entry =
                Runtime.Profile.Timeline[Index];
            const bool bLifecycleEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn ||
                Entry.Action == ETMOPHistoricalVehicleAction::Despawn;
            int32 EntrySecond = INDEX_NONE;
            if (bLifecycleEntry &&
                ResolveTimelineEntrySecond(
                    Runtime.Profile, Index, EntrySecond) &&
                EntrySecond <= CurrentSecond &&
                (DueLifecycleIndex == INDEX_NONE ||
                 EntrySecond >= DueLifecycleSecond))
            {
                DueLifecycleIndex = Index;
                DueLifecycleSecond = EntrySecond;
            }
        }
        if (DueLifecycleIndex == INDEX_NONE ||
            Runtime.Profile.Timeline[DueLifecycleIndex].Action ==
                ETMOPHistoricalVehicleAction::Despawn)
            continue;

        const bool bNewLifecycleSpawn =
            DueLifecycleIndex != Runtime.LastAppliedLifecycleEntryIndex;

        if (Runtime.bDeferredPlacedVehicle && Runtime.Vehicle.IsValid())
        {
            ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
            Vehicle->SetActorHiddenInGame(false);
            Vehicle->SetActorEnableCollision(true);
            Vehicle->SetActorTickEnabled(true);
            Runtime.bDeferredPlacedVehicle = false;
            SuppressBoundaryEntryCollision(Runtime);
            RegisterVehicle(Vehicle);
            UE_LOG(LogTemp, Display,
                TEXT("TMOP Historical Vehicles: activated placed vehicle '%s' at scheduled spawn time."),
                *Runtime.Profile.VehicleId.ToString());
        }
        else if (!Runtime.Vehicle.IsValid())
        {
            FTransform ClearSpawnTransform;
            const FTMOPHistoricalVehicleTimelineEntry& SpawnEntry =
                Runtime.Profile.Timeline[DueLifecycleIndex];
            if (!ResolveTimelinePlacementTransform(
                    SpawnEntry, ClearSpawnTransform))
                ClearSpawnTransform = GetInitialTransform(Runtime.Profile);
            const bool bBoundarySpawn = SpawnEntry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor &&
                SpawnEntry.PlacementAnchorId.ToString().StartsWith(
                    TEXT("Enter"), ESearchCase::IgnoreCase);
            FTransform QueuedTransform;
            if (bBoundarySpawn && bUseBoundarySpawnLead && FindClearBoundarySpawnTransform(
                    ClearSpawnTransform, QueuedTransform))
                ClearSpawnTransform = QueuedTransform;
            else if (bBoundarySpawn && bUseBoundarySpawnLead)
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("TMOP VehicleEntryGhostSpawn: '%s' uses collision-free boundary staging because every queue slot is occupied."),
                    *Runtime.Profile.VehicleId.ToString());
            }
            SpawnVehicle(Runtime, &ClearSpawnTransform);
            SuppressBoundaryEntryCollision(Runtime, bBoundarySpawn);
        }
        if (bNewLifecycleSpawn && Runtime.Vehicle.IsValid())
        {
            if (Runtime.Profile.Timeline[DueLifecycleIndex].Action == ETMOPHistoricalVehicleAction::Spawn)
            {
                FTransform SpawnPose;
                if (ResolveTimelinePlacementTransform(Runtime.Profile.Timeline[DueLifecycleIndex], SpawnPose))
                    Runtime.Vehicle->SetActorTransform(SpawnPose, false, nullptr, ETeleportType::TeleportPhysics);
            }
            Runtime.LastAppliedLifecycleEntryIndex = DueLifecycleIndex;
            UE_LOG(LogTemp, Display,
                TEXT("TMOP Historical Vehicles: lifecycle spawn %d applied for '%s'."),
                DueLifecycleIndex, *Runtime.Profile.VehicleId.ToString());
        }
        if (Runtime.Vehicle.IsValid())
        {
            const bool bStage = Runtime.FirstVisibleSpawnSecond != INDEX_NONE && CurrentSecond < Runtime.FirstVisibleSpawnSecond;
            if (bStage)
            {
                SetVehicleAndOccupantsHidden(Runtime.Vehicle.Get(), true);
                Runtime.Vehicle->SetActorEnableCollision(false);
                Runtime.bHiddenUntilSpawn = true;
            }
            else if (Runtime.bHiddenUntilSpawn)
            {
                SetVehicleAndOccupantsHidden(Runtime.Vehicle.Get(), false);
                Runtime.bHiddenUntilSpawn = false;
                SuppressBoundaryEntryCollision(Runtime, true);
            }
            ++AvailableCount;
        }
    }
    return AvailableCount;
}

bool ATMOPHistoricalVehicleDirector::ResolveTimelinePlacementTransform(
    const FTMOPHistoricalVehicleTimelineEntry& Entry,
    FTransform& OutTransform) const
{
    OutTransform = Entry.WorldTransform;
    if (Entry.PlacementMode !=
        ETMOPHistoricalVehiclePlacementMode::Anchor)
        return true;
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    if (Anchors == nullptr) return false;
    Anchors->DiscoverAnchorsInWorld();
    ATMOPHistoricalAnchor* Anchor = Anchors->FindAnchor(Entry.PlacementAnchorId);
    if (!IsValid(Anchor)) return false;
    OutTransform = Entry.AnchorLocalOffset * FTransform(
        Anchor->GetAnchorRotation(), Anchor->GetAnchorLocation(),
        FVector::OneVector);
    return true;
}

int32 ATMOPHistoricalVehicleDirector::GetInitialSpawnSecond(
    const FTMOPHistoricalVehicleRow& Profile) const
{
    for (int32 Index = 0; Index < Profile.Timeline.Num(); ++Index)
    {
        const FTMOPHistoricalVehicleTimelineEntry& Entry =
            Profile.Timeline[Index];
        if (Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
            Entry.Action == ETMOPHistoricalVehicleAction::Spawn)
        {
            int32 SpawnSecond = INDEX_NONE;
            if (!ResolveTimelineEntrySecond(Profile, Index, SpawnSecond))
                return INDEX_NONE;
            const FString AnchorId = Entry.PlacementAnchorId.ToString();
            const bool bBoundaryEntry =
                Entry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor &&
                AnchorId.StartsWith(TEXT("Enter"),
                    ESearchCase::IgnoreCase);
            // An occupied boundary vehicle must exist when its people timeline
            // assigns the driver and passengers. Deferring it until shortly
            // before BeginDriving makes every 23:00 EnterVehicle fail and the
            // vehicle consequently has no driver when its route begins.
            const bool bOccupiedAtInitialPlacement =
                !Entry.DriverEntityId.IsNone() ||
                !Entry.PassengerEntityIds.IsEmpty();
            if (bUseBoundarySpawnLead && bBoundaryEntry && !bOccupiedAtInitialPlacement)
            {
                for (int32 LaterIndex = Index + 1;
                    LaterIndex < Profile.Timeline.Num(); ++LaterIndex)
                {
                    const FTMOPHistoricalVehicleTimelineEntry& Later =
                        Profile.Timeline[LaterIndex];
                    if (Later.Action ==
                            ETMOPHistoricalVehicleAction::BeginDriving ||
                        Later.Action ==
                            ETMOPHistoricalVehicleAction::EnterTrafficRoute)
                    {
                        int32 DrivingSecond = INDEX_NONE;
                        if (!ResolveDrivingDepartureSecond(
                                Profile, LaterIndex, DrivingSecond))
                            continue;
                        SpawnSecond = FMath::Max(
                            0, DrivingSecond - EntrySpawnLeadSeconds);
                        break;
                    }
                }
            }
            return SpawnSecond;
        }
    }
    return INDEX_NONE;
}

bool ATMOPHistoricalVehicleDirector::FindClearInitialSpawnTransform(
    const FTMOPHistoricalVehicleRow& Profile,
    FTransform& OutTransform) const
{
    UWorld* World = GetWorld();
    if (World == nullptr) return false;
    const FTMOPHistoricalVehicleTimelineEntry* Placement =
        Profile.Timeline.FindByPredicate(
            [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
            {
                return Entry.Action ==
                        ETMOPHistoricalVehicleAction::InitialPlacement ||
                    Entry.Action ==
                        ETMOPHistoricalVehicleAction::Spawn;
            });
    if (Placement == nullptr ||
        Placement->PlacementMode !=
            ETMOPHistoricalVehiclePlacementMode::Anchor ||
        !Placement->PlacementAnchorId.ToString().StartsWith(
            TEXT("Enter"), ESearchCase::IgnoreCase))
    {
        OutTransform = GetInitialTransform(Profile);
        return true;
    }

    const FTransform Base = GetInitialTransform(Profile);
    return FindClearBoundarySpawnTransform(Base, OutTransform);
}

bool ATMOPHistoricalVehicleDirector::FindClearBoundarySpawnTransform(
    const FTransform& Base,
    FTransform& OutTransform) const
{
    UWorld* World = GetWorld();
    if (World == nullptr) return false;
    const FVector Backward = -Base.GetRotation().GetForwardVector();
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(
        SCENE_QUERY_STAT(TMOPVehicleEntrySpawn), false, this);

    const int32 SlotCount = FMath::Max(1, EntrySpawnQueueSlots);
    for (int32 Slot = 0; Slot < SlotCount; ++Slot)
    {
        FTransform Candidate = Base;
        Candidate.AddToTranslation(
            Backward * EntrySpawnQueueSpacingCm * Slot);
        TArray<FOverlapResult> Overlaps;
        World->OverlapMultiByObjectType(
            Overlaps, Candidate.GetLocation(), FQuat::Identity, Objects,
            FCollisionShape::MakeSphere(EntrySpawnClearanceRadiusCm), Query);
        const bool bVehicleOccupied = Overlaps.ContainsByPredicate(
            [](const FOverlapResult& Overlap)
            {
                return IsValid(Cast<ATMOPVehicleBase>(Overlap.GetActor()));
            });
        if (!bVehicleOccupied)
        {
            OutTransform = Candidate;
            return true;
        }
    }
    return false;
}

bool ATMOPHistoricalVehicleDirector::IsBoundaryEntryVehicle(
    const FTMOPHistoricalVehicleRow& Profile) const
{
    const FTMOPHistoricalVehicleTimelineEntry* Placement =
        Profile.Timeline.FindByPredicate(
            [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
            {
                return Entry.Action ==
                        ETMOPHistoricalVehicleAction::InitialPlacement ||
                    Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
            });
    return Placement != nullptr &&
        Placement->PlacementMode ==
            ETMOPHistoricalVehiclePlacementMode::Anchor &&
        Placement->PlacementAnchorId.ToString().StartsWith(
            TEXT("Enter"), ESearchCase::IgnoreCase);
}

void ATMOPHistoricalVehicleDirector::SuppressBoundaryEntryCollision(
    FHistoricalVehicleRuntime& Runtime,
    const bool bForceBoundaryEntry)
{
    ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
    if (!IsValid(Vehicle) ||
        (!bForceBoundaryEntry && !IsBoundaryEntryVehicle(Runtime.Profile)))
        return;
    Vehicle->SetActorEnableCollision(false);
    if (UTMOPTrafficVehicleMovementComponent* Movement =
        Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
        Movement->bDetectPhysicalObstacles = false;
    Runtime.bBoundaryCollisionSuppressed = true;
    Runtime.bBoundaryVehicleHasStartedDriving = false;
    Runtime.BoundaryDrivingSeconds = 0.0f;
    Runtime.BoundaryDrivingStartLocation = Vehicle->GetActorLocation();
}

bool ATMOPHistoricalVehicleDirector::IsVehicleClearForCollisionRestore(
    const ATMOPVehicleBase* Vehicle) const
{
    if (!IsValid(Vehicle) || GetWorld() == nullptr) return false;
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(
        SCENE_QUERY_STAT(TMOPVehicleEntryCollisionRestore), false, Vehicle);
    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, Vehicle->GetActorLocation(), FQuat::Identity, Objects,
        FCollisionShape::MakeSphere(EntrySpawnClearanceRadiusCm), Query);
    return !Overlaps.ContainsByPredicate(
        [Vehicle](const FOverlapResult& Overlap)
        {
            const ATMOPVehicleBase* Other =
                Cast<ATMOPVehicleBase>(Overlap.GetActor());
            return IsValid(Other) && Other != Vehicle;
        });
}

void ATMOPHistoricalVehicleDirector::UpdateBoundaryEntryCollision(
    const float DeltaSeconds)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        if (!Runtime.bBoundaryCollisionSuppressed ||
            !Runtime.bBoundaryVehicleHasStartedDriving || !IsValid(Vehicle))
            continue;
        Runtime.BoundaryDrivingSeconds += FMath::Max(0.0f, DeltaSeconds);
        const float Distance = FVector::Dist2D(
            Runtime.BoundaryDrivingStartLocation, Vehicle->GetActorLocation());
        if (Runtime.BoundaryDrivingSeconds <
                EntryCollisionReleaseDelaySeconds ||
            Distance < EntryCollisionReleaseDistanceCm ||
            !IsVehicleClearForCollisionRestore(Vehicle))
            continue;
        Vehicle->SetActorEnableCollision(true);
        if (UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
            Movement->bDetectPhysicalObstacles = true;
        Runtime.bBoundaryCollisionSuppressed = false;
        UE_LOG(LogTemp, Display,
            TEXT("TMOP VehicleEntryReleased: '%s' restored collision %.0f cm beyond its staging point."),
            *Runtime.Profile.VehicleId.ToString(), Distance);
    }
}

void ATMOPHistoricalVehicleDirector::ApplyDeferredPlacedVehicleState(
    const int32 CurrentSecond)
{
    for (TPair<FName, FHistoricalVehicleRuntime>& Pair : RuntimeVehicles)
    {
        FHistoricalVehicleRuntime& Runtime = Pair.Value;
        ATMOPVehicleBase* Vehicle = Runtime.Vehicle.Get();
        if (!IsValid(Vehicle) || Runtime.bSpawnedByDirector)
        {
            continue;
        }

        const bool bShouldBeActive =
            ShouldSpawn(Runtime.Profile, false) &&
            Runtime.InitialSpawnSecond != INDEX_NONE &&
            Runtime.InitialSpawnSecond <= CurrentSecond;
        if (!bShouldBeActive)
        {
            Vehicle->SetActorHiddenInGame(true);
            Vehicle->SetActorEnableCollision(false);
            Vehicle->SetActorTickEnabled(false);
            Runtime.bDeferredPlacedVehicle = true;
            UnregisterVehicle(Vehicle);
        }
    }
}

void ATMOPHistoricalVehicleDirector::DiscoverPlacedVehicles()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return;
    }

    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
    {
        ATMOPVehicleBase* Vehicle = *It;
        if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone())
        {
            continue;
        }

        FHistoricalVehicleRuntime* Runtime = RuntimeVehicles.Find(Vehicle->VehicleId);
        if (Runtime == nullptr)
        {
            continue;
        }
        if (Runtime->Vehicle.IsValid() && Runtime->Vehicle.Get() != Vehicle)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Historical Vehicles: duplicate placed VehicleId '%s'."),
                *Vehicle->VehicleId.ToString());
            continue;
        }

        Runtime->Vehicle = Vehicle;
        Runtime->bSpawnedByDirector = false;
        Vehicle->DisplayName = Runtime->Profile.DisplayName;
        Vehicle->VehicleCategoryId = Runtime->Profile.CategoryId;
        Vehicle->SourceDocumentNumber = Runtime->Profile.SourceReference;
        Vehicle->EvidenceIcon = Runtime->Profile.EvidenceIcon;
        Vehicle->RegistrationStatus = Runtime->Profile.RegistrationStatus;
        Vehicle->RegistrationOrigin = Runtime->Profile.RegistrationOrigin;
        Vehicle->RegistrationNumber = Runtime->Profile.RegistrationNumber;
        Vehicle->RefreshNameLabel();
        if (ATMOPConfiguredVehicle* Configured =
            Cast<ATMOPConfiguredVehicle>(Vehicle))
        {
            Configured->VehicleModel = Runtime->Profile.ModelData;
            Configured->bOverrideBodyColor =
                Runtime->Profile.bOverrideBodyColor;
            Configured->BodyColor = Runtime->Profile.BodyColor;
            Configured->RoofAccessory = Runtime->Profile.RoofAccessory;
            Configured->AdditionalAccessories = Runtime->Profile.AdditionalAccessories;
            TMOPVehiclePresentation::ApplyProfile(Configured, Runtime->Profile);
        }
        else TMOPVehiclePresentation::ApplyProfile(Vehicle, Runtime->Profile);
        if (UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
            Movement->bFleeingVehicle = Runtime->Profile.bFleeingVehicle;
        RegisterVehicle(Vehicle);
    }
}

ATMOPVehicleBase* ATMOPHistoricalVehicleDirector::SpawnVehicle(
    FHistoricalVehicleRuntime& Runtime,
    const FTransform* SpawnTransformOverride)
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return nullptr;
    }

    TSubclassOf<ATMOPVehicleBase> SpawnClass = TMOPVehiclePresentation::ResolveClass(
        Runtime.Profile, DefaultVehicleClass.Get());
    if (SpawnClass.Get() == nullptr)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP Historical Vehicles: '%s' has no VehicleClass and no default."),
            *Runtime.Profile.VehicleId.ToString());
        return nullptr;
    }

    const FTransform InitialTransform = SpawnTransformOverride != nullptr
        ? *SpawnTransformOverride : GetInitialTransform(Runtime.Profile);
    ATMOPVehicleBase* Vehicle = World->SpawnActorDeferred<ATMOPVehicleBase>(
        SpawnClass, InitialTransform, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if (!IsValid(Vehicle))
    {
        return nullptr;
    }

    Vehicle->VehicleId = Runtime.Profile.VehicleId;
    Vehicle->DisplayName = Runtime.Profile.DisplayName;
    Vehicle->VehicleCategoryId = Runtime.Profile.CategoryId;
    Vehicle->SourceDocumentNumber = Runtime.Profile.SourceReference;
    Vehicle->EvidenceIcon = Runtime.Profile.EvidenceIcon;
    Vehicle->RegistrationStatus = Runtime.Profile.RegistrationStatus;
    Vehicle->RegistrationOrigin = Runtime.Profile.RegistrationOrigin;
    Vehicle->RegistrationNumber = Runtime.Profile.RegistrationNumber;
    if (ATMOPConfiguredVehicle* Configured = Cast<ATMOPConfiguredVehicle>(Vehicle))
    {
        Configured->VehicleModel = Runtime.Profile.ModelData;
        Configured->bOverrideBodyColor =
            Runtime.Profile.bOverrideBodyColor;
        Configured->BodyColor = Runtime.Profile.BodyColor;
        Configured->RoofAccessory = Runtime.Profile.RoofAccessory;
        Configured->AdditionalAccessories = Runtime.Profile.AdditionalAccessories;
    }
    UGameplayStatics::FinishSpawningActor(Vehicle, InitialTransform);
    if (!Cast<ATMOPConfiguredVehicle>(Vehicle))
        TMOPVehiclePresentation::ApplyProfile(Vehicle, Runtime.Profile);

    if (UTMOPTrafficVehicleMovementComponent* Movement =
        Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
        Movement->bFleeingVehicle = Runtime.Profile.bFleeingVehicle;

    // Blueprint construction may restore the spawned class defaults (commonly
    // the white Volvo 240 proxy). Re-apply the row after construction so the
    // DataTable remains authoritative for both model and body colour.
    if (ATMOPConfiguredVehicle* Configured =
        Cast<ATMOPConfiguredVehicle>(Vehicle))
    {
        Configured->VehicleModel = Runtime.Profile.ModelData;
        Configured->bOverrideBodyColor =
            Runtime.Profile.bOverrideBodyColor;
        Configured->BodyColor = Runtime.Profile.BodyColor;
        Configured->RoofAccessory = Runtime.Profile.RoofAccessory;
        Configured->AdditionalAccessories = Runtime.Profile.AdditionalAccessories;
        const bool bApplied = TMOPVehiclePresentation::ApplyProfile(Configured, Runtime.Profile);
        const bool bCorrectMesh =
            bApplied &&
            IsValid(Runtime.Profile.ModelData) &&
            IsValid(Runtime.Profile.ModelData->BodyMesh) &&
            IsValid(Configured->BodyMesh) &&
            Configured->BodyMesh->GetStaticMesh() ==
                Runtime.Profile.ModelData->BodyMesh;

        if (!bCorrectMesh)
        {
            if (IsValid(Configured->BodyMesh))
                Configured->BodyMesh->SetStaticMesh(nullptr);
            UE_LOG(LogTemp, Error,
                TEXT("TMOP Historical Vehicles: '%s' refused proxy mesh. ModelData='%s', expected BodyMesh='%s'."),
                *Runtime.Profile.VehicleId.ToString(),
                IsValid(Runtime.Profile.ModelData)
                    ? *Runtime.Profile.ModelData->GetPathName()
                    : TEXT("None"),
                IsValid(Runtime.Profile.ModelData) &&
                    IsValid(Runtime.Profile.ModelData->BodyMesh)
                    ? *Runtime.Profile.ModelData->BodyMesh->GetPathName()
                    : TEXT("None"));
        }
        else
        {
            UE_LOG(LogTemp, Display,
                TEXT("TMOP Historical Vehicles: '%s' uses ModelData '%s' and mesh '%s'."),
                *Runtime.Profile.VehicleId.ToString(),
                *Runtime.Profile.ModelData->GetPathName(),
                *Configured->BodyMesh->GetStaticMesh()->GetPathName());
        }
    }

    Runtime.Vehicle = Vehicle;
    Runtime.bSpawnedByDirector = true;
    RegisterVehicle(Vehicle);
    return Vehicle;
}

FTransform ATMOPHistoricalVehicleDirector::GetInitialTransform(
    const FTMOPHistoricalVehicleRow& Profile) const
{
    for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Profile.Timeline)
    {
        if (Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
            Entry.Action == ETMOPHistoricalVehicleAction::Spawn)
        {
            if (Entry.PlacementMode ==
                ETMOPHistoricalVehiclePlacementMode::Anchor)
            {
                UTMOPAnchorSubsystem* Anchors =
                    GetGameInstance() != nullptr
                    ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>()
                    : nullptr;
                if (Anchors != nullptr)
                {
                    // Vehicle and anchor BeginPlay order is not guaranteed.
                    Anchors->DiscoverAnchorsInWorld();
                }
                ATMOPHistoricalAnchor* Anchor =
                    Anchors != nullptr
                    ? Anchors->FindAnchor(Entry.PlacementAnchorId)
                    : nullptr;
                if (IsValid(Anchor))
                {
                    return Entry.AnchorLocalOffset *
                        FTransform(
                            Anchor->GetAnchorRotation(),
                            Anchor->GetAnchorLocation(),
                            FVector::OneVector);
                }
                UE_LOG(LogTemp, Error,
                    TEXT("TMOP Historical Vehicles: placement anchor '%s' for vehicle '%s' was not found; using World Transform fallback."),
                    *Entry.PlacementAnchorId.ToString(),
                    *Profile.VehicleId.ToString());
            }
            return Entry.WorldTransform;
        }
    }
    return FTransform::Identity;
}

bool ATMOPHistoricalVehicleDirector::ShouldSpawn(
    const FTMOPHistoricalVehicleRow& Profile,
    const bool bIgnoreRowFlag) const
{
    if (!bIgnoreRowFlag && bRespectRowSpawnFlags && !Profile.bSpawnInSimulation)
    {
        return false;
    }
    return Profile.Timeline.ContainsByPredicate(
        [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
        {
            return Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
        });
}

void ATMOPHistoricalVehicleDirector::RegisterVehicle(ATMOPVehicleBase* Vehicle) const
{
    if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone() || GetGameInstance() == nullptr)
    {
        return;
    }
    if (UTMOPWorldSubsystem* Registry =
        GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
    {
        Registry->RegisterWorldObject(
            Vehicle->VehicleId, HistoricalVehicleObjectType, Vehicle);
    }
}

void ATMOPHistoricalVehicleDirector::UnregisterVehicle(ATMOPVehicleBase* Vehicle) const
{
    if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone() || GetGameInstance() == nullptr)
    {
        return;
    }
    if (UTMOPWorldSubsystem* Registry =
        GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>())
    {
        Registry->UnregisterWorldObject(Vehicle->VehicleId, Vehicle);
    }
}

bool ATMOPHistoricalVehicleDirector::ValidateHistoricalVehicleTable(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!IsValid(HistoricalVehicleTable) ||
        HistoricalVehicleTable->GetRowStruct() !=
            FTMOPHistoricalVehicleRow::StaticStruct())
    {
        OutErrors.Add(
            TEXT("Historical Vehicle Table is missing or has the wrong row structure."));
        return false;
    }

    TSet<FName> VehicleIds;
    for (const TPair<FName, uint8*>& Pair : HistoricalVehicleTable->GetRowMap())
    {
        const FTMOPHistoricalVehicleRow* Row =
            reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
        if (Row == nullptr)
        {
            OutErrors.Add(FString::Printf(TEXT("Row '%s' is null."), *Pair.Key.ToString()));
            continue;
        }
        const FString Prefix = FString::Printf(TEXT("Row '%s'"), *Pair.Key.ToString());
        if (Row->VehicleId.IsNone())
        {
            OutErrors.Add(Prefix + TEXT(" has no VehicleId."));
        }
        else
        {
            if (Pair.Key != Row->VehicleId)
            {
                OutErrors.Add(Prefix + TEXT(" does not match its VehicleId."));
            }
            if (VehicleIds.Contains(Row->VehicleId))
            {
                OutErrors.Add(Prefix + TEXT(" duplicates another VehicleId."));
            }
            VehicleIds.Add(Row->VehicleId);
        }
        if (Row->bSpawnInSimulation && Row->Timeline.IsEmpty())
        {
            OutErrors.Add(Prefix +
                TEXT(" is enabled for simulation but has no Timeline entries."));
        }
        if (Row->bOverrideBodyColor && !IsValid(Row->ModelData.Get()))
        {
            OutErrors.Add(Prefix +
                TEXT(" overrides Body Color but has no Model Data."));
        }
        if (Row->RegistrationStatus ==
                ETMOPVehicleRegistrationStatus::Known)
        {
            if (Row->RegistrationNumber.IsEmpty())
                OutErrors.Add(Prefix +
                    TEXT(" has Known registration status but no registration number."));
            if (Row->RegistrationOrigin ==
                    ETMOPVehicleRegistrationOrigin::Unknown)
                OutErrors.Add(Prefix +
                    TEXT(" has a known registration number but unknown registration origin."));
            if (Row->RegistrationOrigin ==
                ETMOPVehicleRegistrationOrigin::Swedish)
            {
                const FString& Number = Row->RegistrationNumber;
                const bool bSwedishFormat = Number.Len() == 7 &&
                    FChar::IsAlpha(Number[0]) &&
                    FChar::IsAlpha(Number[1]) &&
                    FChar::IsAlpha(Number[2]) &&
                    Number[3] == TEXT('-') &&
                    FChar::IsDigit(Number[4]) &&
                    FChar::IsDigit(Number[5]) &&
                    FChar::IsDigit(Number[6]);
                if (!bSwedishFormat)
                    OutErrors.Add(Prefix +
                        TEXT(" has a Swedish registration that is not formatted ABC-123."));
            }
        }
        TSet<FName> TimelineEntryIds;
        for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Row->Timeline)
        {
            if (Entry.EntryId.IsNone() || TimelineEntryIds.Contains(Entry.EntryId))
                OutErrors.Add(Prefix + FString::Printf(TEXT(" has an empty or duplicate timeline Entry ID '%s'."), *Entry.EntryId.ToString()));
            TimelineEntryIds.Add(Entry.EntryId);
            const bool bDrivingEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::BeginDriving ||
                Entry.Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
            const bool bPlacementEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn ||
                Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                Entry.Action == ETMOPHistoricalVehicleAction::Park ||
                Entry.Action ==
                    ETMOPHistoricalVehicleAction::OffscreenTransfer;
            if (bPlacementEntry &&
                Entry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor &&
                Entry.PlacementAnchorId.IsNone())
            {
                OutErrors.Add(Prefix +
                    TEXT(" has anchor placement without a Placement Anchor ID."));
            }
            if (Entry.Action ==
                    ETMOPHistoricalVehicleAction::OffscreenTransfer &&
                Entry.OffscreenTransferDurationSeconds < 0)
            {
                OutErrors.Add(Prefix +
                    TEXT(" has an Offscreen Transfer with a negative duration."));
            }
            if (bDrivingEntry)
            {
                const FString EntryPrefix = Prefix + FString::Printf(
                    TEXT(" auto-start entry '%s'"), *Entry.EntryId.ToString());
                if (Entry.EntryId.IsNone())
                    OutErrors.Add(EntryPrefix + TEXT(" has no Entry ID."));
                if (TMOPVehicleRoute::Driver(*Row, Entry).IsNone())
                    OutErrors.Add(EntryPrefix + TEXT(" has no Driver Entity ID."));
                if (Entry.VehicleRouteMode ==
                        ETMOPVehicleRouteMode::ManualLaneRoute &&
                    Entry.OrderedLaneIds.IsEmpty())
                {
                    OutErrors.Add(EntryPrefix +
                        TEXT(" uses Manual Lane Route but has no lane IDs."));
                }
                if (Entry.VehicleRouteMode !=
                        ETMOPVehicleRouteMode::ManualLaneRoute &&
                    Entry.RouteDestinationAnchorId.IsNone())
                {
                    OutErrors.Add(EntryPrefix +
                        TEXT(" uses automatic routing but has no destination anchor."));
                }
            }
        }
    }
    return OutErrors.IsEmpty();
}

ATMOPVehicleBase* ATMOPHistoricalVehicleDirector::FindHistoricalVehicle(
    const FName VehicleId) const
{
    const FHistoricalVehicleRuntime* Runtime = RuntimeVehicles.Find(VehicleId);
    return Runtime != nullptr ? Runtime->Vehicle.Get() : nullptr;
}

const FTMOPHistoricalVehicleTimelineEntry*
ATMOPHistoricalVehicleDirector::FindDrivingEntry(
    const FTMOPHistoricalVehicleRow& Profile,
    const FName DriverEntityId,
    const int32 CurrentSecond) const
{
    const FTMOPHistoricalVehicleTimelineEntry* BestEntry = nullptr;
    int32 LatestDeparture = INDEX_NONE;
    for (int32 Index = 0; Index < Profile.Timeline.Num(); ++Index)
    {
        const auto& Entry = Profile.Timeline[Index];
        if (!TMOPVehicleRoute::IsDriving(Entry.Action) ||
            TMOPVehicleRoute::Driver(Profile, Entry) != DriverEntityId) continue;
        int32 Departure, Arrival;
        if (!ResolveDrivingWindow(Profile, Index, Departure, Arrival) ||
            !TMOPVehicleRouteMath::IsActive(CurrentSecond, Departure, Arrival)) continue;
        if (Departure > LatestDeparture)
        { LatestDeparture = Departure; BestEntry = &Entry; }
    }
    return BestEntry;
}

bool ATMOPHistoricalVehicleDirector::ResolveDrivingWindow(
    const FTMOPHistoricalVehicleRow& Profile, int32 Index,
    int32& Departure, int32& Arrival) const
{
    if (!ResolveDrivingDepartureSecond(Profile, Index, Departure)) return false;
    if (Profile.Timeline[Index].bTimeIsArrival)
        return ResolveTimelineEntrySecond(Profile, Index, Arrival) && Arrival > Departure;
    for (int32 Next = Index + 1; Next < Profile.Timeline.Num(); ++Next)
    {
        if (TMOPVehicleRoute::IsDriving(Profile.Timeline[Next].Action)) break;
        if (TMOPVehicleRoute::IsStop(Profile.Timeline[Next].Action))
            return ResolveTimelineEntrySecond(Profile, Next, Arrival) && Arrival > Departure;
    }
    return false;
}

FString ATMOPHistoricalVehicleDirector::GetTimelineFingerprint(FName VehicleId, FName EntryId) const
{
    const auto* Runtime = RuntimeVehicles.Find(VehicleId);
    if (!Runtime) return FString();
    int32 Index = Runtime->Profile.Timeline.IndexOfByPredicate(
        [EntryId](const FTMOPHistoricalVehicleTimelineEntry& Entry) { return Entry.EntryId == EntryId; });
    for (; Index >= 0; --Index)
        if (TMOPVehicleRoute::IsDriving(Runtime->Profile.Timeline[Index].Action))
        {
            int32 Departure, Arrival; FTMOPVehicleRoutePlan Plan; FString Failure;
            if (ResolveDrivingWindow(Runtime->Profile, Index, Departure, Arrival) &&
                TMOPVehicleRoute::Build(GetWorld(), Runtime->Profile, Index, Plan, Failure))
                return TMOPVehicleRoute::Fingerprint(Runtime->Profile, Plan, Departure, Arrival);
            break;
        }
    return FString();
}

bool ATMOPHistoricalVehicleDirector::GetLastDrivingFailure(
    const FName VehicleId,
    FString& OutFailureCode,
    FString& OutFailureDetails) const
{
    const FString* Code = LastDrivingFailureCodes.Find(VehicleId);
    const FString* Details = LastDrivingFailureDetails.Find(VehicleId);
    OutFailureCode = Code != nullptr ? *Code : FString();
    OutFailureDetails = Details != nullptr ? *Details : FString();
    return Code != nullptr && !Code->IsEmpty();
}

bool ATMOPHistoricalVehicleDirector::ReportDrivingFailure(
    const FName VehicleId,
    const FString& FailureCode,
    const FString& FailureDetails)
{
    if (LastDrivingFailureCodes.FindRef(VehicleId) == FailureCode &&
        LastDrivingFailureDetails.FindRef(VehicleId) == FailureDetails) return false;
    LastDrivingFailureCodes.Add(VehicleId, FailureCode);
    LastDrivingFailureDetails.Add(VehicleId, FailureDetails);
    UE_LOG(LogTemp, Error, TEXT("TMOP driving [%s]: %s"),
        *FailureCode, *FailureDetails);
    return false;
}

bool ATMOPHistoricalVehicleDirector::BeginDrivingVehicle(
    const FName VehicleId,
    const FName DriverEntityId,
    const TArray<FName>& OrderedLaneIds,
    const TArray<FName>& PassAnchorIds,
    const ETMOPVehicleRouteMode RouteMode,
    const FName DestinationAnchorId,
    const float StartDistanceAlongFirstLaneCm,
    const bool bUseTimelineRouteOverride)
{
    if (RuntimeVehicles.IsEmpty())
        InitializeHistoricalVehicles();

    FHistoricalVehicleRuntime* Runtime = RuntimeVehicles.Find(VehicleId);
    ATMOPVehicleBase* Vehicle =
        Runtime != nullptr ? Runtime->Vehicle.Get() : nullptr;
    if (Runtime == nullptr || !IsValid(Vehicle))
    {
        return ReportDrivingFailure(VehicleId, TEXT("VehicleNotSpawned"),
            FString::Printf(TEXT("Vehicle '%s' is not spawned."),
                *VehicleId.ToString()));
    }
    if (Runtime->ActiveOffscreenTransferEntryIndex != INDEX_NONE)
    {
        return ReportDrivingFailure(VehicleId,
            TEXT("VehicleOffscreenTransferActive"),
            FString::Printf(
                TEXT("Vehicle '%s' is still in an offscreen transfer."),
                *VehicleId.ToString()));
    }

    ATMOPHistoricalAgent* Driver = Vehicle->GetDriverAgent();
    const FName OccupantId =
        IsValid(Driver) && IsValid(Driver->EntityIdentity)
        ? Driver->EntityIdentity->EntityId : NAME_None;
    if (DriverEntityId.IsNone() || OccupantId != DriverEntityId)
    {
        return ReportDrivingFailure(VehicleId, TEXT("DriverSeatMismatch"),
            FString::Printf(TEXT("'%s' is not in driver seat of '%s'; occupant is '%s'."),
                *DriverEntityId.ToString(), *VehicleId.ToString(),
                *OccupantId.ToString()));
    }
    const UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight() : 0;
    const FName* RequestedEntryId =
        RequestedDrivingEntryOverrides.Find(VehicleId);
    const FTMOPHistoricalVehicleTimelineEntry* DrivingEntry =
        !bUseTimelineRouteOverride ? nullptr : RequestedEntryId != nullptr
        ? Runtime->Profile.Timeline.FindByPredicate(
            [RequestedEntryId](const FTMOPHistoricalVehicleTimelineEntry& Entry)
            {
                return Entry.EntryId == *RequestedEntryId;
            })
        : FindDrivingEntry(Runtime->Profile, DriverEntityId, CurrentSecond);
    if (DrivingEntry && TMOPVehicleRoute::Driver(Runtime->Profile, *DrivingEntry) != DriverEntityId)
        return ReportDrivingFailure(VehicleId, TEXT("WrongRouteDriver"), TEXT("The seated driver does not match this driving row."));
    if (DrivingEntry != nullptr && DrivingEntry->bWaitForListedOccupants)
    {
        TSet<FName> SeatedEntityIds;
        for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
        {
            ATMOPHistoricalAgent* Agent = IsValid(Seat)
                ? Seat->GetOccupant() : nullptr;
            if (IsValid(Agent) && IsValid(Agent->EntityIdentity))
                SeatedEntityIds.Add(Agent->EntityIdentity->EntityId);
        }
        TArray<FName> RequiredEntityIds = DrivingEntry->PassengerEntityIds;
        if (!DrivingEntry->DriverEntityId.IsNone())
            RequiredEntityIds.AddUnique(DrivingEntry->DriverEntityId);
        for (const FName RequiredId : RequiredEntityIds)
            if (!RequiredId.IsNone() && !SeatedEntityIds.Contains(RequiredId))
                return ReportDrivingFailure(VehicleId,
                    TEXT("WaitingForListedOccupants"),
                    FString::Printf(TEXT("'%s' is not seated in '%s' yet."),
                        *RequiredId.ToString(), *VehicleId.ToString()));
    }
    FTMOPHistoricalVehicleRow LegacyProfile;
    const FTMOPHistoricalVehicleRow* RouteProfile = &Runtime->Profile;
    int32 DrivingIndex = DrivingEntry ? int32(DrivingEntry - Runtime->Profile.Timeline.GetData()) : INDEX_NONE;
    int32 Departure = CurrentSecond, Arrival = INDEX_NONE;
    if (DrivingEntry)
    {
        if (!ResolveDrivingWindow(*RouteProfile, DrivingIndex, Departure, Arrival) ||
            !TMOPVehicleRouteMath::IsActive(CurrentSecond, Departure, Arrival))
            return ReportDrivingFailure(VehicleId, TEXT("OutsideDrivingWindow"),
                TEXT("This route has not started yet or its arrival deadline has already passed."));
        if (Runtime->AppliedDrivingEntryIds.Contains(DrivingEntry->EntryId))
            return true; // Idempotent legacy + vehicle-timeline calls.
    }
    else
    {
        if (bUseTimelineRouteOverride && Runtime->Profile.Timeline.ContainsByPredicate(
            [](const FTMOPHistoricalVehicleTimelineEntry& Entry)
            { return TMOPVehicleRoute::IsDriving(Entry.Action); }))
            return ReportDrivingFailure(VehicleId, TEXT("NoActiveVehicleRoute"),
                TEXT("The vehicle timeline is authoritative and has no active route now."));
        LegacyProfile = Runtime->Profile; LegacyProfile.Timeline.Reset();
        FTMOPHistoricalVehicleTimelineEntry Placement;
        Placement.WorldTransform = Vehicle->GetActorTransform(); LegacyProfile.Timeline.Add(Placement);
        FTMOPHistoricalVehicleTimelineEntry LegacyDrive;
        LegacyDrive.Action = ETMOPHistoricalVehicleAction::BeginDriving;
        LegacyDrive.VehicleRouteMode = RouteMode;
        LegacyDrive.OrderedLaneIds = OrderedLaneIds;
        LegacyDrive.RouteViaAnchorIds = PassAnchorIds;
        LegacyDrive.RouteDestinationAnchorId = DestinationAnchorId;
        LegacyDrive.RouteStartDistanceAlongFirstLaneCm = StartDistanceAlongFirstLaneCm;
        LegacyProfile.Timeline.Add(LegacyDrive); RouteProfile = &LegacyProfile; DrivingIndex = 1;
    }
    FTMOPVehicleRoutePlan Plan; FString Failure;
    if (!TMOPVehicleRoute::Build(GetWorld(), *RouteProfile, DrivingIndex, Plan, Failure))
        return ReportDrivingFailure(VehicleId, TEXT("InvalidRoutePlan"), Failure);
    if (Arrival == INDEX_NONE)
        Arrival = Departure + FMath::Max(1, FMath::CeilToInt(Plan.LengthCm / (35.0 / 0.036)));
    const auto& Entry = RouteProfile->Timeline[DrivingIndex];
    auto* Movement = Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    if (!Movement)
    {
        Movement = NewObject<UTMOPTrafficVehicleMovementComponent>(Vehicle, TEXT("HistoricalTrafficMovement"));
        Vehicle->AddInstanceComponent(Movement); Movement->bStartDrivingAutomatically = false;
        Movement->RegisterComponent();
    }
    Movement->bFleeingVehicle = Runtime->Profile.bFleeingVehicle ||
        Entry.DrivingPreset == ETMOPVehicleDrivingPreset::Fleeing;
    Movement->bIgnoreOneWayRestrictions = Entry.bIgnoreOneWayRestrictions;
    Movement->bRunRedLights = Entry.bRunRedLights;
    // Timed routes have one authority: distance / scheduled time. Presets do
    // not secretly replace a deadline with their own arrival estimate.
    Movement->DesiredCruiseSpeedKmh = float(Plan.LengthCm * 0.036 / (Arrival - Departure));
    Movement->bDespawnAtRouteEnd = false;
    if (!Movement->StartRoutePlan(Plan, Departure, Arrival, TimelineCatchUpMaximumSpeedKmh,
        Runtime->bReconstructCurrentRoute && ReconstructionSecond > Departure && ReconstructionSecond < Arrival, Entry.bStopAtViaAnchors))
        return ReportDrivingFailure(VehicleId, TEXT("RouteStartBlocked"),
            TEXT("Could not start the route. Check its start position, lanes and physical clearance."));
    Runtime->bReconstructCurrentRoute = false;
    Movement->ActiveTimelineFingerprint = TMOPVehicleRoute::Fingerprint(*RouteProfile, Plan, Departure, Arrival);
    if (DrivingEntry) Runtime->AppliedDrivingEntryIds.Add(DrivingEntry->EntryId);
    if (Runtime->bBoundaryCollisionSuppressed)
    {
        Movement->bDetectPhysicalObstacles = false; Vehicle->SetActorEnableCollision(false);
        Runtime->bBoundaryVehicleHasStartedDriving = true;
        Runtime->BoundaryDrivingSeconds = 0.0f;
        Runtime->BoundaryDrivingStartLocation = Vehicle->GetActorLocation();
    }
    LastDrivingFailureCodes.Remove(VehicleId);
    LastDrivingFailureDetails.Remove(VehicleId);
    UE_LOG(LogTemp, Display, TEXT("TMOP route '%s': %d -> %d, %.1f m, %d lanes."),
        *Entry.EntryId.ToString(), Departure, Arrival, Plan.LengthCm / 100.0, Plan.LaneIds.Num());
    return true;
}
