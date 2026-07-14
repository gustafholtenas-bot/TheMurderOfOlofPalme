#include "UI/TMOPQuickInventoryWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Inventory/TMOPItemDefinition.h"

void UTMOPQuickInventoryWidget::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    BuildVisualTree();
    SetVisibility(ESlateVisibility::Collapsed);
}

void UTMOPQuickInventoryWidget::InitializeInventoryInput(
    UTMOPInventoryInputComponent* InInventoryInput)
{
    if (InventoryInput == InInventoryInput) return;
    if (IsValid(InventoryInput.Get()))
    {
        InventoryInput->OnRadialMenuStateChanged.RemoveDynamic(this,
            &UTMOPQuickInventoryWidget::HandleMenuStateChanged);
        InventoryInput->OnRadialSelectionChanged.RemoveDynamic(this,
            &UTMOPQuickInventoryWidget::HandleSelectionChanged);
    }
    InventoryInput = InInventoryInput;
    if (IsValid(InventoryInput.Get()))
    {
        InventoryInput->OnRadialMenuStateChanged.AddDynamic(this,
            &UTMOPQuickInventoryWidget::HandleMenuStateChanged);
        InventoryInput->OnRadialSelectionChanged.AddDynamic(this,
            &UTMOPQuickInventoryWidget::HandleSelectionChanged);
    }
}

void UTMOPQuickInventoryWidget::NativeDestruct()
{
    if (IsValid(InventoryInput.Get()))
    {
        InventoryInput->OnRadialMenuStateChanged.RemoveDynamic(this,
            &UTMOPQuickInventoryWidget::HandleMenuStateChanged);
        InventoryInput->OnRadialSelectionChanged.RemoveDynamic(this,
            &UTMOPQuickInventoryWidget::HandleSelectionChanged);
    }
    Super::NativeDestruct();
}

void UTMOPQuickInventoryWidget::BuildVisualTree()
{
    if (WidgetTree == nullptr || WidgetTree->RootWidget != nullptr) return;
    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass(), TEXT("QuickInventoryRoot"));
    WidgetTree->RootWidget = Root;

    MenuPanel = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(),
        TEXT("QuickInventoryPanel"));
    MenuPanel->SetBrushColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.88f));
    MenuPanel->SetPadding(FMargin(38.0f, 26.0f));
    UCanvasPanelSlot* PanelSlot = Root->AddChildToCanvas(MenuPanel);
    PanelSlot->SetAnchors(FAnchors(0.5f, 0.5f));
    PanelSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    PanelSlot->SetAutoSize(true);

    EntryBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(),
        TEXT("QuickInventoryEntries"));
    MenuPanel->SetContent(EntryBox);
}

void UTMOPQuickInventoryWidget::HandleMenuStateChanged(const bool bOpen,
    const int32 SelectedIndex, UTMOPItemDefinition* SelectedItem)
{
    if (bOpen)
    {
        RebuildEntries();
        SetVisibility(ESlateVisibility::Visible);
    }
    else SetVisibility(ESlateVisibility::Collapsed);
}

void UTMOPQuickInventoryWidget::HandleSelectionChanged(const int32 SelectedIndex,
    UTMOPItemDefinition* SelectedItem)
{
    RebuildEntries();
}

void UTMOPQuickInventoryWidget::RebuildEntries()
{
    if (!IsValid(EntryBox.Get()) || !IsValid(InventoryInput.Get())) return;
    EntryBox->ClearChildren();

    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>(
        UTextBlock::StaticClass());
    Title->SetText(NSLOCTEXT("TMOP", "QuickInventoryTitle", "INVENTORY"));
    Title->SetColorAndOpacity(FSlateColor(SelectedColor));
    Title->SetJustification(ETextJustify::Center);
    EntryBox->AddChildToVerticalBox(Title)->SetPadding(FMargin(8.0f, 4.0f, 8.0f, 18.0f));

    const int32 ItemCount = InventoryInput->RadialItems.Num();
    const int32 EntryCount = ItemCount + (InventoryInput->bIncludeEmptyHand ? 1 : 0);
    for (int32 Index = 0; Index < EntryCount; ++Index)
    {
        const bool bEmptyHand = Index == ItemCount;
        const bool bSelected = Index == InventoryInput->SelectedRadialIndex;
        const UTMOPItemDefinition* Item = InventoryInput->RadialItems.IsValidIndex(Index)
            ? InventoryInput->RadialItems[Index].Get() : nullptr;
        const FText Name = bEmptyHand ? EmptyHandText
            : (IsValid(Item) ? Item->DisplayName : FText::FromString(TEXT("?")));
        const FString Prefix = bSelected ? TEXT(">  ") : TEXT("   ");

        UTextBlock* Entry = WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass());
        Entry->SetText(FText::FromString(Prefix + Name.ToString()));
        Entry->SetColorAndOpacity(FSlateColor(bSelected ? SelectedColor : NormalColor));
        Entry->SetJustification(ETextJustify::Center);
        EntryBox->AddChildToVerticalBox(Entry)->SetPadding(FMargin(10.0f, 6.0f));
    }
}
