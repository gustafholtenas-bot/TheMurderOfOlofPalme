#include "UI/TMOPAddressDirectoryWidget.h"

#include "InputCoreTypes.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "UI/TMOPTypographyDirector.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPAddressDirectoryWidget::InitializeDirectory(ATMOPPlayerCharacter* InPlayer)
{
    Player = InPlayer;
    SetIsFocusable(true);
}

void UTMOPAddressDirectoryWidget::ShowDirectory(const FText& Address, const FText& Residents)
{
    // The public directory formatter includes a heading for other consumers.
    FString Body = Residents.ToString();
    const FString Heading = Address.ToString() + TEXT("\n");
    if (Body.StartsWith(Heading)) Body.RightChopInline(Heading.Len());
    if (Body.TrimStartAndEnd().IsEmpty()) Body = TEXT("Inga registrerade boende.");
    ShowInformation(Address, FText::FromString(Body),
        NSLOCTEXT("TMOP", "AddressResidents", "Boende"), FText::GetEmpty());
}

void UTMOPAddressDirectoryWidget::ShowInformation(const FText& Title, const FText& Body,
    const FText& Category, const FText& Source)
{
    AddressText = Title;
    ResidentsText = Body;
    ReadingCategory = Category;
    ReadingSource = Source.ToString().TrimStartAndEnd().IsEmpty() ? FText::GetEmpty()
        : FText::Format(NSLOCTEXT("TMOP", "InformationSource", "Källa: {0}"), Source);
    if (TitleText.IsValid()) TitleText->SetText(AddressText);
    if (DirectoryText.IsValid()) DirectoryText->SetText(ResidentsText);
    if (CategoryText.IsValid()) CategoryText->SetText(ReadingCategory);
    if (SourceText.IsValid()) SourceText->SetText(ReadingSource);
    if (ScrollBox.IsValid()) ScrollBox->ScrollToStart();
    SetVisibility(ESlateVisibility::Visible);
}

void UTMOPAddressDirectoryWidget::HideDirectory()
{
    SetVisibility(ESlateVisibility::Collapsed);
}

TSharedRef<SWidget> UTMOPAddressDirectoryWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Center).Padding(24.0f)
        [ SNew(SBox).WidthOverride(800.0f).HeightOverride(580.0f)
          [ SNew(SBorder).Padding(26.0f)
            .BorderBackgroundColor(FLinearColor(0.02f, 0.025f, 0.035f, 0.97f))
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 16.0f)
              [ SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [ SAssignNew(TitleText, STextBlock).Text(AddressText).AutoWrapText(true)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AddressTitle"),
                      FCoreStyle::GetDefaultFontStyle("Bold", 24)))
                  .ColorAndOpacity(FLinearColor(0.95f, 0.79f, 0.48f)) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(16.0f, 0.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "CloseAddressDirectory", "Stäng"))
                  .OnClicked_UObject(this, &UTMOPAddressDirectoryWidget::HandleClose) ] ]
              + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 12.0f)
              [ SAssignNew(CategoryText, STextBlock).Text(ReadingCategory).AutoWrapText(true)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)) ]
              + SVerticalBox::Slot().FillHeight(1.0f)
              [ SAssignNew(ScrollBox, SScrollBox)
                + SScrollBox::Slot().Padding(0.0f, 0.0f, 14.0f, 0.0f)
                [ SNew(SVerticalBox)
                  + SVerticalBox::Slot().AutoHeight()
                  [ SAssignNew(DirectoryText, STextBlock).Text(ResidentsText)
                    .AutoWrapText(true).WrapTextAt(710.0f).LineHeightPercentage(1.35f)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("AddressResidents"),
                        FCoreStyle::GetDefaultFontStyle("Regular", 18)))
                    .ColorAndOpacity(FLinearColor::White) ]
                  + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 20.0f, 0.0f, 0.0f)
                  [ SAssignNew(SourceText, STextBlock).Text(ReadingSource)
                    .Visibility_Lambda([this]() { return ReadingSource.IsEmpty()
                        ? EVisibility::Collapsed : EVisibility::Visible; })
                    .AutoWrapText(true).WrapTextAt(710.0f)
                    .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
                    .ColorAndOpacity(FLinearColor(0.72f, 0.76f, 0.8f)) ] ] ]
              + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 16.0f, 0.0f, 0.0f)
              [ SNew(STextBlock)
                .Text_Lambda([this]() { return FText::Format(
                    NSLOCTEXT("TMOP", "AddressCloseHint", "{0} / Esc — Stäng"),
                    Player.IsValid() ? Player->GetInteractKeyDisplayText() : FText::FromString(TEXT("E"))); })
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 14))
                .ColorAndOpacity(FLinearColor(0.7f, 0.73f, 0.76f)) ] ] ] ];
}

FReply UTMOPAddressDirectoryWidget::HandleClose()
{
    if (Player.IsValid()) Player->CloseInformation();
    else HideDirectory();
    return FReply::Handled();
}

FReply UTMOPAddressDirectoryWidget::NativeOnPreviewKeyDown(const FGeometry& Geometry,
    const FKeyEvent& KeyEvent)
{
    const FKey Key = KeyEvent.GetKey();
    if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right ||
        (Player.IsValid() && Key == Player->InteractFallbackKey))
    {
        if (!KeyEvent.IsRepeat()) return HandleClose();
        return FReply::Handled();
    }
    return Super::NativeOnPreviewKeyDown(Geometry, KeyEvent);
}

void UTMOPAddressDirectoryWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    TitleText.Reset();
    DirectoryText.Reset();
    CategoryText.Reset();
    SourceText.Reset();
    ScrollBox.Reset();
}
