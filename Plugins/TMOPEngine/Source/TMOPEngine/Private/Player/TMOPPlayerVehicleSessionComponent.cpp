#include "Player/TMOPPlayerVehicleSessionComponent.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Player/TMOPPlayerVehicleDrivingComponent.h"
#include "Vehicles/TMOPVehicleBase.h"

UTMOPPlayerVehicleSessionComponent::UTMOPPlayerVehicleSessionComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
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

void UTMOPPlayerVehicleSessionComponent::SetCameraTarget(AActor* Target,
    const float BlendSeconds) const
{
    const APawn* Pawn = Cast<APawn>(GetOwner());
    APlayerController* Controller = IsValid(Pawn) ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (IsValid(Controller) && IsValid(Target))
        Controller->SetViewTargetWithBlend(Target, FMath::Max(0.0f, BlendSeconds));
}
