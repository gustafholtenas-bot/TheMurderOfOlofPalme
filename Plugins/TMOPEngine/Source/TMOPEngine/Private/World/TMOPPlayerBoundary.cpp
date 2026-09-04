#include "World/TMOPPlayerBoundary.h"

#include "Components/ArrowComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Player/TMOPVehicleTakeoverComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Vehicles/TMOPVehicleBase.h"

ATMOPPlayerBoundary::ATMOPPlayerBoundary()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = true;

    BoundaryRoot = CreateDefaultSubobject<USceneComponent>(TEXT("BoundaryRoot"));
    SetRootComponent(BoundaryRoot);

    BoundaryGuide = CreateDefaultSubobject<UBoxComponent>(TEXT("BoundaryGuide"));
    BoundaryGuide->SetupAttachment(BoundaryRoot);
    BoundaryGuide->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    BoundaryGuide->SetGenerateOverlapEvents(false);
    BoundaryGuide->SetCanEverAffectNavigation(false);
    BoundaryGuide->SetHiddenInGame(true);
    BoundaryGuide->ShapeColor = FColor(64, 255, 96);

    FogWallMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FogWallMesh"));
    FogWallMesh->SetupAttachment(BoundaryRoot);
    FogWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    FogWallMesh->SetGenerateOverlapEvents(false);
    FogWallMesh->SetCanEverAffectNavigation(false);
    FogWallMesh->SetCastShadow(false);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeFinder(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeFinder.Succeeded())
    {
        FogWallMesh->SetStaticMesh(CubeFinder.Object);
    }

    FogEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("FogEffect"));
    FogEffect->SetupAttachment(BoundaryRoot);
    FogEffect->SetAutoActivate(true);

    AllowedSideArrow = CreateDefaultSubobject<UArrowComponent>(
        TEXT("AllowedSideArrow"));
    AllowedSideArrow->SetupAttachment(BoundaryRoot);
    AllowedSideArrow->ArrowColor = FColor(64, 255, 96);
    AllowedSideArrow->ArrowSize = 2.0f;
    AllowedSideArrow->SetHiddenInGame(true);
}

void ATMOPPlayerBoundary::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    RefreshComponents();
}

void ATMOPPlayerBoundary::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bEnablePlayerBlocking)
    {
        ConstrainPlayerToAllowedSide();
    }
}

void ATMOPPlayerBoundary::RefreshComponents()
{
    const float SafeHalfLength = FMath::Max(100.0f, HalfLengthCm);
    const float SafeHalfThickness = FMath::Max(1.0f, HalfThicknessCm);
    const float SafeHalfHeight = FMath::Max(100.0f, HalfHeightCm);
    const FVector Centre(0.0f, 0.0f, SafeHalfHeight);

    if (IsValid(BoundaryGuide))
    {
        BoundaryGuide->SetRelativeLocation(Centre);
        BoundaryGuide->SetBoxExtent(FVector(
            SafeHalfThickness, SafeHalfLength, SafeHalfHeight));
        BoundaryGuide->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        BoundaryGuide->SetCanEverAffectNavigation(false);
    }

    if (IsValid(FogWallMesh))
    {
        FogWallMesh->SetRelativeLocation(Centre);
        // Engine BasicShapes/Cube is 100 cm on every axis.
        FogWallMesh->SetRelativeScale3D(FVector(
            SafeHalfThickness * 2.0f / 100.0f,
            SafeHalfLength * 2.0f / 100.0f,
            SafeHalfHeight * 2.0f / 100.0f));
        FogWallMesh->SetVisibility(bShowFogWall, true);
        FogWallMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        FogWallMesh->SetCanEverAffectNavigation(false);
        if (IsValid(FogMaterial))
        {
            FogWallMesh->SetMaterial(0, FogMaterial);
        }
        else
        {
            FogWallMesh->SetMaterial(0, nullptr);
        }
    }

    if (IsValid(FogEffect))
    {
        FogEffect->SetRelativeLocation(Centre);
        FogEffect->SetRelativeScale3D(FogEffectScale);
        FogEffect->SetAsset(FogNiagaraSystem);
        FogEffect->SetVisibility(IsValid(FogNiagaraSystem), true);
        if (IsValid(FogNiagaraSystem))
        {
            FogEffect->Activate();
        }
        else
        {
            FogEffect->Deactivate();
        }
    }

    if (IsValid(AllowedSideArrow))
    {
        AllowedSideArrow->SetRelativeLocation(FVector(
            SafeHalfThickness + 50.0f, 0.0f, 100.0f));
    }
}

AActor* ATMOPPlayerBoundary::ResolveControlledActor() const
{
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
    if (!IsValid(PlayerPawn))
    {
        return nullptr;
    }

    const UTMOPVehicleTakeoverComponent* Takeover =
        PlayerPawn->FindComponentByClass<UTMOPVehicleTakeoverComponent>();
    if (IsValid(Takeover) && Takeover->IsInsideVehicle() &&
        IsValid(Takeover->CurrentVehicle))
    {
        return bBlockVehicleContainingPlayer
            ? Takeover->CurrentVehicle.Get()
            : nullptr;
    }
    return PlayerPawn;
}

float ATMOPPlayerBoundary::ResolveClearanceCm(
    const AActor* ControlledActor) const
{
    if (const ACharacter* Character = Cast<ACharacter>(ControlledActor))
    {
        if (const UCapsuleComponent* Capsule = Character->GetCapsuleComponent())
        {
            return Capsule->GetScaledCapsuleRadius();
        }
    }

    if (IsValid(ControlledActor))
    {
        const FBox Bounds = ControlledActor->GetComponentsBoundingBox(true);
        if (Bounds.IsValid)
        {
            const FVector Extent = Bounds.GetExtent();
            const FVector Normal = GetActorForwardVector().GetAbs();
            return FVector::DotProduct(Extent, Normal);
        }
    }
    return 50.0f;
}

void ATMOPPlayerBoundary::ConstrainPlayerToAllowedSide()
{
    AActor* ControlledActor = ResolveControlledActor();
    if (!IsValid(ControlledActor))
    {
        return;
    }

    const FTransform BoundaryTransform = GetActorTransform();
    FVector LocalLocation = BoundaryTransform.InverseTransformPosition(
        ControlledActor->GetActorLocation());
    const float Clearance = ResolveClearanceCm(ControlledActor) +
        FMath::Max(0.0f, PushBackPaddingCm);
    const float SafeHalfLength = FMath::Max(100.0f, HalfLengthCm);
    const float SafeHalfThickness = FMath::Max(1.0f, HalfThicknessCm);
    const float SafeHalfHeight = FMath::Max(100.0f, HalfHeightCm);

    if (FMath::Abs(LocalLocation.Y) > SafeHalfLength + Clearance ||
        LocalLocation.Z < -Clearance ||
        LocalLocation.Z > SafeHalfHeight * 2.0f + Clearance)
    {
        return;
    }

    const float MinimumAllowedX = SafeHalfThickness + Clearance;
    if (LocalLocation.X >= MinimumAllowedX)
    {
        return;
    }

    LocalLocation.X = MinimumAllowedX;
    ControlledActor->SetActorLocation(
        BoundaryTransform.TransformPosition(LocalLocation),
        false, nullptr, ETeleportType::TeleportPhysics);
}
