#include "Vehicles/TMOPVehicleBase.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"

ATMOPVehicleBase::ATMOPVehicleBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;
    VehicleCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("VehicleCollision"));
    SetRootComponent(VehicleCollision);
    VehicleCollision->SetBoxExtent(FVector(225.0f, 90.0f, 60.0f));
    VehicleCollision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    VehicleCollision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
    VehicleCollision->SetGenerateOverlapEvents(false);

    VehicleRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VehicleRoot"));
    VehicleRoot->SetupAttachment(VehicleCollision);
    VehicleRoot->SetRelativeLocation(FVector(0.0f, 0.0f, -60.0f));

    NameLabel =
        CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameLabel"));
    NameLabel->SetupAttachment(VehicleCollision);
    NameLabel->SetHorizontalAlignment(
        EHorizTextAligment::EHTA_Center);
    NameLabel->SetVerticalAlignment(
        EVerticalTextAligment::EVRTA_TextCenter);
    NameLabel->SetWorldSize(NameLabelWorldSize);
    NameLabel->SetTextRenderColor(ResolveNameLabelColor());
    NameLabel->SetCastShadow(false);
    NameLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NameLabel->SetGenerateOverlapEvents(false);
    NameLabel->SetHiddenInGame(false);
}

void ATMOPVehicleBase::BeginPlay()
{
    Super::BeginPlay();
    RefreshNameLabel();
}

void ATMOPVehicleBase::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bShowNameLabel || !IsValid(NameLabel) ||
        GetWorld() == nullptr)
        return;
    const APlayerController* PlayerController =
        GetWorld()->GetFirstPlayerController();
    const APlayerCameraManager* CameraManager =
        IsValid(PlayerController)
        ? PlayerController->PlayerCameraManager : nullptr;
    if (!IsValid(CameraManager))
        return;
    const FVector ToCamera =
        CameraManager->GetCameraLocation() -
        NameLabel->GetComponentLocation();
    if (!ToCamera.IsNearlyZero())
        NameLabel->SetWorldRotation(ToCamera.Rotation());
}

void ATMOPVehicleBase::RefreshNameLabel()
{
    if (!IsValid(NameLabel))
        return;
    const FText LabelText = DisplayName.IsEmpty()
        ? FText::FromName(VehicleId) : DisplayName;
    NameLabel->SetText(LabelText);
    NameLabel->SetRelativeLocation(
        FVector(0.0f, 0.0f, NameLabelHeightCm));
    NameLabel->SetWorldSize(NameLabelWorldSize);
    NameLabel->SetTextRenderColor(NameLabelColor);
    const bool bDisplayLabel = ShouldDisplayNameLabel();
    NameLabel->SetVisibility(bDisplayLabel, true);
    SetActorTickEnabled(bDisplayLabel);
}

bool ATMOPVehicleBase::ShouldDisplayNameLabel() const
{
    return bShowNameLabel;
}

FColor ATMOPVehicleBase::ResolveNameLabelColor() const
{
    const FString Category = VehicleCategoryId.ToString().ToUpper();
    const FString Id = VehicleId.ToString().ToUpper();
    if (Category.StartsWith(TEXT("OBSERVED_")) ||
        Id.StartsWith(TEXT("OBSERVED_")))
    {
        return ObservedNameLabelColor;
    }
    return NameLabelColor;
}

void ATMOPVehicleBase::SetNameLabelVisible(const bool bVisible)
{
    bShowNameLabel = bVisible;
    RefreshNameLabel();
}

TArray<UTMOPVehicleSeatComponent*> ATMOPVehicleBase::GetVehicleSeats() const
{
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetComponents<UTMOPVehicleSeatComponent>(Seats);
    return Seats;
}

UTMOPVehicleSeatComponent* ATMOPVehicleBase::GetDriverSeat() const
{
    for (UTMOPVehicleSeatComponent* Seat : GetVehicleSeats())
        if (IsValid(Seat) && Seat->SeatRole == ETMOPVehicleSeatRole::Driver) return Seat;
    return nullptr;
}

bool ATMOPVehicleBase::EnterVehicle(ATMOPHistoricalAgent* Agent, const FName PreferredSeatId)
{
    for (UTMOPVehicleSeatComponent* Seat : GetVehicleSeats())
    {
        if (!IsValid(Seat) || Seat->IsOccupied()) continue;
        if (!PreferredSeatId.IsNone() && Seat->SeatId != PreferredSeatId) continue;
        return Seat->EnterSeat(Agent);
    }
    return false;
}

bool ATMOPVehicleBase::EnterDriverSeat(ATMOPHistoricalAgent* Agent)
{
    UTMOPVehicleSeatComponent* DriverSeat = GetDriverSeat();
    return IsValid(DriverSeat) && DriverSeat->EnterSeat(Agent);
}

bool ATMOPVehicleBase::ExitVehicle(ATMOPHistoricalAgent* Agent)
{
    for (UTMOPVehicleSeatComponent* Seat : GetVehicleSeats())
        if (IsValid(Seat) && Seat->GetOccupant() == Agent) return Seat->ExitSeat(Agent);
    return false;
}

ATMOPHistoricalAgent* ATMOPVehicleBase::GetDriverAgent() const
{
    UTMOPVehicleSeatComponent* DriverSeat = GetDriverSeat();
    return IsValid(DriverSeat) ? DriverSeat->GetOccupant() : nullptr;
}

