#include "Anchors/TMOPHistoricalPlace.h"

#include "Anchors/TMOPAnchorAutoRegistrationComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"

ATMOPHistoricalPlace::ATMOPHistoricalPlace()
{
    AnchorAutoRegistration =
        CreateDefaultSubobject<UTMOPAnchorAutoRegistrationComponent>(
            TEXT("AnchorAutoRegistration"));

    if (EntityIdentity != nullptr)
    {
        EntityIdentity->EntityType = TEXT("Place");
    }
}
