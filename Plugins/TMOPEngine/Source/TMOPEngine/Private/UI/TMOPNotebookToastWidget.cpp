#include "UI/TMOPNotebookToastWidget.h"
#include "HAL/PlatformTime.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UTMOPNotebookToastWidget::RebuildWidget()
{
    return SNew(SOverlay)
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(16, 48)
        [ SNew(SBorder).Padding(14)
          .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
          .BorderBackgroundColor(FLinearColor(0.015f, 0.025f, 0.02f, 0.95f))
          [ SAssignNew(Message, STextBlock)
            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
            .ColorAndOpacity(FLinearColor(0.5f, 1.0f, 0.6f))
            .AutoWrapText(true).WrapTextAt(420.0f) ] ];
}

void UTMOPNotebookToastWidget::ShowObservation(const FText& Name)
{
    TakeWidget();
    Message->SetText(FText::Format(NSLOCTEXT("TMOP", "NotebookAdded",
        "”{0}” tillagd i anteckningsboken."), Name));
    HideAt = FPlatformTime::Seconds() + 4.0;
    SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UTMOPNotebookToastWidget::NativeTick(const FGeometry& Geometry, float DeltaSeconds)
{
    Super::NativeTick(Geometry, DeltaSeconds);
    if (FPlatformTime::Seconds() >= HideAt)
        SetVisibility(ESlateVisibility::Collapsed);
}

void UTMOPNotebookToastWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    Message.Reset();
}
