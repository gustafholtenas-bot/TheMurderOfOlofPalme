#include "Anchors/TMOPHistoricalAnchor.h"

#include "Components/BillboardComponent.h"
#include "Components/TextRenderComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"

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
