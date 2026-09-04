#include "Actions/TMOPActionExecutorComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NavigationSystem.h"
#include "Schedules/TMOPScheduleSubsystem.h"
#include "Time/TMOPClockSubsystem.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"
#include "World/TMOPVerticalTransport.h"

UTMOPActionExecutorComponent::UTMOPActionExecutorComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTMOPActionExecutorComponent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;

    if (GameInstance != nullptr)
    {
        if (UTMOPScheduleSubsystem* Schedules =
            GameInstance->GetSubsystem<UTMOPScheduleSubsystem>())
        {
            Schedules->OnScheduleEntryReady.AddDynamic(
                this,
                &UTMOPActionExecutorComponent::HandleScheduleEntryReady);
        }
    }
}

void UTMOPActionExecutorComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;

    if (GameInstance != nullptr)
    {
        if (UTMOPScheduleSubsystem* Schedules =
            GameInstance->GetSubsystem<UTMOPScheduleSubsystem>())
        {
            Schedules->OnScheduleEntryReady.RemoveDynamic(
                this,
                &UTMOPActionExecutorComponent::HandleScheduleEntryReady);
        }
    }

    QueuedEntries.Reset();
    QueuedTriggerTimes.Reset();
    CancelCurrentAction();
    Super::EndPlay(EndPlayReason);
}

void UTMOPActionExecutorComponent::TickComponent(
    const float DeltaTime,
    const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ExecutionState == ETMOPActionExecutionState::WaitingForVerticalTransport)
    {
        if (!IsValid(ActiveVerticalTransport) ||
            !ActiveVerticalTransport->IsTransporting(GetOwner()))
        {
            ActiveVerticalTransport = nullptr;
            ++CurrentRouteAnchorIndex;
            if (!MoveToCurrentRouteAnchor()) CompleteCurrentAction(false);
            else ExecutionState = ETMOPActionExecutionState::WaitingForArrival;
        }
        return;
    }

    if (ExecutionState != ETMOPActionExecutionState::WaitingForArrival)
    {
        return;
    }

    TimedSpeedUpdateAccumulator += DeltaTime;
    if (TimedSpeedUpdateAccumulator >= 0.5f)
    {
        TimedSpeedUpdateAccumulator = 0.0f;
        UpdateTimedMovementSpeed();
    }

    const AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor))
    {
        CompleteCurrentAction(false);
        return;
    }

    const float DistanceSquared = FVector::DistSquared2D(
        OwnerActor->GetActorLocation(),
        CurrentTargetLocation);

    if (DistanceSquared <= FMath::Square(ArrivalRadius))
    {
        if (CurrentRouteAnchorIds.IsValidIndex(CurrentRouteAnchorIndex + 1))
        {
            const FName From = CurrentRouteAnchorIds[CurrentRouteAnchorIndex];
            const FName To = CurrentRouteAnchorIds[CurrentRouteAnchorIndex + 1];
            if (ATMOPVerticalTransport* Transport =
                ATMOPVerticalTransport::FindTransport(this, From, To))
            {
                if (Transport->RequestTransport(GetOwner(), From, To))
                {
                    if (AController* Controller = GetHistoricalAgent()->GetController())
                        Controller->StopMovement();
                    ActiveVerticalTransport = Transport;
                    ExecutionState = ETMOPActionExecutionState::WaitingForVerticalTransport;
                    return;
                }
            }
            ++CurrentRouteAnchorIndex;
            if (!MoveToCurrentRouteAnchor())
            {
                CompleteCurrentAction(false);
            }
            return;
        }
        CompleteCurrentAction(true);
    }
}

bool UTMOPActionExecutorComponent::ExecuteScheduleEntry(
    const FTMOPScheduleEntry& Entry)
{
    if (IsExecutingAction())
    {
        return false;
    }

    CurrentEntry = Entry;
    bHasCurrentEntry = true;
    bRestoredFromBake = false;
    ExecutionState = ETMOPActionExecutionState::Executing;

    BroadcastExecutionState(ExecutionState);

    if (Entry.ActionType == ETMOPScheduleActionType::MoveToAnchor)
    {
        return BeginMoveToAnchor(Entry);
    }

    return ExecuteImmediateAction(Entry);
}

void UTMOPActionExecutorComponent::CancelCurrentAction()
{
    if (!bHasCurrentEntry)
    {
        ExecutionState = ETMOPActionExecutionState::Idle;
        SetComponentTickEnabled(false);
        return;
    }

    if (ATMOPHistoricalAgent* Agent = GetHistoricalAgent())
    {
        if (AController* Controller = Agent->GetController())
        {
            Controller->StopMovement();
        }
    }

    BroadcastExecutionState(ETMOPActionExecutionState::Failed);
    RestoreMovementSpeed();

    bHasCurrentEntry = false;
    bRestoredFromBake = false;
    CurrentRouteAnchorIds.Reset();
    CurrentRouteAnchorIndex = INDEX_NONE;
    CurrentEntry = FTMOPScheduleEntry();
    ActiveVerticalTransport = nullptr;
    ExecutionState = ETMOPActionExecutionState::Idle;
    SetComponentTickEnabled(false);
}

bool UTMOPActionExecutorComponent::TryGetActiveMoveTarget(
    FVector& OutTargetLocation) const
{
    if (ExecutionState != ETMOPActionExecutionState::WaitingForArrival)
    {
        OutTargetLocation = FVector::ZeroVector;
        return false;
    }
    OutTargetLocation = CurrentTargetLocation;
    return true;
}

bool UTMOPActionExecutorComponent::RestoreBakedMoveToLocation(
    const FVector TargetLocation,
    const ETMOPAgentActivityState RestoredActivity)
{
    ATMOPHistoricalAgent* Agent = GetHistoricalAgent();
    AController* Controller = IsValid(Agent) ? Agent->GetController() : nullptr;
    if (!IsValid(Agent) || !IsValid(Controller)) return false;

    CancelCurrentAction();
    CurrentEntry = FTMOPScheduleEntry();
    ActiveVerticalTransport = nullptr;
    CurrentEntry.ActionType = ETMOPScheduleActionType::MoveToAnchor;
    CurrentTargetLocation = TargetLocation;
    CurrentRouteAnchorIds.Reset();
    CurrentRouteAnchorIndex = INDEX_NONE;
    bHasCurrentEntry = true;
    bRestoredFromBake = true;
    ExecutionState = ETMOPActionExecutionState::WaitingForArrival;
    Agent->SetActivityState(
        RestoredActivity == ETMOPAgentActivityState::Idle
            ? ETMOPAgentActivityState::Walking
            : RestoredActivity);
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, CurrentTargetLocation);
    SetComponentTickEnabled(true);
    return true;
}

void UTMOPActionExecutorComponent::ConfigureNextTimedMove(
    const int32 ExpectedArrivalSecond,
    const float MinimumSpeedCmPerSecond,
    const float MaximumSpeedCmPerSecond)
{
    PendingExpectedArrivalSecond = ExpectedArrivalSecond;
    PendingMinimumSpeedCmPerSecond = FMath::Max(1.0f, MinimumSpeedCmPerSecond);
    PendingMaximumSpeedCmPerSecond = FMath::Max(
        PendingMinimumSpeedCmPerSecond, MaximumSpeedCmPerSecond);
}

bool UTMOPActionExecutorComponent::GetActiveMoveTimingDiagnostics(
    int32& OutExpectedArrivalSecond,
    float& OutRemainingPathCm,
    float& OutRequiredSpeedCmPerSecond,
    bool& bOutPhysicallyPossible) const
{
    if (ActiveExpectedArrivalSecond == INDEX_NONE ||
        ExecutionState != ETMOPActionExecutionState::WaitingForArrival)
        return false;
    OutExpectedArrivalSecond = ActiveExpectedArrivalSecond;
    OutRemainingPathCm = ActiveRemainingPathCm;
    OutRequiredSpeedCmPerSecond = ActiveRequiredSpeedCmPerSecond;
    bOutPhysicallyPossible = bActiveMovePhysicallyPossible;
    return true;
}

void UTMOPActionExecutorComponent::HandleScheduleEntryReady(
    const FName AgentId,
    const FTMOPScheduleEntry Entry,
    const FTMOPTime TriggerTime)
{
    if (!bAutoExecuteScheduleEntries ||
        AgentId != GetOwnerEntityId())
    {
        return;
    }

    if (IsExecutingAction())
    {
        QueueScheduleEntry(Entry, TriggerTime);
        return;
    }

    CurrentScheduledTime = TriggerTime;
    ExecuteScheduleEntry(Entry);
}

void UTMOPActionExecutorComponent::QueueScheduleEntry(
    const FTMOPScheduleEntry& Entry,
    const FTMOPTime& TriggerTime)
{
    // The schedule subsystem can emit the same entry again while it remains
    // unexecuted. Never add duplicate work to an agent's queue.
    if (CurrentEntry.EntryId == Entry.EntryId)
    {
        return;
    }
    for (const FTMOPScheduleEntry& QueuedEntry : QueuedEntries)
    {
        if (QueuedEntry.EntryId == Entry.EntryId)
        {
            return;
        }
    }

    QueuedEntries.Add(Entry);
    QueuedTriggerTimes.Add(TriggerTime);
    UE_LOG(LogTemp, Verbose,
        TEXT("TMOP actions: queued '%s' for '%s' behind '%s'."),
        *Entry.EntryId.ToString(),
        *GetOwnerEntityId().ToString(),
        *CurrentEntry.EntryId.ToString());
}

void UTMOPActionExecutorComponent::ExecuteNextQueuedEntry()
{
    if (IsExecutingAction() || QueuedEntries.IsEmpty())
    {
        return;
    }

    const FTMOPScheduleEntry NextEntry = QueuedEntries[0];
    const FTMOPTime NextTriggerTime = QueuedTriggerTimes.IsValidIndex(0)
        ? QueuedTriggerTimes[0]
        : FTMOPTime();
    QueuedEntries.RemoveAt(0);
    if (!QueuedTriggerTimes.IsEmpty())
    {
        QueuedTriggerTimes.RemoveAt(0);
    }

    CurrentScheduledTime = NextTriggerTime;
    ExecuteScheduleEntry(NextEntry);
}

bool UTMOPActionExecutorComponent::ExecuteImmediateAction(
    const FTMOPScheduleEntry& Entry)
{
    ATMOPHistoricalAgent* Agent = GetHistoricalAgent();
    if (!IsValid(Agent))
    {
        CompleteCurrentAction(false);
        return false;
    }

    switch (Entry.ActionType)
    {
    case ETMOPScheduleActionType::SetActivity:
        Agent->SetActivityState(Entry.ActivityState);
        break;

    case ETMOPScheduleActionType::StandUp:
        if (UGameInstance* GameInstance = GetWorld() != nullptr
            ? GetWorld()->GetGameInstance() : nullptr)
        {
            UTMOPCinemaSeatSubsystem* Seats =
                GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>();
            const FName SeatId = !Entry.TargetEntityId.IsNone()
                ? Entry.TargetEntityId : Agent->InitialSeatAssignment.SeatId;
            UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(SeatId) : nullptr;
            if (!IsValid(Seat) || Seat->GetOccupyingAgent() != Agent || !Seat->StandAgent(Agent))
                Agent->SetActivityState(ETMOPAgentActivityState::Standing);
        }
        else Agent->SetActivityState(ETMOPAgentActivityState::Standing);
        break;

    case ETMOPScheduleActionType::SitAtSeat:
        if (UGameInstance* GameInstance = GetWorld() != nullptr
            ? GetWorld()->GetGameInstance() : nullptr)
        {
            UTMOPCinemaSeatSubsystem* Seats =
                GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>();
            const FName SeatId = !Entry.TargetEntityId.IsNone()
                ? Entry.TargetEntityId : Agent->InitialSeatAssignment.SeatId;
            UTMOPCinemaSeatComponent* Seat = Seats != nullptr ? Seats->FindSeat(SeatId) : nullptr;
            if (!IsValid(Seat) || !Seat->SeatAgent(Agent))
                Agent->SetActivityState(ETMOPAgentActivityState::Seated);
        }
        else Agent->SetActivityState(ETMOPAgentActivityState::Seated);
        break;

    case ETMOPScheduleActionType::WaitAtAnchor:
        Agent->SetActivityState(ETMOPAgentActivityState::Idle);
        break;

    case ETMOPScheduleActionType::Interact:
    case ETMOPScheduleActionType::Custom:
        break;

    case ETMOPScheduleActionType::EnterVehicle:
    case ETMOPScheduleActionType::ExitVehicle:
        // Boarding/alighting is resolved deterministically by the assigned bus manifest.
        break;

    case ETMOPScheduleActionType::None:
    default:
        CompleteCurrentAction(false);
        return false;
    }

    CompleteCurrentAction(true);
    return true;
}

bool UTMOPActionExecutorComponent::BeginMoveToAnchor(
    const FTMOPScheduleEntry& Entry)
{
    ATMOPHistoricalAgent* Agent = GetHistoricalAgent();
    if (!IsValid(Agent) || Entry.TargetAnchorId.IsNone())
    {
        CompleteCurrentAction(false);
        return false;
    }

    CurrentRouteAnchorIds.Reset();
    for (const FName AnchorId : Entry.PassAnchorIds)
        if (!AnchorId.IsNone()) CurrentRouteAnchorIds.Add(AnchorId);
    CurrentRouteAnchorIds.Add(Entry.TargetAnchorId);
    CurrentRouteAnchorIndex = 0;
    ActiveExpectedArrivalSecond = PendingExpectedArrivalSecond;
    ActiveMinimumSpeedCmPerSecond = PendingMinimumSpeedCmPerSecond;
    ActiveMaximumSpeedCmPerSecond = PendingMaximumSpeedCmPerSecond;
    PendingExpectedArrivalSecond = INDEX_NONE;
    PendingMinimumSpeedCmPerSecond = 0.0f;
    PendingMaximumSpeedCmPerSecond = 0.0f;
    TimedSpeedUpdateAccumulator = 0.0f;
    Agent->SetActivityState(Entry.ActivityState);

    if (!Agent->CanMove())
    {
        Agent->SetActivityState(ETMOPAgentActivityState::Walking);
    }

    if (!MoveToCurrentRouteAnchor())
    {
        CompleteCurrentAction(false);
        return false;
    }

    ExecutionState = ETMOPActionExecutionState::WaitingForArrival;
    SetComponentTickEnabled(true);
    UpdateTimedMovementSpeed(true);

    BroadcastExecutionState(ExecutionState);

    return true;
}

bool UTMOPActionExecutorComponent::MoveToCurrentRouteAnchor()
{
    ATMOPHistoricalAgent* Agent = GetHistoricalAgent();
    UWorld* World = GetWorld();
    UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;
    if (!IsValid(Agent) || GameInstance == nullptr ||
        !CurrentRouteAnchorIds.IsValidIndex(CurrentRouteAnchorIndex))
        return false;

    UTMOPAnchorSubsystem* Anchors =
        GameInstance->GetSubsystem<UTMOPAnchorSubsystem>();
    ATMOPHistoricalAnchor* TargetAnchor = Anchors != nullptr
        ? Anchors->FindAnchor(CurrentRouteAnchorIds[CurrentRouteAnchorIndex])
        : nullptr;
    AController* Controller = Agent->GetController();
    if (!IsValid(TargetAnchor) || !IsValid(Controller)) return false;

    const FName StableKey = Agent->EntityIdentity != nullptr
        ? (!Agent->SocialGroupId.IsNone() ? Agent->SocialGroupId
                                         : Agent->EntityIdentity->EntityId)
        : NAME_None;
    CurrentTargetLocation = TargetAnchor->GetPlacementLocation(StableKey);
    if (CurrentRouteAnchorIndex == CurrentRouteAnchorIds.Num() - 1 &&
        !CurrentEntry.TargetAnchorOffsetCm.IsNearlyZero())
        CurrentTargetLocation += CurrentEntry.bTargetAnchorOffsetIsLocal
            ? TargetAnchor->GetActorQuat().RotateVector(
                CurrentEntry.TargetAnchorOffsetCm)
            : CurrentEntry.TargetAnchorOffsetCm;
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, CurrentTargetLocation);
    return true;
}

float UTMOPActionExecutorComponent::CalculateRemainingPathLengthCm() const
{
    const ATMOPHistoricalAgent* Agent = GetHistoricalAgent();
    UWorld* World = GetWorld();
    const UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;
    const UTMOPAnchorSubsystem* Anchors = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    if (!IsValid(Agent) || World == nullptr || Anchors == nullptr) return 0.0f;

    FVector Start = Agent->GetActorLocation();
    double Total = 0.0;
    for (int32 Index = CurrentRouteAnchorIndex;
        Index < CurrentRouteAnchorIds.Num(); ++Index)
    {
        const ATMOPHistoricalAnchor* Anchor =
            Anchors->FindAnchor(CurrentRouteAnchorIds[Index]);
        if (!IsValid(Anchor)) return 0.0f;
        FVector End = Index == CurrentRouteAnchorIndex
            ? CurrentTargetLocation : Anchor->GetAnchorLocation();
        if (Index == CurrentRouteAnchorIds.Num() - 1 &&
            Index != CurrentRouteAnchorIndex &&
            !CurrentEntry.TargetAnchorOffsetCm.IsNearlyZero())
            End += CurrentEntry.bTargetAnchorOffsetIsLocal
                ? Anchor->GetActorQuat().RotateVector(
                    CurrentEntry.TargetAnchorOffsetCm)
                : CurrentEntry.TargetAnchorOffsetCm;
        double Segment = FVector::Dist2D(Start, End);
        UNavigationSystemV1::GetPathLength(
            World, Start, End, Segment, nullptr, nullptr);
        Total += Segment;
        Start = End;
    }
    return static_cast<float>(Total);
}

void UTMOPActionExecutorComponent::UpdateTimedMovementSpeed(
    const bool bForceUpdate)
{
    ATMOPHistoricalAgent* Agent = GetHistoricalAgent();
    if (!IsValid(Agent) || ActiveExpectedArrivalSecond == INDEX_NONE) return;
    UCharacterMovementComponent* Movement = Agent->GetCharacterMovement();
    const UGameInstance* GameInstance = GetWorld() != nullptr
        ? GetWorld()->GetGameInstance() : nullptr;
    const UTMOPClockSubsystem* Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!IsValid(Movement) || Clock == nullptr) return;

    ActiveRemainingPathCm = CalculateRemainingPathLengthCm();
    const int32 RemainingSeconds = ActiveExpectedArrivalSecond -
        Clock->GetCurrentTime().ToSecondsFromMidnight();
    ActiveRequiredSpeedCmPerSecond = RemainingSeconds > 0
        ? ActiveRemainingPathCm / static_cast<float>(RemainingSeconds)
        : ActiveMaximumSpeedCmPerSecond;
    bActiveMovePhysicallyPossible = RemainingSeconds > 0 &&
        ActiveRequiredSpeedCmPerSecond <= ActiveMaximumSpeedCmPerSecond;
    const float ChosenSpeed = FMath::Clamp(
        ActiveRequiredSpeedCmPerSecond,
        ActiveMinimumSpeedCmPerSecond,
        ActiveMaximumSpeedCmPerSecond);
    if (bForceUpdate ||
        !FMath::IsNearlyEqual(Movement->MaxWalkSpeed, ChosenSpeed, 1.0f))
        Movement->MaxWalkSpeed = ChosenSpeed;
}

void UTMOPActionExecutorComponent::RestoreMovementSpeed()
{
    if (ATMOPHistoricalAgent* Agent = GetHistoricalAgent())
        Agent->ApplyMovementSpeedForActivity();
    ActiveExpectedArrivalSecond = INDEX_NONE;
    ActiveMinimumSpeedCmPerSecond = 0.0f;
    ActiveMaximumSpeedCmPerSecond = 0.0f;
    ActiveRemainingPathCm = 0.0f;
    ActiveRequiredSpeedCmPerSecond = 0.0f;
    bActiveMovePhysicallyPossible = true;
}

void UTMOPActionExecutorComponent::CompleteCurrentAction(
    const bool bSuccessful)
{
    if (!bHasCurrentEntry)
    {
        return;
    }

    const ETMOPActionExecutionState FinalState =
        bSuccessful
            ? ETMOPActionExecutionState::Completed
            : ETMOPActionExecutionState::Failed;

    UWorld* World = GetWorld();
    UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;
    bool bDespawnAtExit = false;
    if (bSuccessful &&
        CurrentEntry.ActionType == ETMOPScheduleActionType::MoveToAnchor &&
        !CurrentEntry.TargetAnchorId.IsNone())
    {
        const FString TargetId = CurrentEntry.TargetAnchorId.ToString();
        bDespawnAtExit =
            TargetId.StartsWith(TEXT("Exit"), ESearchCase::IgnoreCase);
        if (!bDespawnAtExit && GameInstance != nullptr)
        {
            UTMOPAnchorSubsystem* Anchors =
                GameInstance->GetSubsystem<UTMOPAnchorSubsystem>();
            const ATMOPHistoricalAnchor* Target =
                Anchors != nullptr
                ? Anchors->FindAnchor(CurrentEntry.TargetAnchorId)
                : nullptr;
            bDespawnAtExit =
                IsValid(Target) &&
                Target->AnchorCategory == ETMOPAnchorCategory::MapExit;
        }
    }

    ATMOPHistoricalAgent* CompletedAgent = GetHistoricalAgent();
    const FName CompletedGroupId =
        IsValid(CompletedAgent) ? CompletedAgent->SocialGroupId : NAME_None;

    if (bSuccessful && !bRestoredFromBake && GameInstance != nullptr)
    {
        if (UTMOPScheduleSubsystem* Schedules =
            GameInstance->GetSubsystem<UTMOPScheduleSubsystem>())
        {
            Schedules->MarkEntryExecuted(
                GetOwnerEntityId(),
                CurrentEntry.EntryId);
        }
    }

    BroadcastExecutionState(FinalState);
    RestoreMovementSpeed();

    bHasCurrentEntry = false;
    bRestoredFromBake = false;
    CurrentRouteAnchorIds.Reset();
    CurrentRouteAnchorIndex = INDEX_NONE;
    CurrentEntry = FTMOPScheduleEntry();
    ActiveVerticalTransport = nullptr;
    ExecutionState = ETMOPActionExecutionState::Idle;
    SetComponentTickEnabled(false);

    if (bDespawnAtExit && World != nullptr)
    {
        int32 DespawnedCount = 0;
        for (TActorIterator<ATMOPHistoricalAgent> It(World); It; ++It)
        {
            ATMOPHistoricalAgent* Candidate = *It;
            const bool bSameAgent = Candidate == CompletedAgent;
            const bool bSameGroup =
                !CompletedGroupId.IsNone() &&
                Candidate->SocialGroupId == CompletedGroupId;
            if (bSameAgent || bSameGroup)
            {
                Candidate->RequestDespawnWithFade();
                ++DespawnedCount;
            }
        }
        UE_LOG(LogTemp, Display,
            TEXT("TMOP people: %d agent(s) reached map exit '%s' and despawned."),
            DespawnedCount, *CurrentTargetLocation.ToString());
        QueuedEntries.Reset();
        QueuedTriggerTimes.Reset();
        return;
    }

    // A later timeline entry must never interrupt an unfinished movement.
    // Start it only after the current action has completed (or failed), while
    // retaining its original trigger time so validation reports lateness.
    ExecuteNextQueuedEntry();
}

void UTMOPActionExecutorComponent::BroadcastExecutionState(
    const ETMOPActionExecutionState State)
{
    OnActionExecutionChanged.Broadcast(
        CurrentEntry.EntryId,
        CurrentEntry.ActionType,
        State);
    OnActionValidationEvent.Broadcast(
        this,
        CurrentEntry,
        CurrentScheduledTime,
        State);
}

FName UTMOPActionExecutorComponent::GetOwnerEntityId() const
{
    const ATMOPHistoricalAgent* Agent = GetHistoricalAgent();

    return Agent != nullptr &&
        Agent->EntityIdentity != nullptr
            ? Agent->EntityIdentity->GetEntityId()
            : NAME_None;
}

ATMOPHistoricalAgent*
UTMOPActionExecutorComponent::GetHistoricalAgent() const
{
    return Cast<ATMOPHistoricalAgent>(GetOwner());
}
