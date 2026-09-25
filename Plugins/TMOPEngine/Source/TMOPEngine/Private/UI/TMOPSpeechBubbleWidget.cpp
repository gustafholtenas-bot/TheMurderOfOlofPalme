#include "UI/TMOPSpeechBubbleWidget.h"
#include "Localization/TMOPLocalization.h"

#include "Styling/CoreStyle.h"
#include "Rendering/DrawElements.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

namespace
{
/** Paints the solid rectangular bubble and a tapered tail without an asset. */
class STMOPSpeechBubbleShape final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPSpeechBubbleShape) {}
        SLATE_DEFAULT_SLOT(FArguments, Content)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        ChildSlot
        .VAlign(VAlign_Top)
        [
            SNew(SBox)
            .WidthOverride(408.0f)
            .HeightOverride(100.0f)
            [
                InArgs._Content.Widget
            ]
        ];
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return FVector2D(412.0f, 154.0f);
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
        const FSlateRect& CullingRect, FSlateWindowElementList& OutDrawElements,
        int32 LayerId, const FWidgetStyle& WidgetStyle,
        bool bParentEnabled) const override
    {
        const FVector2D Size = Geometry.GetLocalSize();
        const float BoxHeight = FMath::Max(80.0f, Size.Y - 54.0f);
        const float Border = 3.0f;
        const FLinearColor Fill(0.005f, 0.007f, 0.012f, 0.96f);
        const FLinearColor Outline(0.95f, 0.95f, 0.95f, 1.0f);
        const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");

        FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
            Geometry.ToPaintGeometry(FVector2D(Size.X, BoxHeight), FSlateLayoutTransform()),
            White, ESlateDrawEffect::None, Fill);

        const FVector2D TailLeft(Size.X * 0.44f, BoxHeight - 1.0f);
        const FVector2D TailRight(Size.X * 0.54f, BoxHeight - 1.0f);
        const FVector2D TailTip(Size.X * 0.49f, Size.Y - 2.0f);
        // Fill the tapered tail with closely spaced horizontal strokes. This is
        // robust across UE Slate renderer versions and needs no texture asset.
        for (int32 Y = FMath::FloorToInt(BoxHeight); Y < FMath::FloorToInt(TailTip.Y); Y += 2)
        {
            const float Alpha = (static_cast<float>(Y) - BoxHeight) /
                FMath::Max(1.0f, TailTip.Y - BoxHeight);
            const FVector2D L = FMath::Lerp(TailLeft, TailTip, Alpha);
            const FVector2D R = FMath::Lerp(TailRight, TailTip, Alpha);
            TArray<FVector2D> FillLine{ FVector2D(L.X, static_cast<float>(Y)),
                FVector2D(R.X, static_cast<float>(Y)) };
            FSlateDrawElement::MakeLines(OutDrawElements, LayerId,
                Geometry.ToPaintGeometry(), FillLine, ESlateDrawEffect::None,
                Fill, true, 3.0f);
        }

        TArray<FVector2D> BorderLines{
            FVector2D(Border * 0.5f, Border * 0.5f),
            FVector2D(Size.X - Border * 0.5f, Border * 0.5f),
            FVector2D(Size.X - Border * 0.5f, BoxHeight - Border * 0.5f),
            TailRight,
            TailTip,
            TailLeft,
            FVector2D(Border * 0.5f, BoxHeight - Border * 0.5f),
            FVector2D(Border * 0.5f, Border * 0.5f)
        };
        FSlateDrawElement::MakeLines(OutDrawElements, LayerId + 1,
            Geometry.ToPaintGeometry(), BorderLines, ESlateDrawEffect::None,
            Outline, true, Border);

        return SCompoundWidget::OnPaint(Args, Geometry, CullingRect,
            OutDrawElements, LayerId + 2, WidgetStyle, bParentEnabled);
    }
};
}

void UTMOPSpeechBubbleWidget::SetSpeechText(const FText& NewText)
{
    bHistoricalPlayback = false;
    PendingSpeechText = NewText;
    FullSpeechString = FTMOPLocalization::String(NewText);
    LanguageRevision = FTMOPLocalization::GetRevision();
    RevealedCharacterAccumulator = 0.0f;
    RevealedCharacterCount = 0;
    if (SpeechText.IsValid()) SpeechText->SetText(FTMOPLocalization::Text(FText::GetEmpty()));
}

void UTMOPSpeechBubbleWidget::SetSpeakerName(const FText& NewName)
{
    PendingSpeakerName = NewName;
    if (SpeakerNameText.IsValid()) SpeakerNameText->SetText(FTMOPLocalization::Text(NewName));
}

void UTMOPSpeechBubbleWidget::NativeTick(
    const FGeometry& MyGeometry, const float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (LanguageRevision != FTMOPLocalization::GetRevision())
    {
        LanguageRevision = FTMOPLocalization::GetRevision();
        FullSpeechString = FTMOPLocalization::String(PendingSpeechText);
        RevealedCharacterCount = FMath::Min(RevealedCharacterCount, FullSpeechString.Len());
        if (SpeechText.IsValid()) SpeechText->SetText(FText::AsCultureInvariant(FullSpeechString.Left(RevealedCharacterCount)));
    }
    if (bHistoricalPlayback) return;
    if (!SpeechText.IsValid() || RevealedCharacterCount >= FullSpeechString.Len()) return;

    RevealedCharacterAccumulator += InDeltaTime *
        FMath::Max(1.0f, TypewriterCharactersPerSecond);
    const int32 CharactersToReveal = FMath::FloorToInt(RevealedCharacterAccumulator);
    if (CharactersToReveal <= 0) return;
    RevealedCharacterAccumulator -= static_cast<float>(CharactersToReveal);
    RevealedCharacterCount = FMath::Min(
        FullSpeechString.Len(), RevealedCharacterCount + CharactersToReveal);
    SpeechText->SetText(FTMOPLocalization::Text(FText::FromString(
        FullSpeechString.Left(RevealedCharacterCount))));
}

TSharedRef<SWidget> UTMOPSpeechBubbleWidget::RebuildWidget()
{
    return SNew(STMOPSpeechBubbleShape)
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(FMargin(12.0f, 7.0f, 12.0f, 1.0f))
            [
                SAssignNew(SpeakerNameText, STextBlock)
                .Text(FTMOPLocalization::Text(PendingSpeakerName))
                .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("SpeechBubble"),
                    FCoreStyle::GetDefaultFontStyle("Bold", 16)))
                .Justification(ETextJustify::Left)
                .ColorAndOpacity(FLinearColor(0.96f, 0.82f, 0.10f, 1.0f))
            ]
            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .VAlign(VAlign_Center)
            .Padding(FMargin(18.0f, 1.0f, 18.0f, 8.0f))
            [
                SAssignNew(SpeechText, STextBlock)
                .Text(FTMOPLocalization::Text(FText::GetEmpty()))
                .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("SpeechBubble"),
                    FCoreStyle::GetDefaultFontStyle("Regular", 18)))
                .AutoWrapText(true)
                .WrapTextAt(372.0f)
                .Justification(ETextJustify::Center)
                .ColorAndOpacity(FLinearColor::White)
            ]
        ];
}

void UTMOPSpeechBubbleWidget::SetPlaybackElapsed(float Seconds)
{
    bHistoricalPlayback = true;
    RevealedCharacterCount = FMath::Clamp(FMath::FloorToInt(FMath::Max(0.0f, Seconds) * TypewriterCharactersPerSecond), 0, FullSpeechString.Len());
    if (SpeechText.IsValid()) SpeechText->SetText(FTMOPLocalization::Text(FText::FromString(FullSpeechString.Left(RevealedCharacterCount))));
}
