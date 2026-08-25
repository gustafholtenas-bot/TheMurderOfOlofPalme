#include "Vehicles/TMOPConfiguredVehicle.h"

#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "MaterialTypes.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Audio/TMOPVehicleAudioComponent.h"
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
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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

    TrafficMovement =
        CreateDefaultSubobject<UTMOPTrafficVehicleMovementComponent>(
            TEXT("TrafficMovement"));
    TrafficMovement->bStartDrivingAutomatically = false;

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
    UTMOPVehicleModelData* Model = VehicleModel.Get();
    if (!IsValid(Model))
        return false;

    // Construction scripts can run while an actor or Blueprint instance is
    // being reinstanced. Never dereference native subobjects until all parts
    // required by the configuration are valid.
    if (!IsValid(VehicleCollision) || !IsValid(VehicleRoot) ||
        !IsValid(VisualRoot) || !IsValid(BodyMesh))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP configured vehicle '%s' skipped configuration because a required component is invalid."),
            *GetPathName());
        return false;
    }

    const FVector CollisionExtent(
        FMath::Max(10.0f, Model->VehicleLengthCm * 0.5f),
        FMath::Max(10.0f, Model->VehicleWidthCm * 0.5f),
        FMath::Max(10.0f, CollisionHalfHeightCm));
    VehicleCollision->SetBoxExtent(CollisionExtent);
    VehicleRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -CollisionExtent.Z));
    VisualRoot->SetRelativeRotation(FRotator(0.0f, VisualYawCorrectionDegrees, 0.0f));
    BodyMesh->SetStaticMesh(Model->BodyMesh);
    FTransform BodyTransform = Model->BodyLocalTransform;
    if (bAutoAlignBodyMeshToGround && IsValid(Model->BodyMesh) &&
        FMath::IsNearlyZero(BodyTransform.GetTranslation().Z))
    {
        const FBoxSphereBounds LocalBounds = Model->BodyMesh->GetBounds();
        FVector Translation = BodyTransform.GetTranslation();
        Translation.Z -= LocalBounds.Origin.Z - LocalBounds.BoxExtent.Z;
        BodyTransform.SetTranslation(Translation);
    }
    BodyMesh->SetRelativeTransform(BodyTransform);
    ApplyBodyColor();
    InitializeVehicleLightMaterials();
    ApplyWheel(WheelFrontLeft, Model->WheelMesh, Model->Wheels.FrontLeft);
    ApplyWheel(WheelFrontRight, Model->WheelMesh, Model->Wheels.FrontRight);
    ApplyWheel(WheelRearLeft, Model->WheelMesh, Model->Wheels.RearLeft);
    ApplyWheel(WheelRearRight, Model->WheelMesh, Model->Wheels.RearRight);
    if (UTMOPTrafficVehicleMovementComponent* Movement =
        FindComponentByClass<UTMOPTrafficVehicleMovementComponent>())
    {
        Movement->VehicleLengthCm = Model->VehicleLengthCm;
        FVector RoadOffset = Movement->VehicleLocalOffset;
        RoadOffset.Z = CollisionExtent.Z;
        Movement->VehicleLocalOffset = RoadOffset;
    }
    return true;
}

bool ATMOPConfiguredVehicle::ApplyBodyColor()
{
    if (!IsValid(VehicleModel) || !IsValid(BodyMesh) ||
        !IsValid(VehicleModel->BodyMesh))
        return false;

    const int32 SlotIndex = ResolveBodyMaterialSlotIndex();
    if (SlotIndex == INDEX_NONE)
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP vehicle '%s': model '%s' has no material slot named '%s' and fallback index %d is invalid."),
            *VehicleId.ToString(), *VehicleModel->ModelId.ToString(),
            *VehicleModel->BodyMaterialSlotName.ToString(),
            VehicleModel->BodyMaterialSlotIndex);
        return false;
    }

    UMaterialInterface* BaseMaterial =
        VehicleModel->BodyMesh->GetMaterial(SlotIndex);
    BodyMesh->SetMaterial(SlotIndex, BaseMaterial);
    if (!bOverrideBodyColor)
        return true;
    if (!IsValid(BaseMaterial))
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP vehicle '%s': Body slot %d has no material."),
            *VehicleId.ToString(), SlotIndex);
        return false;
    }

    const FName ParameterName =
        ResolveBodyColorParameterName(BaseMaterial);
    if (ParameterName.IsNone())
    {
        UE_LOG(LogTemp, Error,
            TEXT("TMOP vehicle '%s': material '%s' in Body slot has no vector colour parameter. Add a Vector Parameter named VehicleColor to whitecar2 and connect it to Base Color."),
            *VehicleId.ToString(), *BaseMaterial->GetPathName());
        return false;
    }

    UMaterialInstanceDynamic* Paint =
        BodyMesh->CreateDynamicMaterialInstance(SlotIndex, BaseMaterial);
    if (!IsValid(Paint))
        return false;

    Paint->SetVectorParameterValue(ParameterName, BodyColor);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP vehicle '%s': applied colour %s to slot '%s' (%d), parameter '%s'."),
        *VehicleId.ToString(), *BodyColor.ToString(),
        *VehicleModel->BodyMaterialSlotName.ToString(), SlotIndex,
        *ParameterName.ToString());
    return true;
}

int32 ATMOPConfiguredVehicle::ResolveBodyMaterialSlotIndex() const
{
    if (!IsValid(VehicleModel) || !IsValid(VehicleModel->BodyMesh))
        return INDEX_NONE;

    const TArray<FStaticMaterial>& Materials =
        VehicleModel->BodyMesh->GetStaticMaterials();
    if (!VehicleModel->BodyMaterialSlotName.IsNone())
    {
        for (int32 Index = 0; Index < Materials.Num(); ++Index)
            if (Materials[Index].MaterialSlotName.IsEqual(
                VehicleModel->BodyMaterialSlotName,
                ENameCase::IgnoreCase))
                return Index;
    }

    return Materials.IsValidIndex(VehicleModel->BodyMaterialSlotIndex)
        ? VehicleModel->BodyMaterialSlotIndex : INDEX_NONE;
}

FName ATMOPConfiguredVehicle::ResolveBodyColorParameterName(
    UMaterialInterface* Material) const
{
    if (!IsValid(Material))
        return NAME_None;

    TArray<FMaterialParameterInfo> ParameterInfos;
    TArray<FGuid> ParameterIds;
    Material->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);

    const auto HasParameter = [&ParameterInfos](const FName Candidate)
    {
        return ParameterInfos.ContainsByPredicate(
            [Candidate](const FMaterialParameterInfo& Info)
            {
                return Info.Name.IsEqual(Candidate, ENameCase::IgnoreCase);
            });
    };

    if (!VehicleModel->BodyColorParameterName.IsNone() &&
        HasParameter(VehicleModel->BodyColorParameterName))
        return VehicleModel->BodyColorParameterName;

    static const FName Aliases[] = {
        TEXT("VehicleColor"), TEXT("BodyColor"), TEXT("CarColor"),
        TEXT("BaseColor Tint"), TEXT("BaseColorTint"),
        TEXT("BaseColor"), TEXT("Color"), TEXT("Tint")
    };
    for (const FName Alias : Aliases)
        if (HasParameter(Alias))
            return Alias;

    return NAME_None;
}

void ATMOPConfiguredVehicle::ApplyWheel(UStaticMeshComponent* Component,
    UStaticMesh* WheelMesh,
    const FTransform& LocalTransform)
{
    if (!IsValid(Component))
    {
        UE_LOG(LogTemp, Warning,
            TEXT("TMOP configured vehicle '%s' skipped an invalid wheel component."),
            *GetPathName());
        return;
    }

    Component->SetStaticMesh(IsValid(WheelMesh) ? WheelMesh : nullptr);
    Component->SetRelativeTransform(LocalTransform);
    Component->SetVisibility(IsValid(WheelMesh));
}

void ATMOPConfiguredVehicle::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateWheelAnimation(DeltaSeconds);
    UpdateVehicleLights();
}

namespace
{
    FString NormalizeVehicleMaterialSlotName(const FName SlotName)
    {
        FString Normalized = SlotName.ToString().ToLower();
        Normalized.ReplaceInline(TEXT("_"), TEXT(""));
        Normalized.ReplaceInline(TEXT("-"), TEXT(""));
        Normalized.ReplaceInline(TEXT(" "), TEXT(""));
        return Normalized;
    }

    bool IsHeadlightSlot(const FString& Name)
    {
        return Name.Contains(TEXT("headlight")) ||
            Name.Contains(TEXT("frontlight"));
    }

    bool IsOrangeLightSlot(const FString& Name)
    {
        return Name.Contains(TEXT("orangeneon")) ||
            Name.Contains(TEXT("orangelight"));
    }

    bool IsRedLightSlot(const FString& Name)
    {
        return Name.Contains(TEXT("redlight")) ||
            Name.Contains(TEXT("redightneon"));
    }

    bool IsEmergencyBlueSlot(const FString& Name)
    {
        return Name == TEXT("blue") || Name.StartsWith(TEXT("blue0")) ||
            Name.Contains(TEXT("bluesiren"));
    }
}

void ATMOPConfiguredVehicle::InitializeVehicleLightMaterials()
{
    VehicleLightMaterials.Reset();
    VehicleLightColors.Reset();
    EmergencyBlueMaterialIndices.Reset();
    bVehicleLightsInitialized = false;
    if (!IsValid(BodyMesh)) return;

    const TArray<FName> SlotNames = BodyMesh->GetMaterialSlotNames();
    for (int32 Index = 0; Index < SlotNames.Num(); ++Index)
    {
        const FString Normalized = NormalizeVehicleMaterialSlotName(
            SlotNames[Index]);
        FLinearColor LightColor = FLinearColor::Black;
        bool bRecognized = true;
        bool bEmergencyBlue = false;
        if (IsHeadlightSlot(Normalized))
            LightColor = FLinearColor(18.0f, 16.0f, 12.0f, 1.0f);
        else if (IsOrangeLightSlot(Normalized))
            LightColor = FLinearColor(18.0f, 4.0f, 0.15f, 1.0f);
        else if (IsRedLightSlot(Normalized))
            LightColor = FLinearColor(18.0f, 0.08f, 0.04f, 1.0f);
        else if (IsEmergencyBlueSlot(Normalized))
        {
            LightColor = FLinearColor(0.05f, 0.35f, 20.0f, 1.0f);
            bEmergencyBlue = true;
        }
        else
        {
            bRecognized = false;
        }
        if (!bRecognized) continue;

        UMaterialInstanceDynamic* Dynamic =
            BodyMesh->CreateDynamicMaterialInstance(Index);
        if (!IsValid(Dynamic)) continue;
        VehicleLightMaterials.Add(Index, Dynamic);
        VehicleLightColors.Add(Index, LightColor);
        if (bEmergencyBlue)
            EmergencyBlueMaterialIndices.Add(Index);
    }

    bVehicleLightsInitialized = true;
    UpdateVehicleLights(true);
}

void ATMOPConfiguredVehicle::UpdateVehicleLights(const bool bForce)
{
    if (!bVehicleLightsInitialized) return;
    const UTMOPTrafficVehicleMovementComponent* Movement =
        FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    // Player takeover suspends the traffic component tick. That still counts
    // as an actively driven vehicle even while it is stationary at a light.
    const bool bDrivingLightsEnabled = IsValid(Movement) &&
        (Movement->IsDrivingEnabled() || !Movement->IsComponentTickEnabled());

    const UTMOPVehicleAudioComponent* Audio =
        FindComponentByClass<UTMOPVehicleAudioComponent>();
    const bool bSirenEnabled = IsValid(Audio) &&
        Audio->bEmergencySirenEnabled;
    const float WorldSeconds = GetWorld() != nullptr
        ? GetWorld()->GetTimeSeconds() : 0.0f;
    const bool bEmergencyBlueEnabled = bSirenEnabled &&
        (FMath::FloorToInt(WorldSeconds * 8.0f) % 2 == 0);

    if (bForce || bDrivingLightsEnabled != bLastDrivingLightsEnabled)
    {
        for (const TPair<int32, TWeakObjectPtr<UMaterialInstanceDynamic>>& Pair :
            VehicleLightMaterials)
            if (!EmergencyBlueMaterialIndices.Contains(Pair.Key))
                ApplyLightMaterialState(Pair.Key, bDrivingLightsEnabled);
        bLastDrivingLightsEnabled = bDrivingLightsEnabled;
    }
    if (bForce || bEmergencyBlueEnabled != bLastEmergencyBlueEnabled)
    {
        for (const int32 Index : EmergencyBlueMaterialIndices)
            ApplyLightMaterialState(Index, bEmergencyBlueEnabled);
        bLastEmergencyBlueEnabled = bEmergencyBlueEnabled;
    }
}

void ATMOPConfiguredVehicle::ApplyLightMaterialState(
    const int32 MaterialIndex, const bool bEnabled)
{
    UMaterialInstanceDynamic* Dynamic = VehicleLightMaterials.FindRef(
        MaterialIndex).Get();
    const FLinearColor* OnColor = VehicleLightColors.Find(MaterialIndex);
    if (!IsValid(Dynamic) || OnColor == nullptr) return;

    const FLinearColor Color = bEnabled ? *OnColor : FLinearColor::Black;
    static const FName ColorParameters[] = {
        TEXT("EmissiveColor"), TEXT("Emissive"), TEXT("EmissiveColour"),
        TEXT("GlowColor"), TEXT("GlowColour")
    };
    for (const FName Parameter : ColorParameters)
        Dynamic->SetVectorParameterValue(Parameter, Color);

    const float Strength = bEnabled ? 1.0f : 0.0f;
    static const FName StrengthParameters[] = {
        TEXT("EmissiveStrength"), TEXT("EmissiveIntensity"),
        TEXT("GlowStrength"), TEXT("GlowIntensity"), TEXT("Intensity")
    };
    for (const FName Parameter : StrengthParameters)
        Dynamic->SetScalarParameterValue(Parameter, Strength);
}

void ATMOPConfiguredVehicle::UpdateWheelAnimation(const float DeltaSeconds)
{
    const UTMOPVehicleModelData* Model = VehicleModel.Get();
    if (!IsValid(Model) || Model->Wheels.WheelRadiusCm <= 0.0f) return;
    const UTMOPTrafficVehicleMovementComponent* Movement =
        FindComponentByClass<UTMOPTrafficVehicleMovementComponent>();
    if (!IsValid(Movement)) return;

    DisplayedWheelSteeringDegrees = FMath::FInterpTo(
        DisplayedWheelSteeringDegrees,
        Movement->VisualSteeringAngleDegrees,
        DeltaSeconds,
        8.0f);
    const float Radians = Movement->CurrentSpeedCmPerSecond * DeltaSeconds /
        Model->Wheels.WheelRadiusCm;
    AccumulatedWheelRollDegrees = FMath::Fmod(
        AccumulatedWheelRollDegrees + FMath::RadiansToDegrees(Radians), 360.0f);
    const FQuat Roll(Model->Wheels.RotationAxis.GetSafeNormal(),
        FMath::DegreesToRadians(AccumulatedWheelRollDegrees));
    const FQuat Steering(FVector::UpVector,
        FMath::DegreesToRadians(DisplayedWheelSteeringDegrees));
    UStaticMeshComponent* Wheels[] = { WheelFrontLeft, WheelFrontRight, WheelRearLeft, WheelRearRight };
    const FTransform Bases[] = { Model->Wheels.FrontLeft, Model->Wheels.FrontRight,
        Model->Wheels.RearLeft, Model->Wheels.RearRight };
    for (int32 Index = 0; Index < 4; ++Index)
    {
        if (!IsValid(Wheels[Index]))
            continue;
        FTransform Animated = Bases[Index];
        const FQuat AnimatedRotation = Roll * Bases[Index].GetRotation();
        Animated.SetRotation(Index < 2
            ? Steering * AnimatedRotation
            : AnimatedRotation);
        Wheels[Index]->SetRelativeTransform(Animated);
    }
}
