#include "Routes/TMOPRouteSubsystem.h"

bool UTMOPRouteSubsystem::RegisterRoute(
    const FTMOPHistoricalRouteDefinition& Route)
{
    if (Route.RouteId.IsNone() || Route.Steps.IsEmpty())
    {
        return false;
    }

    TSet<FName> SeenStepIds;

    for (const FTMOPRouteStep& Step : Route.Steps)
    {
        if (Step.TargetAnchorId.IsNone())
        {
            return false;
        }

        if (!Step.StepId.IsNone())
        {
            if (SeenStepIds.Contains(Step.StepId))
            {
                return false;
            }

            SeenStepIds.Add(Step.StepId);
        }
    }

    Routes.Add(Route.RouteId, Route);
    return true;
}

bool UTMOPRouteSubsystem::RemoveRoute(const FName RouteId)
{
    return Routes.Remove(RouteId) > 0;
}

bool UTMOPRouteSubsystem::HasRoute(const FName RouteId) const
{
    return Routes.Contains(RouteId);
}

bool UTMOPRouteSubsystem::TryGetRoute(
    const FName RouteId,
    FTMOPHistoricalRouteDefinition& OutRoute) const
{
    if (const FTMOPHistoricalRouteDefinition* Found =
        Routes.Find(RouteId))
    {
        OutRoute = *Found;
        return true;
    }

    OutRoute = FTMOPHistoricalRouteDefinition();
    return false;
}

TArray<FName> UTMOPRouteSubsystem::GetRegisteredRouteIds() const
{
    TArray<FName> RouteIds;
    Routes.GetKeys(RouteIds);
    RouteIds.Sort(FNameLexicalLess());
    return RouteIds;
}
