#include "Inventory/TMOPInventoryComponent.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"

UTMOPInventoryComponent::UTMOPInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    CharacterOwner = Cast<ACharacter>(GetOwner());
    Items.Reset();
    for (const FTMOPInventoryEntry& Entry : StartingItems)
        if (IsValid(Entry.Item.Get()) && Entry.Quantity > 0)
            AddItem(Entry.Item.Get(), Entry.Quantity);
}

int32 UTMOPInventoryComponent::FindEntryIndex(const UTMOPItemDefinition* Item) const
{
    if (!IsValid(Item)) return INDEX_NONE;
    return Items.IndexOfByPredicate([Item](const FTMOPInventoryEntry& Entry)
    {
        return Entry.Item.Get() == Item;
    });
}

bool UTMOPInventoryComponent::AddItem(UTMOPItemDefinition* Item, const int32 Quantity)
{
    if (!IsValid(Item) || Item->ItemId.IsNone() || Quantity <= 0) return false;
    const int32 ExistingIndex = FindEntryIndex(Item);
    if (ExistingIndex != INDEX_NONE)
    {
        FTMOPInventoryEntry& Existing = Items[ExistingIndex];
        const int32 NewQuantity = FMath::Clamp(Existing.Quantity + Quantity,
            1, FMath::Max(1, Item->MaximumStack));
        if (NewQuantity == Existing.Quantity) return false;
        Existing.Quantity = NewQuantity;
        OnInventoryChanged.Broadcast(Item, Existing.Quantity);
        return true;
    }
    FTMOPInventoryEntry NewEntry;
    NewEntry.Item = Item;
    NewEntry.Quantity = FMath::Clamp(Quantity, 1, FMath::Max(1, Item->MaximumStack));
    Items.Add(NewEntry);
    OnInventoryChanged.Broadcast(Item, NewEntry.Quantity);
    return true;
}

bool UTMOPInventoryComponent::RemoveItem(UTMOPItemDefinition* Item, const int32 Quantity)
{
    const int32 Index = FindEntryIndex(Item);
    if (Index == INDEX_NONE || Quantity <= 0) return false;
    FTMOPInventoryEntry& Entry = Items[Index];
    Entry.Quantity -= Quantity;
    if (Entry.Quantity > 0)
    {
        OnInventoryChanged.Broadcast(Item, Entry.Quantity);
        return true;
    }
    if (EquippedItem.Get() == Item) EquipEmptyHand();
    Items.RemoveAt(Index);
    OnInventoryChanged.Broadcast(Item, 0);
    return true;
}

bool UTMOPInventoryComponent::EquipItem(UTMOPItemDefinition* Item)
{
    if (!IsValid(Item) || !HasItem(Item) || !Item->bCanEquip) return false;
    if (Item->bOpensMenuInsteadOfEquipping)
    {
        OnItemMenuRequested.Broadcast(Item);
        return true;
    }
    if (EquippedItem.Get() == Item) return true;
    UTMOPItemDefinition* Previous = EquippedItem.Get();
    EquippedItem = Item;
    RebuildHandVisual();
    OnEquippedItemChanged.Broadcast(Previous, Item);
    return true;
}

bool UTMOPInventoryComponent::EquipItemById(const FName ItemId)
{
    return EquipItem(FindItemById(ItemId));
}

void UTMOPInventoryComponent::EquipEmptyHand()
{
    UTMOPItemDefinition* Previous = EquippedItem.Get();
    EquippedItem = nullptr;
    ClearHandVisual();
    if (IsValid(Previous)) OnEquippedItemChanged.Broadcast(Previous, nullptr);
}

bool UTMOPInventoryComponent::EquipNextItem(const int32 Direction)
{
    TArray<UTMOPItemDefinition*> Equippable;
    for (const FTMOPInventoryEntry& Entry : Items)
        if (IsValid(Entry.Item.Get()) && Entry.Item->bCanEquip &&
            !Entry.Item->bOpensMenuInsteadOfEquipping) Equippable.Add(Entry.Item.Get());
    if (Equippable.IsEmpty()) { EquipEmptyHand(); return false; }
    const int32 CurrentIndex = Equippable.IndexOfByKey(EquippedItem.Get());
    const int32 Step = Direction >= 0 ? 1 : -1;
    const int32 NextIndex = CurrentIndex == INDEX_NONE
        ? (Step > 0 ? 0 : Equippable.Num() - 1)
        : (CurrentIndex + Step + Equippable.Num()) % Equippable.Num();
    return EquipItem(Equippable[NextIndex]);
}

bool UTMOPInventoryComponent::HasItem(UTMOPItemDefinition* Item,
    const int32 MinimumQuantity) const
{
    return GetQuantity(Item) >= FMath::Max(1, MinimumQuantity);
}

int32 UTMOPInventoryComponent::GetQuantity(UTMOPItemDefinition* Item) const
{
    const int32 Index = FindEntryIndex(Item);
    return Index != INDEX_NONE ? Items[Index].Quantity : 0;
}

UTMOPItemDefinition* UTMOPInventoryComponent::FindItemById(const FName ItemId) const
{
    for (const FTMOPInventoryEntry& Entry : Items)
        if (IsValid(Entry.Item.Get()) && Entry.Item->ItemId == ItemId) return Entry.Item.Get();
    return nullptr;
}

bool UTMOPInventoryComponent::HasEquippedItem() const
{
    return IsValid(EquippedItem.Get());
}

void UTMOPInventoryComponent::ClearHandVisual()
{
    if (IsValid(HandVisual.Get()))
    {
        HandVisual->DestroyComponent();
        HandVisual = nullptr;
    }
}

void UTMOPInventoryComponent::RebuildHandVisual()
{
    ClearHandVisual();
    if (!IsValid(CharacterOwner.Get()) || !IsValid(EquippedItem.Get()) ||
        !IsValid(EquippedItem->RightHandMesh.Get())) return;
    USkeletalMeshComponent* CharacterMesh = CharacterOwner->GetMesh();
    if (!IsValid(CharacterMesh)) return;
    HandVisual = NewObject<UStaticMeshComponent>(CharacterOwner.Get(), TEXT("TMOPRightHandItemVisual"));
    if (!IsValid(HandVisual.Get())) return;
    HandVisual->SetStaticMesh(EquippedItem->RightHandMesh.Get());
    HandVisual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HandVisual->SetGenerateOverlapEvents(false);
    HandVisual->RegisterComponent();
    HandVisual->AttachToComponent(CharacterMesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale, EquippedItem->RightHandSocket);
    HandVisual->SetRelativeTransform(EquippedItem->RightHandTransform);
}
