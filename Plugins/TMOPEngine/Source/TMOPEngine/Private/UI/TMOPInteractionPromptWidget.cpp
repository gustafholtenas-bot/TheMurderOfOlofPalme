#include "UI/TMOPInteractionPromptWidget.h"

#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UTMOPInteractionPromptWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(0.0f, 0.0f, 0.0f, 120.0f)
        [
            SNew(SBorder).Padding(FMargin(22.0f, 12.0f))
            .BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.88f))
            [ SAssignNew(PromptTextWidget, STextBlock).Text(PromptText)
                .Justification(ETextJustify::Center) ]
        ];
}

void UTMOPInteractionPromptWidget::SetPromptText(const FText& NewText)
{
    if (PromptText.EqualTo(NewText)) return;
    PromptText = NewText;
    if (PromptTextWidget.IsValid()) PromptTextWidget->SetText(PromptText);
    SetVisibility(PromptText.IsEmpty() ? ESlateVisibility::Collapsed
        : ESlateVisibility::HitTestInvisible);
}
