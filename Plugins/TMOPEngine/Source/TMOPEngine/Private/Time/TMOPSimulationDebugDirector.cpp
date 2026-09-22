#include "Time/TMOPSimulationDebugDirector.h"
#include "Items/TMOPItemMeshSubsystem.h"
#include "Testing/TMOPTimelineValidationDirector.h"
#include "Time/TMOPWorldPlaybackComponent.h"
#include "Time/TMOPTimeTravelPolicy.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"

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
    PrimaryActorTick.bTickEvenWhenPaused = true;
    WorldPlayback = CreateDefaultSubobject<UTMOPWorldPlaybackComponent>(TEXT("WorldPlayback"));
}

void ATMOPSimulationDebugDirector::BeginPlay()
{
    Super::BeginPlay();

    const UTMOPSimulationSettings* Settings = GetDefault<UTMOPSimulationSettings>();
    BakeSampleIntervalSeconds = TMOPTimeTravel::SeekStepSeconds;
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
            InputComponent->BindKey(EKeys::B, IE_Pressed, this, &ATMOPSimulationDebugDirector::DebugBakeKey).bExecuteWhenPaused = true;
        }
    }

    const bool bHasBakeRequest = IFileManager::Get().FileExists(
        *(GetResolvedBakePath() + TEXT(".request")));
    if ((bBakeOnNextBeginPlay || bHasBakeRequest) && GetWorld() != nullptr &&
        GetWorld()->IsGameWorld())
    {
        bBakeOnNextBeginPlay = true;
    }
    bInitializePlaybackNextTick = true;
}

void ATMOPSimulationDebugDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bInitializePlaybackNextTick)
    {
        bInitializePlaybackNextTick = false;
        if (bBakeOnNextBeginPlay) StartPersonBakeRecording();
        else if (WorldPlayback) WorldPlayback->LoadAndPrepare(BuildSourceSignature());
    }
    if (!bRecordingBake) return;
    if (WorldPlayback && !WorldPlayback->IsRecording())
    {
        bRecordingBake = false;
        return;
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

    return WorldPlayback && WorldPlayback->RequestSeek(TargetSecond);

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
        if (WorldPlayback) WorldPlayback->FinishRecording(false);
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
    if (WorldPlayback) IFileManager::Get().Delete(*WorldPlayback->GetTapePath(), false, true);
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
    if (!WorldPlayback) return;
    const bool Valid = WorldPlayback->ValidateFile(BuildSourceSignature());
    UE_LOG(LogTemp, Display, TEXT("TMOP bake %s: %s"), Valid ? TEXT("VALID") : TEXT("INVALID"), *WorldPlayback->GetStatus());
}
void ATMOPSimulationDebugDirector::VerifyHistoricalPlayback()
{
    if (WorldPlayback) WorldPlayback->VerifyRepeatability();
}
void ATMOPSimulationDebugDirector::LoadWorldBakeFromDisk()
{
    if (WorldPlayback && GetWorld() && GetWorld()->IsGameWorld()) WorldPlayback->LoadAndPrepare(BuildSourceSignature());
    else ValidateWorldBake();
}

bool ATMOPSimulationDebugDirector::StartPersonBakeRecording()
{
    UTMOPClockSubsystem* Clock = GetClock();
    if (Clock == nullptr || bRecordingBake) return false;
    if (WorldPlayback && WorldPlayback->IsReady())
    {
        UE_LOG(LogTemp, Error, TEXT("Stop Play, choose Bake Entire Simulation, then start a new Play session to author a new history."));
        return false;
    }

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

    TArray<FString> InputErrors;
    if (auto* People = FindPersonDirector())
        if (!People->ValidatePeopleTable(InputErrors) || (People->GroupDefinitionTable && !People->ValidateGroupTable(InputErrors)))
        { for (const auto& Error : InputErrors) UE_LOG(LogTemp, Error, TEXT("TMOP bake: %s"), *Error); return false; }
    if (auto* Vehicles = FindVehicleDirector())
        if (!Vehicles->ValidateHistoricalVehicleTable(InputErrors))
        { for (const auto& Error : InputErrors) UE_LOG(LogTemp, Error, TEXT("TMOP bake: %s"), *Error); return false; }
    ATMOPTimelineValidationDirector* Validator = nullptr;
    for (TActorIterator<ATMOPTimelineValidationDirector> It(GetWorld()); It; ++It) { Validator = *It; break; }
    if (!Validator) Validator = GetWorld()->SpawnActor<ATMOPTimelineValidationDirector>();
    if (!Validator) return false;
    if (WorldPlayback) WorldPlayback->AddTickPrerequisiteActor(Validator);
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

    Clock->PauseClock();
    Clock->bAuthoritativePlayback = false;
    Clock->RestartLoop();
    if (auto* Vehicles = FindVehicleDirector()) Vehicles->InitializeHistoricalVehicles();
    if (auto* People = FindPersonDirector()) People->InitializePersonSimulation();
    RestoreDerivedScheduledSystems();
    Validator->StartValidation();
    if (!WorldPlayback || !WorldPlayback->StartRecording(BuildSourceSignature()))
    {
        bRecordingBake = false;
        return false;
    }
    Clock->SetTimeScale(1.0f);
    Clock->StartClock();
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
    const bool bTapeSaved = WorldPlayback && WorldPlayback->FinishRecording(true);
    return bTapeSaved;
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
    return JumpToSimulationTime(TargetTime);
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
    return WorldPlayback && WorldPlayback->FinishRecording(true);
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
    return WorldPlayback && WorldPlayback->LoadAndPrepare(BuildSourceSignature());
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
    FString Source = FString::Printf(TEXT("TMOP_AUTHORITY_V3_20260922_20HZ:scene%d"), PlaybackSceneRevision);
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
    AddTable(TEXT("ItemMeshes"), GetDefault<UTMOPItemMeshSubsystem>()->ItemMeshTable);
    if (const ATMOPPersonRegistryDirector* People = FindPersonDirector())
    {
        AddTable(TEXT("People"), People->PersonProfileTable);
        AddTable(TEXT("Appearance"), People->AppearanceAssetTable);
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
            for (const auto* Lane : Lanes) if (Lane)
            {
                for (int32 Point=0; Point<Lane->GetNumberOfSplinePoints(); ++Point)
                    WorldParts.Add(FString::Printf(TEXT("|LanePoint=%s:%d:%s:%s"), *Lane->LaneId.ToString(), Point,
                        *Lane->GetTransformAtSplinePoint(Point, ESplineCoordinateSpace::World, true).ToString(),
                        *Lane->GetTangentAtSplinePoint(Point, ESplineCoordinateSpace::World).ToString()));
                WorldParts.Add(FString::Printf(TEXT("|LaneClosed=%s:%d"), *Lane->LaneId.ToString(), Lane->IsClosedLoop()));
            }
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
    // Profile keys may use digits; legacy debug input must not also seek the world.
    for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
        if (Player->bUseControlProfiles) return;
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
    for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
        if (Player->bUseControlProfiles) return;
    if (bRecordingBake) StopPersonBakeRecordingAndSave();
    else StartPersonBakeRecording();
}
