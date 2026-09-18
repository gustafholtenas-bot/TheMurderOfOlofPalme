#include "UI/TMOPNewspaperReaderWidget.h"
#include "UI/TMOPControlUIHelpers.h"

#include "InputCoreTypes.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"
#include "Newspapers/TMOPNewspaperReadingComponent.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/SOverlay.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

namespace
{
class SNewspaperMouseSurface : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(SNewspaperMouseSurface) {}
        SLATE_ARGUMENT(UTMOPNewspaperReaderWidget*, Reader)
        SLATE_ATTRIBUTE(ATMOPPlayerCharacter*, Player)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args) { Reader = Args._Reader; Player = Args._Player; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D::ZeroVector; }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry&, const FSlateRect&,
        FSlateWindowElementList&, int32 LayerId, const FWidgetStyle&, bool) const override
    {
        // Input-only overlay: the newspaper is rendered as a world-space mesh.
        return LayerId;
    }
    virtual FReply OnMouseButtonDown(const FGeometry& G, const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() != EKeys::LeftMouseButton && Event.GetEffectingButton() != EKeys::RightMouseButton)
            return FReply::Unhandled();
        Button = Event.GetEffectingButton(); Last = G.AbsoluteToLocal(Event.GetScreenSpacePosition());
        return FReply::Handled().CaptureMouse(SharedThis(this));
    }
    virtual FReply OnMouseMove(const FGeometry& G, const FPointerEvent& Event) override
    {
        if (!HasMouseCapture()) return FReply::Unhandled();
        const FVector2D Now = G.AbsoluteToLocal(Event.GetScreenSpacePosition());
        if (ATMOPPlayerCharacter* Owner = Player.Get())
            if (IsValid(Owner) && IsValid(Owner->NewspaperReading))
                Owner->NewspaperReading->DragReadingView(Now - Last, G.GetLocalSize(), Button == EKeys::RightMouseButton);
        Last = Now;
        return FReply::Handled();
    }
    virtual FReply OnMouseButtonUp(const FGeometry&, const FPointerEvent& Event) override
    {
        if (HasMouseCapture() && Event.GetEffectingButton() == Button)
        { Button = FKey(); return FReply::Handled().ReleaseMouseCapture(); }
        return FReply::Unhandled();
    }
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& Event) override
    { Button = FKey(); SLeafWidget::OnMouseCaptureLost(Event); }
private:
    TWeakObjectPtr<UTMOPNewspaperReaderWidget> Reader;
    TAttribute<ATMOPPlayerCharacter*> Player;
    FVector2D Last = FVector2D::ZeroVector;
    FKey Button;
};
}

void UTMOPNewspaperReaderWidget::InitializeReader(
    ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    SetIsFocusable(true);
    SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    SetAlignmentInViewport(FVector2D::ZeroVector);
    SetPositionInViewport(FVector2D::ZeroVector, false);
    SetDesiredSizeInViewport(FVector2D::ZeroVector);
}

bool UTMOPNewspaperReaderWidget::OpenNewspaper(
    UTMOPNewspaperItemDefinition* InNewspaper)
{
    if (!IsValid(InNewspaper) || InNewspaper->Pages.IsEmpty()) return false;
    Newspaper = InNewspaper;
    // Start with the folded front page. Next enters the first inner spread.
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
    const int32 LastPageIndex = Newspaper->Pages.Num() - 1;
    if (PageIndex == 0 || PageIndex == LastPageIndex)
    {
        CurrentPageIndex = PageIndex;
    }
    else
    {
        // Inner pages are shown as physical two-page spreads. Always normalize
        // to the left page so direct Blueprint calls cannot create overlap.
        CurrentPageIndex = 1 + 2 * ((PageIndex - 1) / 2);
        CurrentPageIndex = FMath::Min(CurrentPageIndex, LastPageIndex - 1);
    }
    RefreshPage();
    return true;
}

bool UTMOPNewspaperReaderWidget::NextPage()
{
    const int32 PageCount = GetPageCount();
    if (PageCount <= 1 || CurrentPageIndex >= PageCount - 1) return false;
    if (CurrentPageIndex == 0) return GoToPage(PageCount > 2 ? 1 : PageCount - 1);

    const int32 CandidateIndex = CurrentPageIndex + 2;
    return GoToPage(CandidateIndex < PageCount - 1
        ? CandidateIndex : PageCount - 1);
}

bool UTMOPNewspaperReaderWidget::PreviousPage()
{
    const int32 PageCount = GetPageCount();
    if (CurrentPageIndex <= 0) return false;
    if (CurrentPageIndex == PageCount - 1 && PageCount > 2)
    {
        const int32 LastInnerLeft = 1 + 2 * ((PageCount - 3) / 2);
        return GoToPage(LastInnerLeft);
    }
    return CurrentPageIndex <= 1
        ? GoToPage(0) : GoToPage(FMath::Max(1, CurrentPageIndex - 2));
}

void UTMOPNewspaperReaderWidget::SetZoom(const float NewZoom)
{
    Zoom = FMath::Clamp(NewZoom, 0.25f, 4.0f);
}

TSharedRef<SWidget> UTMOPNewspaperReaderWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot()
        [ SNew(SNewspaperMouseSurface).Reader(this).Player_Lambda([this]() { return PlayerCharacter.Get(); }) ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom).Padding(18)
        [ SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
          [ SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0,0,0,0.88f)).Padding(14)
            [ SNew(SVerticalBox)
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
              [ SAssignNew(TitleText, STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
                .ColorAndOpacity(FLinearColor::White) ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0,4)
              [ SAssignNew(PageNumberText, STextBlock).ColorAndOpacity(FLinearColor::White) ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
              [ SAssignNew(PageLabelText, STextBlock).ColorAndOpacity(FLinearColor::White) ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0,8)
              [ SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP","ReaderPrevBottom","Föregående sida"))
                  .OnClicked_UObject(this,&UTMOPNewspaperReaderWidget::HandlePreviousClicked) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
                [ SNew(SButton).Text(FText::FromString(TEXT("−")))
                  .OnClicked_UObject(this,&UTMOPNewspaperReaderWidget::HandleZoomOutClicked) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
                [ SNew(SButton).Text(FText::FromString(TEXT("+")))
                  .OnClicked_UObject(this,&UTMOPNewspaperReaderWidget::HandleZoomInClicked) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP","ReaderResetBottom","Återställ vy"))
                  .OnClicked_Lambda([this]() {
                      if (IsValid(PlayerCharacter) && IsValid(PlayerCharacter->NewspaperReading))
                          PlayerCharacter->NewspaperReading->ResetReadingView();
                      return FReply::Handled(); }) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP","ReaderNextBottom","Nästa sida"))
                  .OnClicked_UObject(this,&UTMOPNewspaperReaderWidget::HandleNextClicked) ]
                + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
                [ SNew(SButton).Text(NSLOCTEXT("TMOP","CloseNewspaper","Stäng"))
                  .OnClicked_UObject(this,&UTMOPNewspaperReaderWidget::HandleCloseClicked) ] ]
              + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center)
              [ SNew(STextBlock).Text(NSLOCTEXT("TMOP","ReaderMouseHelp",
                  "Mushjul: zoom • Vänsterdrag: flytta • Högerdrag: rotera"))
                .Font(FCoreStyle::GetDefaultFontStyle("Regular",12)).ColorAndOpacity(FLinearColor::White) ]
            ] ] ];
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
    const bool bIsFront = CurrentPageIndex == 0;
    const bool bIsBack = CurrentPageIndex == Newspaper->Pages.Num() - 1 &&
        Newspaper->Pages.Num() > 1;
    if (PageNumberText.IsValid())
    {
        if (bIsFront)
            PageNumberText->SetText(NSLOCTEXT("TMOP", "NewspaperFrontCover", "Framsida"));
        else if (bIsBack)
            PageNumberText->SetText(NSLOCTEXT("TMOP", "NewspaperBackCover", "Baksida"));
        else
            PageNumberText->SetText(FText::Format(
                NSLOCTEXT("TMOP", "NewspaperPageCounter", "Uppslag {0}–{1} av {2} sidor"),
                FText::AsNumber(CurrentPageIndex + 1),
                FText::AsNumber(FMath::Min(CurrentPageIndex + 2, Newspaper->Pages.Num() - 1)),
                FText::AsNumber(Newspaper->Pages.Num())));
    }
    if (PageLabelText.IsValid())
    {
        const int32 ResolvedPrintedPageNumber = Newspaper->bAutomaticallyNumberPages
            ? CurrentPageIndex + 1 : FMath::Max(1, Page.PrintedPageNumber);
        const FText Label = Page.PageLabel.IsEmpty() && !bIsFront && !bIsBack
            ? FText::Format(NSLOCTEXT("TMOP", "PrintedNewspaperPage", "Tryckt sida {0}"),
                FText::AsNumber(ResolvedPrintedPageNumber))
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
    const FReply Reply = HandleReaderKey(InKeyEvent.GetKey());
    return Reply.IsEventHandled()
        ? Reply : Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

FReply UTMOPNewspaperReaderWidget::NativeOnPreviewKeyDown(
    const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
    // Slate buttons take keyboard focus when clicked. Handle the reader's
    // shortcuts during tunnelling so they keep working regardless of which
    // on-screen button currently owns focus.
    const FReply Reply = HandleReaderKey(InKeyEvent.GetKey());
    return Reply.IsEventHandled()
        ? Reply : Super::NativeOnPreviewKeyDown(InGeometry, InKeyEvent);
}

FReply UTMOPNewspaperReaderWidget::HandleReaderKey(const FKey& Key)
{
    if (TMOPHasControlProfiles(this))
    {
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuBack)) return HandleCloseClicked();
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuRight)) return PanPage(1.0f, 0.0f);
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuLeft)) return PanPage(-1.0f, 0.0f);
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuUp)) return PanPage(0.0f, -1.0f);
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuDown)) return PanPage(0.0f, 1.0f);
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuNextPage)) return HandleNextClicked();
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuPreviousPage)) return HandlePreviousClicked();
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuZoomIn)) return HandleZoomInClicked();
        if (TMOPMatchesControl(this, Key, ETMOPControlAction::MenuZoomOut)) return HandleZoomOutClicked();
        return FReply::Unhandled();
    }
    if (Key == EKeys::Escape || Key == EKeys::Gamepad_FaceButton_Right) return HandleCloseClicked();
    if (Key == EKeys::Gamepad_DPad_Right) return PanPage(1.0f, 0.0f);
    if (Key == EKeys::Gamepad_DPad_Left) return PanPage(-1.0f, 0.0f);
    if (Key == EKeys::Gamepad_DPad_Up) return PanPage(0.0f, -1.0f);
    if (Key == EKeys::Gamepad_DPad_Down) return PanPage(0.0f, 1.0f);
    if (Key == EKeys::Gamepad_RightShoulder) return HandleNextClicked();
    if (Key == EKeys::Gamepad_LeftShoulder) return HandlePreviousClicked();
    if (Key == EKeys::Gamepad_RightTrigger) return HandleZoomInClicked();
    if (Key == EKeys::Gamepad_LeftTrigger) return HandleZoomOutClicked();
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
    return FReply::Unhandled();
}

FReply UTMOPNewspaperReaderWidget::NativeOnMouseWheel(
    const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    return InMouseEvent.GetWheelDelta() > 0.0f
        ? HandleZoomInClicked() : HandleZoomOutClicked();
}
