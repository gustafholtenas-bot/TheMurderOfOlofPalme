#include "Traffic/TMOPTrafficStopLineComponent.h"

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
