#include "WorldAtlas/STMOPWorldAtlas.h"
#include "WorldAtlas/TMOPWorldAtlasData.h"
#include "WorldAtlas/TMOPGlobeMath.h"
#include "WorldAtlas/TMOPGlobeScene.h"
#include "WorldAtlas/TMOPAtlasAppearance.h"
#include "WorldAtlas/STMOPAtlasHierarchy.h"
#include "Localization/TMOPLocalization.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/SNullWidget.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElements.h"
#include "Rendering/RenderingCommon.h"
#include "Framework/Application/SlateApplication.h"
#include "Rendering/SlateRenderer.h"
#include "Styling/CoreStyle.h"
#include "InputCoreTypes.h"
#include "HAL/PlatformProcess.h"

namespace
{
FText L(const FText& Source) { return FTMOPLocalization::Text(Source); }
FLinearColor Ink(const FString& Kind)
{
    if (Kind == TEXT("conflict")) return FLinearColor(1.0f, .30f, .18f);
    if (Kind == TEXT("arms")) return FLinearColor(1.0f, .72f, .22f);
    if (Kind == TEXT("funds")) return FLinearColor(.25f, .9f, .75f);
    if (Kind == TEXT("actor")) return FLinearColor(.35f, .85f, 1.0f);
    if (Kind == TEXT("event")) return FLinearColor(.75f, .55f, 1.0f);
    return FLinearColor(.8f, .88f, 1.0f);
}

FLinearColor ConflictInk(const FTMOPAtlasEntry& E)
{
    if (E.From > TEXT("1986-02-28")) return FLinearColor(.8f, .5f, 1.f);
    if (!E.To.IsEmpty() && E.To < TEXT("1986-02-28")) return FLinearColor(1.f, .7f, .2f);
    return Ink(TEXT("conflict"));
}
FText ConflictStatus(const FTMOPAtlasEntry& E)
{
    if (E.From > TEXT("1986-02-28")) return L(NSLOCTEXT("TMOP", "AtlasUpcoming", "Börjar efter morddatumet"));
    if (!E.To.IsEmpty() && E.To < TEXT("1986-02-28")) return L(NSLOCTEXT("TMOP", "AtlasEnded", "Avslutad före morddatumet"));
    return L(NSLOCTEXT("TMOP", "AtlasOngoing", "Pågående konfliktperiod vid morddatumet"));
}
bool LinkPoint(const FTMOPAtlasParticipant& A, const FTMOPAtlasParticipant& B, double T,
    const FQuat& Rotation, const FVector2D& Center, double Radius, FVector2D& P)
{
    if (!TMOPGlobe::Project(TMOPGlobe::Arc(TMOPGlobe::Unit(A.Latitude, A.Longitude),
        TMOPGlobe::Unit(B.Latitude, B.Longitude), T), Rotation, Center, Radius, P)) return false;
    P += A.Offset * (1.0 - T) + B.Offset * T;
    return true;
}
class STMOPGlobe final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPGlobe) {}
        SLATE_ARGUMENT(TFunction<void(const FString&)>, OnSelect)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args, TSharedRef<FTMOPWorldAtlasData> InData, TSharedRef<FTMOPAtlasAppearance> InAppearance,
        UStaticMesh* Mesh, UMaterialInterface* Material, FRotator Alignment, bool bInCoasts)
    {
        Data = InData; Appearance = InAppearance; OnSelect = Args._OnSelect; bCoasts = bInCoasts;
        Scene = MakeUnique<FTMOPGlobeScene>(Mesh, Material, Alignment);
        SetClipping(EWidgetClipping::ClipToBounds);
    }
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(520, 520); }
    void Focus(const FTMOPAtlasEntry& E)
    {
        Selected = E.Id; Latitude = E.Latitude; Longitude = E.Longitude; bDirty = true;
        Invalidate(EInvalidateWidgetReason::Paint);
    }
    void Turn(double Lon, double Lat)
    {
        Longitude = FMath::UnwindDegrees(Longitude + Lon);
        Latitude = FMath::Clamp(Latitude + Lat, -89.0, 89.0);
        bDirty = true; Invalidate(EInvalidateWidgetReason::Paint);
    }
    void ZoomBy(double Amount)
    {
        Zoom = FMath::Clamp(Zoom * FMath::Pow(2.0, Amount * 2.0), .55, 4.0);
        Invalidate(EInvalidateWidgetReason::Paint);
    }
    bool bNearby = false, bAllConflictLinks = false;
    bool bFlags = true, bBlocs = true;
    bool bLater = false, bCountries = true, bActors = true, bConflicts = true, bArms = true, bFunds = false;
    FString Selected;
    virtual void Tick(const FGeometry& G, double Time, float Delta) override
    {
        SLeafWidget::Tick(G, Time, Delta);
        if (bDirty && Scene) { Scene->Render(TMOPGlobe::View(Latitude, Longitude)); bDirty = false; }
    }
    bool DrawEntry(const FTMOPAtlasEntry& E) const
    {
        if (!E.Visible(bLater, bNearby)) return false;
        if (E.Kind == TEXT("country") || E.Kind == TEXT("region")) return bCountries;
        if (E.Kind == TEXT("actor")) return bActors;
        if (E.Kind == TEXT("conflict")) return bConflicts;
        if (E.Kind == TEXT("arms")) return bArms;
        if (E.Kind == TEXT("funds")) return bFunds;
        const FTMOPAtlasEntry* Active = Data->Find(Selected);
        return E.Kind == TEXT("event") && Active && (Active->Id == E.Id || Active->Related.Contains(E.Id));
    }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& G, const FSlateRect&,
        FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle&, bool) const override
    {
        const FVector2D Size = G.GetLocalSize(), Center = Size * .5;
        const double Radius = FMath::Min(Size.X, Size.Y) * (100.0 / 240.0) * Zoom;
        const FQuat Rotation = TMOPGlobe::View(Latitude, Longitude);
        auto Project = [&](const FVector& P, FVector2D& XY) { return TMOPGlobe::Project(P, Rotation, Center, Radius, XY); };
        const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
        FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(), White, ESlateDrawEffect::None, FLinearColor::Black);
        if (Scene && Scene->GetBrush())
        {
            const FVector2D ImageSize(Radius * 2.4, Radius * 2.4);
            FSlateDrawElement::MakeBox(Out, Layer + 1,
                G.MakeChild(ImageSize, FSlateLayoutTransform(Center - ImageSize * .5)).ToPaintGeometry(),
                Scene->GetBrush(), ESlateDrawEffect::None, FLinearColor::White);
        }
        auto Segment = [&](const FVector& A, const FVector& B, const FLinearColor& Color, float Width, int32 AtLayer)
        {
            FVector2D P, Q;
            // Never bridge the back of the globe. Sampling keeps limb gaps below a few pixels.
            if (Project(A, P) && Project(B, Q))
                FSlateDrawElement::MakeLines(Out, AtLayer, G.ToPaintGeometry(), TArray<FVector2f>{FVector2f(P), FVector2f(Q)}, ESlateDrawEffect::None, Color, true, Width);
        };
        if (bBlocs && Appearance) PaintLand(G, Out, Layer + 2, Rotation, Center, Radius);
        for (int Lat = -60; Lat <= 60; Lat += 30) for (int Lon = -180; Lon < 180; Lon += 4)
            Segment(TMOPGlobe::Unit(Lat, Lon), TMOPGlobe::Unit(Lat, Lon + 4), FLinearColor(.2f, .4f, .5f, .3f), .6f, Layer + 2);
        for (int Lon = -180; Lon < 180; Lon += 30) for (int Lat = -90; Lat < 90; Lat += 4)
            Segment(TMOPGlobe::Unit(Lat, Lon), TMOPGlobe::Unit(Lat + 4, Lon), FLinearColor(.2f, .4f, .5f, .3f), .6f, Layer + 2);
        if (bCoasts && (!bBlocs || !Appearance || Appearance->Lands.IsEmpty())) for (const auto& Curve : Data->Coastlines) for (int32 I = 1; I < Curve.Num(); ++I)
            Segment(Curve[I - 1], Curve[I], FLinearColor(.42f, .85f, .69f, .9f), 1.1f, Layer + 3);
        const FTMOPAtlasEntry* Active = Data->Find(Selected);
        const TMap<FString, FVector2D> FlagPositions = LayoutFlags(G);
        for (const FTMOPAtlasEntry& E : Data->Entries)
        {
            if (!DrawEntry(E)) continue;
            const FLinearColor Color = E.Kind == TEXT("conflict") ? ConflictInk(E) : Ink(E.Kind);
            const bool bSelected = E.Id == Selected;
            if (E.Kind == TEXT("conflict") && (bSelected || bAllConflictLinks))
            {
                for (const auto& Link : E.Links)
                {
                    const auto* A = E.Participants.FindByPredicate([&](const auto& P) { return P.Id == Link.From; });
                    const auto* B = E.Participants.FindByPredicate([&](const auto& P) { return P.Id == Link.To; });
                    if (!A || !B) continue;
                    const FLinearColor C = Link.Kind == TEXT("support") ? Ink(TEXT("actor")) : Color;
                    for (int32 J = 1; J <= 96; ++J)
                    {
                        FVector2D P, Q;
                        if (LinkPoint(*A, *B, (J - 1) / 96.0, Rotation, Center, Radius, P) && LinkPoint(*A, *B, J / 96.0, Rotation, Center, Radius, Q))
                            FSlateDrawElement::MakeLines(Out, Layer + 4, G.ToPaintGeometry(),
                                TArray<FVector2f>{FVector2f(P), FVector2f(Q)}, ESlateDrawEffect::None, C, true, bSelected ? 2.f : 1.f);
                    }
                    auto Arrow = [&](double T, double PreviousT)
                    {
                        FVector2D P, Q;
                        if (LinkPoint(*A, *B, PreviousT, Rotation, Center, Radius, P) && LinkPoint(*A, *B, T, Rotation, Center, Radius, Q))
                        {
                            const FVector2D D = (Q - P).GetSafeNormal(), N(-D.Y, D.X);
                            FSlateDrawElement::MakeLines(Out, Layer + 5, G.ToPaintGeometry(),
                                TArray<FVector2f>{FVector2f(Q - D * 7 + N * 4), FVector2f(Q), FVector2f(Q - D * 7 - N * 4)}, ESlateDrawEffect::None, C, true, 1.5f);
                        }
                    };
                    Arrow(.78, .76);
                    if (Link.Kind == TEXT("opposition")) Arrow(.22, .24);
                }
                int32 ParticipantNumber = 0;
                for (const auto& Participant : E.Participants)
                {
                    ++ParticipantNumber;
                    FVector2D P;
                    if (!Project(TMOPGlobe::Unit(Participant.Latitude, Participant.Longitude), P)) continue;
                    const FVector2D Anchor = P;
                    P += Participant.Offset;
                    FSlateDrawElement::MakeLines(Out, Layer + 5, G.ToPaintGeometry(),
                        TArray<FVector2f>{FVector2f(Anchor), FVector2f(P)}, ESlateDrawEffect::None, Ink(TEXT("actor")), true, .7f);
                    const FSlateBrush* Flag = bFlags ? Appearance->Flag(Participant.Flag) : nullptr;
                    // Active conflict endpoints remain above country markers and labels.
                    const int32 MarkerLayer = Layer + (bSelected ? 13 : 11);
                    const FVector2D MarkerSize = Flag ? FVector2D(Flag->ImageSize) : FVector2D(8, 8);
                    if (Flag) FSlateDrawElement::MakeBox(Out, MarkerLayer,
                        G.MakeChild(MarkerSize + FVector2D(4, 4), FSlateLayoutTransform(P - MarkerSize * .5 - FVector2D(2, 2))).ToPaintGeometry(),
                        White, ESlateDrawEffect::None, bSelected ? FLinearColor(1.f, .85f, .3f) : FLinearColor::Black);
                    FSlateDrawElement::MakeBox(Out, MarkerLayer + 1,
                        G.MakeChild(MarkerSize, FSlateLayoutTransform(P - MarkerSize * .5)).ToPaintGeometry(),
                        Flag ? Flag : White, ESlateDrawEffect::None, Flag ? FLinearColor::White : Ink(TEXT("actor")));
                    if (bSelected && !Flag)
                        FSlateDrawElement::MakeText(Out, MarkerLayer + 1, G.MakeChild(FVector2D(200, 20), FSlateLayoutTransform(P + FVector2D(7, 4))).ToPaintGeometry(),
                            FText::AsNumber(ParticipantNumber), FCoreStyle::GetDefaultFontStyle("Bold", 11), ESlateDrawEffect::None, Ink(TEXT("actor")));
                }
            }
            for (int32 I = 1; I < E.Route.Num(); ++I)
            {
                const FTMOPAtlasEntry* A = Data->Find(E.Route[I - 1]);
                const FTMOPAtlasEntry* B = Data->Find(E.Route[I]);
                if (!A || !B) continue;
                const FVector From = TMOPGlobe::Unit(A->Latitude, A->Longitude), To = TMOPGlobe::Unit(B->Latitude, B->Longitude);
                for (int32 J = 1; J <= 96; ++J)
                {
                    if (E.Kind == TEXT("funds") && (J / 3) % 2) continue;
                    Segment(TMOPGlobe::Arc(From, To, (J - 1) / 96.0), TMOPGlobe::Arc(From, To, J / 96.0),
                        Color, bSelected ? 3.0f : 1.3f, Layer + 4);
                }
                FVector2D P, Q;
                if (Project(TMOPGlobe::Arc(From, To, .58), P) && Project(TMOPGlobe::Arc(From, To, .60), Q))
                {
                    const FVector2D D = (Q - P).GetSafeNormal(), N(-D.Y, D.X);
                    FSlateDrawElement::MakeLines(Out, Layer + 5, G.ToPaintGeometry(),
                        TArray<FVector2f>{FVector2f(P - D * 7 + N * 4), FVector2f(P), FVector2f(P - D * 7 - N * 4)}, ESlateDrawEffect::None, Color, true, 1.5f);
                }
            }
            FVector2D P;
            if (!E.bMarker || !Project(TMOPGlobe::Unit(E.Latitude, E.Longitude), P)) continue;
            if (!E.MarkerOffset.IsNearlyZero())
            {
                FSlateDrawElement::MakeLines(Out, Layer + 5, G.ToPaintGeometry(),
                    TArray<FVector2f>{FVector2f(P), FVector2f(P + E.MarkerOffset)}, ESlateDrawEffect::None, Color, true, 1.0f);
                P += E.MarkerOffset;
            }
            const bool bRelated = Active && (Active->Related.Contains(E.Id) || Active->Route.Contains(E.Id));
            const double R = bSelected ? 7 : (bRelated ? 5 : 3.5);
            const FSlateBrush* Flag = CountryFlag(E);
            if (Flag)
            {
                const FVector2D* Position = FlagPositions.Find(E.Id);
                if (!Position) continue;
                if ((*Position - P).SizeSquared() > 4)
                    FSlateDrawElement::MakeLines(Out, Layer + 5, G.ToPaintGeometry(),
                        TArray<FVector2f>{FVector2f(P), FVector2f(*Position)}, ESlateDrawEffect::None, FLinearColor(.8f,.85f,.9f,.8f), true, .8f);
                P = *Position;
                const FVector2D FlagSize(Flag->ImageSize);
                const FLinearColor Frame = bSelected ? FLinearColor(1.f, .85f, .3f) :
                    (Hovered == E.Id || bRelated ? FLinearColor::White : FLinearColor(.025f, .025f, .025f));
                FSlateDrawElement::MakeBox(Out, Layer + 6,
                    G.MakeChild(FlagSize + FVector2D(4, 4), FSlateLayoutTransform(P - FlagSize * .5 - FVector2D(2, 2))).ToPaintGeometry(),
                    White, ESlateDrawEffect::None, Frame);
                FSlateDrawElement::MakeBox(Out, Layer + 7,
                    G.MakeChild(FlagSize, FSlateLayoutTransform(P - FlagSize * .5)).ToPaintGeometry(), Flag, ESlateDrawEffect::None, FLinearColor::White);
            }
            else if (E.Kind == TEXT("conflict"))
            {
                const double D = R + 4;
                FSlateDrawElement::MakeLines(Out, Layer + 6, G.ToPaintGeometry(),
                    TArray<FVector2f>{FVector2f(P + FVector2D(0, -D)), FVector2f(P + FVector2D(D, 0)),
                    FVector2f(P + FVector2D(0, D)), FVector2f(P + FVector2D(-D, 0)), FVector2f(P + FVector2D(0, -D))}, ESlateDrawEffect::None, Color, true, 1.7f);
                for (int Sign : {-1, 1})
                    FSlateDrawElement::MakeLines(Out, Layer + 6, G.ToPaintGeometry(),
                        TArray<FVector2f>{FVector2f(P + FVector2D(-3, -3 * Sign)), FVector2f(P + FVector2D(3, 3 * Sign))}, ESlateDrawEffect::None, Color, true, 1.5f);
            }
            else
            FSlateDrawElement::MakeBox(Out, Layer + 6,
                G.MakeChild(FVector2D(R * 2, R * 2), FSlateLayoutTransform(P - FVector2D(R, R))).ToPaintGeometry(),
                White, ESlateDrawEffect::None, Color);
            if (bSelected || Hovered == E.Id)
            {
                const FVector2D Label(FMath::Clamp(P.X + (Flag ? 18 : 12), 4.0, FMath::Max(4.0, Size.X - 180.0)), FMath::Clamp(P.Y - 24, 4.0, FMath::Max(4.0, Size.Y - 25.0)));
                FSlateDrawElement::MakeBox(Out, Layer + 8,
                    G.MakeChild(FVector2D(184, 24), FSlateLayoutTransform(Label - FVector2D(2, 1))).ToPaintGeometry(),
                    White, ESlateDrawEffect::None, FLinearColor(0, 0, 0, .85f));
                FSlateDrawElement::MakeText(Out, Layer + 9, G.MakeChild(FVector2D(180, 24), FSlateLayoutTransform(Label)).ToPaintGeometry(),
                    E.Text(TEXT("title")), FCoreStyle::GetDefaultFontStyle("Bold", 12), ESlateDrawEffect::None, Color);
            }
        }
        return Layer + 15;
    }
    virtual FReply OnMouseButtonDown(const FGeometry& G, const FPointerEvent& E) override
    {
        if (E.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
        Start = Previous = G.AbsoluteToLocal(E.GetScreenSpacePosition()); bMoved = false;
        return FReply::Handled().CaptureMouse(AsShared()).SetUserFocus(AsShared(), EFocusCause::Mouse);
    }
    virtual FReply OnMouseMove(const FGeometry& G, const FPointerEvent& E) override
    {
        const FVector2D P = G.AbsoluteToLocal(E.GetScreenSpacePosition());
        if (HasMouseCapture())
        {
            if ((P - Start).SizeSquared() > 16) bMoved = true;
            if (bMoved) Turn(-(P.X - Previous.X) * .35 / Zoom, (P.Y - Previous.Y) * .35 / Zoom);
            Previous = P; return FReply::Handled();
        }
        Hovered = Hit(G, P);
        FText ParticipantLabel;
        HitParticipant(G, P, &ParticipantLabel);
        SetToolTipText(ParticipantLabel);
        Invalidate(EInvalidateWidgetReason::Paint);
        return FReply::Unhandled();
    }
    virtual FReply OnMouseButtonUp(const FGeometry& G, const FPointerEvent& E) override
    {
        if (E.GetEffectingButton() != EKeys::LeftMouseButton || !HasMouseCapture()) return FReply::Unhandled();
        const FString Id = bMoved ? FString() : Hit(G, G.AbsoluteToLocal(E.GetScreenSpacePosition()));
        if (!Id.IsEmpty()) OnSelect(Id);
        return FReply::Handled().ReleaseMouseCapture();
    }
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& E) override
    { bMoved = false; SLeafWidget::OnMouseCaptureLost(E); }
    virtual void OnMouseLeave(const FPointerEvent& E) override
    { Hovered.Reset(); SetToolTipText(FText::GetEmpty()); SLeafWidget::OnMouseLeave(E); }
    virtual FReply OnMouseWheel(const FGeometry&, const FPointerEvent& E) override
    { ZoomBy(E.GetWheelDelta() * .08); return FReply::Handled(); }
    virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent& E) override
    {
        const FKey K = E.GetKey();
        if (K == EKeys::Left || K == EKeys::Gamepad_DPad_Left) Turn(-12, 0);
        else if (K == EKeys::Right || K == EKeys::Gamepad_DPad_Right) Turn(12, 0);
        else if (K == EKeys::Up || K == EKeys::Gamepad_DPad_Up) Turn(0, 12);
        else if (K == EKeys::Down || K == EKeys::Gamepad_DPad_Down) Turn(0, -12);
        else if (K == EKeys::Add || K == EKeys::Gamepad_RightShoulder) ZoomBy(.08);
        else if (K == EKeys::Subtract || K == EKeys::Gamepad_LeftShoulder) ZoomBy(-.08);
        else return FReply::Unhandled();
        return FReply::Handled();
    }
private:
    const FSlateBrush* CountryFlag(const FTMOPAtlasEntry& E) const
    { return bFlags && Appearance && E.Kind == TEXT("country") ? Appearance->Flag(E.Id) : nullptr; }

    TMap<FString, FVector2D> LayoutFlags(const FGeometry& G) const
    {
        TMap<FString, FVector2D> Positions;
        if (!bFlags) return Positions;
        const FVector2D Size = G.GetLocalSize(), Center = Size * .5;
        const double Radius = FMath::Min(Size.X, Size.Y) * (100.0 / 240.0) * Zoom;
        const FQuat Rotation = TMOPGlobe::View(Latitude, Longitude);
        TArray<TPair<FVector2D, FVector2D>> Occupied;
        // Selection has priority; stable source order after that. Hover never
        // moves a flag, so entering its hit rectangle cannot make it flicker.
        for (int32 Pass = 0; Pass < 2; ++Pass) for (const auto& E : Data->Entries)
        {
            if ((E.Id == Selected) != (Pass == 0) || !E.bMarker || !DrawEntry(E)) continue;
            const FSlateBrush* Flag = CountryFlag(E);
            FVector2D Anchor;
            if (!Flag || !TMOPGlobe::Project(TMOPGlobe::Unit(E.Latitude, E.Longitude), Rotation, Center, Radius, Anchor)) continue;
            Anchor += E.MarkerOffset;
            if (Anchor.X < 0 || Anchor.Y < 0 || Anchor.X > Size.X || Anchor.Y > Size.Y) continue;
            const FVector2D Half = FVector2D(Flag->ImageSize) * .5 + FVector2D(3, 3);
            auto Fit = [&](FVector2D P)
            {
                P.X = FMath::Clamp(P.X, Half.X, FMath::Max(Half.X, Size.X - Half.X));
                P.Y = FMath::Clamp(P.Y, Half.Y, FMath::Max(Half.Y, Size.Y - Half.Y));
                return P;
            };
            auto Free = [&](const FVector2D& P)
            {
                for (const auto& Other : Occupied)
                    if (FMath::Abs(P.X - Other.Key.X) < Half.X + Other.Value.X &&
                        FMath::Abs(P.Y - Other.Key.Y) < Half.Y + Other.Value.Y) return false;
                return true;
            };
            FVector2D P = Fit(Anchor);
            bool bFound = Free(P);
            for (int32 Ring = 1; Ring <= 3 && !bFound; ++Ring) for (int32 Step = 0; Step < 12; ++Step)
            {
                const double Angle = 2.0 * PI * Step / 12;
                const FVector2D Candidate = Fit(Anchor + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * (18.0 * Ring));
                if (Free(Candidate)) { P = Candidate; bFound = true; break; }
            }
            Positions.Add(E.Id, P); Occupied.Emplace(P, Half);
        }
        return Positions;
    }

    void PaintLand(const FGeometry& G, FSlateWindowElementList& Out, int32 Layer,
        const FQuat& Rotation, const FVector2D& Center, double Radius) const
    {
        const FSlateBrush* White = FCoreStyle::Get().GetBrush("WhiteBrush");
        const FSlateResourceHandle Resource = FSlateApplication::Get().GetRenderer()->GetResourceHandle(*White);
        TArray<FSlateVertex> Vertices;
        TArray<SlateIndex> Indices;
        Vertices.Reserve(60000); Indices.Reserve(90000);
        auto Flush = [&]
        {
            if (!Indices.IsEmpty()) FSlateDrawElement::MakeCustomVerts(Out, Layer, Resource, Vertices, Indices, nullptr, 0, 0, ESlateDrawEffect::None);
            Vertices.Reset(); Indices.Reset();
        };
        auto XY = [&](const FVector& P) { return Center + FVector2D(-P.Y, -P.Z) * Radius; };
        // A calm ocean background makes the political map readable even when
        // the user's sphere still has Unreal's default checker material.
        if (!Appearance->Lands.IsEmpty())
        {
            Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),
                FVector2f(Center), FVector2f(.5f, .5f), FLinearColor(.016f,.040f,.065f).ToFColor(true), FColor::White));
            for (int32 I = 0; I <= 192; ++I)
            {
                const double Angle = 2.0 * PI * I / 192;
                const FVector2D P = Center + FVector2D(FMath::Cos(Angle), FMath::Sin(Angle)) * Radius;
                Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),
                    FVector2f(P), FVector2f(.5f,.5f), FLinearColor(.004f,.010f,.020f).ToFColor(true), FColor::White));
                if (I) { Indices.Add(0); Indices.Add(SlateIndex(I)); Indices.Add(SlateIndex(I+1)); }
            }
            Flush();
        }
        for (const auto& Land : Appearance->Lands)
        {
            TArray<FVector> Rotated;
            Rotated.Reserve(Land.Vertices.Num());
            for (const FVector& P : Land.Vertices) Rotated.Add(Rotation.RotateVector(P));
            const FLinearColor Color = FTMOPAtlasAppearance::BlocColor(Land.Bloc);
            for (int32 I = 0; I < Land.Indices.Num(); I += 3)
            {
                FVector Clipped[4];
                const int32 Count = TMOPGlobe::ClipFrontTriangle(Rotated[Land.Indices[I]], Rotated[Land.Indices[I+1]], Rotated[Land.Indices[I+2]], Clipped);
                if (Count < 3) continue;
                if (Vertices.Num() + Count > 60000) Flush(); // safe for 16-bit SlateIndex
                const int32 Base = Vertices.Num();
                for (int32 J = 0; J < Count; ++J)
                {
                    // Opaque fill avoids alpha seams; shading preserves globe depth.
                    FLinearColor Shaded = Color * float(.58 + .42 * FMath::Clamp(Clipped[J].X, 0.0, 1.0));
                    Shaded.A = 1;
                    Vertices.Add(FSlateVertex::Make<ESlateVertexRounding::Disabled>(G.GetAccumulatedRenderTransform(),
                        FVector2f(XY(Clipped[J])), FVector2f(.5f, .5f), Shaded.ToFColor(true), FColor::White));
                }
                for (int32 J = 1; J + 1 < Count; ++J)
                { Indices.Add(SlateIndex(Base)); Indices.Add(SlateIndex(Base + J)); Indices.Add(SlateIndex(Base + J + 1)); }
            }
        }
        Flush();
        for (const auto& Land : Appearance->Lands)
        {
            const bool bSelected = Land.Id == Selected;
            for (const auto& Border : Land.Borders)
            {
                TArray<FVector2f> Line;
                auto DrawLine = [&]
                {
                    if (Line.Num() > 1) FSlateDrawElement::MakeLines(Out, Layer + 1, G.ToPaintGeometry(), Line,
                        ESlateDrawEffect::None, bSelected ? FLinearColor(1.f, .9f, .55f) : FLinearColor(.6f, .7f, .75f, .65f), true, bSelected ? 1.5f : .7f);
                    Line.Reset();
                };
                for (int32 I = 1; I < Border.Num(); ++I)
                {
                    FVector A = Rotation.RotateVector(Border[I-1]), B = Rotation.RotateVector(Border[I]);
                    if (!TMOPGlobe::ClipFrontSegment(A, B)) { DrawLine(); continue; }
                    const FVector2f P(XY(A)), Q(XY(B));
                    if (!Line.IsEmpty() && !Line.Last().Equals(P, .05f)) DrawLine();
                    if (Line.IsEmpty()) Line.Add(P);
                    Line.Add(Q);
                }
                DrawLine();
            }
        }
    }

    FString HitParticipant(const FGeometry& G, const FVector2D& Mouse, FText* Label = nullptr) const
    {
        const FVector2D Center = G.GetLocalSize() * .5;
        const double Radius = FMath::Min(G.GetLocalSize().X, G.GetLocalSize().Y) * (100.0 / 240.0) * Zoom;
        const FQuat Rotation = TMOPGlobe::View(Latitude, Longitude);
        // Reverse paint order, selected layer first. Drawing and hit bounds agree.
        for (int32 Pass = 0; Pass < 2; ++Pass)
            for (int32 I = Data->Entries.Num() - 1; I >= 0; --I)
            {
                const auto& E = Data->Entries[I];
                if (!DrawEntry(E) || E.Kind != TEXT("conflict") || (E.Id != Selected && !bAllConflictLinks) ||
                    (Pass == 0) != (E.Id == Selected)) continue;
                for (int32 J = E.Participants.Num() - 1; J >= 0; --J)
                {
                    const auto& P = E.Participants[J];
                    FVector2D XY;
                    if (!TMOPGlobe::Project(TMOPGlobe::Unit(P.Latitude, P.Longitude), Rotation, Center, Radius, XY)) continue;
                    const FSlateBrush* Flag = bFlags ? Appearance->Flag(P.Flag) : nullptr;
                    const FVector2D D = Mouse - XY - P.Offset;
                    const FVector2D Half = Flag ? FVector2D(Flag->ImageSize) * .5 + FVector2D(2, 2) : FVector2D(7, 7);
                    if (FMath::Abs(D.X) <= Half.X && FMath::Abs(D.Y) <= Half.Y)
                    { if (Label) *Label = E.Text(P.Label); return E.Id; }
                }
            }
        return FString();
    }
    FString Hit(const FGeometry& G, const FVector2D& Mouse) const
    {
        const FString Participant = HitParticipant(G, Mouse);
        if (!Participant.IsEmpty()) return Participant;
        const FVector2D Center = G.GetLocalSize() * .5;
        const double Radius = FMath::Min(G.GetLocalSize().X, G.GetLocalSize().Y) * (100.0 / 240.0) * Zoom;
        const FQuat Rotation = TMOPGlobe::View(Latitude, Longitude);
        const TMap<FString, FVector2D> FlagPositions = LayoutFlags(G);
        FString BestId, FlagId; double Best = 14 * 14, BestFlag = TNumericLimits<double>::Max();
        // Markers take precedence over routes; lists cover coincident locations.
        for (const FTMOPAtlasEntry& E : Data->Entries)
        {
            FVector2D P;
            if (E.bMarker && DrawEntry(E) && TMOPGlobe::Project(TMOPGlobe::Unit(E.Latitude, E.Longitude), Rotation, Center, Radius, P))
            {
                P += E.MarkerOffset;
                const FSlateBrush* Flag = CountryFlag(E);
                if (Flag)
                {
                    const FVector2D* Position = FlagPositions.Find(E.Id);
                    if (!Position) continue;
                    P = *Position;
                }
                const FVector2D Delta = Mouse - P;
                if (Flag)
                {
                    // Flags render above ordinary markers; hit testing must use
                    // that same order even when a conflict shares a capital.
                    if (FMath::Abs(Delta.X) <= Flag->ImageSize.X * .5 + 3 && FMath::Abs(Delta.Y) <= Flag->ImageSize.Y * .5 + 3 && Delta.SizeSquared() < BestFlag)
                    { BestFlag = Delta.SizeSquared(); FlagId = E.Id; }
                    continue;
                }
                const double D = Delta.SizeSquared();
                if (D <= Best) { Best = D; BestId = E.Id; }
            }
        }
        if (!FlagId.IsEmpty()) return FlagId;
        if (!BestId.IsEmpty()) return BestId;
        Best = 7 * 7;
        for (const FTMOPAtlasEntry& E : Data->Entries) if (DrawEntry(E)) for (int32 I = 1; I < E.Route.Num(); ++I)
        {
            const auto* A = Data->Find(E.Route[I - 1]); const auto* B = Data->Find(E.Route[I]);
            if (!A || !B) continue;
            const FVector From = TMOPGlobe::Unit(A->Latitude, A->Longitude), To = TMOPGlobe::Unit(B->Latitude, B->Longitude);
            for (int32 J = 1; J <= 96; ++J)
            {
                if (E.Kind == TEXT("funds") && (J / 3) % 2) continue;
                FVector2D P, Q;
                if (!TMOPGlobe::Project(TMOPGlobe::Arc(From, To, (J - 1) / 96.0), Rotation, Center, Radius, P) ||
                    !TMOPGlobe::Project(TMOPGlobe::Arc(From, To, J / 96.0), Rotation, Center, Radius, Q)) continue;
                const FVector2D D = Q - P;
                const double T = FMath::Clamp(FVector2D::DotProduct(Mouse - P, D) / FMath::Max(D.SizeSquared(), .001), 0.0, 1.0);
                const double Distance = (Mouse - (P + D * T)).SizeSquared();
                if (Distance < Best) { Best = Distance; BestId = E.Id; }
            }
        }
        for (const FTMOPAtlasEntry& E : Data->Entries)
            if (DrawEntry(E) && E.Kind == TEXT("conflict") && (E.Id == Selected || bAllConflictLinks))
                for (const auto& Link : E.Links)
                {
                    const auto* A = E.Participants.FindByPredicate([&](const auto& P) { return P.Id == Link.From; });
                    const auto* B = E.Participants.FindByPredicate([&](const auto& P) { return P.Id == Link.To; });
                    if (!A || !B) continue;
                    for (int32 J = 1; J <= 96; ++J)
                    {
                        FVector2D P, Q;
                        if (!LinkPoint(*A, *B, (J - 1) / 96.0, Rotation, Center, Radius, P) || !LinkPoint(*A, *B, J / 96.0, Rotation, Center, Radius, Q)) continue;
                        const FVector2D D = Q - P;
                        const double T = FMath::Clamp(FVector2D::DotProduct(Mouse - P, D) / FMath::Max(D.SizeSquared(), .001), 0.0, 1.0);
                        const double Distance = (Mouse - P - D * T).SizeSquared();
                        if (Distance < Best) { Best = Distance; BestId = E.Id; }
                    }
                }
        return BestId;
    }
    TSharedPtr<FTMOPWorldAtlasData> Data;
    TSharedPtr<FTMOPAtlasAppearance> Appearance;
    TUniquePtr<FTMOPGlobeScene> Scene;
    TFunction<void(const FString&)> OnSelect;
    double Latitude = 25, Longitude = 15, Zoom = 1;
    bool bCoasts = true, bDirty = true, bMoved = false;
    FVector2D Start, Previous;
    FString Hovered;
};

class STMOPWorldAtlas final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPWorldAtlas) {} SLATE_END_ARGS()
    void Construct(const FArguments&, UStaticMesh* Mesh, UMaterialInterface* Material, FRotator Alignment, bool bCoasts)
    {
        Data = MakeShared<FTMOPWorldAtlasData>();
        if (!Data->Load())
        {
            ChildSlot[SNew(STextBlock).AutoWrapText(true).Text(FText::Format(L(NSLOCTEXT("TMOP", "AtlasLoadError", "Världsdata kunde inte läsas: {0}")), FText::FromString(Data->Error)))];
            return;
        }
        Revision = FTMOPLocalization::GetRevision();
        Appearance = MakeShared<FTMOPAtlasAppearance>();
        Appearance->Load();
        SAssignNew(Globe, STMOPGlobe, Data.ToSharedRef(), Appearance.ToSharedRef(), Mesh, Material, Alignment, bCoasts)
            .OnSelect(TFunction<void(const FString&)>([this](const FString& Id) { Select(Id); }));
        TSharedRef<SHorizontalBox> Tabs = SNew(SHorizontalBox);
        auto Tab = [&](const FString& Kind, const FText& Name)
        {
            Tabs->AddSlot().FillWidth(1).Padding(2)[SNew(SButton).Text(FTMOPLocalization::Bind([Name] { return Name; }))
                .ButtonColorAndOpacity_Lambda([this, Kind] { return Mode == Kind ? FLinearColor(.24f, .36f, .45f) : FLinearColor(.08f, .11f, .15f); })
                .OnClicked_Lambda([this, Kind]
                {
                    Mode = Kind; Search.Reset(); SearchBox->SetText(FText::GetEmpty());
                    for (const auto& E : Data->Entries)
                    {
                        const bool bKind = Mode == TEXT("flow") ? E.Kind == TEXT("arms") || E.Kind == TEXT("funds") : Mode == TEXT("group") ? (E.Kind == TEXT("group") || E.Kind == TEXT("actor") || E.Kind == TEXT("event")) : Mode == TEXT("country") ? (E.Kind == TEXT("country") || E.Kind == TEXT("region")) : E.Kind == Mode;
                        if (bKind && E.Visible(Globe->bLater, Globe->bNearby)) { Select(E.Id); break; }
                    }
                    RebuildList(); return FReply::Handled();
                })];
        };
        Tab(TEXT("country"), NSLOCTEXT("TMOP", "AtlasCountries", "Länder"));
        Tab(TEXT("group"), NSLOCTEXT("TMOP", "AtlasGroups", "Grupper och aktörer"));
        Tab(TEXT("conflict"), NSLOCTEXT("TMOP", "AtlasConflicts", "Konflikter"));
        Tab(TEXT("flow"), NSLOCTEXT("TMOP", "AtlasFlows", "Vapen och finansiering"));
        TSharedRef<SWrapBox> Layers = SNew(SWrapBox).UseAllottedSize(true);
        auto Check = [&](const FText& Label, bool* Flag)
        {
            Layers->AddSlot().Padding(6, 4)[SNew(SCheckBox)
                .IsChecked_Lambda([Flag] { return *Flag ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this, Flag](ECheckBoxState S) { *Flag = S == ECheckBoxState::Checked; Globe->Invalidate(EInvalidateWidgetReason::Paint); })
                [SNew(STextBlock).Text(FTMOPLocalization::Bind([Label] { return Label; }))]];
        };
        Check(NSLOCTEXT("TMOP", "AtlasCountries", "Länder"), &Globe->bCountries);
        Check(NSLOCTEXT("TMOP", "AtlasActors", "Aktörspunkter"), &Globe->bActors);
        Check(NSLOCTEXT("TMOP", "AtlasConflicts", "Konflikter"), &Globe->bConflicts);
        Check(NSLOCTEXT("TMOP", "AtlasArms", "Vapen"), &Globe->bArms);
        Check(NSLOCTEXT("TMOP", "AtlasFunds", "Pengar"), &Globe->bFunds);
        Check(NSLOCTEXT("TMOP", "AtlasAllConflictLinks", "Alla konfliktpilar"), &Globe->bAllConflictLinks);
        Check(NSLOCTEXT("TMOP", "AtlasFlags", "Flaggor på globen"), &Globe->bFlags);
        Check(NSLOCTEXT("TMOP", "AtlasBlocs", "Blockfärger 1986"), &Globe->bBlocs);
        TSharedRef<SHorizontalBox> Controls = SNew(SHorizontalBox);
        auto Control = [&](const FString& Label, TFunction<void()> Action)
        {
            Controls->AddSlot().FillWidth(1).Padding(2)[SNew(SButton).Text(FText::FromString(Label))
                .OnClicked_Lambda([Action] { Action(); return FReply::Handled(); })];
        };
        Control(TEXT("←"), [this] { Globe->Turn(-15, 0); }); Control(TEXT("→"), [this] { Globe->Turn(15, 0); });
        Control(TEXT("↑"), [this] { Globe->Turn(0, 15); }); Control(TEXT("↓"), [this] { Globe->Turn(0, -15); });
        Control(TEXT("−"), [this] { Globe->ZoomBy(-.1); }); Control(TEXT("+"), [this] { Globe->ZoomBy(.1); });
        ChildSlot[SNew(SBorder).Padding(10)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0.f, 0.f, 0.f, .88f))
            [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[Tabs]
            + SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(STextBlock).Text(FTMOPLocalization::Bind([this]
                { return Globe->bLater ? NSLOCTEXT("TMOP", "AtlasLaterDate", "28 februari 1986 • även senare händelser 1986") : NSLOCTEXT("TMOP", "AtlasDate", "28 februari 1986"); }))]
            + SVerticalBox::Slot().AutoHeight()[Layers]
            + SVerticalBox::Slot().AutoHeight().Padding(6, 2)[SNew(STextBlock).AutoWrapText(true)
                .Visibility_Lambda([this] { return Globe->bBlocs ? EVisibility::Visible : EVisibility::Collapsed; })
                .Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "AtlasBlocLegend", "Blått: västanknutet • rött: östanknutet • grått: neutralt/alliansfritt/övrigt. Förenklad indelning och generaliserade gränser för 28 feb 1986."); }))]
            + SVerticalBox::Slot().AutoHeight().Padding(6, 2)[SNew(STextBlock).AutoWrapText(true)
                .Visibility(Appearance->Error.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
                .ColorAndOpacity(FLinearColor(1.f, .75f, .25f))
                .Text(L(NSLOCTEXT("TMOP", "AtlasAppearanceWarning", "Flaggor eller blockfärger kunde inte läsas. Kartans övriga innehåll är tillgängligt.")))]
            + SVerticalBox::Slot().AutoHeight().Padding(6)[SNew(SCheckBox)
                .IsChecked_Lambda([this] { return Globe->bLater ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState S)
                {
                    Globe->bLater = S == ECheckBoxState::Checked;
                    const auto* E = Data->Find(Globe->Selected);
                    if (E && !E->Visible(Globe->bLater, Globe->bNearby)) { Globe->Selected.Reset(); RebuildDetails(); }
                    RebuildList(); Globe->Invalidate(EInvalidateWidgetReason::Paint);
                })[SNew(STextBlock).Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "AtlasLater", "Visa även senare 1986 (efter morddatumet)"); }))]]
            + SVerticalBox::Slot().AutoHeight().Padding(6)[SNew(SCheckBox)
                .IsChecked_Lambda([this] { return Globe->bNearby ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState State)
                {
                    Globe->bNearby = State == ECheckBoxState::Checked;
                    const auto* E = Data->Find(Globe->Selected);
                    if (E && !E->Visible(Globe->bLater, Globe->bNearby)) { Globe->Selected.Reset(); RebuildDetails(); }
                    RebuildList(); Globe->Invalidate(EInvalidateWidgetReason::Paint);
                })[SNew(STextBlock).AutoWrapText(true).Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "AtlasNearby", "Konflikter nära i tid: ±90 dagar (30 nov 1985–29 maj 1986)"); }))]]
            + SVerticalBox::Slot().FillHeight(1)[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(.20f).Padding(3)[SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[SAssignNew(SearchBox, SSearchBox)
                        .HintText(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "AtlasSearch", "Sök i listan"); }))
                        .OnTextChanged_Lambda([this](const FText& T) { Search = T.ToString(); RebuildList(); })]
                    + SVerticalBox::Slot().FillHeight(1)[SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(List, SVerticalBox)]]]
                + SHorizontalBox::Slot().FillWidth(.40f).Padding(3)[SNew(SVerticalBox)
                    + SVerticalBox::Slot().FillHeight(1)[Globe.ToSharedRef()]
                    + SVerticalBox::Slot().AutoHeight()[Controls]
                    + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).AutoWrapText(true).Text(FTMOPLocalization::Bind([]
                        { return NSLOCTEXT("TMOP", "AtlasControls", "Dra för att rotera • hjul för zoom • klicka på punkt eller linje. Pilar och knappar fungerar också."); }))]]
                + SHorizontalBox::Slot().FillWidth(.40f).Padding(6)[SAssignNew(DetailHost, SBox)]]
            + SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true).Text(FTMOPLocalization::Bind([]
                { return NSLOCTEXT("TMOP", "AtlasLegend", "Kryssad romb: konflikt • röd: pågående period • orange: avslutad • lila: börjar senare. Dubbelpil: motsättning • blå enkelpil: stöd • röd enkelpil: våld mot civila. Gult: vapen • streckat grönt: pengar. Punkter och pilar är schematiska, inte fronter eller färdvägar. Pågående period betyder inte strid varje dag. Historisk kontext belägger inte delaktighet i mordet."); }))]]];
        RebuildList(); Select(TEXT("il"));
    }
    virtual void Tick(const FGeometry& G, double Time, float Delta) override
    {
        SCompoundWidget::Tick(G, Time, Delta);
        const uint32 NewRevision = FTMOPLocalization::GetRevision();
        if (Data && Globe && NewRevision != Revision)
        { Revision = NewRevision; RebuildList(); RebuildDetails(); }
    }
private:
    void RebuildList()
    {
        if (!List) return;
        List->ClearChildren();
        int32 Count = 0;
        for (const FTMOPAtlasEntry& E : Data->Entries)
        {
            const bool bKind = Mode == TEXT("flow") ? E.Kind == TEXT("arms") || E.Kind == TEXT("funds") : Mode == TEXT("group") ? (E.Kind == TEXT("group") || E.Kind == TEXT("actor") || E.Kind == TEXT("event")) : Mode == TEXT("country") ? (E.Kind == TEXT("country") || E.Kind == TEXT("region")) : E.Kind == Mode;
            if (!bKind || !E.Visible(Globe->bLater, Globe->bNearby) || (!Search.IsEmpty() && !E.Text(TEXT("title")).ToString().Contains(Search))) continue;
            ++Count;
            const FString Id = E.Id;
            TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
            if (E.Kind == TEXT("country")) if (const FSlateBrush* Flag = Appearance->Flag(Id))
                Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(0, 0, 8, 0)
                    [SNew(SBox).WidthOverride(20).HeightOverride(20 * Flag->ImageSize.Y / Flag->ImageSize.X)[SNew(SImage).Image(Flag)]];
            Row->AddSlot().FillWidth(1).VAlign(VAlign_Center)
                [SNew(STextBlock).Text(E.Kind == TEXT("conflict") ? FText::FromString(E.Text(TEXT("title")).ToString() + TEXT("\n") + ConflictStatus(E).ToString()) : E.Text(TEXT("title"))).AutoWrapText(true)];
            List->AddSlot().AutoHeight().Padding(1, 3)[SNew(SButton).HAlign(HAlign_Left)
                .ButtonColorAndOpacity_Lambda([this, Id] { return Globe->Selected == Id ? FLinearColor(.24f, .36f, .45f) : FLinearColor(.08f, .11f, .15f); })
                .OnClicked_Lambda([this, Id] { Select(Id); return FReply::Handled(); })
                [Row]];
        }
        if (!Count) List->AddSlot().AutoHeight()[SNew(STextBlock).AutoWrapText(true)
            .Text(L(NSLOCTEXT("TMOP", "AtlasNoMatches", "Inga poster för detta urval och datum.")))];
    }
    void Select(const FString& Id)
    {
        const FTMOPAtlasEntry* E = Data->Find(Id);
        if (!E || !E->Visible(Globe->bLater, Globe->bNearby)) return;
        if (!Search.IsEmpty() && !E->Text(TEXT("title")).ToString().Contains(Search))
        { Search.Reset(); SearchBox->SetText(FText::GetEmpty()); }
        Mode = E->Kind == TEXT("region") ? TEXT("country") : E->Kind == TEXT("arms") || E->Kind == TEXT("funds") ? TEXT("flow") : (E->Kind == TEXT("event") || E->Kind == TEXT("actor")) ? TEXT("group") : E->Kind;
        if (E->Kind == TEXT("country") || E->Kind == TEXT("region")) Globe->bCountries = true;
        if (E->Kind == TEXT("actor")) Globe->bActors = true;
        if (E->Kind == TEXT("conflict")) Globe->bConflicts = true;
        if (E->Kind == TEXT("arms")) Globe->bArms = true;
        if (E->Kind == TEXT("funds")) Globe->bFunds = true;
        Globe->Focus(*E); RebuildList(); RebuildDetails();
    }
    void RebuildDetails()
    {
        DetailHost->SetContent(SNullWidget::NullWidget);
        const FTMOPAtlasEntry* E = Data->Find(Globe->Selected);
        if (!E) return;
        TSharedRef<SVerticalBox> Header = SNew(SVerticalBox);
        TSharedRef<SHorizontalBox> Title = SNew(SHorizontalBox);
        if (E->Kind == TEXT("country")) if (const FSlateBrush* Flag = Appearance->Flag(E->Id))
            Title->AddSlot().AutoWidth().Padding(2, 6, 8, 6).VAlign(VAlign_Center)
                [SNew(SBox).WidthOverride(32).HeightOverride(32 * Flag->ImageSize.Y / Flag->ImageSize.X)[SNew(SImage).Image(Flag)]];
        Title->AddSlot().FillWidth(1).VAlign(VAlign_Center)[SNew(STextBlock).AutoWrapText(true)
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18)).Text(E->Text(TEXT("title")))];
        Header->AddSlot().AutoHeight().Padding(4)[Title];
        TSharedRef<SScrollBox> EventScroll = SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(Details, SVerticalBox)];
        TSharedRef<SWidget> Events = SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(0, 0, 0, .88f)).Padding(6)[EventScroll];
        if (!E->Hierarchy.IsEmpty())
        {
            Header->AddSlot().FillHeight(1)[SNew(SSplitter).Orientation(Orient_Vertical)
                + SSplitter::Slot().Value(.55f)[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                    .BorderBackgroundColor(FLinearColor(0, 0, 0, .88f)).Padding(6)[MakeTMOPAtlasHierarchy(*E)]]
                + SSplitter::Slot().Value(.45f)[Events]];
        }
        else Header->AddSlot().FillHeight(1)[Events];
        DetailHost->SetContent(Header);
        auto Paragraph = [&](const FText& Text, bool bHeading = false)
        {
            if (Text.IsEmpty()) return;
            Details->AddSlot().AutoHeight().Padding(2, bHeading ? 9 : 4)[SNew(STextBlock).AutoWrapText(true).Text(Text)
                .Font(FCoreStyle::GetDefaultFontStyle(bHeading ? "Bold" : "Regular", bHeading ? 16 : 13))];
        };
        if (!E->Hierarchy.IsEmpty()) Paragraph(L(NSLOCTEXT("TMOP", "AtlasEventsPane", "Händelser kring mordet och sammanhang")), true);
        Paragraph(E->Text(TEXT("period")));
        if (E->Kind == TEXT("conflict"))
        {
            Paragraph(ConflictStatus(*E));
            Paragraph(E->Text(TEXT("conflict_type")));
            int32 Number = 0;
            for (const auto& Participant : E->Participants)
            {
                ++Number;
                const FSlateBrush* Flag = Appearance->Flag(Participant.Flag);
                TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox);
                if (Flag) Row->AddSlot().AutoWidth().VAlign(VAlign_Center).Padding(2, 2, 8, 2)
                    [SNew(SBox).WidthOverride(24).HeightOverride(24 * Flag->ImageSize.Y / Flag->ImageSize.X)[SNew(SImage).Image(Flag)]];
                Row->AddSlot().FillWidth(1)[SNew(STextBlock).AutoWrapText(true)
                    .Text(Flag ? E->Text(Participant.Label) : FText::FromString(FString::FromInt(Number) + TEXT(". ") + E->Text(Participant.Label).ToString()))];
                Details->AddSlot().AutoHeight().Padding(2, 4)[Row];
            }
            for (const auto& Link : E->Links)
            {
                const auto* A = E->Participants.FindByPredicate([&](const auto& P) { return P.Id == Link.From; });
                const auto* B = E->Participants.FindByPredicate([&](const auto& P) { return P.Id == Link.To; });
                const FText Relation = Link.Kind == TEXT("support") ? L(NSLOCTEXT("TMOP", "AtlasSupportLink", "stöd")) :
                    Link.Kind == TEXT("violence") ? L(NSLOCTEXT("TMOP", "AtlasViolenceLink", "våld mot civila")) : L(NSLOCTEXT("TMOP", "AtlasOppositionLink", "motsättning"));
                if (A && B) Paragraph(FText::FromString(E->Text(A->Label).ToString() +
                    (Link.Kind == TEXT("opposition") ? TEXT(" ↔ ") : TEXT(" → ")) + E->Text(B->Label).ToString() + TEXT(" (") + Relation.ToString() + TEXT(")")));
            }
        }
        if (E->Kind != TEXT("conflict") && (E->From > TEXT("1986-02-28") || E->bLaterOnly)) Paragraph(L(NSLOCTEXT("TMOP", "AtlasAfterDate", "Innehåller senare historik • visas endast med senare 1986 aktiverat")));
        auto Field = [&](const TCHAR* Name, const FText& Label)
        { const FText Body = E->Text(Name); if (!Body.IsEmpty()) { Paragraph(L(Label), true); Paragraph(Body); } };
        Field(TEXT("reactions"), NSLOCTEXT("TMOP", "AtlasReactions", "Reaktioner på Palmes död • efter mordet"));
        Field(TEXT("news_day"), NSLOCTEXT("TMOP", "AtlasNewsDay", "Nyheter den 28 februari 1986"));
        Field(TEXT("news_after"), NSLOCTEXT("TMOP", "AtlasNewsAfter", "Uppföljning efter den 28 februari"));
        if (E->Hierarchy.IsEmpty())
        {
            Field(TEXT("leaders"), NSLOCTEXT("TMOP", "AtlasLeaders", "Ledning vid denna tid"));
            Field(TEXT("government"), NSLOCTEXT("TMOP", "AtlasGovernment", "Regering och styrande parti"));
            Field(TEXT("ministers"), NSLOCTEXT("TMOP", "AtlasMinisters", "Centrala ministrar (urval)"));
            Field(TEXT("intelligence"), NSLOCTEXT("TMOP", "AtlasIntelligence", "Underrättelse- och säkerhetstjänster"));
            Field(TEXT("personnel"), NSLOCTEXT("TMOP", "AtlasPersonnel", "Offentligt dokumenterade befattningshavare"));
            Field(TEXT("research"), NSLOCTEXT("TMOP", "AtlasResearch", "Avgränsning och kvarstående källkontroll"));
        }
        if (E->Kind == TEXT("country") && !Appearance->Alignment(E->Id).IsEmpty())
        {
            Paragraph(L(NSLOCTEXT("TMOP", "AtlasAlignment", "Blocktillhörighet • 28 februari 1986")), true);
            Paragraph(Appearance->Alignment(E->Id));
        }
        Field(TEXT("map_note"), NSLOCTEXT("TMOP", "AtlasMapNote", "Om kartpunkten"));
        Field(TEXT("body"), NSLOCTEXT("TMOP", "AtlasContext", "Historiskt sammanhang"));
        Field(TEXT("allies"), NSLOCTEXT("TMOP", "AtlasAllies", "Allianser och samarbete"));
        Field(TEXT("opponents"), NSLOCTEXT("TMOP", "AtlasOpponents", "Motsättningar och konflikter"));
        Field(TEXT("parties"), NSLOCTEXT("TMOP", "AtlasParties", "Aktörer"));
        Field(TEXT("legal"), NSLOCTEXT("TMOP", "AtlasLegal", "Rättslig status"));
        Field(TEXT("evidence"), NSLOCTEXT("TMOP", "AtlasEvidence", "Uppgiftsstatus"));
        Field(TEXT("investigation"), NSLOCTEXT("TMOP", "AtlasInvestigation", "Koppling till mordutredningen"));
        if (!E->Route.IsEmpty())
        {
            Paragraph(L(NSLOCTEXT("TMOP", "AtlasRoute", "Flödets riktning")), true);
            FString Chain;
            for (const FString& Id : E->Route) if (const auto* Node = Data->Find(Id))
            { if (!Chain.IsEmpty()) Chain += TEXT(" → "); Chain += Node->Text(TEXT("title")).ToString(); }
            Paragraph(FText::FromString(Chain));
        }
        if (!E->Related.IsEmpty()) Paragraph(L(NSLOCTEXT("TMOP", "AtlasRelated", "Relaterade poster")), true);
        for (const FString& Id : E->Related) if (const auto* Related = Data->Find(Id))
        {
            if (!Related->Visible(Globe->bLater, Globe->bNearby)) continue;
            Details->AddSlot().AutoHeight().Padding(2)[SNew(SButton).OnClicked_Lambda([this, Id] { Select(Id); return FReply::Handled(); })
                [SNew(STextBlock).Text(Related->Text(TEXT("title"))).AutoWrapText(true)]];
        }
        Paragraph(L(NSLOCTEXT("TMOP", "AtlasSources", "Källor • publicering anges separat från händelsedatum")), true);
        if (E->Sources.IsEmpty()) Paragraph(L(NSLOCTEXT("TMOP", "AtlasNoSources", "Forskningspost: källbelagd fördjupning återstår.")));
        for (const auto& Source : E->Sources)
        {
            if (Source.Url.IsEmpty())
            {
                Paragraph(FText::FromString(Source.Title + TEXT("\n") + Source.Document));
                continue; // Bibliographic reference to a supplied scan, not a web link.
            }
            const FString Url = Source.Url;
            Details->AddSlot().AutoHeight().Padding(2)[SNew(SButton)
                .ToolTipText(FText::FromString(Url)).OnClicked_Lambda([Url]
                { if (Url.StartsWith(TEXT("https://"))) FPlatformProcess::LaunchURL(*Url, nullptr, nullptr); return FReply::Handled(); })
                [SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(Source.Title + (Source.Published.IsEmpty() ? FString() : TEXT(" (") + Source.Published + TEXT(")"))))]];
        }
    }
    TSharedPtr<FTMOPWorldAtlasData> Data;
    TSharedPtr<FTMOPAtlasAppearance> Appearance;
    TSharedPtr<STMOPGlobe> Globe;
    TSharedPtr<SVerticalBox> List, Details;
    TSharedPtr<SBox> DetailHost;
    TSharedPtr<SSearchBox> SearchBox;
    FString Mode = TEXT("country"), Search;
    uint32 Revision = 0;
};
}

TSharedRef<SWidget> MakeTMOPWorldAtlas(UStaticMesh* Mesh, UMaterialInterface* Material,
    const FRotator& MeshAlignment, bool bCoastlineOverlay)
{
    return SNew(STMOPWorldAtlas, Mesh, Material, MeshAlignment, bCoastlineOverlay);
}
