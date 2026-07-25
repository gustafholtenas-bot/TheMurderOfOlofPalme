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
#include "Schedules/TMOPScheduleSubsystem.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

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

    CancelCurrentAction();
    Super::EndPlay(EndPlayReason);
}

void UTMOPActionExecutorComponent::TickComponent(
    const float DeltaTime,
    const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (ExecutionState != ETMOPActionExecutionState::WaitingForArrival)
    {
        return;
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

    OnActionExecutionChanged.Broadcast(
        Entry.EntryId,
        Entry.ActionType,
        ExecutionState);

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

    OnActionExecutionChanged.Broadcast(
        CurrentEntry.EntryId,
        CurrentEntry.ActionType,
        ETMOPActionExecutionState::Failed);

    bHasCurrentEntry = false;
    bRestoredFromBake = false;
    CurrentRouteAnchorIds.Reset();
    CurrentRouteAnchorIndex = INDEX_NONE;
    CurrentEntry = FTMOPScheduleEntry();
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

    ExecuteScheduleEntry(Entry);
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

    OnActionExecutionChanged.Broadcast(
        Entry.EntryId,
        Entry.ActionType,
        ExecutionState);

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
    UAIBlueprintHelperLibrary::SimpleMoveToLocation(Controller, CurrentTargetLocation);
    return true;
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

    OnActionExecutionChanged.Broadcast(
        CurrentEntry.EntryId,
        CurrentEntry.ActionType,
        FinalState);

    bHasCurrentEntry = false;
    bRestoredFromBake = false;
    CurrentRouteAnchorIds.Reset();
    CurrentRouteAnchorIndex = INDEX_NONE;
    CurrentEntry = FTMOPScheduleEntry();
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
                Candidate->Destroy();
                ++DespawnedCount;
            }
        }
        UE_LOG(LogTemp, Display,
            TEXT("TMOP people: %d agent(s) reached map exit '%s' and despawned."),
            DespawnedCount, *CurrentTargetLocation.ToString());
    }
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
