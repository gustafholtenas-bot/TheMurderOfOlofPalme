#include "TMOPLaneSplineActor.h"
#include "Components/SplineComponent.h"

ATMOPLaneSplineActor::ATMOPLaneSplineActor()
{
    PrimaryActorTick.bCanEverTick = false;
    LaneSpline = CreateDefaultSubobject<USplineComponent>(TEXT("LaneSpline"));
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
