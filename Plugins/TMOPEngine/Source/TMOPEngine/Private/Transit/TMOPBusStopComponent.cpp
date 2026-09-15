#include "Transit/TMOPBusStopComponent.h"

#include "Engine/GameInstance.h"
#include "GameFramework/Actor.h"
#include "Transit/TMOPBusArrivalBoardComponent.h"
#include "Transit/TMOPBusStopSubsystem.h"

UTMOPBusStopComponent::UTMOPBusStopComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPBusStopComponent::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPBusStopSubsystem* Stops = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPBusStopSubsystem>() : nullptr) Stops->RegisterStop(this);
    if (bShowArrivalBoard && !StopId.IsNone() && IsValid(GetOwner()))
    {
        auto* Board = NewObject<UTMOPBusArrivalBoardComponent>(GetOwner(), NAME_None, RF_Transient);
        ArrivalBoard = Board;
        Board->Stop = this;
        Board->ConfigureForStation(StopId, StopName);
        Board->BoardOffset = ArrivalBoardOffset;
        Board->MaximumVisibleDistanceCm = ArrivalBoardVisibleDistanceCm;
        GetOwner()->AddInstanceComponent(Board);
        Board->RegisterComponent();
    }
}

void UTMOPBusStopComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPBusStopSubsystem* Stops = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPBusStopSubsystem>() : nullptr) Stops->UnregisterStop(this);
    if (IsValid(ArrivalBoard)) ArrivalBoard->DestroyComponent();
    ArrivalBoard = nullptr;
    Super::EndPlay(EndPlayReason);
}

bool UTMOPBusStopComponent::ServesRoute(const FName RouteId) const
{
    return !RouteId.IsNone() && ServedRouteIds.Contains(RouteId);
}

FVector UTMOPBusStopComponent::GetBoardingWorldLocation() const
{
    return GetComponentTransform().TransformPosition(BoardingLocalOffset);
}

FVector UTMOPBusStopComponent::GetAlightingWorldLocation() const
{
    return GetComponentTransform().TransformPosition(AlightingLocalOffset);
}

bool UTMOPBusStopComponent::ValidateStop(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (StopId.IsNone()) OutErrors.Add(TEXT("StopId is missing."));
    if (LaneId.IsNone()) OutErrors.Add(TEXT("LaneId is missing."));
    if (ServedRouteIds.IsEmpty()) OutErrors.Add(TEXT("ServedRouteIds is empty."));
    if (MaximumDwellSeconds < MinimumDwellSeconds)
        OutErrors.Add(TEXT("MaximumDwellSeconds is below MinimumDwellSeconds."));
    return OutErrors.IsEmpty();
}

