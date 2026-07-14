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
        TArray<FHitResult> GroundHits;
        FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(TMOPVehicleGround), false,
            DrivenVehicle.Get());
        const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, GroundTraceUpCm);
        const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, GroundTraceDownCm);
        GetWorld()->LineTraceMultiByChannel(GroundHits, TraceStart, TraceEnd,
            ECC_Visibility, GroundParams);
        const FHitResult* AcceptedGround = nullptr;
        for (const FHitResult& Candidate : GroundHits)
        {
            if (Candidate.ImpactNormal.Z < MinimumGroundNormalZ) continue;
            if (bHasGroundContact && Candidate.ImpactPoint.Z >
                LastGroundHeightCm + MaximumGroundRisePerFrameCm) continue;
            AcceptedGround = &Candidate;
            break;
        }
        if (AcceptedGround != nullptr)
        {
            float RootHalfHeight = 0.0f;
            if (const UBoxComponent* RootBox = Cast<UBoxComponent>(
                DrivenVehicle->GetRootComponent()))
                RootHalfHeight = RootBox->GetScaledBoxExtent().Z;
            DesiredLocation.Z = AcceptedGround->ImpactPoint.Z + RootHalfHeight +
                GroundClearanceCm;
            LastGroundHeightCm = AcceptedGround->ImpactPoint.Z;
            bHasGroundContact = true;
            if (bAlignToGroundNormal)
            {
                FVector GroundForward = FVector::VectorPlaneProject(
                    Rotation.Vector(), AcceptedGround->ImpactNormal).GetSafeNormal();
                if (!GroundForward.IsNearlyZero())
                    Rotation = FRotationMatrix::MakeFromXZ(
                        GroundForward, AcceptedGround->ImpactNormal).Rotator();
            }
        }
    }
    FHitResult Hit;
    DrivenVehicle->SetActorLocationAndRotation(DesiredLocation,
        Rotation, bSweepMovement, bSweepMovement ? &Hit : nullptr, ETeleportType::None);
    if (bSweepMovement && Hit.bBlockingHit) CurrentSpeedCmPerSecond = 0.0f;

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
