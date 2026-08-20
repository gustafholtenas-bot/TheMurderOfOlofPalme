#include "UI/TMOPNewspaperReaderWidget.h"

#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
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
    CurrentPageTexture = nullptr;
    PageBrush.SetResourceObject(nullptr);
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
    return GoToPage(CurrentPageIndex + 1);
}

bool UTMOPNewspaperReaderWidget::PreviousPage()
{
    return GoToPage(CurrentPageIndex - 1);
}

void UTMOPNewspaperReaderWidget::SetZoom(const float NewZoom)
{
    Zoom = FMath::Clamp(NewZoom, 0.25f, 4.0f);
    if (PageImage.IsValid())
        PageImage->Invalidate(EInvalidateWidgetReason::Layout);
}

TSharedRef<SWidget> UTMOPNewspaperReaderWidget::RebuildWidget()
{
    return SNew(SBorder)
        .BorderBackgroundColor(FLinearColor(0.008f, 0.009f, 0.012f, 0.985f))
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
            + SVerticalBox::Slot().FillHeight(1.0f).Padding(4.0f)
            [
                SNew(SBorder)
                .BorderBackgroundColor(FLinearColor(0.035f, 0.035f, 0.035f, 1.0f))
                .Padding(8.0f)
                [
                    SNew(SScrollBox)
                    .Orientation(Orient_Horizontal)
                    + SScrollBox::Slot()
                    [
                        SNew(SScrollBox)
                        .Orientation(Orient_Vertical)
                        + SScrollBox::Slot()
                        [
                            SNew(SBox)
                            .WidthOverride_Lambda([this]() { return GetPageWidth(); })
                            .HeightOverride_Lambda([this]() { return GetPageHeight(); })
                            [ SAssignNew(PageImage, SImage).Image(&PageBrush) ]
                        ]
                    ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 12.0f, 8.0f, 2.0f)
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "PreviousNewspaperPage", "Föregående sida"))
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
                ]
                + SHorizontalBox::Slot().AutoWidth()
                [ SNew(SButton).Text(NSLOCTEXT("TMOP", "NextNewspaperPage", "Nästa sida"))
                    .OnClicked_UObject(this, &UTMOPNewspaperReaderWidget::HandleNextClicked) ]
            ]
        ];
}

void UTMOPNewspaperReaderWidget::RefreshPage()
{
    if (!IsValid(Newspaper.Get()) ||
        !Newspaper->Pages.IsValidIndex(CurrentPageIndex)) return;
    const FTMOPNewspaperPage& Page = Newspaper->Pages[CurrentPageIndex];
    CurrentPageTexture = Page.PageImage.LoadSynchronous();
    PageBrush.SetResourceObject(CurrentPageTexture.Get());
    PageBrush.DrawAs = ESlateBrushDrawType::Image;
    if (IsValid(CurrentPageTexture.Get()))
        PageBrush.ImageSize = FVector2D(
            CurrentPageTexture->GetSizeX(), CurrentPageTexture->GetSizeY());

    if (TitleText.IsValid())
    {
        const FString DateSuffix = Newspaper->PublicationDate.IsEmpty()
            ? FString() : TEXT(" — ") + Newspaper->PublicationDate;
        TitleText->SetText(FText::FromString(
            Newspaper->DisplayName.ToString() + DateSuffix));
    }
    if (PageNumberText.IsValid())
        PageNumberText->SetText(FText::Format(
            NSLOCTEXT("TMOP", "NewspaperPageCounter", "Sida {0} av {1}"),
            FText::AsNumber(CurrentPageIndex + 1),
            FText::AsNumber(Newspaper->Pages.Num())));
    if (PageLabelText.IsValid())
    {
        const FText Label = Page.PageLabel.IsEmpty()
            ? FText::Format(NSLOCTEXT("TMOP", "PrintedNewspaperPage", "Tryckt sida {0}"),
                FText::AsNumber(Page.PrintedPageNumber))
            : Page.PageLabel;
        PageLabelText->SetText(Label);
    }
    if (PageImage.IsValid())
        PageImage->Invalidate(EInvalidateWidgetReason::LayoutAndVolatility);
}

FOptionalSize UTMOPNewspaperReaderWidget::GetPageWidth() const
{
    if (!IsValid(CurrentPageTexture.Get())) return FOptionalSize(900.0f * Zoom);
    const float FitScale = FMath::Min(1.0f,
        1100.0f / FMath::Max(1, CurrentPageTexture->GetSizeX()));
    return FOptionalSize(CurrentPageTexture->GetSizeX() * FitScale * Zoom);
}

FOptionalSize UTMOPNewspaperReaderWidget::GetPageHeight() const
{
    if (!IsValid(CurrentPageTexture.Get())) return FOptionalSize(1200.0f * Zoom);
    const float FitScale = FMath::Min(1.0f,
        1100.0f / FMath::Max(1, CurrentPageTexture->GetSizeX()));
    return FOptionalSize(CurrentPageTexture->GetSizeY() * FitScale * Zoom);
}

FReply UTMOPNewspaperReaderWidget::HandlePreviousClicked()
{ PreviousPage(); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleNextClicked()
{ NextPage(); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleZoomOutClicked()
{ SetZoom(Zoom - 0.25f); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleZoomInClicked()
{ SetZoom(Zoom + 0.25f); return FReply::Handled(); }
FReply UTMOPNewspaperReaderWidget::HandleCloseClicked()
{ CloseReader(); return FReply::Handled(); }

FReply UTMOPNewspaperReaderWidget::NativeOnKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    const FKey Key = InKeyEvent.GetKey();
    if (Key == EKeys::Escape) return HandleCloseClicked();
    if (Key == EKeys::Right || Key == EKeys::D || Key == EKeys::PageDown)
        return HandleNextClicked();
    if (Key == EKeys::Left || Key == EKeys::A || Key == EKeys::PageUp)
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
    SetZoom(Zoom + (InMouseEvent.GetWheelDelta() > 0.0f ? 0.25f : -0.25f));
    return FReply::Handled();
}
