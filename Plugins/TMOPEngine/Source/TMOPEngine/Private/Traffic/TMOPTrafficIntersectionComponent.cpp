#include "Traffic/TMOPTrafficIntersectionComponent.h"

UTMOPTrafficIntersectionComponent::UTMOPTrafficIntersectionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPTrafficIntersectionComponent::ValidateIntersection(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (IntersectionId.IsNone()) OutErrors.Add(TEXT("IntersectionId is missing."));
    if (IncomingLaneIds.IsEmpty()) OutErrors.Add(TEXT("Intersection has no incoming lanes."));
    if (OutgoingLaneIds.IsEmpty()) OutErrors.Add(TEXT("Intersection has no outgoing lanes."));
    return OutErrors.IsEmpty();
}
