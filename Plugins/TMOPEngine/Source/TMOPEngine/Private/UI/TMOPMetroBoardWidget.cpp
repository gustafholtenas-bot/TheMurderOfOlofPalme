#include "UI/TMOPMetroBoardWidget.h"
#include "Localization/TMOPLocalization.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPMetroBoardWidget::SetBoard(const FText& Station,
    const FText& Arrivals, const FText& Disclaimer)
{
    PendingStation = Station;
    PendingArrivals = Arrivals;
    PendingDisclaimer = Disclaimer;
    if (StationText.IsValid()) StationText->SetText(FTMOPLocalization::Text(PendingStation));
    if (ArrivalText.IsValid()) ArrivalText->SetText(FTMOPLocalization::Text(PendingArrivals));
    if (DisclaimerText.IsValid()) DisclaimerText->SetText(FTMOPLocalization::Text(PendingDisclaimer));
}

TSharedRef<SWidget> UTMOPMetroBoardWidget::RebuildWidget()
{
    return SNew(SBox).WidthOverride(390.0f)
        [ SNew(SBorder)
          .BorderBackgroundColor(FLinearColor(0.005f, 0.012f, 0.018f, 0.94f))
          .Padding(FMargin(15.0f, 10.0f))
          [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SAssignNew(StationText, STextBlock)
              .Text(FTMOPLocalization::Text(PendingStation))
              .Font(FCoreStyle::GetDefaultFontStyle("Bold", 17))
              .ColorAndOpacity(FLinearColor(0.42f, 0.86f, 0.46f)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 5.0f)
            [ SAssignNew(ArrivalText, STextBlock)
              .Text(FTMOPLocalization::Text(PendingArrivals))
              .Font(FCoreStyle::GetDefaultFontStyle("Regular", 15))
              .ColorAndOpacity(FLinearColor::White) ]
            + SVerticalBox::Slot().AutoHeight()
            [ SAssignNew(DisclaimerText, STextBlock)
              .Text(FTMOPLocalization::Text(PendingDisclaimer))
              .Font(FCoreStyle::GetDefaultFontStyle("Italic", 10))
              .AutoWrapText(true).WrapTextAt(360.0f)
              .ColorAndOpacity(FLinearColor(0.95f, 0.67f, 0.24f)) ] ] ];
}

