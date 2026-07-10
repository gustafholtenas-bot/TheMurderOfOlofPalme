#include "Navigation/TMOPNavigationModifierVolumes.h"

#include "Navigation/TMOPNavigationAreas.h"

ATMOPSidewalkNavVolume::ATMOPSidewalkNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Sidewalk::StaticClass();
}

ATMOPCrosswalkNavVolume::ATMOPCrosswalkNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Crosswalk::StaticClass();
}

ATMOPRoadNavVolume::ATMOPRoadNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Road::StaticClass();
}

ATMOPStairsNavVolume::ATMOPStairsNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Stairs::StaticClass();
}

ATMOPBridgeNavVolume::ATMOPBridgeNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Bridge::StaticClass();
}

ATMOPInteriorNavVolume::ATMOPInteriorNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Interior::StaticClass();
}

ATMOPRestrictedNavVolume::ATMOPRestrictedNavVolume(
    const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    AreaClass = UTMOPNavArea_Restricted::StaticClass();
}
