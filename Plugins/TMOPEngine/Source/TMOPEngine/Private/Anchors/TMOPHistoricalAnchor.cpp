#include "Anchors/TMOPHistoricalAnchor.h"

#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "NavigationSystem.h"

ATMOPHistoricalAnchor::ATMOPHistoricalAnchor()
{
    PrimaryActorTick.bCanEverTick = false;

    if (EntityIdentity != nullptr)
    {
        EntityIdentity->EntityType = TEXT("Anchor");
    }

    Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    Billboard->SetupAttachment(GetRootComponent());
    Billboard->SetHiddenInGame(true);

    DebugLabel = CreateDefaultSubobject<UTextRenderComponent>(TEXT("DebugLabel"));
    DebugLabel->SetupAttachment(GetRootComponent());
    DebugLabel->SetHorizontalAlignment(EHTA_Center);
    DebugLabel->SetWorldSize(24.0f);
    DebugLabel->SetRelativeLocation(FVector(0.0f, 0.0f, 60.0f));
    DebugLabel->SetHiddenInGame(true);
}

void ATMOPHistoricalAnchor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (DebugLabel != nullptr)
    {
        const FString LabelText = !DisplayName.IsEmpty()
            ? DisplayName.ToString()
            : GetAnchorId().ToString();

        DebugLabel->SetText(FText::FromString(LabelText));
        DebugLabel->SetVisibility(bShowDebugLabel);
    }

    if (Billboard != nullptr)
    {
        Billboard->SetVisibility(true);
    }
}

FName ATMOPHistoricalAnchor::GetAnchorId() const
{
    return EntityIdentity != nullptr
        ? EntityIdentity->GetEntityId()
        : NAME_None;
}

FVector ATMOPHistoricalAnchor::GetAnchorLocation() const
{
    return GetActorLocation();
}

FRotator ATMOPHistoricalAnchor::GetAnchorRotation() const
{
    return GetActorRotation();
}

FVector ATMOPHistoricalAnchor::GetPlacementLocation(const FName StableKey)
{
    const FVector Origin = GetActorLocation();
    if (PlacementRadiusCm <= KINDA_SMALL_NUMBER) return Origin;

    const FName Key = StableKey.IsNone() ? GetAnchorId() : StableKey;
    if (const FVector* Reserved = ReservedPlacementLocations.Find(Key)) return *Reserved;

    const uint32 Hash = GetTypeHash(Key.ToString());
    FRandomStream Random(static_cast<int32>(Hash));
    FVector Candidate = Origin;
    for (int32 Attempt = 0; Attempt < 64; ++Attempt)
    {
        // Sqrt produces an even distribution over the circle, not a centre-heavy one.
        const float Radius = FMath::Sqrt(Random.FRand()) * PlacementRadiusCm;
        const float Angle = Random.FRandRange(0.0f, 2.0f * PI);
        Candidate = Origin + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.0f) * Radius;
        bool bClear = true;
        for (const TPair<FName, FVector>& Existing : ReservedPlacementLocations)
            if (FVector::DistSquared2D(Candidate, Existing.Value) < FMath::Square(MinimumSpacingCm))
            { bClear = false; break; }
        if (bClear) break;
    }

    if (bProjectPlacementToNavMesh)
        if (const UNavigationSystemV1* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld()))
        {
            FNavLocation Projected;
            if (Nav->ProjectPointToNavigation(Candidate, Projected,
                FVector(FMath::Max(PlacementRadiusCm, 100.0f),
                    FMath::Max(PlacementRadiusCm, 100.0f), 250.0f)))
                Candidate = Projected.Location;
        }
    ReservedPlacementLocations.Add(Key, Candidate);
    return Candidate;
}
