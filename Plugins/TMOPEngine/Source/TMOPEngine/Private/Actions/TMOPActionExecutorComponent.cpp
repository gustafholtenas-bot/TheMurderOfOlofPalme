#include "Actions/TMOPActionExecutorComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
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
    CurrentEntry = FTMOPScheduleEntry();
    ExecutionState = ETMOPActionExecutionState::Idle;
    SetComponentTickEnabled(false);
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
    UWorld* World = GetWorld();
    UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;

    if (!IsValid(Agent) || GameInstance == nullptr ||
        Entry.TargetAnchorId.IsNone())
    {
        CompleteCurrentAction(false);
        return false;
    }

    UTMOPAnchorSubsystem* Anchors =
        GameInstance->GetSubsystem<UTMOPAnchorSubsystem>();

    ATMOPHistoricalAnchor* TargetAnchor =
        Anchors != nullptr
            ? Anchors->FindAnchor(Entry.TargetAnchorId)
            : nullptr;

    AController* Controller = Agent->GetController();

    if (!IsValid(TargetAnchor) || !IsValid(Controller))
    {
        CompleteCurrentAction(false);
        return false;
    }

    const FName StableKey = Agent->EntityIdentity != nullptr
        ? (!Agent->SocialGroupId.IsNone() ? Agent->SocialGroupId
                                         : Agent->EntityIdentity->EntityId)
        : NAME_None;
    CurrentTargetLocation = TargetAnchor->GetPlacementLocation(StableKey);
    Agent->SetActivityState(Entry.ActivityState);

    if (!Agent->CanMove())
    {
        Agent->SetActivityState(ETMOPAgentActivityState::Walking);
    }

    UAIBlueprintHelperLibrary::SimpleMoveToLocation(
        Controller,
        CurrentTargetLocation);

    ExecutionState = ETMOPActionExecutionState::WaitingForArrival;
    SetComponentTickEnabled(true);

    OnActionExecutionChanged.Broadcast(
        Entry.EntryId,
        Entry.ActionType,
        ExecutionState);

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

    if (bSuccessful && GameInstance != nullptr)
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
    CurrentEntry = FTMOPScheduleEntry();
    ExecutionState = ETMOPActionExecutionState::Idle;
    SetComponentTickEnabled(false);
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
