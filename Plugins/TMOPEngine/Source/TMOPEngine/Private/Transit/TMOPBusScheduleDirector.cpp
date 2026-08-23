#include "Transit/TMOPBusScheduleDirector.h"

#include "Engine/World.h"
#include "Engine/OverlapResult.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "People/TMOPPersonProfileTypes.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Time/TMOPClockSubsystem.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Transit/TMOPBusPassengerComponent.h"
#include "Transit/TMOPBusPassengerManifest.h"
#include "Transit/TMOPBusRouteData.h"
#include "Transit/TMOPBusServiceComponent.h"
#include "Vehicles/TMOPVehicleBase.h"

ATMOPBusScheduleDirector::ATMOPBusScheduleDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPBusScheduleDirector::BeginPlay()
{
    Super::BeginPlay();

    // Buses must exist before the people director evaluates BusSeat or
    // EnterVehicle entries for the same simulation second.
    if (ATMOPPersonRegistryDirector* People = FindPeopleDirector())
    {
        People->AddTickPrerequisiteActor(this);
        if (!IsValid(PersonProfileTable))
            PersonProfileTable = People->PersonProfileTable;
    }
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.AddDynamic(this, &ATMOPBusScheduleDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.AddDynamic(this, &ATMOPBusScheduleDirector::HandleLoopRestarted);
        CurrentLoopNumber = Clock->GetLoopNumber();
        ResolveSchedule(CurrentLoopNumber);
        EvaluateSchedule(Clock->GetCurrentTime());
    }
    else ResolveSchedule(CurrentLoopNumber);
}

void ATMOPBusScheduleDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.RemoveDynamic(this, &ATMOPBusScheduleDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.RemoveDynamic(this, &ATMOPBusScheduleDirector::HandleLoopRestarted);
    }
    ResetBusSchedule();
    Super::EndPlay(EndPlayReason);
}

void ATMOPBusScheduleDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateEntryCollision(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock != nullptr) MonitorActiveRuns(Clock->GetCurrentTime());
}

bool ATMOPBusScheduleDirector::ResolveSchedule(const int32 LoopNumber)
{
    RuntimeRuns.Reset();
    for (int32 Index = 0; Index < ScheduledRuns.Num(); ++Index)
    {
        const FTMOPBusScheduledRun& Run = ScheduledRuns[Index];
        const FTMOPBusRunPeopleRow* RunPeople = FindRunPeopleRow(Run.RunId);
        if (RunPeople != nullptr && !RunPeople->bEnabledInSimulation) continue;
        int32 Seconds = RunPeople != nullptr && RunPeople->bOverrideExactStartTime
            ? RunPeople->ExactStartTime.ToSecondsFromMidnight()
            : Run.ExactStartTime.ToSecondsFromMidnight();
        if (!(RunPeople != nullptr && RunPeople->bOverrideExactStartTime) &&
            !Run.bUseExactStartTime)
        {
            const int32 Earliest = Run.EarliestStartTime.ToSecondsFromMidnight();
            const int32 Latest = Run.LatestStartTime.ToSecondsFromMidnight();
            if (Latest < Earliest) return false;
            FRandomStream Random(ScheduleSeed + LoopNumber * 1013 + Index * 89);
            Seconds = Random.RandRange(Earliest, Latest);
        }
        FTMOPBusRunRuntime Runtime;
        Runtime.RunId = Run.RunId;
        Runtime.ResolvedStartTime = FTMOPTime::FromSecondsFromMidnight(Seconds);
        Runtime.SourceIndex = Index;
        RuntimeRuns.Add(Runtime);
    }
    RuntimeRuns.Sort([](const FTMOPBusRunRuntime& A, const FTMOPBusRunRuntime& B)
        { return A.ResolvedStartTime.ToSecondsFromMidnight() < B.ResolvedStartTime.ToSecondsFromMidnight(); });
    LastEvaluatedSeconds = INDEX_NONE;
    return true;
}

void ATMOPBusScheduleDirector::EvaluateSchedule(const FTMOPTime CurrentTime)
{
    if (bIsResetting) return;
    const int32 Now = CurrentTime.ToSecondsFromMidnight();
    if (bResetWhenTimeMovesBackwards && LastEvaluatedSeconds != INDEX_NONE && Now < LastEvaluatedSeconds)
    {
        ResetBusSchedule();
        ResolveSchedule(CurrentLoopNumber);
    }
    for (FTMOPBusRunRuntime& Runtime : RuntimeRuns)
    {
        const int32 StartSecond =
            Runtime.ResolvedStartTime.ToSecondsFromMidnight();
        if (Runtime.State == ETMOPBusRunState::Pending &&
            StartSecond - SpawnLeadSeconds <= Now)
        {
            if (GetActiveBusCount() >= MaximumSimultaneousBuses) break;
            SpawnRun(Runtime);
        }
        if (Runtime.State == ETMOPBusRunState::Staged &&
            StartSecond <= Now)
            ActivateStagedRun(Runtime);
    }
    MonitorActiveRuns(CurrentTime);
    LastEvaluatedSeconds = Now;
}

bool ATMOPBusScheduleDirector::SpawnRun(FTMOPBusRunRuntime& Runtime)
{
    if (!ScheduledRuns.IsValidIndex(Runtime.SourceIndex) || GetWorld() == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': invalid scheduled-run index or World."),
            *Runtime.RunId.ToString());
        return false;
    }
    const FTMOPBusScheduledRun& Run = ScheduledRuns[Runtime.SourceIndex];
    if (Run.BusClass == nullptr || !IsValid(Run.RouteData))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': BusClass or RouteData is missing."),
            *Runtime.RunId.ToString());
        return false;
    }
    const FName StartLaneId = !Run.InitialLaneId.IsNone()
        ? Run.InitialLaneId
        : (!Run.RouteData->OrderedLaneIds.IsEmpty() ? Run.RouteData->OrderedLaneIds[0] : NAME_None);
    if (StartLaneId.IsNone())
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': no InitialLaneId and route has no lanes."),
            *Runtime.RunId.ToString());
        return false;
    }
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    if (Network == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': no TrafficNetworkSubsystem."),
            *Runtime.RunId.ToString());
        return false;
    }
    const int32 LaneCount = Network->DiscoverLanesInWorld();
    if (!IsValid(Network->FindLane(StartLaneId)))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus '%s': InitialLaneId '%s' not found after discovering %d lanes."),
            *Runtime.RunId.ToString(), *StartLaneId.ToString(), LaneCount);
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("TMOP bus '%s': spawning class '%s' on '%s'."),
        *Runtime.RunId.ToString(), *GetNameSafe(Run.BusClass.Get()), *StartLaneId.ToString());
    ATMOPVehicleBase* Bus = GetWorld()->SpawnActorDeferred<ATMOPVehicleBase>(
        Run.BusClass, FTransform::Identity, this, nullptr,
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!IsValid(Bus))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': SpawnActorDeferred failed."),
            *Runtime.RunId.ToString());
        return false;
    }
    // Keep the staged bus non-colliding until its lane position has been
    // checked. This prevents even a single physics frame of overlap from
    // pushing an existing bus, police vehicle or passenger crowd aside.
    Bus->SetActorEnableCollision(false);
    Bus->VehicleId = Run.RunId;

    // Blueprint SCS components do not reliably exist until deferred spawning
    // has been finished. The old code searched for them before this call.
    UGameplayStatics::FinishSpawningActor(Bus, FTransform::Identity);
    if (!IsValid(Bus))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': FinishSpawningActor failed."),
            *Runtime.RunId.ToString());
        return false;
    }
    UTMOPTrafficVehicleMovementComponent* Movement =
        Bus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    UTMOPBusServiceComponent* Service = Bus->FindComponentByClass<UTMOPBusServiceComponent>();
    if (!IsValid(Movement) || !IsValid(Service))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus '%s': spawned class '%s' lacks Movement=%s or BusService=%s."),
            *Runtime.RunId.ToString(), *GetNameSafe(Run.BusClass.Get()),
            IsValid(Movement) ? TEXT("yes") : TEXT("NO"),
            IsValid(Service) ? TEXT("yes") : TEXT("NO"));
        Bus->Destroy();
        return false;
    }
    Movement->InitialLaneId = StartLaneId;
    Movement->PlannedLaneIds = Run.RouteData->OrderedLaneIds;
    Movement->SpeedLimitMultiplier = Run.SpeedLimitMultiplier;
    Service->RouteData = Run.RouteData;
    Service->ServiceRunId = Run.RunId;
    Service->DwellRandomSeed = ScheduleSeed + CurrentLoopNumber * 1013 + Runtime.SourceIndex * 89;
    if (UTMOPBusPassengerComponent* Passengers =
        Bus->FindComponentByClass<UTMOPBusPassengerComponent>())
    {
        const FName ResolvedDriverEntityId = ResolveDriverEntityId(Run);
        if (!Passengers->AssignScheduledDriver(ResolvedDriverEntityId, Run.RunId))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus '%s': scheduled driver '%s' could not be assigned."),
                *Runtime.RunId.ToString(), *ResolvedDriverEntityId.ToString());
            Bus->Destroy();
            return false;
        }
        // Clear a manifest inherited from the bus Blueprint as well as the
        // per-run asset; otherwise BeginPlay may have initialized duplicate
        // passengers before this director applies the people-timeline mode.
        UTMOPBusPassengerManifest* ManifestToUse =
            bUsePeopleTimelinesForPassengers ? nullptr : Run.PassengerManifest.Get();
        if (!Passengers->InitializePassengerManifest(ManifestToUse, Run.RunId))
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': passenger manifest validation failed."),
                *Runtime.RunId.ToString());
            Bus->Destroy();
            return false;
        }
    }
    else if (!bUsePeopleTimelinesForPassengers && IsValid(Run.PassengerManifest))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus '%s': a PassengerManifest is assigned but BusClass lacks TMOPBusPassengerComponent."),
            *Runtime.RunId.ToString());
        Bus->Destroy();
        return false;
    }
    if (!Movement->InitializeOnLane(StartLaneId, Run.InitialDistanceAlongLane))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': InitializeOnLane('%s') failed."),
            *Runtime.RunId.ToString(), *StartLaneId.ToString());
        Bus->Destroy();
        return false;
    }

    // The service must inspect an already initialized movement component.
    // Initializing it earlier can leave CurrentTargetStop/route progress based
    // on the deferred actor's identity transform, causing the bus to stop at
    // its first stop and never resume.
    Service->InitializeService();
    Runtime.SpawnedBus = Bus;
    TArray<FOverlapResult> Overlaps;
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TMOPBusEntrySpawn), false, Bus);
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, Bus->GetActorLocation(), FQuat::Identity, Objects,
        FCollisionShape::MakeSphere(SpawnClearanceRadiusCm), Query);
    const ATMOPVehicleBase* BlockingVehicle = nullptr;
    for (const FOverlapResult& Overlap : Overlaps)
        if (const ATMOPVehicleBase* Other =
            Cast<ATMOPVehicleBase>(Overlap.GetActor()))
            if (Other != Bus)
            {
                BlockingVehicle = Other;
                break;
            }

    Runtime.bEntryCollisionSuppressed = BlockingVehicle != nullptr;
    Runtime.bEntryVehicleHasStartedDriving = false;
    Runtime.EntryDrivingSeconds = 0.0f;
    Runtime.EntryDrivingStartLocation = Bus->GetActorLocation();
    if (BlockingVehicle != nullptr)
    {
        Movement->bDetectPhysicalObstacles = false;
        Bus->SetActorEnableCollision(false);
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP BusEntryGhostSpawn: bus '%s' stages collision-free because '%s' occupies the shared entry point."),
            *Runtime.RunId.ToString(), *BlockingVehicle->VehicleId.ToString());
    }
    else
    {
        Movement->bDetectPhysicalObstacles = true;
        Bus->SetActorEnableCollision(true);
    }
    Movement->StopDriving();
    Runtime.State = ETMOPBusRunState::Staged;
    OnBusRunSpawned.Broadcast(Runtime.RunId, Bus);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP bus '%s': staged %d seconds before departure."),
        *Runtime.RunId.ToString(), SpawnLeadSeconds);
    if (bUsePeopleTimelinesForPassengers)
        UE_LOG(LogTemp, Display,
            TEXT("TMOP bus '%s': %d enabled passenger profile(s) target this run in DT_TMOP_People."),
            *Runtime.RunId.ToString(), GetTimelinePassengerCount(Runtime.RunId));
    return true;
}

bool ATMOPBusScheduleDirector::ActivateStagedRun(
    FTMOPBusRunRuntime& Runtime)
{
    ATMOPVehicleBase* Bus = Runtime.SpawnedBus;
    UTMOPTrafficVehicleMovementComponent* Movement = IsValid(Bus)
        ? Bus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>()
        : nullptr;
    if (!IsValid(Bus) || !IsValid(Movement))
    {
        Runtime.State = ETMOPBusRunState::Failed;
        return false;
    }
    Movement->StartDriving();
    if (Runtime.bEntryCollisionSuppressed)
    {
        Movement->bDetectPhysicalObstacles = false;
        Bus->SetActorEnableCollision(false);
        Runtime.bEntryVehicleHasStartedDriving = true;
        Runtime.EntryDrivingSeconds = 0.0f;
        Runtime.EntryDrivingStartLocation = Bus->GetActorLocation();
    }
    Runtime.State = ETMOPBusRunState::Active;
    Runtime.LastProgressLocation = Bus->GetActorLocation();
    Runtime.LastProgressWorldSeconds = GetWorld() != nullptr
        ? GetWorld()->GetTimeSeconds() : 0.0f;
    Runtime.LastRecoveryWorldSeconds = -1000.0f;
    Runtime.RecoveryAttempts = 0;
    UE_LOG(LogTemp, Display,
        TEXT("TMOP bus '%s': departed at scheduled time."),
        *Runtime.RunId.ToString());
    return true;
}

bool ATMOPBusScheduleDirector::IsBusClearForCollisionRestore(
    const ATMOPVehicleBase* Bus) const
{
    if (!IsValid(Bus) || GetWorld() == nullptr) return false;
    FCollisionObjectQueryParams Objects;
    Objects.AddObjectTypesToQuery(ECC_WorldDynamic);
    Objects.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams Query(
        SCENE_QUERY_STAT(TMOPBusEntryCollisionRestore), false, Bus);
    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, Bus->GetActorLocation(), FQuat::Identity, Objects,
        FCollisionShape::MakeSphere(SpawnClearanceRadiusCm), Query);
    return !Overlaps.ContainsByPredicate(
        [Bus](const FOverlapResult& Overlap)
        {
            const ATMOPVehicleBase* Other =
                Cast<ATMOPVehicleBase>(Overlap.GetActor());
            return IsValid(Other) && Other != Bus;
        });
}

void ATMOPBusScheduleDirector::UpdateEntryCollision(const float DeltaSeconds)
{
    for (FTMOPBusRunRuntime& Runtime : RuntimeRuns)
    {
        ATMOPVehicleBase* Bus = Runtime.SpawnedBus;
        if (!Runtime.bEntryCollisionSuppressed ||
            !Runtime.bEntryVehicleHasStartedDriving || !IsValid(Bus))
            continue;
        Runtime.EntryDrivingSeconds += FMath::Max(0.0f, DeltaSeconds);
        const float Distance = FVector::Dist2D(
            Runtime.EntryDrivingStartLocation, Bus->GetActorLocation());
        if (Runtime.EntryDrivingSeconds < EntryCollisionReleaseDelaySeconds ||
            Distance < EntryCollisionReleaseDistanceCm ||
            !IsBusClearForCollisionRestore(Bus))
            continue;
        Bus->SetActorEnableCollision(true);
        if (UTMOPTrafficVehicleMovementComponent* Movement =
            Bus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
            Movement->bDetectPhysicalObstacles = true;
        Runtime.bEntryCollisionSuppressed = false;
        UE_LOG(LogTemp, Display,
            TEXT("TMOP BusEntryReleased: bus '%s' restored collision %.0f cm beyond its staging point."),
            *Runtime.RunId.ToString(), Distance);
    }
}

void ATMOPBusScheduleDirector::MonitorActiveRuns(const FTMOPTime CurrentTime)
{
    const int32 Now = CurrentTime.ToSecondsFromMidnight();
    for (FTMOPBusRunRuntime& Runtime : RuntimeRuns)
    {
        if (Runtime.State != ETMOPBusRunState::Active) continue;
        if (!IsValid(Runtime.SpawnedBus))
        {
            Runtime.State = ETMOPBusRunState::Failed;
            continue;
        }
        const FTMOPBusScheduledRun& Run = ScheduledRuns[Runtime.SourceIndex];
        UTMOPTrafficVehicleMovementComponent* Movement =
            Runtime.SpawnedBus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        const bool bTrafficComplete = IsValid(Movement) &&
            Movement->TrafficState == ETMOPTrafficVehicleState::RouteComplete;
        const bool bForcedTime = Run.bUseForcedDespawnTime &&
            Now >= Run.ForcedDespawnTime.ToSecondsFromMidnight();
        if ((Run.bDespawnWhenTrafficRouteCompletes && bTrafficComplete) || bForcedTime)
            CompleteRun(Runtime);
        else
            RecoverStalledBus(Runtime);
    }
}

void ATMOPBusScheduleDirector::RecoverStalledBus(FTMOPBusRunRuntime& Runtime)
{
    if (!IsValid(Runtime.SpawnedBus) || GetWorld() == nullptr) return;
    UTMOPTrafficVehicleMovementComponent* Movement =
        Runtime.SpawnedBus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    if (!IsValid(Movement) ||
        Movement->TrafficState == ETMOPTrafficVehicleState::RouteComplete)
        return;

    const float WorldSeconds = GetWorld()->GetTimeSeconds();
    const FVector Location = Runtime.SpawnedBus->GetActorLocation();
    if (FVector::DistSquared2D(Location, Runtime.LastProgressLocation) >=
        FMath::Square(ProgressDistanceThresholdCm))
    {
        Runtime.LastProgressLocation = Location;
        Runtime.LastProgressWorldSeconds = WorldSeconds;
        Runtime.RecoveryAttempts = 0;
        return;
    }

    const float StationaryFor = WorldSeconds - Runtime.LastProgressWorldSeconds;
    if (StationaryFor < MaximumStationarySeconds ||
        WorldSeconds - Runtime.LastRecoveryWorldSeconds < RecoveryRetryIntervalSeconds)
        return;

    Runtime.LastRecoveryWorldSeconds = WorldSeconds;
    ++Runtime.RecoveryAttempts;
    Movement->StartDriving();
    UE_LOG(LogTemp, Warning,
        TEXT("TMOP BusWatchdog: bus '%s' made no route progress for %.1f s; StartDriving issued (attempt %d)."),
        *Runtime.RunId.ToString(), StationaryFor, Runtime.RecoveryAttempts);
}

void ATMOPBusScheduleDirector::CompleteRun(FTMOPBusRunRuntime& Runtime)
{
    ATMOPVehicleBase* Bus = Runtime.SpawnedBus;
    OnBusRunCompleted.Broadcast(Runtime.RunId, Bus);
    if (IsValid(Bus)) Bus->Destroy();
    Runtime.SpawnedBus = nullptr;
    Runtime.State = ETMOPBusRunState::Completed;
}

void ATMOPBusScheduleDirector::ResetBusSchedule()
{
    bIsResetting = true;
    for (FTMOPBusRunRuntime& Runtime : RuntimeRuns)
    {
        if (IsValid(Runtime.SpawnedBus)) Runtime.SpawnedBus->Destroy();
        Runtime.SpawnedBus = nullptr;
    }
    RuntimeRuns.Reset();
    LastEvaluatedSeconds = INDEX_NONE;
    bIsResetting = false;
}

void ATMOPBusScheduleDirector::HandleSecondChanged(const FTMOPTime NewTime)
{
    EvaluateSchedule(NewTime);
}

void ATMOPBusScheduleDirector::HandleLoopRestarted(const int32 NewLoopNumber,
    const FTMOPTime RestartTime)
{
    CurrentLoopNumber = NewLoopNumber;
    ResetBusSchedule();
    ResolveSchedule(NewLoopNumber);
    EvaluateSchedule(RestartTime);
}

int32 ATMOPBusScheduleDirector::GetActiveBusCount() const
{
    int32 Count = 0;
    for (const FTMOPBusRunRuntime& Runtime : RuntimeRuns)
        if ((Runtime.State == ETMOPBusRunState::Staged ||
             Runtime.State == ETMOPBusRunState::Active) &&
            IsValid(Runtime.SpawnedBus))
            ++Count;
    return Count;
}

ATMOPPersonRegistryDirector* ATMOPBusScheduleDirector::FindPeopleDirector() const
{
    UWorld* World = GetWorld();
    if (World == nullptr) return nullptr;
    for (TActorIterator<ATMOPPersonRegistryDirector> It(World); It; ++It)
        return *It;
    return nullptr;
}

const FTMOPBusRunPeopleRow* ATMOPBusScheduleDirector::FindRunPeopleRow(
    const FName RunId) const
{
    if (RunId.IsNone() || !IsValid(BusRunConfigurationTable) ||
        BusRunConfigurationTable->GetRowStruct() != FTMOPBusRunPeopleRow::StaticStruct())
        return nullptr;
    return BusRunConfigurationTable->FindRow<FTMOPBusRunPeopleRow>(
        RunId, TEXT("TMOPBusRunPeopleLookup"), false);
}

FName ATMOPBusScheduleDirector::ResolveDriverEntityId(
    const FTMOPBusScheduledRun& Run) const
{
    const FTMOPBusRunPeopleRow* Row = FindRunPeopleRow(Run.RunId);
    return Row != nullptr && !Row->DriverEntityId.IsNone()
        ? Row->DriverEntityId : Run.DriverEntityId;
}

int32 ATMOPBusScheduleDirector::GetTimelinePassengerCount(const FName RunId) const
{
    if (RunId.IsNone() || !IsValid(PersonProfileTable) ||
        PersonProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct())
        return 0;

    int32 Count = 0;
    for (const FName RowName : PersonProfileTable->GetRowNames())
    {
        const FTMOPPersonProfileRow* Person =
            PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
                RowName, TEXT("TMOPBusTimelinePassengerCount"), false);
        if (Person == nullptr || !Person->bSpawnInSimulation) continue;
        for (const FTMOPPersonTimelineEntry& Entry : Person->Timeline)
            if (Entry.Usage != ETMOPPersonTimelineUsage::DocumentationOnly &&
                Entry.TargetEntityId == RunId &&
                (Entry.LocationType == ETMOPPersonLocationType::BusSeat ||
                 Entry.LocationType == ETMOPPersonLocationType::StandingInVehicle ||
                 Entry.Action == ETMOPPersonTimelineAction::EnterVehicle))
            {
                ++Count;
                break;
            }
    }
    return Count;
}

bool ATMOPBusScheduleDirector::ValidateSchedule(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    TSet<FName> RunIds;
    TMap<FName, FString> RouteIdOwners;

    TSet<FName> KnownPeople;
    if (bUsePeopleTimelinesForPassengers)
    {
        if (!IsValid(PersonProfileTable) ||
            PersonProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct())
            OutErrors.Add(TEXT("People timelines are enabled but PersonProfileTable is missing or has the wrong row structure."));
        else
            for (const FName PersonRowName : PersonProfileTable->GetRowNames())
                if (const FTMOPPersonProfileRow* Person =
                    PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
                        PersonRowName, TEXT("TMOPBusValidatePeople"), false))
                    KnownPeople.Add(Person->EntityId);
    }
    if (IsValid(BusRunConfigurationTable) &&
        BusRunConfigurationTable->GetRowStruct() != FTMOPBusRunPeopleRow::StaticStruct())
        OutErrors.Add(TEXT("BusRunConfigurationTable has the wrong row structure."));
    for (int32 Index = 0; Index < ScheduledRuns.Num(); ++Index)
    {
        const FTMOPBusScheduledRun& Run = ScheduledRuns[Index];
        if (Run.RunId.IsNone() || RunIds.Contains(Run.RunId))
            OutErrors.Add(FString::Printf(TEXT("Run %d has missing or duplicate RunId."), Index));
        RunIds.Add(Run.RunId);
        const FTMOPBusRunPeopleRow* RunPeople = FindRunPeopleRow(Run.RunId);
        if (RunPeople != nullptr && !RunPeople->bEnabledInSimulation) continue;
        if (!IsValid(Run.RouteData)) OutErrors.Add(FString::Printf(TEXT("Run %d has no RouteData."), Index));
        else if (const FString* ExistingOwner =
            RouteIdOwners.Find(Run.RouteData->RouteId))
        {
            if (*ExistingOwner != Run.RouteData->GetPathName())
                OutErrors.Add(FString::Printf(
                    TEXT("RouteId '%s' is shared by '%s' and '%s'."),
                    *Run.RouteData->RouteId.ToString(), **ExistingOwner,
                    *Run.RouteData->GetPathName()));
        }
        else RouteIdOwners.Add(
            Run.RouteData->RouteId, Run.RouteData->GetPathName());
        if (Run.BusClass == nullptr) OutErrors.Add(FString::Printf(TEXT("Run %d has no BusClass."), Index));
        const FName DriverEntityId = ResolveDriverEntityId(Run);
        if (bUsePeopleTimelinesForPassengers && !DriverEntityId.IsNone() &&
            !KnownPeople.Contains(DriverEntityId))
            OutErrors.Add(FString::Printf(
                TEXT("Run '%s' references missing driver '%s' in DT_TMOP_People."),
                *Run.RunId.ToString(), *DriverEntityId.ToString()));
        const FName ResolvedInitialLaneId = !Run.InitialLaneId.IsNone()
            ? Run.InitialLaneId
            : (IsValid(Run.RouteData) &&
               !Run.RouteData->OrderedLaneIds.IsEmpty()
                ? Run.RouteData->OrderedLaneIds[0] : NAME_None);
        if (Network == nullptr || ResolvedInitialLaneId.IsNone() ||
            !IsValid(Network->FindLane(ResolvedInitialLaneId)))
            OutErrors.Add(FString::Printf(TEXT("Run %d has invalid InitialLaneId."), Index));
        if (!Run.bUseExactStartTime &&
            Run.LatestStartTime.ToSecondsFromMidnight() < Run.EarliestStartTime.ToSecondsFromMidnight())
            OutErrors.Add(FString::Printf(TEXT("Run %d has invalid start window."), Index));
    }
    return OutErrors.IsEmpty();
}
