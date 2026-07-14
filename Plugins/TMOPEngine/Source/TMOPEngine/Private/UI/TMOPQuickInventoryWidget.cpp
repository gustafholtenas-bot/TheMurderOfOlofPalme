#include "UI/TMOPQuickInventoryWidget.h"

#include "Inventory/TMOPItemDefinition.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UTMOPQuickInventoryWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.55f)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SBox).WidthOverride(760.0f).HeightOverride(760.0f)
            [
                SNew(SOverlay)
                + SOverlay::Slot()
                [ SAssignNew(RadialGrid, SUniformGridPanel).SlotPadding(FMargin(12.0f)) ]
                + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SNew(SBorder).Padding(FMargin(28.0f, 18.0f))
                    .BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.04f, 0.96f))
                    [ SAssignNew(CenterLabel, STextBlock)
                        .Text(NSLOCTEXT("TMOP", "InventoryCenter", "INVENTORY"))
                        .Justification(ETextJustify::Center) ]
                ]
            ]
        ];
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

FText UTMOPQuickInventoryWidget::GetEntryName(const int32 Index) const
{
    if (!IsValid(InventoryInput.Get())) return FText::GetEmpty();
    if (InventoryInput->RadialItems.IsValidIndex(Index))
    {
        const UTMOPItemDefinition* Item = InventoryInput->RadialItems[Index].Get();
        return IsValid(Item) ? Item->DisplayName : FText::FromString(TEXT("?"));
    }
    return EmptyHandText;
}

void UTMOPQuickInventoryWidget::RebuildEntries()
{
    if (!RadialGrid.IsValid() || !IsValid(InventoryInput.Get())) return;
    RadialGrid->ClearChildren();
    IconBrushes.Reset();

    static const FIntPoint Positions[8] = {
        {1,0}, {2,0}, {2,1}, {2,2}, {1,2}, {0,2}, {0,1}, {0,0}
    };
    const int32 ItemCount = InventoryInput->RadialItems.Num();
    const int32 EntryCount = FMath::Min(8,
        ItemCount + (InventoryInput->bIncludeEmptyHand ? 1 : 0));

    for (int32 Index = 0; Index < EntryCount; ++Index)
    {
        const bool bSelected = Index == InventoryInput->SelectedRadialIndex;
        const bool bItem = InventoryInput->RadialItems.IsValidIndex(Index);
        const UTMOPItemDefinition* Item = bItem
            ? InventoryInput->RadialItems[Index].Get() : nullptr;
        TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
        Brush->ImageSize = FVector2D(72.0f, 72.0f);
        if (IsValid(Item) && IsValid(Item->Icon)) Brush->SetResourceObject(Item->Icon);
        IconBrushes.Add(Brush);

        RadialGrid->AddSlot(Positions[Index].X, Positions[Index].Y)
        [
            SNew(SBorder).Padding(10.0f)
            .BorderBackgroundColor(bSelected
                ? FLinearColor(0.22f, 0.13f, 0.02f, 0.96f)
                : FLinearColor(0.025f, 0.03f, 0.045f, 0.9f))
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                [ SNew(SBox).WidthOverride(72.0f).HeightOverride(72.0f)
                    [ SNew(SImage).Image(Brush.Get()) ] ]
                + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(3.0f)
                [ SNew(STextBlock).Text(GetEntryName(Index))
                    .ColorAndOpacity(bSelected ? SelectedColor : NormalColor)
                    .Justification(ETextJustify::Center) ]
            ]
        ];
    }

    if (CenterLabel.IsValid())
    {
        const int32 Selected = InventoryInput->SelectedRadialIndex;
        CenterLabel->SetText(Selected == INDEX_NONE
            ? NSLOCTEXT("TMOP", "InventoryChoose", "Välj föremål")
            : GetEntryName(Selected));
        CenterLabel->SetColorAndOpacity(FSlateColor(SelectedColor));
    }
}
