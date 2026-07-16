#include "TMOPLaneSplineActor.h"
#include "Traffic/TMOPTrafficLaneComponent.h"

ATMOPLaneSplineActor::ATMOPLaneSplineActor()
{
    PrimaryActorTick.bCanEverTick = false;
    LaneSpline = CreateDefaultSubobject<UTMOPTrafficLaneComponent>(TEXT("LaneSpline"));
    RootComponent = LaneSpline;
    LaneSpline->SetClosedLoop(false);
    LaneSpline->SetDrawDebug(true);
}

void ATMOPLaneSplineActor::SetLanePoints(const TArray<FVector>& WorldPoints)
{
    Modify();
    LaneSpline->Modify();
    LaneSpline->ClearSplinePoints(false);
    for (const FVector& Point : WorldPoints)
    {
        LaneSpline->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
    }
    LaneSpline->SetClosedLoop(false, false);
    LaneSpline->UpdateSpline();
}
