#include "Vehicles/TMOPConfiguredVehicle.h"

#include "Components/StaticMeshComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleModelData.h"

ATMOPConfiguredVehicle::ATMOPConfiguredVehicle()
{
    PrimaryActorTick.bCanEverTick = true;
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(VehicleRoot);
    WheelFrontLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFrontLeft"));
    WheelFrontRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFrontRight"));
    WheelRearLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRearLeft"));
    WheelRearRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRearRight"));
    UStaticMeshComponent* Parts[] = { WheelFrontLeft, WheelFrontRight, WheelRearLeft,
        WheelRearRight };
    for (UStaticMeshComponent* Part : Parts)
    {
        Part->SetupAttachment(VehicleRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}

void ATMOPConfiguredVehicle::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyConfiguration();
}

bool ATMOPConfiguredVehicle::ApplyConfiguration()
{
    if (!IsValid(VehicleModel)) return false;
    BodyMesh->SetStaticMesh(VehicleModel->BodyMesh);
    BodyMesh->SetRelativeTransform(VehicleModel->BodyLocalTransform);
    ApplyWheel(WheelFrontLeft, VehicleModel->Wheels.FrontLeft);
    ApplyWheel(WheelFrontRight, VehicleModel->Wheels.FrontRight);
    ApplyWheel(WheelRearLeft, VehicleModel->Wheels.RearLeft);
    ApplyWheel(WheelRearRight, VehicleModel->Wheels.RearRight);
    if (UTMOPTrafficVehicleMovementComponent* Movement =
        FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
        Movement->VehicleLengthCm = VehicleModel->VehicleLengthCm;
    return true;
}

void ATMOPConfiguredVehicle::ApplyWheel(UStaticMeshComponent* Component,
    const FTransform& LocalTransform)
{
    Component->SetStaticMesh(VehicleModel->WheelMesh);
    Component->SetRelativeTransform(LocalTransform);
    Component->SetVisibility(IsValid(VehicleModel->WheelMesh));
}

void ATMOPConfiguredVehicle::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateWheelAnimation(DeltaSeconds);
}

void ATMOPConfiguredVehicle::UpdateWheelAnimation(const float DeltaSeconds)
{
    if (!IsValid(VehicleModel) || VehicleModel->Wheels.WheelRadiusCm <= 0.0f) return;
    const UTMOPTrafficVehicleMovementComponent* Movement =
        FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    if (!IsValid(Movement)) return;
    const float Radians = Movement->CurrentSpeedCmPerSecond * DeltaSeconds /
        VehicleModel->Wheels.WheelRadiusCm;
    AccumulatedWheelRollDegrees = FMath::Fmod(
        AccumulatedWheelRollDegrees + FMath::RadiansToDegrees(Radians), 360.0f);
    const FQuat Roll(VehicleModel->Wheels.RotationAxis.GetSafeNormal(),
        FMath::DegreesToRadians(AccumulatedWheelRollDegrees));
    UStaticMeshComponent* Wheels[] = { WheelFrontLeft, WheelFrontRight, WheelRearLeft, WheelRearRight };
    const FTransform Bases[] = { VehicleModel->Wheels.FrontLeft, VehicleModel->Wheels.FrontRight,
        VehicleModel->Wheels.RearLeft, VehicleModel->Wheels.RearRight };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        FTransform Animated = Bases[Index];
        Animated.SetRotation(Roll * Bases[Index].GetRotation());
        Wheels[Index]->SetRelativeTransform(Animated);
    }
}
