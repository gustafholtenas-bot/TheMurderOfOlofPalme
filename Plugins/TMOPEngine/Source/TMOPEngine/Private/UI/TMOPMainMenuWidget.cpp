#include "UI/TMOPMainMenuWidget.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "UI/TMOPMainMenuIntroDirector.h"
#include "UI/TMOPSaveGameService.h"
#include "UI/TMOPTypographyDirector.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPMainMenuWidget::InitializeMainMenu(
    ATMOPMainMenuIntroDirector* InDirector, UTexture2D* InLogo)
{
    Director = InDirector;
    LogoTexture = InLogo;
    LogoBrush.SetResourceObject(LogoTexture);
    LogoBrush.ImageSize = IsValid(LogoTexture)
        ? FVector2D(LogoTexture->GetSizeX(), LogoTexture->GetSizeY())
        : FVector2D(700.0f, 240.0f);
}

void UTMOPMainMenuWidget::ConfigureIntroText(
    const FTMOPIntroTextPresentationSettings& InSettings)
{
    IntroTextSettings = InSettings;
    ApplyIntroTextStyle();
}

void UTMOPMainMenuWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // The main menu must appear at its final size immediately. This also
    // neutralises an old scale animation/render transform inherited by a
    // Blueprint subclass of this native widget.
    StopAllAnimations();
    SetRenderScale(FVector2D(1.0f, 1.0f));
}

void UTMOPMainMenuWidget::NativeTick(
    const FGeometry& MyGeometry, const float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (!bIntroCardVisible || !IntroTextSettings.bUseTypewriter) return;

    TypewriterCharacterAccumulator += InDeltaTime *
        FMath::Max(1.0f, IntroTextSettings.CharactersPerSecond);
    int32 CharactersToReveal = FMath::FloorToInt(TypewriterCharacterAccumulator);
    if (CharactersToReveal <= 0) return;
    TypewriterCharacterAccumulator -= static_cast<float>(CharactersToReveal);

    if (IntroTextSettings.bTypewriterHeading &&
        RevealedHeadingCharacters < FullIntroHeading.Len())
    {
        const int32 Remaining = FullIntroHeading.Len() - RevealedHeadingCharacters;
        const int32 Added = FMath::Min(Remaining, CharactersToReveal);
        RevealedHeadingCharacters += Added;
        CharactersToReveal -= Added;
        if (IntroHeading.IsValid()) IntroHeading->SetText(FText::FromString(
            FullIntroHeading.Left(RevealedHeadingCharacters)));
    }
    if (CharactersToReveal > 0 && RevealedBodyCharacters < FullIntroBody.Len())
    {
        RevealedBodyCharacters = FMath::Min(FullIntroBody.Len(),
            RevealedBodyCharacters + CharactersToReveal);
        if (IntroBody.IsValid()) IntroBody->SetText(FText::FromString(
            FullIntroBody.Left(RevealedBodyCharacters)));
    }
}

TSharedRef<SWidget> UTMOPMainMenuWidget::RebuildWidget()
{
    const FTMOPMenuColorPalette MenuColors =
        ATMOPTypographyDirector::ResolveMenuColors(this);
    const FSlateFontInfo ButtonFont = ATMOPTypographyDirector::ResolveFont(
        this, TEXT("MainMenuButton"), FCoreStyle::GetDefaultFontStyle("Regular", 21));
    const FSlateColor ButtonTextColor = ATMOPTypographyDirector::ResolveColor(
        this, TEXT("MainMenuButton"), MenuColors.MainMenuButtonText);
    auto MenuButton = [this, &ButtonFont, &ButtonTextColor](const FText& Label,
        FOnClicked Clicked)
    {
        return SNew(SButton).ButtonStyle(FCoreStyle::Get(), "NoBorder")
            .ContentPadding(FMargin(20.0f, 8.0f)).OnClicked(Clicked)
            [ SNew(STextBlock).Text(Label).Font(ButtonFont)
              .ColorAndOpacity(ButtonTextColor) ];
    };

    TSharedRef<SWidget> Root = SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SAssignNew(MenuPanel, SVerticalBox)
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ SNew(SBox).WidthOverride(760.0f).HeightOverride(260.0f)
            [ SNew(SImage).Image(&LogoBrush) ] ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 28, 0, 0)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuNewGame", "STARTA NYTT SPEL"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::StartClicked)) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuLoad", "LADDA SPEL"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::LoadClicked)) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuSettings", "INSTÄLLNINGAR"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::SettingsClicked)) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuQuit", "STÄNG AV"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::QuitClicked)) ] ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SAssignNew(LoadPanel, SVerticalBox).Visibility(EVisibility::Collapsed)
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 18.0f)
          [ SNew(STextBlock).Text(NSLOCTEXT("TMOP", "MainMenuLoadHeading", "LADDA SPEL"))
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuLoadHeading"),
                FCoreStyle::GetDefaultFontStyle("Bold", 28)))
            .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                TEXT("MainMenuLoadHeading"), FLinearColor::White)) ]
          + SVerticalBox::Slot().FillHeight(1.0f)
          [ SNew(SBox).WidthOverride(900.0f).HeightOverride(620.0f)
            [ SNew(SBorder).Padding(18.0f)
              [ SNew(SScrollBox)
                + SScrollBox::Slot()[SAssignNew(LoadListBox, SVerticalBox)] ] ] ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 10.0f)
          [ SAssignNew(LoadStatusText, STextBlock)
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuLoadStatus"),
                FCoreStyle::GetDefaultFontStyle("Regular", 14)))
            .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                TEXT("MainMenuLoadStatus"), MenuColors.StatusText)) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 18.0f, 0.0f, 0.0f)
          [ SNew(SButton).Text(NSLOCTEXT("TMOP", "MainMenuLoadBack", "TILLBAKA"))
            .OnClicked_UObject(this, &UTMOPMainMenuWidget::LoadBackClicked) ] ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SAssignNew(IntroPanel, SBorder).Visibility(EVisibility::Collapsed)
          .BorderBackgroundColor(MenuColors.IntroCardBackground)
          .Padding(FMargin(22.0f))
          [ SNew(SBox).WidthOverride(820.0f)
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                .Padding(0.0f, 0.0f, 0.0f, 18.0f)
              [ SAssignNew(IntroImageBox, SBox)
                .WidthOverride(320.0f).HeightOverride(180.0f)
                .Visibility(EVisibility::Collapsed)
                [ SAssignNew(IntroImage, SImage).Image(&CardImageBrush) ] ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
              [ SAssignNew(IntroHeading, STextBlock)
                .Justification(ETextJustify::Center)
                .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("IntroCardHeading"),
                    FCoreStyle::GetDefaultFontStyle("Bold", 24))) ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Fill)
                .Padding(0.0f, 8.0f)
              [ SAssignNew(IntroBody, STextBlock).AutoWrapText(true)
                .WrapTextAt(776.0f).Justification(ETextJustify::Center)
                .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("IntroCardBody"),
                    FCoreStyle::GetDefaultFontStyle("Regular", 17))) ] ] ] ]
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)
          .Padding(0.0f, 0.0f, 30.0f, 30.0f)
        [ SAssignNew(IntroSkipButton, SButton)
          .Visibility(EVisibility::Collapsed)
          .ContentPadding(FMargin(16.0f, 7.0f))
          .OnClicked_UObject(this, &UTMOPMainMenuWidget::SkipIntroClicked)
          [ SNew(STextBlock)
            .Text(NSLOCTEXT("TMOP", "IntroSkip", "SKIP"))
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("IntroSkipButton"),
                FCoreStyle::GetDefaultFontStyle("Regular", 14)))
            .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                TEXT("IntroSkipButton"), FLinearColor::White)) ] ];
    ApplyIntroTextStyle();
    return Root;
}

void UTMOPMainMenuWidget::ApplyIntroTextStyle()
{
    if (IntroHeading.IsValid())
    {
        FSlateFontInfo Font = ATMOPTypographyDirector::ResolveFont(this,
            IntroTextSettings.HeadingStyleId,
            FCoreStyle::GetDefaultFontStyle("Bold", IntroTextSettings.HeadingFontSize));
        Font.Size = IntroTextSettings.HeadingFontSize;
        Font.TypefaceFontName = IntroTextSettings.HeadingTypeface;
        if (IsValid(IntroTextSettings.HeadingFontAsset.Get()))
            Font.FontObject = IntroTextSettings.HeadingFontAsset.Get();
        IntroHeading->SetFont(Font);
        IntroHeading->SetColorAndOpacity(IntroTextSettings.HeadingColor);
    }
    if (IntroBody.IsValid())
    {
        FSlateFontInfo Font = ATMOPTypographyDirector::ResolveFont(this,
            IntroTextSettings.BodyStyleId,
            FCoreStyle::GetDefaultFontStyle("Regular", IntroTextSettings.BodyFontSize));
        Font.Size = IntroTextSettings.BodyFontSize;
        Font.TypefaceFontName = IntroTextSettings.BodyTypeface;
        if (IsValid(IntroTextSettings.BodyFontAsset.Get()))
            Font.FontObject = IntroTextSettings.BodyFontAsset.Get();
        IntroBody->SetFont(Font);
        IntroBody->SetColorAndOpacity(IntroTextSettings.BodyColor);
    }
}

void UTMOPMainMenuWidget::ResetTypewriter()
{
    TypewriterCharacterAccumulator = 0.0f;
    RevealedHeadingCharacters = IntroTextSettings.bUseTypewriter &&
        IntroTextSettings.bTypewriterHeading ? 0 : FullIntroHeading.Len();
    RevealedBodyCharacters = IntroTextSettings.bUseTypewriter
        ? 0 : FullIntroBody.Len();
    if (IntroHeading.IsValid()) IntroHeading->SetText(FText::FromString(
        FullIntroHeading.Left(RevealedHeadingCharacters)));
    if (IntroBody.IsValid()) IntroBody->SetText(FText::FromString(
        FullIntroBody.Left(RevealedBodyCharacters)));
}

void UTMOPMainMenuWidget::SetMenuMode(const bool bShowMenu)
{
    if (MenuPanel.IsValid()) MenuPanel->SetVisibility(
        bShowMenu ? EVisibility::Visible : EVisibility::Collapsed);
    if (IntroPanel.IsValid() && bShowMenu)
        IntroPanel->SetVisibility(EVisibility::Collapsed);
    if (LoadPanel.IsValid() && bShowMenu)
        LoadPanel->SetVisibility(EVisibility::Collapsed);
    if (IntroSkipButton.IsValid() && bShowMenu)
        IntroSkipButton->SetVisibility(EVisibility::Collapsed);
}

void UTMOPMainMenuWidget::SetIntroControlsVisible(const bool bVisible)
{
    if (IntroSkipButton.IsValid()) IntroSkipButton->SetVisibility(
        bVisible ? EVisibility::Visible : EVisibility::Collapsed);
}

void UTMOPMainMenuWidget::SetLoadMenuMode(const bool bShowLoadMenu)
{
    if (MenuPanel.IsValid()) MenuPanel->SetVisibility(
        bShowLoadMenu ? EVisibility::Collapsed : EVisibility::Visible);
    if (IntroPanel.IsValid()) IntroPanel->SetVisibility(EVisibility::Collapsed);
    if (LoadPanel.IsValid()) LoadPanel->SetVisibility(
        bShowLoadMenu ? EVisibility::Visible : EVisibility::Collapsed);
    if (bShowLoadMenu)
    {
        if (LoadStatusText.IsValid()) LoadStatusText->SetText(FText::GetEmpty());
        RebuildLoadList();
    }
}

void UTMOPMainMenuWidget::SetLoadStatus(const FText& Status)
{
    if (LoadStatusText.IsValid()) LoadStatusText->SetText(Status);
}

void UTMOPMainMenuWidget::RebuildLoadList()
{
    if (!LoadListBox.IsValid() || !Director.IsValid()) return;
    LoadListBox->ClearChildren();
    const TArray<FTMOPSaveSlotInfo> Slots = FTMOPSaveGameService::FindSaveSlots(
        Director->ManualSaveSlotPrefix, Director->ManualSaveSlotCount,
        Director->SaveSlotName);
    if (Slots.IsEmpty())
    {
        LoadListBox->AddSlot().AutoHeight().HAlign(HAlign_Center).Padding(8.0f)
        [ SNew(STextBlock).Text(NSLOCTEXT("TMOP", "MainMenuNoSaves",
            "Det finns inga sparade spel ännu."))
          .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuSaveDetails"),
              FCoreStyle::GetDefaultFontStyle("Regular", 15)))
          .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
              TEXT("MainMenuSaveDetails"), FLinearColor(0.8f, 0.82f, 0.85f))) ];
        return;
    }
    for (const FTMOPSaveSlotInfo& Info : Slots)
    {
        const FString Detail = FString::Printf(TEXT("Plats: %s   •   Nivå: %s   •   Sparad: %s"),
            *Info.LocationName,
            Info.MapName.IsEmpty() ? TEXT("Okänd") : *Info.MapName,
            Info.SavedAtText.IsEmpty() ? TEXT("Äldre sparfil") : *Info.SavedAtText);
        LoadListBox->AddSlot().AutoHeight().Padding(4.0f)
        [ SNew(SButton)
          .OnClicked_UObject(this, &UTMOPMainMenuWidget::LoadSlotClicked, Info.SlotName)
          [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock)
              .Text(FText::FromString(FString::Printf(TEXT("%s   —   %s"),
                  *Info.DisplayName, *Info.GameTime.ToDisplayString())))
              .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuSaveTitle"),
                  FCoreStyle::GetDefaultFontStyle("Bold", 18)))
              .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                  TEXT("MainMenuSaveTitle"), FLinearColor::White)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [ SNew(STextBlock).Text(FText::FromString(Detail))
              .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuSaveDetails"),
                  FCoreStyle::GetDefaultFontStyle("Regular", 14)))
              .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                  TEXT("MainMenuSaveDetails"),
                  FLinearColor(0.8f, 0.82f, 0.85f))) ] ] ];
    }
}

void UTMOPMainMenuWidget::SetIntroCard(const FText& Heading,
    const FText& Body, UTexture2D* Image, const bool bVisible)
{
    if (IntroPanel.IsValid()) IntroPanel->SetVisibility(
        bVisible ? EVisibility::Visible : EVisibility::Collapsed);
    bIntroCardVisible = bVisible;
    FullIntroHeading = Heading.ToString();
    FullIntroBody = Body.ToString();
    ResetTypewriter();
    CardTexture = Image;
    CardImageBrush.SetResourceObject(CardTexture);
    if (IntroImage.IsValid())
    {
        IntroImage->SetImage(&CardImageBrush);
        IntroImage->SetVisibility(EVisibility::Visible);
    }
    if (IntroImageBox.IsValid()) IntroImageBox->SetVisibility(IsValid(Image)
        ? EVisibility::Visible : EVisibility::Collapsed);
}

FReply UTMOPMainMenuWidget::StartClicked()
{ if (Director.IsValid()) Director->StartNewGame(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::LoadClicked()
{ if (Director.IsValid()) Director->LoadGame(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::LoadSlotClicked(FString SlotName)
{ if (Director.IsValid()) Director->LoadGameSlot(SlotName); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::LoadBackClicked()
{ if (Director.IsValid()) Director->CloseLoadGameMenu(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::SettingsClicked()
{ if (Director.IsValid()) Director->OpenSettings(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::QuitClicked()
{ if (Director.IsValid()) Director->QuitGame(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::SkipIntroClicked()
{ if (Director.IsValid()) Director->SkipIntro(); return FReply::Handled(); }
