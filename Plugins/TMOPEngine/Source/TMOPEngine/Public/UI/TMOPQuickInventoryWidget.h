#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Styling/SlateBrush.h"
#include "TMOPQuickInventoryWidget.generated.h"

/** Native radial inventory view, safe for both native and Widget Blueprint subclasses. */
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
    FLinearColor NormalColor = FLinearColor(0.82f, 0.84f, 0.88f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Inventory")
    FLinearColor SelectedColor = FLinearColor(1.0f, 0.68f, 0.08f, 1.0f);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeDestruct() override;

private:
    UFUNCTION()
    void HandleMenuStateChanged(bool bOpen, int32 SelectedIndex,
        UTMOPItemDefinition* SelectedItem);

    UFUNCTION()
    void HandleSelectionChanged(int32 SelectedIndex,
        UTMOPItemDefinition* SelectedItem);

    void RebuildEntries();
    FText GetEntryName(int32 Index) const;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryInputComponent> InventoryInput;

    TSharedPtr<class SUniformGridPanel> RadialGrid;
    TSharedPtr<class STextBlock> CenterLabel;
    TArray<TSharedPtr<FSlateBrush>> IconBrushes;
};
