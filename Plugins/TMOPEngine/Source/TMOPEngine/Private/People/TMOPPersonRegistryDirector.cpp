#include "People/TMOPPersonRegistryDirector.h"

#include "Actions/TMOPActionExecutorComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "People/TMOPPersonProfileComponent.h"
#include "People/TMOPPersonRegistrySubsystem.h"
#include "Schedules/TMOPScheduleTypes.h"
#include "Time/TMOPClockSubsystem.h"
#include "Transit/TMOPBusServiceComponent.h"
#include "Transit/TMOPBusStopComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"
#include "World/TMOPWorldSubsystem.h"

ATMOPPersonRegistryDirector::ATMOPPersonRegistryDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPPersonRegistryDirector::BeginPlay()
{
    Super::BeginPlay();
    UTMOPPersonRegistrySubsystem* Registry = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    if (Registry == nullptr || !Registry->ConfigureProfileTable(PersonProfileTable))
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP Person Registry Director has no valid profile table."));
        SetActorTickEnabled(false);
        return;
    }

    RefreshAllActiveProfiles();
    if (bSpawnPeopleAutomatically) InitializePersonSimulation();
}

void ATMOPPersonRegistryDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
        if (Pair.Value.bSpawnedByDirector && Pair.Value.Agent.IsValid())
            Pair.Value.Agent->Destroy();
    RuntimePeople.Reset();
    Super::EndPlay(EndPlayReason);
}

void ATMOPPersonRegistryDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock == nullptr) return;
    const int32 CurrentSecond = Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE && CurrentSecond < LastEvaluatedSecond)
    {
        InitializePersonSimulation();
        return;
    }
    if (CurrentSecond != LastEvaluatedSecond)
    {
        EvaluatePeople(CurrentSecond, false);
        LastEvaluatedSecond = CurrentSecond;
    }
}

int32 ATMOPPersonRegistryDirector::RefreshAllActiveProfiles()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    int32 Loaded = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPPersonProfileComponent*> Components;
        It->GetComponents<UTMOPPersonProfileComponent>(Components);
        for (UTMOPPersonProfileComponent* Component : Components)
            Loaded += IsValid(Component) && Component->LoadProfile() ? 1 : 0;
    }
    return Loaded;
}

int32 ATMOPPersonRegistryDirector::InitializePersonSimulation()
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
        if (Pair.Value.bSpawnedByDirector && Pair.Value.Agent.IsValid())
            Pair.Value.Agent->Destroy();
    RuntimePeople.Reset();
    if (!IsValid(PersonProfileTable) ||
        PersonProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct()) return 0;

    TArray<FString> Errors;
    ValidatePeopleTable(Errors);
    for (const FString& Error : Errors)
        UE_LOG(LogTemp, Error, TEXT("TMOP People: %s"), *Error);

    const TArray<FName> RowNames = PersonProfileTable->GetRowNames();
    for (const FName RowName : RowNames)
    {
        const FTMOPPersonProfileRow* Row = PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
            RowName, TEXT("TMOPPersonSimulation"), false);
        if (Row == nullptr || !Row->bSpawnInSimulation || Row->EntityId.IsNone() ||
            Row->Timeline.IsEmpty()) continue;
        FPersonRuntime Runtime;
        Runtime.RowName = RowName;
        Runtime.Profile = *Row;
        Runtime.Profile.Timeline.Sort([](const FTMOPPersonTimelineEntry& A,
            const FTMOPPersonTimelineEntry& B)
            { return A.Time.ToSecondsFromMidnight() < B.Time.ToSecondsFromMidnight(); });
        UTMOPPersonRegistrySubsystem* Registry =
            GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>();
        if (Registry != nullptr)
            Runtime.Agent = Registry->FindActiveAgent(Row->EntityId);
        RuntimePeople.Add(Row->EntityId, MoveTemp(Runtime));
    }

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const int32 CurrentSecond = Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight()
        : SimulationEpoch.ToSecondsFromMidnight();
    EvaluatePeople(CurrentSecond, bCatchUpToCurrentClockOnBeginPlay);
    LastEvaluatedSecond = CurrentSecond;
    UE_LOG(LogTemp, Display, TEXT("TMOP People: initialized %d timeline profile(s), %d active agent(s)."),
        RuntimePeople.Num(), RefreshAllActiveProfiles());
    return RuntimePeople.Num();
}

void ATMOPPersonRegistryDirector::EvaluatePeople(const int32 CurrentSecond,
    const bool bCatchUp)
{
    for (TPair<FName, FPersonRuntime>& Pair : RuntimePeople)
    {
        FPersonRuntime& Runtime = Pair.Value;
        while (!Runtime.bCompleted && Runtime.Profile.Timeline.IsValidIndex(Runtime.NextTimelineIndex))
        {
            const FTMOPPersonTimelineEntry& Entry =
                Runtime.Profile.Timeline[Runtime.NextTimelineIndex];
            if (Entry.Time.ToSecondsFromMidnight() > CurrentSecond) break;

            if (!Runtime.Agent.IsValid())
            {
                if (Runtime.NextTimelineIndex == 0 &&
                    (Entry.LocationType == ETMOPPersonLocationType::Unknown ||
                     Entry.LocationType == ETMOPPersonLocationType::NotPresent))
                {
                    // Preserve the historical unknown: no physical person exists yet.
                    ++Runtime.NextTimelineIndex;
                    continue;
                }
                if (Runtime.NextTimelineIndex != 0 && Entry.Action != ETMOPPersonTimelineAction::Spawn)
                    break;
                if (!SpawnPerson(Runtime, Entry)) break;
                ++Runtime.NextTimelineIndex;
                continue;
            }

            ATMOPHistoricalAgent* Agent = Runtime.Agent.Get();
            const bool bEntryCatchUp = bCatchUp ||
                Entry.Time.ToSecondsFromMidnight() < CurrentSecond;
            if (!bEntryCatchUp && IsAgentBusy(Agent)) break;
            if (!ApplyTimelineEntry(Runtime, Entry, bEntryCatchUp)) break;
            ++Runtime.NextTimelineIndex;
        }
    }
}

bool ATMOPPersonRegistryDirector::SpawnPerson(FPersonRuntime& Runtime,
    const FTMOPPersonTimelineEntry& InitialEntry)
{
    if (InitialEntry.LocationType == ETMOPPersonLocationType::Unknown ||
        InitialEntry.LocationType == ETMOPPersonLocationType::NotPresent || GetWorld() == nullptr)
        return false;

    TSubclassOf<ATMOPHistoricalAgent> Class = Runtime.Profile.AgentClass != nullptr
        ? Runtime.Profile.AgentClass : DefaultAgentClass;
    if (Class == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP person '%s': no Agent Class and no Default Agent Class."),
            *Runtime.Profile.EntityId.ToString());
        Runtime.bCompleted = true;
        return false;
    }

    FTransform SpawnTransform = InitialEntry.WorldTransform;
    if (InitialEntry.LocationType == ETMOPPersonLocationType::Anchor)
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(InitialEntry.TargetAnchorId) : nullptr;
        if (!IsValid(Anchor)) return false;
        SpawnTransform = Anchor->GetActorTransform();
    }
    else if (InitialEntry.LocationType == ETMOPPersonLocationType::VenueSeat)
    {
        UTMOPCinemaSeatSubsystem* Seats = GetGameInstance()->GetSubsystem<UTMOPCinemaSeatSubsystem>();
        if (Seats != nullptr) Seats->DiscoverSeatsInWorld();
        UTMOPCinemaSeatComponent* Seat = Seats != nullptr
            ? Seats->FindSeat(InitialEntry.TargetSeatId) : nullptr;
        if (!IsValid(Seat)) return false;
        SpawnTransform = Seat->GetSeatWorldTransform();
    }
    else if (InitialEntry.LocationType == ETMOPPersonLocationType::VehicleSeat ||
        InitialEntry.LocationType == ETMOPPersonLocationType::BusSeat)
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(InitialEntry.TargetEntityId);
        if (!IsValid(Vehicle)) return false;
        SpawnTransform = Vehicle->GetActorTransform();
        for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
            if (IsValid(Seat) && (InitialEntry.TargetSeatId.IsNone() ||
                Seat->SeatId == InitialEntry.TargetSeatId))
            {
                SpawnTransform = Seat->GetComponentTransform();
                break;
            }
    }
    else if (InitialEntry.LocationType == ETMOPPersonLocationType::StandingInVehicle)
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(InitialEntry.TargetEntityId);
        if (!IsValid(Vehicle)) return false;
        SpawnTransform = Vehicle->GetActorTransform();
    }

    ATMOPHistoricalAgent* Agent = GetWorld()->SpawnActorDeferred<ATMOPHistoricalAgent>(
        Class, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
    if (!IsValid(Agent)) return false;
    Agent->EntityIdentity->EntityId = Runtime.Profile.EntityId;
    Agent->EntityIdentity->EntityType = TEXT("Agent");
    Agent->DisplayName = Runtime.Profile.FullName;
    Agent->SourceReference = Runtime.Profile.GeneralSourceReference;
    Agent->MovementProfile = Runtime.Profile.MovementProfile;
    Agent->ActivityState = InitialEntry.ActivityState;
    Agent->LifeState = InitialEntry.LifeState;
    UGameplayStatics::FinishSpawningActor(Agent, SpawnTransform);

    UTMOPPersonProfileComponent* ProfileComponent =
        Agent->FindComponentByClass<UTMOPPersonProfileComponent>();
    if (!IsValid(ProfileComponent))
    {
        ProfileComponent = NewObject<UTMOPPersonProfileComponent>(Agent, TEXT("PersonProfile"));
        Agent->AddInstanceComponent(ProfileComponent);
        ProfileComponent->RegisterComponent();
    }
    if (IsValid(ProfileComponent)) ProfileComponent->LoadProfile();
    Runtime.Agent = Agent;
    Runtime.bSpawnedByDirector = true;
    if (!ApplyPlacement(Agent, InitialEntry, true))
    {
        Agent->Destroy();
        Runtime.Agent.Reset();
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("TMOP person '%s' spawned from row '%s'."),
        *Runtime.Profile.EntityId.ToString(), *Runtime.RowName.ToString());
    return true;
}

bool ATMOPPersonRegistryDirector::ApplyTimelineEntry(FPersonRuntime& Runtime,
    const FTMOPPersonTimelineEntry& Entry, const bool bCatchUp)
{
    ATMOPHistoricalAgent* Agent = Runtime.Agent.Get();
    if (!IsValid(Agent)) return false;
    switch (Entry.Action)
    {
    case ETMOPPersonTimelineAction::InitialPlacement:
    case ETMOPPersonTimelineAction::Spawn:
        return ApplyPlacement(Agent, Entry, bCatchUp);
    case ETMOPPersonTimelineAction::Despawn:
        Agent->Destroy();
        Runtime.Agent.Reset();
        Runtime.bSpawnedByDirector = false;
        return true;
    case ETMOPPersonTimelineAction::MoveToAnchor:
        if (bCatchUp && Entry.bTeleportDuringCatchUp)
            return ApplyPlacement(Agent, Entry, true);
        if (!IsValid(Agent->ActionExecutor)) return false;
        {
            FTMOPScheduleEntry Action;
            Action.EntryId = Entry.EntryId;
            Action.ActionType = ETMOPScheduleActionType::MoveToAnchor;
            Action.TargetAnchorId = Entry.TargetAnchorId;
            Action.ActivityState = Entry.ActivityState == ETMOPAgentActivityState::Idle
                ? ETMOPAgentActivityState::Walking : Entry.ActivityState;
            Action.Confidence = Entry.Confidence;
            Action.SourceId = Entry.SourceReference;
            Action.Notes = Entry.Notes;
            return Agent->ActionExecutor->ExecuteScheduleEntry(Action);
        }
    case ETMOPPersonTimelineAction::Wait:
    case ETMOPPersonTimelineAction::ChangeActivity:
        Agent->SetActivityState(Entry.ActivityState);
        return true;
    case ETMOPPersonTimelineAction::ChangeLifeState:
        Agent->SetLifeState(Entry.LifeState);
        return true;
    case ETMOPPersonTimelineAction::SitDown:
    case ETMOPPersonTimelineAction::StandUp:
    case ETMOPPersonTimelineAction::EnterVehicle:
    case ETMOPPersonTimelineAction::ExitVehicle:
        return ApplyPlacement(Agent, Entry, bCatchUp);
    case ETMOPPersonTimelineAction::Interact:
    case ETMOPPersonTimelineAction::Custom:
        Agent->SetActivityState(ETMOPAgentActivityState::Interacting);
        return true;
    default:
        return false;
    }
}

bool ATMOPPersonRegistryDirector::ApplyPlacement(ATMOPHistoricalAgent* Agent,
    const FTMOPPersonTimelineEntry& Entry, const bool bCatchUp)
{
    if (!IsValid(Agent) || GetGameInstance() == nullptr) return false;

    if (Entry.Action == ETMOPPersonTimelineAction::StandUp)
    {
        UTMOPCinemaSeatSubsystem* Seats = GetGameInstance()->GetSubsystem<UTMOPCinemaSeatSubsystem>();
        UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(Entry.TargetSeatId) : nullptr;
        if (IsValid(Seat) && Seat->GetOccupyingAgent() == Agent) Seat->StandAgent(Agent);
        Agent->SetActivityState(ETMOPAgentActivityState::Standing);
        return true;
    }

    if (Entry.Action == ETMOPPersonTimelineAction::ExitVehicle)
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(Entry.TargetEntityId);
        if (!bCatchUp && IsValid(Vehicle) && !Entry.TargetStopId.IsNone())
        {
            UTMOPBusServiceComponent* Service =
                Vehicle->FindComponentByClass<UTMOPBusServiceComponent>();
            UTMOPBusStopComponent* Stop = IsValid(Service)
                ? Service->GetCurrentTargetStop() : nullptr;
            if (!IsValid(Service) || !Service->bDoorsOpen || !IsValid(Stop) ||
                Stop->StopId != Entry.TargetStopId) return false;
        }
        return IsValid(Vehicle) && Vehicle->ExitVehicle(Agent);
    }

    switch (Entry.LocationType)
    {
    case ETMOPPersonLocationType::Anchor:
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Entry.TargetAnchorId) : nullptr;
        if (!IsValid(Anchor)) return false;
        Agent->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        Agent->SetActorLocationAndRotation(Anchor->GetActorLocation(), Anchor->GetActorRotation(),
            false, nullptr, ETeleportType::TeleportPhysics);
        Agent->SetActivityState(Entry.ActivityState);
        return true;
    }
    case ETMOPPersonLocationType::WorldTransform:
        Agent->SetActorTransform(Entry.WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
        Agent->SetActivityState(Entry.ActivityState);
        return true;
    case ETMOPPersonLocationType::VenueSeat:
    {
        UTMOPCinemaSeatSubsystem* Seats = GetGameInstance()->GetSubsystem<UTMOPCinemaSeatSubsystem>();
        if (Seats != nullptr) Seats->DiscoverSeatsInWorld();
        UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(Entry.TargetSeatId) : nullptr;
        return IsValid(Seat) && Seat->SeatAgent(Agent);
    }
    case ETMOPPersonLocationType::VehicleSeat:
    case ETMOPPersonLocationType::BusSeat:
    {
        ATMOPVehicleBase* Vehicle = FindVehicle(Entry.TargetEntityId);
        if (!bCatchUp && IsValid(Vehicle) && !Entry.TargetStopId.IsNone())
        {
            UTMOPBusServiceComponent* Service =
                Vehicle->FindComponentByClass<UTMOPBusServiceComponent>();
            UTMOPBusStopComponent* Stop = IsValid(Service)
                ? Service->GetCurrentTargetStop() : nullptr;
            if (!IsValid(Service) || !Service->bDoorsOpen || !IsValid(Stop) ||
                Stop->StopId != Entry.TargetStopId) return false;
        }
        return IsValid(Vehicle) && Vehicle->EnterVehicle(Agent, Entry.TargetSeatId);
    }
    case ETMOPPersonLocationType::StandingInVehicle:
    {
        AActor* Vehicle = FindVehicle(Entry.TargetEntityId);
        if (!IsValid(Vehicle)) return false;
        Agent->AttachToActor(Vehicle, FAttachmentTransformRules::KeepWorldTransform);
        Agent->SetActivityState(ETMOPAgentActivityState::RidingVehicle);
        return true;
    }
    case ETMOPPersonLocationType::Unknown:
    case ETMOPPersonLocationType::NotPresent:
    default:
        return Entry.Action == ETMOPPersonTimelineAction::Wait ||
            Entry.Action == ETMOPPersonTimelineAction::ChangeActivity;
    }
}

bool ATMOPPersonRegistryDirector::IsAgentBusy(const ATMOPHistoricalAgent* Agent) const
{
    return IsValid(Agent) && IsValid(Agent->ActionExecutor) &&
        Agent->ActionExecutor->IsExecutingAction();
}

ATMOPVehicleBase* ATMOPPersonRegistryDirector::FindVehicle(const FName VehicleId) const
{
    if (VehicleId.IsNone()) return nullptr;
    UTMOPWorldSubsystem* WorldRegistry = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPWorldSubsystem>() : nullptr;
    if (WorldRegistry != nullptr)
        if (ATMOPVehicleBase* Registered =
            Cast<ATMOPVehicleBase>(WorldRegistry->FindWorldObject(VehicleId))) return Registered;
    UWorld* World = GetWorld();
    if (World == nullptr) return nullptr;
    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
        if (It->VehicleId == VehicleId) return *It;
    return nullptr;
}

ATMOPHistoricalAgent* ATMOPPersonRegistryDirector::FindSpawnedPerson(const FName EntityId) const
{
    const FPersonRuntime* Runtime = RuntimePeople.Find(EntityId);
    return Runtime != nullptr ? Runtime->Agent.Get() : nullptr;
}

bool ATMOPPersonRegistryDirector::ValidatePeopleTable(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!IsValid(PersonProfileTable) ||
        PersonProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct())
    {
        OutErrors.Add(TEXT("Person Profile Table is missing or has the wrong row structure."));
        return false;
    }

    TSet<FName> EntityIds;
    for (const FName RowName : PersonProfileTable->GetRowNames())
    {
        const FTMOPPersonProfileRow* Row = PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
            RowName, TEXT("ValidatePeople"), false);
        if (Row == nullptr) continue;
        const FString Prefix = FString::Printf(TEXT("Row '%s'"), *RowName.ToString());
        if (Row->EntityId.IsNone()) OutErrors.Add(Prefix + TEXT(" has no EntityId."));
        if (RowName != Row->EntityId) OutErrors.Add(Prefix + TEXT(" Row Name must equal EntityId."));
        if (EntityIds.Contains(Row->EntityId)) OutErrors.Add(Prefix + TEXT(" duplicates EntityId."));
        EntityIds.Add(Row->EntityId);
        if (Row->bSpawnInSimulation && Row->Timeline.IsEmpty())
            OutErrors.Add(Prefix + TEXT(" is enabled for simulation but Timeline is empty."));
        TSet<FName> EntryIds;
        int32 PreviousSecond = INDEX_NONE;
        for (int32 Index = 0; Index < Row->Timeline.Num(); ++Index)
        {
            const FTMOPPersonTimelineEntry& Entry = Row->Timeline[Index];
            if (Entry.EntryId.IsNone() || EntryIds.Contains(Entry.EntryId))
                OutErrors.Add(Prefix + FString::Printf(TEXT(" has missing/duplicate EntryId at Timeline[%d]."), Index));
            EntryIds.Add(Entry.EntryId);
            const int32 Second = Entry.Time.ToSecondsFromMidnight();
            if (PreviousSecond > Second)
                OutErrors.Add(Prefix + TEXT(" Timeline is not chronological (runtime will sort a copy)."));
            PreviousSecond = Second;
        }
        if (!Row->Timeline.IsEmpty() &&
            Row->Timeline[0].Action != ETMOPPersonTimelineAction::InitialPlacement &&
            Row->Timeline[0].Action != ETMOPPersonTimelineAction::Spawn)
            OutErrors.Add(Prefix + TEXT(" Timeline[0] must be InitialPlacement or Spawn."));
    }
    return OutErrors.IsEmpty();
}
