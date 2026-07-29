#include "Time/TMOPSimulationDebugDirector.h"

#include "AIController.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/LightComponent.h"
#include "Components/InputComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Events/TMOPHistoricalEventDirector.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Groups/TMOPGroupDirector.h"
#include "HAL/FileManager.h"
#include "InputCoreTypes.h"
#include "JsonObjectConverter.h"
#include "Misc/DateTime.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "Observations/TMOPObservationDirector.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationSettings.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Transit/TMOPBusScheduleDirector.h"
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "World/TMOPAerialVehicleDirector.h"
#include "World/TMOPLightScheduleDirector.h"
#include "World/TMOPTimedPropDirector.h"

ATMOPSimulationDebugDirector::ATMOPSimulationDebugDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPSimulationDebugDirector::BeginPlay()
{
    Super::BeginPlay();

    const UTMOPSimulationSettings* Settings = GetDefault<UTMOPSimulationSettings>();
    BakeSampleIntervalSeconds = FMath::Max(1, Settings->PersonBakeIntervalSeconds);
    if (BakeFileName.IsEmpty()) BakeFileName = Settings->DefaultPersonBakeName;
    bEnableTimeShortcutKeys = bEnableTimeShortcutKeys && Settings->bEnableDebugTimeKeys;

    if (bEnableTimeShortcutKeys)
    {
        EnableInput(GetWorld() != nullptr ? GetWorld()->GetFirstPlayerController() : nullptr);
        if (InputComponent != nullptr)
        {
            InputComponent->BindKey(EKeys::One, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey1);
            InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey2);
            InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey3);
            InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey4);
            InputComponent->BindKey(EKeys::Five, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey5);
            InputComponent->BindKey(EKeys::Six, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey6);
            InputComponent->BindKey(EKeys::Seven, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey7);
            InputComponent->BindKey(EKeys::Eight, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey8);
            InputComponent->BindKey(EKeys::Nine, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugKey9);
            InputComponent->BindKey(EKeys::B, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugBakeKey);
        }
    }

    const bool bHasBakeRequest = IFileManager::Get().FileExists(
        *(GetResolvedBakePath() + TEXT(".request")));
    if ((bBakeOnNextBeginPlay || bHasBakeRequest) && GetWorld() != nullptr &&
        GetWorld()->IsGameWorld())
    {
        StartPersonBakeRecording();
    }
    else if (bLoadExistingBakeOnBeginPlay)
    {
        LoadPersonBake();
    }
}

void ATMOPSimulationDebugDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bRecordingBake) return;

    UTMOPClockSubsystem* Clock = GetClock();
    if (Clock == nullptr) return;
    const FTMOPTime Time = Clock->GetCurrentTime();
    const int32 Second = Time.ToSecondsFromMidnight();
    if (LastRecordedSecond != INDEX_NONE && Second < LastRecordedSecond)
    {
        FinishBakeAfterLoop();
        return;
    }
    bool bExactEventBoundary = false;
    if (GetGameInstance() != nullptr)
    {
        if (const UTMOPHistoricalEventSubsystem* Events =
            GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        {
            for (const FName EventId : Events->GetRegisteredEventIds())
            {
                FTMOPHistoricalEventRuntime Runtime;
                if (Events->TryGetEventRuntime(EventId, Runtime) &&
                    Runtime.bHasResolvedTime &&
                    Runtime.ResolvedTime.ToSecondsFromMidnight() == Second)
                {
                    bExactEventBoundary = true;
                    break;
                }
            }
        }
    }
    if (LastRecordedSecond == INDEX_NONE ||
        Second - LastRecordedSecond >= BakeSampleIntervalSeconds ||
        (bExactEventBoundary && Second != LastRecordedSecond))
    {
        CaptureBakeFrame(Time);
        LastRecordedSecond = Second;
    }
}

bool ATMOPSimulationDebugDirector::JumpToSimulationTime(const FTMOPTime TargetTime)
{
    UTMOPClockSubsystem* Clock = GetClock();
    if (Clock == nullptr) return false;
    const int32 TargetSecond = TargetTime.ToSecondsFromMidnight();
    const int32 StartSecond = Clock->GetLoopStartTime().ToSecondsFromMidnight();
    const int32 EndSecond = Clock->GetLoopEndTime().ToSecondsFromMidnight();
    if (TargetSecond < StartSecond || TargetSecond > EndSecond)
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP debug seek rejected: %s is outside %s-%s."),
            *TargetTime.ToDisplayString(), *Clock->GetLoopStartTime().ToDisplayString(),
            *Clock->GetLoopEndTime().ToDisplayString());
        return false;
    }

    const bool bWasRunning = Clock->IsClockRunning();
    Clock->PauseClock();
    Clock->RestartLoop();

    TArray<FString> BakeErrors;
    const bool bUseBake = bApplyBakeAfterTimeJump &&
        !BakeData.Frames.IsEmpty() &&
        ValidateLoadedBake(BakeErrors);
    if (bUseBake)
    {
        if (UTMOPHistoricalEventSubsystem* Events =
            GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>())
        {
            TArray<FTMOPHistoricalEventRuntime> Runtime;
            Runtime.Reserve(BakeData.SharedEvents.Num());
            for (const FTMOPBakedEventState& Event : BakeData.SharedEvents)
            {
                Runtime.Add(Event.Runtime);
            }
            Events->ApplyBakedEventRuntime(Runtime, TargetTime);
        }
    }
    Clock->SetCurrentTime(TargetTime);

    if (ATMOPPersonRegistryDirector* People = FindPersonDirector())
    {
        if (bUseBake) People->InitializePersonSimulationForWorldBake();
        else People->InitializePersonSimulation();
    }
    if (ATMOPHistoricalVehicleDirector* Vehicles = FindVehicleDirector())
        Vehicles->InitializeHistoricalVehicles();

    RestoreDerivedScheduledSystems();
    if (bUseBake)
        ApplyPersonBakeAtTime(TargetTime);
    else if (bApplyBakeAfterTimeJump && !BakeData.Frames.IsEmpty())
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP World Bake is stale/invalid; timeline catch-up was used."));

    if (bWasRunning) Clock->StartClock();
    UE_LOG(LogTemp, Display, TEXT("TMOP debug seek: %s."), *TargetTime.ToDisplayString());
    return true;
}

void ATMOPSimulationDebugDirector::SetSimulationTimeScale(const float NewTimeScale)
{
    if (UTMOPClockSubsystem* Clock = GetClock()) Clock->SetTimeScale(NewTimeScale);
}

void ATMOPSimulationDebugDirector::BakeEntireSimulation()
{
    if (GetWorld() != nullptr && GetWorld()->IsGameWorld())
    {
        StartPersonBakeRecording();
        return;
    }
    const FString RequestPath = GetResolvedBakePath() + TEXT(".request");
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(RequestPath), true);
    FFileHelper::SaveStringToFile(TEXT("TMOP World Bake requested"),
        *RequestPath);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP World Bake armed. Press Play to run the complete bake."));
}

void ATMOPSimulationDebugDirector::CancelWorldBake()
{
    bBakeOnNextBeginPlay = false;
    IFileManager::Get().Delete(
        *(GetResolvedBakePath() + TEXT(".request")), false, true);
    if (bRecordingBake)
    {
        bRecordingBake = false;
        if (UTMOPClockSubsystem* Clock = GetClock())
        {
            Clock->PauseClock();
        }
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP World Bake cancelled; the previous file was preserved."));
    }
    Modify();
    MarkPackageDirty();
}

void ATMOPSimulationDebugDirector::ClearWorldBake()
{
    if (bRecordingBake)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP World Bake cannot be cleared while recording."));
        return;
    }
    BakeData = FTMOPWorldBakeData();
    const FString Path = GetResolvedBakePath();
    IFileManager::Get().Delete(*(Path + TEXT(".request")), false, true);
    if (!IFileManager::Get().FileExists(*Path))
    {
        UE_LOG(LogTemp, Display, TEXT("TMOP World Bake is already empty: %s"),
            *Path);
        return;
    }
    if (IFileManager::Get().Delete(*Path, false, true))
    {
        UE_LOG(LogTemp, Display, TEXT("TMOP World Bake deleted: %s"), *Path);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP World Bake could not be deleted: %s"),
            *Path);
    }
}

void ATMOPSimulationDebugDirector::ValidateWorldBake()
{
    if (BakeData.Frames.IsEmpty())
    {
        LoadPersonBake();
    }
    TArray<FString> Errors;
    if (ValidateLoadedBake(Errors))
    {
        UE_LOG(LogTemp, Display,
            TEXT("TMOP World Bake valid: %d frames, %d Shared Events."),
            BakeData.Frames.Num(), BakeData.SharedEvents.Num());
        return;
    }
    for (const FString& Error : Errors)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP World Bake validation: %s"), *Error);
    }
}

void ATMOPSimulationDebugDirector::LoadWorldBakeFromDisk()
{
    LoadPersonBake();
}

bool ATMOPSimulationDebugDirector::StartPersonBakeRecording()
{
    UTMOPClockSubsystem* Clock = GetClock();
    if (Clock == nullptr || bRecordingBake) return false;

    if (GetWorld() == nullptr || !GetWorld()->IsGameWorld())
    {
        const FString RequestPath = GetResolvedBakePath() + TEXT(".request");
        IFileManager::Get().MakeDirectory(*FPaths::GetPath(RequestPath), true);
        FFileHelper::SaveStringToFile(TEXT("TMOP World Bake requested"),
            *RequestPath);
        UE_LOG(LogTemp, Display,
            TEXT("TMOP World Bake armed. Press Play to start."));
        return true;
    }

    IFileManager::Get().Delete(
        *(GetResolvedBakePath() + TEXT(".request")), false, true);
    BakeData = FTMOPWorldBakeData();
    BakeData.LevelPackageName =
        GetWorld()->GetOutermost()->GetName().Replace(TEXT("UEDPIE_0_"), TEXT(""));
    BakeData.ScenarioStartTime = Clock->GetLoopStartTime();
    BakeData.ScenarioEndTime = Clock->GetLoopEndTime();
    BakeData.SampleIntervalSeconds = FMath::Max(1, BakeSampleIntervalSeconds);
    BakeData.SourceSignature = BuildSourceSignature();
    BakeData.CreatedUtc = FDateTime::UtcNow().ToIso8601();
    LastRecordedSecond = INDEX_NONE;
    bRecordingBake = true;

    const bool bOldApplyBake = bApplyBakeAfterTimeJump;
    bApplyBakeAfterTimeJump = false;
    JumpToSimulationTime(BakeData.ScenarioStartTime);
    bApplyBakeAfterTimeJump = bOldApplyBake;
    CaptureResolvedSharedEvents();
    Clock->SetTimeScale(FMath::Max(1.0f, BakeTimeScale));
    Clock->StartClock();
    CaptureBakeFrame(Clock->GetCurrentTime());
    LastRecordedSecond = Clock->GetCurrentTime().ToSecondsFromMidnight();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP World Bake started at %gx. Authoritative bakes should use 1x."),
        Clock->GetTimeScale());
    return true;
}

bool ATMOPSimulationDebugDirector::StopPersonBakeRecordingAndSave()
{
    if (!bRecordingBake) return false;
    bRecordingBake = false;
    CaptureResolvedSharedEvents();
    return SavePersonBake();
}

void ATMOPSimulationDebugDirector::FinishBakeAfterLoop()
{
    bRecordingBake = false;
    if (UTMOPClockSubsystem* Clock = GetClock()) Clock->PauseClock();
    SavePersonBake();
    UE_LOG(LogTemp, Display, TEXT("TMOP World Bake completed with %d frames."),
        BakeData.Frames.Num());
}

void ATMOPSimulationDebugDirector::CaptureResolvedSharedEvents()
{
    BakeData.SharedEvents.Reset();
    if (GetGameInstance() == nullptr) return;
    UTMOPHistoricalEventSubsystem* Events =
        GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>();
    if (!IsValid(Events)) return;
    for (const FName EventId : Events->GetRegisteredEventIds())
    {
        FTMOPHistoricalEventRuntime Runtime;
        if (Events->TryGetEventRuntime(EventId, Runtime) &&
            Runtime.bHasResolvedTime)
        {
            FTMOPBakedEventState Baked;
            Baked.Runtime = Runtime;
            BakeData.SharedEvents.Add(MoveTemp(Baked));
        }
    }
}

void ATMOPSimulationDebugDirector::CaptureBakeFrame(const FTMOPTime& Time)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    CaptureResolvedSharedEvents();
    FTMOPPersonBakeFrame Frame;
    Frame.Time = Time;

    for (TActorIterator<ATMOPHistoricalAgent> It(World); It; ++It)
    {
        ATMOPHistoricalAgent* Agent = *It;
        if (!IsValid(Agent) || !IsValid(Agent->EntityIdentity) ||
            Agent->EntityIdentity->EntityId.IsNone()) continue;
        FTMOPBakedPersonState State;
        State.EntityId = Agent->EntityIdentity->EntityId;
        State.WorldTransform = Agent->GetActorTransform();
        State.ActivityState = Agent->ActivityState;
        State.LifeState = Agent->LifeState;
        if (const UCharacterMovementComponent* Movement = Agent->GetCharacterMovement())
            State.Velocity = Movement->Velocity;
        if (IsValid(Agent->ActionExecutor))
            State.bHasMoveTarget =
                Agent->ActionExecutor->TryGetActiveMoveTarget(State.MoveTarget);
        Frame.People.Add(MoveTemp(State));
    }

    if (ATMOPGroupDirector* Groups = FindGroupDirector())
        for (const FTMOPGroupSnapshot& Snapshot : Groups->GetAllGroupSnapshots())
        {
            FTMOPBakedGroupState State;
            State.GroupId = Snapshot.GroupId;
            State.MemberEntityIds = Snapshot.MemberEntityIds;
            State.LeaderEntityId = Snapshot.LeaderEntityId;
            State.State = Snapshot.State;
            State.Formation = Snapshot.Formation;
            State.RemainingConversationSeconds =
                Snapshot.RemainingConversationSeconds;
            State.bConversationHasNoAutomaticEnd =
                Snapshot.bConversationHasNoAutomaticEnd;
            State.TargetLocation = Snapshot.TargetLocation;
            State.AcceptanceRadius = Snapshot.AcceptanceRadius;
            Frame.Groups.Add(MoveTemp(State));
        }

    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
    {
        ATMOPVehicleBase* Vehicle = *It;
        if (!IsValid(Vehicle) || Vehicle->VehicleId.IsNone()) continue;
        FTMOPBakedVehicleState State;
        State.VehicleId = Vehicle->VehicleId;
        State.WorldTransform = Vehicle->GetActorTransform();
        State.LinearVelocity = Vehicle->GetVelocity();
        if (const UTMOPTrafficVehicleMovementComponent* Traffic =
            Vehicle->FindComponentByClass<
                UTMOPTrafficVehicleMovementComponent>())
        {
            State.bHasTrafficMovement = true;
            State.CurrentLaneId = Traffic->CurrentLaneId;
            State.DistanceAlongLaneCm = Traffic->DistanceAlongLane;
            State.SpeedCmPerSecond = Traffic->CurrentSpeedCmPerSecond;
            State.TrafficState = Traffic->TrafficState;
            State.bDrivingEnabled = Traffic->IsDrivingEnabled();
            State.PlannedLaneIds = Traffic->PlannedLaneIds;
        }
        for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
        {
            ATMOPHistoricalAgent* Occupant =
                IsValid(Seat) ? Seat->GetOccupant() : nullptr;
            if (!IsValid(Occupant) || !IsValid(Occupant->EntityIdentity) ||
                Occupant->EntityIdentity->EntityId.IsNone()) continue;
            FTMOPBakedVehicleOccupant BakedOccupant;
            BakedOccupant.SeatId = Seat->SeatId;
            BakedOccupant.PersonEntityId =
                Occupant->EntityIdentity->EntityId;
            State.Occupants.Add(MoveTemp(BakedOccupant));
        }
        Frame.Vehicles.Add(MoveTemp(State));
    }

    if (ATMOPObservationDirector* Observations = FindObservationDirector())
    {
        Frame.Observations = Observations->GetAllObservationRuntime();
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<ULightComponent*> Components;
        It->GetComponents<ULightComponent>(Components);
        for (ULightComponent* Light : Components)
        {
            if (!IsValid(Light)) continue;
            FTMOPBakedLightState State;
            State.OwnerActorName = It->GetFName();
            State.ComponentName = Light->GetFName();
            State.bVisible = Light->IsVisible();
            State.Intensity = Light->Intensity;
            State.LightColor = Light->GetLightColor();
            Frame.Lights.Add(MoveTemp(State));
        }
    }

    Frame.People.Sort([](const FTMOPBakedPersonState& A, const FTMOPBakedPersonState& B)
        { return A.EntityId.LexicalLess(B.EntityId); });
    Frame.Vehicles.Sort([](
        const FTMOPBakedVehicleState& A,
        const FTMOPBakedVehicleState& B)
        { return A.VehicleId.LexicalLess(B.VehicleId); });
    BakeData.Frames.Add(MoveTemp(Frame));
}

bool ATMOPSimulationDebugDirector::ApplyPersonBakeAtTime(const FTMOPTime TargetTime)
{
    const FTMOPPersonBakeFrame* Frame = FindNearestBakeFrame(TargetTime);
    if (Frame == nullptr) return false;
    ATMOPPersonRegistryDirector* People = FindPersonDirector();
    if (!IsValid(People)) return false;

    TSet<FName> DesiredPeople;
    for (const FTMOPBakedPersonState& State : Frame->People)
        DesiredPeople.Add(State.EntityId);
    for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It)
    {
        ATMOPHistoricalAgent* Agent = *It;
        if (!IsValid(Agent) || !IsValid(Agent->EntityIdentity)) continue;
        const FName EntityId = Agent->EntityIdentity->EntityId;
        if (!EntityId.IsNone() && !DesiredPeople.Contains(EntityId))
            Agent->Destroy();
    }

    TSet<FName> GroupMembers;
    for (const FTMOPBakedGroupState& Group : Frame->Groups)
        for (const FName MemberId : Group.MemberEntityIds)
            GroupMembers.Add(MemberId);

    for (const FTMOPBakedPersonState& State : Frame->People)
    {
        ATMOPHistoricalAgent* Agent = People->FindSpawnedPerson(State.EntityId);
        if (!IsValid(Agent)) continue;
        if (AAIController* AI = Cast<AAIController>(Agent->GetController())) AI->StopMovement();
        if (IsValid(Agent->ActionExecutor)) Agent->ActionExecutor->CancelCurrentAction();
        Agent->SetActorTransform(State.WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
        Agent->SetLifeState(State.LifeState);
        Agent->SetActivityState(State.ActivityState);
        if (UCharacterMovementComponent* Movement = Agent->GetCharacterMovement())
            Movement->Velocity = State.Velocity;
        if (State.bHasMoveTarget && !GroupMembers.Contains(State.EntityId) &&
            IsValid(Agent->ActionExecutor))
            Agent->ActionExecutor->RestoreBakedMoveToLocation(
                State.MoveTarget, State.ActivityState);
    }

    if (ATMOPGroupDirector* Groups = FindGroupDirector())
    {
        TSet<FName> DesiredGroupIds;
        for (const FTMOPBakedGroupState& State : Frame->Groups)
            DesiredGroupIds.Add(State.GroupId);
        for (const FTMOPGroupSnapshot& Existing :
            Groups->GetAllGroupSnapshots())
            if (!DesiredGroupIds.Contains(Existing.GroupId))
                Groups->DissolveGroup(Existing.GroupId);

        for (const FTMOPBakedGroupState& State : Frame->Groups)
        {
            bool bFound = false;
            FTMOPGroupSnapshot Existing =
                Groups->GetGroupSnapshot(State.GroupId, bFound);
            if (!bFound)
            {
                FTMOPGroupDefinition Definition;
                Definition.GroupId = State.GroupId;
                Definition.MemberEntityIds = State.MemberEntityIds;
                Definition.LeaderEntityId = State.LeaderEntityId;
                Definition.Formation = State.Formation;
                Groups->CreateGroup(Definition);
            }
            else
            {
                for (const FName Member : Existing.MemberEntityIds)
                    if (!State.MemberEntityIds.Contains(Member))
                        Groups->RemoveMember(State.GroupId, Member);
                for (const FName Member : State.MemberEntityIds)
                    if (!Existing.MemberEntityIds.Contains(Member))
                        Groups->AddMember(State.GroupId, Member);
                Groups->SetGroupLeader(
                    State.GroupId, State.LeaderEntityId);
            }
            if (State.State == ETMOPGroupState::Moving)
                Groups->MoveGroupToLocation(
                    State.GroupId, State.TargetLocation,
                    State.AcceptanceRadius);
            else if (State.State == ETMOPGroupState::Conversing)
                Groups->StartConversation(
                    State.GroupId,
                    State.RemainingConversationSeconds,
                    State.bConversationHasNoAutomaticEnd
                        ? -1.0f : State.RemainingConversationSeconds,
                    0);
            else
                Groups->StopGroup(State.GroupId);
        }
    }

    ApplyBakedVehicles(*Frame);
    if (ATMOPObservationDirector* Observations = FindObservationDirector())
    {
        Observations->ApplyBakedObservationRuntime(Frame->Observations);
    }
    ApplyBakedLights(*Frame);

    UE_LOG(LogTemp, Display, TEXT("TMOP World Bake frame %s applied for seek to %s."),
        *Frame->Time.ToDisplayString(), *TargetTime.ToDisplayString());
    return true;
}

void ATMOPSimulationDebugDirector::ApplyBakedVehicles(
    const FTMOPPersonBakeFrame& Frame)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    TMap<FName, const FTMOPBakedVehicleState*> Desired;
    for (const FTMOPBakedVehicleState& State : Frame.Vehicles)
    {
        Desired.Add(State.VehicleId, &State);
    }

    TArray<ATMOPVehicleBase*> ExistingVehicles;
    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
    {
        if (IsValid(*It) && !It->VehicleId.IsNone())
        {
            ExistingVehicles.Add(*It);
        }
    }

    for (ATMOPVehicleBase* Vehicle : ExistingVehicles)
    {
        const FTMOPBakedVehicleState* const* Found =
            Desired.Find(Vehicle->VehicleId);
        if (Found == nullptr)
        {
            Vehicle->Destroy();
            continue;
        }
        const FTMOPBakedVehicleState& State = **Found;
        Vehicle->SetActorTransform(
            State.WorldTransform, false, nullptr, ETeleportType::TeleportPhysics);
        if (UPrimitiveComponent* Root =
            Cast<UPrimitiveComponent>(Vehicle->GetRootComponent()))
        {
            Root->SetPhysicsLinearVelocity(State.LinearVelocity);
        }
        if (State.bHasTrafficMovement)
        {
            if (UTMOPTrafficVehicleMovementComponent* Traffic =
                Vehicle->FindComponentByClass<
                    UTMOPTrafficVehicleMovementComponent>())
            {
                Traffic->RestoreBakedTrafficState(
                    State.CurrentLaneId,
                    State.DistanceAlongLaneCm,
                    State.SpeedCmPerSecond,
                    State.PlannedLaneIds,
                    State.bDrivingEnabled);
            }
        }

        TMap<FName, FName> DesiredSeats;
        for (const FTMOPBakedVehicleOccupant& Occupant : State.Occupants)
        {
            DesiredSeats.Add(Occupant.SeatId, Occupant.PersonEntityId);
        }
        for (UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
        {
            if (!IsValid(Seat)) continue;
            if (ATMOPHistoricalAgent* Occupant = Seat->GetOccupant())
            {
                const FName OccupantId = IsValid(Occupant->EntityIdentity)
                    ? Occupant->EntityIdentity->EntityId : NAME_None;
                const FName* DesiredOccupant = DesiredSeats.Find(Seat->SeatId);
                if (DesiredOccupant == nullptr ||
                    *DesiredOccupant != OccupantId)
                    Seat->ExitSeat(Occupant);
            }
        }
        ATMOPPersonRegistryDirector* People = FindPersonDirector();
        if (!IsValid(People)) continue;
        for (const FTMOPBakedVehicleOccupant& Occupant : State.Occupants)
        {
            ATMOPHistoricalAgent* Agent =
                People->FindSpawnedPerson(Occupant.PersonEntityId);
            if (IsValid(Agent))
            {
                Vehicle->EnterVehicle(Agent, Occupant.SeatId);
            }
        }
    }
}

void ATMOPSimulationDebugDirector::ApplyBakedLights(
    const FTMOPPersonBakeFrame& Frame)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    TMap<FString, const FTMOPBakedLightState*> Desired;
    for (const FTMOPBakedLightState& State : Frame.Lights)
    {
        Desired.Add(State.OwnerActorName.ToString() + TEXT("|") +
            State.ComponentName.ToString(), &State);
    }
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<ULightComponent*> Components;
        It->GetComponents<ULightComponent>(Components);
        for (ULightComponent* Light : Components)
        {
            if (!IsValid(Light)) continue;
            const FString Key = It->GetFName().ToString() + TEXT("|") +
                Light->GetFName().ToString();
            const FTMOPBakedLightState* const* Found = Desired.Find(Key);
            if (Found == nullptr) continue;
            Light->SetVisibility((*Found)->bVisible, true);
            Light->SetIntensity((*Found)->Intensity);
            Light->SetLightColor((*Found)->LightColor);
        }
    }
}

const FTMOPPersonBakeFrame* ATMOPSimulationDebugDirector::FindNearestBakeFrame(
    const FTMOPTime& Time) const
{
    if (BakeData.Frames.IsEmpty()) return nullptr;
    const int32 Target = Time.ToSecondsFromMidnight();
    const FTMOPPersonBakeFrame* Best = nullptr;
    for (const FTMOPPersonBakeFrame& Frame : BakeData.Frames)
    {
        const int32 FrameSecond = Frame.Time.ToSecondsFromMidnight();
        if (FrameSecond <= Target &&
            (Best == nullptr ||
             FrameSecond > Best->Time.ToSecondsFromMidnight())) Best = &Frame;
    }
    return Best != nullptr ? Best : &BakeData.Frames[0];
}

bool ATMOPSimulationDebugDirector::SavePersonBake() const
{
    if (BakeData.Frames.IsEmpty()) return false;
    return SaveWorldBakeAtomically();
}

bool ATMOPSimulationDebugDirector::SaveWorldBakeAtomically() const
{
    TArray<FString> Errors;
    ValidateLoadedBake(Errors);
    if (!BakeData.Frames.IsEmpty())
    {
        const int32 Start = BakeData.ScenarioStartTime.ToSecondsFromMidnight();
        const int32 End = BakeData.ScenarioEndTime.ToSecondsFromMidnight();
        const int32 First =
            BakeData.Frames[0].Time.ToSecondsFromMidnight();
        const int32 Last =
            BakeData.Frames.Last().Time.ToSecondsFromMidnight();
        if (First > Start)
            Errors.Add(TEXT("Bake does not contain the scenario start."));
        if (Last < End - FMath::Max(1, BakeData.SampleIntervalSeconds))
            Errors.Add(TEXT("Bake was stopped before the scenario end."));
    }
    if (!Errors.IsEmpty())
    {
        for (const FString& Error : Errors)
            UE_LOG(LogTemp, Error,
                TEXT("TMOP World Bake not saved; previous file preserved: %s"),
                *Error);
        return false;
    }

    FString Json;
    if (!FJsonObjectConverter::UStructToJsonObjectString(
        FTMOPWorldBakeData::StaticStruct(), &BakeData, Json, 0, 0)) return false;
    const FString Path = GetResolvedBakePath();
    const FString TemporaryPath = Path + TEXT(".tmp");
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
    if (!FFileHelper::SaveStringToFile(Json, *TemporaryPath))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP World Bake temporary save failed: %s"),
            *TemporaryPath);
        return false;
    }
    if (!IFileManager::Get().Move(
        *Path, *TemporaryPath, true, true, false, true))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP World Bake could not replace the previous file: %s"),
            *Path);
        return false;
    }
    UE_LOG(LogTemp, Display, TEXT("TMOP World Bake saved atomically: %s"),
        *Path);
    return true;
}

bool ATMOPSimulationDebugDirector::LoadPersonBake()
{
    FString Json;
    const FString Path = GetResolvedBakePath();
    if (!FFileHelper::LoadFileToString(Json, *Path)) return false;
    FTMOPWorldBakeData Loaded;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(
        Json, &Loaded, 0, 0)) return false;
    BakeData = MoveTemp(Loaded);
    BakeSampleIntervalSeconds = FMath::Max(1, BakeData.SampleIntervalSeconds);
    TArray<FString> Errors;
    const bool bValid = ValidateLoadedBake(Errors);
    if (bValid)
    {
        UE_LOG(LogTemp, Display,
            TEXT("TMOP World Bake loaded: %d frames from %s."),
            BakeData.Frames.Num(), *Path);
    }
    else
    {
        for (const FString& Error : Errors)
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP World Bake loaded but invalid: %s"), *Error);
    }
    return bValid;
}

FString ATMOPSimulationDebugDirector::GetResolvedBakePath() const
{
    FString File = BakeFileName.IsEmpty()
        ? TEXT("TMOP_World_2300_2345") : BakeFileName;
    if (!File.EndsWith(TEXT(".json"))) File += TEXT(".json");
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("TMOP"), TEXT("Bakes"), File);
}

FString ATMOPSimulationDebugDirector::BuildSourceSignature() const
{
    FString Source;
    if (GetWorld() != nullptr)
    {
        Source += GetWorld()->GetOutermost()->GetName()
            .Replace(TEXT("UEDPIE_0_"), TEXT(""));
    }
    auto AddTable = [&Source](const TCHAR* Label, const UDataTable* Table)
    {
        Source += FString::Printf(TEXT("|%s=%s:%d"), Label,
            *GetPathNameSafe(Table),
            IsValid(Table) ? Table->GetRowMap().Num() : -1);
        if (IsValid(Table))
        {
            TArray<FName> Names = Table->GetRowNames();
            Names.Sort(FNameLexicalLess());
            for (const FName Name : Names)
            {
                Source += TEXT(",") + Name.ToString();
                uint8* const* Row = Table->GetRowMap().Find(Name);
                if (Row != nullptr && *Row != nullptr &&
                    Table->GetRowStruct() != nullptr)
                {
                    FString RowJson;
                    FJsonObjectConverter::UStructToJsonObjectString(
                        Table->GetRowStruct(), *Row, RowJson, 0, 0);
                    Source += RowJson;
                }
            }
        }
    };
    if (const ATMOPPersonRegistryDirector* People = FindPersonDirector())
    {
        AddTable(TEXT("People"), People->PersonProfileTable);
        AddTable(TEXT("Groups"), People->GroupDefinitionTable);
    }
    if (const ATMOPHistoricalVehicleDirector* Vehicles = FindVehicleDirector())
    {
        AddTable(TEXT("Vehicles"), Vehicles->HistoricalVehicleTable);
    }
    if (const ATMOPObservationDirector* Observations = FindObservationDirector())
    {
        AddTable(TEXT("Observations"), Observations->ObservationTable);
        AddTable(TEXT("ObservationLinks"), Observations->ObservationLinkTable);
    }
    if (GetWorld() != nullptr)
    {
        TArray<FString> WorldParts;
        for (TActorIterator<ATMOPHistoricalEventDirector> It(GetWorld());
            It; ++It)
        {
            AddTable(TEXT("SharedEvents"), It->EventTable);
            for (const FTMOPHistoricalEventDefinition& Definition :
                It->EventDefinitions)
            {
                FString DefinitionJson;
                FJsonObjectConverter::UStructToJsonObjectString(
                    FTMOPHistoricalEventDefinition::StaticStruct(),
                    &Definition, DefinitionJson, 0, 0);
                WorldParts.Add(TEXT("|InlineEvent=") + DefinitionJson);
            }
        }
        auto AddStruct = [&WorldParts](UScriptStruct* Struct, const void* Value,
            const TCHAR* Label)
        {
            FString Json;
            if (FJsonObjectConverter::UStructToJsonObjectString(
                Struct, Value, Json, 0, 0))
                WorldParts.Add(
                    FString::Printf(TEXT("|%s=%s"), Label, *Json));
        };
        for (TActorIterator<ATMOPTimedPropDirector> It(GetWorld()); It; ++It)
            for (const FTMOPTimedPropEntry& Entry : It->ScheduledEntries)
                AddStruct(FTMOPTimedPropEntry::StaticStruct(), &Entry,
                    TEXT("TimedProp"));
        for (TActorIterator<ATMOPAerialVehicleDirector> It(GetWorld()); It; ++It)
            for (const FTMOPAerialScheduleEntry& Entry : It->ScheduledFlights)
                AddStruct(FTMOPAerialScheduleEntry::StaticStruct(), &Entry,
                    TEXT("Aerial"));
        for (TActorIterator<ATMOPLightScheduleDirector> It(GetWorld()); It; ++It)
            for (const FTMOPLightScheduleEntry& Entry : It->ScheduledEntries)
                AddStruct(FTMOPLightScheduleEntry::StaticStruct(), &Entry,
                    TEXT("Light"));
        for (TActorIterator<ATMOPBusScheduleDirector> It(GetWorld()); It; ++It)
            for (const FTMOPBusScheduledRun& Entry : It->ScheduledRuns)
                AddStruct(FTMOPBusScheduledRun::StaticStruct(), &Entry,
                    TEXT("Bus"));
        for (TActorIterator<ATMOPHistoricalAnchor> It(GetWorld()); It; ++It)
            WorldParts.Add(FString::Printf(TEXT("|Anchor=%s:%s"),
                *It->GetAnchorId().ToString(),
                *It->GetActorTransform().ToHumanReadableString()));
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Lanes;
            It->GetComponents<UTMOPTrafficLaneComponent>(Lanes);
            for (const UTMOPTrafficLaneComponent* Lane : Lanes)
                if (IsValid(Lane))
                    WorldParts.Add(FString::Printf(TEXT("|Lane=%s:%s"),
                        *Lane->LaneId.ToString(),
                        *Lane->GetComponentTransform().ToHumanReadableString()));
        }
        WorldParts.Sort();
        for (const FString& Part : WorldParts) Source += Part;
    }
    Source.ReplaceInline(TEXT("UEDPIE_0_"), TEXT(""));
    return FMD5::HashAnsiString(*Source);
}

bool ATMOPSimulationDebugDirector::ValidateLoadedBake(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (BakeData.FormatVersion != 2)
        OutErrors.Add(TEXT("Unsupported FormatVersion; expected 2."));
    if (BakeData.Frames.IsEmpty())
        OutErrors.Add(TEXT("No World Bake frames."));
    if (BakeData.SharedEvents.IsEmpty())
        OutErrors.Add(TEXT("No resolved Shared Events."));
    if (BakeData.SampleIntervalSeconds < 1)
        OutErrors.Add(TEXT("Sample interval must be at least one second."));
    FString CurrentLevel;
    if (GetWorld() != nullptr)
        CurrentLevel = GetWorld()->GetOutermost()->GetName()
            .Replace(TEXT("UEDPIE_0_"), TEXT(""));
    if (!CurrentLevel.IsEmpty() && BakeData.LevelPackageName != CurrentLevel)
        OutErrors.Add(FString::Printf(
            TEXT("Bake belongs to level '%s', current level is '%s'."),
            *BakeData.LevelPackageName, *CurrentLevel));
    const FString CurrentSignature = BuildSourceSignature();
    if (!BakeData.SourceSignature.IsEmpty() &&
        !CurrentSignature.IsEmpty() &&
        BakeData.SourceSignature != CurrentSignature)
        OutErrors.Add(TEXT("Source DataTables, Shared Events or level setup changed after the bake."));
    int32 PreviousSecond = INDEX_NONE;
    for (const FTMOPPersonBakeFrame& Frame : BakeData.Frames)
    {
        const int32 Second = Frame.Time.ToSecondsFromMidnight();
        if (PreviousSecond != INDEX_NONE && Second <= PreviousSecond)
        {
            OutErrors.Add(TEXT("Frames are not strictly chronological."));
            break;
        }
        PreviousSecond = Second;
    }
    return OutErrors.IsEmpty();
}

void ATMOPSimulationDebugDirector::RestoreDerivedScheduledSystems()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<ATMOPTimedPropDirector> It(World); It; ++It)
        It->RestartScheduleAtCurrentTime();
    for (TActorIterator<ATMOPAerialVehicleDirector> It(World); It; ++It)
        It->RestartScheduleAtCurrentTime();
    for (TActorIterator<ATMOPLightScheduleDirector> It(World); It; ++It)
        It->RestartScheduleAtCurrentTime();
    if (ATMOPObservationDirector* Observations = FindObservationDirector())
        Observations->ResolveCanonicalTimes();
}

UTMOPClockSubsystem* ATMOPSimulationDebugDirector::GetClock() const
{
    return GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
}

ATMOPPersonRegistryDirector* ATMOPSimulationDebugDirector::FindPersonDirector() const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPPersonRegistryDirector> It(GetWorld()); It; ++It) return *It;
    return nullptr;
}

ATMOPGroupDirector* ATMOPSimulationDebugDirector::FindGroupDirector() const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPGroupDirector> It(GetWorld()); It; ++It) return *It;
    return nullptr;
}

ATMOPHistoricalVehicleDirector*
ATMOPSimulationDebugDirector::FindVehicleDirector() const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPHistoricalVehicleDirector> It(GetWorld()); It; ++It)
        return *It;
    return nullptr;
}

ATMOPObservationDirector*
ATMOPSimulationDebugDirector::FindObservationDirector() const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPObservationDirector> It(GetWorld()); It; ++It)
        return *It;
    return nullptr;
}

void ATMOPSimulationDebugDirector::HandleDigit(const int32 Digit)
{
    const APlayerController* PC = GetWorld() != nullptr
        ? GetWorld()->GetFirstPlayerController() : nullptr;
    const bool bShift = IsValid(PC) &&
        (PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift));
    if (bShift)
    {
        UTMOPClockSubsystem* Clock = GetClock();
        if (Clock == nullptr) return;

        if (Digit == 3)
        {
            Clock->SetTimeScale(1.0f);
            UE_LOG(LogTemp, Display,
                TEXT("TMOP debug time scale: 1x."));
            return;
        }
        if (Digit == 4)
        {
            Clock->SetTimeScale(
                FMath::Max(1.0f, Clock->GetTimeScale() * 2.0f));
            UE_LOG(LogTemp, Display,
                TEXT("TMOP debug time scale: %.0fx."),
                Clock->GetTimeScale());
            return;
        }

        int32 DeltaSeconds = 0;
        if (Digit == 1) DeltaSeconds = -30;
        else if (Digit == 2) DeltaSeconds = 30;
        else return;

        const int32 TargetSeconds =
            Clock->GetCurrentTime().ToSecondsFromMidnight() + DeltaSeconds;
        JumpToSimulationTime(
            FTMOPTime::FromSecondsFromMidnight(TargetSeconds));
        return;
    }
    JumpToSimulationTime(FTMOPTime(23, (Digit - 1) * 5, 0));
}

void ATMOPSimulationDebugDirector::DebugKey1() { HandleDigit(1); }
void ATMOPSimulationDebugDirector::DebugKey2() { HandleDigit(2); }
void ATMOPSimulationDebugDirector::DebugKey3() { HandleDigit(3); }
void ATMOPSimulationDebugDirector::DebugKey4() { HandleDigit(4); }
void ATMOPSimulationDebugDirector::DebugKey5() { HandleDigit(5); }
void ATMOPSimulationDebugDirector::DebugKey6() { HandleDigit(6); }
void ATMOPSimulationDebugDirector::DebugKey7() { HandleDigit(7); }
void ATMOPSimulationDebugDirector::DebugKey8() { HandleDigit(8); }
void ATMOPSimulationDebugDirector::DebugKey9() { HandleDigit(9); }

void ATMOPSimulationDebugDirector::DebugBakeKey()
{
    if (bRecordingBake) StopPersonBakeRecordingAndSave();
    else StartPersonBakeRecording();
}
