#pragma once
#include "Player/TMOPLocalMultiplayerSubsystem.h"
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
