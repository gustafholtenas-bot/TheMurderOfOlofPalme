#include "UI/TMOPPauseMenuWidget.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPPauseMenuWidget::InitializePauseMenu(APlayerController* InPlayerController,
    ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerController = InPlayerController;
    PlayerCharacter = InPlayerCharacter;
}

void UTMOPPauseMenuWidget::SetMenuVisible(const bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (bVisible) SetStatus(FText::GetEmpty());
}

TSharedRef<SWidget> UTMOPPauseMenuWidget::RebuildWidget()
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor(0.01f, 0.015f, 0.025f, 0.96f))
        .Padding(48.0f)
        [
            SNew(SBox)
            .WidthOverride(440.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 10.0f, 8.0f, 28.0f)
                [ SNew(STextBlock).Text(NSLOCTEXT("TMOP", "PauseTitle", "PAUS"))
                    .Justification(ETextJustify::Center) ]
                + SVerticalBox::Slot().AutoHeight().Padding(6.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "Resume", "Fortsätt"))
                    .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleResumeClicked) ]
                + SVerticalBox::Slot().AutoHeight().Padding(6.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "Settings", "Inställningar"))
                    .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleSettingsClicked) ]
                + SVerticalBox::Slot().AutoHeight().Padding(6.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "Save", "Spara"))
                    .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleSaveClicked) ]
                + SVerticalBox::Slot().AutoHeight().Padding(6.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "Load", "Ladda"))
                    .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleLoadClicked) ]
                + SVerticalBox::Slot().AutoHeight().Padding(6.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "Quit", "Avsluta"))
                    .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleQuitClicked) ]
                + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 20.0f, 8.0f, 4.0f)
                [ SAssignNew(StatusText, STextBlock).Justification(ETextJustify::Center) ]
            ]
        ];
}

FReply UTMOPPauseMenuWidget::HandleResumeClicked()
{
    if (IsValid(PlayerCharacter.Get())) PlayerCharacter->SetPauseMenuOpen(false);
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleSettingsClicked()
{
    OnSettingsRequested.Broadcast();
    SetStatus(NSLOCTEXT("TMOP", "SettingsPending", "Inställningspanelen kopplas i nästa steg."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleSaveClicked()
{
    OnSaveRequested.Broadcast();
    SetStatus(NSLOCTEXT("TMOP", "SavePending", "Save-systemets data kopplas i en senare sprint."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleLoadClicked()
{
    OnLoadRequested.Broadcast();
    SetStatus(NSLOCTEXT("TMOP", "LoadPending", "Load-systemets data kopplas i en senare sprint."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleQuitClicked()
{
    if (IsValid(PlayerController.Get()))
        UKismetSystemLibrary::QuitGame(this, PlayerController.Get(),
            EQuitPreference::Quit, false);
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::SetStatus(const FText& Text)
{
    if (StatusText.IsValid()) StatusText->SetText(Text);
}
