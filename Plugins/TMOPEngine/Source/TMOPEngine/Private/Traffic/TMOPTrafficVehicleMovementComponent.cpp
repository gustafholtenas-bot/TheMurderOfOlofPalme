#include "Traffic/TMOPTrafficVehicleMovementComponent.h"

#include "Engine/GameInstance.h"
#include "Engine/DamageEvents.h"
#include "Engine/World.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "Animation/TMOPAnimationTypes.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficVehicleSubsystem.h"
#include "GameFramework/Character.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "Time/TMOPClockSubsystem.h"

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
    EndObstacleBypass();
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    if (UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr)
        Traffic->UnregisterVehicle(this);
    Super::EndPlay(EndPlayReason);
}

bool UTMOPTrafficVehicleMovementComponent::InitializeOnLane(const FName LaneId,
    const float StartDistance)
{
    ClearFinalApproach();
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
    const UGameInstance* ClockInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    const auto* TimelineClock = ClockInstance ? ClockInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (TimedArrivalSecond != INDEX_NONE && TimelineClock &&
        (!TimelineClock->IsClockRunning() || TimelineClock->GetTimeScale() <= 0.0f))
        return;

    if (TimedArrivalSecond != INDEX_NONE &&
        GetCurrentSimulationSecondExact() >=
            static_cast<double>(TimedArrivalSecond))
    {
        ForceCompleteTimedArrival();
        return;
    }

    if (bAnchorManeuverInProgress)
    {
        UpdateAnchorManeuver(DeltaTime);
        return;
    }

    if (bFinalApproachInProgress)
    {
        UpdateFinalApproach(DeltaTime);
        return;
    }

    UTMOPTrafficLaneComponent* Lane = GetCurrentLane();
    if (!IsValid(Lane)) { TrafficState = ETMOPTrafficVehicleState::InvalidLane; return; }

    if (bFleeingVehicle)
        UpdateFleeingVehicleImpacts(DeltaTime);

    if (bHasFinalApproach && CurrentLaneId == FinalApproachLaneId &&
        DistanceAlongLane >= FinalApproachLaneDistanceCm - 1.0f)
    {
        DistanceAlongLane = FinalApproachLaneDistanceCm;
        BeginFinalApproach(Lane);
        UpdateFinalApproach(DeltaTime);
        return;
    }

    UpdateObstacleBypass(DeltaTime);

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
    double TravelCm = CurrentSpeedCmPerSecond * DeltaTime;
    if (TimedDepartureSecond != INDEX_NONE && TimedArrivalSecond > TimedDepartureSecond)
    {
        const double Alpha = FMath::Clamp((GetCurrentSimulationSecondExact() - TimedDepartureSecond) /
            double(TimedArrivalSecond - TimedDepartureSecond), 0.0, 1.0);
        // Catch-up may recover lost progress, but cannot put the car ahead of
        // its scheduled route position or at its final anchor too early.
        TravelCm = FMath::Min(TravelCm, FMath::Max(0.0, ActiveRoutePlan.LengthCm * Alpha - TimedRouteProgressCm));
        TimedRouteProgressCm += TravelCm;
    }
    DistanceAlongLane += float(TravelCm);

    if (bHasFinalApproach && CurrentLaneId == FinalApproachLaneId &&
        PreviousDistanceAlongLane <= FinalApproachLaneDistanceCm &&
        DistanceAlongLane >= FinalApproachLaneDistanceCm)
    {
        DistanceAlongLane = FinalApproachLaneDistanceCm;
        BeginFinalApproach(Lane);
        UpdateFinalApproach(DeltaTime);
        return;
    }

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
            const bool bWaitingForDeadline =
                TimedArrivalSecond != INDEX_NONE &&
                GetCurrentSimulationSecondExact() <
                    static_cast<double>(TimedArrivalSecond);
            TrafficState = bWaitingForDeadline
                ? ETMOPTrafficVehicleState::Stopped
                : ETMOPTrafficVehicleState::RouteComplete;
            bDrivingEnabled = bWaitingForDeadline;
            ApplyVehicleTransform(Lane);
            if (bDespawnAtRouteEnd)
            {
                DespawnAtCompletedRoute();
                return;
            }
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

void UTMOPTrafficVehicleMovementComponent::UpdateObstacleBypass(const float DeltaTime)
{
    AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor)) return;

    if (bObstacleBypassActive)
    {
        ObstacleBypassSecondsRemaining -= DeltaTime;
        if (ObstacleBypassSecondsRemaining <= 0.0f)
            EndObstacleBypass();
        return;
    }

    float DistanceCm = -1.0f;
    AActor* BlockingActor = nullptr;
    const bool bBlocked = GetPhysicalObstacleDiagnostics(DistanceCm, BlockingActor) &&
        IsValid(BlockingActor) && DistanceCm <= MinimumGapCm + 120.0f &&
        CurrentSpeedCmPerSecond <= 5.0f;
    if (!bBlocked)
    {
        PersistentBlockingActor.Reset();
        PersistentBlockSeconds = 0.0f;
        bHornPlayedForCurrentBlock = false;
        return;
    }

    if (PersistentBlockingActor.Get() != BlockingActor)
    {
        PersistentBlockingActor = BlockingActor;
        PersistentBlockSeconds = 0.0f;
        bHornPlayedForCurrentBlock = false;
    }
    PersistentBlockSeconds += DeltaTime;
    TryAutomaticHorn(BlockingActor);
    if (PersistentBlockSeconds >= ObstacleBypassAfterSeconds)
        BeginObstacleBypass(BlockingActor);
}

void UTMOPTrafficVehicleMovementComponent::TryAutomaticHorn(
    AActor* BlockingActor)
{
    if (bFleeingVehicle || !bHonkAtBlockingPawns ||
        bHornPlayedForCurrentBlock ||
        PersistentBlockSeconds < HornAfterBlockedSeconds ||
        !IsValid(Cast<APawn>(BlockingActor)) || GetWorld() == nullptr)
        return;

    const float Now = GetWorld()->GetTimeSeconds();
    if (Now - LastHornWorldSeconds < HornCooldownSeconds) return;
    if (UTMOPVehicleAudioComponent* Audio =
        GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPVehicleAudioComponent>()
        : nullptr)
    {
        Audio->PlayHorn();
        LastHornWorldSeconds = Now;
        bHornPlayedForCurrentBlock = true;
    }
}

void UTMOPTrafficVehicleMovementComponent::UpdateFleeingVehicleImpacts(
    const float DeltaTime)
{
    UWorld* World = GetWorld();
    AActor* OwnerActor = GetOwner();
    if (!IsValid(World) || !IsValid(OwnerActor) ||
        CurrentSpeedCmPerSecond <= 5.0f)
        return;

    const FVector Forward = OwnerActor->GetActorForwardVector();
    const FVector Up = OwnerActor->GetActorUpVector();
    const FVector Start = OwnerActor->GetActorLocation() +
        Forward * (VehicleLengthCm * 0.45f) +
        Up * ObstacleSensorHalfHeightCm;
    const FVector End = Start + Forward * FMath::Max(
        100.0f, CurrentSpeedCmPerSecond * DeltaTime + 75.0f);
    const FCollisionShape Shape = FCollisionShape::MakeBox(FVector(
        35.0f,
        FMath::Max(60.0f, ObstacleSensorHalfWidthCm),
        FMath::Max(70.0f, ObstacleSensorHalfHeightCm)));
    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
    FCollisionQueryParams Query(
        SCENE_QUERY_STAT(TMOPFleeingVehicleImpact), false, OwnerActor);
    TArray<AActor*> AttachedActors;
    OwnerActor->GetAttachedActors(AttachedActors);
    Query.AddIgnoredActors(AttachedActors);

    TArray<FHitResult> Hits;
    if (!World->SweepMultiByObjectType(
        Hits, Start, End, OwnerActor->GetActorQuat(),
        ObjectTypes, Shape, Query))
        return;

    const float Now = World->GetTimeSeconds();
    for (const FHitResult& Hit : Hits)
    {
        ACharacter* Character = Cast<ACharacter>(Hit.GetActor());
        if (!IsValid(Character) || Character->GetAttachParentActor() == OwnerActor)
            continue;
        if (LastFleeingImpactActor.Get() == Character &&
            Now - LastFleeingImpactWorldSeconds < 1.0f)
            continue;

        LastFleeingImpactActor = Character;
        LastFleeingImpactWorldSeconds = Now;
        FDamageEvent DamageEvent(UDamageType::StaticClass());
        Character->TakeDamage(FleeingImpactDamage, DamageEvent,
            OwnerActor->GetInstigatorController(), OwnerActor);
        Character->LaunchCharacter(
            Forward * FleeingImpactLaunchStrength +
                FVector::UpVector * FleeingImpactUpwardStrength,
            true, true);
        if (UTMOPAnimationStateComponent* Animation =
            Character->FindComponentByClass<UTMOPAnimationStateComponent>())
            Animation->TriggerReaction(
                ETMOPAnimReaction::FallingFromHit, 1.25f);
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP fleeing vehicle: '%s' struck '%s' instead of stopping."),
            *OwnerActor->GetName(), *Character->GetName());
    }
}

void UTMOPTrafficVehicleMovementComponent::BeginObstacleBypass(AActor* BlockingActor)
{
    AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor) || !IsValid(BlockingActor)) return;

    ObstacleBypassBaseLateralOffsetCm = AdditionalLateralOffsetCm;
    // Alternate side deterministically so a whole queue does not choose one trace.
    const float Side = (GetTypeHash(OwnerActor->GetFName()) & 1u) == 0u ? 1.0f : -1.0f;
    AdditionalLateralOffsetCm = ObstacleBypassBaseLateralOffsetCm +
        Side * ObstacleBypassLateralOffsetCm;
    bCollisionWasEnabledBeforeBypass = OwnerActor->GetActorEnableCollision();
    OwnerActor->SetActorEnableCollision(false);
    bObstacleBypassActive = true;
    ObstacleBypassSecondsRemaining = ObstacleBypassDurationSeconds;
    PersistentBlockSeconds = 0.0f;
    PersistentBlockingActor.Reset();
    UE_LOG(LogTemp, Warning,
        TEXT("TMOP obstacle bypass: '%s' passes '%s' after %.1f stationary seconds."),
        *OwnerActor->GetName(), *BlockingActor->GetName(), ObstacleBypassAfterSeconds);
}

void UTMOPTrafficVehicleMovementComponent::EndObstacleBypass()
{
    if (!bObstacleBypassActive) return;
    AActor* OwnerActor = GetOwner();
    AdditionalLateralOffsetCm = ObstacleBypassBaseLateralOffsetCm;
    if (IsValid(OwnerActor))
        OwnerActor->SetActorEnableCollision(bCollisionWasEnabledBeforeBypass);
    bObstacleBypassActive = false;
    ObstacleBypassSecondsRemaining = 0.0f;
}

void UTMOPTrafficVehicleMovementComponent::DespawnAtCompletedRoute()
{
    AActor* OwnerActor = GetOwner();
    if (!IsValid(OwnerActor))
    {
        return;
    }

    TArray<AActor*> AttachedActors;
    OwnerActor->GetAttachedActors(AttachedActors, true, true);
    for (AActor* Attached : AttachedActors)
    {
        // Historical occupants leave the simulation with the vehicle. Never
        // destroy a player character merely because they drove to a route end.
        if (ATMOPHistoricalAgent* HistoricalAgent =
            Cast<ATMOPHistoricalAgent>(Attached))
        {
            HistoricalAgent->Destroy();
        }
    }

    UE_LOG(LogTemp, Display,
        TEXT("TMOP traffic vehicle '%s' reached its route exit and despawned."),
        *OwnerActor->GetName());
    OwnerActor->Destroy();
}

float UTMOPTrafficVehicleMovementComponent::CalculateTargetSpeed(UTMOPTrafficLaneComponent* Lane)
{
    float Target = DesiredCruiseSpeedKmh > 0.0f
        ? DesiredCruiseSpeedKmh * (100000.0f / 3600.0f)
        : Lane->GetSpeedLimitCentimetersPerSecond() *
            FMath::Max(0.0f, SpeedLimitMultiplier);
    if (TimedArrivalSecond != INDEX_NONE)
    {
        const double RemainingSeconds =
            static_cast<double>(TimedArrivalSecond) -
            GetCurrentSimulationSecondExact();
        const UGameInstance* TimedGameInstance = GetWorld() != nullptr
            ? GetWorld()->GetGameInstance() : nullptr;
        const UTMOPClockSubsystem* TimedClock = TimedGameInstance != nullptr
            ? TimedGameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
        const float SimulationRate = TimedClock != nullptr
            ? FMath::Max(0.0f, TimedClock->GetTimeScale()) : 1.0f;
        const float RequiredSpeed = RemainingSeconds > KINDA_SMALL_NUMBER
            ? CalculateRemainingRouteDistanceCm() /
                static_cast<float>(RemainingSeconds) * SimulationRate
            : MaximumTimedCatchUpSpeedCmPerSecond;
        Target = FMath::Clamp(
            RequiredSpeed, 0.0f,
            MaximumTimedCatchUpSpeedCmPerSecond * SimulationRate);
    }
    TrafficState = ETMOPTrafficVehicleState::Driving;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficVehicleSubsystem* Traffic = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficVehicleSubsystem>() : nullptr;
    float CenterDistance = 0.0f;
    UTMOPTrafficVehicleMovementComponent* Lead = !bObstacleBypassActive && Traffic != nullptr
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
    const float PhysicalObstacleDistance = bObstacleBypassActive
        ? -1.0f : GetPhysicalObstacleDistance();
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
    if (bHasFinalApproach && !bFinalApproachInProgress &&
        CurrentLaneId == FinalApproachLaneId)
    {
        const float Remaining = FMath::Max(0.0f,
            FinalApproachLaneDistanceCm - DistanceAlongLane);
        const float SafeSpeed = FMath::Sqrt(2.0f *
            FMath::Max(1.0f, ServiceBrakeCmPerSecondSquared) * Remaining);
        Target = FMath::Min(Target, SafeSpeed);
        if (SafeSpeed < CurrentSpeedCmPerSecond)
            TrafficState = ETMOPTrafficVehicleState::BrakingForConstraint;
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
            if ((Connection.bAllowed || bIgnoreOneWayRestrictions) &&
                Connection.TargetLaneId == PlannedNext) return PlannedNext;
        return NAME_None;
    }
    for (const FTMOPLaneConnection& Connection : CurrentLane->NextLanes)
        if (Connection.bAllowed || bIgnoreOneWayRestrictions)
            return Connection.TargetLaneId;
    return NAME_None;
}

void UTMOPTrafficVehicleMovementComponent::ApplyVehicleTransform(UTMOPTrafficLaneComponent* Lane)
{
    if (!IsValid(Lane) || GetOwner() == nullptr) return;
    UpdateVisualSteeringForLane(Lane);
    const FTransform LaneTransform = Lane->GetLaneTransformAtDistance(DistanceAlongLane);
    const FVector RuntimeOffset = VehicleLocalOffset + FVector(0.0f, AdditionalLateralOffsetCm, 0.0f);
    const FTransform Offset(VehicleRotationOffset, RuntimeOffset, FVector::OneVector);
    GetOwner()->SetActorTransform(Offset * LaneTransform, false, nullptr, ETeleportType::TeleportPhysics);
}

void UTMOPTrafficVehicleMovementComponent::UpdateVisualSteeringForLane(
    UTMOPTrafficLaneComponent* Lane)
{
    if (!IsValid(Lane))
    {
        VisualSteeringAngleDegrees = 0.0f;
        return;
    }

    const float Remaining = Lane->GetSplineLength() - DistanceAlongLane;
    const float LookAheadCm = FMath::Min(300.0f, Remaining);
    if (LookAheadCm < 10.0f)
    {
        VisualSteeringAngleDegrees = 0.0f;
        return;
    }

    const float CurrentYaw = Lane->GetLaneTransformAtDistance(
        DistanceAlongLane).Rotator().Yaw;
    const float AheadYaw = Lane->GetLaneTransformAtDistance(
        DistanceAlongLane + LookAheadCm).Rotator().Yaw;
    const float YawDeltaRadians = FMath::DegreesToRadians(
        FMath::FindDeltaAngleDegrees(CurrentYaw, AheadYaw));
    const float ApproximateWheelbaseCm = FMath::Max(100.0f,
        VehicleLengthCm * 0.6f);
    const float SteeringDegrees = FMath::RadiansToDegrees(FMath::Atan(
        ApproximateWheelbaseCm * YawDeltaRadians / LookAheadCm));
    VisualSteeringAngleDegrees = FMath::Clamp(
        SteeringDegrees,
        -MaximumVisualSteeringDegrees,
        MaximumVisualSteeringDegrees);
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
    EndObstacleBypass();
    ClearAnchorManeuver();
    ClearFinalApproach();
    bDrivingEnabled = false;
    CurrentSpeedCmPerSecond = 0.0f;
    VisualSteeringAngleDegrees = 0.0f;
    TrafficState = ETMOPTrafficVehicleState::Stopped;
    TimedArrivalSecond = INDEX_NONE;
    TimedDepartureSecond = INDEX_NONE;
}

void UTMOPTrafficVehicleMovementComponent::ConfigureTimedArrival(
    const int32 ExpectedArrivalSecond,
    const float MaximumCatchUpSpeedKmh)
{
    TimedArrivalSecond = ExpectedArrivalSecond;
    MaximumTimedCatchUpSpeedCmPerSecond = FMath::Max(
        100.0f, MaximumCatchUpSpeedKmh * (100000.0f / 3600.0f));
}

bool UTMOPTrafficVehicleMovementComponent::StartAnchorManeuver(
    const TArray<FTransform>& InAnchorTransforms, const int32 ExpectedArrivalSecond,
    const float CurveStrength, const bool bReverse)
{
    FTMOPVehicleRoutePlan Plan;
    TMOPVehicleRoute::BuildManeuver(InAnchorTransforms, CurveStrength, bReverse,
        ETMOPVehicleManeuverTurn::Automatic, 0.0f, Plan);
    const int32 Now = FMath::FloorToInt(GetCurrentSimulationSecondExact());
    const int32 Arrival = ExpectedArrivalSecond != INDEX_NONE ? ExpectedArrivalSecond :
        Now + FMath::Max(1, FMath::CeilToInt(Plan.LengthCm /
            FMath::Max(100.0f, DesiredCruiseSpeedKmh / 0.036f)));
    return StartRoutePlan(Plan, Now, Arrival, 90.0f, false);
}

bool UTMOPTrafficVehicleMovementComponent::StartRoutePlan(
    const FTMOPVehicleRoutePlan& Plan, int32 Departure, int32 Arrival,
    float CatchUpKmh, bool bSeek, bool bStopAtVia)
{
    if (!GetOwner() || Plan.Samples.Num() < 2 || Arrival <= Departure) return false;
    StopDriving();
    ActiveRoutePlan = Plan;
    if (!bSeek && FVector::Distance(GetOwner()->GetActorLocation(), Plan.Samples[0].GetLocation()) > 300.0)
        return false;
    LastArrivalCorrectionCm = 0.0;
    bLastArrivalBlocked = false;
    LastArrivalBlocker.Reset();
    bManeuverStopAtVia = bStopAtVia;
    const double Now = GetCurrentSimulationSecondExact();
    const double Alpha = bSeek ? FMath::Clamp((Now - Departure) / double(Arrival - Departure), 0.0, 1.0) : 0.0;
    ManeuverProgressCm = TMOPVehicleRoute::DistanceAtTime(Plan, Alpha, bStopAtVia);
    ConfigureTimedArrival(Arrival, CatchUpKmh);
    if (Plan.bAnchorManeuver)
    {
        PlannedLaneIds.Reset(); CurrentLaneId = NAME_None; PlannedLaneIndex = INDEX_NONE;
        AnchorManeuverTransforms = Plan.Anchors;
        AnchorManeuverApproximateLengthCm = float(Plan.LengthCm);
        AnchorManeuverStartSecond = Departure;
        AnchorManeuverDurationSeconds = float(Arrival - Departure);
        bAnchorManeuverInProgress = true;
        bDrivingEnabled = true;
        TrafficState = ETMOPTrafficVehicleState::AnchorManeuver;
        if (bSeek) GetOwner()->SetActorTransform(Plan.Sample(ManeuverProgressCm),
            false, nullptr, ETeleportType::TeleportPhysics);
        else
        {
            // Do not silently teleport a normally starting car onto a remote anchor.
            if (FVector::Distance(GetOwner()->GetActorLocation(), Plan.Samples[0].GetLocation()) > 300.0)
            { StopDriving(); return false; }
            if (!ApplyManeuverPose(Plan.Samples[0])) { StopDriving(); return false; }
        }
    }
    else
    {
        PlannedLaneIds = Plan.LaneIds;
        UGameInstance* Instance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
        auto* Network = Instance ? Instance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
        if (!Network || PlannedLaneIds.IsEmpty()) return false;
        Network->DiscoverLanesInWorld();
        float Remaining = float(ManeuverProgressCm);
        int32 LaneIndex = 0;
        float Coordinate = Plan.StartDistanceCm;
        for (; LaneIndex < PlannedLaneIds.Num(); ++LaneIndex)
        {
            const auto* Lane = Network->FindLane(PlannedLaneIds[LaneIndex]);
            if (!Lane) return false;
            const float Begin = LaneIndex == 0 ? Plan.StartDistanceCm : 0.0f;
            const float End = LaneIndex == PlannedLaneIds.Num() - 1 ? Plan.EndDistanceCm : Lane->GetSplineLength();
            if (Remaining <= End - Begin || LaneIndex == PlannedLaneIds.Num() - 1)
            { Coordinate = FMath::Min(End, Begin + Remaining); break; }
            Remaining -= End - Begin;
        }
        const int32 ChosenIndex = FMath::Min(LaneIndex, PlannedLaneIds.Num() - 1);
        if (!InitializeOnLane(PlannedLaneIds[ChosenIndex], Coordinate)) return false;
        PlannedLaneIndex = ChosenIndex;
        if (Plan.bHasDestination)
            ConfigureFinalApproach(PlannedLaneIds.Last(), Plan.EndDistanceCm, Plan.Destination);
        // Authored routes must not wander onto a neighbor that is not in the plan.
        bAllowLaneChanges = false;
        StartDriving();
        if (bSeek && ChosenIndex == PlannedLaneIds.Num() - 1 &&
            Coordinate >= Plan.EndDistanceCm && Plan.bHasDestination)
        {
            BeginFinalApproach(Network->FindLane(PlannedLaneIds.Last()));
            FinalApproachStartTransform = Plan.Sample(ManeuverProgressCm);
            GetOwner()->SetActorTransform(FinalApproachStartTransform, false, nullptr, ETeleportType::TeleportPhysics);
            FinalApproachStartSecond = Now;
            FinalApproachDurationSeconds = FMath::Max(0.01f, float(Arrival - Now));
        }
    }
    ConfigureTimedArrival(Arrival, CatchUpKmh);
    TimedDepartureSecond = Departure;
    TimedRouteProgressCm = ManeuverProgressCm;
    return true;
}

bool UTMOPTrafficVehicleMovementComponent::ApplyManeuverPose(const FTransform& Pose)
{
    AActor* VehicleActor = GetOwner();
    if (!VehicleActor) return false;
    FTMOPVehicleRoutePlan Step;
    Step.AddSample(VehicleActor->GetActorTransform()); Step.AddSample(Pose);
    FHitResult Hit;
    if (TMOPVehicleRoute::FindObstacle(GetWorld(), Step,
        FVector(VehicleLengthCm * 0.45f, ObstacleSensorHalfWidthCm, 50.0f), Hit, VehicleActor))
    {
        bLastArrivalBlocked = true;
        LastArrivalBlocker = GetNameSafe(Hit.GetActor());
        CurrentSpeedCmPerSecond = 0.0f;
        return false;
    }
    FTransform ScaledPose = Pose;
    ScaledPose.SetScale3D(VehicleActor->GetActorScale3D());
    VehicleActor->SetActorTransform(ScaledPose, true, &Hit, ETeleportType::TeleportPhysics);
    if (Hit.bBlockingHit)
    {
        bLastArrivalBlocked = true;
        LastArrivalBlocker = GetNameSafe(Hit.GetActor());
        CurrentSpeedCmPerSecond = 0.0f;
        return false;
    }
    bLastArrivalBlocked = false;
    LastArrivalBlocker.Reset();
    return true;
}

double UTMOPTrafficVehicleMovementComponent::GetCurrentSimulationSecondExact() const
{
    const UGameInstance* GameInstance = GetWorld() != nullptr
        ? GetWorld()->GetGameInstance() : nullptr;
    const UTMOPClockSubsystem* Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    return Clock != nullptr ? Clock->GetCurrentTimeSecondsExact() : 0.0;
}

float UTMOPTrafficVehicleMovementComponent::CalculateRemainingRouteDistanceCm() const
{
    if (bAnchorManeuverInProgress)
    {
        float Alpha = AnchorManeuverElapsedSeconds /
            FMath::Max(0.1f, AnchorManeuverDurationSeconds);
        if (TimedArrivalSecond != INDEX_NONE)
        {
            const double Total = static_cast<double>(TimedArrivalSecond) -
                AnchorManeuverStartSecond;
            Alpha = Total > 0.0
                ? static_cast<float>((GetCurrentSimulationSecondExact() -
                    AnchorManeuverStartSecond) / Total)
                : 1.0f;
        }
        return AnchorManeuverApproximateLengthCm *
            (1.0f - FMath::Clamp(Alpha, 0.0f, 1.0f));
    }
    const UTMOPTrafficLaneComponent* Lane = GetCurrentLane();
    if (!IsValid(Lane)) return 0.0f;
    float Remaining = FMath::Max(0.0f,
        (bHasFinalApproach && CurrentLaneId == FinalApproachLaneId
            ? FinalApproachLaneDistanceCm : Lane->GetSplineLength()) -
        DistanceAlongLane);
    UGameInstance* GameInstance = GetWorld() != nullptr
        ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    for (int32 Index = PlannedLaneIndex + 1;
        Network != nullptr && Index < PlannedLaneIds.Num(); ++Index)
    {
        const UTMOPTrafficLaneComponent* RouteLane =
            Network->FindLane(PlannedLaneIds[Index]);
        if (IsValid(RouteLane))
            Remaining += Index == PlannedLaneIds.Num() - 1 &&
                    bHasFinalApproach &&
                    PlannedLaneIds[Index] == FinalApproachLaneId
                ? FinalApproachLaneDistanceCm
                : RouteLane->GetSplineLength();
    }
    if (bHasFinalApproach && Network != nullptr)
    {
        const UTMOPTrafficLaneComponent* FinalLane =
            Network->FindLane(FinalApproachLaneId);
        if (IsValid(FinalLane))
            Remaining += FVector::Dist2D(
                FinalLane->GetLaneTransformAtDistance(
                    FinalApproachLaneDistanceCm).GetLocation(),
                FinalApproachTargetTransform.GetLocation());
    }
    return Remaining;
}

bool UTMOPTrafficVehicleMovementComponent::ForceCompleteTimedArrival()
{
    if (TimedArrivalSecond == INDEX_NONE || !GetOwner()) return false;
    FTransform Target = GetOwner()->GetActorTransform();
    if (bAnchorManeuverInProgress && !ActiveRoutePlan.Samples.IsEmpty()) Target = ActiveRoutePlan.Destination;
    else if (bHasFinalApproach) Target = FinalApproachTargetTransform;
    else if (auto* Lane = GetCurrentLane()) Target = Lane->GetLaneTransformAtDistance(Lane->GetSplineLength());
    LastArrivalCorrectionCm = FVector::Distance(GetOwner()->GetActorLocation(), Target.GetLocation());
    bool bApplied = true;
    if (bAnchorManeuverInProgress)
    {
        // Sweep along the curve, never cut straight through an obstacle at the deadline.
        while (ManeuverProgressCm < ActiveRoutePlan.LengthCm - 0.01)
        {
            const double Next = FMath::Min(ActiveRoutePlan.LengthCm, ManeuverProgressCm + 30.0);
            if (!ApplyManeuverPose(ActiveRoutePlan.Sample(Next))) { bApplied = false; break; }
            ManeuverProgressCm = Next;
        }
    }
    else bApplied = ApplyManeuverPose(Target);
    ClearAnchorManeuver(); ClearFinalApproach();
    TimedArrivalSecond = INDEX_NONE;
    bDrivingEnabled = false; CurrentSpeedCmPerSecond = 0.0f; VisualSteeringAngleDegrees = 0.0f;
    TrafficState = bApplied ? ETMOPTrafficVehicleState::RouteComplete : ETMOPTrafficVehicleState::Stopped;
    return bApplied;
}

FTransform UTMOPTrafficVehicleMovementComponent::EvaluateAnchorManeuver(const float Alpha) const
{
    return ActiveRoutePlan.Sample(TMOPVehicleRoute::DistanceAtTime(ActiveRoutePlan, Alpha, bManeuverStopAtVia));
}

void UTMOPTrafficVehicleMovementComponent::UpdateAnchorManeuver(const float DeltaTime)
{
    if (!bAnchorManeuverInProgress || !GetOwner()) return;
    const double Alpha = FMath::Clamp((GetCurrentSimulationSecondExact() - AnchorManeuverStartSecond) /
        FMath::Max(0.1, double(AnchorManeuverDurationSeconds)), 0.0, 1.0);
    const double Desired = TMOPVehicleRoute::DistanceAtTime(ActiveRoutePlan, Alpha, bManeuverStopAtVia);
    const double Before = ManeuverProgressCm;
    const auto* Instance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr;
    const auto* Clock = Instance ? Instance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    const double TimeScale = Clock ? Clock->GetTimeScale() : 1.0;
    const double Limit = FMath::Min(Desired, Before + MaximumTimedCatchUpSpeedCmPerSecond * DeltaTime * TimeScale);
    while (ManeuverProgressCm < Limit - 0.01)
    {
        const double Next = FMath::Min(Limit, ManeuverProgressCm + 30.0);
        if (!ApplyManeuverPose(ActiveRoutePlan.Sample(Next))) break;
        ManeuverProgressCm = Next;
    }
    CurrentSpeedCmPerSecond = DeltaTime > 0.0f ? float((ManeuverProgressCm - Before) / DeltaTime) : 0.0f;
    TrafficState = bLastArrivalBlocked ? ETMOPTrafficVehicleState::Stopped : ETMOPTrafficVehicleState::AnchorManeuver;
}

void UTMOPTrafficVehicleMovementComponent::ClearAnchorManeuver()
{
    bAnchorManeuverInProgress = false;
    bAnchorManeuverReverse = false;
    AnchorManeuverTransforms.Reset();
    AnchorManeuverSegmentWeights.Reset();
    AnchorManeuverElapsedSeconds = 0.0f;
    AnchorManeuverApproximateLengthCm = 0.0f;
}

void UTMOPTrafficVehicleMovementComponent::ConfigureFinalApproach(
    const FName FinalLaneId,
    const float FinalLaneDistanceCm,
    const FTransform& TargetTransform)
{
    UGameInstance* GameInstance = GetWorld() != nullptr
        ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    UTMOPTrafficLaneComponent* Lane = Network != nullptr
        ? Network->FindLane(FinalLaneId) : nullptr;
    if (FinalLaneId.IsNone() || !IsValid(Lane))
    {
        ClearFinalApproach();
        return;
    }

    bHasFinalApproach = true;
    bFinalApproachInProgress = false;
    FinalApproachLaneId = FinalLaneId;
    FinalApproachLaneDistanceCm = FMath::Clamp(
        FinalLaneDistanceCm, 0.0f, Lane->GetSplineLength());
    FinalApproachTargetTransform = TargetTransform;
    FinalApproachElapsedSeconds = 0.0f;
}

bool UTMOPTrafficVehicleMovementComponent::UpdateFinalApproachTarget(
    const FTransform& TargetTransform)
{
    if (!bHasFinalApproach) return false;
    FinalApproachTargetTransform = TargetTransform;
    return true;
}

void UTMOPTrafficVehicleMovementComponent::BeginFinalApproach(
    UTMOPTrafficLaneComponent* Lane)
{
    if (!bHasFinalApproach || bFinalApproachInProgress ||
        !IsValid(Lane) || GetOwner() == nullptr)
        return;

    ApplyVehicleTransform(Lane);
    FinalApproachStartTransform = GetOwner()->GetActorTransform();
    const FVector LocalTarget = FinalApproachStartTransform.InverseTransformPosition(
        FinalApproachTargetTransform.GetLocation());
    VisualSteeringAngleDegrees = FMath::Clamp(
        FMath::RadiansToDegrees(FMath::Atan2(LocalTarget.Y, LocalTarget.X)),
        -MaximumVisualSteeringDegrees,
        MaximumVisualSteeringDegrees);
    const float Distance = FVector::Distance(
        FinalApproachStartTransform.GetLocation(),
        FinalApproachTargetTransform.GetLocation());
    FinalApproachDurationSeconds = FMath::Clamp(
        Distance / FMath::Max(50.0f, FinalApproachSpeedCmPerSecond),
        MinimumFinalApproachDurationSeconds,
        MaximumFinalApproachDurationSeconds);
    FinalApproachStartSecond = GetCurrentSimulationSecondExact();
    if (TimedArrivalSecond != INDEX_NONE)
        FinalApproachDurationSeconds = FMath::Max(0.01f,
            float(TimedArrivalSecond - FinalApproachStartSecond));
    FinalApproachElapsedSeconds = 0.0f;
    CurrentSpeedCmPerSecond = Distance /
        FMath::Max(0.1f, FinalApproachDurationSeconds);
    bFinalApproachInProgress = true;
    TrafficState = ETMOPTrafficVehicleState::FinalApproach;
}

void UTMOPTrafficVehicleMovementComponent::UpdateFinalApproach(
    const float DeltaTime)
{
    AActor* OwnerActor = GetOwner();
    if (!bFinalApproachInProgress || !IsValid(OwnerActor)) return;

    if (TimedArrivalSecond != INDEX_NONE)
        FinalApproachElapsedSeconds = float(GetCurrentSimulationSecondExact() - FinalApproachStartSecond);
    else FinalApproachElapsedSeconds += DeltaTime;
    const float LinearAlpha = FMath::Clamp(
        FinalApproachElapsedSeconds /
            FMath::Max(0.1f, FinalApproachDurationSeconds),
        0.0f, 1.0f);
    const float SmoothAlpha = FMath::SmoothStep(0.0f, 1.0f, LinearAlpha);
    FTransform BlendedTransform;
    BlendedTransform.Blend(
        FinalApproachStartTransform,
        FinalApproachTargetTransform,
        SmoothAlpha);
    if (!ApplyManeuverPose(BlendedTransform)) return;
    TrafficState = ETMOPTrafficVehicleState::FinalApproach;

    if (LinearAlpha >= 1.0f)
    {
        OwnerActor->SetActorTransform(
            FinalApproachTargetTransform,
            false,
            nullptr,
            ETeleportType::TeleportPhysics);
        bFinalApproachInProgress = false;
        bHasFinalApproach = false;
        bDrivingEnabled = false;
        CurrentSpeedCmPerSecond = 0.0f;
        VisualSteeringAngleDegrees = 0.0f;
        TrafficState = ETMOPTrafficVehicleState::RouteComplete;
        TimedArrivalSecond = INDEX_NONE;
    }
}

void UTMOPTrafficVehicleMovementComponent::ClearFinalApproach()
{
    bHasFinalApproach = false;
    bFinalApproachInProgress = false;
    FinalApproachLaneId = NAME_None;
    FinalApproachLaneDistanceCm = 0.0f;
    FinalApproachElapsedSeconds = 0.0f;
    FinalApproachDurationSeconds = 1.0f;
}

bool UTMOPTrafficVehicleMovementComponent::RestoreBakedTrafficState(
    const FName LaneId,
    const float DistanceCm,
    const float SpeedCmPerSecond,
    const TArray<FName>& RouteLaneIds,
    const bool bShouldDrive)
{
    PlannedLaneIds = RouteLaneIds;
    InitialLaneId = LaneId;
    if (!InitializeOnLane(LaneId, DistanceCm))
    {
        return false;
    }
    CurrentSpeedCmPerSecond = FMath::Max(0.0f, SpeedCmPerSecond);
    bDrivingEnabled = bShouldDrive;
    TrafficState = bShouldDrive
        ? ETMOPTrafficVehicleState::Driving
        : ETMOPTrafficVehicleState::Stopped;
    return true;
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
    float DistanceCm = -1.0f;
    AActor* BlockingActor = nullptr;
    GetPhysicalObstacleDiagnostics(DistanceCm, BlockingActor);
    return DistanceCm;
}

bool UTMOPTrafficVehicleMovementComponent::GetPhysicalObstacleDiagnostics(
    float& OutDistanceCm,
    AActor*& OutBlockingActor) const
{
    OutDistanceCm = -1.0f;
    OutBlockingActor = nullptr;
    UWorld* World = GetWorld();
    const AActor* OwnerActor = GetOwner();
    if (bFleeingVehicle || !bDetectPhysicalObstacles ||
        World == nullptr || OwnerActor == nullptr)
        return false;

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
        ObjectTypes, Shape, QueryParams)) return false;

    float Nearest = TNumericLimits<float>::Max();
    for (const FHitResult& Hit : Hits)
    {
        if (!IsValid(Hit.GetActor()) || Hit.GetActor() == OwnerActor) continue;
        const float ForwardDistance = FVector::DotProduct(Hit.ImpactPoint - Start, Forward);
        if (ForwardDistance >= 0.0f && ForwardDistance < Nearest)
        {
            Nearest = ForwardDistance;
            OutBlockingActor = Hit.GetActor();
        }
    }
    if (Nearest >= TNumericLimits<float>::Max()) return false;
    OutDistanceCm = Nearest;
    return true;
}

UTMOPTrafficLaneComponent* UTMOPTrafficVehicleMovementComponent::GetCurrentLane() const
{
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPTrafficNetworkSubsystem* Network = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    return Network != nullptr ? Network->FindLane(CurrentLaneId) : nullptr;
}
