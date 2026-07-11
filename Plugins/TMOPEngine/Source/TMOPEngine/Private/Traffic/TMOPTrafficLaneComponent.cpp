#include "Traffic/TMOPTrafficLaneComponent.h"

#include "Engine/GameInstance.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"

UTMOPTrafficLaneComponent::UTMOPTrafficLaneComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SetClosedLoop(false);
}

void UTMOPTrafficLaneComponent::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr)
        Network->RegisterLane(this);
}

void UTMOPTrafficLaneComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr)
        Network->UnregisterLane(this);
    Super::EndPlay(EndPlayReason);
}

float UTMOPTrafficLaneComponent::GetSpeedLimitCentimetersPerSecond() const
{
    return FMath::Max(0.0f, SpeedLimitKmh) * (100000.0f / 3600.0f);
}

FTransform UTMOPTrafficLaneComponent::GetLaneTransformAtDistance(const float Distance) const
{
    return GetTransformAtDistanceAlongSpline(FMath::Clamp(Distance, 0.0f, GetSplineLength()),
        ESplineCoordinateSpace::World, true);
}

FVector UTMOPTrafficLaneComponent::GetLaneLocationAtDistance(const float Distance) const
{
    return GetLocationAtDistanceAlongSpline(FMath::Clamp(Distance, 0.0f, GetSplineLength()),
        ESplineCoordinateSpace::World);
}

bool UTMOPTrafficLaneComponent::AllowsBus() const
{
    return LaneType == ETMOPTrafficLaneType::General ||
        LaneType == ETMOPTrafficLaneType::BusAllowed ||
        LaneType == ETMOPTrafficLaneType::BusOnly;
}

bool UTMOPTrafficLaneComponent::ValidateLane(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (LaneId.IsNone()) OutErrors.Add(TEXT("LaneId is missing."));
    if (RoadId.IsNone()) OutErrors.Add(TEXT("RoadId is missing."));
    if (DirectionId.IsNone()) OutErrors.Add(TEXT("DirectionId is missing."));
    if (!bRightHandTraffic) OutErrors.Add(TEXT("Lane is not marked for right-hand traffic."));
    if (LaneIndexFromRight < 1 || LaneIndexFromRight > LaneCountSameDirection)
        OutErrors.Add(TEXT("LaneIndexFromRight is outside LaneCountSameDirection."));
    if (GetNumberOfSplinePoints() < 2 || GetSplineLength() < 100.0f)
        OutErrors.Add(TEXT("Lane spline needs at least two points and useful length."));
    for (const FTMOPLaneConnection& Connection : NextLanes)
        if (Connection.TargetLaneId.IsNone()) OutErrors.Add(TEXT("A NextLanes connection has no TargetLaneId."));
    return OutErrors.IsEmpty();
}
