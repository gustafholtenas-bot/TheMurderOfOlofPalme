#include "UI/TMOPAgentInfoChartWidget.h"
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
    const FTMOPPersonProfileRow& Profile, const FText& TimelineSummary,
    const bool bPoliceInterviewed, const FName InspectedEntityId)
{
    const FText Name = UTMOPPersonNameLibrary::FormatPersonName(Profile.FullName, Profile.FirstName, Profile.LastName);
    if (NameText.IsValid()) NameText->SetText(Name.IsEmpty()
        ? NSLOCTEXT("TMOP", "UnnamedPersonDisplay", "Okänd person") : Name);

    TArray<FString> IdentityParts;
    if (Profile.IsDogProfile())
        IdentityParts.Add(TEXT("Hund"));
    else switch (Profile.Gender)
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
    TArray<FString> TimelineWords;
    TimelineSummary.ToString().ParseIntoArrayWS(TimelineWords);
    if (TimelineText.IsValid()) TimelineText->SetText(TimelineWords.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoTimeline", "Ingen läsbar tidslinje är registrerad.")
        : FText::FromString(FString::Join(TimelineWords, TEXT(" "))));
    if (ObservationText.IsValid()) ObservationText->SetText(
        Profile.ObservationSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoObservations",
                "Inga egna observationer är sammanfattade ännu.")
            : Profile.ObservationSummary);
    if (ObserversText.IsValid())
        ObserversText->SetText(BuildObserverSummary(InspectedEntityId));
    if (PostMurderEventsText.IsValid()) PostMurderEventsText->SetText(
        Profile.PostMurderEventsSummary.IsEmpty()
            ? NSLOCTEXT("TMOP", "AgentInfoNoPostMurderEvents",
                "Inga källbelagda händelser efter mordet är registrerade ännu.")
            : Profile.PostMurderEventsSummary);
    RefreshEvidenceGallery(Profile);
    FString Sources = Profile.AgentInfoSourceReference;
    if (Sources.IsEmpty()) Sources = Profile.GeneralSourceReference;
    if (SourceText.IsValid()) SourceText->SetText(Sources.IsEmpty()
        ? NSLOCTEXT("TMOP", "AgentInfoNoSources", "Källhänvisning saknas.")
        : FText::FromString(Sources));
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
        Lines.Add(FString::Printf(TEXT("• %s"), *DisplayName.ToString()));
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
                    SNew(STextBlock).Text(Caption)
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
                          .Text(NSLOCTEXT("TMOP", "AgentInfoPortraitPlaceholder", "INGA BILDER REGISTRERADE"))
                          .Justification(ETextJustify::Center)
                          .ColorAndOpacity(FLinearColor(0.08f, 0.08f, 0.08f, 1)) ] ]
                      + SOverlay::Slot()
                      [ SAssignNew(EvidenceGallery, SScrollBox)
                        .Orientation(Orient_Horizontal) ] ] ]
                  + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
                  [ SNew(SButton)
                    .ContentPadding(FMargin(14.0f, 7.0f))
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
                  [ SectionHeader(NSLOCTEXT("TMOP", "AgentInfoObserversHeader",
                      "OBSERVERAD AV")) ]
                  + SScrollBox::Slot().Padding(0, 0, 12, 20)
                  [ SAssignNew(ObserversText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AgentInfoBody"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
                    .ColorAndOpacity(FLinearColor(0.55f, 0.92f, 0.63f))
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
        Lines.AddUnique(FTMOPTime::FromSecondsFromMidnight(P.Second).ToDisplayString() + TEXT(" — ") + P.Address.ToString() + (P.bPlayerObservation ? TEXT(" (egen observation)") : TEXT("")));
    if (ObservationPlacesText.IsValid()) ObservationPlacesText->SetText(FText::FromString(
        Lines.IsEmpty() ? TEXT("Ingen fastställd observationsplats registrerad.") : FString::Join(Lines, TEXT("\n"))));
}
