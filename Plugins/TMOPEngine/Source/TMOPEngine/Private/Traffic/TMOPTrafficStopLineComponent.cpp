#include "Traffic/TMOPTrafficStopLineComponent.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

void UTMOPTrafficStopLineComponent::ProjectPositionOntoLane()
{
    if (!GetWorld() || LaneId.IsNone()) return;
    UTMOPTrafficLaneComponent* Found = nullptr;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Lanes; It->GetComponents(Lanes);
        for (auto* Lane : Lanes)
            if (Lane->LaneId == LaneId)
            {
                if (Found) { UE_LOG(LogTemp, Error, TEXT("Duplicate lane ID; projection cancelled.")); return; }
                Found = Lane;
            }
    }
    if (!Found) { UE_LOG(LogTemp, Error, TEXT("Stop line lane not found.")); return; }
    Modify();
    const float Key = Found->FindInputKeyClosestToWorldLocation(GetComponentLocation());
    DistanceAlongLane = Found->GetDistanceAlongSplineAtSplineInputKey(Key);
    SetWorldLocation(Found->GetLocationAtDistanceAlongSpline(DistanceAlongLane, ESplineCoordinateSpace::World));
    MarkPackageDirty();
}

UTMOPTrafficStopLineComponent::UTMOPTrafficStopLineComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPTrafficStopLineComponent::ValidateStopLine(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (StopLineId.IsNone()) OutErrors.Add(TEXT("StopLineId is missing."));
    if (LaneId.IsNone()) OutErrors.Add(TEXT("LaneId is missing."));
    if (SignalGroupId.IsNone()) OutErrors.Add(TEXT("SignalGroupId is missing."));
    return OutErrors.IsEmpty();
}
