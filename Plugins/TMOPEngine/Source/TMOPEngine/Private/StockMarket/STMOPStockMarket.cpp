#include "StockMarket/STMOPStockMarket.h"
#include "StockMarket/TMOPStockMarketData.h"
#include "Localization/TMOPLocalization.h"
#include "HAL/PlatformProcess.h"
#include "Styling/CoreStyle.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace TMOPStockMarketUI
{
FText L(const FText& Text) { return FTMOPLocalization::Text(Text); }
FText Missing() { return L(NSLOCTEXT("TMOP", "MarketMissingValue", "Saknas")); }
FText Number(double Value, int32 Decimals, bool Signed = false)
{
    FNumberFormattingOptions Options;
    Options.MinimumFractionalDigits = Options.MaximumFractionalDigits = Decimals;
    Options.AlwaysSign = Signed;
    // Suppress negative zero caused only by display rounding.
    if (FMath::Abs(Value) < 0.5 * FMath::Pow(10.0, -Decimals)) Value = 0;
    return FText::AsNumber(Value, &Options);
}
FText Level(const TOptional<double>& Value, int32 Decimals)
{
    return Value.IsSet() ? Number(Value.GetValue(), Decimals) : Missing();
}
FText Percent(const FTMOPMarketEntry& E)
{
    double Points = 0, Rate = 0;
    return E.Change(Points, Rate) ? FText::Format(FText::FromString(TEXT("{0} %")), Number(Rate, 2, true)) : Missing();
}
FLinearColor ChangeColor(const FTMOPMarketEntry& E)
{
    double Points = 0, Rate = 0;
    if (!E.Change(Points, Rate)) return FLinearColor(.9f, .73f, .4f);
    return Rate > 0 ? FLinearColor(.40f, .90f, .64f) : Rate < 0 ? FLinearColor(1.f, .48f, .48f) : FLinearColor::White;
}
FText Status(const FString& Value)
{
    if (Value == TEXT("close")) return L(NSLOCTEXT("TMOP", "MarketClose", "Slutnotering"));
    if (Value == TEXT("provisional")) return L(NSLOCTEXT("TMOP", "MarketProvisional", "Preliminär"));
    if (Value == TEXT("source_conflict")) return L(NSLOCTEXT("TMOP", "MarketConflict", "Källavvikelse"));
    if (Value == TEXT("missing")) return L(NSLOCTEXT("TMOP", "MarketMissingStatus", "Ofullständig"));
    return L(NSLOCTEXT("TMOP", "MarketDaily", "Dagsnotering"));
}
FText Kind(const FString& Value)
{
    if (Value == TEXT("sector")) return L(NSLOCTEXT("TMOP", "MarketSector", "Branschindex"));
    if (Value == TEXT("world")) return L(NSLOCTEXT("TMOP", "MarketWorldIndex", "Världsindex"));
    return L(NSLOCTEXT("TMOP", "MarketMarketIndex", "Marknadsindex"));
}
FText Region(const FString& Value)
{
    if (Value == TEXT("europe")) return L(NSLOCTEXT("TMOP", "MarketEurope", "Europa"));
    if (Value == TEXT("north_america")) return L(NSLOCTEXT("TMOP", "MarketNorthAmerica", "Nordamerika"));
    if (Value == TEXT("asia_pacific")) return L(NSLOCTEXT("TMOP", "MarketAsiaPacific", "Asien och Oceanien"));
    if (Value == TEXT("africa")) return L(NSLOCTEXT("TMOP", "MarketAfrica", "Afrika"));
    if (Value == TEXT("world")) return L(NSLOCTEXT("TMOP", "MarketWorld", "Världen"));
    return L(NSLOCTEXT("TMOP", "MarketAllRegions", "Alla regioner"));
}
TSharedRef<STextBlock> Text(const FText& Value, bool Bold = false, int32 Size = 14, FLinearColor Color = FLinearColor::White)
{
    return SNew(STextBlock).Text(Value).AutoWrapText(true).ColorAndOpacity(Color)
        .Font(FCoreStyle::GetDefaultFontStyle(Bold ? "Bold" : "Regular", Size));
}
TSharedRef<SWidget> Metric(const FText& Label, const FText& Value, float Width, FLinearColor Color = FLinearColor::White)
{
    return SNew(SBox).WidthOverride(Width)[SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()[Text(Label, false, 11, FLinearColor(.70f, .76f, .83f))]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 3)[Text(Value, true, 15, Color)]];
}

class STMOPStockMarket final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPStockMarket) {} SLATE_END_ARGS()
    void Construct(const FArguments&)
    {
        if (!Data.Load())
        {
            ChildSlot[Text(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MarketLoadError", "Börsdata kunde inte läsas: {0}"), FText::FromString(Data.Error)))];
            return;
        }
        Revision = FTMOPLocalization::GetRevision();
        Regions = {TEXT("all"), TEXT("europe"), TEXT("north_america"), TEXT("asia_pacific"), TEXT("africa"), TEXT("world")};
        TSharedRef<SWrapBox> Controls = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(8, 6));
        Controls->AddSlot()[SNew(SBox).WidthOverride(250)[SNew(SSearchBox)
            .HintText(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "MarketSearch", "Sök land eller index …"); }))
            .OnTextChanged_Lambda([this](const FText& Value) { Search = Value.ToString().TrimStartAndEnd(); RebuildList(); })]];
        Controls->AddSlot()[SNew(SButton)
            .Text_Lambda([this] { return Region(Regions[RegionIndex]); })
            .ToolTipText(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "MarketCycleRegion", "Växla region"); }))
            .OnClicked_Lambda([this] { RegionIndex = (RegionIndex + 1) % Regions.Num(); RebuildList(); return FReply::Handled(); })];
        Controls->AddSlot()[SNew(SButton).Text_Lambda([this]
            {
                if (SortMode == 1) return L(NSLOCTEXT("TMOP", "MarketSortUp", "Störst uppgång"));
                if (SortMode == 2) return L(NSLOCTEXT("TMOP", "MarketSortDown", "Störst nedgång"));
                return L(NSLOCTEXT("TMOP", "MarketSortName", "Land och index"));
            })
            .ToolTipText(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "MarketCycleSort", "Växla sortering"); }))
            .OnClicked_Lambda([this] { SortMode = (SortMode + 1) % 3; RebuildList(); return FReply::Handled(); })];
        Controls->AddSlot()[SNew(SCheckBox).IsChecked_Lambda([this] { return bWithValues ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bWithValues = State == ECheckBoxState::Checked; RebuildList(); })
            [SNew(STextBlock).Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "MarketWithValues", "Med båda värdena"); })).ColorAndOpacity(FLinearColor::White)]];
        Controls->AddSlot()[SNew(SCheckBox).IsChecked_Lambda([this] { return bHideSectors ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State) { bHideSectors = State == ECheckBoxState::Checked; RebuildList(); })
            [SNew(STextBlock).Text(FTMOPLocalization::Bind([] { return NSLOCTEXT("TMOP", "MarketHideSectors", "Dölj branschindex"); })).ColorAndOpacity(FLinearColor::White)]];
        ChildSlot[SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(.012f, .018f, .027f, .94f)).Padding(12)
            [SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
                [SNew(STextBlock).AutoWrapText(true).ColorAndOpacity(FLinearColor::White).Text_Lambda([this]
                    { return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MarketPeriod", "Före: {0}   Efter: {1}   •   Historisk jämförelse"), FText::FromString(Data.BeforeDate), FText::FromString(Data.AfterDate)); })]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)[Controls]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 6)
                [SNew(STextBlock).ColorAndOpacity(FLinearColor(.70f, .76f, .83f)).AutoWrapText(true).Text_Lambda([this]
                    { return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MarketCount", "Visar {0} av {1} index. Välj en rad för källor och anmärkningar."), FText::AsNumber(Visible.Num()), FText::AsNumber(Data.Entries.Num())); })]
                + SVerticalBox::Slot().FillHeight(1)
                [SNew(SSplitter).Orientation(Orient_Vertical)
                    + SSplitter::Slot().Value(.60f)[SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(List, SVerticalBox)]]
                    + SSplitter::Slot().Value(.40f)[SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(Details, SVerticalBox)]]]
            ]];
        RebuildList();
    }
    virtual void Tick(const FGeometry& Geometry, double Time, float Delta) override
    {
        SCompoundWidget::Tick(Geometry, Time, Delta);
        const uint32 Current = FTMOPLocalization::GetRevision();
        if (List && Current != Revision) { Revision = Current; RebuildList(); }
    }
private:
    void RebuildList()
    {
        if (!List) return;
        Visible.Reset();
        for (const auto& E : Data.Entries)
        {
            double Points = 0, Rate = 0;
            if (RegionIndex != 0 && E->Region != Regions[RegionIndex]) continue;
            if (bHideSectors && E->Kind == TEXT("sector")) continue;
            if (bWithValues && !E->Change(Points, Rate)) continue;
            const FString Name = E->Market.Resolve(E->Id, TEXT("market")).ToString() + TEXT(" ") + E->Name.Resolve(E->Id, TEXT("name")).ToString();
            if (!Search.IsEmpty() && !Name.Contains(Search, ESearchCase::IgnoreCase)) continue;
            Visible.Add(E);
        }
        Visible.Sort([this](const TSharedPtr<FTMOPMarketEntry>& A, const TSharedPtr<FTMOPMarketEntry>& B)
        {
            if (SortMode != 0)
            {
                double AP = 0, AR = 0, BP = 0, BR = 0;
                const bool AV = A->Change(AP, AR), BV = B->Change(BP, BR);
                if (AV != BV) return AV; // Unknown results go last for both sort directions.
                if (AV && AR != BR) return SortMode == 1 ? AR > BR : AR < BR;
            }
            const FString AN = A->Market.Resolve(A->Id, TEXT("market")).ToString() + A->Name.Resolve(A->Id, TEXT("name")).ToString();
            const FString BN = B->Market.Resolve(B->Id, TEXT("market")).ToString() + B->Name.Resolve(B->Id, TEXT("name")).ToString();
            return AN == BN ? A->Id < B->Id : AN < BN;
        });
        if (!Visible.ContainsByPredicate([this](const auto& E) { return E->Id == Selected; }))
            Selected = Visible.IsEmpty() ? FString() : Visible[0]->Id;
        List->ClearChildren();
        if (Visible.IsEmpty()) List->AddSlot().AutoHeight().Padding(8)[Text(L(NSLOCTEXT("TMOP", "MarketNoResults", "Inga index matchar filtren.")))];
        for (const auto& E : Visible)
        {
            TSharedRef<SWrapBox> Row = SNew(SWrapBox).UseAllottedSize(true).InnerSlotPadding(FVector2D(12, 8));
            Row->AddSlot()[SNew(SBox).WidthOverride(230)[SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()[Text(E->Market.Resolve(E->Id, TEXT("market")), true, 14)]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 3)[Text(E->Name.Resolve(E->Id, TEXT("name")))]]];
            Row->AddSlot()[Metric(L(NSLOCTEXT("TMOP", "MarketBefore", "Före")), Level(E->Before, E->Decimals), 115)];
            Row->AddSlot()[Metric(L(NSLOCTEXT("TMOP", "MarketAfter", "Efter")), Level(E->After, E->Decimals), 115)];
            Row->AddSlot()[Metric(L(NSLOCTEXT("TMOP", "MarketChange", "Förändring")), Percent(*E), 115, ChangeColor(*E))];
            Row->AddSlot()[Metric(Kind(E->Kind), Status(E->Status), 145,
                E->Status == TEXT("source_conflict") || E->Status == TEXT("provisional") || E->Status == TEXT("missing") ? FLinearColor(.95f, .76f, .40f) : FLinearColor(.70f, .76f, .83f))];
            List->AddSlot().AutoHeight().Padding(0, 2)[SNew(SButton).ContentPadding(10).HAlign(HAlign_Fill)
                .ButtonColorAndOpacity_Lambda([this, E] { return E->Id == Selected ? FLinearColor(.18f, .29f, .39f) : FLinearColor(.055f, .075f, .10f); })
                .OnClicked_Lambda([this, E] { Selected = E->Id; RebuildDetails(); return FReply::Handled(); })[Row]];
        }
        RebuildDetails();
    }
    void RebuildDetails()
    {
        if (!Details) return;
        Details->ClearChildren();
        auto Body = [this](const FText& Value, bool Bold = false)
            { Details->AddSlot().AutoHeight().Padding(3, 5)[Text(Value, Bold)]; };
        const auto* Found = Visible.FindByPredicate([this](const auto& E) { return E->Id == Selected; });
        if (Found)
        {
            const auto& E = **Found;
            Body(FText::Format(FText::FromString(TEXT("{0} — {1}")), E.Market.Resolve(E.Id, TEXT("market")), E.Name.Resolve(E.Id, TEXT("name"))), true);
            double Points = 0, Rate = 0;
            if (E.Change(Points, Rate))
                Body(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MarketDetailChange", "Förändring: {0} indexpunkter ({1})."), Number(Points, E.Decimals, true), Percent(E)));
            else Body(L(NSLOCTEXT("TMOP", "MarketCannotCalculate", "Förändring kan inte beräknas. En användbar observation saknas eller basvärdet är noll.")));
            Body(FText::Format(FText::FromString(TEXT("{0} • {1}")), Kind(E.Kind), Status(E.Status)));
            const FText Notes = E.Notes.Resolve(E.Id, TEXT("notes"));
            if (!Notes.IsEmpty()) Body(Notes);
            Body(L(NSLOCTEXT("TMOP", "MarketSources", "Källor — öppnas i webbläsaren")), true);
            for (const auto& Citation : E.Citations)
            {
                const FTMOPMarketSource* Source = Data.FindSource(Citation.SourceId);
                if (!Source) continue;
                const FString URL = Source->URL;
                Details->AddSlot().AutoHeight().Padding(3, 3)[SNew(SButton).HAlign(HAlign_Left).ContentPadding(8)
                    .OnClicked_Lambda([URL] { FPlatformProcess::LaunchURL(*URL, nullptr, nullptr); return FReply::Handled(); })
                    [Text(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MarketCitation", "{0}, {1}, s. {2}"),
                        FText::FromString(Source->Title), FText::FromString(Source->Published), FText::FromString(Citation.Pages)))]];
            }
        }
        Body(L(NSLOCTEXT("TMOP", "MarketMethod", "Så läses jämförelsen")), true);
        Body(Data.Method.Resolve(TEXT("comparison"), TEXT("method")));
    }
    FTMOPStockMarketData Data;
    TArray<TSharedPtr<FTMOPMarketEntry>> Visible;
    TArray<FString> Regions;
    FString Search, Selected;
    int32 SortMode = 0, RegionIndex = 0;
    bool bWithValues = false, bHideSectors = false;
    uint32 Revision = 0;
    TSharedPtr<SVerticalBox> List, Details;
};
}

TSharedRef<SWidget> MakeTMOPStockMarket() { return SNew(TMOPStockMarketUI::STMOPStockMarket); }
