#include "Traffic/TMOPTrafficSignalComponent.h"

UTMOPTrafficSignalComponent::UTMOPTrafficSignalComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPTrafficSignalComponent::ApplySignalState(const ETMOPTrafficSignalState NewState)
{
    CurrentState = NewState;
}
