#include "Navigation/TMOPNavigationAreas.h"

UTMOPNavArea_Sidewalk::UTMOPNavArea_Sidewalk()
{
    DefaultCost = 1.0f;
    FixedAreaEnteringCost = 0.0f;
    DrawColor = FColor(220, 80, 180);
}

UTMOPNavArea_Crosswalk::UTMOPNavArea_Crosswalk()
{
    DefaultCost = 1.05f;
    FixedAreaEnteringCost = 0.0f;
    DrawColor = FColor(245, 245, 245);
}

UTMOPNavArea_Road::UTMOPNavArea_Road()
{
    DefaultCost = 8.0f;
    FixedAreaEnteringCost = 5.0f;
    DrawColor = FColor(70, 70, 70);
}

UTMOPNavArea_Stairs::UTMOPNavArea_Stairs()
{
    DefaultCost = 1.7f;
    FixedAreaEnteringCost = 0.5f;
    DrawColor = FColor(235, 170, 70);
}

UTMOPNavArea_Bridge::UTMOPNavArea_Bridge()
{
    DefaultCost = 1.4f;
    FixedAreaEnteringCost = 0.25f;
    DrawColor = FColor(80, 170, 230);
}

UTMOPNavArea_Interior::UTMOPNavArea_Interior()
{
    DefaultCost = 1.2f;
    FixedAreaEnteringCost = 0.0f;
    DrawColor = FColor(150, 110, 220);
}

UTMOPNavArea_Restricted::UTMOPNavArea_Restricted()
{
    DrawColor = FColor::Red;
}
