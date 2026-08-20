#include "World/TMOPVerticalTransport.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Components/SceneComponent.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"

ATMOPVerticalTransport::ATMOPVerticalTransport()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bStartWithTickEnabled = false;
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(SceneRoot);
}

bool ATMOPVerticalTransport::Connects(const FName From, const FName To) const
{
    return (From == LowerAnchorId && To == UpperAnchorId) ||
           (From == UpperAnchorId && To == LowerAnchorId);
}

ATMOPVerticalTransport* ATMOPVerticalTransport::FindTransport(
    UObject* WorldContextObject, const FName From, const FName To)
{
    UWorld* World = WorldContextObject != nullptr ? WorldContextObject->GetWorld() : nullptr;
    if (World == nullptr) return nullptr;
    for (TActorIterator<ATMOPVerticalTransport> It(World); It; ++It)
        if (It->Connects(From, To)) return *It;
    return nullptr;
}

bool ATMOPVerticalTransport::RequestTransport(
    AActor* Passenger, const FName From, const FName To)
{
    if (!IsValid(Passenger) || bTransporting || !Connects(From, To)) return false;
    UGameInstance* GI = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPAnchorSubsystem* Anchors = GI != nullptr ? GI->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
    ATMOPHistoricalAnchor* FromAnchor = Anchors != nullptr ? Anchors->FindAnchor(From) : nullptr;
    ATMOPHistoricalAnchor* ToAnchor = Anchors != nullptr ? Anchors->FindAnchor(To) : nullptr;
    if (!IsValid(FromAnchor) || !IsValid(ToAnchor)) return false;

    ActivePassenger = Passenger;
    StartLocation = FromAnchor->GetAnchorLocation();
    EndLocation = ToAnchor->GetAnchorLocation();
    ElapsedSeconds = -BoardingDelaySeconds;
    bTransporting = true;
    Passenger->SetActorLocation(StartLocation, false, nullptr, ETeleportType::TeleportPhysics);
    SetActorTickEnabled(true);
    return true;
}

bool ATMOPVerticalTransport::IsTransporting(const AActor* Passenger) const
{
    return bTransporting && ActivePassenger == Passenger;
}

void ATMOPVerticalTransport::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bTransporting || !IsValid(ActivePassenger))
    {
        bTransporting = false;
        ActivePassenger = nullptr;
        SetActorTickEnabled(false);
        return;
    }

    ElapsedSeconds += DeltaSeconds;
    if (ElapsedSeconds < 0.0f) return;
    const float Alpha = FMath::Clamp(ElapsedSeconds / FMath::Max(TravelDurationSeconds, 0.1f), 0.0f, 1.0f);
    const FVector Location = FMath::Lerp(StartLocation, EndLocation, Alpha);
    ActivePassenger->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
    if (IsValid(MovingVisual)) MovingVisual->SetWorldLocation(Location);

    if (Alpha >= 1.0f)
    {
        ActivePassenger->SetActorLocation(EndLocation, false, nullptr, ETeleportType::TeleportPhysics);
        ActivePassenger = nullptr;
        bTransporting = false;
        SetActorTickEnabled(false);
    }
}
