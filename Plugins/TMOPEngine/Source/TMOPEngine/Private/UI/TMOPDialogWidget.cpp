#include "UI/TMOPDialogWidget.h"

#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "InputCoreTypes.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPDialogWidget::InitializeDialog(
    ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    SetIsFocusable(true);
}

void UTMOPDialogWidget::ShowDialog(
    const FText& Speaker, const FText& Dialog)
{
    if (SpeakerText.IsValid()) SpeakerText->SetText(Speaker);
    if (DialogText.IsValid()) DialogText->SetText(Dialog);
    SetVisibility(ESlateVisibility::Visible);
}

void UTMOPDialogWidget::HideDialog()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UTMOPDialogWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        .HAlign(HAlign_Center)
        .VAlign(VAlign_Bottom)
        .Padding(48.0f, 48.0f, 48.0f, 72.0f)
        [
            SNew(SBox)
            .WidthOverride(900.0f)
            [
                SNew(SBorder)
                .BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.94f))
                .Padding(FMargin(28.0f, 22.0f))
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 12.0f)
                    [
                        SAssignNew(SpeakerText, STextBlock)
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 24))
                        .ColorAndOpacity(FLinearColor(0.86f, 0.69f, 0.30f))
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 18.0f)
                    [
                        SAssignNew(DialogText, STextBlock)
                        .Font(FCoreStyle::GetDefaultFontStyle("Regular", 21))
                        .AutoWrapText(true)
                        .WrapTextAt(820.0f)
                        .ColorAndOpacity(FLinearColor::White)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .HAlign(HAlign_Right)
                    [
                        SNew(SButton)
                        .Text(NSLOCTEXT("TMOP", "CloseDialog", "Stäng"))
                        .OnClicked_UObject(
                            this, &UTMOPDialogWidget::HandleCloseClicked)
                    ]
                ]
            ]
        ];
}

FReply UTMOPDialogWidget::HandleCloseClicked()
{
    if (IsValid(PlayerCharacter.Get()))
        PlayerCharacter->ClosePersonDialog();
    return FReply::Handled();
}

FReply UTMOPDialogWidget::NativeOnKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    if (InKeyEvent.GetKey() == EKeys::Escape)
        return HandleCloseClicked();
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
