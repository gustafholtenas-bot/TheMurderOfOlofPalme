#include "UI/TMOPMapWidget.h"

#include "Engine/Texture2D.h"
#include "HAL/PlatformTime.h"
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
        SetCanTick(true);
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return OwnerWidget.IsValid() && OwnerWidget->IsMinimap()
            ? FVector2D(290.0f, 290.0f) : FVector2D(1280.0f, 720.0f);
    }

    virtual void Tick(const FGeometry& AllottedGeometry, const double CurrentTime,
        const float DeltaTime) override
    {
        SLeafWidget::Tick(AllottedGeometry, CurrentTime, DeltaTime);
        Invalidate(EInvalidateWidgetReason::Paint);
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
        const bool bFullMap = !Widget->IsMinimap();
        const FVector2D ContentOrigin = bFullMap
            ? FVector2D(220.0f, 58.0f) : FVector2D::ZeroVector;
        const FVector2D ContentSize = bFullMap
            ? FVector2D(FMath::Max(1.0f, ViewSize.X - 260.0f),
                FMath::Max(1.0f, ViewSize.Y - 138.0f))
            : ViewSize;

        const bool bRotateClockwise = Map->bRotateDisplay90DegreesClockwise;
        const auto ToDisplayUV = [bRotateClockwise](const FVector2D UV)
        {
            return bRotateClockwise ? FVector2D(1.0f - UV.Y, UV.X) : UV;
        };

        const float SourceTextureAspect = IsValid(Map->MapTexture)
            ? FMath::Max(0.1f, float(Map->MapTexture->GetSizeX()) /
                FMath::Max(1, Map->MapTexture->GetSizeY())) : 1.0f;
        const float TextureAspect = bRotateClockwise
            ? 1.0f / SourceTextureAspect : SourceTextureAspect;
        FVector2D BaseSize = ContentSize;
        if (ContentSize.X / FMath::Max(1.0f, ContentSize.Y) > TextureAspect)
            BaseSize.X = ContentSize.Y * TextureAspect;
        else BaseSize.Y = ContentSize.X / TextureAspect;
        const FVector2D MapSize = BaseSize * Widget->GetZoom();
        const FVector2D CenterUV = ToDisplayUV(Widget->GetViewCenterUV());
        const FVector2D MapOrigin = ContentOrigin + ContentSize * 0.5f -
            CenterUV * MapSize;
        const FGeometry MapGeometry = Geometry.MakeChild(MapSize,
            FSlateLayoutTransform(MapOrigin));

        if (IsValid(Map->MapTexture) && MapBrush != nullptr)
        {
            if (bRotateClockwise)
            {
                const FVector2D SourceDrawSize(MapSize.Y, MapSize.X);
                const FVector2D SourceDrawOrigin = MapOrigin +
                    (MapSize - SourceDrawSize) * 0.5f;
                const FGeometry SourceGeometry = Geometry.MakeChild(SourceDrawSize,
                    FSlateLayoutTransform(SourceDrawOrigin));
                FSlateDrawElement::MakeRotatedBox(Out, Layer++,
                    SourceGeometry.ToPaintGeometry(), MapBrush,
                    ESlateDrawEffect::None, PI * 0.5f, TOptional<FVector2D>(),
                    FSlateDrawElement::RelativeToElement, FLinearColor::White);
            }
            else
                FSlateDrawElement::MakeBox(Out, Layer++, MapGeometry.ToPaintGeometry(),
                    MapBrush, ESlateDrawEffect::None, FLinearColor::White);
        }
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

        if (!bFullMap || Widget->ShouldShowPlaces())
        for (const FTMOPMapMarker& Marker : Map->Markers)
        {
            if (!Marker.bDiscovered) continue;
            const FVector2D P = MapOrigin +
                ToDisplayUV(Map->WorldToMapUV(Marker.WorldLocation)) * MapSize;
            if (P.X < -80.0f || P.Y < -30.0f || P.X > ViewSize.X + 80.0f || P.Y > ViewSize.Y + 40.0f)
                continue;
            const float IconSize = Widget->IsMinimap() ? 10.0f : 22.0f;
            const FGeometry IconGeometry = Geometry.MakeChild(FVector2D(IconSize),
                FSlateLayoutTransform(P - FVector2D(IconSize * 0.5f)));
            UTexture2D* IconTexture = IsValid(Marker.Icon)
                ? Marker.Icon.Get() : Map->GetCategoryIcon(Marker.Category);
            if (IsValid(IconTexture))
            {
                FSlateBrush IconBrush;
                IconBrush.DrawAs = ESlateBrushDrawType::Image;
                IconBrush.SetResourceObject(IconTexture);
                FSlateDrawElement::MakeBox(Out, Layer, IconGeometry.ToPaintGeometry(),
                    &IconBrush, ESlateDrawEffect::None, FLinearColor::White);
            }
            else
            {
                FSlateDrawElement::MakeBox(Out, Layer, IconGeometry.ToPaintGeometry(),
                    White, ESlateDrawEffect::None, Marker.Color);
                FText Symbol = FText::FromString(TEXT("•"));
                switch (Marker.Category)
                {
                case ETMOPMapMarkerCategory::Restaurant: Symbol = FText::FromString(TEXT("R")); break;
                case ETMOPMapMarkerCategory::Cinema: Symbol = FText::FromString(TEXT("B")); break;
                case ETMOPMapMarkerCategory::Metro: Symbol = FText::FromString(TEXT("T")); break;
                case ETMOPMapMarkerCategory::Club: Symbol = FText::FromString(TEXT("K")); break;
                case ETMOPMapMarkerCategory::Pub: Symbol = FText::FromString(TEXT("P")); break;
                case ETMOPMapMarkerCategory::Church: Symbol = FText::FromString(TEXT("†")); break;
                case ETMOPMapMarkerCategory::ATM: Symbol = FText::FromString(TEXT("A")); break;
                case ETMOPMapMarkerCategory::Hotel: Symbol = FText::FromString(TEXT("H")); break;
                case ETMOPMapMarkerCategory::BusStop: Symbol = FText::FromString(TEXT("B")); break;
                default: break;
                }
                if (!Widget->IsMinimap())
                    FSlateDrawElement::MakeText(Out, Layer + 1,
                        IconGeometry.ToPaintGeometry(), Symbol,
                        FCoreStyle::GetDefaultFontStyle("Bold", 10),
                        ESlateDrawEffect::None, FLinearColor::Black);
            }

            if (!Widget->IsMinimap() && !Marker.DisplayName.IsEmpty())
            {
                const FGeometry LabelGeometry = Geometry.MakeChild(FVector2D(150.0f, 24.0f),
                    FSlateLayoutTransform(P + FVector2D(-75.0f, IconSize * 0.6f + 2.0f)));
                FSlateDrawElement::MakeText(Out, Layer + 1, LabelGeometry.ToPaintGeometry(),
                    Marker.DisplayName, FCoreStyle::GetDefaultFontStyle("Bold", 12),
                    ESlateDrawEffect::None, FLinearColor::White);
            }
        }
        Layer += 2;

        FVector2D OlofScreenPoint = FVector2D::ZeroVector;
        bool bHasOlofScreenPoint = false;
        FVector OlofLocation;
        if (Map->GetOlofPalmeMapLocation(OlofLocation))
        {
            const FVector2D OlofP = MapOrigin +
                ToDisplayUV(Map->WorldToMapUV(OlofLocation)) * MapSize;
            OlofScreenPoint = OlofP;
            bHasOlofScreenPoint = true;
            const float OlofSize = Widget->IsMinimap() ? 14.0f : 26.0f;
            const FGeometry OlofGeometry = Geometry.MakeChild(FVector2D(OlofSize),
                FSlateLayoutTransform(OlofP - FVector2D(OlofSize * 0.5f)));
            if (IsValid(Map->OlofPalmeIcon))
            {
                FSlateBrush OlofBrush;
                OlofBrush.DrawAs = ESlateBrushDrawType::Image;
                OlofBrush.SetResourceObject(Map->OlofPalmeIcon);
                FSlateDrawElement::MakeBox(Out, Layer, OlofGeometry.ToPaintGeometry(),
                    &OlofBrush, ESlateDrawEffect::None, FLinearColor::White);
            }
            else
            {
                FSlateDrawElement::MakeBox(Out, Layer, OlofGeometry.ToPaintGeometry(),
                    White, ESlateDrawEffect::None, Map->OlofPalmeMarkerColor);
                FSlateDrawElement::MakeText(Out, Layer + 1,
                    OlofGeometry.ToPaintGeometry(), FText::FromString(TEXT("OP")),
                    FCoreStyle::GetDefaultFontStyle("Bold", Widget->IsMinimap() ? 7 : 10),
                    ESlateDrawEffect::None, FLinearColor::White);
            }
            Layer += 2;
        }

        const float PulsePhase = static_cast<float>(FPlatformTime::Seconds() * 3.2);
        const float PulseScale = 1.0f + FMath::Sin(PulsePhase) * 0.18f;
        const float PulseAlpha = 0.82f + FMath::Sin(PulsePhase) * 0.18f;
        auto DrawTrackedAgents = [&](const TArray<FVector>& Locations,
            UTexture2D* Icon, const FLinearColor& Color, const FText& Symbol,
            const bool bPulse)
        {
            const float BaseMarkerSize = Widget->IsMinimap() ? 8.0f : 15.0f;
            const float Size = BaseMarkerSize * (bPulse ? PulseScale : 1.0f);
            FLinearColor DrawColor = Color;
            if (bPulse) DrawColor.A *= PulseAlpha;
            for (const FVector& Location : Locations)
            {
                const FVector2D Point = MapOrigin +
                    ToDisplayUV(Map->WorldToMapUV(Location)) * MapSize;
                if (Point.X < MapOrigin.X || Point.Y < MapOrigin.Y ||
                    Point.X > MapOrigin.X + MapSize.X || Point.Y > MapOrigin.Y + MapSize.Y)
                    continue;
                const FGeometry MarkerGeometry = Geometry.MakeChild(FVector2D(Size),
                    FSlateLayoutTransform(Point - FVector2D(Size * 0.5f)));
                if (IsValid(Icon))
                {
                    FSlateBrush TrackingBrush;
                    TrackingBrush.DrawAs = ESlateBrushDrawType::Image;
                    TrackingBrush.SetResourceObject(Icon);
                    FSlateDrawElement::MakeBox(Out, Layer,
                        MarkerGeometry.ToPaintGeometry(), &TrackingBrush,
                        ESlateDrawEffect::None,
                        bPulse ? FLinearColor(1.0f, 1.0f, 1.0f, PulseAlpha)
                            : FLinearColor::White);
                }
                else
                {
                    FSlateDrawElement::MakeBox(Out, Layer,
                        MarkerGeometry.ToPaintGeometry(), White,
                        ESlateDrawEffect::None, DrawColor);
                    if (!Widget->IsMinimap())
                        FSlateDrawElement::MakeText(Out, Layer + 1,
                            MarkerGeometry.ToPaintGeometry(), Symbol,
                            FCoreStyle::GetDefaultFontStyle("Bold", 8),
                            ESlateDrawEffect::None, FLinearColor::Black);
                }
            }
            Layer += 2;
        };
        if (!bFullMap || Widget->ShouldShowObservations())
        {
            TArray<FVector> ObservedLocations;
            Map->GetObservedPersonMapLocations(ObservedLocations);
            DrawTrackedAgents(ObservedLocations, Map->ObservedPersonIcon,
                Map->ObservedPersonMarkerColor, FText::FromString(TEXT("O")), true);
        }
        if (!bFullMap || Widget->ShouldShowPolice())
        {
            TArray<FVector> PoliceLocations;
            Map->GetPoliceMapLocations(PoliceLocations);
            DrawTrackedAgents(PoliceLocations, Map->PoliceTrackingIcon,
                Map->PoliceMarkerColor, FText::FromString(TEXT("P")), false);
        }

        const FVector2D PlayerP = MapOrigin + ToDisplayUV(
            Map->WorldToMapUV(Map->GetTrackedWorldLocation())) * MapSize;
        FVector2D Forward = Map->GetTrackedMapDirection();
        if (bRotateClockwise) Forward = FVector2D(-Forward.Y, Forward.X);
        const FVector2D Right(-Forward.Y, Forward.X);
        const float PlayerPulse = PulseScale;
        const TArray<FVector2D> Arrow = {
            PlayerP + Forward * (13.0f * PlayerPulse),
            PlayerP - Forward * (9.0f * PlayerPulse) + Right * (7.0f * PlayerPulse),
            PlayerP - Forward * (5.0f * PlayerPulse),
            PlayerP - Forward * (9.0f * PlayerPulse) - Right * (7.0f * PlayerPulse),
            PlayerP + Forward * (13.0f * PlayerPulse) };
        FSlateDrawElement::MakeLines(Out, Layer++, Geometry.ToPaintGeometry(), Arrow,
            ESlateDrawEffect::None,
            FLinearColor(0.12f, 0.72f, 1.0f, PulseAlpha), true, 3.0f);

        if (bFullMap)
        {
            struct FLegendEntry
            {
                FText Label;
                UTexture2D* Icon = nullptr;
                FLinearColor Color = FLinearColor::White;
                FText Symbol;
            };
            TArray<FLegendEntry> Legend;
            auto AddCategory = [&](const TCHAR* Label,
                const ETMOPMapMarkerCategory Category, const FLinearColor Color,
                const TCHAR* Symbol)
            {
                FLegendEntry Entry;
                Entry.Label = FText::FromString(FString(Label));
                Entry.Icon = Map->GetCategoryIcon(Category);
                Entry.Color = Color;
                Entry.Symbol = FText::FromString(Symbol);
                Legend.Add(Entry);
            };
            AddCategory(TEXT("Restaurang"), ETMOPMapMarkerCategory::Restaurant,
                FLinearColor(1.0f, 0.65f, 0.12f), TEXT("R"));
            AddCategory(TEXT("Biograf"), ETMOPMapMarkerCategory::Cinema,
                FLinearColor(0.95f, 0.3f, 0.25f), TEXT("B"));
            AddCategory(TEXT("Tunnelbana"), ETMOPMapMarkerCategory::Metro,
                FLinearColor(0.15f, 0.65f, 1.0f), TEXT("T"));
            AddCategory(TEXT("Klubb"), ETMOPMapMarkerCategory::Club,
                FLinearColor(0.75f, 0.25f, 1.0f), TEXT("K"));
            AddCategory(TEXT("Pub"), ETMOPMapMarkerCategory::Pub,
                FLinearColor(0.25f, 0.85f, 0.45f), TEXT("P"));
            AddCategory(TEXT("Kyrka"), ETMOPMapMarkerCategory::Church,
                FLinearColor(0.88f, 0.88f, 0.72f), TEXT("†"));
            AddCategory(TEXT("Bankomat"), ETMOPMapMarkerCategory::ATM,
                FLinearColor(0.3f, 0.95f, 0.6f), TEXT("A"));
            AddCategory(TEXT("Hotell"), ETMOPMapMarkerCategory::Hotel,
                FLinearColor(0.45f, 0.7f, 1.0f), TEXT("H"));
            AddCategory(TEXT("Busshållplats"), ETMOPMapMarkerCategory::BusStop,
                FLinearColor(0.95f, 0.78f, 0.18f), TEXT("B"));

            Legend.Add({NSLOCTEXT("TMOP", "MapLegendObserved", "Observerade personer/bilar"),
                Map->ObservedPersonIcon, Map->ObservedPersonMarkerColor,
                FText::FromString(TEXT("O"))});
            Legend.Add({NSLOCTEXT("TMOP", "MapLegendPolice", "Polis"),
                Map->PoliceTrackingIcon, Map->PoliceMarkerColor,
                FText::FromString(TEXT("P"))});
            Legend.Add({NSLOCTEXT("TMOP", "MapLegendPlayer", "Du – live"),
                nullptr, FLinearColor(0.12f, 0.72f, 1.0f),
                FText::FromString(TEXT("▲"))});
            Legend.Add({NSLOCTEXT("TMOP", "MapLegendOlof", "Olof Palme – live"),
                Map->OlofPalmeIcon, Map->OlofPalmeMarkerColor,
                FText::FromString(TEXT("OP"))});

            const float LegendX = 24.0f;
            float LegendY = 105.0f;
            const FGeometry LegendTitle = Geometry.MakeChild(FVector2D(185.0f, 28.0f),
                FSlateLayoutTransform(FVector2D(LegendX, LegendY - 38.0f)));
            FSlateDrawElement::MakeText(Out, Layer, LegendTitle.ToPaintGeometry(),
                NSLOCTEXT("TMOP", "MapLegendTitle", "TECKENFÖRKLARING"),
                FCoreStyle::GetDefaultFontStyle("Bold", 13),
                ESlateDrawEffect::None, FLinearColor(0.92f, 0.92f, 0.88f));
            for (const FLegendEntry& Entry : Legend)
            {
                const float IconSize = 18.0f;
                const FGeometry IconGeometry = Geometry.MakeChild(FVector2D(IconSize),
                    FSlateLayoutTransform(FVector2D(LegendX, LegendY)));
                if (IsValid(Entry.Icon))
                {
                    FSlateBrush LegendBrush;
                    LegendBrush.DrawAs = ESlateBrushDrawType::Image;
                    LegendBrush.SetResourceObject(Entry.Icon);
                    FSlateDrawElement::MakeBox(Out, Layer,
                        IconGeometry.ToPaintGeometry(), &LegendBrush,
                        ESlateDrawEffect::None, FLinearColor::White);
                }
                else
                {
                    FSlateDrawElement::MakeBox(Out, Layer,
                        IconGeometry.ToPaintGeometry(), White,
                        ESlateDrawEffect::None, Entry.Color);
                    FSlateDrawElement::MakeText(Out, Layer + 1,
                        IconGeometry.ToPaintGeometry(), Entry.Symbol,
                        FCoreStyle::GetDefaultFontStyle("Bold", 9),
                        ESlateDrawEffect::None, FLinearColor::Black);
                }
                const FGeometry TextGeometry = Geometry.MakeChild(FVector2D(155.0f, 22.0f),
                    FSlateLayoutTransform(FVector2D(LegendX + 28.0f, LegendY - 1.0f)));
                FSlateDrawElement::MakeText(Out, Layer + 1,
                    TextGeometry.ToPaintGeometry(), Entry.Label,
                    FCoreStyle::GetDefaultFontStyle("Regular", 11),
                    ESlateDrawEffect::None, FLinearColor(0.9f, 0.9f, 0.88f));
                LegendY += 27.0f;
            }

            LegendY += 16.0f;
            struct FFilterEntry
            {
                FText Label;
                bool bEnabled = true;
            };
            const TArray<FFilterEntry> Filters = {
                {NSLOCTEXT("TMOP", "MapFilterPlaces", "Visa platser"),
                    Widget->ShouldShowPlaces()},
                {NSLOCTEXT("TMOP", "MapFilterObservations", "Visa observationer"),
                    Widget->ShouldShowObservations()},
                {NSLOCTEXT("TMOP", "MapFilterPolice", "Visa poliser"),
                    Widget->ShouldShowPolice()}
            };
            for (const FFilterEntry& Filter : Filters)
            {
                const FGeometry BoxGeometry = Geometry.MakeChild(FVector2D(18.0f),
                    FSlateLayoutTransform(FVector2D(LegendX, LegendY)));
                FSlateDrawElement::MakeBox(Out, Layer, BoxGeometry.ToPaintGeometry(),
                    White, ESlateDrawEffect::None,
                    Filter.bEnabled ? FLinearColor(0.92f, 0.92f, 0.92f, 1.0f)
                        : FLinearColor(0.15f, 0.15f, 0.15f, 0.9f));
                if (Filter.bEnabled)
                    FSlateDrawElement::MakeText(Out, Layer + 1,
                        BoxGeometry.ToPaintGeometry(), FText::FromString(TEXT("✓")),
                        FCoreStyle::GetDefaultFontStyle("Bold", 12),
                        ESlateDrawEffect::None, FLinearColor::Black);
                const FGeometry FilterText = Geometry.MakeChild(FVector2D(160.0f, 22.0f),
                    FSlateLayoutTransform(FVector2D(LegendX + 28.0f, LegendY - 1.0f)));
                FSlateDrawElement::MakeText(Out, Layer + 1,
                    FilterText.ToPaintGeometry(), Filter.Label,
                    FCoreStyle::GetDefaultFontStyle("Regular", 11),
                    ESlateDrawEffect::None,
                    Filter.bEnabled ? FLinearColor::White
                        : FLinearColor(0.5f, 0.5f, 0.5f, 1.0f));
                LegendY += 28.0f;
            }
            Layer += 2;

            const FVector2D PlayerLabelAnchor(ContentOrigin.X + ContentSize.X * 0.5f,
                22.0f);
            FSlateDrawElement::MakeLines(Out, Layer++, Geometry.ToPaintGeometry(),
                {PlayerLabelAnchor + FVector2D(0.0f, 25.0f), PlayerP},
                ESlateDrawEffect::None, FLinearColor::White, true, 2.0f);
            const FGeometry PlayerTopLabel = Geometry.MakeChild(FVector2D(180.0f, 28.0f),
                FSlateLayoutTransform(PlayerLabelAnchor + FVector2D(-90.0f, -7.0f)));
            FSlateDrawElement::MakeText(Out, Layer++, PlayerTopLabel.ToPaintGeometry(),
                NSLOCTEXT("TMOP", "PlayerFixedMapLabel", "DU ÄR HÄR"),
                FCoreStyle::GetDefaultFontStyle("Bold", 15),
                ESlateDrawEffect::None, FLinearColor::White);

            const FVector2D LabelAnchor(ViewSize.X * 0.5f, ViewSize.Y - 48.0f);
            if (bHasOlofScreenPoint)
            {
                FLinearColor OlofLineColor = Map->OlofPalmeMarkerColor;
                OlofLineColor.A = 0.9f;
                FSlateDrawElement::MakeLines(Out, Layer++, Geometry.ToPaintGeometry(),
                    {LabelAnchor, OlofScreenPoint}, ESlateDrawEffect::None,
                    OlofLineColor, true, 2.0f);
            }
            const FGeometry OlofBottomLabel = Geometry.MakeChild(
                FVector2D(220.0f, 30.0f),
                FSlateLayoutTransform(LabelAnchor + FVector2D(-110.0f, 7.0f)));
            FSlateDrawElement::MakeText(Out, Layer++,
                OlofBottomLabel.ToPaintGeometry(),
                NSLOCTEXT("TMOP", "OlofPalmeFixedMapLabel", "OLOF PALME"),
                FCoreStyle::GetDefaultFontStyle("Bold", 15),
                ESlateDrawEffect::None, Map->OlofPalmeMarkerColor);
        }

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

    virtual FReply OnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() == EKeys::LeftMouseButton &&
            OwnerWidget.IsValid() && !OwnerWidget->IsMinimap())
        {
            const FVector2D WidgetPosition = Geometry.AbsoluteToLocal(
                Event.GetScreenSpacePosition());
            constexpr float FilterX = 24.0f;
            constexpr float FilterY = 472.0f;
            constexpr float FilterWidth = 190.0f;
            constexpr float FilterRowHeight = 28.0f;
            if (WidgetPosition.X >= FilterX && WidgetPosition.X <= FilterX + FilterWidth &&
                WidgetPosition.Y >= FilterY &&
                WidgetPosition.Y < FilterY + FilterRowHeight * 3.0f)
            {
                const int32 FilterIndex = FMath::FloorToInt(
                    (WidgetPosition.Y - FilterY) / FilterRowHeight);
                OwnerWidget->ToggleMapFilter(FilterIndex);
                return FReply::Handled();
            }
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
        + SOverlay::Slot().HAlign(HAlign_Right).VAlign(VAlign_Top).Padding(24.0f)
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
    // The large map always opens centred. The minimap still follows the player
    // through GetViewCenterUV().
    FullMapCenterUV = FVector2D(0.5f, 0.5f);
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
    const FVector2D SafeSize(FMath::Max(1.0f, ViewSize.X - 260.0f),
        FMath::Max(1.0f, ViewSize.Y - 138.0f));
    const FVector2D DisplayDelta = PixelDelta / (SafeSize * FullMapZoom);
    const FVector2D SourceDelta = IsValid(MapComponent) &&
        MapComponent->bRotateDisplay90DegreesClockwise
        ? FVector2D(DisplayDelta.Y, -DisplayDelta.X) : DisplayDelta;
    FullMapCenterUV -= SourceDelta;
    FullMapCenterUV.X = FMath::Clamp(FullMapCenterUV.X, 0.0f, 1.0f);
    FullMapCenterUV.Y = FMath::Clamp(FullMapCenterUV.Y, 0.0f, 1.0f);
}

void UTMOPMapWidget::RequestClose()
{
    if (IsValid(PlayerCharacter)) PlayerCharacter->CloseWorldMap();
}

void UTMOPMapWidget::ToggleMapFilter(const int32 FilterIndex)
{
    switch (FilterIndex)
    {
    case 0: bShowPlaces = !bShowPlaces; break;
    case 1: bShowObservations = !bShowObservations; break;
    case 2: bShowPolice = !bShowPolice; break;
    default: return;
    }
    InvalidateLayoutAndVolatility();
}
