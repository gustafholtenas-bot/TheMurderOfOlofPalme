#pragma once

#include "CoreMinimal.h"
#include "NavAreas/NavArea.h"
#include "NavAreas/NavArea_Null.h"
#include "TMOPNavigationAreas.generated.h"

/**
 * Preferred surface for ordinary pedestrians.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Sidewalk : public UNavArea
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Sidewalk();
};

/**
 * Preferred place for ordinary pedestrians to cross a road.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Crosswalk : public UNavArea
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Crosswalk();
};

/**
 * Drivable road surface. Pedestrians may use it, but at a high path cost.
 * Historical routes can still force explicit road anchors.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Road : public UNavArea
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Road();
};

/**
 * Pedestrian stairs. Slightly more expensive than ordinary sidewalk.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Stairs : public UNavArea
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Stairs();
};

/**
 * Pedestrian bridge or raised passage.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Bridge : public UNavArea
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Bridge();
};

/**
 * Interior walkable surface.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Interior : public UNavArea
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Interior();
};

/**
 * Completely blocked navigation area.
 */
UCLASS()
class TMOPENGINE_API UTMOPNavArea_Restricted : public UNavArea_Null
{
    GENERATED_BODY()

public:
    UTMOPNavArea_Restricted();
};
