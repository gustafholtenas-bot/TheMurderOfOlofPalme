#include "Player/TMOPPlayerVehicleDrivingComponent.h"

#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
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
    bHasGroundContact = false;
    SetComponentTickEnabled(false);
    if (IsValid(PreviousVehicle)) OnPlayerDrivingStateChanged.Broadcast(PreviousVehicle, false);
}

void UTMOPPlayerVehicleDrivingComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!IsValid(DrivenVehicle) || DeltaTime <= 0.0f) return;

    const float MaxForward = MaximumForwardSpeedKmh * (100000.0f / 3600.0f);
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
        FMath::Max(1.0f, MaxForward), 0.0f, 1.0f);
    const float EffectiveSteering = FMath::DegreesToRadians(VisualSteeringAngleDegrees) *
        FMath::Lerp(1.0f, 0.55f, SpeedAlpha);
    const float YawDeltaRadians = MinimumTurnRadiusCm > 1.0f
        ? (CurrentSpeedCmPerSecond / MinimumTurnRadiusCm) * FMath::Sin(EffectiveSteering) * DeltaTime
        : 0.0f;

    FRotator Rotation = DrivenVehicle->GetActorRotation();
    Rotation.Yaw += FMath::RadiansToDegrees(YawDeltaRadians);
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

            DesiredLocation.Z =
                TargetGroundHeight + RootHalfHeight + GroundClearanceCm;
            LastGroundHeightCm = TargetGroundHeight;
            bHasGroundContact = true;

            if (bAlignToGroundNormal && !AverageNormal.IsNearlyZero())
            {
                const FVector GroundForward = FVector::VectorPlaneProject(
                    Forward, AverageNormal).GetSafeNormal();
                if (!GroundForward.IsNearlyZero())
                    Rotation = FRotationMatrix::MakeFromXZ(
                        GroundForward, AverageNormal).Rotator();
            }
        }
    }

    FHitResult Hit;
    const bool bMoved = DrivenVehicle->SetActorLocationAndRotation(
        DesiredLocation, Rotation, bSweepMovement,
        bSweepMovement ? &Hit : nullptr, ETeleportType::None);
    if (bSweepMovement && (!bMoved || Hit.bBlockingHit))
    {
        // A low kerb may touch the body before every ground probe has reached
        // its top. Try a bounded vertical step, then keep normal sweeping.
        const float ObstacleHeight =
            Hit.ImpactPoint.Z - DrivenVehicle->GetActorLocation().Z;
        if (ObstacleHeight <= MaximumStepUpHeightCm)
        {
            const FVector StepLocation = DesiredLocation +
                FVector(0.0f, 0.0f, MaximumStepUpHeightCm);
            FHitResult StepHit;
            if (DrivenVehicle->SetActorLocationAndRotation(
                StepLocation, Rotation, true, &StepHit, ETeleportType::None) &&
                !StepHit.bBlockingHit)
            {
                LastGroundHeightCm += MaximumStepUpHeightCm;
            }
            else
            {
                CurrentSpeedCmPerSecond = 0.0f;
            }
        }
        else
        {
            CurrentSpeedCmPerSecond = 0.0f;
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

float UTMOPPlayerVehicleDrivingComponent::GetCurrentSpeedKmh() const
{
    return CurrentSpeedCmPerSecond * (3600.0f / 100000.0f);
}

bool UTMOPPlayerVehicleDrivingComponent::IsDriving() const
{
    return IsValid(DrivenVehicle.Get());
}
