#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "TMOPPlayerHUDDataComponent.generated.h"

class UTexture2D;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTMOPHUDDataChangedSignature);

/** Lightweight data source for the future HUD widget. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerHUDDataComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerHUDDataComponent();
    virtual void BeginPlay() override;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    FText SimulationTimeText;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    FText EquippedItemName;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    TObjectPtr<UTexture2D> EquippedItemIcon;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    bool bMapUnlocked = false;

    UPROPERTY(BlueprintAssignable, Category="TMOP|HUD")
    FTMOPHUDDataChangedSignature OnHUDDataChanged;

    /** Feed this from the central simulation clock until direct clock binding is added. */
    UFUNCTION(BlueprintCallable, Category="TMOP|HUD")
    void SetSimulationTime(int32 Hour, int32 Minute, int32 Second);

    UFUNCTION(BlueprintCallable, Category="TMOP|HUD")
    void SetMapUnlocked(bool bUnlocked);

private:
    UFUNCTION()
    void HandleEquippedItemChanged(UTMOPItemDefinition* PreviousItem,
        UTMOPItemDefinition* NewItem);

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryComponent> Inventory;
};
