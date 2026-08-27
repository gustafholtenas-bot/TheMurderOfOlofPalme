#include "UI/TMOPSpeechBubbleWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

void UTMOPSpeechBubbleWidget::SetSpeechText(const FText& NewText)
{
    if (SpeechText.IsValid()) SpeechText->SetText(NewText);
}

TSharedRef<SWidget> UTMOPSpeechBubbleWidget::RebuildWidget()
{
    return SNew(SBox)
        .WidthOverride(480.0f)
        .MinDesiredHeight(70.0f)
        [
            SNew(SBorder)
            .BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.92f))
            .Padding(FMargin(18.0f, 12.0f))
            [
                SAssignNew(SpeechText, STextBlock)
                .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("SpeechBubble"),
                    FCoreStyle::GetDefaultFontStyle("Regular", 20)))
                .AutoWrapText(true)
                .WrapTextAt(440.0f)
                .Justification(ETextJustify::Center)
                .ColorAndOpacity(FLinearColor::White)
            ]
        ];
}
