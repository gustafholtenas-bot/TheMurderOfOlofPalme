#include "Traffic/TMOPTrafficVehicleMovementComponent.h"

#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleSubsystem.h"

UTMOPTrafficVehicleMovementComponent::UTMOPTrafficVehicleMovementComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UTMOPTrafficVehicleMovementComponent::BeginPlay()
{
    Super::BeginPlay();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr)
        Traffic->RegisterVehicle(this);
    if (!InitialLaneId.IsNone()) InitializeOnLane(InitialLaneId, 0.0f);
    bDrivingEnabled = bStartDrivingAutomatically;
}

void UTMOPTrafficVehicleMovementComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr)
        Traffic->UnregisterVehicle(this);
    Super::EndPlay(EndPlayReason);
}

bool UTMOPTrafficVehicleMovementComponent::InitializeOnLane(const FName LaneId,
    const float StartDistance)
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    if (Network == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("TMOP traffic vehicle '%s': no TrafficNetworkSubsystem."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"));
        CurrentLaneId = LaneId;
        TrafficState = ETMOPTrafficVehicleState::InvalidLane;
        return false;
    }
    UTMOPTrafficLaneComponent* Lane = Network->FindLane(LaneId);
    if (!IsValid(Lane))
    {
        const int32 Discovered = Network->DiscoverLanesInWorld();
        Lane = Network->FindLane(LaneId);
        UE_LOG(LogTemp, Display, TEXT("TMOP lane retry: discovered %d lanes while looking for '%s'."),
            Discovered, *LaneId.ToString());
    }
    if (!IsValid(Lane))
    {
        CurrentLaneId = LaneId;
        TrafficState = ETMOPTrafficVehicleState::InvalidLane;
        UE_LOG(LogTemp, Error, TEXT("TMOP traffic vehicle '%s': lane '%s' was not found."),
            GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"), *LaneId.ToString());
        return false;
    }
    CurrentLaneId = LaneId;
    DistanceAlongLane = FMath::Clamp(StartDistance, 0.0f, Lane->GetSplineLength());
    PlannedLaneIndex = PlannedLaneIds.IndexOfByKey(LaneId);
    StopConstraints.Reset();
    TrafficState = ETMOPTrafficVehicleState::Stopped;
    ApplyVehicleTransform(Lane);
    UE_LOG(LogTemp, Display, TEXT("TMOP traffic vehicle '%s' initialized on lane '%s' at %.1f cm."),
        GetOwner() != nullptr ? *GetOwner()->GetName() : TEXT("None"), *LaneId.ToString(), DistanceAlongLane);
    return true;
}

void UTMOPTrafficVehicleMovementComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bDrivingEnabled || DeltaTime <= 0.0f) return;
    UTMOPTrafficLaneComponent* Lane = GetCurrentLane();
    if (!IsValid(Lane)) { TrafficState = ETMOPTrafficVehicleState::InvalidLane; return; }

    LaneChangeCooldownSeconds = FMath::Max(0.0f, LaneChangeCooldownSeconds - DeltaTime);
    LaneChangeCheckAccumulator += DeltaTime;
    if (!IsChangingLane() && bAllowLaneChanges && LaneChangeCooldownSeconds <= 0.0f &&
        LaneChangeCheckAccumulator >= LaneChangeCheckIntervalSeconds)
    {
        LaneChangeCheckAccumulator = 0.0f;
        EvaluateAutomaticLaneChange(Lane);
    }
    const float TargetSpeed = CalculateTargetSpeed(Lane);
    const float DeltaSpeed = TargetSpeed - CurrentSpeedCmPerSecond;
    const float MaxChange = (DeltaSpeed >= 0.0f ? AccelerationCmPerSecondSquared
        : ServiceBrakeCmPerSecondSquared) * DeltaTime;
    CurrentSpeedCmPerSecond += FMath::Clamp(DeltaSpeed, -MaxChange, MaxChange);
    CurrentSpeedCmPerSecond = FMath::Max(0.0f, CurrentSpeedCmPerSecond);
    // Remember the active constraint before advancing. At normal frame rates a
    // vehicle can otherwise move from just before a stop to just after it in a
    // single tick. Once the constraint is behind the vehicle it is no longer
    // considered active, so the vehicle would accelerate away without ever
    // reaching the bus service's arrival tolerance.
    const float ActiveStopDistance = GetNearestActiveStopDistance();
    const float PreviousDistanceAlongLane = DistanceAlongLane;
    DistanceAlongLane += CurrentSpeedCmPerSecond * DeltaTime;

    if (ActiveStopDistance >= 0.0f &&
        PreviousDistanceAlongLane <= ActiveStopDistance &&
        DistanceAlongLane >= ActiveStopDistance)
    {
        DistanceAlongLane = ActiveStopDistance;
        CurrentSpeedCmPerSecond = 0.0f;
        TrafficState = ETMOPTrafficVehicleState::Stopped;
    }

    while (DistanceAlongLane >= Lane->GetSplineLength())
    {
        DistanceAlongLane -= Lane->GetSplineLength();
        if (!AdvanceToNextLane(Lane))
        {
            DistanceAlongLane = Lane->GetSplineLength();
            CurrentSpeedCmPerSecond = 0.0f;
            TrafficState = ETMOPTrafficVehicleState::RouteComplete;
            bDrivingEnabled = false;
            break;
        }
        Lane = GetCurrentLane();
        if (!IsValid(Lane)) break;
    }
    if (IsValid(Lane))
    {
        if (IsChangingLane()) UpdateLaneChange(DeltaTime, Lane);
        else ApplyVehicleTransform(Lane);
    }
}

float UTMOPTrafficVehicleMovementComponent::CalculateTargetSpeed(UTMOPTrafficLaneComponent* Lane)
{
    float Target = Lane->GetSpeedLimitCentimetersPerSecond() * FMath::Max(0.0f, SpeedLimitMultiplier);
    TrafficState = ETMOPTrafficVehicleState::Driving;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr;
    float CenterDistance = 0.0f;
    UTMOPTrafficVehicleMovementComponent* Lead = Traffic != nullptr
        ? Traffic->FindLeadVehicle(this, CenterDistance) : nullptr;
    if (IsValid(Lead))
    {
        const float Gap = CenterDistance - 0.5f * (VehicleLengthCm + Lead->VehicleLengthCm);
        const float DesiredGap = MinimumGapCm + CurrentSpeedCmPerSecond * DesiredTimeHeadwaySeconds;
        if (Gap < DesiredGap)
        {
            const float Alpha = FMath::Clamp((Gap - MinimumGapCm) /
                FMath::Max(1.0f, DesiredGap - MinimumGapCm), 0.0f, 1.0f);
            Target = FMath::Min(Target, Lead->CurrentSpeedCmPerSecond * Alpha);
            TrafficState = ETMOPTrafficVehicleState::FollowingVehicle;
        }
    }
    const float PhysicalObstacleDistance = GetPhysicalObstacleDistance();
    if (PhysicalObstacleDistance >= 0.0f)
    {
        const float AvailableBrakingDistance = FMath::Max(0.0f,
            PhysicalObstacleDistance - MinimumGapCm);
        const float SafeSpeed = FMath::Sqrt(2.0f *
            FMath::Max(1.0f, ServiceBrakeCmPerSecondSquared) * AvailableBrakingDistance);
        Target = FMath::Min(Target, SafeSpeed);
        if (SafeSpeed < CurrentSpeedCmPerSecond)
            TrafficState = ETMOPTrafficVehicleState::BrakingForConstraint;
        if (PhysicalObstacleDistance <= MinimumGapCm) Target = 0.0f;
    }
    const float NearestStopDistance = GetNearestActiveStopDistance();
    if (NearestStopDistance >= 0.0f)
    {
        const float Remaining = NearestStopDistance - DistanceAlongLane;
        const float BrakingDistance = FMath::Square(CurrentSpeedCmPerSecond) /
            (2.0f * FMath::Max(1.0f, ServiceBrakeCmPerSecondSquared));
        if (Remaining <= BrakingDistance + MinimumGapCm)
        {
            const float StopAlpha = FMath::Clamp(Remaining / FMath::Max(1.0f, BrakingDistance + MinimumGapCm), 0.0f, 1.0f);
            Target = FMath::Min(Target, Lane->GetSpeedLimitCentimetersPerSecond() * StopAlpha);
            TrafficState = ETMOPTrafficVehicleState::BrakingForConstraint;
        }
        if (Remaining <= 5.0f) Target = 0.0f;
    }
    if (ChooseNextLaneId(Lane).IsNone())
    {
        const float Remaining = Lane->GetSplineLength() - DistanceAlongLane;
        const float BrakingDistance = FMath::Square(CurrentSpeedCmPerSecond) /
            (2.0f * FMath::Max(1.0f, ServiceBrakeCmPerSecondSquared));
        if (Remaining <= BrakingDistance + MinimumGapCm)
        {
            const float StopAlpha = FMath::Clamp(Remaining /
                FMath::Max(1.0f, BrakingDistance + MinimumGapCm), 0.0f, 1.0f);
            Target = FMath::Min(Target, Lane->GetSpeedLimitCentimetersPerSecond() * StopAlpha);
            TrafficState = ETMOPTrafficVehicleState::BrakingForConstraint;
        }
        if (Remaining <= 5.0f) Target = 0.0f;
    }
    if (Target <= 1.0f && CurrentSpeedCmPerSecond <= 1.0f)
        TrafficState = ETMOPTrafficVehicleState::Stopped;
    return FMath::Max(0.0f, Target);
}

bool UTMOPTrafficVehicleMovementComponent::AdvanceToNextLane(UTMOPTrafficLaneComponent* CurrentLane)
{
    const FName NextId = ChooseNextLaneId(CurrentLane);
    if (NextId.IsNone()) return false;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    UTMOPTrafficLaneComponent* Next = Network != nullptr ? Network->FindLane(NextId) : nullptr;
    if (!IsValid(Next)) return false;
    CurrentLaneId = NextId;
    StopConstraints.Reset();
    if (!PlannedLaneIds.IsEmpty()) PlannedLaneIndex = PlannedLaneIds.IndexOfByKey(NextId);
    return true;
}

FName UTMOPTrafficVehicleMovementComponent::ChooseNextLaneId(
    const UTMOPTrafficLaneComponent* CurrentLane) const
{
    if (!IsValid(CurrentLane)) return NAME_None;
    if (PlannedLaneIndex != INDEX_NONE && PlannedLaneIndex + 1 < PlannedLaneIds.Num())
    {
        const FName PlannedNext = PlannedLaneIds[PlannedLaneIndex + 1];
        for (const FTMOPLaneConnection& Connection : CurrentLane->NextLanes)
            if (Connection.bAllowed && Connection.TargetLaneId == PlannedNext) return PlannedNext;
        return NAME_None;
    }
    for (const FTMOPLaneConnection& Connection : CurrentLane->NextLanes)
        if (Connection.bAllowed) return Connection.TargetLaneId;
    return NAME_None;
}

void UTMOPTrafficVehicleMovementComponent::ApplyVehicleTransform(UTMOPTrafficLaneComponent* Lane)
{
    if (!IsValid(Lane) || GetOwner() == nullptr) return;
    const FTransform LaneTransform = Lane->GetLaneTransformAtDistance(DistanceAlongLane);
    const FVector RuntimeOffset = VehicleLocalOffset + FVector(0.0f, AdditionalLateralOffsetCm, 0.0f);
    const FTransform Offset(VehicleRotationOffset, RuntimeOffset, FVector::OneVector);
    GetOwner()->SetActorTransform(Offset * LaneTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

bool UTMOPTrafficVehicleMovementComponent::RequestLaneChange(const FName RequestedLaneId)
{
    if (IsChangingLane() || RequestedLaneId.IsNone() || RequestedLaneId == CurrentLaneId) return false;
    UTMOPTrafficLaneComponent* Source = GetCurrentLane();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    UTMOPTrafficLaneComponent* Target = Network != nullptr ? Network->FindLane(RequestedLaneId) : nullptr;
    if (!IsValid(Source) || !IsValid(Target) || Source->RoadId != Target->RoadId ||
        Source->DirectionId != Target->DirectionId) return false;
    const bool bIsNeighbor = Source->LeftNeighborLaneId == RequestedLaneId ||
        Source->RightNeighborLaneId == RequestedLaneId;
    if (!bIsNeighbor || Source->GetSplineLength() - DistanceAlongLane < MinimumLaneChangeDistanceFromLaneEndCm)
        return false;
    const float Ratio = Source->GetSplineLength() > 1.0f ? DistanceAlongLane / Source->GetSplineLength() : 0.0f;
    const float CandidateDistance = Ratio * Target->GetSplineLength();
    if (!IsTargetLaneSafe(Target, CandidateDistance)) return false;
    TargetLaneId = RequestedLaneId;
    TargetLaneDistance = CandidateDistance;
    LaneChangeElapsedSeconds = 0.0f;
    TrafficState = ETMOPTrafficVehicleState::ChangingLane;
    return true;
}

void UTMOPTrafficVehicleMovementComponent::EvaluateAutomaticLaneChange(UTMOPTrafficLaneComponent* Lane)
{
    if (!IsValid(Lane)) return;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr;
    float LeadDistance = 0.0f;
    UTMOPTrafficVehicleMovementComponent* Lead = Traffic != nullptr
        ? Traffic->FindLeadVehicle(this, LeadDistance) : nullptr;
    const float DesiredGap = MinimumGapCm + CurrentSpeedCmPerSecond * DesiredTimeHeadwaySeconds;
    if (IsValid(Lead) && LeadDistance < DesiredGap * 1.5f && !Lane->LeftNeighborLaneId.IsNone())
    {
        RequestLaneChange(Lane->LeftNeighborLaneId);
        return;
    }
    if (bKeepRightWhenPossible && !Lane->RightNeighborLaneId.IsNone())
        RequestLaneChange(Lane->RightNeighborLaneId);
}

bool UTMOPTrafficVehicleMovementComponent::IsTargetLaneSafe(
    UTMOPTrafficLaneComponent* TargetLane, const float CandidateDistance) const
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr;
    if (!IsValid(TargetLane) || Traffic == nullptr) return false;
    float Ahead = 0.0f;
    float Behind = 0.0f;
    Traffic->FindNearestVehiclesInLane(TargetLane->LaneId, CandidateDistance, this, Ahead, Behind);
    return Ahead >= MinimumTargetLaneFrontGapCm && Behind >= MinimumTargetLaneRearGapCm;
}

void UTMOPTrafficVehicleMovementComponent::UpdateLaneChange(const float DeltaTime,
    UTMOPTrafficLaneComponent* SourceLane)
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    UTMOPTrafficLaneComponent* TargetLane = Network != nullptr ? Network->FindLane(TargetLaneId) : nullptr;
    if (!IsValid(SourceLane) || !IsValid(TargetLane))
    {
        TargetLaneId = NAME_None;
        LaneChangeCooldownSeconds = 2.0f;
        return;
    }
    const float Ratio = SourceLane->GetSplineLength() > 1.0f
        ? DistanceAlongLane / SourceLane->GetSplineLength() : 0.0f;
    TargetLaneDistance = Ratio * TargetLane->GetSplineLength();
    LaneChangeElapsedSeconds += DeltaTime;
    const float Alpha = FMath::Clamp(LaneChangeElapsedSeconds /
        FMath::Max(0.1f, LaneChangeDurationSeconds), 0.0f, 1.0f);
    const float SmoothAlpha = FMath::InterpEaseInOut(0.0f, 1.0f, Alpha, 2.0f);
    const FTransform SourceTransform = SourceLane->GetLaneTransformAtDistance(DistanceAlongLane);
    const FTransform TargetTransform = TargetLane->GetLaneTransformAtDistance(TargetLaneDistance);
    FTransform Blended;
    Blended.Blend(SourceTransform, TargetTransform, SmoothAlpha);
    const FVector RuntimeOffset = VehicleLocalOffset + FVector(0.0f, AdditionalLateralOffsetCm, 0.0f);
    const FTransform Offset(VehicleRotationOffset, RuntimeOffset, FVector::OneVector);
    if (GetOwner() != nullptr)
        GetOwner()->SetActorTransform(Offset * Blended, false, nullptr, ETeleportType::TeleportPhysics);
    TrafficState = ETMOPTrafficVehicleState::ChangingLane;
    if (Alpha >= 1.0f)
    {
        CurrentLaneId = TargetLaneId;
        DistanceAlongLane = TargetLaneDistance;
        TargetLaneId = NAME_None;
        LaneChangeElapsedSeconds = 0.0f;
        LaneChangeCooldownSeconds = 3.0f;
        TrafficState = ETMOPTrafficVehicleState::Driving;
    }
}

void UTMOPTrafficVehicleMovementComponent::StartDriving()
{
    if (IsValid(GetCurrentLane())) bDrivingEnabled = true;
}

void UTMOPTrafficVehicleMovementComponent::StopDriving()
{
    bDrivingEnabled = false;
    CurrentSpeedCmPerSecond = 0.0f;
    TrafficState = ETMOPTrafficVehicleState::Stopped;
}

void UTMOPTrafficVehicleMovementComponent::SetAdditionalLateralOffset(const float OffsetCm)
{
    AdditionalLateralOffsetCm = OffsetCm;
    if (UTMOPTrafficLaneComponent* Lane = GetCurrentLane()) ApplyVehicleTransform(Lane);
}

void UTMOPTrafficVehicleMovementComponent::SetExternalStopDistance(
    const float StopDistanceAlongCurrentLane)
{
    SetNamedStopConstraint(TEXT("LEGACY_EXTERNAL_STOP"), StopDistanceAlongCurrentLane);
}

void UTMOPTrafficVehicleMovementComponent::SetNamedStopConstraint(const FName SourceId,
    const float StopDistanceAlongCurrentLane)
{
    if (SourceId.IsNone()) return;
    if (StopDistanceAlongCurrentLane < 0.0f) StopConstraints.Remove(SourceId);
    else StopConstraints.Add(SourceId, StopDistanceAlongCurrentLane);
}

void UTMOPTrafficVehicleMovementComponent::ClearNamedStopConstraint(const FName SourceId)
{
    StopConstraints.Remove(SourceId);
}

void UTMOPTrafficVehicleMovementComponent::ClearAllStopConstraints()
{
    StopConstraints.Reset();
}

float UTMOPTrafficVehicleMovementComponent::GetNearestActiveStopDistance() const
{
    float Nearest = TNumericLimits<float>::Max();
    bool bFound = false;
    for (const TPair<FName, float>& Constraint : StopConstraints)
    {
        if (Constraint.Value >= DistanceAlongLane && Constraint.Value < Nearest)
        {
            Nearest = Constraint.Value;
            bFound = true;
        }
    }
    return bFound ? Nearest : -1.0f;
}

float UTMOPTrafficVehicleMovementComponent::GetCurrentSpeedKmh() const
{
    return CurrentSpeedCmPerSecond * (3600.0f / 100000.0f);
}

float UTMOPTrafficVehicleMovementComponent::GetPhysicalObstacleDistance() const
{
    UWorld* World = GetWorld();
    const AActor* OwnerActor = GetOwner();
    if (!bDetectPhysicalObstacles || World == nullptr || OwnerActor == nullptr) return -1.0f;

    const FVector Forward = OwnerActor->GetActorForwardVector();
    const FVector Up = OwnerActor->GetActorUpVector();
    const FVector Start = OwnerActor->GetActorLocation() +
        Forward * (VehicleLengthCm * 0.5f + 10.0f) + Up * ObstacleSensorHalfHeightCm;
    const float BrakingDistance = FMath::Square(CurrentSpeedCmPerSecond) /
        (2.0f * FMath::Max(1.0f, ServiceBrakeCmPerSecondSquared));
    const float LookAhead = FMath::Clamp(MinimumGapCm +
        CurrentSpeedCmPerSecond * DesiredTimeHeadwaySeconds + BrakingDistance,
        MinimumObstacleLookAheadCm, MaximumObstacleLookAheadCm);
    const FVector End = Start + Forward * LookAhead;

    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
    ObjectTypes.AddObjectTypesToQuery(ECC_Vehicle);
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(TMOPVehicleObstacleSensor), false,
        OwnerActor);
    TArray<AActor*> AttachedActors;
    OwnerActor->GetAttachedActors(AttachedActors);
    QueryParams.AddIgnoredActors(AttachedActors);

    TArray<FHitResult> Hits;
    const FCollisionShape Shape = FCollisionShape::MakeBox(
        FVector(10.0f, ObstacleSensorHalfWidthCm, ObstacleSensorHalfHeightCm));
    if (!World->SweepMultiByObjectType(Hits, Start, End, OwnerActor->GetActorQuat(),
        ObjectTypes, Shape, QueryParams)) return -1.0f;

    float Nearest = TNumericLimits<float>::Max();
    for (const FHitResult& Hit : Hits)
    {
        if (!IsValid(Hit.GetActor()) || Hit.GetActor() == OwnerActor) continue;
        const float ForwardDistance = FVector::DotProduct(Hit.ImpactPoint - Start, Forward);
        if (ForwardDistance >= 0.0f) Nearest = FMath::Min(Nearest, ForwardDistance);
    }
    return Nearest < TNumericLimits<float>::Max() ? Nearest : -1.0f;
}

UTMOPTrafficLaneComponent* UTMOPTrafficVehicleMovementComponent::GetCurrentLane() const
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    return Network != nullptr ? Network->FindLane(CurrentLaneId) : nullptr;
}
