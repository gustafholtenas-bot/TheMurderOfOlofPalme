#include "Anchors/TMOPHistoricalPlace.h"

#include "Entities/TMOPWorldEntityComponent.h"

ATMOPHistoricalPlace::ATMOPHistoricalPlace()
{
    if (EntityIdentity != nullptr)
    {
        EntityIdentity->EntityType = TEXT("Place");
    }
}
