#include "UI/TMOPMainMenuWidget.h"
#include "Localization/TMOPLocalization.h"
#include "UI/TMOPLanguageSelector.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "UI/TMOPMainMenuIntroDirector.h"
#include "UI/TMOPPlayerAppearancePanel.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "UI/TMOPSaveGameService.h"
#include "UI/TMOPTypographyDirector.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Text/STextBlock.h"

UTMOPMainMenuWidget::UTMOPMainMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    // SetMenuInput focuses this widget. A non-focusable root causes UIOnly to
    // reject focus and can leave keyboard/controller navigation on a stale menu.
    SetIsFocusable(true);
}

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
    if (bIntroCardVisible && IntroLanguageRevision != FTMOPLocalization::GetRevision())
    {
        IntroLanguageRevision = FTMOPLocalization::GetRevision();
        FullIntroHeading = FTMOPLocalization::String(IntroHeadingSource);
        FullIntroBody = FTMOPLocalization::String(IntroBodySource);
        ResetTypewriter();
    }
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
        if (IntroHeading.IsValid()) IntroHeading->SetText(FTMOPLocalization::Text(FText::FromString(
            FullIntroHeading.Left(RevealedHeadingCharacters))));
    }
    if (CharactersToReveal > 0 && RevealedBodyCharacters < FullIntroBody.Len())
    {
        RevealedBodyCharacters = FMath::Min(FullIntroBody.Len(),
            RevealedBodyCharacters + CharactersToReveal);
        if (IntroBody.IsValid()) IntroBody->SetText(FTMOPLocalization::Text(FText::FromString(
            FullIntroBody.Left(RevealedBodyCharacters))));
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
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(Label)).Font(ButtonFont)
              .ColorAndOpacity(ButtonTextColor) ];
    };

    TSharedRef<SWidget> Root = SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SAssignNew(MenuPanel, SVerticalBox)
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ SNew(SBox).WidthOverride(760.0f).HeightOverride(260.0f)
            [ SNew(SImage).Image(&LogoBrush) ] ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ SNew(SHorizontalBox).Visibility_Lambda([this] { return bChoosingPlayerCount ? EVisibility::Visible : EVisibility::Collapsed; })
            + SHorizontalBox::Slot().AutoWidth()
            [ MenuButton(NSLOCTEXT("TMOP", "MainMenuPlayers1", "1 SPELARE"), FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::PlayerCountClicked, 1)) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ MenuButton(NSLOCTEXT("TMOP", "MainMenuPlayers2", "2 SPELARE"), FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::PlayerCountClicked, 2)) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ MenuButton(NSLOCTEXT("TMOP", "MainMenuPlayers3", "3 SPELARE"), FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::PlayerCountClicked, 3)) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ MenuButton(NSLOCTEXT("TMOP", "MainMenuPlayers4", "4 SPELARE"), FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::PlayerCountClicked, 4)) ] ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ SNew(STextBlock).Visibility_Lambda([this] { return bChoosingPlayerCount ? EVisibility::Visible : EVisibility::Collapsed; }).Text(FTMOPLocalization::Bind([this]()
            { return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MainMenuPlayerCount", "Valt: {0} spelare · lokal delad skärm"), FText::AsNumber(Director.IsValid() ? Director->LocalPlayerCount : 1)); })) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 8)
          [ SNew(SButton).Visibility_Lambda([this] { return bChoosingPlayerCount ? EVisibility::Visible : EVisibility::Collapsed; }).OnClicked_UObject(this, &UTMOPMainMenuWidget::KeyboardModeClicked)
            [ SNew(STextBlock).Text(FTMOPLocalization::Bind([this]()
              {
                  if (!Director.IsValid() || !Director->bKeyboardForPlayerOne)
                      return NSLOCTEXT("TMOP", "MainMenuControllerMode", "Styrning: en handkontroll per spelare");
                  if (Director->LocalPlayerCount == 2 && Director->bSharedKeyboardForPlayerTwo)
                      return NSLOCTEXT("TMOP", "MainMenuSharedKeyboard", "Styrning: P1 + P2 delar tangentbordet");
                  return NSLOCTEXT("TMOP", "MainMenuKeyboardMode", "Styrning: P1 tangentbord/mus · övriga handkontroller");
              })) ] ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(FLinearColor(1,0.4f,0.2f))
            .Text(FTMOPLocalization::Bind([this]() { return Director.IsValid() ? Director->StartupStatus : FText::GetEmpty(); })) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 28, 0, 0)
          [ SNew(SButton).Text(FTMOPLocalization::Bind([this] { return bChoosingPlayerCount ? NSLOCTEXT("TMOP", "MainMenuNextAppearance", "NÄSTA: VÄLJ UTSEENDE") : NSLOCTEXT("TMOP", "MainMenuStartNew", "STARTA NYTT SPEL"); }))
              .OnClicked_UObject(this, &UTMOPMainMenuWidget::StartClicked) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "MainMenuBack", "TILLBAKA")))
              .Visibility_Lambda([this] { return bChoosingPlayerCount ? EVisibility::Visible : EVisibility::Collapsed; })
              .OnClicked_Lambda([this] { bChoosingPlayerCount = false; return FReply::Handled(); }) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuLoad", "LADDA SPEL"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::LoadClicked)) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuSettings", "INSTÄLLNINGAR"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::SettingsClicked)) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MakeTMOPLanguageSelector(GetGameInstance()) ]
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
          [ MenuButton(NSLOCTEXT("TMOP", "MainMenuQuit", "STÄNG AV"),
              FOnClicked::CreateUObject(this, &UTMOPMainMenuWidget::QuitClicked)) ] ]
        + SOverlay::Slot().Padding(20)
        [SAssignNew(AppearanceHost, SBox).Visibility(EVisibility::Collapsed)]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [ SAssignNew(LoadPanel, SVerticalBox).Visibility(EVisibility::Collapsed)
          + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 0.0f, 0.0f, 18.0f)
          [ SNew(STextBlock).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "MainMenuLoadHeading", "LADDA SPEL")))
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
          [ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "MainMenuLoadBack", "TILLBAKA")))
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
            .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "IntroSkip", "SKIP")))
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

FReply UTMOPMainMenuWidget::PlayerCountClicked(int32 Count)
{
    if (Director.IsValid()) Director->LocalPlayerCount = FMath::Clamp(Count, 1, 4);
    return FReply::Handled();
}

FReply UTMOPMainMenuWidget::KeyboardModeClicked()
{
    if (Director.IsValid())
    {
        // Cycle all supported layouts without hiding the established
        // P1-keyboard + P2-controller option.
        if (!Director->bKeyboardForPlayerOne)
        {
            Director->bKeyboardForPlayerOne = true;
            Director->bSharedKeyboardForPlayerTwo = false;
        }
        else if (!Director->bSharedKeyboardForPlayerTwo)
        {
            Director->bSharedKeyboardForPlayerTwo = true;
        }
        else
        {
            Director->bKeyboardForPlayerOne = false;
            Director->bSharedKeyboardForPlayerTwo = false;
        }
    }
    return FReply::Handled();
}

void UTMOPMainMenuWidget::ResetTypewriter()
{
    TypewriterCharacterAccumulator = 0.0f;
    RevealedHeadingCharacters = IntroTextSettings.bUseTypewriter &&
        IntroTextSettings.bTypewriterHeading ? 0 : FullIntroHeading.Len();
    RevealedBodyCharacters = IntroTextSettings.bUseTypewriter
        ? 0 : FullIntroBody.Len();
    if (IntroHeading.IsValid()) IntroHeading->SetText(FTMOPLocalization::Text(FText::FromString(
        FullIntroHeading.Left(RevealedHeadingCharacters))));
    if (IntroBody.IsValid()) IntroBody->SetText(FTMOPLocalization::Text(FText::FromString(
        FullIntroBody.Left(RevealedBodyCharacters))));
}

void UTMOPMainMenuWidget::HideAppearanceSetup()
{
    if (AppearanceHost.IsValid())
    {
        AppearanceHost->SetVisibility(EVisibility::Collapsed);
        AppearanceHost->SetContent(SNullWidget::NullWidget);
    }
    AppearanceSwitcher.Reset();
}

void UTMOPMainMenuWidget::ShowPlayerCountPage()
{
    SetMenuMode(true);
    bChoosingPlayerCount = true;
}

void UTMOPMainMenuWidget::ShowAppearanceSetup(int32 Count)
{
    if (!AppearanceHost.IsValid() || !Director.IsValid()) return;
    SetMenuMode(false);
    SetIntroControlsVisible(false);
    if (LoadPanel.IsValid()) LoadPanel->SetVisibility(EVisibility::Collapsed);
    TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
    TSharedRef<SHorizontalBox> Tabs = SNew(SHorizontalBox);
    SAssignNew(AppearanceSwitcher, SWidgetSwitcher);
    for (int32 PlayerIndex=0; PlayerIndex<FMath::Clamp(Count, 1, 4); ++PlayerIndex)
    {
        const TWeakObjectPtr<ATMOPMainMenuIntroDirector> Owner = Director;
        Tabs->AddSlot().FillWidth(1).Padding(4)[SNew(SButton)
            .Text(FTMOPLocalization::Bind([Owner, PlayerIndex] { return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MainMenuAppearancePlayer", "SPELARE {0} — {1}"), FText::AsNumber(PlayerIndex + 1), Owner.IsValid() && Owner->IsAppearanceReady(PlayerIndex) ? NSLOCTEXT("TMOP", "MainMenuAppearanceReady", "KLAR ✓") : NSLOCTEXT("TMOP", "MainMenuChooseAppearance", "VÄLJ UTSEENDE")); }))
            .OnClicked_Lambda([this, PlayerIndex] {
                if (AppearanceSwitcher.IsValid()) AppearanceSwitcher->SetActiveWidgetIndex(PlayerIndex);
                return FReply::Handled();
            })];
        auto* Player = Cast<ATMOPPlayerCharacter>(UGameplayStatics::GetPlayerCharacter(this, PlayerIndex));
        AppearanceSwitcher->AddSlot()[SNew(STMOPPlayerAppearancePanel).Player(Player).Startup(true)
            .Ready_Lambda([Owner, PlayerIndex] { return Owner.IsValid() && Owner->IsAppearanceReady(PlayerIndex); })
            .OnEdited_Lambda([Owner, PlayerIndex] { if (Owner.IsValid()) Owner->AppearanceEdited(PlayerIndex); })
            .OnReady_Lambda([Owner, PlayerIndex] { if (Owner.IsValid()) Owner->ConfirmPlayerAppearance(PlayerIndex); })];
    }
    Body->AddSlot().AutoHeight()[Tabs];
    Body->AddSlot().FillHeight(1)[AppearanceSwitcher.ToSharedRef()];
    Body->AddSlot().AutoHeight().Padding(6)[SNew(STextBlock).AutoWrapText(true)
        .Text(FTMOPLocalization::Bind([this] { return Director.IsValid() ? Director->StartupStatus : FText::GetEmpty(); }))];
    Body->AddSlot().AutoHeight().Padding(6)[SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "MainMenuBackPlayerCount", "Tillbaka till antal spelare")))
        .OnClicked_Lambda([this] { if (Director.IsValid()) Director->CancelAppearanceSetup(); return FReply::Handled(); })];
    AppearanceHost->SetContent(SNew(SBorder).Padding(12).BorderBackgroundColor(FLinearColor(0.025f,0.025f,0.03f,1))[Body]);
    AppearanceHost->SetVisibility(EVisibility::Visible);
    AppearanceSwitcher->SetActiveWidgetIndex(0);
}

void UTMOPMainMenuWidget::SetMenuMode(const bool bShowMenu)
{
    if (bShowMenu) { HideAppearanceSetup(); bChoosingPlayerCount = false; }
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
        if (LoadStatusText.IsValid()) LoadStatusText->SetText(FTMOPLocalization::Text(FText::GetEmpty()));
        RebuildLoadList();
    }
}

void UTMOPMainMenuWidget::SetLoadStatus(const FText& Status)
{
    if (LoadStatusText.IsValid()) LoadStatusText->SetText(FTMOPLocalization::Text(Status));
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
        [ SNew(STextBlock).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "MainMenuNoSaves",
            "Det finns inga sparade spel ännu.")))
          .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuSaveDetails"),
              FCoreStyle::GetDefaultFontStyle("Regular", 15)))
          .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
              TEXT("MainMenuSaveDetails"), FLinearColor(0.8f, 0.82f, 0.85f))) ];
        return;
    }
    for (const FTMOPSaveSlotInfo& Info : Slots)
    {
        const FString Detail = FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPMainMenuWidget.01e8a8aeac3fa85a", "Plats: {0}   •   Nivå: {1}   •   Sparad: {2}"), FTMOPLocalization::Text(FString(Info.LocationName)), FTMOPLocalization::Text(FString(Info.MapName.IsEmpty() ? TEXT("Okänd") : *Info.MapName)), FTMOPLocalization::Text(FString(Info.SavedAtText.IsEmpty() ? TEXT("Äldre sparfil") : *Info.SavedAtText))).ToString();
        LoadListBox->AddSlot().AutoHeight().Padding(4.0f)
        [ SNew(SButton)
          .OnClicked_UObject(this, &UTMOPMainMenuWidget::LoadSlotClicked, Info.SlotName)
          [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock)
              .Text(FTMOPLocalization::Text(FText::FromString(FString::Printf(TEXT("%s   —   %s"),
                  *Info.DisplayName, *Info.GameTime.ToDisplayString()))))
              .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("MainMenuSaveTitle"),
                  FCoreStyle::GetDefaultFontStyle("Bold", 18)))
              .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                  TEXT("MainMenuSaveTitle"), FLinearColor::White)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(Detail)))
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
    IntroHeadingSource = Heading;
    FullIntroHeading = FTMOPLocalization::String(Heading);
    IntroBodySource = Body;
    FullIntroBody = FTMOPLocalization::String(Body);
    IntroLanguageRevision = FTMOPLocalization::GetRevision();
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
{
    if (!bChoosingPlayerCount) ShowPlayerCountPage();
    else if (Director.IsValid()) Director->BeginAppearanceSetup(Director->LocalPlayerCount);
    return FReply::Handled();
}
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
