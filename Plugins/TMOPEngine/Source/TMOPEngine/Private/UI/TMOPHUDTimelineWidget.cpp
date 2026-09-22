#include "UI/TMOPHUDTimelineWidget.h"
#include "Time/TMOPWorldPlaybackComponent.h"
#include "Time/TMOPTimeTravelPolicy.h"
#include "Widgets/Layout/SConstraintCanvas.h"
#include "Input/Reply.h"
#include "InputCoreTypes.h"
#include "Time/TMOPClockSubsystem.h"
#include "Events/TMOPHistoricalEventSubsystem.h"
#include "Engine/GameInstance.h"
#include "Widgets/SLeafWidget.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Fonts/FontMeasure.h"

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
    virtual bool SupportsKeyboardFocus() const override { return true; }
    bool OverHandle(const FGeometry& G, FVector2D Screen) const
    {
        if (!Owner.IsValid() || !Owner->IsTimelineInputEnabled()) return false;
        auto* Clock = Owner->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
        const double Start = Clock->GetLoopStartTime().ToSecondsFromMidnight();
        const double End = Clock->GetLoopEndTime().ToSecondsFromMidnight();
        const double X = 26 + (Clock->GetCurrentTimeSecondsExact() - Start) / FMath::Max(1.0, End - Start) * (G.GetLocalSize().X - 52);
        const FVector2D P = G.AbsoluteToLocal(Screen);
        return FMath::Abs(P.X - X) <= 14 && FMath::Abs(P.Y - (G.GetLocalSize().Y - 40)) <= 20;
    }
    void UpdateTarget(const FGeometry& G, const FPointerEvent& E)
    {
        auto* Clock = Owner->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
        const int32 Start = Clock->GetLoopStartTime().ToSecondsFromMidnight();
        const int32 End = Clock->GetLoopEndTime().ToSecondsFromMidnight();
        const double X = G.AbsoluteToLocal(E.GetScreenSpacePosition()).X;
        Target = TMOPTimeTravel::Snap(Start + (X - 26) / FMath::Max(1.0, G.GetLocalSize().X - 52) * (End - Start), Start, End);
    }
    virtual FReply OnMouseButtonDown(const FGeometry& G, const FPointerEvent& E) override
    {
        if (E.GetEffectingButton() != EKeys::LeftMouseButton || !OverHandle(G, E.GetScreenSpacePosition())) return FReply::Unhandled();
        auto* Replay = UTMOPWorldPlaybackComponent::Find(Owner.Get());
        if (!Replay || !Replay->BeginScrub(Owner.Get())) return FReply::Handled();
        bDragging = true;
        UpdateTarget(G, E);
        return FReply::Handled().CaptureMouse(SharedThis(this)).SetUserFocus(SharedThis(this));
    }
    virtual FReply OnMouseMove(const FGeometry& G, const FPointerEvent& E) override
    {
        bHoverHandle = OverHandle(G, E.GetScreenSpacePosition());
        if (!bDragging) return FReply::Unhandled();
        UpdateTarget(G, E);
        return FReply::Handled();
    }
    virtual void OnMouseLeave(const FPointerEvent& E) override
    { bHoverHandle = false; SLeafWidget::OnMouseLeave(E); }
    virtual FReply OnMouseButtonUp(const FGeometry& G, const FPointerEvent& E) override
    {
        if (!bDragging || E.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
        UpdateTarget(G, E);
        bDragging = false;
        if (auto* Replay = UTMOPWorldPlaybackComponent::Find(Owner.Get()))
            if (!Replay->RequestSeek(Target, Owner.Get())) Replay->CancelScrub(Owner.Get());
        return FReply::Handled().ReleaseMouseCapture();
    }
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& E) override
    {
        bDragging = false;
        if (auto* Replay = UTMOPWorldPlaybackComponent::Find(Owner.Get())) Replay->CancelScrub(Owner.Get());
        SLeafWidget::OnMouseCaptureLost(E);
    }
    virtual FReply OnKeyDown(const FGeometry& G, const FKeyEvent& E) override
    {
        if (E.GetKey() == EKeys::Escape && bDragging)
        {
            bDragging = false;
            if (auto* Replay = UTMOPWorldPlaybackComponent::Find(Owner.Get())) Replay->CancelScrub(Owner.Get());
            return FReply::Handled().ReleaseMouseCapture();
        }
        return SLeafWidget::OnKeyDown(G, E);
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
        const double W = View.X;
        const double Left = 0;
        const double X0 = Left + 26, X1 = Left + W - 26;
        const double Y = FMath::Max(20.0, View.Y - 40.0);
        if (X1 <= X0) return Layer;
        const auto Geo = [&G](FVector2D P,FVector2D S) { return G.MakeChild(S,FSlateLayoutTransform(P)).ToPaintGeometry(); };
        const auto Box = [&](FVector2D P,FVector2D S,FLinearColor C,int32 L)
        { FSlateDrawElement::MakeBox(Out,L,Geo(P,S),FCoreStyle::Get().GetBrush("WhiteBrush"),ESlateDrawEffect::None,C); };
        const int32 FontSize = W < 550 ? 9 : 11;
        const auto Text = [&](FString S,FVector2D P,FLinearColor C)
        { FSlateDrawElement::MakeText(Out,Layer+4,Geo(P,FVector2D(W,20)),S,FCoreStyle::GetDefaultFontStyle("Regular",FontSize),ESlateDrawEffect::None,C); };
        const double Start = Clock->GetLoopStartTime().ToSecondsFromMidnight();
        const double End = Clock->GetLoopEndTime().ToSecondsFromMidnight();
        const auto X = [&](double Second) { return X0 + FMath::Clamp((Second-Start)/FMath::Max(1.0, End-Start),0.0,1.0)*(X1-X0); };
        struct FLabel { FString Text; double Point; double LeftEdge; double Width; int32 Row; };
        TArray<FLabel> Labels;
        const auto Measure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();
        const auto Font = FCoreStyle::GetDefaultFontStyle("Regular",FontSize);
        for (int32 I=0; I<Owner->Markers.Num() && I<6; ++I)
        {
            const auto& M = Owner->Markers[I];
            FTMOPHistoricalEventRuntime Runtime;
            if (!Events || M.SharedEventId.IsNone() ||
                !Events->TryGetEventRuntime(M.SharedEventId,Runtime) || !Runtime.bHasResolvedTime ||
                Runtime.State == ETMOPEventRuntimeState::Cancelled) continue;
            const int32 Second = Runtime.ResolvedTime.ToSecondsFromMidnight();
            if (Second < Start || Second > End) continue;
            const FString Caption = M.Label.ToString();
            const double Width = Measure->Measure(Caption,Font).X;
            const double Point = X(Second);
            const double LabelLeft = FMath::Clamp(Point-Width*0.5, Left+6,
                FMath::Max(Left+6,Left+W-Width-6));
            Labels.Add({Caption,Point,LabelLeft,Width,0});
        }
        Labels.StableSort([](const FLabel& A,const FLabel& B) { return A.Point < B.Point; });
        TArray<double> RowEnds;
        for (auto& Label : Labels)
        {
            int32 Row = 0;
            while (Row < RowEnds.Num() && Label.LeftEdge < RowEnds[Row]+10) ++Row;
            if (Row == RowEnds.Num()) RowEnds.Add(0);
            RowEnds[Row] = Label.LeftEdge+Label.Width;
            Label.Row = Row;
        }
        const double Top = Y - 16 - FMath::Max(1,RowEnds.Num())*18;
        Box(FVector2D(Left,Top),FVector2D(W,View.Y-Top-8),FLinearColor(0,0,0,0.8f),Layer);
        Box(FVector2D(X0,Y),FVector2D(X1-X0,2),FLinearColor(0.5f,0.5f,0.5f),Layer+1);
        for (int32 Minute=0; Minute<=45; Minute+=5)
        {
            const double P=X(82800+Minute*60);
            Box(FVector2D(P,Y),FVector2D(1,6),FLinearColor::White,Layer+2);
            if (W>=550 || Minute%15==0)
                Text(FString::Printf(TEXT("23:%02d"),Minute),FVector2D(P-17,Y+9),FLinearColor::White);
        }
        const FLinearColor Gold(1,0.76f,0.2f);
        for (const auto& Label : Labels)
        {
            const double TextY = Y-24-Label.Row*18;
            Box(FVector2D(Label.Point-1,Y-8),FVector2D(2,9),Gold,Layer+2);
            Box(FVector2D(Label.Point,TextY+15),FVector2D(1,Y-TextY-15),
                FLinearColor(1,0.76f,0.2f,0.4f),Layer+1);
            Text(Label.Text,FVector2D(Label.LeftEdge,TextY),Gold);
        }
        const double Now=Clock->GetCurrentTimeSecondsExact();
        const double P=X(Now);
        Box(FVector2D(P-1,Y-13),FVector2D(3,22),FLinearColor(0.15f,1,0.85f),Layer+3);
        if (bHoverHandle && !bDragging)
            Text(TEXT("dra för tidsförflyttning"), FVector2D(FMath::Clamp(P-90, X0, FMath::Max(X0, X1-180)), Y-65), FLinearColor::White);
        if (bDragging)
        {
            Box(FVector2D(X(Target)-2,Y-17), FVector2D(4,29), FLinearColor::White, Layer+5);
            Text(FTMOPTime::FromSecondsFromMidnight(Target).ToDisplayString(), FVector2D(FMath::Clamp(X(Target)-35,X0,FMath::Max(X0,X1-70)),Y-45), FLinearColor::White);
        }
        if (auto* Replay = UTMOPWorldPlaybackComponent::Find(Owner.Get()))
        {
            if (!Replay->GetStatus().IsEmpty())
                Text(Replay->GetStatus(), FVector2D(X0,12), FLinearColor::White);
            else if (Replay->IsScrubbing() && !bDragging)
                Text(TEXT("En spelare väljer tid…"), FVector2D(X0,12), FLinearColor::White);
            if (Replay->IsBusy())
            {
                const double Angle = FSlateApplication::Get().GetCurrentTime() * 6;
                TArray<FVector2D> Points;
                for (int32 I=0; I<10; ++I)
                    Points.Add(FVector2D(X1-10 + FMath::Cos(Angle+I*0.4)*7,20 + FMath::Sin(Angle+I*0.4)*7));
                FSlateDrawElement::MakeLines(Out,Layer+6,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,FLinearColor::White,true,2);
            }
        }
        return Layer+6;
    }
private:
    TWeakObjectPtr<UTMOPHUDTimelineWidget> Owner;
    bool bDragging = false;
    bool bHoverHandle = false;
    int32 Target = 82800;
};
}
TSharedRef<SWidget> UTMOPHUDTimelineWidget::RebuildWidget()
{
    TimelineSlate = SNew(STMOPHUDTimeline).Owner(this);
    TimelineSlate->SetVisibility(bTimelineInputEnabled ? EVisibility::Visible : EVisibility::HitTestInvisible);
    return SNew(SConstraintCanvas)
        + SConstraintCanvas::Slot().Anchors(FAnchors(0.2f,1.0f,0.8f,1.0f))
        .Offset(FMargin(0,-150,0,150))
        [ TimelineSlate.ToSharedRef() ];
}

void UTMOPHUDTimelineWidget::SetTimelineInputEnabled(bool bEnabled)
{
    bTimelineInputEnabled = bEnabled;
    if (!bEnabled && TimelineSlate.IsValid() && TimelineSlate->HasMouseCapture())
        FSlateApplication::Get().ReleaseAllPointerCapture();
    if (TimelineSlate.IsValid()) TimelineSlate->SetVisibility(bEnabled ? EVisibility::Visible : EVisibility::HitTestInvisible);
    if (!bEnabled) if (auto* Replay = UTMOPWorldPlaybackComponent::Find(this)) Replay->CancelScrub(this);
}
void UTMOPHUDTimelineWidget::NativeDestruct()
{
    if (auto* Replay = UTMOPWorldPlaybackComponent::Find(this)) Replay->CancelScrub(this);
    Super::NativeDestruct();
}
