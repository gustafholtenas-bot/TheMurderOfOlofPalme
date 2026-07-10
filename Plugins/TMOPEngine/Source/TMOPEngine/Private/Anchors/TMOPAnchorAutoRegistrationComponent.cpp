#include "Anchors/TMOPAnchorAutoRegistrationComponent.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Anchors/TMOPHistoricalPlace.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Entities/TMOPWorldEntityComponent.h"

UTMOPAnchorAutoRegistrationComponent::UTMOPAnchorAutoRegistrationComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPAnchorAutoRegistrationComponent::BeginPlay()
{
    Super::BeginPlay();

    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr
        ? World->GetGameInstance()
        : nullptr;

    UTMOPAnchorSubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPAnchorSubsystem>()
        : nullptr;

    if (Registry == nullptr)
    {
        return;
    }

    if (ATMOPHistoricalAnchor* Anchor =
        Cast<ATMOPHistoricalAnchor>(GetOwner()))
    {
        Registry->RegisterAnchor(Anchor);
    }
    else if (ATMOPHistoricalPlace* Place =
        Cast<ATMOPHistoricalPlace>(GetOwner()))
    {
        Registry->RegisterPlace(Place);
    }
}

void UTMOPAnchorAutoRegistrationComponent::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    UWorld* World = GetWorld();
    UGameInstance* GameInstance = World != nullptr
        ? World->GetGameInstance()
        : nullptr;

    UTMOPAnchorSubsystem* Registry = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPAnchorSubsystem>()
        : nullptr;

    if (Registry != nullptr)
    {
        if (ATMOPHistoricalAnchor* Anchor =
            Cast<ATMOPHistoricalAnchor>(GetOwner()))
        {
            Registry->UnregisterAnchor(Anchor->GetAnchorId());
        }
        else if (ATMOPHistoricalPlace* Place =
            Cast<ATMOPHistoricalPlace>(GetOwner()))
        {
            if (Place->EntityIdentity != nullptr)
            {
                Registry->UnregisterPlace(
                    Place->EntityIdentity->GetEntityId());
            }
        }
    }

    Super::EndPlay(EndPlayReason);
}
