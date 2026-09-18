#pragma once
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"

/** Existing fixed-size reading/menu panels must fit even a quarter-screen. */
inline TSharedRef<SWidget> TMOPFitLocalPanel(const UObject* Context, TSharedRef<SWidget> Content,
    bool bConstrainSinglePlayer = false)
{
    const TWeakObjectPtr<const UObject> Weak(Context);
    auto IsSplit = [Weak, bConstrainSinglePlayer]()
    {
        return bConstrainSinglePlayer || (Weak.IsValid() &&
            UTMOPLocalMultiplayerSubsystem::IsMultiplayer(Weak.Get()));
    };
    return SNew(SScaleBox).StretchDirection(EStretchDirection::DownOnly)
        .HAlign(bConstrainSinglePlayer ? HAlign_Left : HAlign_Center)
        .VAlign(bConstrainSinglePlayer ? VAlign_Top : VAlign_Center)
        .Stretch_Lambda([IsSplit]() { return IsSplit() ? EStretch::ScaleToFit : EStretch::None; })
        [ SNew(SBox)
          .WidthOverride_Lambda([IsSplit]() { return IsSplit() ? FOptionalSize(1280) : FOptionalSize(); })
          .HeightOverride_Lambda([IsSplit]() { return IsSplit() ? FOptionalSize(800) : FOptionalSize(); })
          [ Content ] ];
}

/** Fill the owning player's viewport while keeping a usable minimum design size.
 * Unlike a fixed 1280x800 canvas, this expands on large/wide displays.
 * Viewport pixels are converted to Slate units before the fit calculation.
 */
inline TSharedRef<SWidget> TMOPFillLocalPanel(UUserWidget* Context, TSharedRef<SWidget> Content)
{
    const TWeakObjectPtr<UUserWidget> Weak(Context);
    auto CanvasSize = [Weak]() -> FVector2D
    {
        if (!Weak.IsValid()) return FVector2D(1280.0f, 800.0f);
        FVector2D Origin, Pixels;
        UTMOPLocalMultiplayerSubsystem::GetPlayerViewRect(
            Weak->GetOwningPlayer(), Origin, Pixels);
        if (Pixels.X <= 0.0f || Pixels.Y <= 0.0f)
            return FVector2D(1280.0f, 800.0f);
        const float DPI = FMath::Max(0.01f,
            UWidgetLayoutLibrary::GetViewportScale(Weak.Get()));
        const FVector2D Available = Pixels / DPI;
        const double Scale = FMath::Min(1.0,
            FMath::Min(Available.X / 1280.0, Available.Y / 800.0));
        // Same aspect ratio as the viewport: ScaleToFit leaves no unused strips.
        return Available / Scale;
    };
    return SNew(SScaleBox)
        .Stretch(EStretch::ScaleToFit)
        .StretchDirection(EStretchDirection::DownOnly)
        .HAlign(HAlign_Left).VAlign(VAlign_Top)
        [ SNew(SBox)
          .WidthOverride_Lambda([CanvasSize]() { return FOptionalSize(CanvasSize().X); })
          .HeightOverride_Lambda([CanvasSize]() { return FOptionalSize(CanvasSize().Y); })
          .HAlign(HAlign_Fill).VAlign(VAlign_Fill)
          .Clipping(EWidgetClipping::ClipToBounds)
          [ Content ] ];
}
