#include "Inventory/TMOPInventoryInputComponent.h"

#include "Inventory/TMOPInventoryComponent.h"

UTMOPInventoryInputComponent::UTMOPInventoryInputComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPInventoryInputComponent::BeginPlay()
{
    Super::BeginPlay();
    Inventory = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryComponent>() : nullptr;
}

UTMOPInventoryComponent* UTMOPInventoryInputComponent::GetInventory() const
{
    return Inventory.Get();
}

void UTMOPInventoryInputComponent::RefreshRadialItems()
{
    RadialItems.Reset();
    if (!IsValid(Inventory.Get())) return;
    for (const FTMOPInventoryEntry& Entry : Inventory->Items)
    {
        UTMOPItemDefinition* Item = Entry.Item.Get();
        if (IsValid(Item) && (Item->bCanEquip || Item->bOpensMenuInsteadOfEquipping))
        {
            RadialItems.Add(Item);
            if (RadialItems.Num() >= MaximumRadialItems) break;
        }
    }
}

bool UTMOPInventoryInputComponent::OpenRadialMenu()
{
    if (bRadialMenuOpen || !IsValid(Inventory.Get())) return false;
    RefreshRadialItems();
    if (RadialItems.IsEmpty() && !bIncludeEmptyHand) return false;
    bRadialMenuOpen = true;
    SelectedRadialIndex = INDEX_NONE;
    OnRadialMenuStateChanged.Broadcast(true, SelectedRadialIndex, nullptr);
    return true;
}

void UTMOPInventoryInputComponent::UpdateRadialSelection(const FVector2D Direction)
{
    if (!bRadialMenuOpen) return;
    const int32 SegmentCount = RadialItems.Num() + (bIncludeEmptyHand ? 1 : 0);
    if (SegmentCount <= 0 || Direction.Size() < RadialDeadZone) return;
    // Index zero begins at the top; indices proceed clockwise.
    float Degrees = FMath::RadiansToDegrees(FMath::Atan2(Direction.X, Direction.Y));
    if (Degrees < 0.0f) Degrees += 360.0f;
    const float SegmentDegrees = 360.0f / static_cast<float>(SegmentCount);
    const int32 NewIndex = FMath::FloorToInt((Degrees + SegmentDegrees * 0.5f) /
        SegmentDegrees) % SegmentCount;
    if (NewIndex == SelectedRadialIndex) return;
    SelectedRadialIndex = NewIndex;
    OnRadialSelectionChanged.Broadcast(SelectedRadialIndex, GetSelectedItem());
}

UTMOPItemDefinition* UTMOPInventoryInputComponent::GetSelectedItem() const
{
    return RadialItems.IsValidIndex(SelectedRadialIndex)
        ? RadialItems[SelectedRadialIndex].Get() : nullptr;
}

bool UTMOPInventoryInputComponent::ConfirmRadialSelection()
{
    if (!bRadialMenuOpen || !IsValid(Inventory.Get())) return false;
    bool bResult = false;
    if (SelectedRadialIndex != INDEX_NONE)
    {
        UTMOPItemDefinition* Selected = GetSelectedItem();
        if (IsValid(Selected)) bResult = Inventory->EquipItem(Selected);
        else if (bIncludeEmptyHand && SelectedRadialIndex == RadialItems.Num())
        {
            Inventory->EquipEmptyHand();
            bResult = true;
        }
    }
    bRadialMenuOpen = false;
    OnRadialMenuStateChanged.Broadcast(false, SelectedRadialIndex, GetSelectedItem());
    return bResult;
}

void UTMOPInventoryInputComponent::CancelRadialMenu()
{
    if (!bRadialMenuOpen) return;
    bRadialMenuOpen = false;
    OnRadialMenuStateChanged.Broadcast(false, SelectedRadialIndex, GetSelectedItem());
}

bool UTMOPInventoryInputComponent::CycleInventory(const int32 Direction)
{
    return IsValid(Inventory.Get()) && Inventory->EquipNextItem(Direction);
}

bool UTMOPInventoryInputComponent::SendEquippedItemInput(const ETMOPItemInput Input,
    const ETMOPItemInputPhase Phase)
{
    if (!IsValid(Inventory.Get()) || !Inventory->HasEquippedItem()) return false;
    OnEquippedItemInput.Broadcast(Inventory->EquippedItem.Get(), Input, Phase);
    return true;
}
