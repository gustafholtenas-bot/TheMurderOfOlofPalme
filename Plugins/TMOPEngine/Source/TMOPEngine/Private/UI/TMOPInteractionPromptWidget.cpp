#include "UI/TMOPInteractionPromptWidget.h"

#include "Styling/CoreStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

TSharedRef<SWidget> UTMOPInteractionPromptWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
            [
                SNew(SBox).WidthOverride(28.0f).HeightOverride(28.0f)
                [
                    SAssignNew(TargetReticleWidget, STextBlock)
                    .Text(FText::FromString(TEXT("□")))
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("InteractionReticle"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 28)))
                    .ColorAndOpacity(FLinearColor(0.96f, 0.96f, 0.90f, 0.96f))
                    .Justification(ETextJustify::Center)
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 8.0f)
            [
                SAssignNew(TargetInformationPanel, SBorder)
                .Padding(FMargin(16.0f, 7.0f))
                .BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.82f))
                [ SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                    [ SAssignNew(TargetTitleWidget, STextBlock).Text(TargetTitle)
                        .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("InteractionTargetName"),
                            FCoreStyle::GetDefaultFontStyle("Bold", 14)))
                        .Justification(ETextJustify::Center) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                    [ SAssignNew(TargetDetailsWidget, STextBlock).Text(TargetDetails)
                        .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("InteractionHint"),
                            FCoreStyle::GetDefaultFontStyle("Regular", 12)))
                        .ColorAndOpacity(FLinearColor(0.72f, 0.76f, 0.80f, 1.0f))
                        .Justification(ETextJustify::Center) ] ]
            ]
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 5.0f)
            [
                SAssignNew(PromptPanel, SBorder).Padding(FMargin(16.0f, 7.0f))
                .BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.82f))
                [ SAssignNew(PromptTextWidget, STextBlock).Text(PromptText)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("InteractionPrompt"),
                        FCoreStyle::GetDefaultFontStyle("Bold", 15)))
                    .Justification(ETextJustify::Center) ]
            ]
        ];
}

void UTMOPInteractionPromptWidget::SetTargetInformation(
    const FText& Title, const FText& Details)
{
    TargetTitle = Title;
    TargetDetails = Details;
    if (TargetTitleWidget.IsValid()) TargetTitleWidget->SetText(TargetTitle);
    if (TargetDetailsWidget.IsValid()) TargetDetailsWidget->SetText(TargetDetails);
    RefreshTargetVisibility();
}

void UTMOPInteractionPromptWidget::SetPromptText(const FText& NewText)
{
    if (!PromptText.EqualTo(NewText))
    {
        PromptText = NewText;
        if (PromptTextWidget.IsValid()) PromptTextWidget->SetText(PromptText);
    }
    RefreshTargetVisibility();
}

void UTMOPInteractionPromptWidget::RefreshTargetVisibility()
{
    const bool bHasTarget = !TargetTitle.IsEmpty();
    const bool bHasPrompt = !PromptText.IsEmpty();
    if (TargetInformationPanel.IsValid())
        TargetInformationPanel->SetVisibility(bHasTarget
            ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
    if (PromptPanel.IsValid())
        PromptPanel->SetVisibility(bHasPrompt
            ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
    if (TargetReticleWidget.IsValid())
        TargetReticleWidget->SetVisibility((bHasTarget || bHasPrompt)
            ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
    SetVisibility((bHasTarget || bHasPrompt)
        ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
}
