#include "UI/TMOPHUDTimelineWidget.h"
#include "Time/TMOPClockSubsystem.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Engine/GameInstance.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
class STMOPHUDTimeline : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPHUDTimeline) {}
        SLATE_ARGUMENT(UTMOPHUDTimelineWidget*, Owner)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        Owner = Args._Owner;
        ForceVolatile(true); // Read clock and resolved events even after seeks/loop resets.
        SetClipping(EWidgetClipping::ClipToBoundsAlways);
    }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(800,150); }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&,
        FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
    {
        if (!Owner.IsValid() || !Owner->GetGameInstance()) return Layer;
        const auto* Clock = Owner->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
        const auto* Events = Owner->GetGameInstance()->GetSubsystem<UTMOPHistoricalEventSubsystem>();
        if (!Clock) return Layer;
        const FVector2D View = G.GetLocalSize();
        // Reserve the outer fifths for the watch and minimap in each player's viewport.
        const double W = View.X * 0.60;
        const double Left = View.X * 0.20;
        const double Top = FMath::Max(0.0, View.Y - 152.0);
        const double X0 = Left + 26, X1 = Left + W - 26, Y = Top + 47;
        if (X1 <= X0) return Layer;
        const auto Geo = [&G](FVector2D P,FVector2D S) { return G.MakeChild(S,FSlateLayoutTransform(P)).ToPaintGeometry(); };
        const auto Box = [&](FVector2D P,FVector2D S,FLinearColor C,int32 L)
        { FSlateDrawElement::MakeBox(Out,L,Geo(P,S),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,C); };
        const int32 FontSize = W < 550 ? 9 : 11;
        const auto Text = [&](FString S,FVector2D P,FLinearColor C)
        { FSlateDrawElement::MakeText(Out,Layer+4,Geo(P,FVector2D(W,20)),S,FCoreStyle::GetDefaultFontStyle("Regular",FontSize),ESlateDrawEffect::None,C); };
        const auto X = [&](double Second) { return X0 + FMath::Clamp((Second-82800.0)/2700.0,0.0,1.0)*(X1-X0); };
        Box(FVector2D(Left,Top),FVector2D(W,140),FLinearColor(0,0,0,0.8f),Layer);
        Box(FVector2D(X0,Y),FVector2D(X1-X0,2),FLinearColor(0.5f,0.5f,0.5f),Layer+1);
        for (int32 Minute=0; Minute<=45; Minute+=5)
        {
            const double P=X(82800+Minute*60);
            Box(FVector2D(P,Y),FVector2D(1,6),FLinearColor::White,Layer+2);
            if (W>=550 || Minute%15==0)
                Text(FString::Printf(TEXT("23:%02d"),Minute),FVector2D(P-17,Y+9),FLinearColor::White);
        }
        const FLinearColor Gold(1,0.76f,0.2f);
        for (int32 I=0; I<Owner->Markers.Num() && I<6; ++I)
        {
            const auto& M=Owner->Markers[I];
            FTMOPHistoricalEventRuntime Runtime;
            const bool Resolved=Events && !M.SharedEventId.IsNone() &&
                Events->TryGetEventRuntime(M.SharedEventId,Runtime) && Runtime.bHasResolvedTime &&
                Runtime.State!=ETMOPEventRuntimeState::Cancelled;
            const int32 Second=Runtime.ResolvedTime.ToSecondsFromMidnight();
            if (Resolved && Second>=82800 && Second<=85500)
            {
                const double P=X(Second);
                Box(FVector2D(P-1,Y-9),FVector2D(2,10),Gold,Layer+2);
                // Six separate label heights keep even coincident event markers readable.
                Text(FString::FromInt(I+1),FVector2D(P-3,Top+2+(I%3)*11),Gold);
            }
            const FString Time=Resolved ? Runtime.ResolvedTime.ToDisplayString() : TEXT("—");
            const double Column=(W-20)/3;
            Text(FString::Printf(TEXT("%d %s"),I+1,*M.Label.ToString()),
                FVector2D(Left+10+(I%3)*Column,Top+76+(I/3)*29),Gold);
            Text(Time,FVector2D(Left+10+(I%3)*Column,Top+89+(I/3)*29),FLinearColor::White);
        }
        const double Now=Clock->GetCurrentTimeSecondsExact();
        const double P=X(Now);
        Box(FVector2D(P-1,Y-13),FVector2D(3,22),FLinearColor(0.15f,1,0.85f),Layer+3);
        return Layer+4;
    }
private:
    TWeakObjectPtr<UTMOPHUDTimelineWidget> Owner;
};
}
TSharedRef<SWidget> UTMOPHUDTimelineWidget::RebuildWidget()
{
    return SNew(STMOPHUDTimeline).Owner(this);
}
