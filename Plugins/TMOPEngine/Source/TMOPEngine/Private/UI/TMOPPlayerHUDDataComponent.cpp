#include "UI/TMOPPlayerHUDDataComponent.h"

#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"

UTMOPPlayerHUDDataComponent::UTMOPPlayerHUDDataComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SimulationTimeText = FText::FromString(TEXT("23:00:00"));
}

void UTMOPPlayerHUDDataComponent::BeginPlay()
{
    Super::BeginPlay();
    Inventory = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryComponent>() : nullptr;
    if (IsValid(Inventory.Get()))
    {
        Inventory->OnEquippedItemChanged.AddDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleEquippedItemChanged);
        HandleEquippedItemChanged(nullptr, Inventory->EquippedItem.Get());
    }
}

void UTMOPPlayerHUDDataComponent::SetSimulationTime(const int32 Hour,
    const int32 Minute, const int32 Second)
{
    SimulationTimeText = FText::FromString(FString::Printf(TEXT("%02d:%02d:%02d"),
        FMath::Clamp(Hour, 0, 23), FMath::Clamp(Minute, 0, 59),
        FMath::Clamp(Second, 0, 59)));
    OnHUDDataChanged.Broadcast();
}

void UTMOPPlayerHUDDataComponent::SetMapUnlocked(const bool bUnlocked)
{
    if (bMapUnlocked == bUnlocked) return;
    bMapUnlocked = bUnlocked;
    OnHUDDataChanged.Broadcast();
}

void UTMOPPlayerHUDDataComponent::HandleEquippedItemChanged(
    UTMOPItemDefinition* PreviousItem, UTMOPItemDefinition* NewItem)
{
    EquippedItemName = IsValid(NewItem) ? NewItem->DisplayName : FText::GetEmpty();
    EquippedItemIcon = IsValid(NewItem) ? NewItem->Icon : nullptr;
    OnHUDDataChanged.Broadcast();
}
