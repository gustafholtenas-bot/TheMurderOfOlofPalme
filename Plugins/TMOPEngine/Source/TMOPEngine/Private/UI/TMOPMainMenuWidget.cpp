#include "UI/TMOPMainMenuWidget.h"

#include "Engine/Texture2D.h"
#include "Styling/CoreStyle.h"
#include "UI/TMOPMainMenuIntroDirector.h"
#include "UI/TMOPTypographyDirector.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
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

TSharedRef<SWidget> UTMOPMainMenuWidget::RebuildWidget()
{
    const FSlateFontInfo ButtonFont = ATMOPTypographyDirector::ResolveFont(
        this, TEXT("MenuButton"), FCoreStyle::GetDefaultFontStyle("Regular", 21));
    auto MenuButton = [this, &ButtonFont](const FText& Label,
        FOnClicked Clicked)
    {
        return SNew(SButton).ButtonStyle(FCoreStyle::Get(), "NoBorder")
            .ContentPadding(FMargin(20.0f, 8.0f)).OnClicked(Clicked)
            [ SNew(STextBlock).Text(Label).Font(ButtonFont)
              .ColorAndOpacity(FLinearColor::White) ];
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
        + SOverlay::Slot().HAlign(HAlign_Left).VAlign(VAlign_Bottom)
          .Padding(70.0f, 70.0f)
        [ SAssignNew(IntroPanel, SBorder).Visibility(EVisibility::Collapsed)
          .BorderBackgroundColor(FLinearColor(0.01f, 0.015f, 0.025f, 0.82f))
          .Padding(FMargin(22.0f))
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(260.0f).HeightOverride(150.0f)
              [ SAssignNew(IntroImage, SImage).Image(&CardImageBrush)
                .Visibility(EVisibility::Collapsed) ] ]
            + SHorizontalBox::Slot().FillWidth(1.0f).Padding(22.0f, 0.0f)
            [ SNew(SBox).WidthOverride(620.0f)
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SAssignNew(IntroHeading, STextBlock)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("Heading"),
                      FCoreStyle::GetDefaultFontStyle("Bold", 24))) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f)
                [ SAssignNew(IntroBody, STextBlock).AutoWrapText(true)
                  .WrapTextAt(600.0f)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("Body"),
                      FCoreStyle::GetDefaultFontStyle("Regular", 17))) ] ] ] ] ];
    return Root;
}

void UTMOPMainMenuWidget::SetMenuMode(const bool bShowMenu)
{
    if (MenuPanel.IsValid()) MenuPanel->SetVisibility(
        bShowMenu ? EVisibility::Visible : EVisibility::Collapsed);
    if (IntroPanel.IsValid() && bShowMenu)
        IntroPanel->SetVisibility(EVisibility::Collapsed);
}

void UTMOPMainMenuWidget::SetIntroCard(const FText& Heading,
    const FText& Body, UTexture2D* Image, const bool bVisible)
{
    if (IntroPanel.IsValid()) IntroPanel->SetVisibility(
        bVisible ? EVisibility::Visible : EVisibility::Collapsed);
    if (IntroHeading.IsValid()) IntroHeading->SetText(Heading);
    if (IntroBody.IsValid()) IntroBody->SetText(Body);
    CardTexture = Image;
    CardImageBrush.SetResourceObject(CardTexture);
    if (IntroImage.IsValid())
    {
        IntroImage->SetImage(&CardImageBrush);
        IntroImage->SetVisibility(IsValid(Image)
            ? EVisibility::Visible : EVisibility::Collapsed);
    }
}

FReply UTMOPMainMenuWidget::StartClicked()
{ if (Director.IsValid()) Director->StartNewGame(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::LoadClicked()
{ if (Director.IsValid()) Director->LoadGame(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::SettingsClicked()
{ if (Director.IsValid()) Director->OpenSettings(); return FReply::Handled(); }
FReply UTMOPMainMenuWidget::QuitClicked()
{ if (Director.IsValid()) Director->QuitGame(); return FReply::Handled(); }
