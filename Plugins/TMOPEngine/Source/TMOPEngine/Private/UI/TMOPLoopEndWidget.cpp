#include "UI/TMOPLoopEndWidget.h"
#include "Localization/TMOPLocalization.h"
#include "UI/TMOPLocalPanel.h"

#include "Framework/Application/SlateApplication.h"
#include "InputCoreTypes.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "UI/TMOPTypographyDirector.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPLoopEndWidget::InitializeLoopEnd(ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    SetIsFocusable(true);
}

void UTMOPLoopEndWidget::SetMenuVisible(const bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    // Focus is assigned by the owning player, never to the global keyboard user.
}

TSharedRef<SWidget> UTMOPLoopEndWidget::RebuildWidget()
{
    const FTMOPMenuColorPalette Colors = ATMOPTypographyDirector::ResolveMenuColors(this);
    const FSlateFontInfo ButtonFont = ATMOPTypographyDirector::ResolveFont(this,
        TEXT("LoopEndButton"), FCoreStyle::GetDefaultFontStyle("Regular", 19));
    const FSlateColor ButtonText = ATMOPTypographyDirector::ResolveColor(this,
        TEXT("LoopEndButton"), Colors.PauseMenuButtonText);

    auto MakeButton = [&Colors, &ButtonFont, &ButtonText](const FText& Label,
        const FOnClicked& Clicked)
    {
        return SNew(SButton)
            .ButtonColorAndOpacity(Colors.ButtonBackground)
            .ContentPadding(FMargin(28.0f, 12.0f))
            .HAlign(HAlign_Center)
            .OnClicked(Clicked)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(Label)).Font(ButtonFont)
              .ColorAndOpacity(ButtonText) ];
    };

    return TMOPFitLocalPanel(this, SNew(SOverlay)
        + SOverlay::Slot()
        [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.82f)) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(32.0f)
        [ SNew(SBox).WidthOverride(670.0f)
          [ SNew(SBorder).BorderBackgroundColor(Colors.MenuBackground).Padding(42.0f)
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
              [ SNew(STextBlock)
                .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "LoopEndTitle", "LOOPEN ÄR SLUT")))
                .Font(ATMOPTypographyDirector::ResolveFont(this,
                    TEXT("LoopEndTitle"), FCoreStyle::GetDefaultFontStyle("Bold", 31)))
                .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                    TEXT("LoopEndTitle"), Colors.AccentText)) ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                .Padding(0.0f, 12.0f, 0.0f, 28.0f)
              [ SNew(STextBlock)
                .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "LoopEndQuestion",
                    "Klockan är 23:45. Vad vill du göra?")))
                .Font(ATMOPTypographyDirector::ResolveFont(this,
                    TEXT("LoopEndBody"), FCoreStyle::GetDefaultFontStyle("Regular", 18)))
                .ColorAndOpacity(FLinearColor::White) ]
              + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f)
              [ SAssignNew(ReplayButton, SButton)
                .ButtonColorAndOpacity(Colors.ButtonBackground)
                .ContentPadding(FMargin(28.0f, 12.0f))
                .HAlign(HAlign_Center)
                .OnClicked_UObject(this, &UTMOPLoopEndWidget::HandleReplayClicked)
                [ SNew(STextBlock)
                  .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "LoopEndReplay", "SPELA OM FRÅN BÖRJAN")))
                  .Font(ButtonFont).ColorAndOpacity(ButtonText) ] ]
              + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f)
              [ MakeButton(NSLOCTEXT("TMOP", "LoopEndMainMenu", "GÅ TILL HUVUDMENYN"),
                    FOnClicked::CreateUObject(this,
                        &UTMOPLoopEndWidget::HandleMainMenuClicked)) ]
              + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f)
              [ MakeButton(NSLOCTEXT("TMOP", "LoopEndQuit", "AVSLUTA SPELET"),
                    FOnClicked::CreateUObject(this,
                        &UTMOPLoopEndWidget::HandleQuitClicked)) ] ] ] ]);
}

FReply UTMOPLoopEndWidget::HandleReplayClicked()
{
    if (IsValid(PlayerCharacter)) PlayerCharacter->ReplayLoopFromBeginning();
    return FReply::Handled();
}

FReply UTMOPLoopEndWidget::HandleMainMenuClicked()
{
    if (IsValid(PlayerCharacter)) PlayerCharacter->ReturnToMainMenuFromLoopEnd();
    return FReply::Handled();
}

FReply UTMOPLoopEndWidget::HandleQuitClicked()
{
    if (IsValid(PlayerCharacter)) PlayerCharacter->QuitFromLoopEnd();
    return FReply::Handled();
}

FReply UTMOPLoopEndWidget::NativeOnKeyDown(
    const FGeometry& Geometry, const FKeyEvent& KeyEvent)
{
    // The end state must always be resolved through one of the three choices.
    if (KeyEvent.GetKey() == EKeys::Escape ||
        KeyEvent.GetKey() == EKeys::Gamepad_FaceButton_Right ||
        KeyEvent.GetKey() == EKeys::Gamepad_Special_Right)
        return FReply::Handled();
    return Super::NativeOnKeyDown(Geometry, KeyEvent);
}

void UTMOPLoopEndWidget::ReleaseSlateResources(const bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    ReplayButton.Reset();
}
