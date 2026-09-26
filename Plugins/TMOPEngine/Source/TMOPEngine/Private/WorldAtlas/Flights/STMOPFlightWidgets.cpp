#include "WorldAtlas/Flights/STMOPFlightWidgets.h"
#include "WorldAtlas/TMOPGlobeMath.h"
#include "Localization/TMOPLocalization.h"
#include "HAL/PlatformProcess.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

namespace TMOPFlightUI
{
FText L(const FText& T) { return FTMOPLocalization::Text(T); }
TSharedRef<STextBlock> Text(const FText& T, bool Heading = false)
{
    return SNew(STextBlock).Text(T).AutoWrapText(true).ColorAndOpacity(FLinearColor::White)
        .Font(FCoreStyle::GetDefaultFontStyle(Heading ? "Bold" : "Regular", Heading ? 15 : 12));
}
FText Evidence(const FString& Status)
{
    return L(Status == TEXT("confirmed") ? NSLOCTEXT("TMOP", "FlightConfirmed", "Dokumenterat genomförd") : NSLOCTEXT("TMOP", "FlightScheduled", "Enligt tidtabell • genomförande ej belagt"));
}
FText ResearchStatus(const FString& Status)
{
    if (Status == TEXT("partial")) return L(NSLOCTEXT("TMOP", "FlightResearchPartial", "Delvis inläst"));
    if (Status == TEXT("scan_found")) return L(NSLOCTEXT("TMOP", "FlightResearchScan", "Skanning hittad • granskning återstår"));
    if (Status == TEXT("catalog_found")) return L(NSLOCTEXT("TMOP", "FlightResearchCatalog", "Katalogpost hittad"));
    if (Status == TEXT("no_matching_edition")) return L(NSLOCTEXT("TMOP", "FlightResearchNoEdition", "Rätt utgåva saknas i sökta källor"));
    return L(NSLOCTEXT("TMOP", "FlightResearchTodo", "Återstår att undersöka"));
}
using FOption = TSharedPtr<FString>;
class SBrowser final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SBrowser) {} SLATE_END_ARGS()
    void Construct(const FArguments&, TSharedRef<FTMOPFlightState> InState, TFunction<void(int32)> InFocus)
    {
        State = InState; Focus = InFocus;
        Countries.Add(MakeShared<FString>()); Airlines.Add(MakeShared<FString>());
        for (const auto& C : State->Data->Countries) Countries.Add(MakeShared<FString>(C.Id));
        for (const auto& A : State->Data->Airlines) Airlines.Add(MakeShared<FString>(A.Id));
        auto CountryName = [this](const FString& Id)
        {
            if (Id.IsEmpty()) return L(NSLOCTEXT("TMOP", "FlightAllCountries", "Alla länder"));
            for (const auto& C : State->Data->Countries) if (C.Id == Id) return C.Name.Resolve(C.Id, TEXT("name"));
            return FText::FromString(Id);
        };
        auto AirlineName = [this](const FString& Id)
        {
            if (Id.IsEmpty()) return L(NSLOCTEXT("TMOP", "FlightAllAirlines", "Alla flygbolag"));
            for (const auto& A : State->Data->Airlines) if (A.Id == Id) return FText::FromString(A.Name);
            return FText::FromString(Id);
        };
        ChildSlot[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(2)[SNew(SComboBox<FOption>).OptionsSource(&Countries)
                .OnGenerateWidget_Lambda([CountryName](FOption V) { return Text(CountryName(*V)); })
                .OnSelectionChanged_Lambda([this](FOption V, ESelectInfo::Type) { if (V) { State->Country = *V; State->Refilter(); Refresh(); } })
                [SNew(STextBlock).AutoWrapText(true).Text_Lambda([this, CountryName] { return CountryName(State->Country); })]]
            + SVerticalBox::Slot().AutoHeight().Padding(2)[SNew(SComboBox<FOption>).OptionsSource(&Airlines)
                .OnGenerateWidget_Lambda([AirlineName](FOption V) { return Text(AirlineName(*V)); })
                .OnSelectionChanged_Lambda([this](FOption V, ESelectInfo::Type) { if (V) { State->Airline = *V; State->Refilter(); Refresh(); } })
                [SNew(STextBlock).AutoWrapText(true).Text_Lambda([this, AirlineName] { return AirlineName(State->Airline); })]]
            + SVerticalBox::Slot().AutoHeight().Padding(2)[SNew(SSearchBox).InitialText(FText::FromString(State->Search))
                .HintText(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "FlightSearch", "Flygnummer, flygplats eller bolag"); }))
                .OnTextChanged_Lambda([this](const FText& Value) { State->Search = Value.ToString(); State->Refilter(); Refresh(); })]
            + SVerticalBox::Slot().AutoHeight().Padding(2)[SNew(SCheckBox)
                .IsChecked_Lambda([this] { return State->bOnlyAirborne ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
                .OnCheckStateChanged_Lambda([this](ECheckBoxState V) { State->bOnlyAirborne = V == ECheckBoxState::Checked; ++State->ListRevision; Refresh(); })
                [SNew(STextBlock).AutoWrapText(true).Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "FlightOnlyAirborne", "Lista bara flyg i luften vid vald tid"); }))]]
            + SVerticalBox::Slot().AutoHeight().Padding(2)[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this]
                { return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "FlightListCount", "{0} avgångar i urvalet • {1} i luften"), FText::AsNumber(State->Filtered.Num()), FText::AsNumber(State->Active.Num())); })]
            + SVerticalBox::Slot().AutoHeight().Padding(2)[SNew(STextBlock).AutoWrapText(true)
                .Visibility_Lambda([this] { return State->Listed().IsEmpty() ? EVisibility::Visible : EVisibility::Collapsed; })
                .Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "FlightNoData", "Inga inlästa avgångar matchar. Det betyder inte att flygtrafik saknades."); }))]
            + SVerticalBox::Slot().FillHeight(1)[SAssignNew(List, SListView<TSharedPtr<int32>>).ListItemsSource(&Items).ItemHeight(62)
                .SelectionMode(ESelectionMode::Single)
                .OnGenerateRow_Lambda([this](TSharedPtr<int32> Item, const TSharedRef<STableViewBase>& Owner)
                {
                    const auto& F = State->Data->Legs[*Item];
                    return SNew(STableRow<TSharedPtr<int32>>, Owner).Padding(4)
                        [Text(FText::FromString(State->Label(*Item).ToString() + TEXT("\n") + F.DepartureUTC + TEXT("\n") + Evidence(F.Status).ToString()))];
                })
                .OnSelectionChanged_Lambda([this](TSharedPtr<int32> Item, ESelectInfo::Type How)
                    { if (Item && How != ESelectInfo::Direct) { State->Select(*Item); Focus(*Item); } })]];
        Refresh();
    }
    virtual void Tick(const FGeometry& G, double T, float D) override
    {
        SCompoundWidget::Tick(G,T,D);
        if (Revision != State->ListRevision || Language != FTMOPLocalization::GetRevision()) Refresh();
        if (Selection != State->SelectionRevision && List)
        {
            Selection = State->SelectionRevision;
            const auto* Found = Items.FindByPredicate([this](const auto& I) { return *I == State->Selected; });
            if (Found) List->SetSelection(*Found, ESelectInfo::Direct); else List->ClearSelection();
        }
    }
private:
    void Refresh()
    {
        Revision = State->ListRevision; Language = FTMOPLocalization::GetRevision();
        Items.Reset(); for (int32 I : State->Listed()) Items.Add(MakeShared<int32>(I));
        if (List) List->RequestListRefresh();
        Selection = MAX_uint32;
    }
    TSharedPtr<FTMOPFlightState> State;
    TFunction<void(int32)> Focus;
    TArray<FOption> Countries, Airlines;
    TArray<TSharedPtr<int32>> Items;
    TSharedPtr<SListView<TSharedPtr<int32>>> List;
    uint32 Revision = 0, Language = 0, Selection = MAX_uint32;
};

class SDetails final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SDetails) {} SLATE_END_ARGS()
    void Construct(const FArguments&, TSharedRef<FTMOPFlightState> InState)
    { State = InState; ChildSlot[SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(Body, SVerticalBox)]]; Rebuild(); }
    virtual void Tick(const FGeometry& G, double T, float D) override
    {
        SCompoundWidget::Tick(G,T,D);
        if (Selection != State->SelectionRevision || Filter != State->FilterRevision || Language != FTMOPLocalization::GetRevision()) Rebuild();
    }
private:
    void Rebuild()
    {
        Selection = State->SelectionRevision; Filter = State->FilterRevision; Language = FTMOPLocalization::GetRevision(); Body->ClearChildren();
        auto Add = [this](const FText& Value, bool Heading = false) { Body->AddSlot().AutoHeight().Padding(4,5)[Text(Value,Heading)]; };
        auto Source = [this, Add](const FTMOPFlightSource& S, const FString& Page)
        {
            const FString URL = S.URL;
            Body->AddSlot().AutoHeight().Padding(4)[SNew(SButton).ContentPadding(6)
                .OnClicked_Lambda([URL] { FPlatformProcess::LaunchURL(*URL,nullptr,nullptr); return FReply::Handled(); })
                [Text(FText::FromString(S.Title + (Page.IsEmpty() ? FString() : TEXT(" • ") + Page)))]];
            Add(S.Notes.Resolve(S.Id,TEXT("notes")));
        };
        if (!State->Data->Error.IsEmpty()) Add(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "FlightLoadError", "Flygdata kunde inte läsas: {0}"), FText::FromString(State->Data->Error)));
        if (State->Data->Legs.IsValidIndex(State->Selected))
        {
            const auto& F = State->Data->Legs[State->Selected];
            const auto& A = State->Data->Airports[F.Origin]; const auto& B = State->Data->Airports[F.Destination];
            Add(State->Label(State->Selected), true); Add(Evidence(F.Status));
            Add(FText::FromString(A.Name + TEXT(" → ") + B.Name));
            Add(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "FlightLocalTimes", "Lokala tider (UTC-offset visas):\nAvgång: {0}\nAnkomst: {1}"), FText::FromString(F.DepartureLocal),FText::FromString(F.ArrivalLocal)));
            Add(FText::FromString(TEXT("UTC: ") + F.DepartureUTC + TEXT(" → ") + F.ArrivalUTC));
            Add(F.Notes.Resolve(F.ScheduleId,TEXT("notes")));
            Body->AddSlot().AutoHeight().Padding(4)[SNew(SButton)
                .Text(L(NSLOCTEXT("TMOP", "FlightJump", "Visa vid avgång")))
                .OnClicked_Lambda([this] { State->bPlaying = false; if (State->Data->Legs.IsValidIndex(State->Selected)) State->SetTime(State->Data->Legs[State->Selected].Departure + 1); return FReply::Handled(); })];
            Source(State->Data->Sources[F.Source], F.Page);
        }
        Add(L(NSLOCTEXT("TMOP", "FlightCoverage", "Insamling land för land • ofullständigt underlag")), true);
        Add(State->Data->Method.Resolve(TEXT("method"), TEXT("text")));
        Add(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "FlightCoverageCount", "Register: {0} länder, {1} flygbolag, {2} källposter. Inläst: {3} avgångar."),
            FText::AsNumber(State->Data->Countries.Num()),FText::AsNumber(State->Data->Airlines.Num()),FText::AsNumber(State->Data->Sources.Num()),FText::AsNumber(State->Data->Legs.Num())));
        Add(L(NSLOCTEXT("TMOP", "FlightCountryFilter", "Landfiltret omfattar avgång, ankomst eller flygbolagets hemland. Registret nedan grupperar bolagen efter hemland.")));
        for (const auto& Country : State->Data->Countries)
        {
            if (!State->Country.IsEmpty() && Country.Id != State->Country) continue;
            bool bHeader = false;
            for (const auto& Airline : State->Data->Airlines)
            {
                if (!Airline.Countries.Contains(Country.Id) || (!State->Airline.IsEmpty() && Airline.Id != State->Airline)) continue;
                if (!bHeader) { Add(Country.Name.Resolve(Country.Id,TEXT("name")),true); bHeader = true; }
                Add(FText::FromString(Airline.Name + TEXT(" — ") + ResearchStatus(Airline.Status).ToString()),true);
                for (const FString& Id : Airline.Sources)
                    for (const auto& S : State->Data->Sources) if (S.Id == Id) Source(S,FString());
            }
        }
    }
    TSharedPtr<FTMOPFlightState> State;
    TSharedPtr<SVerticalBox> Body;
    uint32 Selection = 0, Filter = 0, Language = 0;
};
}

TSharedRef<SWidget> MakeTMOPFlightBrowser(TSharedRef<FTMOPFlightState> State, TFunction<void(int32)> Focus)
{ return SNew(TMOPFlightUI::SBrowser, State, Focus); }
TSharedRef<SWidget> MakeTMOPFlightDetails(TSharedRef<FTMOPFlightState> State)
{ return SNew(TMOPFlightUI::SDetails, State); }
TSharedRef<SWidget> MakeTMOPFlightControls(TSharedRef<FTMOPFlightState> State)
{
    using namespace TMOPFlightUI;
    TSharedRef<SWrapBox> Buttons = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(4,4));
    auto Button = [&](const FText& Label, TFunction<void()> Click)
    { Buttons->AddSlot()[SNew(SButton).Text(FTMOPLocalization::Bind([Label] { return Label; }))
        .OnClicked_Lambda([Click] { Click(); return FReply::Handled(); })]; };
    Buttons->AddSlot()[SNew(SButton).Text_Lambda([State] { return L(State->bPlaying ? NSLOCTEXT("TMOP", "FlightPause", "Pausa") : NSLOCTEXT("TMOP", "FlightPlay", "Spela")); })
        .OnClicked_Lambda([State] { if (State->Seconds >= State->Data->Duration) State->SetTime(0); State->bPlaying = !State->bPlaying; return FReply::Handled(); })];
    Button(NSLOCTEXT("TMOP", "FlightStart", "−24 h"),[State] { State->bPlaying=false; State->SetTime(0); });
    Button(NSLOCTEXT("TMOP", "FlightMurder", "Mordtiden"),[State] { State->bPlaying=false; State->SetTime(State->Data->Anchor); });
    Button(NSLOCTEXT("TMOP", "FlightEnd", "+24 h"),[State] { State->bPlaying=false; State->SetTime(State->Data->Duration); });
    Buttons->AddSlot()[SNew(SButton).Text_Lambda([State] { return FText::FromString(FString::Printf(TEXT("%.0f×"), State->Speed)); })
        .ToolTipText(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "FlightSpeed", "Växla hastighet: 300×, 1800×, 7200×"); }))
        .OnClicked_Lambda([State] { State->Speed = State->Speed == 300 ? 1800 : State->Speed == 1800 ? 7200 : 300; return FReply::Handled(); })];
    Buttons->AddSlot()[SNew(SCheckBox).IsChecked_Lambda([State] { return State->bAllRoutes ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
        .OnCheckStateChanged_Lambda([State](ECheckBoxState V) { State->bAllRoutes = V == ECheckBoxState::Checked; })
        [SNew(STextBlock).Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "FlightAllRoutes", "Alla inlästa rutter i urvalet"); })).AutoWrapText(true)]];
    return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .IsEnabled_Lambda([State] { return State->Data && State->Data->Error.IsEmpty(); })
        .BorderBackgroundColor(FLinearColor(0,0,0,.92f)).Padding(6)
        .Visibility_Lambda([State] { return State->bEnabled ? EVisibility::Visible : EVisibility::Collapsed; })
        [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text_Lambda([State] { return State->Clock(); }).AutoWrapText(true)]
            + SVerticalBox::Slot().AutoHeight().Padding(0,4)[SNew(SSlider)
                .Value_Lambda([State] { return float(State->Seconds / State->Data->Duration); })
                .OnMouseCaptureBegin_Lambda([State] { State->bPlaying = false; })
                .OnValueChanged_Lambda([State](float Value) { State->bPlaying = false; State->SetTime(Value * State->Data->Duration); })]
            + SVerticalBox::Slot().AutoHeight()[Buttons]
            + SVerticalBox::Slot().AutoHeight().Padding(0,4)[SNew(STextBlock).AutoWrapText(true).Text(FTMOPLocalization::Bind([]
                { return NSLOCTEXT("TMOP", "FlightLegend", "Cyan: tidtabell • grönt: genomförande belagt • gult: valt flyg. Rörelse längs en schematisk storcirkel, inte en registrerad flygbana. Flygklockan styr bara denna karta."); }))]];
}

void PaintTMOPFlights(const FTMOPFlightState& State, const FGeometry& G, FSlateWindowElementList& Out,
    int32 Layer, const FQuat& Rotation, const FVector2D& Center, double Radius)
{
    if (!State.bEnabled || !State.Data || !State.Data->Error.IsEmpty()) return;
    const auto& D = *State.Data;
    auto Path = [&](int32 Index, FLinearColor Color, float Width)
    {
        const auto& F = D.Legs[Index]; const FVector A = D.Airports[F.Origin].Unit, B = D.Airports[F.Destination].Unit;
        TArray<FVector2f> Points;
        auto Flush = [&] { if (Points.Num()>1) FSlateDrawElement::MakeLines(Out,Layer,G.ToPaintGeometry(),Points,ESlateDrawEffect::None,Color,true,Width); Points.Reset(); };
        for (int32 J=1; J<=64; ++J)
        {
            FVector P=Rotation.RotateVector(TMOPGlobe::Arc(A,B,(J-1)/64.0));
            FVector Q=Rotation.RotateVector(TMOPGlobe::Arc(A,B,J/64.0));
            if (!TMOPGlobe::ClipFrontSegment(P,Q)) { Flush(); continue; }
            const FVector2f XY(Center + FVector2D(-P.Y,-P.Z)*Radius), ZW(Center + FVector2D(-Q.Y,-Q.Z)*Radius);
            if (!Points.IsEmpty() && !Points.Last().Equals(XY,.1f)) Flush();
            if (Points.IsEmpty()) Points.Add(XY); Points.Add(ZW);
        }
        Flush();
    };
    if (State.bAllRoutes) for (int32 I : State.Routes) Path(I,FLinearColor(.25f,.70f,.90f,.3f),.8f);
    else
    {
        TSet<FString> Drawn;
        for (int32 I : State.Active)
        {
            const auto& F = D.Legs[I]; const FString Key = FString::FromInt(F.Origin) + TEXT("/") + FString::FromInt(F.Destination);
            if (!Drawn.Contains(Key)) { Drawn.Add(Key); Path(I,FLinearColor(.25f,.70f,.90f,.5f),1.f); }
        }
    }
    if (D.Legs.IsValidIndex(State.Selected)) Path(State.Selected,FLinearColor(1,.85f,.3f),2.f);
    for (int32 I : State.Active)
    {
        const auto& F=D.Legs[I];
        const FVector A=D.Airports[F.Origin].Unit, B=D.Airports[F.Destination].Unit;
        const double T=F.Progress(State.Seconds); FVector2D P,Q;
        if (!TMOPGlobe::Project(TMOPGlobe::Arc(A,B,T),Rotation,Center,Radius,P)) continue;
        const bool Forward=T<.99;
        TMOPGlobe::Project(TMOPGlobe::Arc(A,B,FMath::Clamp(T+(Forward?.005:-.005),0.0,1.0)),Rotation,Center,Radius,Q);
        FVector2D Direction=(Forward?Q-P:P-Q).GetSafeNormal(); if (Direction.IsNearlyZero()) Direction=FVector2D(0,-1);
        const FVector2D N(-Direction.Y,Direction.X);
        const FLinearColor C=I==State.Selected?FLinearColor(1,.85f,.3f):F.Status==TEXT("confirmed")?FLinearColor(.35f,1,.55f):FLinearColor(.4f,.85f,1);
        FSlateDrawElement::MakeLines(Out,Layer+1,G.ToPaintGeometry(),TArray<FVector2f>{FVector2f(P-Direction*5+N*4),FVector2f(P+Direction*6),FVector2f(P-Direction*5-N*4),FVector2f(P-Direction*2),FVector2f(P-Direction*5+N*4)},ESlateDrawEffect::None,C,true,1.5f);
    }
}
int32 HitTMOPFlight(const FTMOPFlightState& State, const FQuat& Rotation, const FVector2D& Center, double Radius, const FVector2D& Mouse)
{
    if (!State.bEnabled || !State.Data) return INDEX_NONE;
    int32 Best=INDEX_NONE; double Distance=100;
    for (int32 I : State.Active)
    {
        const auto& F=State.Data->Legs[I]; FVector2D P;
        if (TMOPGlobe::Project(TMOPGlobe::Arc(State.Data->Airports[F.Origin].Unit,State.Data->Airports[F.Destination].Unit,F.Progress(State.Seconds)),Rotation,Center,Radius,P))
        {
            const double D=(P-Mouse).SizeSquared(); if(D<Distance){Distance=D;Best=I;}
        }
    }
    return Best;
}
