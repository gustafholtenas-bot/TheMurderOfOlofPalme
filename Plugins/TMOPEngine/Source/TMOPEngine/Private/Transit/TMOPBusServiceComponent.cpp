#include "Transit/TMOPBusServiceComponent.h"

#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Transit/TMOPBusRouteData.h"
#include "Transit/TMOPBusStopComponent.h"
#include "Transit/TMOPBusStopSubsystem.h"

UTMOPBusServiceComponent::UTMOPBusServiceComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTMOPBusServiceComponent::BeginPlay()
{
    Super::BeginPlay();
    InitializeService();
}

bool UTMOPBusServiceComponent::InitializeService()
{
    Movement = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>() : nullptr;
    if (!IsValid(RouteData) || !IsValid(Movement) || RouteData->RouteId.IsNone() ||
        RouteData->OrderedLaneIds.IsEmpty())
    {
        ServiceState = ETMOPBusServiceState::InvalidRoute;
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus service '%s': invalid route data or missing movement component."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"));
        return false;
    }
    DwellRandom.Initialize(DwellRandomSeed + GetTypeHash(ServiceRunId));
    CurrentStopIndex = 0;
    Movement->PlannedLaneIds = RouteData->OrderedLaneIds;
    ServiceState = RouteData->OrderedStopIds.IsEmpty()
        ? ETMOPBusServiceState::RouteComplete : ETMOPBusServiceState::DrivingToStop;
    UE_LOG(LogTemp, Display,
        TEXT("TMOP bus service '%s': route '%s' initialized with %d stop(s)."),
        GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
        *RouteData->RouteId.ToString(), RouteData->OrderedStopIds.Num());
    return true;
}

void UTMOPBusServiceComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (ServiceState == ETMOPBusServiceState::DrivingToStop) UpdateApproachToStop();
    else if (ServiceState == ETMOPBusServiceState::Dwelling)
    {
        RemainingDwellSeconds -= DeltaTime;
        if (RemainingDwellSeconds <= 0.0f)
            if (UTMOPBusStopComponent* Stop = GetCurrentTargetStop()) FinishDwell(Stop);
    }
}

UTMOPBusStopComponent* UTMOPBusServiceComponent::GetCurrentTargetStop() const
{
    if (!IsValid(RouteData) || !RouteData->OrderedStopIds.IsValidIndex(CurrentStopIndex)) return nullptr;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPBusStopSubsystem* Stops = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPBusStopSubsystem>() : nullptr;
    return Stops != nullptr ? Stops->FindStop(RouteData->OrderedStopIds[CurrentStopIndex]) : nullptr;
}

void UTMOPBusServiceComponent::UpdateApproachToStop()
{
    UTMOPBusStopComponent* Stop = GetCurrentTargetStop();
    if (!IsValid(Stop) || !IsValid(Movement))
    {
        ServiceState = ETMOPBusServiceState::InvalidRoute;
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus service '%s': stop index %d could not be resolved."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"), CurrentStopIndex);
        return;
    }
    const FName ConstraintId = GetStopConstraintId(Stop);
    if (Movement->CurrentLaneId != Stop->LaneId)
    {
        Movement->ClearNamedStopConstraint(ConstraintId);
        return;
    }
    const float StopDistance = FMath::Max(0.0f, Stop->DistanceAlongLane - Stop->StopBufferCm);
    Movement->SetNamedStopConstraint(ConstraintId, StopDistance);
    const bool bAtStop = FMath::Abs(Movement->DistanceAlongLane - StopDistance) <= 25.0f &&
        Movement->CurrentSpeedCmPerSecond <= 5.0f;
    if (bAtStop) BeginDwell(Stop);
}

void UTMOPBusServiceComponent::BeginDwell(UTMOPBusStopComponent* Stop)
{
    Movement->StopDriving();
    OpenDoors();
    RemainingDwellSeconds = DwellRandom.FRandRange(
        FMath::Max(0.0f, Stop->MinimumDwellSeconds),
        FMath::Max(Stop->MinimumDwellSeconds, Stop->MaximumDwellSeconds));
    ServiceState = ETMOPBusServiceState::Dwelling;
    UE_LOG(LogTemp, Display,
        TEXT("TMOP bus service '%s': arrived at stop '%s' on '%s' at %.1f cm; dwelling %.1f s."),
        GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
        *Stop->StopId.ToString(), *Stop->LaneId.ToString(),
        Movement->DistanceAlongLane, RemainingDwellSeconds);
    OnArrivedAtStop.Broadcast(Stop->StopId, CurrentStopIndex);
}

void UTMOPBusServiceComponent::FinishDwell(UTMOPBusStopComponent* Stop)
{
    CloseDoors();
    Movement->ClearNamedStopConstraint(GetStopConstraintId(Stop));
    UE_LOG(LogTemp, Display,
        TEXT("TMOP bus service '%s': departing stop '%s'."),
        GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
        *Stop->StopId.ToString());
    OnDepartedStop.Broadcast(Stop->StopId, CurrentStopIndex);
    ++CurrentStopIndex;
    if (!RouteData->OrderedStopIds.IsValidIndex(CurrentStopIndex))
        ServiceState = ETMOPBusServiceState::RouteComplete;
    else ServiceState = ETMOPBusServiceState::DrivingToStop;
    Movement->StartDriving();
}

void UTMOPBusServiceComponent::OpenDoors() { bDoorsOpen = true; }
void UTMOPBusServiceComponent::CloseDoors() { bDoorsOpen = false; }

FName UTMOPBusServiceComponent::GetStopConstraintId(const UTMOPBusStopComponent* Stop) const
{
    return IsValid(Stop)
        ? FName(*FString::Printf(TEXT("BUS_STOP_%s"), *Stop->StopId.ToString())) : NAME_None;
}
