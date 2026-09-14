#include "UI/TMOPAgentInfoChartWidget.h"
#include "UI/TMOPLocalPanel.h"
#include "UI/TMOPControlUIHelpers.h"
#include "People/TMOPPersonNameLibrary.h"
#include "InputCoreTypes.h"
#include "Engine/Texture2D.h"

#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SNullWidget.h"
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
    const FText Name = UTMOPPersonNameLibrary::FormatPersonName(Profile.FullName, Profile.FirstName, Profile.LastName);
    if (NameText.IsValid()) NameText->SetText(Name.IsEmpty()
        ? NSLOCTEXT("TMOP", "UnnamedPersonDisplay", "Okänd person") : Name);

    TArray<FString> IdentityParts;
    switch (Profile.Gender)
    {
    case ETMOPPersonGender::Female: IdentityParts.Add(TEXT("Kvinna")); break;
    case ETMOPPersonGender::Male: IdentityParts.Add(TEXT("Man")); break;
    case ETMOPPersonGender::OtherOrUnspecified:
        IdentityParts.Add(TEXT("Annat / ej angivet")); break;
    default: IdentityParts.Add(TEXT("Kön ej angivet")); break;
    }
    if (Profile.AgeAtEvent > 0)
        IdentityParts.Add(FString::Printf(TEXT("%d år 1986"), Profile.AgeAtEvent));
    if (!Profile.Occupation.IsEmpty()) IdentityParts.Add(Profile.Occupation);
    if (!Profile.Uppslag.IsEmpty())
        IdentityParts.Add(FString::Printf(TEXT("Uppslag %s"), *Profile.Uppslag));
    if (IdentityText.IsValid()) IdentityText->SetText(FText::FromString(
        IdentityParts.IsEmpty() ? TEXT("Historisk person")
            : FString::Join(IdentityParts, TEXT("  •  "))));
    if (InterviewStatusText.IsValid()) InterviewStatusText->SetText(
        bPoliceInterviewed
            ? NSLOCTEXT("TMOP", "AgentInfoInterviewedYes", "FÖRHÖRD AV POLIS: JA")
            : NSLOCTEXT("TMOP", "AgentInfoInterviewedNo", "FÖRHÖRD AV POLIS: EJ FÖRHÖRD / EJ BELAGT"));
    if (InterviewStatusText.IsValid()) InterviewStatusText->SetColorAndOpacity(
        bPoliceInterviewed ? FLinearColor(0.40f, 0.85f, 0.58f)
                           : FLinearColor(0.95f, 0.12f, 0.10f));
    if (TimelineText.IsValid()) TimelineText->SetText(TimelineSummary.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoTimeline", "Ingen läsbar tidslinje är registrerad.")
        : TimelineSummary);
    if (ObservationText.IsValid()) ObservationText->SetText(
        Profile.ObservationSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoObservations",
                "Inga egna observationer är sammanfattade ännu.")
            : Profile.ObservationSummary);
    if (PostMurderEventsText.IsValid()) PostMurderEventsText->SetText(
        Profile.PostMurderEventsSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoPostMurderEvents",
                "Inga källbelagda händelser efter mordet är registrerade ännu.")
            : Profile.PostMurderEventsSummary);
    UTexture2D* Portrait = Profile.ReferenceImage.IsNull()
        ? nullptr : Profile.ReferenceImage.LoadSynchronous();
    PortraitBrush = FSlateBrush();
    PortraitBrush.DrawAs = ESlateBrushDrawType::Image;
    PortraitBrush.ImageSize = FVector2D(180.0f, 230.0f);
    PortraitBrush.SetResourceObject(Portrait);
    if (PortraitImage.IsValid())
    {
        PortraitImage->SetImage(&PortraitBrush);
        PortraitImage->SetVisibility(Portrait
            ? EVisibility::Visible : EVisibility::Collapsed);
    }
    if (PortraitPlaceholder.IsValid())
        PortraitPlaceholder->SetVisibility(Portrait
            ? EVisibility::Collapsed : EVisibility::Visible);
    FString Sources = Profile.AgentInfoSourceReference;
    if (Sources.IsEmpty()) Sources = Profile.GeneralSourceReference;
    if (SourceText.IsValid()) SourceText->SetText(Sources.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoSources", "Källhänvisning saknas.")
        : FText::FromString(Sources));
    bChartVisible = true;
    if (ScrollBox.IsValid()) ScrollBox->ScrollToStart();
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
    return TMOPFitLocalPanel(this, SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
        [ SAssignNew(MainPanel, SBorder)
          .Visibility(EVisibility::Collapsed)
          .BorderBackgroundColor(FLinearColor::Transparent)
          .Padding(0.0f)
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(0.62f)
            [ SNullWidget::NullWidget ]
            + SHorizontalBox::Slot().FillWidth(1.0f)
              .Padding(10.0f, 28.0f, 28.0f, 28.0f)
            [ SNew(SBorder)
              .BorderBackgroundColor(FLinearColor(0.005f, 0.007f, 0.010f, 0.97f))
              .Padding(FMargin(26.0f, 20.0f))
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SHorizontalBox)
                  + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Top)
                  [ SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [ SAssignNew(NameText, STextBlock)
                      .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoName"),
                          FCoreStyle::GetDefaultFontStyle("Bold", 30)))
                      .ColorAndOpacity(FLinearColor::White) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 8)
                    [ SAssignNew(IdentityText, STextBlock)
                      .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoIdentity"),
                          FCoreStyle::GetDefaultFontStyle("Regular", 15)))
                      .ColorAndOpacity(FLinearColor(0.78f, 0.80f, 0.83f)) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ SAssignNew(InterviewStatusText, STextBlock)
                      .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoStatus"),
                          FCoreStyle::GetDefaultFontStyle("Bold", 15))) ] ]
                  + SHorizontalBox::Slot().AutoWidth().Padding(18, 0)
                  [ SNew(SBox).WidthOverride(180).HeightOverride(230)
                    [ SNew(SOverlay)
                      + SOverlay::Slot()
                      [ SNew(SBorder)
                        .BorderBackgroundColor(FLinearColor(0.92f, 0.92f, 0.90f, 1))
                        [ SAssignNew(PortraitPlaceholder, STextBlock)
                          .Text(NSLOCTEXT("TMOP", "AgentInfoPortraitPlaceholder", "BILD"))
                          .Justification(ETextJustify::Center)
                          .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1)) ] ]
                      + SOverlay::Slot()
                      [ SAssignNew(PortraitImage, SImage).Image(&PortraitBrush) ] ] ]
                  + SHorizontalBox::Slot().AutoWidth()
                  [ SNew(SButton)
                    .Text(NSLOCTEXT("TMOP", "CloseAgentInfo", "Stäng"))
                    .OnClicked_UObject(this,
                        &UTMOPAgentInfoChartWidget::HandleCloseClicked) ] ]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [ SAssignNew(ScrollBox, SScrollBox)
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
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoPostMurderHeader",
                      "HÄNDELSER EFTER MORDET")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(PostMurderEventsText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.96f))
                    .AutoWrapText(true).WrapTextAt(820.0f) ]
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
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoSourcesHeader",
                      "KÄLLOR")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 5)
                  [ SAssignNew(SourceText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoSources"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 13)))
                    .ColorAndOpacity(FLinearColor(0.62f, 0.69f, 0.74f))
                    .AutoWrapText(true).WrapTextAt(820.0f) ] ] ] ] ] ]);
}

FReply UTMOPAgentInfoChartWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (bChartVisible)
    {
        const FKey Key = Event.GetKey();
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::Cancel) ||
            TMOPMatchesControl(this, Key, ETMOPControlAction::MenuBack) ||
            (!TMOPHasControlProfiles(this) &&
             (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)))
            return HandleCloseClicked();
        float Delta = 0.0f;
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuUp) ||
            (!TMOPHasControlProfiles(this) && Key == EKeys::Gamepad_DPad_Up)) Delta = -64.0f;
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuDown) ||
            (!TMOPHasControlProfiles(this) && Key == EKeys::Gamepad_DPad_Down)) Delta = 64.0f;
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuPreviousPage) ||
            (!TMOPHasControlProfiles(this) && Key == EKeys::Gamepad_LeftShoulder)) Delta = -360.0f;
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuNextPage) ||
            (!TMOPHasControlProfiles(this) && Key == EKeys::Gamepad_RightShoulder)) Delta = 360.0f;
        if (ScrollBox.IsValid() && Delta != 0.0f)
        {
            ScrollBox->SetScrollOffset(FMath::Max(0.0f, ScrollBox->GetScrollOffset() + Delta));
            return FReply::Handled();
        }
    }
    return Super::NativeOnPreviewKeyDown(Geometry, Event);
}

void UTMOPAgentInfoChartWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    ScrollBox.Reset();
    MainPanel.Reset();
    NameText.Reset();
    IdentityText.Reset();
    InterviewStatusText.Reset();
    TimelineText.Reset();
    ObservationText.Reset();
    PostMurderEventsText.Reset();
    SourceText.Reset();
    PortraitImage.Reset();
    PortraitPlaceholder.Reset();
    PortraitBrush.SetResourceObject(nullptr);
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
