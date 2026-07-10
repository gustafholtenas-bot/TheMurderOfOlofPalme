#include "Anchors/TMOPAnchorSubsystem.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Anchors/TMOPHistoricalPlace.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"

void UTMOPAnchorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Anchors.Reset();
    Places.Reset();
}

void UTMOPAnchorSubsystem::Deinitialize()
{
    Anchors.Reset();
    Places.Reset();
    Super::Deinitialize();
}

bool UTMOPAnchorSubsystem::RegisterAnchor(ATMOPHistoricalAnchor* Anchor)
{
    if (!IsValid(Anchor))
    {
        return false;
    }

    const FName AnchorId = Anchor->GetAnchorId();
    if (AnchorId.IsNone())
    {
        return false;
    }

    if (const TWeakObjectPtr<ATMOPHistoricalAnchor>* Existing = Anchors.Find(AnchorId))
    {
        if (Existing->IsValid() && Existing->Get() != Anchor)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("TMOP rejected duplicate anchor ID '%s'."),
                *AnchorId.ToString());
            return false;
        }
    }

    Anchors.Add(AnchorId, Anchor);
    OnAnchorRegistered.Broadcast(AnchorId, Anchor);
    return true;
}

bool UTMOPAnchorSubsystem::RegisterPlace(ATMOPHistoricalPlace* Place)
{
    if (!IsValid(Place) ||
        Place->EntityIdentity == nullptr ||
        Place->EntityIdentity->GetEntityId().IsNone())
    {
        return false;
    }

    const FName PlaceId = Place->EntityIdentity->GetEntityId();

    if (const TWeakObjectPtr<ATMOPHistoricalPlace>* Existing = Places.Find(PlaceId))
    {
        if (Existing->IsValid() && Existing->Get() != Place)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("TMOP rejected duplicate place ID '%s'."),
                *PlaceId.ToString());
            return false;
        }
    }

    Places.Add(PlaceId, Place);
    OnPlaceRegistered.Broadcast(PlaceId, Place);
    return true;
}

bool UTMOPAnchorSubsystem::UnregisterAnchor(const FName AnchorId)
{
    return Anchors.Remove(AnchorId) > 0;
}

bool UTMOPAnchorSubsystem::UnregisterPlace(const FName PlaceId)
{
    return Places.Remove(PlaceId) > 0;
}

ATMOPHistoricalAnchor* UTMOPAnchorSubsystem::FindAnchor(const FName AnchorId) const
{
    if (const TWeakObjectPtr<ATMOPHistoricalAnchor>* Found = Anchors.Find(AnchorId))
    {
        return Found->Get();
    }

    return nullptr;
}

ATMOPHistoricalPlace* UTMOPAnchorSubsystem::FindPlace(const FName PlaceId) const
{
    if (const TWeakObjectPtr<ATMOPHistoricalPlace>* Found = Places.Find(PlaceId))
    {
        return Found->Get();
    }

    return nullptr;
}

TArray<ATMOPHistoricalAnchor*> UTMOPAnchorSubsystem::GetAnchorsForPlace(
    const FName PlaceId) const
{
    TArray<ATMOPHistoricalAnchor*> Results;

    for (const TPair<FName, TWeakObjectPtr<ATMOPHistoricalAnchor>>& Pair : Anchors)
    {
        ATMOPHistoricalAnchor* Anchor = Pair.Value.Get();
        if (IsValid(Anchor) && Anchor->ParentPlaceId == PlaceId)
        {
            Results.Add(Anchor);
        }
    }

    return Results;
}

TArray<ATMOPHistoricalAnchor*> UTMOPAnchorSubsystem::GetAnchorsByCategory(
    const ETMOPAnchorCategory Category) const
{
    TArray<ATMOPHistoricalAnchor*> Results;

    for (const TPair<FName, TWeakObjectPtr<ATMOPHistoricalAnchor>>& Pair : Anchors)
    {
        ATMOPHistoricalAnchor* Anchor = Pair.Value.Get();
        if (IsValid(Anchor) && Anchor->AnchorCategory == Category)
        {
            Results.Add(Anchor);
        }
    }

    return Results;
}

int32 UTMOPAnchorSubsystem::DiscoverAnchorsInWorld()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return 0;
    }

    int32 RegisteredCount = 0;

    for (TActorIterator<ATMOPHistoricalPlace> It(World); It; ++It)
    {
        RegisteredCount += RegisterPlace(*It) ? 1 : 0;
    }

    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        RegisteredCount += RegisterAnchor(*It) ? 1 : 0;
    }

    return RegisteredCount;
}
