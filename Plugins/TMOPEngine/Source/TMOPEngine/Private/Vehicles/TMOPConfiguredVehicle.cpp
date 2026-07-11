#include "Vehicles/TMOPConfiguredVehicle.h"

#include "Components/StaticMeshComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Vehicles/TMOPVehicleAppearanceData.h"
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
    RoofAccessory1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofAccessory1"));
    RoofAccessory2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofAccessory2"));
    RoofAccessory3 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RoofAccessory3"));
    UStaticMeshComponent* Parts[] = { WheelFrontLeft, WheelFrontRight, WheelRearLeft,
        WheelRearRight, RoofAccessory1, RoofAccessory2, RoofAccessory3 };
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
    ApplyAccessory(RoofAccessory1, 0);
    ApplyAccessory(RoofAccessory2, 1);
    ApplyAccessory(RoofAccessory3, 2);
    if (IsValid(AppearancePreset))
    {
        if (IsValid(AppearancePreset->BodyMaterial))
            BodyMesh->SetMaterial(VehicleModel->BodyMaterialSlotIndex, AppearancePreset->BodyMaterial);
        if (IsValid(AppearancePreset->GlassMaterial))
            BodyMesh->SetMaterial(VehicleModel->GlassMaterialSlotIndex, AppearancePreset->GlassMaterial);
        if (IsValid(AppearancePreset->LiveryMaterial))
            BodyMesh->SetMaterial(VehicleModel->LiveryMaterialSlotIndex, AppearancePreset->LiveryMaterial);
    }
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

void ATMOPConfiguredVehicle::ApplyAccessory(UStaticMeshComponent* Component,
    const int32 AccessoryIndex)
{
    if (!IsValid(AppearancePreset) || !AppearancePreset->RoofAccessories.IsValidIndex(AccessoryIndex))
    {
        Component->SetStaticMesh(nullptr);
        Component->SetVisibility(false);
        return;
    }
    const FTMOPRoofAccessoryVisual& Accessory = AppearancePreset->RoofAccessories[AccessoryIndex];
    Component->SetStaticMesh(Accessory.Mesh);
    Component->SetRelativeTransform(Accessory.LocalTransform * VehicleModel->RoofMountTransform);
    if (IsValid(Accessory.Material)) Component->SetMaterial(0, Accessory.Material);
    Component->SetVisibility(IsValid(Accessory.Mesh));
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
