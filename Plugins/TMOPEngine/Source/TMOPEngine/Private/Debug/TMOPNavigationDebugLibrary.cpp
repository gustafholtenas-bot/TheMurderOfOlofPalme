#include "Debug/TMOPNavigationDebugLibrary.h"

#include "Actions/TMOPActionExecutorComponent.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "DrawDebugHelpers.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Routes/TMOPRouteFollowerComponent.h"

FString UTMOPNavigationDebugLibrary::BuildAgentNavigationReport(
    const ATMOPHistoricalAgent* Agent)
{
    if (!IsValid(Agent))
    {
        return TEXT("TMOP Navigation: invalid agent");
    }

    const FName EntityId =
        Agent->EntityIdentity != nullptr
            ? Agent->EntityIdentity->GetEntityId()
            : NAME_None;

    FString RouteId = TEXT("<none>");
    int32 RouteStep = INDEX_NONE;

    if (Agent->RouteFollower != nullptr)
    {
        const FTMOPRouteRuntime Runtime =
            Agent->RouteFollower->GetRouteRuntime();

        RouteId = Runtime.RouteId.ToString();
        RouteStep = Runtime.CurrentStepIndex;
    }

    const FString ActionState =
        Agent->ActionExecutor != nullptr
            ? StaticEnum<ETMOPActionExecutionState>()
                ->GetNameStringByValue(
                    static_cast<int64>(
                        Agent->ActionExecutor->GetExecutionState()))
            : TEXT("<none>");

    return FString::Printf(
        TEXT("TMOP Navigation\n")
        TEXT("Agent: %s\n")
        TEXT("Policy: %s\n")
        TEXT("Activity: %s\n")
        TEXT("Route: %s\n")
        TEXT("Route step: %d\n")
        TEXT("Action state: %s"),
        *EntityId.ToString(),
        *StaticEnum<ETMOPMovementPolicy>()
            ->GetNameStringByValue(
                static_cast<int64>(Agent->MovementPolicy)),
        *StaticEnum<ETMOPAgentActivityState>()
            ->GetNameStringByValue(
                static_cast<int64>(Agent->ActivityState)),
        *RouteId,
        RouteStep,
        *ActionState);
}

void UTMOPNavigationDebugLibrary::DrawAgentNavigationDebug(
    UObject* WorldContextObject,
    const ATMOPHistoricalAgent* Agent,
    const float Duration)
{
    if (!IsValid(WorldContextObject) || !IsValid(Agent))
    {
        return;
    }

    UWorld* World = WorldContextObject->GetWorld();
    if (World == nullptr)
    {
        return;
    }

    const FVector Location = Agent->GetActorLocation();

    DrawDebugSphere(
        World,
        Location,
        45.0f,
        16,
        FColor::Cyan,
        false,
        Duration);

    DrawDebugString(
        World,
        Location + FVector(0.0f, 0.0f, 110.0f),
        BuildAgentNavigationReport(Agent),
        nullptr,
        FColor::White,
        Duration,
        true);
}
