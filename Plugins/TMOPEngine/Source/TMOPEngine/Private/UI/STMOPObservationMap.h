#pragma once
#include "Widgets/SLeafWidget.h"
#include "UI/TMOPMapComponent.h"
#include "Observations/TMOPNotebookTypes.h"
#include "Time/TMOPTime.h"
#include "Engine/Texture2D.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

/** Static, per-person observation map. No live actors or future route markers. */
class STMOPObservationMap : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPObservationMap) {}
        SLATE_ARGUMENT(UTMOPMapComponent*, Map)
        SLATE_ARGUMENT(TArray<FTMOPNotebookLocation>, Points)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        Map = Args._Map; Points = Args._Points;
        SetClipping(EWidgetClipping::ClipToBoundsAlways);
        if (Map.IsValid() && IsValid(Map->MapTexture))
        {
            Brush.SetResourceObject(Map->MapTexture);
            Brush.DrawAs = ESlateBrushDrawType::Image;
        }
    }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(440, 270); }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&,
        FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
    {
        const auto* White = FCoreStyle::Get().GetBrush("WhiteBrush");
        const FVector2D View = G.GetLocalSize();
        const auto Geo = [&G](FVector2D P, FVector2D Size) { return G.MakeChild(Size, FSlateLayoutTransform(P)).ToPaintGeometry(); };
        FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(), White, ESlateDrawEffect::None, FLinearColor(0.08f, 0.08f, 0.08f));
        if (!Map.IsValid() || !IsValid(Map->MapTexture) || Points.IsEmpty())
        {
            FSlateDrawElement::MakeText(Out, Layer + 1, Geo(FVector2D(12,12), View),
                Points.IsEmpty() ? TEXT("Inga fastställda observationspunkter.") : TEXT("Kartbild saknas i spelarens Map Component."),
                FCoreStyle::GetDefaultFontStyle("Regular", 13), ESlateDrawEffect::None, FLinearColor::White);
            return Layer + 1;
        }
        const bool Rotate = Map->bRotateDisplay90DegreesClockwise;
        const auto UV = [&](FVector Location) { const FVector2D P = Map->WorldToMapUV(Location);
            return Rotate ? FVector2D(1-P.Y, P.X) : P; };
        FVector2D Min = UV(Points[0].WorldLocation), Max = Min;
        for (const auto& P : Points) { const FVector2D U = UV(P.WorldLocation);
            Min.X = FMath::Min(Min.X,U.X); Min.Y = FMath::Min(Min.Y,U.Y);
            Max.X = FMath::Max(Max.X,U.X); Max.Y = FMath::Max(Max.Y,U.Y); }
        double Aspect = double(Map->MapTexture->GetSizeX()) / FMath::Max(1,Map->MapTexture->GetSizeY());
        if (Rotate) Aspect = 1.0 / Aspect;
        FVector2D Base(View.X, View.X / Aspect);
        if (Base.Y > View.Y) Base = FVector2D(View.Y * Aspect, View.Y);
        const double Zoom = FMath::Clamp(FMath::Min((View.X-80) / FMath::Max(1.0,(Max.X-Min.X)*Base.X),
            (View.Y-90) / FMath::Max(1.0,(Max.Y-Min.Y)*Base.Y)), 0.1, 8.0);
        const FVector2D Size = Base * Zoom;
        const FVector2D Origin = View * 0.5 - (Min+Max)*0.5*Size;
        if (Rotate)
        {
            const FVector2D SourceSize(Size.Y,Size.X);
            FSlateDrawElement::MakeRotatedBox(Out, Layer+1, Geo(Origin+(Size-SourceSize)*0.5,SourceSize),
                &Brush, ESlateDrawEffect::None, PI*0.5f, TOptional<FVector2D>(),
                FSlateDrawElement::RelativeToElement,FLinearColor::White);
        }
        else FSlateDrawElement::MakeBox(Out, Layer+1, Geo(Origin,Size), &Brush, ESlateDrawEffect::None,FLinearColor::White);
        TArray<FVector2D> Labels;
        for (const auto& Point : Points)
        {
            const FVector2D P = Origin + UV(Point.WorldLocation)*Size;
            FVector2D Label(FMath::Clamp(P.X-32.0, 4.0, FMath::Max(4.0,View.X-74.0)), P.Y-24);
            for (int32 Attempt=0; Attempt<Points.Num(); ++Attempt)
            {
                if (!Labels.ContainsByPredicate([&](const FVector2D& Other)
                    { return FMath::Abs(Other.X-Label.X)<74 && FMath::Abs(Other.Y-Label.Y)<18; })) break;
                Label.Y -= 19;
            }
            Labels.Add(Label);
            FSlateDrawElement::MakeBox(Out,Layer+2,Geo(P-FVector2D(4,4),FVector2D(8,8)),White,ESlateDrawEffect::None,FLinearColor::Red);
            FSlateDrawElement::MakeBox(Out,Layer+3,Geo(Label-FVector2D(2,1),FVector2D(74,18)),White,ESlateDrawEffect::None,FLinearColor::Black);
            FSlateDrawElement::MakeText(Out,Layer+4,Geo(Label,FVector2D(74,18)),
                FTMOPTime::FromSecondsFromMidnight(Point.Second).ToDisplayString(),
                FCoreStyle::GetDefaultFontStyle("Bold",12),ESlateDrawEffect::None,FLinearColor::White);
        }
        return Layer+4;
    }
private:
    TWeakObjectPtr<UTMOPMapComponent> Map;
    TArray<FTMOPNotebookLocation> Points;
    FSlateBrush Brush;
};
