#include "Transit/TMOPBusScheduleDirector.h"

#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
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
        int32 Seconds = Run.ExactStartTime.ToSecondsFromMidnight();
        if (!Run.bUseExactStartTime)
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
        if (Runtime.State != ETMOPBusRunState::Pending ||
            Runtime.ResolvedStartTime.ToSecondsFromMidnight() > Now) continue;
        if (GetActiveBusCount() >= MaximumSimultaneousBuses) break;
        if (!SpawnRun(Runtime)) Runtime.State = ETMOPBusRunState::Failed;
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
    Service->RouteData = Run.RouteData;
    Service->ServiceRunId = Run.RunId;
    Service->DwellRandomSeed = ScheduleSeed + CurrentLoopNumber * 1013 + Runtime.SourceIndex * 89;
    Service->InitializeService();
    if (UTMOPBusPassengerComponent* Passengers =
        Bus->FindComponentByClass<UTMOPBusPassengerComponent>())
    {
        if (!Passengers->InitializePassengerManifest(Run.PassengerManifest, Run.RunId))
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP bus '%s': passenger manifest validation failed."),
                *Runtime.RunId.ToString());
            Bus->Destroy();
            return false;
        }
    }
    else if (IsValid(Run.PassengerManifest))
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
    Movement->StartDriving();
    Runtime.SpawnedBus = Bus;
    Runtime.State = ETMOPBusRunState::Active;
    OnBusRunSpawned.Broadcast(Runtime.RunId, Bus);
    UE_LOG(LogTemp, Display, TEXT("TMOP bus '%s': spawn succeeded."),
        *Runtime.RunId.ToString());
    return true;
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
        const UTMOPTrafficVehicleMovementComponent* Movement =
            Runtime.SpawnedBus->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        const bool bTrafficComplete = IsValid(Movement) &&
            Movement->TrafficState == ETMOPTrafficVehicleState::RouteComplete;
        const bool bForcedTime = Run.bUseForcedDespawnTime &&
            Now >= Run.ForcedDespawnTime.ToSecondsFromMidnight();
        if ((Run.bDespawnWhenTrafficRouteCompletes && bTrafficComplete) || bForcedTime)
            CompleteRun(Runtime);
    }
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
        if (Runtime.State == ETMOPBusRunState::Active && IsValid(Runtime.SpawnedBus)) ++Count;
    return Count;
}

bool ATMOPBusScheduleDirector::ValidateSchedule(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    TSet<FName> RunIds;
    for (int32 Index = 0; Index < ScheduledRuns.Num(); ++Index)
    {
        const FTMOPBusScheduledRun& Run = ScheduledRuns[Index];
        if (Run.RunId.IsNone() || RunIds.Contains(Run.RunId))
            OutErrors.Add(FString::Printf(TEXT("Run %d has missing or duplicate RunId."), Index));
        RunIds.Add(Run.RunId);
        if (!IsValid(Run.RouteData)) OutErrors.Add(FString::Printf(TEXT("Run %d has no RouteData."), Index));
        if (Run.BusClass == nullptr) OutErrors.Add(FString::Printf(TEXT("Run %d has no BusClass."), Index));
        if (Network == nullptr || !IsValid(Network->FindLane(Run.InitialLaneId)))
            OutErrors.Add(FString::Printf(TEXT("Run %d has invalid InitialLaneId."), Index));
        if (!Run.bUseExactStartTime &&
            Run.LatestStartTime.ToSecondsFromMidnight() < Run.EarliestStartTime.ToSecondsFromMidnight())
            OutErrors.Add(FString::Printf(TEXT("Run %d has invalid start window."), Index));
    }
    return OutErrors.IsEmpty();
}
