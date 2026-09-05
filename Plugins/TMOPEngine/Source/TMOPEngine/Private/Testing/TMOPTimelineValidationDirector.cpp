#include "Testing/TMOPTimelineValidationDirector.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/SceneComponent.h"
#include "Dom/JsonObject.h"
#include "Engine/DataTable.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Groups/TMOPGroupDirector.h"
#include "Groups/TMOPGroupTypes.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "People/TMOPPersonProfileTypes.h"
#include "Time/TMOPClockSubsystem.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"

namespace
{
FString CsvEscape(const FString& Value)
{
    FString Escaped = Value;
    Escaped.ReplaceInline(TEXT("\""), TEXT("\"\""));
    return FString::Printf(TEXT("\"%s\""), *Escaped);
}

FString SeverityText(const ETMOPTimelineValidationSeverity Severity)
{
    switch (Severity)
    {
    case ETMOPTimelineValidationSeverity::Passed: return TEXT("Passed");
    case ETMOPTimelineValidationSeverity::Warning: return TEXT("Warning");
    default: return TEXT("Error");
    }
}

template <typename TEnum>
FString EnumText(const TEnum Value)
{
    const UEnum* Enum = StaticEnum<TEnum>();
    return Enum != nullptr
        ? Enum->GetNameStringByValue(static_cast<int64>(Value))
        : TEXT("Unknown");
}

FString ActorDiagnosticName(const AActor* Actor)
{
    if (!IsValid(Actor)) return FString();
    if (const ATMOPVehicleBase* Vehicle = Cast<ATMOPVehicleBase>(Actor))
        return Vehicle->VehicleId.IsNone()
            ? Vehicle->GetName() : Vehicle->VehicleId.ToString();
    if (const ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(Actor))
        if (IsValid(Agent->EntityIdentity))
            return Agent->EntityIdentity->GetEntityId().ToString();
    return Actor->GetName();
}

FString MovementModeText(const EMovementMode Mode)
{
    switch (Mode)
    {
    case MOVE_None: return TEXT("None");
    case MOVE_Walking: return TEXT("Walking");
    case MOVE_NavWalking: return TEXT("NavWalking");
    case MOVE_Falling: return TEXT("Falling");
    case MOVE_Swimming: return TEXT("Swimming");
    case MOVE_Flying: return TEXT("Flying");
    case MOVE_Custom: return TEXT("Custom");
    default: return TEXT("Unknown");
    }
}

TSharedRef<FJsonObject> VectorJson(const FVector& Value)
{
    TSharedRef<FJsonObject> Object = MakeShared<FJsonObject>();
    Object->SetNumberField(TEXT("x"), Value.X);
    Object->SetNumberField(TEXT("y"), Value.Y);
    Object->SetNumberField(TEXT("z"), Value.Z);
    return Object;
}
}

ATMOPTimelineValidationDirector::ATMOPTimelineValidationDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.0f;
    // The shot snapshot must run after people/vehicle directors have applied
    // entries scheduled for the exact same simulation second.
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void ATMOPTimelineValidationDirector::BeginPlay()
{
    Super::BeginPlay();
    for (TActorIterator<ATMOPPersonRegistryDirector> It(GetWorld()); It; ++It)
    {
        PeopleDirector = *It;
        PeopleDirector->OnTimelineEntryApplied.AddUObject(
            this,
            &ATMOPTimelineValidationDirector::HandlePersonTimelineApplied);
        break;
    }
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It)
    {
        GroupDirector = *It;
        GroupDirector->OnGroupStateChanged.AddDynamic(
            this,
            &ATMOPTimelineValidationDirector::HandleGroupStateChanged);
        break;
    }
    for (TActorIterator<ATMOPHistoricalVehicleDirector> It(GetWorld()); It; ++It)
    {
        VehicleDirector = *It;
        VehicleDirector->OnTimelineEntryArrived.AddUObject(
            this,
            &ATMOPTimelineValidationDirector::HandleVehicleTimelineArrived);
        break;
    }
    if (bStartAutomatically) StartValidation();
}

void ATMOPTimelineValidationDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    const bool bWasActive = bValidationActive;
    bValidationActive = false;
    if (bExportOnEndPlay && bWasActive) ExportReports();
    if (PeopleDirector.IsValid())
        PeopleDirector->OnTimelineEntryApplied.RemoveAll(this);
    if (GroupDirector.IsValid())
        GroupDirector->OnGroupStateChanged.RemoveDynamic(
            this,
            &ATMOPTimelineValidationDirector::HandleGroupStateChanged);
    if (VehicleDirector.IsValid())
        VehicleDirector->OnTimelineEntryArrived.RemoveAll(this);
    for (TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
        if (Pair.Value.Executor.IsValid())
            Pair.Value.Executor->OnActionValidationEvent.RemoveAll(this);
    TrackedAgents.Reset();
    TrackedVehicles.Reset();
    Super::EndPlay(EndPlayReason);
}

void ATMOPTimelineValidationDirector::StartValidation()
{
    for (TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
        if (Pair.Value.Executor.IsValid())
            Pair.Value.Executor->OnActionValidationEvent.RemoveAll(this);
    Records.Reset();
    AgentSnapshots.Reset();
    VehicleSnapshots.Reset();
    TrackedAgents.Reset();
    TrackedVehicles.Reset();
    SampleAccumulator = 0.0f;
    NextSnapshotSecond = INDEX_NONE;
    LastObservedSecond = INDEX_NONE;
    MaximumObservedSecond = INDEX_NONE;
    bShotSnapshotCaptured = false;
    bSharedEventDefinitionsValidated = false;
    bValidationActive = true;
    DiscoverAgents();
    DiscoverVehicles();
    const int32 CurrentSecond = GetSimulationSecond();
    ValidationStartSecond = CurrentSecond;
    if (CurrentSecond != INDEX_NONE)
    {
        LastObservedSecond = CurrentSecond;
        MaximumObservedSecond = CurrentSecond;
        NextSnapshotSecond = CurrentSecond;
        CaptureSnapshots(CurrentSecond, TEXT("ValidationStart"), true);
        NextSnapshotSecond += FMath::Max(1, SnapshotIntervalSeconds);
    }
    UE_LOG(LogTemp, Display, TEXT("TMOP timeline validation started."));
}

void ATMOPTimelineValidationDirector::StopValidation(const bool bExportReports)
{
    bValidationActive = false;
    if (bExportReports) ExportReports();
}

void ATMOPTimelineValidationDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bValidationActive) return;

    if (!bSharedEventDefinitionsValidated)
    {
        ValidateSharedEventReferences();
        bSharedEventDefinitionsValidated = true;
    }

    const int32 CurrentSecond = GetSimulationSecond();
    if (CurrentSecond != INDEX_NONE)
    {
        // PIE resets the game clock before destroying the world. Exporting
        // after that reset used to create hundreds of false "despawned" and
        // "interrupted" failures at 23:00. Freeze the report at the last
        // actually observed simulation second instead.
        if (LastObservedSecond != INDEX_NONE &&
            CurrentSecond < LastObservedSecond)
        {
            bValidationActive = false;
            UE_LOG(LogTemp, Display,
                TEXT("TMOP validation detected clock rewind %d -> %d; exporting before PIE teardown."),
                LastObservedSecond, CurrentSecond);
            ExportReports();
            return;
        }
        LastObservedSecond = CurrentSecond;
        MaximumObservedSecond = MaximumObservedSecond == INDEX_NONE
            ? CurrentSecond : FMath::Max(MaximumObservedSecond, CurrentSecond);
    }

    SampleAccumulator += DeltaSeconds;
    if (SampleAccumulator < SampleIntervalSeconds) return;
    const float SampleDelta = SampleAccumulator;
    SampleAccumulator = 0.0f;
    DiscoverAgents();
    DiscoverVehicles();
    SampleAgents(SampleDelta);
    SampleVehicles(SampleDelta);

    const int32 ShotSecond = ShotSnapshotTime.ToSecondsFromMidnight();
    if (!bShotSnapshotCaptured && CurrentSecond != INDEX_NONE &&
        CurrentSecond >= ShotSecond)
    {
        CaptureSnapshots(CurrentSecond, TEXT("ShotMoment"),
            bCaptureAllAgentsAtShot);
        bShotSnapshotCaptured = true;
    }
    if (CurrentSecond != INDEX_NONE && NextSnapshotSecond != INDEX_NONE &&
        CurrentSecond >= NextSnapshotSecond)
    {
        CaptureSnapshots(CurrentSecond, TEXT("Periodic"), false);
        NextSnapshotSecond = CurrentSecond +
            FMath::Max(1, SnapshotIntervalSeconds);
    }
}

void ATMOPTimelineValidationDirector::DiscoverAgents()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPHistoricalAgent> It(World); It; ++It)
    {
        ATMOPHistoricalAgent* Agent = *It;
        if (!IsValid(Agent) || !IsValid(Agent->ActionExecutor) ||
            !IsValid(Agent->EntityIdentity)) continue;
        const FName EntityId = Agent->EntityIdentity->GetEntityId();
        if (EntityId.IsNone()) continue;
        FTrackedAgent* Existing = TrackedAgents.Find(EntityId);
        if (Existing != nullptr && Existing->Agent.IsValid()) continue;
        if (Existing != nullptr && Existing->Executor.IsValid())
            Existing->Executor->OnActionValidationEvent.RemoveAll(this);

        FTrackedAgent Tracked;
        Tracked.Agent = Agent;
        Tracked.Executor = Agent->ActionExecutor;
        Tracked.LastLocation = Agent->GetActorLocation();
        // Older ActionExecutor builds do not expose GetCurrentEntryId(). The
        // validation delegate below supplies the active ID as soon as an
        // action begins, so discovery can safely start with no active entry.
        Tracked.ActiveEntryId = NAME_None;
        Agent->ActionExecutor->OnActionValidationEvent.AddUObject(
            this, &ATMOPTimelineValidationDirector::HandleActionValidation);
        TrackedAgents.Add(EntityId, MoveTemp(Tracked));

        FTMOPTimelineValidationRecord Spawn;
        Spawn.EntityId = EntityId;
        Spawn.Event = TEXT("Spawned");
        Spawn.ActualSecond = GetSimulationSecond();
        Spawn.ActualLocation = Agent->GetActorLocation();
        Spawn.Message = TEXT("Agent discovered in simulation.");
        AddRecord(Spawn);
    }
}

void ATMOPTimelineValidationDirector::DiscoverVehicles()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
    {
        ATMOPVehicleBase* Vehicle = *It;
        if (!IsValid(Vehicle)) continue;
        const FName VehicleId = Vehicle->VehicleId.IsNone()
            ? FName(*Vehicle->GetName()) : Vehicle->VehicleId;
        FTrackedVehicle* Existing = TrackedVehicles.Find(VehicleId);
        if (Existing != nullptr && Existing->Vehicle.IsValid()) continue;

        FTrackedVehicle Tracked;
        Tracked.Vehicle = Vehicle;
        Tracked.LastLocation = Vehicle->GetActorLocation();
        TrackedVehicles.Add(VehicleId, MoveTemp(Tracked));
        CaptureVehicleSnapshot(
            VehicleId, TrackedVehicles.FindChecked(VehicleId),
            GetSimulationSecond(), TEXT("VehicleDiscovered"));
    }
}

void ATMOPTimelineValidationDirector::SampleVehicles(const float DeltaSeconds)
{
    for (TPair<FName, FTrackedVehicle>& Pair : TrackedVehicles)
    {
        FTrackedVehicle& Tracked = Pair.Value;
        ATMOPVehicleBase* Vehicle = Tracked.Vehicle.Get();
        if (!IsValid(Vehicle))
        {
            if (!Tracked.bMissingReported)
            {
                FTMOPVehicleValidationSnapshot Missing;
                Missing.VehicleId = Pair.Key;
                Missing.SampleSecond = GetSimulationSecond();
                Missing.Reason = TEXT("VehicleMissingOrDespawned");
                Missing.Location = Tracked.LastLocation;
                VehicleSnapshots.Add(MoveTemp(Missing));
                Tracked.bMissingReported = true;
            }
            continue;
        }
        const float MovedCm = FVector::Dist2D(
            Tracked.LastLocation, Vehicle->GetActorLocation());
        Tracked.LastLocation = Vehicle->GetActorLocation();
        const UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        const bool bExpectedToMove = IsValid(Movement) &&
            Movement->IsDrivingEnabled() &&
            Movement->TrafficState != ETMOPTrafficVehicleState::RouteComplete;
        if (bExpectedToMove && MovedCm <= StationaryDistanceCm)
            Tracked.StationarySeconds += DeltaSeconds;
        else
            Tracked.StationarySeconds = 0.0f;
        Tracked.MaximumStationarySeconds = FMath::Max(
            Tracked.MaximumStationarySeconds, Tracked.StationarySeconds);
    }
}

bool ATMOPTimelineValidationDirector::ShouldCapturePeriodicAgent(
    const FTrackedAgent& Tracked) const
{
    if (bCaptureAllAgentsPeriodically) return true;
    const ATMOPHistoricalAgent* Agent = Tracked.Agent.Get();
    if (!bCaptureImportantAgentsPeriodically || !IsValid(Agent)) return false;
    if (!Tracked.ActiveEntryId.IsNone() ||
        Cast<ATMOPVehicleBase>(Agent->GetAttachParentActor()) != nullptr ||
        !Agent->SocialGroupId.IsNone())
        return true;
    const FString Category = Agent->PersonCategoryId.ToString().ToUpper();
    return Category.Contains(TEXT("MAIN")) ||
        Category.Contains(TEXT("PALME")) ||
        Category.Contains(TEXT("POLICE")) ||
        Category.Contains(TEXT("AMBULANCE")) ||
        Category.Contains(TEXT("SUSPECT"));
}

void ATMOPTimelineValidationDirector::CaptureSnapshots(
    const int32 SampleSecond,
    const FString& Reason,
    const bool bForceAllAgents)
{
    for (const TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
        if (bForceAllAgents || ShouldCapturePeriodicAgent(Pair.Value))
            CaptureAgentSnapshot(Pair.Key, Pair.Value, SampleSecond, Reason);
    if (bCaptureAllVehiclesPeriodically || Reason != TEXT("Periodic"))
        for (const TPair<FName, FTrackedVehicle>& Pair : TrackedVehicles)
            CaptureVehicleSnapshot(Pair.Key, Pair.Value, SampleSecond, Reason);
}

bool ATMOPTimelineValidationDirector::FindVehicleSeatForAgent(
    const ATMOPVehicleBase* Vehicle,
    const ATMOPHistoricalAgent* Agent,
    FName& OutSeatId) const
{
    OutSeatId = NAME_None;
    if (!IsValid(Vehicle) || !IsValid(Agent)) return false;
    for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
        if (IsValid(Seat) && Seat->GetOccupant() == Agent)
        {
            OutSeatId = Seat->SeatId;
            return true;
        }
    return false;
}

ATMOPVehicleBase* ATMOPTimelineValidationDirector::FindVehicle(
    const FName VehicleId) const
{
    if (VehicleDirector.IsValid())
        if (ATMOPVehicleBase* Vehicle =
            VehicleDirector->FindHistoricalVehicle(VehicleId))
            return Vehicle;
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPVehicleBase> It(GetWorld()); It; ++It)
        if (It->VehicleId == VehicleId) return *It;
    return nullptr;
}

bool ATMOPTimelineValidationDirector::FindExpectedShotAnchor(
    const FName EntityId,
    FName& OutAnchorId) const
{
    OutAnchorId = NAME_None;
    if (!PeopleDirector.IsValid() ||
        !IsValid(PeopleDirector->PersonProfileTable)) return false;
    const FTMOPPersonProfileRow* Row =
        PeopleDirector->PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
            EntityId, TEXT("ShotValidation"), false);
    if (Row == nullptr)
        for (const TPair<FName, uint8*>& Pair :
            PeopleDirector->PersonProfileTable->GetRowMap())
        {
            const FTMOPPersonProfileRow* Candidate =
                reinterpret_cast<const FTMOPPersonProfileRow*>(Pair.Value);
            if (Candidate != nullptr && Candidate->EntityId == EntityId)
            {
                Row = Candidate;
                break;
            }
        }
    if (Row == nullptr) return false;

    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    if (Anchors == nullptr) return false;

    // Prefer the naming convention exported by the level. This is deliberately
    // strict: a later trip to Mordplatsen must not make a passenger appear to
    // be expected there when the shots are fired.
    const FName ExactAnchor(*FString::Printf(
        TEXT("ANCHOR_SHOT1_%s"), *EntityId.ToString()));
    if (IsValid(Anchors->FindAnchor(ExactAnchor)))
    {
        OutAnchorId = ExactAnchor;
        return true;
    }

    for (const FName VehicleId : Row->AssociatedVehicleIds)
        if (FindExpectedShotAnchorForVehicle(VehicleId, OutAnchorId))
            return true;

    const int32 ShotSecond = ShotSnapshotTime.ToSecondsFromMidnight();
    int32 BestScore = TNumericLimits<int32>::Max();
    for (const FTMOPPersonTimelineEntry& Entry : Row->Timeline)
    {
        const FString Anchor = Entry.TargetAnchorId.ToString();
        const bool bExplicitShotAnchor = Anchor.StartsWith(TEXT("ANCHOR_SHOT1_"));
        const bool bMurderSite =
            Entry.TargetAnchorId == FName(TEXT("Mordplatsen"));
        if (Entry.Action != ETMOPPersonTimelineAction::MoveToAnchor ||
            !Entry.bTimeIsArrival || (!bExplicitShotAnchor && !bMurderSite))
            continue;
        const int32 Score = FMath::Abs(
            Entry.Time.ToSecondsFromMidnight() - ShotSecond);
        if (Score <= ShotTimelineMatchWindowSeconds && Score < BestScore &&
            IsValid(Anchors->FindAnchor(Entry.TargetAnchorId)))
        {
            BestScore = Score;
            OutAnchorId = Entry.TargetAnchorId;
        }
    }
    return !OutAnchorId.IsNone();
}

bool ATMOPTimelineValidationDirector::FindExpectedShotAnchorForVehicle(
    const FName VehicleId,
    FName& OutAnchorId) const
{
    OutAnchorId = NAME_None;
    if (VehicleId.IsNone() || GetGameInstance() == nullptr) return false;
    UTMOPAnchorSubsystem* Anchors =
        GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
    if (Anchors == nullptr) return false;

    FString Suffix = VehicleId.ToString();
    Suffix.RemoveFromStart(TEXT("VEHICLE_"), ESearchCase::IgnoreCase);
    const FName ExactAnchor(*FString::Printf(
        TEXT("ANCHOR_SHOT1_%s"), *Suffix));
    if (IsValid(Anchors->FindAnchor(ExactAnchor)))
    {
        OutAnchorId = ExactAnchor;
        return true;
    }
    return false;
}

bool ATMOPTimelineValidationDirector::FindNearestShotAnchor(
    const FVector& Location,
    FName& OutAnchorId,
    float& OutDistanceCm) const
{
    OutAnchorId = NAME_None;
    OutDistanceCm = -1.0f;
    if (GetGameInstance() == nullptr) return false;
    UTMOPAnchorSubsystem* Anchors =
        GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
    if (Anchors == nullptr) return false;

    float BestDistance = TNumericLimits<float>::Max();
    for (const ATMOPHistoricalAnchor* Anchor :
        Anchors->GetAnchorsByCategory(ETMOPAnchorCategory::WitnessPosition))
    {
        if (!IsValid(Anchor) ||
            !Anchor->GetAnchorId().ToString().StartsWith(TEXT("ANCHOR_SHOT1_")))
            continue;
        const float Distance = FVector::Dist2D(
            Location, Anchor->GetAnchorLocation());
        if (Distance < BestDistance)
        {
            BestDistance = Distance;
            OutAnchorId = Anchor->GetAnchorId();
        }
    }
    if (OutAnchorId.IsNone()) return false;
    OutDistanceCm = BestDistance;
    return true;
}

void ATMOPTimelineValidationDirector::CaptureAgentSnapshot(
    const FName EntityId,
    const FTrackedAgent& Tracked,
    const int32 SampleSecond,
    const FString& Reason)
{
    const ATMOPHistoricalAgent* Agent = Tracked.Agent.Get();
    if (!IsValid(Agent)) return;
    FTMOPAgentValidationSnapshot Snapshot;
    Snapshot.EntityId = EntityId;
    Snapshot.SampleSecond = SampleSecond;
    Snapshot.Reason = Reason;
    Snapshot.Location = Agent->GetActorLocation();
    Snapshot.Velocity = Agent->GetVelocity();
    Snapshot.ActivityState = EnumText(Agent->ActivityState);
    Snapshot.LifeState = EnumText(Agent->LifeState);
    Snapshot.ActiveEntryId = Tracked.ActiveEntryId;
    Snapshot.TargetAnchorId = Tracked.ActiveTargetAnchorId;
    Snapshot.StationarySeconds = Tracked.StationarySeconds;
    Snapshot.AttachedParentName = ActorDiagnosticName(
        Agent->GetAttachParentActor());
    Snapshot.GroupId = Agent->SocialGroupId;
    Snapshot.bCollisionEnabled = Agent->GetActorEnableCollision();
    FindNearestShotAnchor(Snapshot.Location,
        Snapshot.NearestShotAnchorId,
        Snapshot.DistanceToNearestShotAnchorCm);

    const AAIController* Controller =
        Cast<AAIController>(Agent->GetController());
    if (IsValid(Controller))
    {
        Snapshot.ControllerName = Controller->GetName();
        Snapshot.PathFollowingStatus = EnumText(Controller->GetMoveStatus());
        Snapshot.NavigationGoal = Controller->GetImmediateMoveDestination();
        if (!Snapshot.NavigationGoal.IsNearlyZero())
            Snapshot.DistanceToNavigationGoalCm = FVector::Dist2D(
                Snapshot.Location, Snapshot.NavigationGoal);
    }
    if (const UCharacterMovementComponent* CharacterMovement =
        Agent->GetCharacterMovement())
        Snapshot.MovementMode = MovementModeText(
            CharacterMovement->MovementMode.GetValue());
    if (const UCapsuleComponent* Capsule = Agent->GetCapsuleComponent())
        Snapshot.CollisionProfileName =
            Capsule->GetCollisionProfileName().ToString();

    if (const UNavigationSystemV1* Navigation =
        FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
    {
        FNavLocation Projected;
        Snapshot.bProjectedToNavMesh = Navigation->ProjectPointToNavigation(
            Snapshot.Location, Projected, FVector(100.0f, 100.0f, 300.0f));
        if (Snapshot.bProjectedToNavMesh)
        {
            const float CapsuleHalfHeight = IsValid(Agent->GetCapsuleComponent())
                ? Agent->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()
                : 0.0f;
            Snapshot.DistanceToNavMeshCm = FMath::Max(0.0f,
                FVector::Dist(Snapshot.Location, Projected.Location) -
                CapsuleHalfHeight);
            Snapshot.bOnNavMesh = Snapshot.DistanceToNavMeshCm <= 50.0f;
        }
    }

    if (!Tracked.ActiveTargetAnchorId.IsNone() && GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Tracked.ActiveTargetAnchorId) : nullptr;
        if (IsValid(Anchor))
            Snapshot.DistanceToTargetCm = FVector::Dist2D(
                Snapshot.Location, Anchor->GetAnchorLocation());
    }

    if (FindExpectedShotAnchor(EntityId, Snapshot.ExpectedShotAnchorId) &&
        GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* ShotAnchor = Anchors != nullptr
            ? Anchors->FindAnchor(Snapshot.ExpectedShotAnchorId) : nullptr;
        if (IsValid(ShotAnchor))
        {
            Snapshot.DistanceToExpectedShotAnchorCm = FVector::Dist2D(
                Snapshot.Location, ShotAnchor->GetAnchorLocation());
            Snapshot.bAtExpectedShotAnchor =
                Snapshot.DistanceToExpectedShotAnchorCm <=
                    ShotAnchorToleranceCm;
        }
    }

    const ATMOPVehicleBase* AttachedVehicle =
        Cast<ATMOPVehicleBase>(Agent->GetAttachParentActor());
    if (!IsValid(AttachedVehicle))
        for (const TPair<FName, FTrackedVehicle>& VehiclePair : TrackedVehicles)
        {
            FName SeatId;
            if (FindVehicleSeatForAgent(
                VehiclePair.Value.Vehicle.Get(), Agent, SeatId))
            {
                AttachedVehicle = VehiclePair.Value.Vehicle.Get();
                break;
            }
        }
    if (IsValid(AttachedVehicle))
    {
        Snapshot.VehicleId = AttachedVehicle->VehicleId;
        FindVehicleSeatForAgent(AttachedVehicle, Agent, Snapshot.SeatId);
    }

    if (!Snapshot.GroupId.IsNone() && GroupDirector.IsValid())
    {
        bool bFound = false;
        const FTMOPGroupSnapshot Group =
            GroupDirector->GetGroupSnapshot(Snapshot.GroupId, bFound);
        if (bFound)
        {
            Snapshot.GroupLeaderId = Group.LeaderEntityId;
            Snapshot.GroupState = EnumText(Group.State);
            Snapshot.GroupFormation = EnumText(Group.Formation);
            const FTrackedAgent* Leader =
                TrackedAgents.Find(Group.LeaderEntityId);
            if (Leader != nullptr && Leader->Agent.IsValid())
                Snapshot.DistanceToGroupLeaderCm = FVector::Dist2D(
                    Snapshot.Location, Leader->Agent->GetActorLocation());
        }
    }
    AgentSnapshots.Add(MoveTemp(Snapshot));
}

bool ATMOPTimelineValidationDirector::FindNextVehicleStop(
    const FName VehicleId,
    const int32 CurrentSecond,
    FName& OutAnchorId,
    int32& OutStopSecond) const
{
    OutAnchorId = NAME_None;
    OutStopSecond = INDEX_NONE;
    if (!VehicleDirector.IsValid() ||
        !IsValid(VehicleDirector->HistoricalVehicleTable)) return false;
    const FTMOPHistoricalVehicleRow* Row =
        VehicleDirector->HistoricalVehicleTable->FindRow<
            FTMOPHistoricalVehicleRow>(VehicleId, TEXT("Validation"), false);
    if (Row == nullptr)
        for (const TPair<FName, uint8*>& Pair :
            VehicleDirector->HistoricalVehicleTable->GetRowMap())
        {
            const FTMOPHistoricalVehicleRow* Candidate =
                reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
            if (Candidate != nullptr && Candidate->VehicleId == VehicleId)
            {
                Row = Candidate;
                break;
            }
        }
    if (Row == nullptr) return false;
    int32 BestTimeDistance = TNumericLimits<int32>::Max();
    for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Row->Timeline)
    {
        const bool bStop = Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
            Entry.Action == ETMOPHistoricalVehicleAction::Park ||
            Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement;
        const int32 StopSecond = Entry.Time.ToSecondsFromMidnight();
        const int32 TimeDistance = FMath::Abs(StopSecond - CurrentSecond);
        // The closest scheduled stop is more useful than only the next stop:
        // just after a missed stop we must keep reporting that missed anchor.
        if (bStop && TimeDistance < BestTimeDistance)
        {
            BestTimeDistance = TimeDistance;
            OutStopSecond = StopSecond;
            OutAnchorId = Entry.PlacementAnchorId;
        }
    }
    return OutStopSecond != INDEX_NONE;
}

void ATMOPTimelineValidationDirector::CaptureVehicleSnapshot(
    const FName VehicleId,
    const FTrackedVehicle& Tracked,
    const int32 SampleSecond,
    const FString& Reason)
{
    const ATMOPVehicleBase* Vehicle = Tracked.Vehicle.Get();
    if (!IsValid(Vehicle)) return;
    FTMOPVehicleValidationSnapshot Snapshot;
    Snapshot.VehicleId = VehicleId;
    Snapshot.SampleSecond = SampleSecond;
    Snapshot.Reason = Reason;
    Snapshot.Location = Vehicle->GetActorLocation();
    Snapshot.YawDegrees = Vehicle->GetActorRotation().Yaw;
    Snapshot.StationarySeconds = Tracked.StationarySeconds;
    Snapshot.bCollisionEnabled = Vehicle->GetActorEnableCollision();

    if (FindExpectedShotAnchorForVehicle(
        VehicleId, Snapshot.ExpectedShotAnchorId) &&
        GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* ShotAnchor = Anchors != nullptr
            ? Anchors->FindAnchor(Snapshot.ExpectedShotAnchorId) : nullptr;
        if (IsValid(ShotAnchor))
        {
            Snapshot.DistanceToExpectedShotAnchorCm = FVector::Dist2D(
                Snapshot.Location, ShotAnchor->GetAnchorLocation());
            Snapshot.bAtExpectedShotAnchor =
                Snapshot.DistanceToExpectedShotAnchorCm <=
                    ShotAnchorToleranceCm;
        }
    }

    if (VehicleDirector.IsValid() &&
        IsValid(VehicleDirector->HistoricalVehicleTable))
    {
        const FTMOPHistoricalVehicleRow* Row =
            VehicleDirector->HistoricalVehicleTable->FindRow<
                FTMOPHistoricalVehicleRow>(VehicleId, TEXT("ValidationSnapshot"), false);
        if (Row == nullptr)
            for (const TPair<FName, uint8*>& Pair :
                VehicleDirector->HistoricalVehicleTable->GetRowMap())
            {
                const FTMOPHistoricalVehicleRow* Candidate =
                    reinterpret_cast<const FTMOPHistoricalVehicleRow*>(Pair.Value);
                if (Candidate != nullptr && Candidate->VehicleId == VehicleId)
                {
                    Row = Candidate;
                    break;
                }
            }
        if (Row != nullptr)
        {
            Snapshot.ProfileTimelineEntryCount = Row->Timeline.Num();
            Snapshot.ProfileStopEntryCount = 0;
            for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Row->Timeline)
                if (Entry.Action == ETMOPHistoricalVehicleAction::Stop ||
                    Entry.Action == ETMOPHistoricalVehicleAction::Park)
                    ++Snapshot.ProfileStopEntryCount;
        }
    }

    const UTMOPTrafficVehicleMovementComponent* Movement =
        Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    if (IsValid(Movement))
    {
        Snapshot.SpeedCmPerSecond = Movement->CurrentSpeedCmPerSecond;
        Snapshot.TrafficState = EnumText(Movement->TrafficState);
        Snapshot.CurrentLaneId = Movement->CurrentLaneId;
        Snapshot.DistanceAlongLaneCm = Movement->DistanceAlongLane;
        Snapshot.bObstacleDetectionEnabled = Movement->bDetectPhysicalObstacles;
        Snapshot.bHasStopConstraint = Movement->HasExternalStopConstraint();
        const float StopDistance = Movement->GetNearestActiveStopDistance();
        Snapshot.RemainingStopConstraintCm = StopDistance >= 0.0f
            ? StopDistance - Movement->DistanceAlongLane : -1.0f;
        const int32 LaneIndex =
            Movement->PlannedLaneIds.IndexOfByKey(Movement->CurrentLaneId);
        if (LaneIndex != INDEX_NONE)
        {
            Snapshot.RemainingLaneCount =
                FMath::Max(0, Movement->PlannedLaneIds.Num() - LaneIndex - 1);
            if (LaneIndex + 1 < Movement->PlannedLaneIds.Num())
                Snapshot.NextLaneId = Movement->PlannedLaneIds[LaneIndex + 1];
        }
        float ObstacleDistance = -1.0f;
        AActor* BlockingActor = nullptr;
        if (Movement->GetPhysicalObstacleDiagnostics(
            ObstacleDistance, BlockingActor))
        {
            Snapshot.BlockingActorName = ActorDiagnosticName(BlockingActor);
            Snapshot.BlockingActorClass = IsValid(BlockingActor)
                ? BlockingActor->GetClass()->GetName() : FString();
            Snapshot.BlockingActorDistanceCm = ObstacleDistance;
        }
    }

    FindNextVehicleStop(VehicleId, SampleSecond,
        Snapshot.PlannedStopAnchorId, Snapshot.PlannedStopSecond);
    if (!Snapshot.PlannedStopAnchorId.IsNone() && GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Snapshot.PlannedStopAnchorId) : nullptr;
        if (IsValid(Anchor))
            Snapshot.DistanceToPlannedStopCm = FVector::Dist2D(
                Snapshot.Location, Anchor->GetAnchorLocation());
    }

    TArray<FString> Seats;
    for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
        if (IsValid(Seat) && IsValid(Seat->GetOccupantCharacter()))
            Seats.Add(FString::Printf(TEXT("%s=%s"),
                *Seat->SeatId.ToString(),
                *ActorDiagnosticName(Seat->GetOccupantCharacter())));
    Snapshot.OccupiedSeats = FString::Join(Seats, TEXT(";"));

    TArray<FString> Nearby;
    for (const TPair<FName, FTrackedVehicle>& Other : TrackedVehicles)
        if (Other.Key != VehicleId && Other.Value.Vehicle.IsValid() &&
            FVector::Dist2D(Snapshot.Location,
                Other.Value.Vehicle->GetActorLocation()) <= NearbyVehicleRadiusCm)
            Nearby.Add(Other.Key.ToString());
    Nearby.Sort();
    Snapshot.NearbyVehicleIds = FString::Join(Nearby, TEXT(";"));
    VehicleSnapshots.Add(MoveTemp(Snapshot));
}

void ATMOPTimelineValidationDirector::SampleAgents(const float DeltaSeconds)
{
    for (TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
    {
        FTrackedAgent& Tracked = Pair.Value;
        ATMOPHistoricalAgent* Agent = Tracked.Agent.Get();
        UTMOPActionExecutorComponent* Executor = Tracked.Executor.Get();
        if (!IsValid(Agent) || !IsValid(Executor))
        {
            if (!Tracked.bMissingReported)
            {
                FTMOPTimelineValidationRecord Missing;
                Missing.EntityId = Pair.Key;
                Missing.Event = TEXT("AgentMissingOrDespawned");
                Missing.ActualSecond = GetSimulationSecond();
                Missing.ActualLocation = Tracked.LastLocation;
                Missing.Severity = ETMOPTimelineValidationSeverity::Warning;
                Missing.Message = TEXT("Previously tracked agent is no longer valid in the world.");
                AddRecord(Missing);
                Tracked.bMissingReported = true;
            }
            continue;
        }

        FVector Target;
        bool bMoving = Executor->TryGetActiveMoveTarget(Target);
        if (Tracked.bRegistryManagedMove && !Agent->SocialGroupId.IsNone() &&
            GroupDirector.IsValid())
        {
            bool bFoundGroup = false;
            const FTMOPGroupSnapshot Group = GroupDirector->GetGroupSnapshot(
                Agent->SocialGroupId, bFoundGroup);
            if (bFoundGroup && Group.State == ETMOPGroupState::Arrived)
            {
                if (Tracked.bRegistryManagedTimedArrival &&
                    Tracked.ActivePlannedSecond != INDEX_NONE &&
                    !Tracked.ActiveEntryId.IsNone())
                {
                    FTMOPTimelineValidationRecord Arrival;
                    Arrival.EntityId = Pair.Key;
                    Arrival.EntryId = Tracked.ActiveEntryId;
                    Arrival.Event = TEXT("Completed");
                    Arrival.Action = TEXT("Move To Anchor");
                    Arrival.TargetAnchorId = Tracked.ActiveTargetAnchorId;
                    Arrival.PlannedSecond = Tracked.ActivePlannedSecond;
                    Arrival.ActualSecond = GetSimulationSecond();
                    Arrival.TimeDeviationSeconds =
                        Arrival.ActualSecond - Arrival.PlannedSecond;
                    Arrival.bScheduledAsArrival = true;
                    Arrival.ActualLocation = Agent->GetActorLocation();
                    PopulateRecordRuntimeDiagnostics(Arrival, &Tracked);
                    const float AbsDeviation =
                        FMath::Abs(Arrival.TimeDeviationSeconds);
                    if (AbsDeviation > TimingErrorSeconds)
                        Arrival.Severity =
                            ETMOPTimelineValidationSeverity::Error;
                    else if (AbsDeviation > TimingWarningSeconds)
                        Arrival.Severity =
                            ETMOPTimelineValidationSeverity::Warning;
                    Arrival.Message = AbsDeviation < 0.5f
                        ? TEXT("Group arrived at the historical deadline.")
                        : FString::Printf(
                            TEXT("Group arrived %.0f seconds %s the historical deadline."),
                            AbsDeviation,
                            Arrival.TimeDeviationSeconds >= 0.0f
                                ? TEXT("late") : TEXT("early"));
                    AddRecord(Arrival);
                }
                Tracked.ActiveEntryId = NAME_None;
                Tracked.ActiveTargetAnchorId = NAME_None;
                Tracked.ActivePlannedSecond = INDEX_NONE;
                Tracked.bRegistryManagedMove = false;
                Tracked.bRegistryManagedTimedArrival = false;
                Tracked.StationarySeconds = 0.0f;
                Tracked.bStuckReportedForCurrentMove = false;
                Tracked.LastLocation = Agent->GetActorLocation();
                continue;
            }
            if (bFoundGroup && Group.LeaderEntityId != Pair.Key)
            {
                // Followers can pause while the leader advances. Only the
                // leader represents whole-group progress for stuck detection.
                Tracked.StationarySeconds = 0.0f;
                Tracked.bStuckReportedForCurrentMove = false;
                Tracked.LastLocation = Agent->GetActorLocation();
                continue;
            }
        }
        if (!bMoving && Tracked.bRegistryManagedMove &&
            Agent->ActivityState == ETMOPAgentActivityState::Walking)
        {
            if (const AAIController* Controller =
                Cast<AAIController>(Agent->GetController()))
            {
                Target = Controller->GetImmediateMoveDestination();
                bMoving = !Target.IsNearlyZero();
            }
        }
        if (!bMoving)
        {
            Tracked.StationarySeconds = 0.0f;
            Tracked.bStuckReportedForCurrentMove = false;
            Tracked.LastLocation = Agent->GetActorLocation();
            continue;
        }

        // A character that is already inside the accepted target radius is
        // not meaningfully stuck. Navigation may spend a few samples waiting
        // for the move-complete callback, especially in dense cinema crowds.
        // Reporting that as an error produced false positives at 6–120 cm.
        if (FVector::Dist2D(Agent->GetActorLocation(), Target) <=
            ArrivalWarningDistanceCm)
        {
            Tracked.StationarySeconds = 0.0f;
            Tracked.bStuckReportedForCurrentMove = false;
            Tracked.LastLocation = Agent->GetActorLocation();
            continue;
        }

        const float MovedCm = FVector::Dist2D(
            Tracked.LastLocation, Agent->GetActorLocation());
        Tracked.LastLocation = Agent->GetActorLocation();
        if (MovedCm <= StationaryDistanceCm)
            Tracked.StationarySeconds += DeltaSeconds;
        else
            Tracked.StationarySeconds = 0.0f;

        if (Tracked.StationarySeconds >= StuckAfterSeconds &&
            !Tracked.bStuckReportedForCurrentMove)
        {
            Tracked.bStuckReportedForCurrentMove = true;
            FTMOPTimelineValidationRecord Record;
            Record.EntityId = Pair.Key;
            Record.EntryId = Tracked.ActiveEntryId;
            Record.Event = TEXT("Stuck");
            Record.ActualSecond = GetSimulationSecond();
            Record.ActualLocation = Agent->GetActorLocation();
            Record.DistanceToTargetCm = FVector::Dist2D(
                Agent->GetActorLocation(), Target);
            Record.Severity = ETMOPTimelineValidationSeverity::Error;
            Record.Message = FString::Printf(
                TEXT("Moved no more than %.0f cm per sample for %.1f seconds."),
                StationaryDistanceCm, Tracked.StationarySeconds);
            PopulateRecordRuntimeDiagnostics(Record, &Tracked);
            AddRecord(Record);
            CaptureAgentSnapshot(Pair.Key, Tracked,
                Record.ActualSecond, TEXT("Stuck"));
        }
    }
}

void ATMOPTimelineValidationDirector::PopulateRecordRuntimeDiagnostics(
    FTMOPTimelineValidationRecord& Record,
    const FTrackedAgent* Tracked) const
{
    const ATMOPHistoricalAgent* Agent = Tracked != nullptr
        ? Tracked->Agent.Get() : nullptr;
    if (!IsValid(Agent)) return;
    Record.bActorCollisionEnabled = Agent->GetActorEnableCollision();
    Record.AttachedParentName = ActorDiagnosticName(
        Agent->GetAttachParentActor());
    Record.GroupId = Agent->SocialGroupId;

    if (FindExpectedShotAnchor(Record.EntityId, Record.ExpectedShotAnchorId) &&
        GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* ShotAnchor = Anchors != nullptr
            ? Anchors->FindAnchor(Record.ExpectedShotAnchorId) : nullptr;
        if (IsValid(ShotAnchor))
        {
            Record.DistanceToExpectedShotAnchorCm = FVector::Dist2D(
                Agent->GetActorLocation(), ShotAnchor->GetAnchorLocation());
            Record.bAtExpectedShotAnchor =
                Record.DistanceToExpectedShotAnchorCm <=
                    ShotAnchorToleranceCm;
        }
    }

    const ATMOPVehicleBase* Vehicle =
        Cast<ATMOPVehicleBase>(Agent->GetAttachParentActor());
    FName SeatId = NAME_None;
    if (!IsValid(Vehicle))
        for (const TPair<FName, FTrackedVehicle>& Pair : TrackedVehicles)
            if (FindVehicleSeatForAgent(Pair.Value.Vehicle.Get(), Agent, SeatId))
            {
                Vehicle = Pair.Value.Vehicle.Get();
                break;
            }
    if (IsValid(Vehicle))
    {
        Record.ActualVehicleId = Vehicle->VehicleId;
        if (SeatId.IsNone()) FindVehicleSeatForAgent(Vehicle, Agent, SeatId);
        Record.ActualSeatId = SeatId;
        const UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        if (IsValid(Movement))
        {
            Record.VehicleTrafficState = EnumText(Movement->TrafficState);
            Record.VehicleLaneId = Movement->CurrentLaneId;
            Record.VehicleSpeedCmPerSecond = Movement->CurrentSpeedCmPerSecond;
            const int32 LaneIndex =
                Movement->PlannedLaneIds.IndexOfByKey(Movement->CurrentLaneId);
            Record.VehicleRouteRemaining = LaneIndex != INDEX_NONE
                ? FMath::Max(0,
                    Movement->PlannedLaneIds.Num() - LaneIndex - 1)
                : Movement->PlannedLaneIds.Num();
            Record.bVehicleObstacleDetectionEnabled =
                Movement->bDetectPhysicalObstacles;
            float DistanceCm = -1.0f;
            AActor* BlockingActor = nullptr;
            if (Movement->GetPhysicalObstacleDiagnostics(
                DistanceCm, BlockingActor))
            {
                Record.BlockingActorName = ActorDiagnosticName(BlockingActor);
                Record.BlockingActorClass = IsValid(BlockingActor)
                    ? BlockingActor->GetClass()->GetName() : FString();
                Record.BlockingActorDistanceCm = DistanceCm;
            }
        }
    }

    if (!Record.GroupId.IsNone() && GroupDirector.IsValid())
    {
        bool bFound = false;
        const FTMOPGroupSnapshot Group =
            GroupDirector->GetGroupSnapshot(Record.GroupId, bFound);
        if (bFound)
        {
            Record.GroupLeaderId = Group.LeaderEntityId;
            Record.GroupState = EnumText(Group.State);
            Record.GroupFormation = EnumText(Group.Formation);
            const FTrackedAgent* Leader =
                TrackedAgents.Find(Group.LeaderEntityId);
            if (Leader != nullptr && Leader->Agent.IsValid())
                Record.DistanceToGroupLeaderCm = FVector::Dist2D(
                    Agent->GetActorLocation(),
                    Leader->Agent->GetActorLocation());
        }
    }
}

void ATMOPTimelineValidationDirector::DiagnosePersonFailure(
    const FTMOPPersonTimelineEntry& Entry,
    const FTrackedAgent* Tracked,
    FString& OutCode,
    FString& OutDetails) const
{
    OutCode = TEXT("UnknownApplyFailure");
    OutDetails = TEXT("The action returned false without a more specific runtime condition.");
    const ATMOPHistoricalAgent* Agent = Tracked != nullptr
        ? Tracked->Agent.Get() : nullptr;

    if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor)
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
        if (Anchors == nullptr ||
            !IsValid(Anchors->FindAnchor(Entry.TargetAnchorId)))
        {
            OutCode = TEXT("AnchorNotFound");
            OutDetails = FString::Printf(TEXT("Target anchor '%s' is unavailable."),
                *Entry.TargetAnchorId.ToString());
        }
        else
        {
            OutCode = TEXT("NavigationStartRejected");
            OutDetails = TEXT("The target anchor exists, but navigation/group movement did not start.");
        }
        return;
    }

    const bool bVehicleAction =
        Entry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
        Entry.Action == ETMOPPersonTimelineAction::ExitVehicle ||
        Entry.Action == ETMOPPersonTimelineAction::BeginDriving;
    if (!bVehicleAction) return;

    ATMOPVehicleBase* Vehicle = FindVehicle(Entry.TargetEntityId);
    if (!IsValid(Vehicle))
    {
        OutCode = TEXT("VehicleNotFound");
        OutDetails = FString::Printf(TEXT("Vehicle '%s' is not present."),
            *Entry.TargetEntityId.ToString());
        return;
    }

    if (Entry.Action == ETMOPPersonTimelineAction::ExitVehicle)
    {
        const UTMOPTrafficVehicleMovementComponent* Movement =
            Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
        if (IsValid(Movement) && !Movement->PlannedLaneIds.IsEmpty() &&
            Movement->TrafficState != ETMOPTrafficVehicleState::RouteComplete)
        {
            OutCode = TEXT("VehicleRouteIncomplete");
            OutDetails = FString::Printf(
                TEXT("Vehicle is %s on lane '%s' at %.0f cm/s with %d planned lanes."),
                *EnumText(Movement->TrafficState),
                *Movement->CurrentLaneId.ToString(),
                Movement->CurrentSpeedCmPerSecond,
                Movement->PlannedLaneIds.Num());
            return;
        }
        FName ActualSeatId;
        if (!FindVehicleSeatForAgent(Vehicle, Agent, ActualSeatId))
        {
            OutCode = TEXT("AgentNotRegisteredInVehicleSeat");
            OutDetails = FString::Printf(
                TEXT("Agent parent is '%s', but no seat in '%s' owns the agent."),
                *ActorDiagnosticName(IsValid(Agent)
                    ? Agent->GetAttachParentActor() : nullptr),
                *Entry.TargetEntityId.ToString());
            return;
        }
        OutCode = TEXT("VehicleExitRejected");
        OutDetails = FString::Printf(
            TEXT("Agent occupies seat '%s', but ExitSeat rejected the request."),
            *ActualSeatId.ToString());
        return;
    }

    if (Entry.Action == ETMOPPersonTimelineAction::EnterVehicle)
    {
        if (!Entry.TargetSeatId.IsNone())
        {
            UTMOPVehicleSeatComponent* RequestedSeat = nullptr;
            for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
                if (IsValid(Seat) && Seat->SeatId == Entry.TargetSeatId)
                {
                    RequestedSeat = Seat;
                    break;
                }
            if (!IsValid(RequestedSeat))
            {
                OutCode = TEXT("SeatNotFound");
                OutDetails = FString::Printf(TEXT("Seat '%s' does not exist in '%s'."),
                    *Entry.TargetSeatId.ToString(), *Entry.TargetEntityId.ToString());
                return;
            }
            if (RequestedSeat->IsOccupied())
            {
                OutCode = TEXT("SeatOccupied");
                OutDetails = FString::Printf(TEXT("Seat '%s' is occupied by '%s'."),
                    *Entry.TargetSeatId.ToString(),
                    *ActorDiagnosticName(RequestedSeat->GetOccupantCharacter()));
                return;
            }
        }
        OutCode = TEXT("VehicleEnterRejected");
        OutDetails = TEXT("Vehicle and requested seat exist, but EnterSeat rejected the request.");
        return;
    }

    if (!IsValid(Vehicle->GetDriverSeat()) ||
        Vehicle->GetDriverSeat()->GetOccupant() != Agent)
    {
        OutCode = TEXT("DriverSeatMismatch");
        OutDetails = FString::Printf(TEXT("Driver seat occupant is '%s'."),
            *ActorDiagnosticName(IsValid(Vehicle->GetDriverSeat())
                ? Vehicle->GetDriverSeat()->GetOccupantCharacter() : nullptr));
    }
    else if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute &&
        Entry.OrderedLaneIds.IsEmpty())
    {
        OutCode = TEXT("ManualRouteEmptyOrUnresolved");
        OutDetails = TEXT("No manual lane IDs were supplied and no usable mirrored vehicle route was found.");
    }
    else if (Entry.VehicleRouteMode != ETMOPVehicleRouteMode::ManualLaneRoute &&
        Entry.DrivingDestinationAnchorId.IsNone())
    {
        OutCode = TEXT("AutomaticDestinationMissing");
        OutDetails = TEXT("Automatic driving requires DrivingDestinationAnchorId.");
    }
    else
    {
        FString DirectorCode;
        FString DirectorDetails;
        if (VehicleDirector.IsValid() &&
            VehicleDirector->GetLastDrivingFailure(
                Entry.TargetEntityId, DirectorCode, DirectorDetails))
        {
            OutCode = DirectorCode;
            OutDetails = DirectorDetails;
        }
        else
        {
            OutCode = TEXT("VehicleRouteInitializationFailed");
            OutDetails = TEXT("Driver and route request are present; lane discovery, lane connectivity or InitializeOnLane failed.");
        }
    }
}

void ATMOPTimelineValidationDirector::HandleActionValidation(
    UTMOPActionExecutorComponent* Executor,
    const FTMOPScheduleEntry& Entry,
    const FTMOPTime ScheduledTime,
    const ETMOPActionExecutionState State)
{
    if (!bValidationActive || !IsValid(Executor)) return;
    const FName EntityId = GetEntityId(Executor);
    FTrackedAgent* Tracked = TrackedAgents.Find(EntityId);
    AActor* OwnerActor = Executor->GetOwner();

    FTMOPTimelineValidationRecord Record;
    Record.EntityId = EntityId;
    Record.EntryId = Entry.EntryId;
    Record.Action = EnumText(Entry.ActionType);
    Record.TimingMode = EnumText(Entry.TimingMode);
    Record.HistoricalSecond = Entry.AbsoluteTime.ToSecondsFromMidnight();
    Record.TargetAnchorId = Entry.TargetAnchorId;
    Record.TargetEntityId = Entry.TargetEntityId;
    Record.PlannedSecond = ScheduledTime.ToSecondsFromMidnight();
    Record.ActualSecond = GetSimulationSecond();
    Record.TimeDeviationSeconds =
        Record.PlannedSecond > 0
        ? Record.ActualSecond - Record.PlannedSecond : 0.0f;
    Record.ActualLocation = IsValid(OwnerActor)
        ? OwnerActor->GetActorLocation() : FVector::ZeroVector;
    if (Tracked != nullptr)
    {
        Record.PreviousEntryId = Tracked->LastTimelineEntryId;
        Record.PreviousAnchorId = Tracked->LastTimelineAnchorId;
    }
    PopulateRecordRuntimeDiagnostics(Record, Tracked);

    int32 TimedExpectedArrival = INDEX_NONE;
    float TimedRemainingPath = -1.0f;
    float TimedRequiredSpeed = -1.0f;
    bool bTimedPossible = true;
    const bool bHasTimedMove = Executor->GetActiveMoveTimingDiagnostics(
        TimedExpectedArrival, TimedRemainingPath,
        TimedRequiredSpeed, bTimedPossible);
    if (bHasTimedMove)
    {
        // This record represents an arrival deadline. The editor report
        // reader uses this flag to distinguish it from ordinary movement
        // rows whose scheduled time only means "start walking now".
        Record.bScheduledAsArrival = true;
        Record.ExpectedArrivalSecond = TimedExpectedArrival;
        Record.RemainingPathCm = TimedRemainingPath;
        Record.RequiredSpeedCmPerSecond = TimedRequiredSpeed;
        Record.bPhysicallyPossible = bTimedPossible;
        if (State == ETMOPActionExecutionState::Completed ||
            State == ETMOPActionExecutionState::Failed)
        {
            Record.PlannedSecond = TimedExpectedArrival;
            Record.TimeDeviationSeconds =
                Record.ActualSecond - TimedExpectedArrival;
        }
    }

    if (State == ETMOPActionExecutionState::Executing ||
        State == ETMOPActionExecutionState::WaitingForArrival)
    {
        if (Tracked != nullptr)
        {
            Tracked->ActiveEntryId = Entry.EntryId;
            Tracked->ActiveTargetAnchorId = Entry.TargetAnchorId;
            Tracked->ActivePlannedSecond = Record.PlannedSecond;
            Tracked->bRegistryManagedMove = false;
            Tracked->StationarySeconds = 0.0f;
            Tracked->bStuckReportedForCurrentMove = false;
        }
        if (State == ETMOPActionExecutionState::Executing)
        {
            Record.Event = TEXT("Started");
            const float AbsDeviation = FMath::Abs(Record.TimeDeviationSeconds);
            if (AbsDeviation > TimingErrorSeconds)
                Record.Severity = ETMOPTimelineValidationSeverity::Error;
            else if (AbsDeviation > TimingWarningSeconds)
                Record.Severity = ETMOPTimelineValidationSeverity::Warning;
            Record.Message = TEXT("Timeline action started.");
            AddRecord(Record);
        }
        else
        {
            int32 ExpectedArrival = INDEX_NONE;
            float RemainingPath = -1.0f;
            float RequiredSpeed = -1.0f;
            bool bPossible = true;
            if (Executor->GetActiveMoveTimingDiagnostics(
                ExpectedArrival, RemainingPath, RequiredSpeed, bPossible))
            {
                Record.Event = TEXT("TravelPreflight");
                Record.ExpectedArrivalSecond = ExpectedArrival;
                Record.RemainingPathCm = RemainingPath;
                Record.RequiredSpeedCmPerSecond = RequiredSpeed;
                Record.bPhysicallyPossible = bPossible;
                Record.Severity = bPossible
                    ? ETMOPTimelineValidationSeverity::Passed
                    : ETMOPTimelineValidationSeverity::Error;
                Record.Message = bPossible
                    ? FString::Printf(
                        TEXT("NavMesh route %.0f cm; required speed %.0f cm/s."),
                        RemainingPath, RequiredSpeed)
                    : FString::Printf(
                        TEXT("Impossible arrival without teleport: NavMesh route %.0f cm requires %.0f cm/s, above the realistic limit."),
                        RemainingPath, RequiredSpeed);
                AddRecord(Record);
            }
        }
        return;
    }

    if (State != ETMOPActionExecutionState::Completed &&
        State != ETMOPActionExecutionState::Failed) return;

    Record.Event = State == ETMOPActionExecutionState::Completed
        ? TEXT("Completed") : TEXT("Failed");
    if (!Entry.TargetAnchorId.IsNone() && IsValid(OwnerActor))
    {
        UGameInstance* GI = GetGameInstance();
        UTMOPAnchorSubsystem* Anchors = GI != nullptr
            ? GI->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
        const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Entry.TargetAnchorId) : nullptr;
        if (IsValid(Anchor))
            Record.DistanceToTargetCm = FVector::Dist2D(
                OwnerActor->GetActorLocation(), Anchor->GetAnchorLocation());
        else
        {
            Record.Severity = ETMOPTimelineValidationSeverity::Error;
            Record.Message = TEXT("Target anchor does not exist.");
        }
    }

    if (State == ETMOPActionExecutionState::Failed)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Error;
        Record.Message = TEXT("Action executor reported failure.");
    }
    else if (bHasTimedMove &&
        FMath::Abs(Record.TimeDeviationSeconds) > TimingErrorSeconds)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Error;
        Record.Message = FString::Printf(
            TEXT("Arrived %.0f seconds %s the historical deadline without teleporting."),
            FMath::Abs(Record.TimeDeviationSeconds),
            Record.TimeDeviationSeconds >= 0.0f ? TEXT("late") : TEXT("early"));
    }
    else if (bHasTimedMove &&
        FMath::Abs(Record.TimeDeviationSeconds) > TimingWarningSeconds)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Warning;
        Record.Message = FString::Printf(
            TEXT("Arrived %.0f seconds %s the historical deadline."),
            FMath::Abs(Record.TimeDeviationSeconds),
            Record.TimeDeviationSeconds >= 0.0f ? TEXT("late") : TEXT("early"));
    }
    else if (Record.DistanceToTargetCm > ArrivalWarningDistanceCm)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Warning;
        Record.Message = TEXT("Action completed outside the expected anchor radius.");
    }
    else if (Record.Message.IsEmpty())
        Record.Message = TEXT("Action completed successfully.");

    AddRecord(Record);
    if (Tracked != nullptr)
    {
        if (State == ETMOPActionExecutionState::Completed)
        {
            Tracked->LastTimelineEntryId = Entry.EntryId;
            if (!Entry.TargetAnchorId.IsNone())
                Tracked->LastTimelineAnchorId = Entry.TargetAnchorId;
        }
        Tracked->ActiveEntryId = NAME_None;
        Tracked->ActiveTargetAnchorId = NAME_None;
        Tracked->ActivePlannedSecond = INDEX_NONE;
        Tracked->bRegistryManagedMove = false;
        Tracked->bRegistryManagedTimedArrival = false;
        Tracked->StationarySeconds = 0.0f;
    }
}

void ATMOPTimelineValidationDirector::HandlePersonTimelineApplied(
    const FName EntityId,
    const FTMOPPersonTimelineEntry& Entry,
    const int32 ResolvedSecond,
    const bool bSuccessful,
    const bool bCatchUp)
{
    if (!bValidationActive || bCatchUp) return;

    FTrackedAgent* Tracked = TrackedAgents.Find(EntityId);
    if (Tracked == nullptr)
    {
        DiscoverAgents();
        Tracked = TrackedAgents.Find(EntityId);
    }

    if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor &&
        Tracked != nullptr && Tracked->Executor.IsValid() &&
        Tracked->ActiveEntryId == Entry.EntryId)
        return;

    FTMOPTimelineValidationRecord Record;
    Record.EntityId = EntityId;
    Record.EntryId = Entry.EntryId;
    Record.Action = EnumText(Entry.Action);
    Record.TimingMode = EnumText(Entry.TimingMode);
    Record.HistoricalSecond = Entry.Time.ToSecondsFromMidnight();
    Record.bScheduledAsArrival = Entry.bTimeIsArrival;
    Record.TargetAnchorId = Entry.TargetAnchorId;
    Record.TargetEntityId = Entry.TargetEntityId;
    Record.TargetSeatId = Entry.TargetSeatId;
    Record.PlannedSecond = ResolvedSecond;
    Record.ActualSecond = GetSimulationSecond();
    Record.TimeDeviationSeconds =
        Record.ActualSecond - Record.PlannedSecond;
    if (Tracked != nullptr && Tracked->Agent.IsValid())
    {
        Record.ActualLocation = Tracked->Agent->GetActorLocation();
        Record.PreviousEntryId = Tracked->LastTimelineEntryId;
        Record.PreviousAnchorId = Tracked->LastTimelineAnchorId;
    }
    PopulateRecordRuntimeDiagnostics(Record, Tracked);

    if (!bSuccessful)
    {
        DiagnosePersonFailure(
            Entry, Tracked, Record.FailureCode, Record.FailureDetails);
        bool bEmit = true;
        if (Tracked != nullptr)
        {
            const bool bSameFailure =
                Tracked->LastFailureEntryId == Entry.EntryId &&
                Tracked->LastFailureCode == Record.FailureCode;
            if (!bSameFailure)
            {
                Tracked->LastFailureEntryId = Entry.EntryId;
                Tracked->LastFailureCode = Record.FailureCode;
                Tracked->FirstFailureSecond = Record.ActualSecond;
                Tracked->LastFailureRecordSecond = INDEX_NONE;
                Tracked->FailureAttemptCount = 0;
            }
            ++Tracked->FailureAttemptCount;
            Record.RetryCount = Tracked->FailureAttemptCount;
            Record.RetryDurationSeconds =
                Record.ActualSecond - Tracked->FirstFailureSecond;
            bEmit = Tracked->LastFailureRecordSecond == INDEX_NONE ||
                Record.ActualSecond - Tracked->LastFailureRecordSecond >=
                    FMath::Max(1, FailureRepeatReportIntervalSeconds);
            if (bEmit)
                Tracked->LastFailureRecordSecond = Record.ActualSecond;
        }
        if (!bEmit) return;
        Record.Event = TEXT("Failed");
        Record.Severity = ETMOPTimelineValidationSeverity::Error;
        Record.Message = FString::Printf(TEXT("%s: %s"),
            *Record.FailureCode, *Record.FailureDetails);
        AddRecord(Record);
        if (Tracked != nullptr)
            CaptureAgentSnapshot(EntityId, *Tracked,
                Record.ActualSecond, TEXT("Failure"));
        return;
    }

    if (Tracked != nullptr && Tracked->LastFailureEntryId == Entry.EntryId &&
        Tracked->FailureAttemptCount > 0)
    {
        FTMOPTimelineValidationRecord Recovery = Record;
        Recovery.Event = TEXT("Recovered");
        Recovery.RetryCount = Tracked->FailureAttemptCount;
        Recovery.RetryDurationSeconds =
            Record.ActualSecond - Tracked->FirstFailureSecond;
        Recovery.FailureCode = Tracked->LastFailureCode;
        Recovery.Severity = ETMOPTimelineValidationSeverity::Warning;
        Recovery.Message = FString::Printf(
            TEXT("Succeeded after %d failed attempts over %.0f seconds."),
            Recovery.RetryCount, Recovery.RetryDurationSeconds);
        AddRecord(Recovery);
        Tracked->LastFailureEntryId = NAME_None;
        Tracked->LastFailureCode.Reset();
        Tracked->FirstFailureSecond = INDEX_NONE;
        Tracked->LastFailureRecordSecond = INDEX_NONE;
        Tracked->FailureAttemptCount = 0;
    }

    if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor)
    {
        Record.Event = TEXT("Started");
        Record.Message = TEXT("Group-managed movement started.");
        if (Tracked != nullptr)
        {
            Tracked->ActiveEntryId = Entry.EntryId;
            Tracked->ActiveTargetAnchorId = Entry.TargetAnchorId;
            Tracked->ActivePlannedSecond = ResolvedSecond;
            Tracked->bRegistryManagedMove = true;
            Tracked->bRegistryManagedTimedArrival = Entry.bTimeIsArrival;
            Tracked->StationarySeconds = 0.0f;
            Tracked->bStuckReportedForCurrentMove = false;
        }
    }
    else
    {
        Record.Event = TEXT("Applied");
        Record.Message = TEXT("People timeline entry applied successfully.");
    }
    if (Tracked != nullptr)
    {
        Tracked->LastTimelineEntryId = Entry.EntryId;
        if (!Entry.TargetAnchorId.IsNone())
            Tracked->LastTimelineAnchorId = Entry.TargetAnchorId;
    }
    AddRecord(Record);
}

void ATMOPTimelineValidationDirector::HandleVehicleTimelineArrived(
    const FName VehicleId,
    const FTMOPHistoricalVehicleTimelineEntry& Entry,
    const int32 PlannedSecond,
    const int32 ActualSecond,
    const bool bSuccessful)
{
    if (!bValidationActive) return;

    FTMOPTimelineValidationRecord Record;
    Record.EntityId = VehicleId;
    Record.EntryId = Entry.EntryId;
    Record.Event = bSuccessful
        ? TEXT("VehicleArrived") : TEXT("VehicleArrivalFailed");
    Record.Action = EnumText(Entry.Action);
    Record.TimingMode = EnumText(Entry.TimingMode);
    Record.HistoricalSecond = Entry.Time.ToSecondsFromMidnight();
    Record.bScheduledAsArrival = true;
    Record.TargetAnchorId = Entry.PlacementAnchorId;
    Record.PlannedSecond = PlannedSecond;
    Record.ActualSecond = ActualSecond;
    Record.TimeDeviationSeconds = ActualSecond - PlannedSecond;

    const ATMOPVehicleBase* Vehicle = FindVehicle(VehicleId);
    if (IsValid(Vehicle))
        Record.ActualLocation = Vehicle->GetActorLocation();
    if (!Entry.PlacementAnchorId.IsNone() && GetGameInstance() != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors =
            GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>();
        const ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
            ? Anchors->FindAnchor(Entry.PlacementAnchorId) : nullptr;
        if (IsValid(Anchor) && IsValid(Vehicle))
            Record.DistanceToTargetCm = FVector::Dist2D(
                Vehicle->GetActorLocation(), (Entry.AnchorLocalOffset * FTransform(
                    Anchor->GetAnchorRotation(), Anchor->GetAnchorLocation())).GetLocation());
    }

    for (TActorIterator<ATMOPHistoricalVehicleDirector> It(GetWorld()); It; ++It)
    {
        Record.TimelineFingerprint = It->GetTimelineFingerprint(VehicleId, Entry.EntryId);
        if (!Record.TimelineFingerprint.IsEmpty()) break;
    }
    if (const auto* Movement = IsValid(Vehicle)
        ? Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>() : nullptr)
    {
        Record.ArrivalCorrectionCm = float(Movement->LastArrivalCorrectionCm);
        Record.bArrivalBlocked = Movement->bLastArrivalBlocked;
        if (Record.bArrivalBlocked) Record.FailureDetails = Movement->LastArrivalBlocker;
    }
    const float AbsDeviation = FMath::Abs(Record.TimeDeviationSeconds);
    if (!bSuccessful)
    {
        Record.Severity = ETMOPTimelineValidationSeverity::Error;
        Record.FailureCode = TEXT("VehicleMissedArrivalAnchor");
        Record.Message = Record.DistanceToTargetCm >= 0.0f
            ? FString::Printf(
                TEXT("Vehicle missed the arrival anchor by %.0f cm."),
                Record.DistanceToTargetCm)
            : TEXT("Vehicle did not reach the scheduled arrival anchor.");
    }
    else
    {
        if (AbsDeviation > TimingErrorSeconds)
            Record.Severity = ETMOPTimelineValidationSeverity::Error;
        else if (AbsDeviation > TimingWarningSeconds)
            Record.Severity = ETMOPTimelineValidationSeverity::Warning;
        Record.Message = AbsDeviation < 0.5f
            ? TEXT("Vehicle arrived at the historical deadline.")
            : FString::Printf(
                TEXT("Vehicle arrived %.0f seconds %s the historical deadline."),
                AbsDeviation,
                Record.TimeDeviationSeconds >= 0.0f
                    ? TEXT("late") : TEXT("early"));
    }
    if (bSuccessful && Record.ArrivalCorrectionCm > 1.0f)
    {
        if (Record.Severity == ETMOPTimelineValidationSeverity::Passed)
            Record.Severity = ETMOPTimelineValidationSeverity::Warning;
        Record.Message = FString::Printf(TEXT("Deadline reached with a %.2f m position correction; not an unassisted arrival."),
            Record.ArrivalCorrectionCm / 100.0f);
    }
    if (Record.bArrivalBlocked)
        Record.Message += TEXT(" Blocked by: ") + Record.FailureDetails;
    AddRecord(Record);
}

void ATMOPTimelineValidationDirector::HandleGroupStateChanged(
    const FName GroupId,
    const ETMOPGroupState NewState)
{
    if (!bValidationActive || GroupId.IsNone() ||
        NewState != ETMOPGroupState::Arrived)
        return;

    const int32 ActualSecond = GetSimulationSecond();
    for (TPair<FName, FTrackedAgent>& Pair : TrackedAgents)
    {
        FTrackedAgent& Tracked = Pair.Value;
        ATMOPHistoricalAgent* Agent = Tracked.Agent.Get();
        if (!IsValid(Agent) || Agent->SocialGroupId != GroupId ||
            !Tracked.bRegistryManagedTimedArrival ||
            Tracked.ActivePlannedSecond == INDEX_NONE ||
            Tracked.ActiveEntryId.IsNone())
            continue;

        FTMOPTimelineValidationRecord Arrival;
        Arrival.EntityId = Pair.Key;
        Arrival.EntryId = Tracked.ActiveEntryId;
        Arrival.Event = TEXT("Completed");
        Arrival.Action = TEXT("Move To Anchor");
        Arrival.TargetAnchorId = Tracked.ActiveTargetAnchorId;
        Arrival.PlannedSecond = Tracked.ActivePlannedSecond;
        Arrival.ActualSecond = ActualSecond;
        Arrival.TimeDeviationSeconds =
            Arrival.ActualSecond - Arrival.PlannedSecond;
        Arrival.bScheduledAsArrival = true;
        Arrival.ActualLocation = Agent->GetActorLocation();
        PopulateRecordRuntimeDiagnostics(Arrival, &Tracked);
        const float AbsDeviation =
            FMath::Abs(Arrival.TimeDeviationSeconds);
        if (AbsDeviation > TimingErrorSeconds)
            Arrival.Severity = ETMOPTimelineValidationSeverity::Error;
        else if (AbsDeviation > TimingWarningSeconds)
            Arrival.Severity = ETMOPTimelineValidationSeverity::Warning;
        Arrival.Message = AbsDeviation < 0.5f
            ? TEXT("Group arrived at the historical deadline.")
            : FString::Printf(
                TEXT("Group arrived %.0f seconds %s the historical deadline."),
                AbsDeviation,
                Arrival.TimeDeviationSeconds >= 0.0f
                    ? TEXT("late") : TEXT("early"));
        AddRecord(Arrival);

        // The state-change delegate is synchronous, so this is the exact
        // simulation second. Clearing here prevents the slower diagnostic
        // sampler from recording a duplicate one second later.
        Tracked.ActiveEntryId = NAME_None;
        Tracked.ActiveTargetAnchorId = NAME_None;
        Tracked.ActivePlannedSecond = INDEX_NONE;
        Tracked.bRegistryManagedMove = false;
        Tracked.bRegistryManagedTimedArrival = false;
        Tracked.StationarySeconds = 0.0f;
        Tracked.bStuckReportedForCurrentMove = false;
        Tracked.LastLocation = Agent->GetActorLocation();
    }
}

void ATMOPTimelineValidationDirector::ValidateSharedEventReferences()
{
    if (!PeopleDirector.IsValid() ||
        !IsValid(PeopleDirector->PersonProfileTable) ||
        GetGameInstance() == nullptr)
        return;

    const UTMOPHistoricalEventSubsystem* Events =
        GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>();
    if (Events == nullptr) return;

    for (const TPair<FName, uint8*>& Pair :
        PeopleDirector->PersonProfileTable->GetRowMap())
    {
        const FTMOPPersonProfileRow* Profile =
            reinterpret_cast<const FTMOPPersonProfileRow*>(Pair.Value);
        if (Profile == nullptr) continue;
        for (const FTMOPPersonTimelineEntry& Entry : Profile->Timeline)
        {
            if (Entry.TimingMode != ETMOPEventTimingMode::Relative ||
                Entry.SharedEventId.IsNone() ||
                Events->HasEventDefinition(Entry.SharedEventId))
                continue;

            FTMOPTimelineValidationRecord Record;
            Record.EntityId = Profile->EntityId;
            Record.EntryId = Entry.EntryId;
            Record.Event = TEXT("MissingSharedEventDefinition");
            Record.Action = EnumText(Entry.Action);
            Record.TimingMode = EnumText(Entry.TimingMode);
            Record.HistoricalSecond = Entry.Time.ToSecondsFromMidnight();
            Record.TargetAnchorId = Entry.TargetAnchorId;
            Record.ActualSecond = GetSimulationSecond();
            Record.FailureCode = TEXT("MissingSharedEventDefinition");
            Record.FailureDetails = FString::Printf(
                TEXT("Shared event '%s' is referenced but not registered."),
                *Entry.SharedEventId.ToString());
            Record.Severity = ETMOPTimelineValidationSeverity::Error;
            Record.Message = Record.FailureDetails;
            AddRecord(Record);
        }
    }
}

void ATMOPTimelineValidationDirector::AddRecord(
    const FTMOPTimelineValidationRecord& Record)
{
    Records.Add(Record);
    if (Record.Severity != ETMOPTimelineValidationSeverity::Passed)
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP validation %s %s/%s: %s"),
            *SeverityText(Record.Severity),
            *Record.EntityId.ToString(), *Record.EntryId.ToString(),
            *Record.Message);
}

int32 ATMOPTimelineValidationDirector::GetSimulationSecond() const
{
    const UGameInstance* GI = GetGameInstance();
    const UTMOPClockSubsystem* Clock = GI != nullptr
        ? GI->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    return Clock != nullptr
        ? Clock->GetCurrentTime().ToSecondsFromMidnight() : INDEX_NONE;
}

FName ATMOPTimelineValidationDirector::GetEntityId(
    const UTMOPActionExecutorComponent* Executor) const
{
    const ATMOPHistoricalAgent* Agent = IsValid(Executor)
        ? Cast<ATMOPHistoricalAgent>(Executor->GetOwner()) : nullptr;
    return IsValid(Agent) && IsValid(Agent->EntityIdentity)
        ? Agent->EntityIdentity->GetEntityId() : NAME_None;
}

bool ATMOPTimelineValidationDirector::ExportReports()
{
    const FString Directory = FPaths::Combine(
        FPaths::ProjectSavedDir(), TEXT("TMOP"), TEXT("Validation"));
    IFileManager::Get().MakeDirectory(*Directory, true);
    const FString Stamp = FDateTime::Now().ToString(TEXT("%Y%m%d_%H%M%S"));
    const FString Base = FPaths::Combine(
        Directory, FString::Printf(TEXT("TimelineValidation_%s"), *Stamp));

    const int32 ExportSecond = MaximumObservedSecond != INDEX_NONE
        ? MaximumObservedSecond : GetSimulationSecond();
    auto AppendCsv = [](FString& Out, const TArray<FString>& Fields)
    {
        TArray<FString> Escaped;
        Escaped.Reserve(Fields.Num());
        for (const FString& Field : Fields) Escaped.Add(CsvEscape(Field));
        Out += FString::Join(Escaped, TEXT(",")) + TEXT("\n");
    };
    auto N = [](const double Value) { return FString::SanitizeFloat(Value); };
    auto I = [](const int32 Value) { return FString::FromInt(Value); };
    auto B = [](const bool Value) { return Value ? TEXT("true") : TEXT("false"); };

    FString Csv;
    AppendCsv(Csv, {
        TEXT("EntityId"), TEXT("EntryId"), TEXT("Event"), TEXT("Action"),
        TEXT("TimingMode"), TEXT("HistoricalSecond"), TEXT("ScheduledAsArrival"),
        TEXT("PreviousEntryId"), TEXT("PreviousAnchorId"), TEXT("TargetAnchorId"),
        TEXT("TargetEntityId"), TEXT("TargetSeatId"), TEXT("PlannedSecond"),
        TEXT("ActualSecond"), TEXT("TimeDeviationSeconds"), TEXT("DistanceToTargetCm"),
        TEXT("ExpectedShotAnchorId"), TEXT("DistanceToExpectedShotAnchorCm"),
        TEXT("AtExpectedShotAnchor"),
        TEXT("ExpectedArrivalSecond"), TEXT("RemainingPathCm"),
        TEXT("RequiredSpeedCmPerSecond"), TEXT("PhysicallyPossible"),
        TEXT("X"), TEXT("Y"), TEXT("Z"), TEXT("ActualVehicleId"),
        TEXT("ActualSeatId"), TEXT("AttachedParentName"), TEXT("VehicleTrafficState"),
        TEXT("VehicleLaneId"), TEXT("VehicleSpeedCmPerSecond"),
        TEXT("VehicleRouteRemaining"), TEXT("ActorCollisionEnabled"),
        TEXT("VehicleObstacleDetectionEnabled"), TEXT("BlockingActorName"),
        TEXT("BlockingActorClass"), TEXT("BlockingActorDistanceCm"),
        TEXT("GroupId"), TEXT("GroupLeaderId"), TEXT("GroupState"),
        TEXT("GroupFormation"), TEXT("DistanceToGroupLeaderCm"),
        TEXT("FailureCode"), TEXT("FailureDetails"), TEXT("RetryCount"),
        TEXT("RetryDurationSeconds"), TEXT("Severity"), TEXT("Message")
    });
    TArray<TSharedPtr<FJsonValue>> JsonRecords;
    for (const FTMOPTimelineValidationRecord& R : Records)
    {
        ETMOPTimelineValidationSeverity ExportSeverity = R.Severity;
        FString ExportMessage = R.Message;
        if (R.Event == TEXT("Failed") && ExportSecond != INDEX_NONE &&
            R.ActualSecond >= ExportSecond - 1 &&
            R.Message == TEXT("Action executor reported failure."))
        {
            ExportSeverity = ETMOPTimelineValidationSeverity::Warning;
            ExportMessage = TEXT("Action was interrupted when validation/simulation ended.");
        }
        AppendCsv(Csv, {
            R.EntityId.ToString(), R.EntryId.ToString(), R.Event, R.Action,
            R.TimingMode, I(R.HistoricalSecond), B(R.bScheduledAsArrival),
            R.PreviousEntryId.ToString(), R.PreviousAnchorId.ToString(),
            R.TargetAnchorId.ToString(), R.TargetEntityId.ToString(),
            R.TargetSeatId.ToString(), I(R.PlannedSecond), I(R.ActualSecond),
            N(R.TimeDeviationSeconds), N(R.DistanceToTargetCm),
            R.ExpectedShotAnchorId.ToString(),
            N(R.DistanceToExpectedShotAnchorCm), B(R.bAtExpectedShotAnchor),
            I(R.ExpectedArrivalSecond), N(R.RemainingPathCm),
            N(R.RequiredSpeedCmPerSecond), B(R.bPhysicallyPossible),
            N(R.ActualLocation.X), N(R.ActualLocation.Y), N(R.ActualLocation.Z),
            R.ActualVehicleId.ToString(), R.ActualSeatId.ToString(),
            R.AttachedParentName, R.VehicleTrafficState,
            R.VehicleLaneId.ToString(), N(R.VehicleSpeedCmPerSecond),
            I(R.VehicleRouteRemaining), B(R.bActorCollisionEnabled),
            B(R.bVehicleObstacleDetectionEnabled), R.BlockingActorName,
            R.BlockingActorClass, N(R.BlockingActorDistanceCm),
            R.GroupId.ToString(), R.GroupLeaderId.ToString(), R.GroupState,
            R.GroupFormation, N(R.DistanceToGroupLeaderCm), R.FailureCode,
            R.FailureDetails, I(R.RetryCount), N(R.RetryDurationSeconds),
            SeverityText(ExportSeverity), ExportMessage
        });

        TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("entityId"), R.EntityId.ToString());
        O->SetStringField(TEXT("entryId"), R.EntryId.ToString());
        O->SetStringField(TEXT("event"), R.Event);
        O->SetStringField(TEXT("timelineFingerprint"), R.TimelineFingerprint);
        O->SetNumberField(TEXT("arrivalCorrectionCm"), R.ArrivalCorrectionCm);
        O->SetBoolField(TEXT("arrivalBlocked"), R.bArrivalBlocked);
        O->SetStringField(TEXT("targetAnchorId"), R.TargetAnchorId.ToString());
        O->SetNumberField(TEXT("plannedSecond"), R.PlannedSecond);
        O->SetNumberField(TEXT("actualSecond"), R.ActualSecond);
        O->SetNumberField(TEXT("timeDeviationSeconds"), R.TimeDeviationSeconds);
        O->SetNumberField(TEXT("distanceToTargetCm"), R.DistanceToTargetCm);
        O->SetStringField(TEXT("expectedShotAnchorId"),
            R.ExpectedShotAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToExpectedShotAnchorCm"),
            R.DistanceToExpectedShotAnchorCm);
        O->SetBoolField(TEXT("atExpectedShotAnchor"),
            R.bAtExpectedShotAnchor);
        O->SetNumberField(TEXT("expectedArrivalSecond"), R.ExpectedArrivalSecond);
        O->SetNumberField(TEXT("remainingPathCm"), R.RemainingPathCm);
        O->SetNumberField(TEXT("requiredSpeedCmPerSecond"), R.RequiredSpeedCmPerSecond);
        O->SetBoolField(TEXT("physicallyPossible"), R.bPhysicallyPossible);
        O->SetObjectField(TEXT("actualLocation"), VectorJson(R.ActualLocation));
        O->SetStringField(TEXT("action"), R.Action);
        O->SetStringField(TEXT("timingMode"), R.TimingMode);
        O->SetNumberField(TEXT("historicalSecond"), R.HistoricalSecond);
        O->SetBoolField(TEXT("scheduledAsArrival"), R.bScheduledAsArrival);
        O->SetStringField(TEXT("previousEntryId"), R.PreviousEntryId.ToString());
        O->SetStringField(TEXT("previousAnchorId"), R.PreviousAnchorId.ToString());
        O->SetStringField(TEXT("targetEntityId"), R.TargetEntityId.ToString());
        O->SetStringField(TEXT("targetSeatId"), R.TargetSeatId.ToString());
        O->SetStringField(TEXT("failureCode"), R.FailureCode);
        O->SetStringField(TEXT("failureDetails"), R.FailureDetails);
        O->SetNumberField(TEXT("retryCount"), R.RetryCount);
        O->SetNumberField(TEXT("retryDurationSeconds"), R.RetryDurationSeconds);
        O->SetStringField(TEXT("actualVehicleId"), R.ActualVehicleId.ToString());
        O->SetStringField(TEXT("actualSeatId"), R.ActualSeatId.ToString());
        O->SetStringField(TEXT("attachedParentName"), R.AttachedParentName);
        O->SetStringField(TEXT("vehicleTrafficState"), R.VehicleTrafficState);
        O->SetStringField(TEXT("vehicleLaneId"), R.VehicleLaneId.ToString());
        O->SetNumberField(TEXT("vehicleSpeedCmPerSecond"), R.VehicleSpeedCmPerSecond);
        O->SetNumberField(TEXT("vehicleRouteRemaining"), R.VehicleRouteRemaining);
        O->SetBoolField(TEXT("actorCollisionEnabled"), R.bActorCollisionEnabled);
        O->SetBoolField(TEXT("vehicleObstacleDetectionEnabled"),
            R.bVehicleObstacleDetectionEnabled);
        O->SetStringField(TEXT("blockingActorName"), R.BlockingActorName);
        O->SetStringField(TEXT("blockingActorClass"), R.BlockingActorClass);
        O->SetNumberField(TEXT("blockingActorDistanceCm"), R.BlockingActorDistanceCm);
        O->SetStringField(TEXT("groupId"), R.GroupId.ToString());
        O->SetStringField(TEXT("groupLeaderId"), R.GroupLeaderId.ToString());
        O->SetStringField(TEXT("groupState"), R.GroupState);
        O->SetStringField(TEXT("groupFormation"), R.GroupFormation);
        O->SetNumberField(TEXT("distanceToGroupLeaderCm"),
            R.DistanceToGroupLeaderCm);
        O->SetStringField(TEXT("severity"), SeverityText(ExportSeverity));
        O->SetStringField(TEXT("message"), ExportMessage);
        JsonRecords.Add(MakeShared<FJsonValueObject>(O));
    }

    FString AgentCsv;
    AppendCsv(AgentCsv, {
        TEXT("EntityId"), TEXT("SampleSecond"), TEXT("Reason"), TEXT("X"),
        TEXT("Y"), TEXT("Z"), TEXT("VelocityX"), TEXT("VelocityY"),
        TEXT("VelocityZ"), TEXT("ActivityState"), TEXT("LifeState"),
        TEXT("ActiveEntryId"), TEXT("TargetAnchorId"), TEXT("DistanceToTargetCm"),
        TEXT("ExpectedShotAnchorId"), TEXT("DistanceToExpectedShotAnchorCm"),
        TEXT("AtExpectedShotAnchor"),
        TEXT("NearestShotAnchorId"), TEXT("DistanceToNearestShotAnchorCm"),
        TEXT("StationarySeconds"), TEXT("ControllerName"),
        TEXT("PathFollowingStatus"), TEXT("MovementMode"),
        TEXT("NavigationGoalX"), TEXT("NavigationGoalY"),
        TEXT("NavigationGoalZ"), TEXT("DistanceToNavigationGoalCm"),
        TEXT("ProjectedToNavMesh"), TEXT("OnNavMesh"),
        TEXT("DistanceToNavMeshCm"), TEXT("CollisionProfileName"),
        TEXT("VehicleId"), TEXT("SeatId"),
        TEXT("AttachedParentName"), TEXT("GroupId"), TEXT("GroupLeaderId"),
        TEXT("GroupState"), TEXT("GroupFormation"),
        TEXT("DistanceToGroupLeaderCm"), TEXT("CollisionEnabled")
    });
    TArray<TSharedPtr<FJsonValue>> JsonAgentSnapshots;
    for (const FTMOPAgentValidationSnapshot& S : AgentSnapshots)
    {
        AppendCsv(AgentCsv, {
            S.EntityId.ToString(), I(S.SampleSecond), S.Reason,
            N(S.Location.X), N(S.Location.Y), N(S.Location.Z),
            N(S.Velocity.X), N(S.Velocity.Y), N(S.Velocity.Z),
            S.ActivityState, S.LifeState, S.ActiveEntryId.ToString(),
            S.TargetAnchorId.ToString(), N(S.DistanceToTargetCm),
            S.ExpectedShotAnchorId.ToString(),
            N(S.DistanceToExpectedShotAnchorCm), B(S.bAtExpectedShotAnchor),
            S.NearestShotAnchorId.ToString(),
            N(S.DistanceToNearestShotAnchorCm), N(S.StationarySeconds),
            S.ControllerName, S.PathFollowingStatus, S.MovementMode,
            N(S.NavigationGoal.X), N(S.NavigationGoal.Y), N(S.NavigationGoal.Z),
            N(S.DistanceToNavigationGoalCm), B(S.bProjectedToNavMesh),
            B(S.bOnNavMesh), N(S.DistanceToNavMeshCm),
            S.CollisionProfileName, S.VehicleId.ToString(), S.SeatId.ToString(),
            S.AttachedParentName, S.GroupId.ToString(), S.GroupLeaderId.ToString(),
            S.GroupState, S.GroupFormation, N(S.DistanceToGroupLeaderCm),
            B(S.bCollisionEnabled)
        });
        TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("entityId"), S.EntityId.ToString());
        O->SetNumberField(TEXT("sampleSecond"), S.SampleSecond);
        O->SetStringField(TEXT("reason"), S.Reason);
        O->SetObjectField(TEXT("location"), VectorJson(S.Location));
        O->SetObjectField(TEXT("velocity"), VectorJson(S.Velocity));
        O->SetStringField(TEXT("activityState"), S.ActivityState);
        O->SetStringField(TEXT("lifeState"), S.LifeState);
        O->SetStringField(TEXT("activeEntryId"), S.ActiveEntryId.ToString());
        O->SetStringField(TEXT("targetAnchorId"), S.TargetAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToTargetCm"), S.DistanceToTargetCm);
        O->SetStringField(TEXT("expectedShotAnchorId"),
            S.ExpectedShotAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToExpectedShotAnchorCm"),
            S.DistanceToExpectedShotAnchorCm);
        O->SetBoolField(TEXT("atExpectedShotAnchor"),
            S.bAtExpectedShotAnchor);
        O->SetStringField(TEXT("nearestShotAnchorId"),
            S.NearestShotAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToNearestShotAnchorCm"),
            S.DistanceToNearestShotAnchorCm);
        O->SetNumberField(TEXT("stationarySeconds"), S.StationarySeconds);
        O->SetStringField(TEXT("controllerName"), S.ControllerName);
        O->SetStringField(TEXT("pathFollowingStatus"), S.PathFollowingStatus);
        O->SetStringField(TEXT("movementMode"), S.MovementMode);
        O->SetObjectField(TEXT("navigationGoal"), VectorJson(S.NavigationGoal));
        O->SetNumberField(TEXT("distanceToNavigationGoalCm"),
            S.DistanceToNavigationGoalCm);
        O->SetBoolField(TEXT("projectedToNavMesh"), S.bProjectedToNavMesh);
        O->SetBoolField(TEXT("onNavMesh"), S.bOnNavMesh);
        O->SetNumberField(TEXT("distanceToNavMeshCm"), S.DistanceToNavMeshCm);
        O->SetStringField(TEXT("collisionProfileName"), S.CollisionProfileName);
        O->SetStringField(TEXT("vehicleId"), S.VehicleId.ToString());
        O->SetStringField(TEXT("seatId"), S.SeatId.ToString());
        O->SetStringField(TEXT("attachedParentName"), S.AttachedParentName);
        O->SetStringField(TEXT("groupId"), S.GroupId.ToString());
        O->SetStringField(TEXT("groupLeaderId"), S.GroupLeaderId.ToString());
        O->SetStringField(TEXT("groupState"), S.GroupState);
        O->SetStringField(TEXT("groupFormation"), S.GroupFormation);
        O->SetNumberField(TEXT("distanceToGroupLeaderCm"),
            S.DistanceToGroupLeaderCm);
        O->SetBoolField(TEXT("collisionEnabled"), S.bCollisionEnabled);
        JsonAgentSnapshots.Add(MakeShared<FJsonValueObject>(O));
    }

    FString VehicleCsv;
    AppendCsv(VehicleCsv, {
        TEXT("VehicleId"), TEXT("SampleSecond"), TEXT("Reason"), TEXT("X"),
        TEXT("Y"), TEXT("Z"), TEXT("YawDegrees"), TEXT("SpeedCmPerSecond"),
        TEXT("TrafficState"), TEXT("CurrentLaneId"), TEXT("DistanceAlongLaneCm"),
        TEXT("NextLaneId"), TEXT("RemainingLaneCount"), TEXT("StationarySeconds"),
        TEXT("CollisionEnabled"), TEXT("ObstacleDetectionEnabled"),
        TEXT("HasStopConstraint"), TEXT("RemainingStopConstraintCm"),
        TEXT("PlannedStopAnchorId"), TEXT("PlannedStopSecond"),
        TEXT("DistanceToPlannedStopCm"), TEXT("ExpectedShotAnchorId"),
        TEXT("DistanceToExpectedShotAnchorCm"), TEXT("AtExpectedShotAnchor"),
        TEXT("ProfileTimelineEntryCount"), TEXT("ProfileStopEntryCount"),
        TEXT("BlockingActorName"),
        TEXT("BlockingActorClass"), TEXT("BlockingActorDistanceCm"),
        TEXT("OccupiedSeats"), TEXT("NearbyVehicleIds")
    });
    TArray<TSharedPtr<FJsonValue>> JsonVehicleSnapshots;
    for (const FTMOPVehicleValidationSnapshot& S : VehicleSnapshots)
    {
        AppendCsv(VehicleCsv, {
            S.VehicleId.ToString(), I(S.SampleSecond), S.Reason,
            N(S.Location.X), N(S.Location.Y), N(S.Location.Z), N(S.YawDegrees),
            N(S.SpeedCmPerSecond), S.TrafficState, S.CurrentLaneId.ToString(),
            N(S.DistanceAlongLaneCm), S.NextLaneId.ToString(),
            I(S.RemainingLaneCount), N(S.StationarySeconds),
            B(S.bCollisionEnabled), B(S.bObstacleDetectionEnabled),
            B(S.bHasStopConstraint), N(S.RemainingStopConstraintCm),
            S.PlannedStopAnchorId.ToString(), I(S.PlannedStopSecond),
            N(S.DistanceToPlannedStopCm), S.ExpectedShotAnchorId.ToString(),
            N(S.DistanceToExpectedShotAnchorCm), B(S.bAtExpectedShotAnchor),
            I(S.ProfileTimelineEntryCount), I(S.ProfileStopEntryCount),
            S.BlockingActorName,
            S.BlockingActorClass, N(S.BlockingActorDistanceCm),
            S.OccupiedSeats, S.NearbyVehicleIds
        });
        TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("vehicleId"), S.VehicleId.ToString());
        O->SetNumberField(TEXT("sampleSecond"), S.SampleSecond);
        O->SetStringField(TEXT("reason"), S.Reason);
        O->SetObjectField(TEXT("location"), VectorJson(S.Location));
        O->SetNumberField(TEXT("yawDegrees"), S.YawDegrees);
        O->SetNumberField(TEXT("speedCmPerSecond"), S.SpeedCmPerSecond);
        O->SetStringField(TEXT("trafficState"), S.TrafficState);
        O->SetStringField(TEXT("currentLaneId"), S.CurrentLaneId.ToString());
        O->SetNumberField(TEXT("distanceAlongLaneCm"), S.DistanceAlongLaneCm);
        O->SetStringField(TEXT("nextLaneId"), S.NextLaneId.ToString());
        O->SetNumberField(TEXT("remainingLaneCount"), S.RemainingLaneCount);
        O->SetNumberField(TEXT("stationarySeconds"), S.StationarySeconds);
        O->SetBoolField(TEXT("collisionEnabled"), S.bCollisionEnabled);
        O->SetBoolField(TEXT("obstacleDetectionEnabled"),
            S.bObstacleDetectionEnabled);
        O->SetBoolField(TEXT("hasStopConstraint"), S.bHasStopConstraint);
        O->SetNumberField(TEXT("remainingStopConstraintCm"),
            S.RemainingStopConstraintCm);
        O->SetStringField(TEXT("plannedStopAnchorId"),
            S.PlannedStopAnchorId.ToString());
        O->SetNumberField(TEXT("plannedStopSecond"), S.PlannedStopSecond);
        O->SetNumberField(TEXT("distanceToPlannedStopCm"),
            S.DistanceToPlannedStopCm);
        O->SetStringField(TEXT("expectedShotAnchorId"),
            S.ExpectedShotAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToExpectedShotAnchorCm"),
            S.DistanceToExpectedShotAnchorCm);
        O->SetBoolField(TEXT("atExpectedShotAnchor"),
            S.bAtExpectedShotAnchor);
        O->SetNumberField(TEXT("profileTimelineEntryCount"),
            S.ProfileTimelineEntryCount);
        O->SetNumberField(TEXT("profileStopEntryCount"),
            S.ProfileStopEntryCount);
        O->SetStringField(TEXT("blockingActorName"), S.BlockingActorName);
        O->SetStringField(TEXT("blockingActorClass"), S.BlockingActorClass);
        O->SetNumberField(TEXT("blockingActorDistanceCm"),
            S.BlockingActorDistanceCm);
        O->SetStringField(TEXT("occupiedSeats"), S.OccupiedSeats);
        O->SetStringField(TEXT("nearbyVehicleIds"), S.NearbyVehicleIds);
        JsonVehicleSnapshots.Add(MakeShared<FJsonValueObject>(O));
    }

    struct FEntitySummary
    {
        int32 FirstSecond = INDEX_NONE;
        int32 LastSecond = INDEX_NONE;
        int32 EventCount = 0;
        int32 FailureCount = 0;
        int32 RecoveryCount = 0;
        int32 StuckCount = 0;
        float MaximumAbsDeviation = 0.0f;
        float MaximumTargetDistance = 0.0f;
        bool bHasShotLocation = false;
        FVector ShotLocation = FVector::ZeroVector;
        FName ShotVehicleId = NAME_None;
        FName ShotSeatId = NAME_None;
        FName ExpectedShotAnchorId = NAME_None;
        float DistanceToExpectedShotAnchorCm = -1.0f;
        bool bAtExpectedShotAnchor = false;
        FString LastFailureCode;
    };
    TMap<FName, FEntitySummary> EntitySummaries;
    for (const FTMOPTimelineValidationRecord& R : Records)
    {
        FEntitySummary& S = EntitySummaries.FindOrAdd(R.EntityId);
        S.FirstSecond = S.FirstSecond == INDEX_NONE
            ? R.ActualSecond : FMath::Min(S.FirstSecond, R.ActualSecond);
        S.LastSecond = FMath::Max(S.LastSecond, R.ActualSecond);
        ++S.EventCount;
        if (R.Event == TEXT("Failed"))
        {
            ++S.FailureCount;
            S.LastFailureCode = R.FailureCode;
        }
        if (R.Event == TEXT("Recovered")) ++S.RecoveryCount;
        if (R.Event == TEXT("Stuck")) ++S.StuckCount;
        S.MaximumAbsDeviation = FMath::Max(
            S.MaximumAbsDeviation, FMath::Abs(R.TimeDeviationSeconds));
        if (R.DistanceToTargetCm >= 0.0f)
            S.MaximumTargetDistance = FMath::Max(
                S.MaximumTargetDistance, R.DistanceToTargetCm);
    }
    for (const FTMOPAgentValidationSnapshot& Snapshot : AgentSnapshots)
        if (Snapshot.Reason == TEXT("ShotMoment"))
        {
            FEntitySummary& S = EntitySummaries.FindOrAdd(Snapshot.EntityId);
            S.bHasShotLocation = true;
            S.ShotLocation = Snapshot.Location;
            S.ShotVehicleId = Snapshot.VehicleId;
            S.ShotSeatId = Snapshot.SeatId;
            S.ExpectedShotAnchorId = Snapshot.ExpectedShotAnchorId;
            S.DistanceToExpectedShotAnchorCm =
                Snapshot.DistanceToExpectedShotAnchorCm;
            S.bAtExpectedShotAnchor = Snapshot.bAtExpectedShotAnchor;
        }

    FString SummaryCsv;
    AppendCsv(SummaryCsv, {
        TEXT("EntityId"), TEXT("FirstSecond"), TEXT("LastSecond"),
        TEXT("EventCount"), TEXT("FailureRecordCount"), TEXT("RecoveryCount"),
        TEXT("StuckCount"), TEXT("MaximumAbsDeviationSeconds"),
        TEXT("MaximumTargetDistanceCm"), TEXT("HasShotLocation"),
        TEXT("ShotX"), TEXT("ShotY"), TEXT("ShotZ"), TEXT("ShotVehicleId"),
        TEXT("ShotSeatId"), TEXT("ExpectedShotAnchorId"),
        TEXT("DistanceToExpectedShotAnchorCm"), TEXT("AtExpectedShotAnchor"),
        TEXT("LastFailureCode")
    });
    TArray<TSharedPtr<FJsonValue>> JsonEntitySummaries;
    TArray<FName> SummaryIds;
    EntitySummaries.GetKeys(SummaryIds);
    SummaryIds.Sort([](const FName& A, const FName& B)
    {
        return A.LexicalLess(B);
    });
    for (const FName EntityId : SummaryIds)
    {
        const FEntitySummary& S = EntitySummaries.FindChecked(EntityId);
        AppendCsv(SummaryCsv, {
            EntityId.ToString(), I(S.FirstSecond), I(S.LastSecond),
            I(S.EventCount), I(S.FailureCount), I(S.RecoveryCount),
            I(S.StuckCount), N(S.MaximumAbsDeviation),
            N(S.MaximumTargetDistance), B(S.bHasShotLocation),
            N(S.ShotLocation.X), N(S.ShotLocation.Y), N(S.ShotLocation.Z),
            S.ShotVehicleId.ToString(), S.ShotSeatId.ToString(),
            S.ExpectedShotAnchorId.ToString(),
            N(S.DistanceToExpectedShotAnchorCm), B(S.bAtExpectedShotAnchor),
            S.LastFailureCode
        });
        TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("entityId"), EntityId.ToString());
        O->SetNumberField(TEXT("firstSecond"), S.FirstSecond);
        O->SetNumberField(TEXT("lastSecond"), S.LastSecond);
        O->SetNumberField(TEXT("eventCount"), S.EventCount);
        O->SetNumberField(TEXT("failureRecordCount"), S.FailureCount);
        O->SetNumberField(TEXT("recoveryCount"), S.RecoveryCount);
        O->SetNumberField(TEXT("stuckCount"), S.StuckCount);
        O->SetNumberField(TEXT("maximumAbsDeviationSeconds"),
            S.MaximumAbsDeviation);
        O->SetNumberField(TEXT("maximumTargetDistanceCm"),
            S.MaximumTargetDistance);
        O->SetBoolField(TEXT("hasShotLocation"), S.bHasShotLocation);
        O->SetObjectField(TEXT("shotLocation"), VectorJson(S.ShotLocation));
        O->SetStringField(TEXT("shotVehicleId"), S.ShotVehicleId.ToString());
        O->SetStringField(TEXT("shotSeatId"), S.ShotSeatId.ToString());
        O->SetStringField(TEXT("expectedShotAnchorId"),
            S.ExpectedShotAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToExpectedShotAnchorCm"),
            S.DistanceToExpectedShotAnchorCm);
        O->SetBoolField(TEXT("atExpectedShotAnchor"),
            S.bAtExpectedShotAnchor);
        O->SetStringField(TEXT("lastFailureCode"), S.LastFailureCode);
        JsonEntitySummaries.Add(MakeShared<FJsonValueObject>(O));
    }

    struct FVehicleSummary
    {
        int32 FirstSecond = INDEX_NONE;
        int32 LastSecond = INDEX_NONE;
        int32 SnapshotCount = 0;
        int32 BlockedSamples = 0;
        int32 CollisionDisabledSamples = 0;
        int32 NearbyVehicleSamples = 0;
        int32 RouteCompleteSamples = 0;
        float MaximumStationarySeconds = 0.0f;
        float MinimumDistanceToPlannedStopCm = TNumericLimits<float>::Max();
        bool bHasShotLocation = false;
        FVector ShotLocation = FVector::ZeroVector;
        FName ExpectedShotAnchorId = NAME_None;
        float DistanceToExpectedShotAnchorCm = -1.0f;
        bool bAtExpectedShotAnchor = false;
        int32 ProfileTimelineEntryCount = INDEX_NONE;
        int32 ProfileStopEntryCount = INDEX_NONE;
        FString LastTrafficState;
        FName LastLaneId = NAME_None;
        FString LastBlockingActor;
    };
    TMap<FName, FVehicleSummary> VehicleSummaries;
    for (const FTMOPVehicleValidationSnapshot& Snapshot : VehicleSnapshots)
    {
        FVehicleSummary& S = VehicleSummaries.FindOrAdd(Snapshot.VehicleId);
        S.FirstSecond = S.FirstSecond == INDEX_NONE
            ? Snapshot.SampleSecond
            : FMath::Min(S.FirstSecond, Snapshot.SampleSecond);
        S.LastSecond = FMath::Max(S.LastSecond, Snapshot.SampleSecond);
        ++S.SnapshotCount;
        if (!Snapshot.BlockingActorName.IsEmpty())
        {
            ++S.BlockedSamples;
            S.LastBlockingActor = Snapshot.BlockingActorName;
        }
        if (!Snapshot.bCollisionEnabled) ++S.CollisionDisabledSamples;
        if (!Snapshot.NearbyVehicleIds.IsEmpty()) ++S.NearbyVehicleSamples;
        if (Snapshot.TrafficState == TEXT("RouteComplete"))
            ++S.RouteCompleteSamples;
        S.MaximumStationarySeconds = FMath::Max(
            S.MaximumStationarySeconds, Snapshot.StationarySeconds);
        if (Snapshot.DistanceToPlannedStopCm >= 0.0f)
            S.MinimumDistanceToPlannedStopCm = FMath::Min(
                S.MinimumDistanceToPlannedStopCm,
                Snapshot.DistanceToPlannedStopCm);
        if (Snapshot.Reason == TEXT("ShotMoment"))
        {
            S.bHasShotLocation = true;
            S.ShotLocation = Snapshot.Location;
            S.ExpectedShotAnchorId = Snapshot.ExpectedShotAnchorId;
            S.DistanceToExpectedShotAnchorCm =
                Snapshot.DistanceToExpectedShotAnchorCm;
            S.bAtExpectedShotAnchor = Snapshot.bAtExpectedShotAnchor;
        }
        S.ProfileTimelineEntryCount = Snapshot.ProfileTimelineEntryCount;
        S.ProfileStopEntryCount = Snapshot.ProfileStopEntryCount;
        S.LastTrafficState = Snapshot.TrafficState;
        S.LastLaneId = Snapshot.CurrentLaneId;
    }

    FString VehicleSummaryCsv;
    AppendCsv(VehicleSummaryCsv, {
        TEXT("VehicleId"), TEXT("FirstSecond"), TEXT("LastSecond"),
        TEXT("SnapshotCount"), TEXT("BlockedSamples"),
        TEXT("CollisionDisabledSamples"), TEXT("NearbyVehicleSamples"),
        TEXT("RouteCompleteSamples"), TEXT("MaximumStationarySeconds"),
        TEXT("MinimumDistanceToPlannedStopCm"), TEXT("LastTrafficState"),
        TEXT("LastLaneId"), TEXT("LastBlockingActor"),
        TEXT("HasShotLocation"), TEXT("ShotX"), TEXT("ShotY"), TEXT("ShotZ"),
        TEXT("ExpectedShotAnchorId"), TEXT("DistanceToExpectedShotAnchorCm"),
        TEXT("AtExpectedShotAnchor"), TEXT("ProfileTimelineEntryCount"),
        TEXT("ProfileStopEntryCount")
    });
    TArray<TSharedPtr<FJsonValue>> JsonVehicleSummaries;
    TArray<FName> VehicleSummaryIds;
    VehicleSummaries.GetKeys(VehicleSummaryIds);
    VehicleSummaryIds.Sort([](const FName& A, const FName& B)
    {
        return A.LexicalLess(B);
    });
    for (const FName VehicleId : VehicleSummaryIds)
    {
        const FVehicleSummary& S = VehicleSummaries.FindChecked(VehicleId);
        const float MinimumStopDistance =
            S.MinimumDistanceToPlannedStopCm < TNumericLimits<float>::Max()
            ? S.MinimumDistanceToPlannedStopCm : -1.0f;
        AppendCsv(VehicleSummaryCsv, {
            VehicleId.ToString(), I(S.FirstSecond), I(S.LastSecond),
            I(S.SnapshotCount), I(S.BlockedSamples),
            I(S.CollisionDisabledSamples), I(S.NearbyVehicleSamples),
            I(S.RouteCompleteSamples), N(S.MaximumStationarySeconds),
            N(MinimumStopDistance), S.LastTrafficState,
            S.LastLaneId.ToString(), S.LastBlockingActor,
            B(S.bHasShotLocation), N(S.ShotLocation.X), N(S.ShotLocation.Y),
            N(S.ShotLocation.Z), S.ExpectedShotAnchorId.ToString(),
            N(S.DistanceToExpectedShotAnchorCm), B(S.bAtExpectedShotAnchor),
            I(S.ProfileTimelineEntryCount), I(S.ProfileStopEntryCount)
        });
        TSharedRef<FJsonObject> O = MakeShared<FJsonObject>();
        O->SetStringField(TEXT("vehicleId"), VehicleId.ToString());
        O->SetNumberField(TEXT("firstSecond"), S.FirstSecond);
        O->SetNumberField(TEXT("lastSecond"), S.LastSecond);
        O->SetNumberField(TEXT("snapshotCount"), S.SnapshotCount);
        O->SetNumberField(TEXT("blockedSamples"), S.BlockedSamples);
        O->SetNumberField(TEXT("collisionDisabledSamples"),
            S.CollisionDisabledSamples);
        O->SetNumberField(TEXT("nearbyVehicleSamples"),
            S.NearbyVehicleSamples);
        O->SetNumberField(TEXT("routeCompleteSamples"),
            S.RouteCompleteSamples);
        O->SetNumberField(TEXT("maximumStationarySeconds"),
            S.MaximumStationarySeconds);
        O->SetNumberField(TEXT("minimumDistanceToPlannedStopCm"),
            MinimumStopDistance);
        O->SetStringField(TEXT("lastTrafficState"), S.LastTrafficState);
        O->SetStringField(TEXT("lastLaneId"), S.LastLaneId.ToString());
        O->SetStringField(TEXT("lastBlockingActor"), S.LastBlockingActor);
        O->SetBoolField(TEXT("hasShotLocation"), S.bHasShotLocation);
        O->SetObjectField(TEXT("shotLocation"), VectorJson(S.ShotLocation));
        O->SetStringField(TEXT("expectedShotAnchorId"),
            S.ExpectedShotAnchorId.ToString());
        O->SetNumberField(TEXT("distanceToExpectedShotAnchorCm"),
            S.DistanceToExpectedShotAnchorCm);
        O->SetBoolField(TEXT("atExpectedShotAnchor"),
            S.bAtExpectedShotAnchor);
        O->SetNumberField(TEXT("profileTimelineEntryCount"),
            S.ProfileTimelineEntryCount);
        O->SetNumberField(TEXT("profileStopEntryCount"),
            S.ProfileStopEntryCount);
        JsonVehicleSummaries.Add(MakeShared<FJsonValueObject>(O));
    }

    // Include the profiles actually loaded by this PIE world. This makes a
    // stale Unreal DataTable import immediately visible in the next report.
    TArray<TSharedPtr<FJsonValue>> JsonLoadedVehicleProfiles;
    if (VehicleDirector.IsValid() &&
        IsValid(VehicleDirector->HistoricalVehicleTable))
    {
        TArray<FName> RowNames;
        VehicleDirector->HistoricalVehicleTable->GetRowMap().GetKeys(RowNames);
        RowNames.Sort([](const FName& A, const FName& B)
        {
            return A.LexicalLess(B);
        });
        for (const FName RowName : RowNames)
        {
            uint8* const* Raw =
                VehicleDirector->HistoricalVehicleTable->GetRowMap().Find(RowName);
            const FTMOPHistoricalVehicleRow* Row = Raw != nullptr
                ? reinterpret_cast<const FTMOPHistoricalVehicleRow*>(*Raw) : nullptr;
            if (Row == nullptr) continue;
            TSharedRef<FJsonObject> Profile = MakeShared<FJsonObject>();
            Profile->SetStringField(TEXT("rowName"), RowName.ToString());
            Profile->SetStringField(TEXT("vehicleId"), Row->VehicleId.ToString());
            Profile->SetBoolField(TEXT("spawnInSimulation"), Row->bSpawnInSimulation);
            TArray<TSharedPtr<FJsonValue>> Entries;
            for (const FTMOPHistoricalVehicleTimelineEntry& Entry : Row->Timeline)
            {
                TSharedRef<FJsonObject> E = MakeShared<FJsonObject>();
                E->SetStringField(TEXT("entryId"), Entry.EntryId.ToString());
                E->SetStringField(TEXT("action"), EnumText(Entry.Action));
                E->SetNumberField(TEXT("second"),
                    Entry.Time.ToSecondsFromMidnight());
                E->SetStringField(TEXT("placementAnchorId"),
                    Entry.PlacementAnchorId.ToString());
                E->SetStringField(TEXT("driverEntityId"),
                    Entry.DriverEntityId.ToString());
                TArray<TSharedPtr<FJsonValue>> Lanes;
                for (const FName LaneId : Entry.OrderedLaneIds)
                    Lanes.Add(MakeShared<FJsonValueString>(LaneId.ToString()));
                E->SetArrayField(TEXT("orderedLaneIds"), Lanes);
                Entries.Add(MakeShared<FJsonValueObject>(E));
            }
            Profile->SetNumberField(TEXT("timelineEntryCount"),
                Row->Timeline.Num());
            Profile->SetArrayField(TEXT("timeline"), Entries);
            JsonLoadedVehicleProfiles.Add(
                MakeShared<FJsonValueObject>(Profile));
        }
    }

    TArray<TSharedPtr<FJsonValue>> JsonTrackedPersonProfiles;
    if (PeopleDirector.IsValid() && IsValid(PeopleDirector->PersonProfileTable))
    {
        TArray<FName> EntityIds;
        TrackedAgents.GetKeys(EntityIds);
        EntityIds.Sort([](const FName& A, const FName& B)
        {
            return A.LexicalLess(B);
        });
        for (const FName EntityId : EntityIds)
        {
            const FTMOPPersonProfileRow* Row =
                PeopleDirector->PersonProfileTable->FindRow<FTMOPPersonProfileRow>(
                    EntityId, TEXT("ValidationExport"), false);
            if (Row == nullptr) continue;
            TSharedRef<FJsonObject> Profile = MakeShared<FJsonObject>();
            Profile->SetStringField(TEXT("entityId"), Row->EntityId.ToString());
            Profile->SetStringField(TEXT("categoryId"), Row->CategoryId.ToString());
            Profile->SetStringField(TEXT("socialGroupId"),
                Row->SocialGroupId.ToString());
            Profile->SetStringField(TEXT("groupLeaderEntityId"),
                Row->GroupLeaderEntityId.ToString());
            TArray<TSharedPtr<FJsonValue>> Entries;
            for (const FTMOPPersonTimelineEntry& Entry : Row->Timeline)
            {
                TSharedRef<FJsonObject> E = MakeShared<FJsonObject>();
                E->SetStringField(TEXT("entryId"), Entry.EntryId.ToString());
                E->SetStringField(TEXT("action"), EnumText(Entry.Action));
                E->SetStringField(TEXT("timingMode"), EnumText(Entry.TimingMode));
                E->SetNumberField(TEXT("historicalSecond"),
                    Entry.Time.ToSecondsFromMidnight());
                E->SetNumberField(TEXT("eventOffsetSeconds"),
                    Entry.EventOffsetSeconds);
                E->SetBoolField(TEXT("timeIsArrival"), Entry.bTimeIsArrival);
                E->SetStringField(TEXT("targetAnchorId"),
                    Entry.TargetAnchorId.ToString());
                E->SetStringField(TEXT("targetEntityId"),
                    Entry.TargetEntityId.ToString());
                E->SetStringField(TEXT("targetSeatId"),
                    Entry.TargetSeatId.ToString());
                Entries.Add(MakeShared<FJsonValueObject>(E));
            }
            Profile->SetNumberField(TEXT("timelineEntryCount"),
                Row->Timeline.Num());
            Profile->SetArrayField(TEXT("timeline"), Entries);
            JsonTrackedPersonProfiles.Add(
                MakeShared<FJsonValueObject>(Profile));
        }
    }

    TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
    Root->SetNumberField(TEXT("schemaVersion"), 3);
    Root->SetStringField(TEXT("generatedUtc"),
        FDateTime::UtcNow().ToIso8601());
    Root->SetStringField(TEXT("mapName"),
        GetWorld() != nullptr ? GetWorld()->GetMapName() : FString());
    Root->SetNumberField(TEXT("validationStartSecond"), ValidationStartSecond);
    Root->SetNumberField(TEXT("exportSecond"), ExportSecond);
    Root->SetNumberField(TEXT("lastObservedSecond"), LastObservedSecond);
    Root->SetNumberField(TEXT("maximumObservedSecond"), MaximumObservedSecond);
    Root->SetNumberField(TEXT("trackedAgents"), TrackedAgents.Num());
    Root->SetNumberField(TEXT("trackedVehicles"), TrackedVehicles.Num());
    Root->SetNumberField(TEXT("recordCount"), Records.Num());
    Root->SetNumberField(TEXT("agentSnapshotCount"), AgentSnapshots.Num());
    Root->SetNumberField(TEXT("vehicleSnapshotCount"), VehicleSnapshots.Num());
    Root->SetNumberField(TEXT("snapshotIntervalSeconds"), SnapshotIntervalSeconds);
    Root->SetNumberField(TEXT("sampleIntervalSeconds"), SampleIntervalSeconds);
    Root->SetNumberField(TEXT("stuckAfterSeconds"), StuckAfterSeconds);
    Root->SetNumberField(TEXT("arrivalWarningDistanceCm"),
        ArrivalWarningDistanceCm);
    Root->SetNumberField(TEXT("timingWarningSeconds"), TimingWarningSeconds);
    Root->SetNumberField(TEXT("timingErrorSeconds"), TimingErrorSeconds);
    Root->SetNumberField(TEXT("failureRepeatReportIntervalSeconds"),
        FailureRepeatReportIntervalSeconds);
    Root->SetNumberField(TEXT("nearbyVehicleRadiusCm"), NearbyVehicleRadiusCm);
    Root->SetNumberField(TEXT("shotAnchorToleranceCm"),
        ShotAnchorToleranceCm);
    Root->SetNumberField(TEXT("shotTimelineMatchWindowSeconds"),
        ShotTimelineMatchWindowSeconds);
    Root->SetBoolField(TEXT("captureAllAgentsPeriodically"),
        bCaptureAllAgentsPeriodically);
    Root->SetBoolField(TEXT("captureImportantAgentsPeriodically"),
        bCaptureImportantAgentsPeriodically);
    Root->SetBoolField(TEXT("captureAllVehiclesPeriodically"),
        bCaptureAllVehiclesPeriodically);
    Root->SetNumberField(TEXT("shotSnapshotSecond"),
        ShotSnapshotTime.ToSecondsFromMidnight());
    if (PeopleDirector.IsValid() && IsValid(PeopleDirector->PersonProfileTable))
    {
        Root->SetStringField(TEXT("loadedPeopleTablePath"),
            PeopleDirector->PersonProfileTable->GetPathName());
        Root->SetNumberField(TEXT("loadedPeopleTableRowCount"),
            PeopleDirector->PersonProfileTable->GetRowMap().Num());
    }
    if (VehicleDirector.IsValid() &&
        IsValid(VehicleDirector->HistoricalVehicleTable))
    {
        Root->SetStringField(TEXT("loadedVehicleTablePath"),
            VehicleDirector->HistoricalVehicleTable->GetPathName());
        Root->SetNumberField(TEXT("loadedVehicleTableRowCount"),
            VehicleDirector->HistoricalVehicleTable->GetRowMap().Num());
    }
    Root->SetArrayField(TEXT("records"), JsonRecords);
    Root->SetArrayField(TEXT("agentSnapshots"), JsonAgentSnapshots);
    Root->SetArrayField(TEXT("vehicleSnapshots"), JsonVehicleSnapshots);
    Root->SetArrayField(TEXT("entitySummaries"), JsonEntitySummaries);
    Root->SetArrayField(TEXT("vehicleSummaries"), JsonVehicleSummaries);
    Root->SetArrayField(TEXT("loadedVehicleProfiles"),
        JsonLoadedVehicleProfiles);
    Root->SetArrayField(TEXT("trackedPersonProfiles"),
        JsonTrackedPersonProfiles);
    FString Json;
    const TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Json);
    FJsonSerializer::Serialize(Root, Writer);

    const bool bCsv = FFileHelper::SaveStringToFile(
        Csv, *(Base + TEXT(".csv")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bAgentCsv = FFileHelper::SaveStringToFile(
        AgentCsv, *(Base + TEXT("_AgentSnapshots.csv")),
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bVehicleCsv = FFileHelper::SaveStringToFile(
        VehicleCsv, *(Base + TEXT("_VehicleSnapshots.csv")),
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bSummaryCsv = FFileHelper::SaveStringToFile(
        SummaryCsv, *(Base + TEXT("_EntitySummary.csv")),
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bVehicleSummaryCsv = FFileHelper::SaveStringToFile(
        VehicleSummaryCsv, *(Base + TEXT("_VehicleSummary.csv")),
        FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    const bool bJson = FFileHelper::SaveStringToFile(
        Json, *(Base + TEXT(".json")), FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP validation exported %d events, %d agent snapshots and %d vehicle snapshots to %s"),
        Records.Num(), AgentSnapshots.Num(), VehicleSnapshots.Num(), *Directory);
    return bCsv && bAgentCsv && bVehicleCsv && bSummaryCsv &&
        bVehicleSummaryCsv && bJson;
}
