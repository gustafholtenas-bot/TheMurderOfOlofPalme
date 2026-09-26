#include "WorldAtlas/STMOPAtlasHierarchy.h"
#include "WorldAtlas/TMOPWorldAtlasData.h"
#include "Localization/TMOPLocalization.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"

namespace
{
FText L(const FText& Text) { return FTMOPLocalization::Text(Text); }

// An elbow alongside each subtree. A non-last sibling's vertical segment runs
// past its descendants; the last sibling terminates at its own card.
class STMOPAtlasBranch final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPAtlasBranch) : _Last(false), _Reporting(false) {}
        SLATE_ARGUMENT(bool, Last)
        SLATE_ARGUMENT(bool, Reporting)
        SLATE_DEFAULT_SLOT(FArguments, Content)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        bLast = Args._Last; bReporting = Args._Reporting;
        ChildSlot.Padding(18, 0, 0, 0)[Args._Content.Widget];
    }
    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G, const FSlateRect& Clip,
        FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const override
    {
        const FLinearColor Color = bReporting ? FLinearColor(.5f, .8f, 1.f) : FLinearColor(.45f, .48f, .52f);
        auto Segment = [&](const FVector2f& A, const FVector2f& B)
        { FSlateDrawElement::MakeLines(Out, Layer, G.ToPaintGeometry(), TArray<FVector2f>{A, B}, ESlateDrawEffect::None, Color, true, 1.3f); };
        const float Bottom = bLast ? 20.f : float(G.GetLocalSize().Y);
        if (bReporting)
        { Segment(FVector2f(8, 0), FVector2f(8, Bottom)); Segment(FVector2f(8, 20), FVector2f(18, 20)); }
        else
        {
            for (float Y = 0; Y < Bottom; Y += 7) Segment(FVector2f(8, Y), FVector2f(8, FMath::Min(Y + 3, Bottom)));
            Segment(FVector2f(8, 20), FVector2f(11, 20)); Segment(FVector2f(15, 20), FVector2f(18, 20));
        }
        return SCompoundWidget::OnPaint(Args, G, Clip, Out, Layer + 1, Style, bEnabled);
    }
private:
    bool bLast = false, bReporting = false;
};

class STMOPAtlasHierarchy final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPAtlasHierarchy) {} SLATE_END_ARGS()
    void Construct(const FArguments&, const FTMOPAtlasEntry* InEntry)
    {
        Entry = InEntry;
        for (const auto& N : Entry->Hierarchy)
        {
            int32 Depth = 0;
            const FTMOPAtlasOffice* Current = &N;
            while (Current && !Current->Parent.IsEmpty())
            {
                ++Depth;
                const FString Parent = Current->Parent;
                Current = Entry->Hierarchy.FindByPredicate([&Parent](const auto& Candidate) { return Candidate.Id == Parent; });
            }
            MinimumTreeWidth = FMath::Max(MinimumTreeWidth, 260.f + Depth * 26.f);
        }
        ChildSlot[SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true)
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 15))
                .Text(L(NSLOCTEXT("TMOP", "AtlasHierarchy", "Befattningar och ledning • 28 februari 1986")))]
            + SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true)
                .Text(L(NSLOCTEXT("TMOP", "AtlasTreeLegend", "Hel linje: rapporteringsväg. Streckad: gruppering, inte fastställd chefslinje. Klicka på +/− för att öppna eller stänga en gren.")))]
            + SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().Padding(3)[SNew(SButton)
                    .Text(L(NSLOCTEXT("TMOP", "AtlasExpandTree", "Öppna alla")))
                    .OnClicked_Lambda([this] { Collapsed.Reset(); Rebuild(); return FReply::Handled(); })]
                + SHorizontalBox::Slot().AutoWidth().Padding(3)[SNew(SButton)
                    .Text(L(NSLOCTEXT("TMOP", "AtlasCollapseTree", "Stäng grenar")))
                    .OnClicked_Lambda([this]
                    { for (const auto& N : Entry->Hierarchy) if (!N.Parent.IsEmpty()) Collapsed.Add(N.Id); Rebuild(); return FReply::Handled(); })]]
            + SVerticalBox::Slot().FillHeight(1)[SNew(SScrollBox).Orientation(Orient_Horizontal)
                + SScrollBox::Slot()[SNew(SBox)
                    .WidthOverride_Lambda([this] { return FOptionalSize(FMath::Max(MinimumTreeWidth, float(GetCachedGeometry().GetLocalSize().X) - 24.f)); })
                    [SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(Roots, SVerticalBox)]]]]];
        Rebuild();
    }
private:
    TSharedRef<SWidget> Node(const FTMOPAtlasOffice& Office)
    {
        TArray<const FTMOPAtlasOffice*> Children;
        for (const auto& N : Entry->Hierarchy) if (N.Parent == Office.Id) Children.Add(&N);
        const FString Id = Office.Id;
        const bool bChildren = !Children.IsEmpty();
        const FString Relation = L(Office.Relation == TEXT("reports_to") ?
            NSLOCTEXT("TMOP", "AtlasReportsTo", "Rapporteringsväg till överordnad befattning") :
            NSLOCTEXT("TMOP", "AtlasGroupedOffice", "Grupperad befattning; ingen chefslinje fastställs")).ToString();
        FString Tooltip = Relation;
        for (const FString& Source : Office.Sources) Tooltip += TEXT("\n") + Source;
        TSharedRef<SVerticalBox> Result = SNew(SVerticalBox);
        Result->AddSlot().AutoHeight().Padding(0, 3)[SNew(SBorder)
            .BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor(.12f, .18f, .23f, .96f)).Padding(1)
            [SNew(SButton).HAlign(HAlign_Fill).ContentPadding(FMargin(7, 5))
                .ToolTipText(FText::FromString(Tooltip))
                .OnClicked_Lambda([this, Id, bChildren]
                { if (bChildren) { if (Collapsed.Contains(Id)) Collapsed.Remove(Id); else Collapsed.Add(Id); } return FReply::Handled(); })
                [SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().Padding(0, 0, 6, 0)[SNew(STextBlock)
                        .Text_Lambda([this, Id, bChildren] { return FText::FromString(bChildren ? (Collapsed.Contains(Id) ? TEXT("+") : TEXT("−")) : TEXT("•")); })]
                    + SHorizontalBox::Slot().FillWidth(1)[SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).AutoWrapText(true).Text(Entry->Text(Office.Label))]
                        + SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).AutoWrapText(true)
                            .Visibility(Office.Note.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
                            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)).Text(Entry->Text(Office.Note))]]]]];
        TSharedRef<SVerticalBox> Branches = SNew(SVerticalBox)
            .Visibility_Lambda([this, Id] { return Collapsed.Contains(Id) ? EVisibility::Collapsed : EVisibility::Visible; });
        for (int32 I = 0; I < Children.Num(); ++I)
            Branches->AddSlot().AutoHeight()[SNew(STMOPAtlasBranch).Last(I == Children.Num() - 1)
                .Reporting(Children[I]->Relation == TEXT("reports_to"))[Node(*Children[I])]];
        Result->AddSlot().AutoHeight().Padding(8, 0, 0, 0)[Branches];
        return Result;
    }
    void Rebuild()
    {
        Roots->ClearChildren();
        for (const auto& Office : Entry->Hierarchy) if (Office.Parent.IsEmpty())
            Roots->AddSlot().AutoHeight().Padding(3)[Node(Office)];
        Roots->AddSlot().AutoHeight().Padding(3)[SNew(SButton)
            .Text(L(NSLOCTEXT("TMOP", "AtlasProfileNotes", "Regering, tjänster och källanmärkningar")))
            .OnClicked_Lambda([this] { bContext = !bContext; return FReply::Handled(); })];
        TSharedRef<SVerticalBox> Context = SNew(SVerticalBox)
            .Visibility_Lambda([this] { return bContext ? EVisibility::Visible : EVisibility::Collapsed; });
        for (const TCHAR* Field : {TEXT("government"), TEXT("leaders"), TEXT("ministers"), TEXT("intelligence"), TEXT("personnel"), TEXT("research")})
            if (!Entry->Text(Field).IsEmpty()) Context->AddSlot().AutoHeight().Padding(4, 8)
                [SNew(STextBlock).AutoWrapText(true).Text(Entry->Text(Field))];
        Roots->AddSlot().AutoHeight()[Context];
    }
    const FTMOPAtlasEntry* Entry = nullptr;
    TSharedPtr<SVerticalBox> Roots;
    TSet<FString> Collapsed;
    bool bContext = false;
    float MinimumTreeWidth = 300.f;
};
}

TSharedRef<SWidget> MakeTMOPAtlasHierarchy(const FTMOPAtlasEntry& Entry)
{ return SNew(STMOPAtlasHierarchy, &Entry); }
