#include "UI/TMOPInteractionPromptWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UTMOPInteractionPromptWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [
                SNew(SBox).WidthOverride(28.0f).HeightOverride(28.0f)
                [
                    SAssignNew(TargetReticleWidget, STextBlock)
                    .Text(FText::FromString(TEXT("□")))
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 28))
                    .ColorAndOpacity(FLinearColor(0.96f, 0.96f, 0.90f, 0.96f))
                    .Justification(ETextJustify::Center)
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 8.0f)
            [
                SNew(SBorder).Padding(FMargin(16.0f, 7.0f))
                .BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.82f))
                [ SAssignNew(PromptTextWidget, STextBlock).Text(PromptText)
                    .Justification(ETextJustify::Center) ]
            ]
        ];
}

void UTMOPInteractionPromptWidget::SetPromptText(const FText& NewText)
{
    if (!PromptText.EqualTo(NewText))
    {
        PromptText = NewText;
        if (PromptTextWidget.IsValid()) PromptTextWidget->SetText(PromptText);
    }
    SetVisibility(PromptText.IsEmpty() ? ESlateVisibility::Collapsed
        : ESlateVisibility::HitTestInvisible);
}
