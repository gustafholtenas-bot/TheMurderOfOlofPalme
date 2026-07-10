#include "Navigation/TMOPNavigationQueryFilters.h"

/*
 * UE 5.8 no longer exposes the older SetAreaCost and
 * SetFixedAreaEnteringCost helpers on UNavigationQueryFilter.
 *
 * For this compatibility version the filter classes remain distinct and are
 * selected per agent, while the actual base costs come from the TMOP NavArea
 * classes themselves:
 *
 * Sidewalk   1.0
 * Crosswalk  1.05
 * Interior   1.2
 * Bridge     1.4
 * Stairs     1.7
 * Road       8.0 + entering cost
 * Restricted blocked
 *
 * Per-policy overrides will be reintroduced later through UE 5.8's current
 * filter configuration API after the first navigation test is working.
 */

UTMOPNavFilter_NormalPedestrian::UTMOPNavFilter_NormalPedestrian()
{
}

UTMOPNavFilter_CautiousPedestrian::UTMOPNavFilter_CautiousPedestrian()
{
}

UTMOPNavFilter_FastPedestrian::UTMOPNavFilter_FastPedestrian()
{
}

UTMOPNavFilter_HistoricalOverride::UTMOPNavFilter_HistoricalOverride()
{
}

UTMOPNavFilter_Emergency::UTMOPNavFilter_Emergency()
{
}
