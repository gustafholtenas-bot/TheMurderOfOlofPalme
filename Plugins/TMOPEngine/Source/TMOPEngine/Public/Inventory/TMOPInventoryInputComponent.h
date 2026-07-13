#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "TMOPInventoryInputComponent.generated.h"


UENUM(BlueprintType)
enum class ETMOPItemInput : uint8
{
    Primary,
    Secondary,
    Alternate
};

UENUM(BlueprintType)
enum class ETMOPItemInputPhase : uint8
{
    Started,
    Triggered,
    Completed,
    Cancelled
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTMOPRadialMenuStateSignature,
    bool, bOpen, int32, SelectedIndex, UTMOPItemDefinition*, SelectedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTMOPRadialSelectionSignature,
    int32, SelectedIndex, UTMOPItemDefinition*, SelectedItem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTMOPEquippedItemInputSignature,
    UTMOPItemDefinition*, Item, ETMOPItemInput, Input,
    ETMOPItemInputPhase, Phase);

/** Input/radial-menu layer for the single-slot TMOP inventory. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPInventoryInputComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPInventoryInputComponent();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Inventory Input|Radial",
        meta=(ClampMin="4", ClampMax="12"))
    int32 MaximumRadialItems = 8;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Inventory Input|Radial",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float RadialDeadZone = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Inventory Input|Radial")
    bool bIncludeEmptyHand = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Inventory Input|Radial")
    bool bRadialMenuOpen = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Inventory Input|Radial")
    int32 SelectedRadialIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Inventory Input|Radial")
    TArray<TObjectPtr<UTMOPItemDefinition>> RadialItems;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Inventory Input|Events")
    FTMOPRadialMenuStateSignature OnRadialMenuStateChanged;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Inventory Input|Events")
    FTMOPRadialSelectionSignature OnRadialSelectionChanged;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Inventory Input|Events")
    FTMOPEquippedItemInputSignature OnEquippedItemInput;

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory Input|Radial")
    bool OpenRadialMenu();

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory Input|Radial")
    void UpdateRadialSelection(FVector2D Direction);

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory Input|Radial")
    bool ConfirmRadialSelection();

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory Input|Radial")
    void CancelRadialMenu();

    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory Input")
    bool CycleInventory(int32 Direction);

    /** Returns true when an equipped item consumed the input. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Inventory Input")
    bool SendEquippedItemInput(ETMOPItemInput Input, ETMOPItemInputPhase Phase);

    UFUNCTION(BlueprintPure, Category="TMOP|Inventory Input")
    UTMOPInventoryComponent* GetInventory() const;

private:
    void RefreshRadialItems();
    UTMOPItemDefinition* GetSelectedItem() const;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryComponent> Inventory;
};
