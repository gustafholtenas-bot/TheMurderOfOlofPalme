#include "Routes/TMOPRouteFollowerComponent.h"

#include "Actions/TMOPActionExecutorComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Routes/TMOPRouteSubsystem.h"

UTMOPRouteFollowerComponent::UTMOPRouteFollowerComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPRouteFollowerComponent::BeginPlay()
{
    Super::BeginPlay();

    if (const ATMOPHistoricalAgent* Agent =
        Cast<ATMOPHistoricalAgent>(GetOwner()))
    {
        if (Agent->ActionExecutor != nullptr)
        {
            DefaultArrivalRadius =
                Agent->ActionExecutor->ArrivalRadius;

            Agent->ActionExecutor->OnActionExecutionChanged.AddDynamic(
                this,
                &UTMOPRouteFollowerComponent::HandleActionExecutionChanged);
        }
    }
}

void UTMOPRouteFollowerComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    if (const ATMOPHistoricalAgent* Agent =
        Cast<ATMOPHistoricalAgent>(GetOwner()))
    {
        if (Agent->ActionExecutor != nullptr)
        {
            Agent->ActionExecutor->OnActionExecutionChanged.RemoveDynamic(
                this,
                &UTMOPRouteFollowerComponent::HandleActionExecutionChanged);
        }
    }

    CancelRoute();
    Super::EndPlay(EndPlayReason);
}

bool UTMOPRouteFollowerComponent::StartRouteById(
    const FName RouteId)
{
    UWorld* World = GetWorld();
    UGameInstance* GameInstance =
        World != nullptr ? World->GetGameInstance() : nullptr;

    UTMOPRouteSubsystem* Routes =
        GameInstance != nullptr
            ? GameInstance->GetSubsystem<UTMOPRouteSubsystem>()
            : nullptr;

    if (Routes == nullptr)
    {
        return false;
    }

    FTMOPHistoricalRouteDefinition Route;
    return Routes->TryGetRoute(RouteId, Route) &&
        StartRoute(Route);
}

bool UTMOPRouteFollowerComponent::StartRoute(
    const FTMOPHistoricalRouteDefinition& Route)
{
    if (Route.RouteId.IsNone() ||
        Route.Steps.IsEmpty() ||
        IsFollowingRoute())
    {
        return false;
    }

    CurrentRoute = Route;
    Runtime.RouteId = Route.RouteId;
    Runtime.State = ETMOPRouteExecutionState::Executing;
    Runtime.CurrentStepIndex = 0;
    Runtime.StartedLoopNumber = 0;

    if (ATMOPHistoricalAgent* Agent =
        Cast<ATMOPHistoricalAgent>(GetOwner()))
    {
        Agent->MovementPolicy = Route.MovementPolicy;
    }

    OnRouteStateChanged.Broadcast(
        Runtime.RouteId,
        Runtime.State,
        Runtime.CurrentStepIndex);

    return BeginCurrentStep();
}

void UTMOPRouteFollowerComponent::CancelRoute()
{
    if (Runtime.State != ETMOPRouteExecutionState::Executing)
    {
        return;
    }

    Runtime.State = ETMOPRouteExecutionState::Cancelled;

    if (const ATMOPHistoricalAgent* Agent =
        Cast<ATMOPHistoricalAgent>(GetOwner()))
    {
        if (Agent->ActionExecutor != nullptr)
        {
            Agent->ActionExecutor->CancelCurrentAction();
            Agent->ActionExecutor->ArrivalRadius =
                DefaultArrivalRadius;
        }
    }

    OnRouteStateChanged.Broadcast(
        Runtime.RouteId,
        Runtime.State,
        Runtime.CurrentStepIndex);
}

void UTMOPRouteFollowerComponent::HandleActionExecutionChanged(
    const FName EntryId,
    const ETMOPScheduleActionType ActionType,
    const ETMOPActionExecutionState State)
{
    if (Runtime.State != ETMOPRouteExecutionState::Executing ||
        EntryId != MakeActionEntryId(Runtime.CurrentStepIndex))
    {
        return;
    }

    if (State == ETMOPActionExecutionState::Completed)
    {
        AdvanceRoute();
    }
    else if (State == ETMOPActionExecutionState::Failed)
    {
        FinishRoute(ETMOPRouteExecutionState::Failed);
    }
}

bool UTMOPRouteFollowerComponent::BeginCurrentStep()
{
    if (!CurrentRoute.Steps.IsValidIndex(Runtime.CurrentStepIndex))
    {
        FinishRoute(ETMOPRouteExecutionState::Completed);
        return true;
    }

    ATMOPHistoricalAgent* Agent =
        Cast<ATMOPHistoricalAgent>(GetOwner());

    if (Agent == nullptr || Agent->ActionExecutor == nullptr)
    {
        FinishRoute(ETMOPRouteExecutionState::Failed);
        return false;
    }

    const FTMOPRouteStep& Step =
        CurrentRoute.Steps[Runtime.CurrentStepIndex];

    Agent->ActionExecutor->ArrivalRadius =
        Step.ArrivalRadiusOverride > 0.0f
            ? Step.ArrivalRadiusOverride
            : DefaultArrivalRadius;

    FTMOPScheduleEntry Entry;
    Entry.EntryId = MakeActionEntryId(Runtime.CurrentStepIndex);
    Entry.ActionType = ETMOPScheduleActionType::MoveToAnchor;
    Entry.TargetAnchorId = Step.TargetAnchorId;
    Entry.ActivityState = Step.ActivityState;
    Entry.Confidence = Step.Confidence;
    Entry.SourceId = Step.SourceId;
    Entry.Notes = Step.Notes;

    OnRouteStateChanged.Broadcast(
        Runtime.RouteId,
        Runtime.State,
        Runtime.CurrentStepIndex);

    if (!Agent->ActionExecutor->ExecuteScheduleEntry(Entry))
    {
        FinishRoute(ETMOPRouteExecutionState::Failed);
        return false;
    }

    return true;
}

void UTMOPRouteFollowerComponent::AdvanceRoute()
{
    ++Runtime.CurrentStepIndex;

    if (Runtime.CurrentStepIndex >= CurrentRoute.Steps.Num())
    {
        if (CurrentRoute.bLoopRoute)
        {
            Runtime.CurrentStepIndex = 0;
            BeginCurrentStep();
        }
        else
        {
            FinishRoute(ETMOPRouteExecutionState::Completed);
        }

        return;
    }

    BeginCurrentStep();
}

void UTMOPRouteFollowerComponent::FinishRoute(
    const ETMOPRouteExecutionState FinalState)
{
    Runtime.State = FinalState;

    if (const ATMOPHistoricalAgent* Agent =
        Cast<ATMOPHistoricalAgent>(GetOwner()))
    {
        if (Agent->ActionExecutor != nullptr)
        {
            Agent->ActionExecutor->ArrivalRadius =
                DefaultArrivalRadius;
        }
    }

    OnRouteStateChanged.Broadcast(
        Runtime.RouteId,
        Runtime.State,
        Runtime.CurrentStepIndex);
}

FName UTMOPRouteFollowerComponent::MakeActionEntryId(
    const int32 StepIndex) const
{
    return FName(
        *FString::Printf(
            TEXT("%s__STEP_%03d"),
            *CurrentRoute.RouteId.ToString(),
            StepIndex));
}
