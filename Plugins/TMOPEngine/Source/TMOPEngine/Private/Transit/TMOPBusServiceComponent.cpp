#include "Transit/TMOPBusServiceComponent.h"

#include "Components/StaticMeshComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Transit/TMOPBusPassengerComponent.h"
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
    DiscoverDoorComponents();
    InitializeService();
}

bool UTMOPBusServiceComponent::InitializeService()
{
    Movement = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>() : nullptr;
    PassengerSystem = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPBusPassengerComponent>() : nullptr;
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
    UpdateDoorAnimation(DeltaTime);
    UpdateStopPullIn(DeltaTime);

    if (bDeparturePending)
    {
        RemainingDepartureDelaySeconds -= DeltaTime;
        if (RemainingDepartureDelaySeconds <= 0.0f)
        {
            bDeparturePending = false;
            if (ServiceState != ETMOPBusServiceState::RouteComplete && IsValid(Movement))
                Movement->StartDriving();
        }
        return;
    }

    if (ServiceState == ETMOPBusServiceState::DrivingToStop) UpdateApproachToStop();
    else if (ServiceState == ETMOPBusServiceState::Dwelling)
    {
        RemainingDwellSeconds -= DeltaTime;
        if (RemainingDwellSeconds <= 0.0f)
        {
            const bool bDoorwayBlocked = bWaitForClearDoorway && IsValid(PassengerSystem) &&
                !PassengerSystem->IsDoorwayClear();
            if (bDoorwayBlocked && DoorwayHoldSeconds < MaximumDoorwayHoldSeconds)
            {
                DoorwayHoldSeconds += DeltaTime;
            }
            else if (UTMOPBusStopComponent* Stop = GetCurrentTargetStop())
            {
                if (bDoorwayBlocked)
                    UE_LOG(LogTemp, Warning,
                        TEXT("TMOP bus service '%s': maximum doorway hold reached; departing with obstruction warning."),
                        GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"));
                FinishDwell(Stop);
            }
        }
    }
}

UTMOPBusStopComponent* UTMOPBusServiceComponent::GetCurrentTargetStop() const
{
    if (!IsValid(RouteData) || !RouteData->OrderedStopIds.IsValidIndex(CurrentStopIndex)) return nullptr;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPBusStopSubsystem* Stops = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPBusStopSubsystem>() : nullptr;
    if (Stops == nullptr) return nullptr;
    const FName StopId = RouteData->OrderedStopIds[CurrentStopIndex];
    UTMOPBusStopComponent* Stop = Stops->FindStop(StopId);
    if (!IsValid(Stop))
    {
        Stops->DiscoverStopsInWorld();
        Stop = Stops->FindStop(StopId);
    }
    return Stop;
}

void UTMOPBusServiceComponent::UpdateApproachToStop()
{
    UTMOPBusStopComponent* Stop = GetCurrentTargetStop();
    if (!IsValid(Movement))
    {
        ServiceState = ETMOPBusServiceState::InvalidRoute;
        return;
    }
    if (!IsValid(Stop))
    {
        Movement->StopDriving();
        if (!bWaitingForStopRegistration)
        {
            bWaitingForStopRegistration = true;
            const FName MissingStopId = IsValid(RouteData) &&
                RouteData->OrderedStopIds.IsValidIndex(CurrentStopIndex)
                ? RouteData->OrderedStopIds[CurrentStopIndex] : NAME_None;
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP bus service '%s': waiting for stop %d '%s' to register; bus held in place."),
                GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
                CurrentStopIndex, *MissingStopId.ToString());
        }
        return;
    }
    if (bWaitingForStopRegistration)
    {
        bWaitingForStopRegistration = false;
        Movement->StartDriving();
        UE_LOG(LogTemp, Display, TEXT("TMOP bus service '%s': stop '%s' is now registered."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
            *Stop->StopId.ToString());
    }

    if (LastDiagnosedStopId != Stop->StopId)
    {
        LastDiagnosedStopId = Stop->StopId;
        const int32 StopLaneIndex = IsValid(RouteData)
            ? RouteData->OrderedLaneIds.IndexOfByKey(Stop->LaneId) : INDEX_NONE;
        UE_LOG(LogTemp, Display,
            TEXT("TMOP bus service '%s': next stop %d '%s', lane '%s', distance %.1f cm, buffer %.1f cm."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"), CurrentStopIndex,
            *Stop->StopId.ToString(), *Stop->LaneId.ToString(),
            Stop->DistanceAlongLane, Stop->StopBufferCm);
        if (StopLaneIndex == INDEX_NONE)
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP bus service '%s': stop '%s' uses lane '%s', which is absent from route '%s'."),
                GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
                *Stop->StopId.ToString(), *Stop->LaneId.ToString(),
                IsValid(RouteData) ? *RouteData->RouteId.ToString() : TEXT("None"));
            Movement->StopDriving();
            ServiceState = ETMOPBusServiceState::InvalidRoute;
            return;
        }
    }
    const FName ConstraintId = GetStopConstraintId(Stop);
    if (Movement->CurrentLaneId != Stop->LaneId)
    {
        Movement->ClearNamedStopConstraint(ConstraintId);
        return;
    }
    const float StopDistance = FMath::Max(0.0f, Stop->DistanceAlongLane - Stop->StopBufferCm);
    if (Movement->DistanceAlongLane > StopDistance + 25.0f)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP bus service '%s': stop '%s' is behind the bus (bus %.1f cm, stop %.1f cm) on lane '%s'. Check Ordered Stop IDs and stop distance."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"),
            *Stop->StopId.ToString(), Movement->DistanceAlongLane, StopDistance,
            *Stop->LaneId.ToString());
        Movement->StopDriving();
        ServiceState = ETMOPBusServiceState::InvalidRoute;
        return;
    }
    Movement->SetNamedStopConstraint(ConstraintId, StopDistance);
    const bool bAtStop = FMath::Abs(Movement->DistanceAlongLane - StopDistance) <= 25.0f &&
        Movement->CurrentSpeedCmPerSecond <= 5.0f;
    if (bAtStop) BeginDwell(Stop);
}

void UTMOPBusServiceComponent::BeginDwell(UTMOPBusStopComponent* Stop)
{
    Movement->StopDriving();
    OpenDoors();
    DoorwayHoldSeconds = 0.0f;
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
    LastDiagnosedStopId = NAME_None;
    if (ServiceState != ETMOPBusServiceState::RouteComplete)
    {
        bDeparturePending = true;
        RemainingDepartureDelaySeconds = bAnimateDoorsAutomatically
            ? FMath::Max(0.05f, DoorAnimationSeconds) : 0.0f;
    }
}

void UTMOPBusServiceComponent::DiscoverDoorComponents()
{
    DoorComponents.Reset();
    DoorClosedLocations.Reset();
    if (!bAnimateDoorsAutomatically || GetOwner() == nullptr) return;

    TArray<UStaticMeshComponent*> MeshComponents;
    GetOwner()->GetComponents<UStaticMeshComponent>(MeshComponents);
    for (UStaticMeshComponent* Mesh : MeshComponents)
    {
        if (!IsValid(Mesh)) continue;
        const FName ComponentName = Mesh->GetFName();
        const bool bExactMatch = !DoorComponentNames.IsEmpty() &&
            DoorComponentNames.Contains(ComponentName);
        const bool bAutomaticMatch = DoorComponentNames.IsEmpty() &&
            ComponentName.ToString().Contains(TEXT("door"), ESearchCase::IgnoreCase);
        if (!bExactMatch && !bAutomaticMatch) continue;

        Mesh->SetMobility(EComponentMobility::Movable);
        DoorComponents.Add(Mesh);
        DoorClosedLocations.Add(Mesh->GetRelativeLocation());
        UE_LOG(LogTemp, Display, TEXT("TMOP bus service '%s': automatic door '%s' discovered."),
            *GetOwner()->GetName(), *ComponentName.ToString());
    }
    if (DoorComponents.IsEmpty())
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP bus service '%s': no door mesh components found. Name them with 'Door' or fill Door Component Names."),
            *GetOwner()->GetName());
}

void UTMOPBusServiceComponent::UpdateDoorAnimation(const float DeltaTime)
{
    if (!bAnimateDoorsAutomatically || DoorComponents.IsEmpty()) return;
    const float TargetAlpha = bDoorsOpen ? 1.0f : 0.0f;
    DoorAnimationAlpha = FMath::FInterpConstantTo(DoorAnimationAlpha, TargetAlpha,
        DeltaTime, 1.0f / FMath::Max(0.05f, DoorAnimationSeconds));
    const int32 Count = FMath::Min(DoorComponents.Num(), DoorClosedLocations.Num());
    for (int32 Index = 0; Index < Count; ++Index)
        if (IsValid(DoorComponents[Index]))
            DoorComponents[Index]->SetRelativeLocation(
                DoorClosedLocations[Index] + DoorOpenOffset * DoorAnimationAlpha);
}

void UTMOPBusServiceComponent::UpdateStopPullIn(const float DeltaTime)
{
    if (!IsValid(Movement)) return;
    float TargetOffset = 0.0f;
    if (bPullIntoStops)
    {
        if (ServiceState == ETMOPBusServiceState::Dwelling || bDeparturePending)
        {
            TargetOffset = StopPullInOffsetCm;
        }
        else if (ServiceState == ETMOPBusServiceState::DrivingToStop)
        {
            if (UTMOPBusStopComponent* Stop = GetCurrentTargetStop();
                IsValid(Stop) && Movement->CurrentLaneId == Stop->LaneId)
            {
                const float StopDistance = FMath::Max(0.0f,
                    Stop->DistanceAlongLane - Stop->StopBufferCm);
                const float Remaining = FMath::Max(0.0f,
                    StopDistance - Movement->DistanceAlongLane);
                const float Alpha = 1.0f - FMath::Clamp(Remaining /
                    FMath::Max(100.0f, StopPullInApproachDistanceCm), 0.0f, 1.0f);
                TargetOffset = StopPullInOffsetCm * Alpha;
            }
        }
    }
    CurrentStopPullInOffsetCm = FMath::FInterpConstantTo(CurrentStopPullInOffsetCm,
        TargetOffset, DeltaTime, FMath::Max(1.0f, StopPullInInterpolationSpeedCmPerSecond));
    Movement->SetAdditionalLateralOffset(CurrentStopPullInOffsetCm);
}

void UTMOPBusServiceComponent::OpenDoors() { bDoorsOpen = true; }
void UTMOPBusServiceComponent::CloseDoors() { bDoorsOpen = false; }

FName UTMOPBusServiceComponent::GetStopConstraintId(const UTMOPBusStopComponent* Stop) const
{
    return IsValid(Stop)
        ? FName(*FString::Printf(TEXT("BUS_STOP_%s"), *Stop->StopId.ToString())) : NAME_None;
}
