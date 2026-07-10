#include "Navigation/TMOPNavigationPolicyLibrary.h"

#include "Navigation/TMOPNavigationQueryFilters.h"

bool UTMOPNavigationPolicyLibrary::PolicyNormallyAvoidsRoad(
    const ETMOPMovementPolicy Policy)
{
    return Policy == ETMOPMovementPolicy::NormalPedestrian ||
        Policy == ETMOPMovementPolicy::CautiousPedestrian ||
        Policy == ETMOPMovementPolicy::FastPedestrian;
}

bool UTMOPNavigationPolicyLibrary::PolicyAllowsHistoricalRoadOverride(
    const ETMOPMovementPolicy Policy)
{
    return Policy == ETMOPMovementPolicy::HistoricalOverride ||
        Policy == ETMOPMovementPolicy::Emergency ||
        Policy == ETMOPMovementPolicy::Police;
}

float UTMOPNavigationPolicyLibrary::GetSuggestedSurfaceCost(
    const ETMOPRouteSurfacePreference SurfacePreference)
{
    switch (SurfacePreference)
    {
    case ETMOPRouteSurfacePreference::SidewalkPreferred:
        return 1.0f;
    case ETMOPRouteSurfacePreference::CrosswalkPreferred:
        return 1.0f;
    case ETMOPRouteSurfacePreference::RoadAllowed:
        return 6.0f;
    case ETMOPRouteSurfacePreference::RoadRequired:
        return 0.8f;
    case ETMOPRouteSurfacePreference::StairsAllowed:
        return 1.7f;
    case ETMOPRouteSurfacePreference::InteriorOnly:
        return 1.2f;
    default:
        return 1.0f;
    }
}

TSubclassOf<UNavigationQueryFilter>
UTMOPNavigationPolicyLibrary::GetFilterClassForPolicy(
    const ETMOPMovementPolicy Policy)
{
    switch (Policy)
    {
    case ETMOPMovementPolicy::CautiousPedestrian:
        return UTMOPNavFilter_CautiousPedestrian::StaticClass();
    case ETMOPMovementPolicy::FastPedestrian:
        return UTMOPNavFilter_FastPedestrian::StaticClass();
    case ETMOPMovementPolicy::HistoricalOverride:
        return UTMOPNavFilter_HistoricalOverride::StaticClass();
    case ETMOPMovementPolicy::Emergency:
    case ETMOPMovementPolicy::Police:
        return UTMOPNavFilter_Emergency::StaticClass();
    case ETMOPMovementPolicy::VehiclePassenger:
    case ETMOPMovementPolicy::NormalPedestrian:
    default:
        return UTMOPNavFilter_NormalPedestrian::StaticClass();
    }
}
