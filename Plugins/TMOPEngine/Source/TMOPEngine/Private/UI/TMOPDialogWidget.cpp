#include "UI/TMOPDialogWidget.h"

#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "InputCoreTypes.h"
#include "Player/TMOPPlayerCharacter.h"
#include "RecordedCalls/TMOPRecordedCallDirector.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

UTMOPDialogWidget::UTMOPDialogWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    RadioIcon = LoadObject<UTexture2D>(nullptr,
        TEXT("/Game/TMOP/Items/Icons_PNG/I_TMOP_Radio.I_TMOP_Radio"));
}

void UTMOPDialogWidget::InitializeDialog(ATMOPPlayerCharacter* InPlayerCharacter)
{
    PlayerCharacter = InPlayerCharacter;
    SetIsFocusable(true);
}

void UTMOPDialogWidget::ShowDialog(const FText& Speaker, const FText& Dialog)
{
    if (SpeakerText.IsValid()) SpeakerText->SetText(Speaker);
    FullDialogString = Dialog.ToString();
    DialogRevealCharacters = 0.0f;
    if (DialogText.IsValid()) DialogText->SetText(FText::GetEmpty());
    bDialogVisible = true;
    RefreshVisibility();
}

void UTMOPDialogWidget::HideDialog()
{
    bDialogVisible = false;
    FullDialogString.Reset();
    RefreshVisibility();
}

TSharedRef<SWidget> UTMOPDialogWidget::RebuildWidget()
{
    RadioIconBrush.ImageSize = FVector2D(58.0f);
    RadioIconBrush.DrawAs = ESlateBrushDrawType::Image;
    RadioIconBrush.SetResourceObject(RadioIcon);
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Bottom)
        .Padding(48.0f, 48.0f, 48.0f, 86.0f)
        [ SNew(SBox).WidthOverride(960.0f)
          [ SNew(SVerticalBox)
            // Dialog comes first and therefore stacks above simultaneous radio traffic.
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 10.0f)
            [ SAssignNew(DialogPanel, SBorder).Visibility(EVisibility::Collapsed)
              .BorderBackgroundColor(FLinearColor(0.015f, 0.02f, 0.03f, 0.94f))
              .Padding(FMargin(24.0f, 16.0f))
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SHorizontalBox)
                  + SHorizontalBox::Slot().FillWidth(1.0f)
                  [ SAssignNew(SpeakerText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("DialogSpeakerName"),
                        FCoreStyle::GetDefaultFontStyle("Bold", 19)))
                    .ColorAndOpacity(FLinearColor(0.95f, 0.72f, 0.22f)) ]
                  + SHorizontalBox::Slot().AutoWidth()
                  [ SNew(SButton).Text(NSLOCTEXT("TMOP", "CloseDialog", "Stäng"))
                    .OnClicked_UObject(this, &UTMOPDialogWidget::HandleCloseClicked) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 9.0f, 0.0f, 2.0f)
                [ SAssignNew(DialogText, STextBlock)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("DialogBody"),
                      FCoreStyle::GetDefaultFontStyle("Regular", 18)))
                  .AutoWrapText(true).WrapTextAt(900.0f)
                  .ColorAndOpacity(FLinearColor::White) ] ] ]
            + SVerticalBox::Slot().AutoHeight()
            [ SAssignNew(RadioPanel, SBorder).Visibility(EVisibility::Collapsed)
              .BorderBackgroundColor(FLinearColor(0.035f, 0.045f, 0.055f, 0.95f))
              .Padding(FMargin(20.0f, 13.0f))
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SHorizontalBox)
                  + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                  [ SAssignNew(RadioLeftSpeakerText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("DialogSpeakerName"),
                        FCoreStyle::GetDefaultFontStyle("Bold", 16)))
                    .ColorAndOpacity(FLinearColor(0.68f, 0.82f, 0.92f)) ]
                  + SHorizontalBox::Slot().AutoWidth().HAlign(HAlign_Center)
                  [ SNew(SBox).WidthOverride(58.0f).HeightOverride(58.0f)
                    [ SNew(SImage).Image(&RadioIconBrush) ] ]
                  + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
                    .HAlign(HAlign_Right)
                  [ SAssignNew(RadioRightSpeakerText, STextBlock)
                    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("DialogSpeakerName"),
                        FCoreStyle::GetDefaultFontStyle("Bold", 16)))
                    .Justification(ETextJustify::Right)
                    .ColorAndOpacity(FLinearColor(0.68f, 0.82f, 0.92f)) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(16.0f, 7.0f, 16.0f, 3.0f)
                [ SAssignNew(RadioSubtitleText, STextBlock)
                  .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("Subtitle"),
                      FCoreStyle::GetDefaultFontStyle("Regular", 17)))
                  .AutoWrapText(true).WrapTextAt(880.0f)
                  .Justification(ETextJustify::Center)
                  .ColorAndOpacity(FLinearColor::White) ] ] ] ] ];
}

void UTMOPDialogWidget::NativeTick(const FGeometry& MyGeometry, const float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    RefreshRadioSubtitle();
    AdvanceTypewriter(InDeltaTime);
}

void UTMOPDialogWidget::RefreshRadioSubtitle()
{
    if (!RecordedCallDirector.IsValid() && GetWorld() != nullptr)
        for (TActorIterator<ATMOPRecordedCallDirector> It(GetWorld()); It; ++It)
        { RecordedCallDirector = *It; break; }
    ATMOPRecordedCallDirector* Director = RecordedCallDirector.Get();
    if (!IsValid(Director)) { bRadioVisible = false; RefreshVisibility(); return; }
    const TArray<FName> ActiveIds = Director->GetActiveRecordingIds();
    FName RecordingId = ActiveRadioRecordingId;
    if (!ActiveIds.Contains(RecordingId))
        RecordingId = ActiveIds.IsEmpty() ? NAME_None : ActiveIds.Last();

    FName SegmentId;
    FText LeftSpeaker, RightSpeaker, Transcript;
    bool bRadioStyle = false;
    if (RecordingId.IsNone() || !Director->GetActiveSubtitle(RecordingId,
        SegmentId, LeftSpeaker, RightSpeaker, Transcript, bRadioStyle))
    {
        bRadioVisible = false;
        ActiveRadioRecordingId = NAME_None;
        ActiveRadioSegmentId = NAME_None;
        RefreshVisibility();
        return;
    }
    if (RecordingId != ActiveRadioRecordingId || SegmentId != ActiveRadioSegmentId)
    {
        ActiveRadioRecordingId = RecordingId;
        ActiveRadioSegmentId = SegmentId;
        FullRadioString = Transcript.ToString();
        RadioRevealCharacters = 0.0f;
        if (RadioSubtitleText.IsValid()) RadioSubtitleText->SetText(FText::GetEmpty());
    }
    if (RadioLeftSpeakerText.IsValid()) RadioLeftSpeakerText->SetText(LeftSpeaker);
    if (RadioRightSpeakerText.IsValid()) RadioRightSpeakerText->SetText(RightSpeaker);
    bRadioVisible = true;
    RefreshVisibility();
}

void UTMOPDialogWidget::AdvanceTypewriter(const float DeltaTime)
{
    const float Advance = FMath::Max(1.0f, TypewriterCharactersPerSecond) *
        FMath::Max(0.0f, DeltaTime);
    if (bDialogVisible && DialogText.IsValid())
    {
        DialogRevealCharacters = FMath::Min(float(FullDialogString.Len()),
            DialogRevealCharacters + Advance);
        DialogText->SetText(FText::FromString(
            FullDialogString.Left(FMath::FloorToInt(DialogRevealCharacters))));
    }
    if (bRadioVisible && RadioSubtitleText.IsValid())
    {
        RadioRevealCharacters = FMath::Min(float(FullRadioString.Len()),
            RadioRevealCharacters + Advance);
        RadioSubtitleText->SetText(FText::FromString(
            FullRadioString.Left(FMath::FloorToInt(RadioRevealCharacters))));
    }
}

void UTMOPDialogWidget::RefreshVisibility()
{
    if (DialogPanel.IsValid()) DialogPanel->SetVisibility(bDialogVisible
        ? EVisibility::Visible : EVisibility::Collapsed);
    if (RadioPanel.IsValid()) RadioPanel->SetVisibility(bRadioVisible
        ? EVisibility::HitTestInvisible : EVisibility::Collapsed);
    // Keep the empty root ticking so a radio call can appear without a dialog already open.
    SetVisibility(bDialogVisible ? ESlateVisibility::Visible
        : ESlateVisibility::HitTestInvisible);
}

FReply UTMOPDialogWidget::HandleCloseClicked()
{
    if (IsValid(PlayerCharacter.Get())) PlayerCharacter->ClosePersonDialog();
    return FReply::Handled();
}

FReply UTMOPDialogWidget::NativeOnKeyDown(const FGeometry& InGeometry,
    const FKeyEvent& InKeyEvent)
{
    if (bDialogVisible && InKeyEvent.GetKey() == EKeys::Escape)
        return HandleCloseClicked();
    return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}
