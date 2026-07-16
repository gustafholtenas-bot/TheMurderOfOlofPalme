#include "TMOPLaneAnchorComponent.h"
#include "TMOPLaneSplineActor.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "EngineUtils.h"

UTMOPLaneAnchorComponent::UTMOPLaneAnchorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPLaneAnchorComponent::AttachToNearestLane()
{
    AActor* Owner = GetOwner();
    UWorld* World = GetWorld();
    if (!Owner || !World) return;
    ATMOPLaneSplineActor* Best = nullptr;
    FVector BestPoint = FVector::ZeroVector;
    float BestDistanceSquared = TNumericLimits<float>::Max();
    for (TActorIterator<ATMOPLaneSplineActor> It(World); It; ++It)
    {
        ATMOPLaneSplineActor* Candidate = *It;
        if (!Candidate || !Candidate->LaneSpline || Candidate->bIsCrossing) continue;
        const FVector Point = Candidate->LaneSpline->FindLocationClosestToWorldLocation(
            Owner->GetActorLocation(), ESplineCoordinateSpace::World);
        const float D2 = FVector::DistSquared(Point, Owner->GetActorLocation());
        if (D2 < BestDistanceSquared) { BestDistanceSquared = D2; Best = Candidate; BestPoint = Point; }
    }
    if (!Best) return;
    Modify(); Owner->Modify();
    LaneActor = Best; LaneID = Best->LaneID;
    const float Key = Best->LaneSpline->FindInputKeyClosestToWorldLocation(Owner->GetActorLocation());
    DistanceAlongLaneCm = Best->LaneSpline->GetDistanceAlongSplineAtSplineInputKey(Key);
    if (bSnapOwnerToLane) Owner->SetActorLocation(BestPoint);
}

void UTMOPLaneAnchorComponent::RefreshFromLaneID()
{
    if (LaneID.IsNone() || !GetWorld()) return;
    for (TActorIterator<ATMOPLaneSplineActor> It(GetWorld()); It; ++It)
    {
        if (It->LaneID == LaneID) { Modify(); LaneActor = *It; return; }
    }
    LaneActor = nullptr;
}

FVector UTMOPLaneAnchorComponent::GetLaneWorldLocation() const
{
    return LaneActor && LaneActor->LaneSpline
        ? LaneActor->LaneSpline->GetLocationAtDistanceAlongSpline(DistanceAlongLaneCm, ESplineCoordinateSpace::World)
        : (GetOwner() ? GetOwner()->GetActorLocation() : FVector::ZeroVector);
}
