#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "TMOPInventoryComponent.generated.h"

class ACharacter;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPInventoryEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Inventory")
    TObjectPtr<UTMOPItemDefinition> Item;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Inventory",
        meta=(ClampMin="1"))
    int32 Quantity = 1;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTMOPInventoryChangedSignature,
    UTMOPItemDefinition*, Item, int32, NewQuantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTMOPEquippedItemChangedSignature,
    UTMOPItemDefinition*, PreviousItem, UTMOPItemDefinition*, NewItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTMOPItemMenuRequestedSignature,
    UTMOPItemDefinition*, Item);

/** Simple single-slot inventory. Only one item may be equipped in the right hand. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPInventoryComponent();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Inventory")
    TArray<FTMOPInventoryEntry> StartingItems;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Inventory")
    TArray<FTMOPInventoryEntry> Items;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Inventory")
    TObjectPtr<UTMOPItemDefinition> EquippedItem;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Inventory|Events")
    FTMOPInventoryChangedSignature OnInventoryChanged;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Inventory|Events")
    FTMOPEquippedItemChangedSignature OnEquippedItemChanged;

    /** Map/notebook and other menu items request their UI through this event. */
    UPROPERTY(BlueprintAssignable, Category="TMOP|Inventory|Events")
    FTMOPItemMenuRequestedSignature OnItemMenuRequested;

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory")
    bool AddItem(UTMOPItemDefinition* Item, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory")
    bool RemoveItem(UTMOPItemDefinition* Item, int32 Quantity = 1);

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory")
    bool EquipItem(UTMOPItemDefinition* Item);

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory")
    bool EquipItemById(FName ItemId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory")
    void EquipEmptyHand();

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory")
    bool EquipNextItem(int32 Direction = 1);

    UFUNCTION(BlueprintPure, Category="TMOP|Inventory")
    bool HasItem(UTMOPItemDefinition* Item, int32 MinimumQuantity = 1) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Inventory")
    int32 GetQuantity(UTMOPItemDefinition* Item) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Inventory")
    UTMOPItemDefinition* FindItemById(FName ItemId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Inventory")
    bool HasEquippedItem() const;

private:
    int32 FindEntryIndex(const UTMOPItemDefinition* Item) const;
    void RebuildHandVisual();
    void ClearHandVisual();

    UPROPERTY(Transient)
    TObjectPtr<ACharacter> CharacterOwner;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMeshComponent> HandVisual;
};
