#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/TMOPPlayerVehicleDrivingComponent.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleBase.h"

UTMOPPlayerVehicleSessionComponent::UTMOPPlayerVehicleSessionComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UTMOPPlayerVehicleSessionComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    const UTMOPVehicleTakeoverComponent* Takeover = GetTakeover();
    if (!IsValid(Takeover)) return;
    if (IsValid(LocalVehicleCamera)) { UpdateLocalVehicleCamera(DeltaTime); return; }
    if (ATMOPConfiguredVehicle* Vehicle =
        Cast<ATMOPConfiguredVehicle>(Takeover->CurrentVehicle))
        Vehicle->UpdatePlayerVehicleCamera(DeltaTime);
}

UTMOPVehicleTakeoverComponent* UTMOPPlayerVehicleSessionComponent::GetTakeover() const
{
    return GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPVehicleTakeoverComponent>() : nullptr;
}

UTMOPPlayerVehicleDrivingComponent* UTMOPPlayerVehicleSessionComponent::GetDriving() const
{
    return GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPPlayerVehicleDrivingComponent>() : nullptr;
}

ETMOPVehicleTakeoverResult UTMOPPlayerVehicleSessionComponent::EnterNearestVehicle(
    const bool bPreferDriverSeat)
{
    UTMOPVehicleTakeoverComponent* Takeover = GetTakeover();
    if (!IsValid(Takeover)) return ETMOPVehicleTakeoverResult::FailedInternal;
    const ETMOPVehicleTakeoverResult Result = Takeover->TryEnterNearestVehicle(bPreferDriverSeat);
    if (Result == ETMOPVehicleTakeoverResult::SuccessEmptySeat ||
        Result == ETMOPVehicleTakeoverResult::SuccessDriverRemoved)
    {
        const bool bDriver = Takeover->IsDriver();
        if (bDriver)
        {
            UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving();
            if (!IsValid(Driving) || !Driving->BeginDriving(Takeover->CurrentVehicle))
            {
                Takeover->ExitCurrentVehicle();
                return ETMOPVehicleTakeoverResult::FailedInternal;
            }
        }
        if (bUseVehicleAsCameraTarget)
            SetCameraTarget(Takeover->CurrentVehicle, EnterCameraBlendSeconds);
        SetComponentTickEnabled(true);
        OnVehicleSessionStarted.Broadcast(Takeover->CurrentVehicle, bDriver);
    }
    return Result;
}

ETMOPVehicleTakeoverResult UTMOPPlayerVehicleSessionComponent::EnterVehicle(
    ATMOPVehicleBase* Vehicle, const bool bPreferDriverSeat)
{
    UTMOPVehicleTakeoverComponent* Takeover = GetTakeover();
    if (!IsValid(Takeover)) return ETMOPVehicleTakeoverResult::FailedInternal;
    const ETMOPVehicleTakeoverResult Result = Takeover->TryEnterVehicle(Vehicle, bPreferDriverSeat);
    if (Result == ETMOPVehicleTakeoverResult::SuccessEmptySeat ||
        Result == ETMOPVehicleTakeoverResult::SuccessDriverRemoved)
    {
        const bool bDriver = Takeover->IsDriver();
        if (bDriver)
        {
            UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving();
            if (!IsValid(Driving) || !Driving->BeginDriving(Takeover->CurrentVehicle))
            {
                Takeover->ExitCurrentVehicle();
                return ETMOPVehicleTakeoverResult::FailedInternal;
            }
        }
        if (bUseVehicleAsCameraTarget) SetCameraTarget(Vehicle, EnterCameraBlendSeconds);
        SetComponentTickEnabled(true);
        OnVehicleSessionStarted.Broadcast(Vehicle, bDriver);
    }
    return Result;
}

bool UTMOPPlayerVehicleSessionComponent::ExitVehicle()
{
    UTMOPVehicleTakeoverComponent* Takeover = GetTakeover();
    if (!IsValid(Takeover) || !Takeover->IsInsideVehicle()) return false;
    ATMOPVehicleBase* Vehicle = Takeover->CurrentVehicle;
    const bool bDriver = Takeover->IsDriver();
    if (bDriver)
        if (UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving()) Driving->EndDriving();
    if (!Takeover->ExitCurrentVehicle()) return false;
    SetCameraTarget(GetOwner(), ExitCameraBlendSeconds);
    SetComponentTickEnabled(false);
    OnVehicleSessionEnded.Broadcast(Vehicle, bDriver);
    return true;
}

bool UTMOPPlayerVehicleSessionComponent::IsInVehicle() const
{
    const UTMOPVehicleTakeoverComponent* Takeover = GetTakeover();
    return IsValid(Takeover) && Takeover->IsInsideVehicle();
}

bool UTMOPPlayerVehicleSessionComponent::IsDrivingVehicle() const
{
    const UTMOPVehicleTakeoverComponent* Takeover = GetTakeover();
    return IsValid(Takeover) && Takeover->IsInsideVehicle() && Takeover->IsDriver();
}

void UTMOPPlayerVehicleSessionComponent::VehicleThrottle(const float Value)
{
    if (IsDrivingVehicle()) if (UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving()) Driving->SetThrottleInput(Value);
}

void UTMOPPlayerVehicleSessionComponent::VehicleSteering(const float Value)
{
    if (IsDrivingVehicle()) if (UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving()) Driving->SetSteeringInput(Value);
}

void UTMOPPlayerVehicleSessionComponent::VehicleBrake(const float Value)
{
    if (IsDrivingVehicle()) if (UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving()) Driving->SetBrakeInput(Value);
}

void UTMOPPlayerVehicleSessionComponent::VehicleHandbrake(const bool bPressed)
{
    if (IsDrivingVehicle()) if (UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving()) Driving->SetHandbrakeInput(bPressed);
}

void UTMOPPlayerVehicleSessionComponent::VehicleHighSpeedMode(const bool bEnabled)
{
    if (IsDrivingVehicle())
        if (UTMOPPlayerVehicleDrivingComponent* Driving = GetDriving())
            Driving->SetHighSpeedMode(bEnabled);
}

void UTMOPPlayerVehicleSessionComponent::SetCameraTarget(AActor* Target,
    const float BlendSeconds)
{
    const APawn* Pawn = Cast<APawn>(GetOwner());
    APlayerController* Controller = IsValid(Pawn) ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (IsValid(LocalVehicleCamera))
    {
        LocalVehicleCamera->SetLifeSpan(FMath::Max(0.1f, BlendSeconds + 0.1f));
        LocalVehicleCamera = nullptr; LocalVehicleBoom = nullptr;
    }
    if (IsValid(Controller) && UTMOPLocalMultiplayerSubsystem::IsMultiplayer(this))
        if (ATMOPConfiguredVehicle* Vehicle = Cast<ATMOPConfiguredVehicle>(Target))
        {
            FActorSpawnParameters Params; Params.Owner = GetOwner();
            LocalVehicleCamera = GetWorld()->SpawnActor<ACameraActor>(Params);
            if (LocalVehicleCamera)
            {
                LocalVehicleCamera->AttachToActor(Vehicle, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
                LocalVehicleBoom = NewObject<USpringArmComponent>(LocalVehicleCamera);
                LocalVehicleBoom->SetupAttachment(LocalVehicleCamera->GetRootComponent());
                LocalVehicleBoom->TargetArmLength = Vehicle->VehicleCameraBoom ? Vehicle->VehicleCameraBoom->TargetArmLength : 600.0f;
                LocalVehicleBoom->SetRelativeLocation(Vehicle->VehicleCameraBoom ? Vehicle->VehicleCameraBoom->GetRelativeLocation() : FVector(0,0,130));
                // Manual trace below ignores the vehicle and all its occupants.
                LocalVehicleBoom->bDoCollisionTest = false;
                LocalVehicleCamera->AddInstanceComponent(LocalVehicleBoom);
                LocalVehicleBoom->RegisterComponent();
                LocalVehicleCamera->GetCameraComponent()->AttachToComponent(LocalVehicleBoom,
                    FAttachmentTransformRules::SnapToTargetNotIncludingScale, USpringArmComponent::SocketName);
                Controller->SetControlRotation(FRotator(Vehicle->DefaultCameraPitchDegrees, Vehicle->GetActorRotation().Yaw, 0));
                LastLocalControlRotation = Controller->GetControlRotation();
                SecondsSinceLocalCameraInput = 0;
                UpdateLocalVehicleCamera(0);
                Target = LocalVehicleCamera;
            }
        }
    if (IsValid(Controller) && IsValid(Target))
        Controller->SetViewTargetWithBlend(Target, FMath::Max(0.0f, BlendSeconds));
}

void UTMOPPlayerVehicleSessionComponent::UpdateLocalVehicleCamera(float DeltaTime)
{
    const APawn* Pawn = Cast<APawn>(GetOwner());
    APlayerController* PC = Pawn ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const auto* Takeover = GetTakeover();
    ATMOPConfiguredVehicle* Vehicle = Takeover ? Cast<ATMOPConfiguredVehicle>(Takeover->CurrentVehicle) : nullptr;
    if (!PC || !Vehicle || !LocalVehicleBoom || !LocalVehicleCamera) return;
    FRotator Rotation = PC->GetControlRotation();
    if (Vehicle->bLockCameraBehindVehicle || !Vehicle->bAllowMouseOrbitCamera)
        Rotation = FRotator(Vehicle->DefaultCameraPitchDegrees, Vehicle->GetActorRotation().Yaw, 0);
    else
    {
        const bool bMoved = !Rotation.Equals(LastLocalControlRotation, 0.01f);
        SecondsSinceLocalCameraInput = bMoved ? 0 : SecondsSinceLocalCameraInput + DeltaTime;
        if (SecondsSinceLocalCameraInput >= Vehicle->CameraReturnDelaySeconds)
            Rotation = FMath::RInterpTo(Rotation, FRotator(Vehicle->DefaultCameraPitchDegrees,
                Vehicle->GetActorRotation().Yaw, 0), DeltaTime, Vehicle->CameraReturnSpeed);
        PC->SetControlRotation(Rotation);
    }
    LastLocalControlRotation = Rotation;
    Rotation.Pitch = FMath::Clamp(FRotator::NormalizeAxis(Rotation.Pitch),
        Vehicle->MinimumCameraPitchDegrees, Vehicle->MaximumCameraPitchDegrees);
    LocalVehicleBoom->SetWorldRotation(Rotation);
    const float Distance = Vehicle->VehicleCameraBoom ? Vehicle->VehicleCameraBoom->TargetArmLength : 600.0f;
    const FVector Start = LocalVehicleBoom->GetComponentLocation();
    const FVector End = Start - Rotation.Vector() * Distance;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TMOPLocalVehicleCamera), false, Vehicle);
    for (ATMOPPlayerCharacter* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
        Query.AddIgnoredActor(Player);
    FHitResult Hit;
    const bool bHit = GetWorld()->SweepSingleByChannel(Hit, Start, End, FQuat::Identity,
        ECC_Camera, FCollisionShape::MakeSphere(12.0f), Query);
    LocalVehicleBoom->TargetArmLength = bHit ? FMath::Max(20.0f, Hit.Distance - 12.0f) : Distance;
}

void UTMOPPlayerVehicleSessionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
    if (LocalVehicleCamera) LocalVehicleCamera->Destroy();
    LocalVehicleCamera = nullptr; LocalVehicleBoom = nullptr;
    Super::EndPlay(Reason);
}
