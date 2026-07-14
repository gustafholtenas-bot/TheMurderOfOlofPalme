#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "TMOPQuickInventoryWidget.generated.h"

class UBorder;
class UTextBlock;
class UVerticalBox;

/** Functional placeholder UI for the radial inventory backend. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPQuickInventoryWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Inventory")
    void InitializeInventoryInput(UTMOPInventoryInputComponent* InInventoryInput);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Inventory")
    FText EmptyHandText = NSLOCTEXT("TMOP", "EmptyHand", "Tom hand");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Inventory")
    FLinearColor NormalColor = FLinearColor(0.85f, 0.85f, 0.85f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Inventory")
    FLinearColor SelectedColor = FLinearColor(1.0f, 0.72f, 0.12f, 1.0f);

protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleMenuStateChanged(bool bOpen, int32 SelectedIndex,
        UTMOPItemDefinition* SelectedItem);

    UFUNCTION()
    void HandleSelectionChanged(int32 SelectedIndex,
        UTMOPItemDefinition* SelectedItem);

    void BuildVisualTree();
    void RebuildEntries();

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryInputComponent> InventoryInput;

    UPROPERTY(Transient)
    TObjectPtr<UBorder> MenuPanel;

    UPROPERTY(Transient)
    TObjectPtr<UVerticalBox> EntryBox;
};
