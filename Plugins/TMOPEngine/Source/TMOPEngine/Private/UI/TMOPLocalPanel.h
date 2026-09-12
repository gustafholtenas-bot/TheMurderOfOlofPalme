#pragma once
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"

/** Existing fixed-size reading/menu panels must fit even a quarter-screen. */
inline TSharedRef<SWidget> TMOPFitLocalPanel(const UObject* Context, TSharedRef<SWidget> Content)
{
    const TWeakObjectPtr<const UObject> Weak(Context);
    auto IsSplit = [Weak]() { return Weak.IsValid() && UTMOPLocalMultiplayerSubsystem::IsMultiplayer(Weak.Get()); };
    return SNew(SScaleBox).StretchDirection(EStretchDirection::DownOnly)
        .Stretch_Lambda([IsSplit]() { return IsSplit() ? EStretch::ScaleToFit : EStretch::None; })
        [ SNew(SBox)
          .WidthOverride_Lambda([IsSplit]() { return IsSplit() ? FOptionalSize(1280) : FOptionalSize(); })
          .HeightOverride_Lambda([IsSplit]() { return IsSplit() ? FOptionalSize(800) : FOptionalSize(); })
          [ Content ] ];
}
