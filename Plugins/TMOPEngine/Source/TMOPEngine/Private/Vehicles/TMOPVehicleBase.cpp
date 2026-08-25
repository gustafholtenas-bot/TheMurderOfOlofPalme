#include "Vehicles/TMOPVehicleBase.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneComponent.h"
#include "Components/TextRenderComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
FString CompactVehicleSourceNumber(FString Source)
{
    Source.TrimStartAndEndInline();
    int32 Cut = INDEX_NONE;
    const TCHAR Separators[] = { TCHAR(','), TCHAR(';'), TCHAR('\n'), TCHAR('\r') };
    for (const TCHAR Separator : Separators)
    {
        int32 Found = INDEX_NONE;
        if (Source.FindChar(Separator, Found) && (Cut == INDEX_NONE || Found < Cut))
            Cut = Found;
    }
    if (Cut != INDEX_NONE) Source.LeftInline(Cut);
    Source.TrimStartAndEndInline();
    return Source.Left(32);
}
}

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
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnlitTextMaterial(
        TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque.DefaultTextMaterialOpaque"));
    if (UnlitTextMaterial.Succeeded())
    {
        NameLabelUnlitMaterial = UnlitTextMaterial.Object;
        NameLabel->SetTextMaterial(NameLabelUnlitMaterial);
    }
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
    FString Label = DisplayName.IsEmpty()
        ? VehicleId.ToString() : DisplayName.ToString();
    if (RegistrationStatus == ETMOPVehicleRegistrationStatus::Known &&
        !RegistrationNumber.IsEmpty())
    {
        FString CompactLabel = Label.ToUpper();
        CompactLabel.ReplaceInline(TEXT(" "), TEXT(""));
        CompactLabel.ReplaceInline(TEXT("-"), TEXT(""));
        FString CompactRegistration = RegistrationNumber.ToUpper();
        CompactRegistration.ReplaceInline(TEXT(" "), TEXT(""));
        CompactRegistration.ReplaceInline(TEXT("-"), TEXT(""));
        if (!CompactLabel.Contains(CompactRegistration))
            Label += FString::Printf(TEXT(" — %s"), *RegistrationNumber);
    }
    const FString Category = VehicleCategoryId.ToString().ToUpper();
    const FString Id = VehicleId.ToString().ToUpper();
    ETMOPEntityEvidenceIcon ResolvedIcon = EvidenceIcon;
    if (ResolvedIcon == ETMOPEntityEvidenceIcon::Automatic)
        ResolvedIcon = Category.StartsWith(TEXT("OBSERVED_")) ||
            Id.StartsWith(TEXT("OBSERVED_"))
            ? ETMOPEntityEvidenceIcon::Observed
            : ETMOPEntityEvidenceIcon::OtherDocumentation;
    const FString* Symbol = &OtherDocumentationSymbol;
    if (ResolvedIcon == ETMOPEntityEvidenceIcon::Observed)
        Symbol = &ObservedSymbol;
    else if (ResolvedIcon == ETMOPEntityEvidenceIcon::PoliceInterview)
        Symbol = &PoliceInterviewSymbol;
    FString FullLabel = *Symbol;
    const FString CompactSource = CompactVehicleSourceNumber(SourceDocumentNumber);
    FullLabel += TEXT("\n") + (CompactSource.IsEmpty()
        ? MissingSourceText : CompactSource);
    FullLabel += TEXT("\n") + Label;
    NameLabel->SetText(FText::FromString(FullLabel));
    NameLabel->SetRelativeLocation(
        FVector(0.0f, 0.0f, NameLabelHeightCm));
    NameLabel->SetWorldSize(NameLabelWorldSize);
    NameLabel->SetTextRenderColor(ResolveNameLabelColor());
    if (IsValid(NameLabelUnlitMaterial))
        NameLabel->SetTextMaterial(NameLabelUnlitMaterial);
    const bool bDisplayLabel = ShouldDisplayNameLabel();
    NameLabel->SetVisibility(bDisplayLabel, true);
    // Vehicle subclasses use Actor Tick for wheels and lights even when the
    // optional debug label is hidden.
    SetActorTickEnabled(true);
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
        const bool bEntered = Seat->EnterSeat(Agent);
        if (bEntered)
            if (UTMOPVehicleAudioComponent* Audio =
                FindComponentByClass<UTMOPVehicleAudioComponent>())
                Audio->PlayDoorCycle();
        return bEntered;
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
        if (IsValid(Seat) && Seat->GetOccupant() == Agent)
        {
            const bool bExited = Seat->ExitSeat(Agent);
            if (bExited)
                if (UTMOPVehicleAudioComponent* Audio =
                    FindComponentByClass<UTMOPVehicleAudioComponent>())
                    Audio->PlayDoorCycle();
            return bExited;
        }
    return false;
}

ATMOPHistoricalAgent* ATMOPVehicleBase::GetDriverAgent() const
{
    UTMOPVehicleSeatComponent* DriverSeat = GetDriverSeat();
    return IsValid(DriverSeat) ? DriverSeat->GetOccupant() : nullptr;
}
