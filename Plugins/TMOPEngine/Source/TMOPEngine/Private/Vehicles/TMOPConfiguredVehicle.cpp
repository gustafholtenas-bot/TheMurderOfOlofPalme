#include "Vehicles/TMOPConfiguredVehicle.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Vehicles/TMOPVehicleDoorComponent.h"

ATMOPConfiguredVehicle::ATMOPConfiguredVehicle()
{
    PrimaryActorTick.bCanEverTick = true;
    VisualRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VisualRoot"));
    VisualRoot->SetupAttachment(VehicleRoot);
    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    BodyMesh->SetupAttachment(VisualRoot);
    BodyMesh->SetSimulatePhysics(false);
    WheelFrontLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFrontLeft"));
    WheelFrontRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelFrontRight"));
    WheelRearLeft = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRearLeft"));
    WheelRearRight = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WheelRearRight"));
    UStaticMeshComponent* Parts[] = { WheelFrontLeft, WheelFrontRight, WheelRearLeft,
        WheelRearRight };
    for (UStaticMeshComponent* Part : Parts)
    {
        Part->SetupAttachment(VisualRoot);
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    VehicleCameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("VehicleCameraBoom"));
    VehicleCameraBoom->SetupAttachment(VehicleRoot);
    VehicleCameraBoom->TargetArmLength = 700.0f;
    VehicleCameraBoom->TargetOffset = FVector(0.0f, 0.0f, 180.0f);
    VehicleCameraBoom->SetRelativeRotation(FRotator(-12.0f, 0.0f, 0.0f));
    VehicleCameraBoom->bDoCollisionTest = true;
    VehicleCameraBoom->bEnableCameraLag = true;
    VehicleCameraBoom->CameraLagSpeed = 7.0f;
    VehicleCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("VehicleCamera"));
    VehicleCamera->SetupAttachment(VehicleCameraBoom, USpringArmComponent::SocketName);

    SeatDriver = CreateDefaultSubobject<UTMOPVehicleSeatComponent>(TEXT("SeatDriver"));
    SeatFrontPassenger = CreateDefaultSubobject<UTMOPVehicleSeatComponent>(TEXT("SeatFrontPassenger"));
    SeatRearLeft = CreateDefaultSubobject<UTMOPVehicleSeatComponent>(TEXT("SeatRearLeft"));
    SeatRearCenter = CreateDefaultSubobject<UTMOPVehicleSeatComponent>(TEXT("SeatRearCenter"));
    SeatRearRight = CreateDefaultSubobject<UTMOPVehicleSeatComponent>(TEXT("SeatRearRight"));

    UTMOPVehicleSeatComponent* Seats[] = { SeatDriver, SeatFrontPassenger,
        SeatRearLeft, SeatRearCenter, SeatRearRight };
    for (UTMOPVehicleSeatComponent* Seat : Seats) Seat->SetupAttachment(VisualRoot);

    SeatDriver->SeatId = TEXT("FRONT_LEFT");
    SeatDriver->SeatRole = ETMOPVehicleSeatRole::Driver;
    SeatDriver->SetRelativeLocation(FVector(28.0f, -43.0f, 72.0f));
    SeatDriver->ExitLocalOffset = FVector(0.0f, -100.0f, 0.0f);

    SeatFrontPassenger->SeatId = TEXT("FRONT_RIGHT");
    SeatFrontPassenger->SeatRole = ETMOPVehicleSeatRole::FrontPassenger;
    SeatFrontPassenger->SetRelativeLocation(FVector(28.0f, 43.0f, 72.0f));
    SeatFrontPassenger->ExitLocalOffset = FVector(0.0f, 100.0f, 0.0f);

    SeatRearLeft->SeatId = TEXT("REAR_LEFT");
    SeatRearLeft->SeatRole = ETMOPVehicleSeatRole::RearLeft;
    SeatRearLeft->SetRelativeLocation(FVector(-40.0f, -47.0f, 71.0f));
    SeatRearLeft->ExitLocalOffset = FVector(0.0f, -100.0f, 0.0f);

    SeatRearCenter->SeatId = TEXT("REAR_CENTER");
    SeatRearCenter->SeatRole = ETMOPVehicleSeatRole::OtherPassenger;
    SeatRearCenter->SetRelativeLocation(FVector(-40.0f, 0.0f, 71.0f));
    SeatRearCenter->ExitLocalOffset = FVector(0.0f, -100.0f, 0.0f);

    SeatRearRight->SeatId = TEXT("REAR_RIGHT");
    SeatRearRight->SeatRole = ETMOPVehicleSeatRole::RearRight;
    SeatRearRight->SetRelativeLocation(FVector(-40.0f, 47.0f, 71.0f));
    SeatRearRight->ExitLocalOffset = FVector(0.0f, 100.0f, 0.0f);

    DoorFrontLeft = CreateDefaultSubobject<UTMOPVehicleDoorComponent>(TEXT("DoorFrontLeft"));
    DoorFrontRight = CreateDefaultSubobject<UTMOPVehicleDoorComponent>(TEXT("DoorFrontRight"));
    DoorRearLeft = CreateDefaultSubobject<UTMOPVehicleDoorComponent>(TEXT("DoorRearLeft"));
    DoorRearRight = CreateDefaultSubobject<UTMOPVehicleDoorComponent>(TEXT("DoorRearRight"));

    UTMOPVehicleDoorComponent* Doors[] = { DoorFrontLeft, DoorFrontRight,
        DoorRearLeft, DoorRearRight };
    for (UTMOPVehicleDoorComponent* Door : Doors)
    {
        Door->SetupAttachment(VisualRoot);
        Door->ApproachLocalOffset = FVector::ZeroVector;
    }

    DoorFrontLeft->DoorId = TEXT("FRONT_LEFT_DOOR");
    DoorFrontLeft->SeatId = SeatDriver->SeatId;
    DoorFrontLeft->DoorSide = ETMOPVehicleDoorSide::Left;
    DoorFrontLeft->SetRelativeLocation(FVector(13.0f, -79.0f, 66.0f));

    DoorFrontRight->DoorId = TEXT("FRONT_RIGHT_DOOR");
    DoorFrontRight->SeatId = SeatFrontPassenger->SeatId;
    DoorFrontRight->DoorSide = ETMOPVehicleDoorSide::Right;
    DoorFrontRight->SetRelativeLocation(FVector(13.0f, 79.0f, 66.0f));

    DoorRearLeft->DoorId = TEXT("REAR_LEFT_DOOR");
    DoorRearLeft->SeatId = SeatRearLeft->SeatId;
    DoorRearLeft->DoorSide = ETMOPVehicleDoorSide::Left;
    DoorRearLeft->SetRelativeLocation(FVector(-63.0f, -79.0f, 66.0f));

    DoorRearRight->DoorId = TEXT("REAR_RIGHT_DOOR");
    DoorRearRight->SeatId = SeatRearRight->SeatId;
    DoorRearRight->DoorSide = ETMOPVehicleDoorSide::Right;
    DoorRearRight->SetRelativeLocation(FVector(-63.0f, 79.0f, 66.0f));
}

void ATMOPConfiguredVehicle::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ApplyConfiguration();
}

bool ATMOPConfiguredVehicle::ApplyConfiguration()
{
    if (!IsValid(VehicleModel)) return false;
    VisualRoot->SetRelativeRotation(FRotator(0.0f, VisualYawCorrectionDegrees, 0.0f));
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
