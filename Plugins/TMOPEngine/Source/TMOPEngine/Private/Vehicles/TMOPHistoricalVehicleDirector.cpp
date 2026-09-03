#include "Vehicles/TMOPHistoricalVehicleDirector.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
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
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "World/TMOPWorldSubsystem.h"

namespace
{
    const FName HistoricalVehicleObjectType(TEXT("HistoricalVehicle"));

    float SpeedForPreset(const ETMOPVehicleDrivingPreset Preset)
    {
        switch (Preset)
        {
        case ETMOPVehicleDrivingPreset::Parking: return 8.0f;
        case ETMOPVehicleDrivingPreset::SlowCity: return 20.0f;
        case ETMOPVehicleDrivingPreset::NormalCity: return 35.0f;
        case ETMOPVehicleDrivingPreset::Fast: return 60.0f;
        case ETMOPVehicleDrivingPreset::Emergency: return 90.0f;
        case ETMOPVehicleDrivingPreset::Fleeing: return 110.0f;
        case ETMOPVehicleDrivingPreset::AutomaticFromTimeline:
        default: return 0.0f;
        }
    }
}

bool ATMOPHistoricalVehicleDirector::ResolveTimelineEntrySecond(
    const FTMOPHistoricalVehicleRow& Profile,
    const int32 EntryIndex,
    int32& OutSecond) const
{
    if (!Profile.Timeline.IsValidIndex(EntryIndex)) return false;
    const FTMOPHistoricalVehicleTimelineEntry& Entry =
        Profile.Timeline[EntryIndex];
    if (Entry.TimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
    {
        int32 PreviousSecond = INDEX_NONE;
        if (EntryIndex <= 0 ||
            !ResolveTimelineEntrySecond(
                Profile, EntryIndex - 1, PreviousSecond))
            return false;
        OutSecond = FMath::Max(0,
            PreviousSecond + Entry.EventOffsetSeconds);
        return true;
    }
    if (Entry.TimingMode != ETMOPEventTimingMode::Relative)
    {
        OutSecond = Entry.Time.ToSecondsFromMidnight();
        return true;
    }
    if (Entry.SharedEventId.IsNone() || GetGameInstance() == nullptr)
        return false;
    const UTMOPHistoricalEventSubsystem* Events =
        GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>();
    FTMOPHistoricalEventRuntime EventRuntime;
    if (!IsValid(Events) ||
        !Events->TryGetEventRuntime(Entry.SharedEventId, EventRuntime) ||
        !EventRuntime.bHasResolvedTime)
        return false;
    OutSecond = EventRuntime.ResolvedTime.ToSecondsFromMidnight() +
        Entry.EventOffsetSeconds;
    return true;
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
    if (LastEvaluatedSecond != INDEX_NONE &&
        CurrentSecond < LastEvaluatedSecond)
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
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                Entry.Action == ETMOPHistoricalVehicleAction::Park;
            if (!bTimedPlacement) continue;
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
            if (UTMOPTrafficVehicleMovementComponent* Movement =
                Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
            {
                // A scheduled Stop/Park is a target time, not permission to
                // snap a delayed car away from its lane. Let an already
                // configured final approach finish naturally and update its
                // exact parking transform from this timeline entry.
                if ((Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                     Entry.Action == ETMOPHistoricalVehicleAction::Park) &&
                    Movement->UpdateFinalApproachTarget(Target))
                {
                    Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
                    UE_LOG(LogTemp, Display,
                        TEXT("TMOP VehicleTimedPlacement: '%s' deferred '%s' to its smooth final approach."),
                        *Runtime.Profile.VehicleId.ToString(),
                        *Entry.EntryId.ToString());
                    continue;
                }
                if (Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                    Entry.Action == ETMOPHistoricalVehicleAction::Park)
                {
                    const float CorrectionDistanceCm = FVector::Dist2D(
                        Vehicle->GetActorLocation(), Target.GetLocation());
                    if (!bAllowDistantTimedParkingTeleport &&
                        CorrectionDistanceCm > TimedParkingAlignmentToleranceCm)
                    {
                        // A timetable is not permission to hide an invalid or
                        // blocked route. Stop where the vehicle really is and
                        // let validation expose the missed parking anchor.
                        // Keep the planned route incomplete so occupants do
                        // not disembark at the wrong location.
                        Movement->StopDriving();
                        Runtime.AppliedPlacementEntryIds.Add(Entry.EntryId);
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
            UE_LOG(LogTemp, Display,
                TEXT("TMOP VehicleTimedPlacement: '%s' applied '%s' at %02d:%02d:%02d."),
                *Runtime.Profile.VehicleId.ToString(), *Entry.EntryId.ToString(),
                FTMOPTime::FromSecondsFromMidnight(EntrySecond).Hour,
                FTMOPTime::FromSecondsFromMidnight(EntrySecond).Minute,
                FTMOPTime::FromSecondsFromMidnight(EntrySecond).Second);
        }
    }
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
    LastEvaluatedSecond = CurrentSecond;
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
            if (bBoundarySpawn && FindClearBoundarySpawnTransform(
                    ClearSpawnTransform, QueuedTransform))
                ClearSpawnTransform = QueuedTransform;
            else if (bBoundarySpawn)
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
            Runtime.LastAppliedLifecycleEntryIndex = DueLifecycleIndex;
            UE_LOG(LogTemp, Display,
                TEXT("TMOP Historical Vehicles: lifecycle spawn %d applied for '%s'."),
                DueLifecycleIndex, *Runtime.Profile.VehicleId.ToString());
        }
        if (Runtime.Vehicle.IsValid())
        {
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
            if (bBoundaryEntry && !bOccupiedAtInitialPlacement)
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
                        const int32 DepartureIndex = Later.bTimeIsArrival
                            ? LaterIndex - 1 : LaterIndex;
                        if (!ResolveTimelineEntrySecond(
                                Profile, DepartureIndex, DrivingSecond))
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
            Configured->ApplyConfiguration();
        }
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

    // A valid ModelData row must never fall back to a Blueprint's proxy mesh.
    // If no explicit class resolves, use the native configured vehicle.
    TSubclassOf<ATMOPVehicleBase> SpawnClass = DefaultVehicleClass;
    if (IsValid(Runtime.Profile.ModelData))
    {
        SpawnClass = ATMOPConfiguredVehicle::StaticClass();
    }
    if (UClass* RowClass = Runtime.Profile.VehicleClass.Get())
    {
        if (RowClass->IsChildOf(ATMOPVehicleBase::StaticClass()))
        {
            // Configured Blueprint subclasses can contain a permanently visible
            // proxy mesh. ModelData is authoritative, so use the clean native
            // configured actor for that case. Bespoke non-configured classes
            // (bus, ambulance, etc.) remain supported.
            SpawnClass =
                IsValid(Runtime.Profile.ModelData) &&
                RowClass->IsChildOf(ATMOPConfiguredVehicle::StaticClass())
                ? ATMOPConfiguredVehicle::StaticClass()
                : RowClass;
        }
        else
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Historical Vehicles: class on '%s' is not a TMOPVehicleBase."),
                *Runtime.Profile.VehicleId.ToString());
        }
    }
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
    }
    UGameplayStatics::FinishSpawningActor(Vehicle, InitialTransform);

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
        const bool bApplied = Configured->ApplyConfiguration();
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
        for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Row->Timeline)
        {
            const bool bPlacementEntry =
                Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
                Entry.Action == ETMOPHistoricalVehicleAction::Spawn;
            if (bPlacementEntry &&
                Entry.PlacementMode ==
                    ETMOPHistoricalVehiclePlacementMode::Anchor &&
                Entry.PlacementAnchorId.IsNone())
            {
                OutErrors.Add(Prefix +
                    TEXT(" has anchor placement without a Placement Anchor ID."));
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
    int32 BestDifference = MAX_int32;
    for (int32 Index = 0; Index < Profile.Timeline.Num(); ++Index)
    {
        const FTMOPHistoricalVehicleTimelineEntry& Entry =
            Profile.Timeline[Index];
        const bool bDrivingAction =
            Entry.Action == ETMOPHistoricalVehicleAction::BeginDriving ||
            Entry.Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
        const bool bDriverMatches = Entry.DriverEntityId.IsNone() ||
            Entry.DriverEntityId == DriverEntityId;
        if (!bDrivingAction || !bDriverMatches) continue;
        int32 DepartureSecond = INDEX_NONE;
        const int32 DepartureIndex = Entry.bTimeIsArrival ? Index - 1 : Index;
        if (!ResolveTimelineEntrySecond(Profile, DepartureIndex, DepartureSecond))
            continue;
        const int32 Difference = FMath::Abs(DepartureSecond - CurrentSecond);
        if (Difference < BestDifference)
        {
            BestDifference = Difference;
            BestEntry = &Entry;
        }
    }
    return BestEntry;
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
    const float StartDistanceAlongFirstLaneCm)
{
    LastDrivingFailureCodes.Remove(VehicleId);
    LastDrivingFailureDetails.Remove(VehicleId);
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
    if (!Runtime->Profile.KnownDriverEntityId.IsNone() &&
        Runtime->Profile.KnownDriverEntityId != DriverEntityId)
    {
        return ReportDrivingFailure(VehicleId, TEXT("KnownDriverMismatch"),
            FString::Printf(TEXT("'%s' does not match known driver '%s' for '%s'."),
                *DriverEntityId.ToString(),
                *Runtime->Profile.KnownDriverEntityId.ToString(),
                *VehicleId.ToString()));
    }

    const UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight() : 0;
    const FTMOPHistoricalVehicleTimelineEntry* DrivingEntry =
        FindDrivingEntry(Runtime->Profile, DriverEntityId, CurrentSecond);
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
    TArray<FName> Route = OrderedLaneIds;
    ETMOPVehicleRouteMode EffectiveRouteMode = RouteMode;
    if (DrivingEntry != nullptr &&
        !DrivingEntry->OrderedLaneIds.IsEmpty())
    {
        // An editor-generated vehicle route is authoritative when the person
        // entry does not provide its own route.
        Route = DrivingEntry->OrderedLaneIds;
        EffectiveRouteMode = ETMOPVehicleRouteMode::ManualLaneRoute;
    }
    Route.RemoveAll([](const FName LaneId) { return LaneId.IsNone(); });

    FName ResolvedDestinationAnchorId = DrivingEntry != nullptr &&
        !DrivingEntry->RouteDestinationAnchorId.IsNone()
        ? DrivingEntry->RouteDestinationAnchorId : DestinationAnchorId;
    TArray<FName> ResolvedPassAnchorIds = PassAnchorIds;
    if (DrivingEntry != nullptr && !DrivingEntry->RouteViaAnchorIds.IsEmpty())
        ResolvedPassAnchorIds = DrivingEntry->RouteViaAnchorIds;

    float ResolvedStartDistance = StartDistanceAlongFirstLaneCm;
    FName FinalDestinationLaneId = NAME_None;
    float FinalDestinationLaneDistance = 0.0f;
    FTransform FinalDestinationTransform = FTransform::Identity;
    if (EffectiveRouteMode != ETMOPVehicleRouteMode::ManualLaneRoute)
    {
        if (ResolvedDestinationAnchorId.IsNone() || GetGameInstance() == nullptr)
        {
            return ReportDrivingFailure(VehicleId,
                TEXT("AutomaticDestinationMissing"),
                FString::Printf(TEXT("Automatic route for '%s' needs a destination anchor ID."),
                    *VehicleId.ToString()));
        }
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Destination =
            Anchors != nullptr
            ? Anchors->FindAnchor(ResolvedDestinationAnchorId) : nullptr;
        UTMOPTrafficNetworkSubsystem* Network =
            GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>();
        if (!IsValid(Destination) || Network == nullptr)
        {
            return ReportDrivingFailure(VehicleId,
                !IsValid(Destination) ? TEXT("DestinationAnchorUnavailable")
                    : TEXT("TrafficNetworkUnavailable"),
                FString::Printf(TEXT("Destination anchor '%s' valid=%s; traffic network valid=%s."),
                    *ResolvedDestinationAnchorId.ToString(),
                    IsValid(Destination) ? TEXT("true") : TEXT("false"),
                    Network != nullptr ? TEXT("true") : TEXT("false")));
        }
        Network->DiscoverLanesInWorld();

        FName RouteStartLaneId;
        if (EffectiveRouteMode == ETMOPVehicleRouteMode::ManualThenAutomatic &&
            !Route.IsEmpty())
        {
            RouteStartLaneId = Route.Last();
        }
        else
        {
            float NearestStartDistance = 0.0f;
            RouteStartLaneId = DrivingEntry != nullptr
                ? DrivingEntry->RouteStartLaneId : NAME_None;
            const bool bHasExplicitStartLane =
                !RouteStartLaneId.IsNone() &&
                IsValid(Network->FindLane(RouteStartLaneId));
            if (!bHasExplicitStartLane && !Network->FindNearestLane(
                    Vehicle->GetActorLocation(),
                    RouteStartLaneId,
                    NearestStartDistance))
            {
                return ReportDrivingFailure(VehicleId,
                    TEXT("StartLaneNotFound"),
                    FString::Printf(TEXT("No start lane was found near '%s' at %s."),
                        *VehicleId.ToString(),
                        *Vehicle->GetActorLocation().ToCompactString()));
            }
            Route.Reset();
            if (StartDistanceAlongFirstLaneCm <= 0.0f)
            {
                ResolvedStartDistance = NearestStartDistance;
            }
        }

        TArray<FName> RouteAnchorIds = ResolvedPassAnchorIds;
        RouteAnchorIds.RemoveAll(
            [](const FName AnchorId) { return AnchorId.IsNone(); });
        RouteAnchorIds.Add(ResolvedDestinationAnchorId);

        FName SegmentStartLaneId = RouteStartLaneId;
        for (const FName RouteAnchorId : RouteAnchorIds)
        {
            ATMOPHistoricalAnchor* RouteAnchor =
                Anchors->FindAnchor(RouteAnchorId);
            if (!IsValid(RouteAnchor))
            {
                return ReportDrivingFailure(VehicleId,
                    TEXT("RouteAnchorUnavailable"),
                    FString::Printf(TEXT("Pass/destination anchor '%s' is unavailable."),
                        *RouteAnchorId.ToString()));
            }

            TArray<FName> AutomaticSegment;
            FName SegmentDestinationLaneId;
            float SegmentDestinationDistance = 0.0f;
            if (!Network->FindNearestReachableLane(
                RouteAnchor->GetAnchorLocation(),
                SegmentStartLaneId,
                SegmentDestinationLaneId,
                SegmentDestinationDistance,
                AutomaticSegment))
            {
                return ReportDrivingFailure(VehicleId,
                    TEXT("LaneRouteDisconnected"),
                    FString::Printf(TEXT("No reachable lane candidate from '%s' through anchor '%s'."),
                        *SegmentStartLaneId.ToString(),
                        *RouteAnchorId.ToString()));
            }
            if (!Route.IsEmpty() && !AutomaticSegment.IsEmpty() &&
                Route.Last() == AutomaticSegment[0])
            {
                AutomaticSegment.RemoveAt(0);
            }
            Route.Append(AutomaticSegment);
            SegmentStartLaneId = SegmentDestinationLaneId;
            if (RouteAnchorId == ResolvedDestinationAnchorId)
            {
                FinalDestinationLaneId = SegmentDestinationLaneId;
                FinalDestinationLaneDistance = SegmentDestinationDistance;
                FinalDestinationTransform = FTransform(
                    RouteAnchor->GetAnchorRotation(),
                    RouteAnchor->GetAnchorLocation(),
                    FVector::OneVector);
            }
        }
        UE_LOG(LogTemp, Display,
            TEXT("TMOP driving: calculated %d lane(s) through %d pass anchor(s) to '%s'."),
            Route.Num(), ResolvedPassAnchorIds.Num(),
            *ResolvedDestinationAnchorId.ToString());
    }

    // Manual lane routes also need their exact parking transform. Previously
    // DestinationAnchorId was ignored in this mode, so emergency vehicles
    // drove to the end of the last lane and a timed Stop later teleported them
    // back. Use the closest point on the final supplied lane and the same
    // smooth final approach as automatic routing.
    if (EffectiveRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute &&
        !ResolvedDestinationAnchorId.IsNone() && !Route.IsEmpty() &&
        GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        UTMOPTrafficNetworkSubsystem* Network =
            GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>();
        ATMOPHistoricalAnchor* Destination = Anchors != nullptr
            ? Anchors->FindAnchor(ResolvedDestinationAnchorId) : nullptr;
        if (Network != nullptr) Network->DiscoverLanesInWorld();
        UTMOPTrafficLaneComponent* FinalLane = Network != nullptr
            ? Network->FindLane(Route.Last()) : nullptr;
        if (!IsValid(Destination) || !IsValid(FinalLane))
        {
            return ReportDrivingFailure(VehicleId,
                !IsValid(Destination)
                    ? TEXT("ManualDestinationAnchorUnavailable")
                    : TEXT("ManualFinalLaneUnavailable"),
                FString::Printf(
                    TEXT("Manual route for '%s' cannot resolve destination '%s' on final lane '%s'."),
                    *VehicleId.ToString(),
                    *ResolvedDestinationAnchorId.ToString(),
                    *Route.Last().ToString()));
        }
        FinalDestinationLaneId = Route.Last();
        const float DestinationInputKey =
            FinalLane->FindInputKeyClosestToWorldLocation(
                Destination->GetAnchorLocation());
        FinalDestinationLaneDistance =
            FinalLane->GetDistanceAlongSplineAtSplineInputKey(
                DestinationInputKey);
        FinalDestinationTransform = FTransform(
            Destination->GetAnchorRotation(),
            Destination->GetAnchorLocation(), FVector::OneVector);
    }

    if (Route.IsEmpty())
    {
        return ReportDrivingFailure(VehicleId, TEXT("OrderedLaneRouteEmpty"),
            FString::Printf(TEXT("Vehicle '%s' has no ordered lane route."),
                *VehicleId.ToString()));
    }

    UTMOPTrafficVehicleMovementComponent* Movement =
        Vehicle->FindComponentByClass<
            UTMOPTrafficVehicleMovementComponent>();
    if (!IsValid(Movement))
    {
        Movement = NewObject<UTMOPTrafficVehicleMovementComponent>(
            Vehicle, TEXT("HistoricalTrafficMovement"));
        Vehicle->AddInstanceComponent(Movement);
        Movement->bStartDrivingAutomatically = false;
        Movement->RegisterComponent();
    }

    Movement->StopDriving();
    Movement->bFleeingVehicle = Runtime->Profile.bFleeingVehicle;
    Movement->bIgnoreOneWayRestrictions = DrivingEntry != nullptr &&
        DrivingEntry->bIgnoreOneWayRestrictions;
    Movement->bRunRedLights = DrivingEntry != nullptr &&
        DrivingEntry->bRunRedLights;
    Movement->DesiredCruiseSpeedKmh = DrivingEntry != nullptr
        ? FMath::Max(0.0f, DrivingEntry->CruiseSpeedOverrideKmh) : 0.0f;
    if (DrivingEntry != nullptr && Movement->DesiredCruiseSpeedKmh <= 0.0f)
        Movement->DesiredCruiseSpeedKmh =
            SpeedForPreset(DrivingEntry->DrivingPreset);
    if (DrivingEntry != nullptr &&
        DrivingEntry->DrivingPreset == ETMOPVehicleDrivingPreset::Fleeing)
    {
        Movement->bFleeingVehicle = true;
        Movement->bRunRedLights = true;
        Movement->bIgnoreOneWayRestrictions = true;
    }
    if (DrivingEntry != nullptr && Movement->DesiredCruiseSpeedKmh <= 0.0f)
    {
        const int32 DrivingIndex = static_cast<int32>(
            DrivingEntry - Runtime->Profile.Timeline.GetData());
        int32 DepartureSecond = INDEX_NONE;
        int32 ArrivalSecond = INDEX_NONE;
        bool bHasDeparture = false;
        bool bHasArrival = false;
        if (DrivingEntry->bTimeIsArrival)
        {
            bHasDeparture = ResolveTimelineEntrySecond(Runtime->Profile,
                DrivingIndex - 1, DepartureSecond);
            bHasArrival = ResolveTimelineEntrySecond(Runtime->Profile,
                DrivingIndex, ArrivalSecond);
        }
        else
        {
            bHasDeparture = ResolveTimelineEntrySecond(Runtime->Profile,
                DrivingIndex, DepartureSecond);
            for (int32 Index = DrivingIndex + 1;
                Index < Runtime->Profile.Timeline.Num(); ++Index)
            {
                const ETMOPHistoricalVehicleAction Action =
                    Runtime->Profile.Timeline[Index].Action;
                if (Action != ETMOPHistoricalVehicleAction::Stop &&
                    Action != ETMOPHistoricalVehicleAction::Park &&
                    Action != ETMOPHistoricalVehicleAction::ExitTrafficRoute)
                    continue;
                if (ResolveTimelineEntrySecond(Runtime->Profile,
                    Index, ArrivalSecond))
                { bHasArrival = true; break; }
            }
        }
        const int32 DurationSeconds = ArrivalSecond - DepartureSecond;
        if (bHasDeparture && bHasArrival && DurationSeconds > 0 &&
            GetGameInstance() != nullptr)
        {
            UTMOPTrafficNetworkSubsystem* Network =
                GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>();
            if (Network != nullptr)
            {
                Network->DiscoverLanesInWorld();
                double DistanceCm = 0.0;
                bool bCompleteRoute = true;
                for (const FName LaneId : Route)
                {
                    UTMOPTrafficLaneComponent* Lane = Network->FindLane(LaneId);
                    if (!IsValid(Lane)) { bCompleteRoute = false; break; }
                    DistanceCm += Lane->GetSplineLength();
                }
                if (bCompleteRoute && DistanceCm > 0.0)
                    Movement->DesiredCruiseSpeedKmh = static_cast<float>(
                        (DistanceCm / 100000.0) /
                        (static_cast<double>(DurationSeconds) / 3600.0));
            }
        }
    }
    // Historical vehicles are governed by their DataTable timeline. Reaching
    // the final lane means stop and remain available; only an explicit
    // HistoricalVehicle Despawn entry may destroy the car and its occupants.
    Movement->bDespawnAtRouteEnd = false;
    Movement->PlannedLaneIds = Route;
    Movement->InitialLaneId = Route[0];
    if (!Movement->InitializeOnLane(
        Route[0], ResolvedStartDistance))
    {
        TArray<FString> RouteNames;
        RouteNames.Reserve(Route.Num());
        for (const FName LaneId : Route)
            RouteNames.Add(LaneId.ToString());
        return ReportDrivingFailure(VehicleId,
            TEXT("InitializeOnLaneFailed"),
            FString::Printf(TEXT("InitializeOnLane rejected lane '%s' at %.1f cm for route [%s]."),
                *Route[0].ToString(), ResolvedStartDistance,
                *FString::Join(RouteNames, TEXT(","))));
    }
    if (!FinalDestinationLaneId.IsNone())
    {
        Movement->ConfigureFinalApproach(
            FinalDestinationLaneId,
            FinalDestinationLaneDistance,
            FinalDestinationTransform);
    }
    Movement->StartDriving();
    if (Runtime->bBoundaryCollisionSuppressed)
    {
        Movement->bDetectPhysicalObstacles = false;
        Vehicle->SetActorEnableCollision(false);
        Runtime->bBoundaryVehicleHasStartedDriving = true;
        Runtime->BoundaryDrivingSeconds = 0.0f;
        Runtime->BoundaryDrivingStartLocation = Vehicle->GetActorLocation();
    }
    UE_LOG(LogTemp, Display,
        TEXT("TMOP driving: '%s' started '%s' on %d ordered lane(s)."),
        *DriverEntityId.ToString(), *VehicleId.ToString(), Route.Num());
    return true;
}
