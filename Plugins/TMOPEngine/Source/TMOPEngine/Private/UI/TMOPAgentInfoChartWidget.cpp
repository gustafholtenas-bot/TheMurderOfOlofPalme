#include "UI/TMOPAgentInfoChartWidget.h"
#include "Localization/TMOPLocalization.h"
#include "UI/TMOPLocalPanel.h"
#include "UI/STMOPObservationMap.h"
#include "UI/TMOPControlUIHelpers.h"
#include "People/TMOPPersonNameLibrary.h"
#include "People/TMOPPersonRegistrySubsystem.h"
#include "Observations/TMOPObservationDirector.h"
#include "InputCoreTypes.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"

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
    const FTMOPPersonProfileRow& SourceProfile, const FText& TimelineSummary,
    const bool bPoliceInterviewed, const FName InspectedEntityId)
{
    const FTMOPPersonProfileRow Profile = FTMOPLocalization::RowView(
        TEXT("DT_TMOP_People"), InspectedEntityId.ToString(), SourceProfile);
    const FText Name = UTMOPPersonNameLibrary::FormatPersonName(Profile.FullName, Profile.FirstName, Profile.LastName);
    if (NameText.IsValid()) NameText->SetText(FTMOPLocalization::Text(Name.IsEmpty()
        ? NSLOCTEXT("TMOP", "UnnamedPersonDisplay", "Okänd person") : Name));

    TArray<FString> IdentityParts;
    if (Profile.IsDogProfile())
        IdentityParts.Add(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.0e80fd31dfbcb743", "Hund").ToString());
    else switch (Profile.Gender)
    {
    case ETMOPPersonGender::Female: IdentityParts.Add(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.7f3e681856b576f0", "Kvinna").ToString()); break;
    case ETMOPPersonGender::Male: IdentityParts.Add(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.8b70bb3a4d458cf2", "Man").ToString()); break;
    case ETMOPPersonGender::OtherOrUnspecified:
        IdentityParts.Add(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.ba49620fda091110", "Annat / ej angivet").ToString()); break;
    default: IdentityParts.Add(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.fcae4b26307c86a1", "Kön ej angivet").ToString()); break;
    }
    if (Profile.AgeAtEvent > 0)
        IdentityParts.Add(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.c0780ef220aac770", "{0} år 1986"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Profile.AgeAtEvent))).ToString());
    if (!Profile.Occupation.IsEmpty()) IdentityParts.Add(FTMOPLocalization::String(Profile.Occupation));
    if (!Profile.Uppslag.IsEmpty())
        IdentityParts.Add(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.86069abb0a6683c6", "Uppslag {0}"), FTMOPLocalization::Text(FString(Profile.Uppslag))).ToString());
    if (IdentityText.IsValid()) IdentityText->SetText(FTMOPLocalization::Text(FText::FromString(IdentityParts.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.d59ed2e9b98bc827", "Historisk person").ToString()
            : FString::Join(IdentityParts, TEXT("  •  ")))));
    if (InterviewStatusText.IsValid()) InterviewStatusText->SetText(FTMOPLocalization::Text(
        bPoliceInterviewed
            ? NSLOCTEXT("TMOP", "AgentInfoInterviewedYes", "FÖRHÖRD AV POLIS: JA")
            : NSLOCTEXT("TMOP", "AgentInfoInterviewedNo", "FÖRHÖRD AV POLIS: EJ FÖRHÖRD / EJ BELAGT")));
    if (InterviewStatusText.IsValid()) InterviewStatusText->SetColorAndOpacity(
        bPoliceInterviewed ? FLinearColor(0.40f, 0.85f, 0.58f)
                           : FLinearColor(0.95f, 0.12f, 0.10f));
    // Resolve before any whitespace changes so long table texts retain their identity.
    if (TimelineText.IsValid()) TimelineText->SetText(TimelineSummary.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoTimeline", "Ingen läsbar tidslinje är registrerad.")
        : FTMOPLocalization::Text(TimelineSummary));
    if (ObservationText.IsValid()) ObservationText->SetText(FTMOPLocalization::Text(
        Profile.ObservationSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoObservations",
                "Inga egna observationer är sammanfattade ännu.")
            : Profile.ObservationSummary));
    if (ObserversText.IsValid())
        ObserversText->SetText(FTMOPLocalization::Text(BuildObserverSummary(InspectedEntityId)));
    if (PostMurderEventsText.IsValid()) PostMurderEventsText->SetText(FTMOPLocalization::Text(
        Profile.PostMurderEventsSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoPostMurderEvents",
                "Inga källbelagda händelser efter mordet är registrerade ännu.")
            : Profile.PostMurderEventsSummary));
    RefreshEvidenceGallery(Profile);
    FString Sources = Profile.AgentInfoSourceReference;
    if (Sources.IsEmpty()) Sources = Profile.GeneralSourceReference;
    if (SourceText.IsValid()) SourceText->SetText(FTMOPLocalization::Text(Sources.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoSources", "Källhänvisning saknas.")
        : FText::FromString(Sources)));
    bChartVisible = true;
    if (ScrollBox.IsValid()) ScrollBox->ScrollToStart();
    RefreshVisibility();
}

FText UTMOPAgentInfoChartWidget::BuildObserverSummary(
    const FName InspectedEntityId) const
{
    ATMOPObservationDirector* ObservationDirector = nullptr;
    if (UWorld* World = GetWorld())
        for (TActorIterator<ATMOPObservationDirector> It(World); It; ++It)
        {
            ObservationDirector = *It;
            break;
        }
    if (!IsValid(ObservationDirector) || InspectedEntityId.IsNone())
        return NSLOCTEXT("TMOP", "AgentInfoNoObserverData",
            "Inga registrerade observatörer.");

    const TArray<FName> ObserverIds =
        ObservationDirector->GetObserverEntityIdsForTarget(InspectedEntityId);
    if (ObserverIds.IsEmpty())
        return NSLOCTEXT("TMOP", "AgentInfoNoObservers",
            "Ingen namngiven person är kopplad som observatör ännu.");

    UTMOPPersonRegistrySubsystem* Registry = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
    TArray<FString> Lines;
    for (const FName ObserverId : ObserverIds)
    {
        FText DisplayName;
        FTMOPPersonProfileRow ObserverProfile;
        if (IsValid(Registry) &&
            Registry->GetPersonProfile(ObserverId, ObserverProfile))
            DisplayName = UTMOPPersonNameLibrary::FormatPersonName(
                ObserverProfile.FullName, ObserverProfile.FirstName,
                ObserverProfile.LastName);
        if (DisplayName.IsEmpty())
            DisplayName = FText::FromString(
                ObserverId.ToString().Replace(TEXT("_"), TEXT(" ")));
        Lines.Add(FString::Printf(TEXT("• %s"), *FTMOPLocalization::String(DisplayName)));
    }
    return FText::FromString(FString::Join(Lines, TEXT("\n")));
}

void UTMOPAgentInfoChartWidget::RefreshEvidenceGallery(
    const FTMOPPersonProfileRow& Profile)
{
    EvidenceImageBrushes.Reset();
    if (!EvidenceGallery.IsValid()) return;
    EvidenceGallery->ClearChildren();

    TArray<FTMOPEvidenceImage> Images = Profile.EvidenceImages;
    if (!Profile.ReferenceImage.IsNull())
    {
        FTMOPEvidenceImage Reference;
        Reference.Image = Profile.ReferenceImage;
        Reference.Type = ETMOPEvidenceImageType::Photograph;
        Reference.Caption = NSLOCTEXT("TMOP", "AgentInfoReferencePhoto",
            "Referensbild");
        Images.Insert(Reference, 0);
    }

    for (const FTMOPEvidenceImage& Evidence : Images)
    {
        UTexture2D* Texture = Evidence.Image.IsNull()
            ? nullptr : Evidence.Image.LoadSynchronous();
        if (!IsValid(Texture)) continue;

        TSharedPtr<FSlateBrush> Brush = MakeShared<FSlateBrush>();
        Brush->DrawAs = ESlateBrushDrawType::Image;
        Brush->ImageSize = FVector2D(180.0f, 210.0f);
        Brush->SetResourceObject(Texture);
        EvidenceImageBrushes.Add(Brush);

        FText Caption = Evidence.Caption;
        if (Caption.IsEmpty())
        {
            switch (Evidence.Type)
            {
            case ETMOPEvidenceImageType::PhantomImage:
                Caption = NSLOCTEXT("TMOP", "EvidencePhantomImage", "Fantombild"); break;
            case ETMOPEvidenceImageType::Sketch:
                Caption = NSLOCTEXT("TMOP", "EvidenceSketch", "Skiss"); break;
            case ETMOPEvidenceImageType::Reconstruction:
                Caption = NSLOCTEXT("TMOP", "EvidenceReconstruction", "Rekonstruktion"); break;
            case ETMOPEvidenceImageType::Photograph:
                Caption = NSLOCTEXT("TMOP", "EvidencePhotograph", "Fotografi"); break;
            case ETMOPEvidenceImageType::Document:
                Caption = NSLOCTEXT("TMOP", "EvidenceDocument", "Dokumentbild"); break;
            default:
                Caption = NSLOCTEXT("TMOP", "EvidenceOtherImage", "Bild"); break;
            }
        }

        EvidenceGallery->AddSlot()
        .Padding(FMargin(0.0f, 0.0f, 12.0f, 0.0f))
        [
            SNew(SBox).WidthOverride(180.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBox).WidthOverride(180.0f).HeightOverride(210.0f)
                    [ SNew(SImage).Image(Brush.Get()) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
                [
                    SNew(STextBlock).Text(FTMOPLocalization::Text(Caption))
                    .Font(ATMOPTypographyDirector::ResolveFont(this,
                        TEXT("AgentInfoCaption"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 12)))
                    .ColorAndOpacity(FLinearColor(0.78f, 0.80f, 0.83f))
                    .AutoWrapText(true).WrapTextAt(180.0f)
                ]
            ]
        ];
    }

    const bool bHasImages = !EvidenceImageBrushes.IsEmpty();
    EvidenceGallery->SetVisibility(bHasImages
        ? EVisibility::Visible : EVisibility::Collapsed);
    if (EvidenceGalleryPlaceholder.IsValid())
        EvidenceGalleryPlaceholder->SetVisibility(bHasImages
            ? EVisibility::Collapsed : EVisibility::Visible);
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
        return SNew(STextBlock).Text(FTMOPLocalization::Text(Text)).AutoWrapText(true)
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoHeading"),
                FCoreStyle::GetDefaultFontStyle("Bold", 17)))
            .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                TEXT("AgentInfoHeading"), FLinearColor(0.95f, 0.70f, 0.20f)));
    };
    return TMOPFillLocalPanel(this, SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Fill).VAlign(VAlign_Fill)
        [ SAssignNew(MainPanel, SBorder)
          .Visibility(EVisibility::Collapsed)
          .BorderBackgroundColor(FLinearColor::Transparent)
          .Padding(0.0f)
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(0.45f)
            [ SNullWidget::NullWidget ]
            + SHorizontalBox::Slot().FillWidth(1.0f)
              .Padding(10.0f, 28.0f, 28.0f, 28.0f)
            [ SNew(SBorder)
              .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
              .BorderBackgroundColor(FLinearColor::Black)
              .Padding(FMargin(26.0f, 20.0f))
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SHorizontalBox)
                  + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Top)
                  [ SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [ SAssignNew(NameText, STextBlock)
                      .AutoWrapText(true)
                      .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoName"),
                          FCoreStyle::GetDefaultFontStyle("Bold", 30)))
                      .ColorAndOpacity(FLinearColor::White) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 8)
                    [ SAssignNew(IdentityText, STextBlock)
                      .AutoWrapText(true)
                      .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoIdentity"),
                          FCoreStyle::GetDefaultFontStyle("Regular", 15)))
                      .ColorAndOpacity(FLinearColor(0.78f, 0.80f, 0.83f)) ]
                    + SVerticalBox::Slot().AutoHeight()
                    [ SAssignNew(InterviewStatusText, STextBlock)
                      .AutoWrapText(true)
                      .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoStatus"),
                          FCoreStyle::GetDefaultFontStyle("Bold", 15))) ] ]
                  + SHorizontalBox::Slot().AutoWidth().Padding(18, 0)
                  [ SNew(SBox).WidthOverride(380).HeightOverride(250)
                    [ SNew(SOverlay)
                      + SOverlay::Slot()
                      [ SNew(SBorder)
                        .BorderBackgroundColor(FLinearColor(0.92f, 0.92f, 0.90f, 1))
                        [ SAssignNew(EvidenceGalleryPlaceholder, STextBlock)
                          .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "AgentInfoPortraitPlaceholder", "INGA BILDER REGISTRERADE")))
                          .Justification(ETextJustify::Center)
                          .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1)) ] ]
                      + SOverlay::Slot()
                      [ SAssignNew(EvidenceGallery, SScrollBox)
                        .Orientation(Orient_Horizontal) ] ] ]
                  + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
                  [ SNew(SButton)
                    .ContentPadding(FMargin(14.0f, 7.0f))
                    .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "CloseAgentInfo", "Stäng")))
                    .OnClicked_UObject(this,
                        &UTMOPAgentInfoChartWidget::HandleCloseClicked) ] ]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [ SAssignNew(ScrollBox, SScrollBox)
                  .ScrollBarAlwaysVisible(true)
                  .Clipping(EWidgetClipping::ClipToBounds)
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoObservationHeader",
                      "OBSERVATIONER OCH FÖRHÖRSUPPGIFTER")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(ObservationText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.96f))
                    .AutoWrapText(true) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoObserversHeader",
                      "OBSERVERAD AV")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(ObserversText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.55f, 0.92f, 0.63f))
                    .AutoWrapText(true) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoPostMurderHeader",
                      "HÄNDELSER EFTER MORDET")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(PostMurderEventsText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.96f))
                    .AutoWrapText(true) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoMapHeader", "OBSERVATIONSPLATSER")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 8)
                  [ SAssignNew(ObservationMapHost, SBox).HeightOverride(270) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(ObservationPlacesText, STextBlock).AutoWrapText(true)
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
                    .ColorAndOpacity(FLinearColor::White) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoTimelineHeader",
                      "PERSONENS TIDSLINJE")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(TimelineText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.94f, 0.96f))
                    .AutoWrapText(true) ]
                  + SScrollBox::Slot().Padding(0, 4, 12, 5)
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoSourcesHeader",
                      "KÄLLOR")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 5)
                  [ SAssignNew(SourceText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoSources"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 13)))
                    .ColorAndOpacity(FLinearColor(0.62f, 0.69f, 0.74f))
                    .AutoWrapText(true) ] ] ] ] ] ]);
}

FReply UTMOPAgentInfoChartWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (bChartVisible)
    {
        const FKey Key = Event.GetKey();
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::Interact) ||
            TMOPMatchesControl(this, Key, ETMOPControlAction::Cancel) ||
            TMOPMatchesControl(this, Key, ETMOPControlAction::MenuBack) ||
            (!TMOPHasControlProfiles(this) &&
             (Key == EKeys::E || Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right)))
        {
            if (Event.IsRepeat()) return FReply::Handled();
            return HandleCloseClicked();
        }
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
    ObservationMapHost.Reset();
    ObservationPlacesText.Reset();
    NameText.Reset();
    IdentityText.Reset();
    InterviewStatusText.Reset();
    TimelineText.Reset();
    ObservationText.Reset();
    ObserversText.Reset();
    PostMurderEventsText.Reset();
    SourceText.Reset();
    EvidenceGallery.Reset();
    EvidenceGalleryPlaceholder.Reset();
    EvidenceImageBrushes.Reset();
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

void UTMOPAgentInfoChartWidget::SetObservationLocations(const TArray<FTMOPNotebookLocation>& Points)
{
    if (ObservationMapHost.IsValid())
        ObservationMapHost->SetContent(SNew(STMOPObservationMap)
            .Map(PlayerCharacter.IsValid() ? PlayerCharacter->FindComponentByClass<UTMOPMapComponent>() : nullptr)
            .Points(Points));
    TArray<FString> Lines;
    for (const auto& P : Points)
        Lines.AddUnique(FTMOPTime::FromSecondsFromMidnight(P.Second).ToDisplayString() + TEXT(" — ") + P.Address.ToString() + (P.bPlayerObservation ? NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.b2aa15e0ed32de6d", " (egen observation)").ToString() : TEXT("")));
    if (ObservationPlacesText.IsValid()) ObservationPlacesText->SetText(FTMOPLocalization::Text(FText::FromString(Lines.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPAgentInfoChartWidget.37e66f08b14ac98c", "Ingen fastställd observationsplats registrerad.").ToString() : FString::Join(Lines, TEXT("\n")))));
}
