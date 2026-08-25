#include "UI/TMOPMapWidget.h"

#include "Engine/Texture2D.h"
#include "InputCoreTypes.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Styling/CoreStyle.h"
#include "UI/TMOPMapComponent.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
class STMOPMapCanvas : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPMapCanvas) {}
        SLATE_ARGUMENT(UTMOPMapWidget*, OwnerWidget)
        SLATE_ARGUMENT(FSlateBrush*, MapBrush)
    SLATE_END_ARGS()

    void Construct(const FArguments& Args)
    {
        OwnerWidget = Args._OwnerWidget;
        MapBrush = Args._MapBrush;
        SetClipping(EWidgetClipping::ClipToBounds);
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return OwnerWidget.IsValid() && OwnerWidget->IsMinimap()
            ? FVector2D(290.0f, 290.0f) : FVector2D(1280.0f, 720.0f);
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
        const FSlateRect& CullingRect, FSlateWindowElementList& Out,
        int32 Layer, const FWidgetStyle& Style, bool bParentEnabled) const override
    {
        const UTMOPMapWidget* Widget = OwnerWidget.Get();
        const UTMOPMapComponent* Map = Widget != nullptr ? Widget->GetMapComponent() : nullptr;
        if (Map == nullptr) return Layer;

        const FVector2D ViewSize = Geometry.GetLocalSize();
        const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
        FSlateDrawElement::MakeBox(Out, Layer++, Geometry.ToPaintGeometry(), White,
            ESlateDrawEffect::None, FLinearColor(0.018f, 0.022f, 0.026f, 0.98f));

        const float TextureAspect = IsValid(Map->MapTexture)
            ? FMath::Max(0.1f, float(Map->MapTexture->GetSizeX()) /
                FMath::Max(1, Map->MapTexture->GetSizeY())) : 1.0f;
        FVector2D BaseSize = ViewSize;
        if (ViewSize.X / FMath::Max(1.0f, ViewSize.Y) > TextureAspect)
            BaseSize.X = ViewSize.Y * TextureAspect;
        else BaseSize.Y = ViewSize.X / TextureAspect;
        const FVector2D MapSize = BaseSize * Widget->GetZoom();
        const FVector2D CenterUV = Widget->GetViewCenterUV();
        const FVector2D MapOrigin = ViewSize * 0.5f - CenterUV * MapSize;
        const FGeometry MapGeometry = Geometry.MakeChild(MapSize,
            FSlateLayoutTransform(MapOrigin));

        if (IsValid(Map->MapTexture) && MapBrush != nullptr)
            FSlateDrawElement::MakeBox(Out, Layer++, MapGeometry.ToPaintGeometry(),
                MapBrush, ESlateDrawEffect::None, FLinearColor::White);
        else
        {
            FSlateDrawElement::MakeBox(Out, Layer++, MapGeometry.ToPaintGeometry(), White,
                ESlateDrawEffect::None, FLinearColor(0.12f, 0.14f, 0.15f, 1.0f));
            for (int32 I = 1; I < 10; ++I)
            {
                const float T = I / 10.0f;
                FSlateDrawElement::MakeLines(Out, Layer,
                    Geometry.ToPaintGeometry(), {MapOrigin + FVector2D(MapSize.X * T, 0.0f),
                        MapOrigin + FVector2D(MapSize.X * T, MapSize.Y)},
                    ESlateDrawEffect::None, FLinearColor(0.22f, 0.24f, 0.25f, 1.0f));
                FSlateDrawElement::MakeLines(Out, Layer,
                    Geometry.ToPaintGeometry(), {MapOrigin + FVector2D(0.0f, MapSize.Y * T),
                        MapOrigin + FVector2D(MapSize.X, MapSize.Y * T)},
                    ESlateDrawEffect::None, FLinearColor(0.22f, 0.24f, 0.25f, 1.0f));
            }
            ++Layer;
        }

        if (!Widget->IsMinimap())
            for (const FTMOPMapMarker& Marker : Map->Markers)
            {
                if (!Marker.bDiscovered) continue;
                const FVector2D P = MapOrigin + Map->WorldToMapUV(Marker.WorldLocation) * MapSize;
                if (P.X < -12.0f || P.Y < -12.0f || P.X > ViewSize.X + 12.0f || P.Y > ViewSize.Y + 12.0f)
                    continue;
                const FGeometry Dot = Geometry.MakeChild(FVector2D(12.0f),
                    FSlateLayoutTransform(P - FVector2D(6.0f)));
                FSlateDrawElement::MakeBox(Out, Layer, Dot.ToPaintGeometry(), White,
                    ESlateDrawEffect::None, Marker.Color);
            }

        const FVector2D PlayerP = MapOrigin +
            Map->WorldToMapUV(Map->GetTrackedWorldLocation()) * MapSize;
        const float Angle = FMath::DegreesToRadians(Map->GetTrackedMapYawDegrees());
        const FVector2D Forward(FMath::Cos(Angle), -FMath::Sin(Angle));
        const FVector2D Right(-Forward.Y, Forward.X);
        const TArray<FVector2D> Arrow = {
            PlayerP + Forward * 13.0f,
            PlayerP - Forward * 9.0f + Right * 7.0f,
            PlayerP - Forward * 5.0f,
            PlayerP - Forward * 9.0f - Right * 7.0f,
            PlayerP + Forward * 13.0f };
        FSlateDrawElement::MakeLines(Out, Layer++, Geometry.ToPaintGeometry(), Arrow,
            ESlateDrawEffect::None, FLinearColor(0.12f, 0.72f, 1.0f, 1.0f), true, 3.0f);

        return Layer;
    }

    virtual FReply OnMouseWheel(const FGeometry&, const FPointerEvent& Event) override
    {
        if (UTMOPMapWidget* Widget = OwnerWidget.Get(); Widget && !Widget->IsMinimap())
        {
            Widget->ChangeZoom(Event.GetWheelDelta());
            return FReply::Handled();
        }
        return FReply::Unhandled();
    }

    virtual FReply OnMouseButtonDown(const FGeometry&, const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() == EKeys::LeftMouseButton &&
            OwnerWidget.IsValid() && !OwnerWidget->IsMinimap())
        {
            bDragging = true;
            LastMousePosition = Event.GetScreenSpacePosition();
            return FReply::Handled().CaptureMouse(SharedThis(this));
        }
        return FReply::Unhandled();
    }

    virtual FReply OnMouseMove(const FGeometry& Geometry, const FPointerEvent& Event) override
    {
        if (!bDragging || !OwnerWidget.IsValid()) return FReply::Unhandled();
        const FVector2D Position = Event.GetScreenSpacePosition();
        OwnerWidget->PanByPixels(Position - LastMousePosition, Geometry.GetLocalSize());
        LastMousePosition = Position;
        return FReply::Handled();
    }

    virtual FReply OnMouseButtonUp(const FGeometry&, const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() == EKeys::LeftMouseButton && bDragging)
        {
            bDragging = false;
            return FReply::Handled().ReleaseMouseCapture();
        }
        return FReply::Unhandled();
    }

    virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent& Event) override
    {
        if (Event.GetKey() == EKeys::Escape || Event.GetKey() == EKeys::M)
        {
            if (OwnerWidget.IsValid()) OwnerWidget->RequestClose();
            return FReply::Handled();
        }
        return FReply::Unhandled();
    }

    virtual bool SupportsKeyboardFocus() const override { return !OwnerWidget.IsValid() || !OwnerWidget->IsMinimap(); }

private:
    TWeakObjectPtr<UTMOPMapWidget> OwnerWidget;
    FSlateBrush* MapBrush = nullptr;
    bool bDragging = false;
    FVector2D LastMousePosition = FVector2D::ZeroVector;
};
}

void UTMOPMapWidget::InitializeMap(UTMOPMapComponent* InMapComponent,
    ATMOPPlayerCharacter* InPlayerCharacter, const bool bInMinimap)
{
    MapComponent = InMapComponent;
    PlayerCharacter = InPlayerCharacter;
    bMinimap = bInMinimap;
    MapBrush.ImageSize = FVector2D(1024.0f);
    MapBrush.DrawAs = ESlateBrushDrawType::Image;
    MapBrush.SetResourceObject(IsValid(MapComponent) ? MapComponent->MapTexture.Get() : nullptr);
    ResetViewToPlayer();
    SetIsFocusable(!bMinimap);
}

TSharedRef<SWidget> UTMOPMapWidget::RebuildWidget()
{
    TSharedRef<STMOPMapCanvas> Canvas = SNew(STMOPMapCanvas)
        .OwnerWidget(this).MapBrush(&MapBrush);
    if (bMinimap)
        return SNew(SOverlay)
            + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Bottom)
                .Padding(FMargin(0.0f, 0.0f, 28.0f, 28.0f))
            [ SNew(SBox).WidthOverride(290.0f).HeightOverride(290.0f)[ Canvas ] ];
    return SNew(SOverlay)
        + SOverlay::Slot()[ Canvas ]
        + SOverlay::Slot().HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(24.0f)
        [ SNew(STextBlock)
            .Text(NSLOCTEXT("TMOP", "WorldMapHelp", "KARTA  •  mushjul: zoom  •  dra: panorera  •  M/Esc: stäng"))
            .ColorAndOpacity(FLinearColor::White) ];
}

void UTMOPMapWidget::SetMapVisible(const bool bVisible)
{
    SetVisibility(bVisible ? (bMinimap ? ESlateVisibility::HitTestInvisible
        : ESlateVisibility::Visible) : ESlateVisibility::Collapsed);
}

void UTMOPMapWidget::ResetViewToPlayer()
{
    FullMapZoom = 1.0f;
    FullMapCenterUV = IsValid(MapComponent)
        ? MapComponent->WorldToMapUV(MapComponent->GetTrackedWorldLocation())
        : FVector2D(0.5f);
}

float UTMOPMapWidget::GetZoom() const
{
    return bMinimap && IsValid(MapComponent) ? MapComponent->MinimapZoom : FullMapZoom;
}

FVector2D UTMOPMapWidget::GetViewCenterUV() const
{
    return bMinimap && IsValid(MapComponent)
        ? MapComponent->WorldToMapUV(MapComponent->GetTrackedWorldLocation())
        : FullMapCenterUV;
}

void UTMOPMapWidget::ChangeZoom(const float WheelDelta)
{
    FullMapZoom = FMath::Clamp(FullMapZoom * FMath::Pow(1.2f, WheelDelta), 1.0f, 10.0f);
}

void UTMOPMapWidget::PanByPixels(const FVector2D PixelDelta, const FVector2D ViewSize)
{
    const FVector2D SafeSize(FMath::Max(1.0f, ViewSize.X), FMath::Max(1.0f, ViewSize.Y));
    FullMapCenterUV -= PixelDelta / (SafeSize * FullMapZoom);
    FullMapCenterUV.X = FMath::Clamp(FullMapCenterUV.X, 0.0f, 1.0f);
    FullMapCenterUV.Y = FMath::Clamp(FullMapCenterUV.Y, 0.0f, 1.0f);
}

void UTMOPMapWidget::RequestClose()
{
    if (IsValid(PlayerCharacter)) PlayerCharacter->CloseWorldMap();
}
