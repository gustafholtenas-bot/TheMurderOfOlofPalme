#include "UI/TMOPAgentInfoChartWidget.h"

#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

void UTMOPAgentInfoChartWidget::InitializeAgentInfo(
    ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    SetIsFocusable(true);
}

void UTMOPAgentInfoChartWidget::ShowAgentInfo(
    const FTMOPPersonProfileRow& Profile, const FText& TimelineSummary,
    const bool bPoliceInterviewed)
{
    if (NameText.IsValid()) NameText->SetText(Profile.FullName.IsEmpty()
        ? FText::FromName(Profile.EntityId) : Profile.FullName);

    TArray<FString> IdentityParts;
    if (!Profile.Occupation.IsEmpty()) IdentityParts.Add(Profile.Occupation);
    if (Profile.AgeAtEvent > 0)
        IdentityParts.Add(FString::Printf(TEXT("%d år 1986"), Profile.AgeAtEvent));
    if (!Profile.Uppslag.IsEmpty())
        IdentityParts.Add(FString::Printf(TEXT("Uppslag %s"), *Profile.Uppslag));
    if (IdentityText.IsValid()) IdentityText->SetText(FText::FromString(
        IdentityParts.IsEmpty() ? TEXT("Historisk person")
            : FString::Join(IdentityParts, TEXT("  •  "))));
    if (InterviewStatusText.IsValid()) InterviewStatusText->SetText(
        bPoliceInterviewed
            ? NSLOCTEXT("TMOP", "AgentInfoInterviewedYes", "POLISFÖRHÖRD: JA")
            : NSLOCTEXT("TMOP", "AgentInfoInterviewedNo", "POLISFÖRHÖRD: NEJ / EJ BELAGT"));
    if (TimelineText.IsValid()) TimelineText->SetText(TimelineSummary.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoTimeline", "Ingen läsbar tidslinje är registrerad.")
        : TimelineSummary);
    if (ObservationText.IsValid()) ObservationText->SetText(
        Profile.ObservationSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoObservations",
                "Inga egna observationer är sammanfattade ännu.")
            : Profile.ObservationSummary);
    FString Sources = Profile.AgentInfoSourceReference;
    if (Sources.IsEmpty()) Sources = Profile.GeneralSourceReference;
    if (SourceText.IsValid()) SourceText->SetText(Sources.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoSources", "Källhänvisning saknas.")
        : FText::FromString(Sources));
    bChartVisible = true;
    RefreshVisibility();
}

void UTMOPAgentInfoChartWidget::HideAgentInfo()
{
    bChartVisible = false;
    RefreshVisibility();
}

TSharedRef<SWidget> UTMOPAgentInfoChartWidget::RebuildWidget()
{
    const auto SectionHeader = [this](const FText& Text)
    {
        return SNew(STextBlock).Text(Text)
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoHeading"),
                FCoreStyle::GetDefaultFontStyle("Bold", 17)))
            .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                TEXT("AgentInfoHeading"), FLinearColor(0.95f, 0.70f, 0.20f)));
    };
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
        [ SAssignNew(MainPanel, SBorder)
          .Visibility(EVisibility::Collapsed)
          .BorderBackgroundColor(FLinearColor(0.008f, 0.012f, 0.018f, 0.94f))
          .Padding(FMargin(32.0f))
          [ SNew(SBox).WidthOverride(920.0f).MaxDesiredHeight(820.0f)
            .HAlign(HAlign_Center).VAlign(VAlign_Center)
            [ SNew(SBorder)
              .BorderBackgroundColor(FLinearColor(0.025f, 0.035f, 0.045f, 0.98f))
              .Padding(FMargin(28.0f, 22.0f))
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SHorizontalBox)
                  + SHorizontalBox::Slot().FillWidth(1.0f)
                  [ SAssignNew(NameText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoName"),
                        FCoreStyle::GetDefaultFontStyle("Bold", 28)))
                    .ColorAndOpacity(FLinearColor::White) ]
                  + SHorizontalBox::Slot().AutoWidth()
                  [ SNew(SButton)
                    .Text(NSLOCTEXT("TMOP", "CloseAgentInfo", "Stäng"))
                    .OnClicked_UObject(this,
                        &UTMOPAgentInfoChartWidget::HandleCloseClicked) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4, 0, 4)
                [ SAssignNew(IdentityText, STextBlock)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoIdentity"),
                      FCoreStyle::GetDefaultFontStyle("Regular", 15)))
                  .ColorAndOpacity(FLinearColor(0.68f, 0.76f, 0.82f)) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 3, 0, 14)
                [ SAssignNew(InterviewStatusText, STextBlock)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoStatus"),
                      FCoreStyle::GetDefaultFontStyle("Bold", 15)))
                  .ColorAndOpacity(FLinearColor(0.40f, 0.85f, 0.58f)) ]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [ SNew(SScrollBox)
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoTimelineHeader",
                      "PERSONENS TIDSLINJE")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(TimelineText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.96f))
                    .AutoWrapText(true).WrapTextAt(820.0f) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoObservationHeader",
                      "OBSERVATIONER OCH FÖRHÖRSUPPGIFTER")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(ObservationText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.96f))
                    .AutoWrapText(true).WrapTextAt(820.0f) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoSourcesHeader",
                      "KÄLLOR")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 5)
                  [ SAssignNew(SourceText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoSources"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 13)))
                    .ColorAndOpacity(FLinearColor(0.62f, 0.69f, 0.74f))
                    .AutoWrapText(true).WrapTextAt(820.0f) ] ] ] ] ] ];
}

FReply UTMOPAgentInfoChartWidget::HandleCloseClicked()
{
    if (ATMOPPlayerCharacter* Player = PlayerCharacter.Get())
        Player->CloseAgentInfoChart();
    return FReply::Handled();
}

void UTMOPAgentInfoChartWidget::RefreshVisibility()
{
    SetVisibility(bChartVisible
        ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (MainPanel.IsValid()) MainPanel->SetVisibility(bChartVisible
        ? EVisibility::Visible : EVisibility::Collapsed);
}
