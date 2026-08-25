#include "UI/TMOPNewspaperReaderWidget.h"

#include "InputCoreTypes.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"
#include "Newspapers/TMOPNewspaperReadingComponent.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UTMOPNewspaperReaderWidget::InitializeReader(
    ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    SetIsFocusable(true);
}

bool UTMOPNewspaperReaderWidget::OpenNewspaper(
    UTMOPNewspaperItemDefinition* InNewspaper)
{
    if (!IsValid(InNewspaper) || InNewspaper->Pages.IsEmpty()) return false;
    Newspaper = InNewspaper;
    CurrentPageIndex = 0;
    Zoom = FMath::Clamp(InNewspaper->InitialZoom, 0.25f, 4.0f);
    SetVisibility(ESlateVisibility::Visible);
    RefreshPage();
    return true;
}

void UTMOPNewspaperReaderWidget::CloseReader()
{
    if (IsValid(PlayerCharacter.Get()))
    {
        PlayerCharacter->CloseNewspaper();
        return;
    }
    DismissReader();
}

void UTMOPNewspaperReaderWidget::DismissReader()
{
    Newspaper = nullptr;
    SetVisibility(ESlateVisibility::Collapsed);
}

int32 UTMOPNewspaperReaderWidget::GetPageCount() const
{
    return IsValid(Newspaper.Get()) ? Newspaper->Pages.Num() : 0;
}

bool UTMOPNewspaperReaderWidget::GoToPage(const int32 PageIndex)
{
    if (!IsValid(Newspaper.Get()) ||
        !Newspaper->Pages.IsValidIndex(PageIndex)) return false;
    CurrentPageIndex = PageIndex;
    RefreshPage();
    return true;
}

bool UTMOPNewspaperReaderWidget::NextPage()
{
    return GoToPage(CurrentPageIndex + 2);
}

bool UTMOPNewspaperReaderWidget::PreviousPage()
{
    return GoToPage(CurrentPageIndex - 2);
}

void UTMOPNewspaperReaderWidget::SetZoom(const float NewZoom)
{
    Zoom = FMath::Clamp(NewZoom, 0.25f, 4.0f);
}

TSharedRef<SWidget> UTMOPNewspaperReaderWidget::RebuildWidget()
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor::Transparent)
        .Padding(18.0f)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f, 8.0f, 12.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                [
                    SAssignNew(TitleText, STextBlock)
                    .Font(FCoreStyle::GetDefaultFontStyle("Bold", 23))
                    .ColorAndOpacity(FLinearColor(0.92f, 0.90f, 0.82f))
                ]
                + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
                [ SNew(SButton).Text(FText::FromString(TEXT("−")))
                    .ToolTipText(NSLOCTEXT("TMOP", "NewspaperZoomOut", "Zooma ut"))
                    .OnClicked_UObject(this, &UTMOPNewspaperReaderWidget::HandleZoomOutClicked) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f)
                [ SNew(SButton).Text(FText::FromString(TEXT("+")))
                    .ToolTipText(NSLOCTEXT("TMOP", "NewspaperZoomIn", "Zooma in"))
                    .OnClicked_UObject(this, &UTMOPNewspaperReaderWidget::HandleZoomInClicked) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(16.0f, 0.0f, 0.0f, 0.0f)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "CloseNewspaper", "Stäng"))
                    .OnClicked_UObject(this, &UTMOPNewspaperReaderWidget::HandleCloseClicked) ]
            ]
            + SVerticalBox::Slot().FillHeight(1.0f)[ SNew(SSpacer) ]
            + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 12.0f, 8.0f, 2.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "PreviousNewspaperPage", "Q  Föregående sida"))
                    .OnClicked_UObject(this, &UTMOPNewspaperReaderWidget::HandlePreviousClicked) ]
                + SHorizontalBox::Slot().FillWidth(1.0f).HAlign(HAlign_Center).VAlign(VAlign_Center)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                    [ SAssignNew(PageNumberText, STextBlock)
                        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 17)) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
                    [ SAssignNew(PageLabelText, STextBlock)
                        .ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f)) ]
                    + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0.0f, 3.0f)
                    [ SNew(STextBlock)
                        .Text(NSLOCTEXT("TMOP", "NewspaperReaderControls",
                            "Pilar/WASD: flytta  •  Mushjul/+/−: zoom  •  Q/E: vänd blad  •  Esc: stäng"))
                        .ColorAndOpacity(FLinearColor(0.58f, 0.58f, 0.58f)) ]
                ]
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "NextNewspaperPage", "Nästa sida  E"))
                    .OnClicked_UObject(this, &UTMOPNewspaperReaderWidget::HandleNextClicked) ]
            ]
        ];
}

FReply UTMOPNewspaperReaderWidget::PanPage(
    const float HorizontalDirection, const float VerticalDirection)
{
    if (IsValid(PlayerCharacter.Get()) && IsValid(PlayerCharacter->NewspaperReading))
        PlayerCharacter->NewspaperReading->Pan(HorizontalDirection, VerticalDirection);
    return FReply::Handled();
}

void UTMOPNewspaperReaderWidget::RefreshPage()
{
    if (!IsValid(Newspaper.Get()) ||
        !Newspaper->Pages.IsValidIndex(CurrentPageIndex)) return;
    const FTMOPNewspaperPage& Page = Newspaper->Pages[CurrentPageIndex];
    if (IsValid(PlayerCharacter.Get()) && IsValid(PlayerCharacter->NewspaperReading))
        PlayerCharacter->NewspaperReading->ShowPage(CurrentPageIndex, true);

    if (TitleText.IsValid())
    {
        const FString DateSuffix = Newspaper->PublicationDate.IsEmpty()
            ? FString() : TEXT(" — ") + Newspaper->PublicationDate;
        TitleText->SetText(FText::FromString(
            Newspaper->DisplayName.ToString() + DateSuffix));
    }
    if (PageNumberText.IsValid())
        PageNumberText->SetText(FText::Format(
            NSLOCTEXT("TMOP", "NewspaperPageCounter", "Uppslag {0}–{1} av {2} sidor"),
            FText::AsNumber(CurrentPageIndex + 1),
            FText::AsNumber(FMath::Min(CurrentPageIndex + 2, Newspaper->Pages.Num())),
            FText::AsNumber(Newspaper->Pages.Num())));
    if (PageLabelText.IsValid())
    {
        const FText Label = Page.PageLabel.IsEmpty()
            ? FText::Format(NSLOCTEXT("TMOP", "PrintedNewspaperPage", "Tryckt sida {0}"),
                FText::AsNumber(Page.PrintedPageNumber))
            : Page.PageLabel;
        PageLabelText->SetText(Label);
    }
}

FReply UTMOPNewspaperReaderWidget::HandlePreviousClicked()
{ PreviousPage(); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleNextClicked()
{ NextPage(); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleZoomOutClicked()
{ SetZoom(Zoom - 0.25f); if (IsValid(PlayerCharacter.Get()) && IsValid(PlayerCharacter->NewspaperReading)) PlayerCharacter->NewspaperReading->Zoom(-1.0f); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleZoomInClicked()
{ SetZoom(Zoom + 0.25f); if (IsValid(PlayerCharacter.Get()) && IsValid(PlayerCharacter->NewspaperReading)) PlayerCharacter->NewspaperReading->Zoom(1.0f); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleCloseClicked()
{ CloseReader(); return FReply::Handled(); }

FReply UTMOPNewspaperReaderWidget::NativeOnKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Escape) return HandleCloseClicked();
    if (Key == EKeys::Right || Key == EKeys::D) return PanPage(1.0f, 0.0f);
    if (Key == EKeys::Left || Key == EKeys::A) return PanPage(-1.0f, 0.0f);
    if (Key == EKeys::Up || Key == EKeys::W) return PanPage(0.0f, -1.0f);
    if (Key == EKeys::Down || Key == EKeys::S) return PanPage(0.0f, 1.0f);
    if (Key == EKeys::E || Key == EKeys::PageDown || Key == EKeys::SpaceBar)
        return HandleNextClicked();
    if (Key == EKeys::Q || Key == EKeys::PageUp)
        return HandlePreviousClicked();
    if (Key == EKeys::Add || Key == EKeys::Equals)
        return HandleZoomInClicked();
    if (Key == EKeys::Subtract || Key == EKeys::Hyphen)
        return HandleZoomOutClicked();
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UTMOPNewspaperReaderWidget::NativeOnMouseWheel(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return InMouseEvent.GetWheelDelta() > 0.0f
        ? HandleZoomInClicked() : HandleZoomOutClicked();
}
