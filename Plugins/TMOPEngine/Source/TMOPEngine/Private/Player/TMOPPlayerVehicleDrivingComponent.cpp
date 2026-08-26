#include "Player/TMOPPlayerVehicleDrivingComponent.h"

#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "Components/BoxComponent.h"

UTMOPPlayerVehicleDrivingComponent::UTMOPPlayerVehicleDrivingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

bool UTMOPPlayerVehicleDrivingComponent::BeginDriving(ATMOPVehicleBase* Vehicle)
{
    if (!IsValid(Vehicle)) return false;
    if (DrivenVehicle == Vehicle) return true;
    if (IsDriving()) EndDriving();

    DrivenVehicle = Vehicle;
    SuspendedTrafficMovement = Vehicle->FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    if (SuspendedTrafficMovement.IsValid())
    {
        CurrentSpeedCmPerSecond = SuspendedTrafficMovement->CurrentSpeedCmPerSecond;
        SuspendedTrafficMovement->StopDriving();
        SuspendedTrafficMovement->SetComponentTickEnabled(false);
    }
    else
    {
        CurrentSpeedCmPerSecond = 0.0f;
    }

    ThrottleInput = SteeringInput = BrakeInput = 0.0f;
    bHandbrakeInput = false;
    bHighSpeedMode = false;
    CurrentYawRateDegreesPerSecond = 0.0f;
    bHasGroundContact = false;
    SetComponentTickEnabled(true);
    OnPlayerDrivingStateChanged.Broadcast(DrivenVehicle, true);
    return true;
}

void UTMOPPlayerVehicleDrivingComponent::EndDriving()
{
    ATMOPVehicleBase* PreviousVehicle = DrivenVehicle;
    if (SuspendedTrafficMovement.IsValid())
    {
        SuspendedTrafficMovement->CurrentSpeedCmPerSecond = FMath::Abs(CurrentSpeedCmPerSecond);
        SuspendedTrafficMovement->SetComponentTickEnabled(true);
        SuspendedTrafficMovement->StopDriving();
    }
    SuspendedTrafficMovement.Reset();
    DrivenVehicle = nullptr;
    CurrentSpeedCmPerSecond = 0.0f;
    ThrottleInput = SteeringInput = BrakeInput = 0.0f;
    bHandbrakeInput = false;
    bHighSpeedMode = false;
    CurrentYawRateDegreesPerSecond = 0.0f;
    bHasGroundContact = false;
    SetComponentTickEnabled(false);
    if (IsValid(PreviousVehicle)) OnPlayerDrivingStateChanged.Broadcast(PreviousVehicle, false);
}

void UTMOPPlayerVehicleDrivingComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!IsValid(DrivenVehicle) || DeltaTime <= 0.0f) return;

    const float MaxForwardKmh = bHighSpeedMode
        ? MaximumForwardSpeedKmh
        : FMath::Min(NormalForwardSpeedKmh, MaximumForwardSpeedKmh);
    const float MaxForward = MaxForwardKmh * (100000.0f / 3600.0f);
    const float AbsoluteMaxForward = MaximumForwardSpeedKmh * (100000.0f / 3600.0f);
    const float MaxReverse = MaximumReverseSpeedKmh * (100000.0f / 3600.0f);
    float EffectiveThrottle = ThrottleInput;
    float EffectiveBrake = BrakeInput;
    const bool bOpposingForward = CurrentSpeedCmPerSecond >
        DirectionChangeSpeedThresholdCmPerSecond && ThrottleInput < 0.0f;
    const bool bOpposingReverse = CurrentSpeedCmPerSecond <
        -DirectionChangeSpeedThresholdCmPerSecond && ThrottleInput > 0.0f;
    if (bOpposingForward || bOpposingReverse)
    {
        EffectiveBrake = FMath::Max(EffectiveBrake, FMath::Abs(ThrottleInput));
        EffectiveThrottle = 0.0f;
    }
    const float DesiredSpeed = EffectiveThrottle >= 0.0f
        ? EffectiveThrottle * MaxForward : EffectiveThrottle * MaxReverse;
    const bool bAccelerating = !FMath::IsNearlyZero(EffectiveThrottle);
    const float Deceleration = bHandbrakeInput ? HandbrakeDecelerationCmPerSecondSquared
        : (EffectiveBrake > KINDA_SMALL_NUMBER ? BrakeDecelerationCmPerSecondSquared * EffectiveBrake
            : CoastingDecelerationCmPerSecondSquared);
    const float Rate = bAccelerating ? EngineAccelerationCmPerSecondSquared : Deceleration;
    CurrentSpeedCmPerSecond = FMath::FInterpConstantTo(CurrentSpeedCmPerSecond,
        bAccelerating ? DesiredSpeed : 0.0f, DeltaTime, Rate);
    if (EffectiveBrake > KINDA_SMALL_NUMBER || bHandbrakeInput)
        CurrentSpeedCmPerSecond = FMath::FInterpConstantTo(CurrentSpeedCmPerSecond,
            0.0f, DeltaTime, Deceleration);

    VisualSteeringAngleDegrees = FMath::FInterpTo(VisualSteeringAngleDegrees,
        SteeringInput * MaximumSteeringDegrees, DeltaTime, SteeringResponse);
    if (SuspendedTrafficMovement.IsValid())
    {
        // The configured vehicle reads its wheel visuals from the shared
        // traffic component even while player driving has suspended its tick.
        SuspendedTrafficMovement->CurrentSpeedCmPerSecond =
            CurrentSpeedCmPerSecond;
        SuspendedTrafficMovement->VisualSteeringAngleDegrees =
            VisualSteeringAngleDegrees;
    }
    const float SpeedAlpha = FMath::Clamp(FMath::Abs(CurrentSpeedCmPerSecond) /
        FMath::Max(1.0f, AbsoluteMaxForward), 0.0f, 1.0f);
    const float EffectiveSteering = FMath::DegreesToRadians(VisualSteeringAngleDegrees) *
        FMath::Lerp(1.0f, 0.55f, SpeedAlpha);
    const float DesiredYawRateDegrees = MinimumTurnRadiusCm > 1.0f
        ? FMath::RadiansToDegrees(
            (CurrentSpeedCmPerSecond / MinimumTurnRadiusCm) * FMath::Sin(EffectiveSteering))
        : 0.0f;
    CurrentYawRateDegreesPerSecond = FMath::FInterpTo(
        CurrentYawRateDegreesPerSecond, DesiredYawRateDegrees,
        DeltaTime, YawInertiaResponse);

    FRotator Rotation = DrivenVehicle->GetActorRotation();
    Rotation.Yaw += CurrentYawRateDegreesPerSecond * DeltaTime;
    const FVector DeltaLocation = Rotation.Vector() * CurrentSpeedCmPerSecond * DeltaTime;
    FVector DesiredLocation = DrivenVehicle->GetActorLocation() + DeltaLocation;
    if (bFollowGround && GetWorld() != nullptr)
    {
        float RootHalfLength = MinimumProbeLongitudinalOffsetCm;
        float RootHalfWidth = MinimumProbeLateralOffsetCm;
        float RootHalfHeight = 0.0f;
        if (const UBoxComponent* RootBox = Cast<UBoxComponent>(
            DrivenVehicle->GetRootComponent()))
        {
            const FVector Extent = RootBox->GetScaledBoxExtent();
            RootHalfLength = FMath::Max(RootHalfLength, Extent.X * ProbeExtentScale);
            RootHalfWidth = FMath::Max(RootHalfWidth, Extent.Y * ProbeExtentScale);
            RootHalfHeight = Extent.Z;
        }

        const FVector Forward = Rotation.Vector().GetSafeNormal2D();
        const FVector Right = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y).GetSafeNormal2D();
        const TArray<FVector> ProbeOffsets = {
            FVector::ZeroVector,
            Forward * RootHalfLength,
            -Forward * RootHalfLength,
            Right * RootHalfWidth,
            -Right * RootHalfWidth
        };

        TArray<FHitResult> AcceptedHits;
        FCollisionQueryParams GroundParams(
            SCENE_QUERY_STAT(TMOPVehicleGround), false, DrivenVehicle.Get());
        GroundParams.AddIgnoredActor(DrivenVehicle.Get());

        for (const FVector& Offset : ProbeOffsets)
        {
            TArray<FHitResult> ProbeHits;
            const FVector ProbePosition = DesiredLocation + Offset;
            const FVector TraceStart =
                ProbePosition + FVector(0.0f, 0.0f, GroundTraceUpCm);
            const FVector TraceEnd =
                ProbePosition - FVector(0.0f, 0.0f, GroundTraceDownCm);
            GetWorld()->LineTraceMultiByChannel(
                ProbeHits, TraceStart, TraceEnd, ECC_Visibility, GroundParams);

            for (const FHitResult& Candidate : ProbeHits)
            {
                if (Candidate.ImpactNormal.Z < MinimumGroundNormalZ) continue;
                if (bHasGroundContact &&
                    Candidate.ImpactPoint.Z >
                        LastGroundHeightCm + MaximumStepUpHeightCm)
                    continue;
                AcceptedHits.Add(Candidate);
                break;
            }
        }

        if (!AcceptedHits.IsEmpty())
        {
            float TargetGroundHeight = 0.0f;
            FVector AverageNormal = FVector::ZeroVector;
            for (const FHitResult& GroundHit : AcceptedHits)
            {
                TargetGroundHeight += GroundHit.ImpactPoint.Z;
                AverageNormal += GroundHit.ImpactNormal;
            }
            TargetGroundHeight /= static_cast<float>(AcceptedHits.Num());
            AverageNormal = AverageNormal.GetSafeNormal();

            if (bHasGroundContact)
            {
                const float MaxRiseThisFrame =
                    StepUpSpeedCmPerSecond * DeltaTime;
                TargetGroundHeight = FMath::Min(
                    TargetGroundHeight,
                    LastGroundHeightCm + MaxRiseThisFrame);
            }

            const float TargetVehicleZ =
                TargetGroundHeight + RootHalfHeight + GroundClearanceCm;
            DesiredLocation.Z = bHasGroundContact
                ? FMath::FInterpTo(DrivenVehicle->GetActorLocation().Z,
                    TargetVehicleZ, DeltaTime, GroundHeightResponse)
                : TargetVehicleZ;
            LastGroundHeightCm = TargetGroundHeight;
            bHasGroundContact = true;

            if (bAlignToGroundNormal && !AverageNormal.IsNearlyZero())
            {
                const FVector GroundForward = FVector::VectorPlaneProject(
                    Forward, AverageNormal).GetSafeNormal();
                if (!GroundForward.IsNearlyZero())
                {
                    const FRotator GroundRotation = FRotationMatrix::MakeFromXZ(
                        GroundForward, AverageNormal).Rotator();
                    Rotation = FMath::RInterpTo(Rotation, GroundRotation,
                        DeltaTime, GroundRotationResponse);
                }
            }
        }
    }

    const FVector MovementStart = DrivenVehicle->GetActorLocation();
    const FRotator RotationStart = DrivenVehicle->GetActorRotation();
    const float ImpactSpeedCmPerSecond = FMath::Abs(CurrentSpeedCmPerSecond);
    FHitResult Hit;
    const bool bMoved = DrivenVehicle->SetActorLocationAndRotation(
        DesiredLocation, Rotation, bSweepMovement,
        bSweepMovement ? &Hit : nullptr, ETeleportType::None);
    if (bSweepMovement && (!bMoved || Hit.bBlockingHit))
    {
        // A box sweep into a kerb blocks before the forward ground probe can
        // pull the vehicle upward. Retry as three distinct moves: restore the
        // valid start, lift vertically, then sweep forward at the raised
        // height. This behaves like a wheel climbing a low kerb instead of a
        // diagonal body teleport.
        DrivenVehicle->SetActorLocationAndRotation(
            MovementStart, RotationStart, false, nullptr, ETeleportType::TeleportPhysics);
        bool bSteppedUp = false;
        if (MaximumStepUpHeightCm > 0.0f &&
            FVector::DistSquared2D(MovementStart, DesiredLocation) > 1.0f)
        {
            const FVector RaisedStart = MovementStart +
                FVector(0.0f, 0.0f, MaximumStepUpHeightCm);
            FHitResult LiftHit;
            const bool bLifted = DrivenVehicle->SetActorLocationAndRotation(
                RaisedStart, RotationStart, true, &LiftHit, ETeleportType::None) &&
                !LiftHit.bBlockingHit;
            if (bLifted)
            {
                FVector RaisedDestination = DesiredLocation;
                RaisedDestination.Z = FMath::Max(
                    RaisedDestination.Z, RaisedStart.Z);
                FHitResult ForwardHit;
                bSteppedUp = DrivenVehicle->SetActorLocationAndRotation(
                    RaisedDestination, Rotation, true, &ForwardHit,
                    ETeleportType::None) && !ForwardHit.bBlockingHit;
            }
        }
        if (!bSteppedUp)
        {
            DrivenVehicle->SetActorLocationAndRotation(
                MovementStart, RotationStart, false, nullptr,
                ETeleportType::TeleportPhysics);
            CurrentSpeedCmPerSecond = 0.0f;
            CurrentYawRateDegreesPerSecond *= 0.35f;

            const float WorldSeconds = GetWorld() != nullptr
                ? GetWorld()->GetTimeSeconds() : 0.0f;
            if (WorldSeconds - LastCollisionSoundWorldSeconds >= 0.35f)
                if (UTMOPVehicleAudioComponent* Audio =
                    DrivenVehicle->FindComponentByClass<UTMOPVehicleAudioComponent>())
                {
                    Audio->PlayCollision(ImpactSpeedCmPerSecond);
                    LastCollisionSoundWorldSeconds = WorldSeconds;
                }
        }
    }

    const float WheelRollDegrees = FMath::RadiansToDegrees(
        CurrentSpeedCmPerSecond * DeltaTime / FMath::Max(1.0f, VisualWheelRadiusCm));
    VisualWheelRotationDegrees = FMath::Fmod(VisualWheelRotationDegrees + WheelRollDegrees, 360.0f);
}

void UTMOPPlayerVehicleDrivingComponent::SetThrottleInput(const float Value)
{
    ThrottleInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void UTMOPPlayerVehicleDrivingComponent::SetSteeringInput(const float Value)
{
    SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f);
}

void UTMOPPlayerVehicleDrivingComponent::SetBrakeInput(const float Value)
{
    BrakeInput = FMath::Clamp(Value, 0.0f, 1.0f);
}

void UTMOPPlayerVehicleDrivingComponent::SetHandbrakeInput(const bool bPressed)
{
    bHandbrakeInput = bPressed;
}

void UTMOPPlayerVehicleDrivingComponent::SetHighSpeedMode(const bool bEnabled)
{
    bHighSpeedMode = bEnabled;
}

float UTMOPPlayerVehicleDrivingComponent::GetCurrentSpeedKmh() const
{
    return CurrentSpeedCmPerSecond * (3600.0f / 100000.0f);
}

bool UTMOPPlayerVehicleDrivingComponent::IsDriving() const
{
    return IsValid(DrivenVehicle.Get());
}
