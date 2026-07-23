#include "Time/TMOPSimulationDebugDirector.h"

#include "AIController.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Components/InputComponent.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Groups/TMOPGroupDirector.h"
#include "HAL/FileManager.h"
#include "InputCoreTypes.h"
#include "JsonObjectConverter.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationSettings.h"

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

    if (bLoadExistingBakeOnBeginPlay) LoadPersonBake();
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
    if (LastRecordedSecond == INDEX_NONE ||
        Second - LastRecordedSecond >= BakeSampleIntervalSeconds)
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
    Clock->SetCurrentTime(TargetTime);

    if (ATMOPPersonRegistryDirector* People = FindPersonDirector())
        People->InitializePersonSimulation();
    if (bApplyBakeAfterTimeJump && !BakeData.Frames.IsEmpty())
        ApplyPersonBakeAtTime(TargetTime);

    if (bWasRunning) Clock->StartClock();
    UE_LOG(LogTemp, Display, TEXT("TMOP debug seek: %s."), *TargetTime.ToDisplayString());
    return true;
}

void ATMOPSimulationDebugDirector::SetSimulationTimeScale(const float NewTimeScale)
{
    if (UTMOPClockSubsystem* Clock = GetClock()) Clock->SetTimeScale(NewTimeScale);
}

bool ATMOPSimulationDebugDirector::StartPersonBakeRecording()
{
    UTMOPClockSubsystem* Clock = GetClock();
    if (Clock == nullptr || bRecordingBake) return false;

    BakeData = FTMOPPersonBakeData();
    BakeData.ScenarioStartTime = Clock->GetLoopStartTime();
    BakeData.ScenarioEndTime = Clock->GetLoopEndTime();
    BakeData.SampleIntervalSeconds = FMath::Max(1, BakeSampleIntervalSeconds);
    LastRecordedSecond = INDEX_NONE;
    bRecordingBake = true;

    const bool bOldApplyBake = bApplyBakeAfterTimeJump;
    bApplyBakeAfterTimeJump = false;
    JumpToSimulationTime(BakeData.ScenarioStartTime);
    bApplyBakeAfterTimeJump = bOldApplyBake;
    Clock->SetTimeScale(1.0f);
    Clock->StartClock();
    CaptureBakeFrame(Clock->GetCurrentTime());
    LastRecordedSecond = Clock->GetCurrentTime().ToSecondsFromMidnight();
    UE_LOG(LogTemp, Display, TEXT("TMOP person bake recording started at standard speed."));
    return true;
}

bool ATMOPSimulationDebugDirector::StopPersonBakeRecordingAndSave()
{
    if (!bRecordingBake) return false;
    bRecordingBake = false;
    return SavePersonBake();
}

void ATMOPSimulationDebugDirector::FinishBakeAfterLoop()
{
    bRecordingBake = false;
    if (UTMOPClockSubsystem* Clock = GetClock()) Clock->PauseClock();
    SavePersonBake();
    UE_LOG(LogTemp, Display, TEXT("TMOP person bake recording completed with %d frames."),
        BakeData.Frames.Num());
}

void ATMOPSimulationDebugDirector::CaptureBakeFrame(const FTMOPTime& Time)
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
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
            State.State = Snapshot.State;
            State.TargetLocation = Snapshot.TargetLocation;
            State.AcceptanceRadius = Snapshot.AcceptanceRadius;
            Frame.Groups.Add(MoveTemp(State));
        }

    Frame.People.Sort([](const FTMOPBakedPersonState& A, const FTMOPBakedPersonState& B)
        { return A.EntityId.LexicalLess(B.EntityId); });
    BakeData.Frames.Add(MoveTemp(Frame));
}

bool ATMOPSimulationDebugDirector::ApplyPersonBakeAtTime(const FTMOPTime TargetTime)
{
    const FTMOPPersonBakeFrame* Frame = FindNearestBakeFrame(TargetTime);
    if (Frame == nullptr) return false;
    ATMOPPersonRegistryDirector* People = FindPersonDirector();
    if (!IsValid(People)) return false;

    TSet<FName> GroupMembers;
    if (ATMOPGroupDirector* Groups = FindGroupDirector())
        for (const FTMOPGroupSnapshot& Snapshot : Groups->GetAllGroupSnapshots())
            for (const FName MemberId : Snapshot.MemberEntityIds) GroupMembers.Add(MemberId);

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
        for (const FTMOPBakedGroupState& State : Frame->Groups)
            if (State.State == ETMOPGroupState::Moving &&
                Groups->DoesGroupExist(State.GroupId))
                Groups->MoveGroupToLocation(
                    State.GroupId, State.TargetLocation, State.AcceptanceRadius);

    UE_LOG(LogTemp, Display, TEXT("TMOP person bake frame %s applied for seek to %s."),
        *Frame->Time.ToDisplayString(), *TargetTime.ToDisplayString());
    return true;
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
    FString Json;
    if (!FJsonObjectConverter::UStructToJsonObjectString(
        FTMOPPersonBakeData::StaticStruct(), &BakeData, Json, 0, 0)) return false;
    const FString Path = GetResolvedBakePath();
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path), true);
    const bool bSaved = FFileHelper::SaveStringToFile(Json, *Path);
    if (bSaved)
    {
        UE_LOG(LogTemp, Display, TEXT("TMOP person bake save: %s"), *Path);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP person bake save failed: %s"), *Path);
    }
    return bSaved;
}

bool ATMOPSimulationDebugDirector::LoadPersonBake()
{
    FString Json;
    const FString Path = GetResolvedBakePath();
    if (!FFileHelper::LoadFileToString(Json, *Path)) return false;
    FTMOPPersonBakeData Loaded;
    if (!FJsonObjectConverter::JsonObjectStringToUStruct(
        Json, &Loaded, 0, 0)) return false;
    BakeData = MoveTemp(Loaded);
    BakeSampleIntervalSeconds = FMath::Max(1, BakeData.SampleIntervalSeconds);
    UE_LOG(LogTemp, Display, TEXT("TMOP person bake loaded: %d frames from %s."),
        BakeData.Frames.Num(), *Path);
    return !BakeData.Frames.IsEmpty();
}

FString ATMOPSimulationDebugDirector::GetResolvedBakePath() const
{
    FString File = BakeFileName.IsEmpty() ? TEXT("TMOP_People_2300_2345") : BakeFileName;
    if (!File.EndsWith(TEXT(".json"))) File += TEXT(".json");
    return FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("TMOP"), TEXT("Bakes"), File);
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

void ATMOPSimulationDebugDirector::HandleDigit(const int32 Digit)
{
    const APlayerController* PC = GetWorld() != nullptr
        ? GetWorld()->GetFirstPlayerController() : nullptr;
    const bool bShift = IsValid(PC) &&
        (PC->IsInputKeyDown(EKeys::LeftShift) || PC->IsInputKeyDown(EKeys::RightShift));
    if (bShift)
    {
        if (Digit >= 1 && Digit <= 5) JumpToSimulationTime(FTMOPTime(23, 40 + Digit, 0));
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
