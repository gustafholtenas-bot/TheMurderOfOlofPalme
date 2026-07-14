#include "Player/TMOPPlayerVehicleDrivingComponent.h"

#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleBase.h"

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
    const float DesiredSpeed = ThrottleInput >= 0.0f
        ? ThrottleInput * MaxForward : ThrottleInput * MaxReverse;
    const bool bAccelerating = !FMath::IsNearlyZero(ThrottleInput);
    const float Deceleration = bHandbrakeInput ? HandbrakeDecelerationCmPerSecondSquared
        : (BrakeInput > KINDA_SMALL_NUMBER ? BrakeDecelerationCmPerSecondSquared * BrakeInput
            : CoastingDecelerationCmPerSecondSquared);
    const float Rate = bAccelerating ? EngineAccelerationCmPerSecondSquared : Deceleration;
    CurrentSpeedCmPerSecond = FMath::FInterpConstantTo(CurrentSpeedCmPerSecond,
        bAccelerating ? DesiredSpeed : 0.0f, DeltaTime, Rate);
    if (BrakeInput > KINDA_SMALL_NUMBER || bHandbrakeInput)
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
        FHitResult GroundHit;
        FCollisionQueryParams GroundParams(SCENE_QUERY_STAT(TMOPVehicleGround), false,
            DrivenVehicle.Get());
        const FVector TraceStart = DesiredLocation + FVector(0.0f, 0.0f, GroundTraceUpCm);
        const FVector TraceEnd = DesiredLocation - FVector(0.0f, 0.0f, GroundTraceDownCm);
        if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd,
            ECC_Visibility, GroundParams))
        {
            DesiredLocation.Z = GroundHit.ImpactPoint.Z + GroundClearanceCm;
            if (bAlignToGroundNormal)
            {
                FVector GroundForward = FVector::VectorPlaneProject(
                    Rotation.Vector(), GroundHit.ImpactNormal).GetSafeNormal();
                if (!GroundForward.IsNearlyZero())
                    Rotation = FRotationMatrix::MakeFromXZ(
                        GroundForward, GroundHit.ImpactNormal).Rotator();
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
