#include "UI/TMOPPauseMenuWidget.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/GameUserSettings.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Research/TMOPUppslagTypes.h"
#include "Rendering/DrawElements.h"
#include "Scalability.h"
#include "Styling/CoreStyle.h"
#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationDebugDirector.h"
#include "UI/TMOPMapWidget.h"
#include "UI/TMOPMapComponent.h"
#include "UI/TMOPMenuSaveGame.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "World/TMOPFindingActor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
/** Natural ordering keeps EAD34 before EAD145 and preserves suffix ordering. */
int32 CompareUppslagIdsNaturally(const FString& Left, const FString& Right)
{
    int32 LeftIndex = 0;
    int32 RightIndex = 0;
    while (LeftIndex < Left.Len() && RightIndex < Right.Len())
    {
        if (FChar::IsDigit(Left[LeftIndex]) && FChar::IsDigit(Right[RightIndex]))
        {
            const int32 LeftRunStart = LeftIndex;
            const int32 RightRunStart = RightIndex;
            while (LeftIndex < Left.Len() && FChar::IsDigit(Left[LeftIndex])) ++LeftIndex;
            while (RightIndex < Right.Len() && FChar::IsDigit(Right[RightIndex])) ++RightIndex;

            int32 LeftSignificant = LeftRunStart;
            int32 RightSignificant = RightRunStart;
            while (LeftSignificant + 1 < LeftIndex && Left[LeftSignificant] == TEXT('0'))
                ++LeftSignificant;
            while (RightSignificant + 1 < RightIndex && Right[RightSignificant] == TEXT('0'))
                ++RightSignificant;

            const int32 LeftDigits = LeftIndex - LeftSignificant;
            const int32 RightDigits = RightIndex - RightSignificant;
            if (LeftDigits != RightDigits) return LeftDigits < RightDigits ? -1 : 1;
            for (int32 Offset = 0; Offset < LeftDigits; ++Offset)
                if (Left[LeftSignificant + Offset] != Right[RightSignificant + Offset])
                    return Left[LeftSignificant + Offset] < Right[RightSignificant + Offset] ? -1 : 1;
            continue;
        }

        const TCHAR LeftCharacter = FChar::ToUpper(Left[LeftIndex]);
        const TCHAR RightCharacter = FChar::ToUpper(Right[RightIndex]);
        if (LeftCharacter != RightCharacter)
            return LeftCharacter < RightCharacter ? -1 : 1;
        ++LeftIndex;
        ++RightIndex;
    }
    if (LeftIndex == Left.Len() && RightIndex == Right.Len()) return 0;
    return LeftIndex == Left.Len() ? -1 : 1;
}

bool UppslagIdNaturalLess(const FName Left, const FName Right)
{
    return CompareUppslagIdsNaturally(Left.ToString(), Right.ToString()) < 0;
}

enum class ETMOPCoverageState : uint8
{
    Unknown = 0,
    Added = 1,
    OnlineNotAdded = 2,
    PoliceOnly = 3,
    PoliceHighPriority = 4
};

bool IsUppslagAvailableOnline(const FTMOPUppslagRow& Row)
{
    switch (Row.Availability)
    {
    case ETMOPUppslagAvailability::Available:
    case ETMOPUppslagAvailability::PartiallyAvailable:
    case ETMOPUppslagAvailability::Partial:
    case ETMOPUppslagAvailability::MissingPage:
    case ETMOPUppslagAvailability::AvailableMasked:
        return true;
    default:
        return Row.bRetrieved || Row.bPartiallyAdded;
    }
}

ETMOPCoverageState ClassifyUppslagCoverage(const FTMOPUppslagRow& Row)
{
    if (Row.bAddedToProject) return ETMOPCoverageState::Added;
    if (Row.Availability == ETMOPUppslagAvailability::NotReleased)
        return Row.bHighPriorityForGame
            ? ETMOPCoverageState::PoliceHighPriority
            : ETMOPCoverageState::PoliceOnly;
    if (IsUppslagAvailableOnline(Row)) return ETMOPCoverageState::OnlineNotAdded;
    return ETMOPCoverageState::Unknown;
}

FLinearColor CoverageStateColor(const ETMOPCoverageState State)
{
    switch (State)
    {
    case ETMOPCoverageState::Added: return FLinearColor(0.12f, 0.72f, 0.28f, 1.0f);
    case ETMOPCoverageState::OnlineNotAdded: return FLinearColor(0.96f, 0.72f, 0.08f, 1.0f);
    case ETMOPCoverageState::PoliceOnly: return FLinearColor(0.42f, 0.45f, 0.50f, 1.0f);
    case ETMOPCoverageState::PoliceHighPriority: return FLinearColor(0.90f, 0.08f, 0.06f, 1.0f);
    default: return FLinearColor(0.13f, 0.14f, 0.16f, 1.0f);
    }
}

struct FTMOPCoverageCounts
{
    int32 Total = 0;
    int32 Added = 0;
    int32 OnlineNotAdded = 0;
    int32 PoliceOnly = 0;
    int32 PoliceHighPriority = 0;
    int32 Unknown = 0;

    void Add(const ETMOPCoverageState State)
    {
        ++Total;
        switch (State)
        {
        case ETMOPCoverageState::Added: ++Added; break;
        case ETMOPCoverageState::OnlineNotAdded: ++OnlineNotAdded; break;
        case ETMOPCoverageState::PoliceOnly: ++PoliceOnly; break;
        case ETMOPCoverageState::PoliceHighPriority: ++PoliceHighPriority; break;
        default: ++Unknown; break;
        }
    }

    int32 AddedPercent() const
    {
        return Total > 0 ? FMath::RoundToInt(100.0f * Added / Total) : 0;
    }
};

/** Draws one entire investigation-section coverage strip in a single widget. */
class STMOPUppslagCoverageBar final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPUppslagCoverageBar) {}
        /** ETMOPCoverageState values in natural uppslag order. */
        SLATE_ARGUMENT(TArray<uint8>, EntryStates)
        SLATE_ARGUMENT(float, DesiredWidth)
    SLATE_END_ARGS()

    void Construct(const FArguments& Arguments)
    {
        EntryStates = Arguments._EntryStates;
        DesiredWidth = FMath::Max(18.0f, Arguments._DesiredWidth);
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return FVector2D(DesiredWidth, 24.0f);
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
        const FSlateRect& CullingRect, FSlateWindowElementList& DrawElements,
        int32 LayerId, const FWidgetStyle& WidgetStyle,
        bool bParentEnabled) const override
    {
        const FVector2D Size = Geometry.GetLocalSize();
        const float BarWidth = static_cast<float>(Size.X);
        const float BarHeight = static_cast<float>(Size.Y);
        const FSlateBrush* WhiteBrush = FCoreStyle::Get().GetBrush("WhiteBrush");
        FSlateDrawElement::MakeBox(DrawElements, LayerId, Geometry.ToPaintGeometry(),
            WhiteBrush, ESlateDrawEffect::None,
            CoverageStateColor(ETMOPCoverageState::Unknown));

        if (!EntryStates.IsEmpty() && BarWidth > 2.0f)
        {
            int32 Counts[5] = {0, 0, 0, 0, 0};
            for (const uint8 RawState : EntryStates)
            {
                const int32 StateIndex = FMath::Clamp(static_cast<int32>(RawState), 0, 4);
                ++Counts[StateIndex];
            }

            float X = 0.0f;
            // Draw known categories first; unknown entries retain the dark remainder.
            for (int32 StateIndex = 1; StateIndex <= 4; ++StateIndex)
            {
                if (Counts[StateIndex] <= 0) continue;
                const float SegmentWidth = BarWidth * Counts[StateIndex] /
                    static_cast<float>(EntryStates.Num());
                const FGeometry SegmentGeometry = Geometry.MakeChild(
                    FVector2D(SegmentWidth, BarHeight),
                    FSlateLayoutTransform(FVector2D(X, 0.0f)));
                FSlateDrawElement::MakeBox(DrawElements, LayerId + 1,
                    SegmentGeometry.ToPaintGeometry(), WhiteBrush,
                    ESlateDrawEffect::None,
                    CoverageStateColor(static_cast<ETMOPCoverageState>(StateIndex)));
                X += SegmentWidth;
            }
        }

        const FLinearColor BorderColor(0.72f, 0.74f, 0.78f, 1.0f);
        TArray<FVector2f> Border;
        Border.Add(FVector2f(0.5f, 0.5f));
        Border.Add(FVector2f(BarWidth - 0.5f, 0.5f));
        Border.Add(FVector2f(BarWidth - 0.5f, BarHeight - 0.5f));
        Border.Add(FVector2f(0.5f, BarHeight - 0.5f));
        Border.Add(FVector2f(0.5f, 0.5f));
        FSlateDrawElement::MakeLines(DrawElements, LayerId + 2,
            Geometry.ToPaintGeometry(), Border, ESlateDrawEffect::None,
            BorderColor, false, 1.5f);
        return LayerId + 2;
    }

private:
    TArray<uint8> EntryStates;
    float DesiredWidth = 620.0f;
};

FText SectionTitle(const ETMOPPauseHubSection Section)
{
    switch (Section)
    {
    case ETMOPPauseHubSection::Inventory: return NSLOCTEXT("TMOP", "HubInventory", "INVENTORY");
    case ETMOPPauseHubSection::Evidence: return NSLOCTEXT("TMOP", "HubEvidence", "NOTEBOOK / EVIDENCE");
    case ETMOPPauseHubSection::Sources: return NSLOCTEXT("TMOP", "HubSources", "KÄLLOR / UPPSLAG");
    case ETMOPPauseHubSection::Publications: return NSLOCTEXT("TMOP", "HubPublications", "NEWSPAPERS / BOOKS");
    case ETMOPPauseHubSection::Settings: return NSLOCTEXT("TMOP", "HubSettings", "SETTINGS");
    case ETMOPPauseHubSection::Controls: return NSLOCTEXT("TMOP", "HubControls", "CONTROLS");
    case ETMOPPauseHubSection::SaveLoad: return NSLOCTEXT("TMOP", "HubSaveLoad", "SAVE / LOAD");
    case ETMOPPauseHubSection::Quit: return NSLOCTEXT("TMOP", "HubQuit", "QUIT");
    case ETMOPPauseHubSection::MoveInTime: return NSLOCTEXT("TMOP", "HubMoveTime", "MOVE IN TIME");
    default: return FText::GetEmpty();
    }
}
}

void UTMOPPauseMenuWidget::InitializePauseMenu(APlayerController* InController,
    ATMOPPlayerCharacter* InCharacter)
{
    PlayerController = InController;
    PlayerCharacter = InCharacter;
    SetIsFocusable(true);
}

void UTMOPPauseMenuWidget::SetMenuVisible(const bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (bVisible)
    {
        SetStatus(FText::GetEmpty());
        ShowSection(CurrentSection);
    }
}

TSharedRef<SWidget> UTMOPPauseMenuWidget::RebuildWidget()
{
    auto MakeNavigationButton = [this](const FText& Label, const ETMOPPauseHubSection Section)
    {
        return SNew(SButton).Text(Label)
            .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleSectionClicked, Section);
    };
    TSharedRef<SVerticalBox> NavigationPanel = SNew(SVerticalBox);
    auto AddNavigationEntry = [&NavigationPanel, &MakeNavigationButton](
        const ETMOPPauseHubSection Section)
    {
        NavigationPanel->AddSlot().AutoHeight().Padding(3.0f)
        [ MakeNavigationButton(SectionTitle(Section), Section) ];
    };
    AddNavigationEntry(ETMOPPauseHubSection::Inventory);
    AddNavigationEntry(ETMOPPauseHubSection::Evidence);
    AddNavigationEntry(ETMOPPauseHubSection::Sources);
    AddNavigationEntry(ETMOPPauseHubSection::Publications);
    AddNavigationEntry(ETMOPPauseHubSection::Settings);
    AddNavigationEntry(ETMOPPauseHubSection::Controls);
    AddNavigationEntry(ETMOPPauseHubSection::SaveLoad);
    AddNavigationEntry(ETMOPPauseHubSection::Quit);
    NavigationPanel->AddSlot().FillHeight(1.0f)[SNew(SSpacer)];
    NavigationPanel->AddSlot().AutoHeight().Padding(3.0f, 20.0f, 3.0f, 3.0f)
    [ MakeNavigationButton(SectionTitle(ETMOPPauseHubSection::MoveInTime),
        ETMOPPauseHubSection::MoveInTime) ];
    NavigationPanel->AddSlot().AutoHeight().Padding(3.0f, 14.0f, 3.0f, 3.0f)
    [ SNew(SButton).Text(NSLOCTEXT("TMOP", "HubResume", "FORTSÄTT (ENTER / ESC)"))
      .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleResumeClicked) ];

    TSharedRef<SVerticalBox> PagePanel = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f)
        [ SAssignNew(SectionTitleText, STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold", 21)) ]
        + SVerticalBox::Slot().FillHeight(1.0f)
        [ SNew(SScrollBox) + SScrollBox::Slot()[SAssignNew(ContentBox, SVerticalBox)] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 14.0f, 0.0f, 0.0f)
        [ SAssignNew(StatusText, STextBlock).ColorAndOpacity(FLinearColor(0.95f, 0.7f, 0.2f)) ];

    TSharedRef<SWidget> RootWidget = SNew(SBorder)
        .BorderBackgroundColor(FLinearColor(0.008f, 0.012f, 0.022f, 0.97f)).Padding(34.0f)
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f, 8.0f, 20.0f)
          [ SNew(STextBlock).Text(NSLOCTEXT("TMOP", "PauseHubTitle", "THE MURDER OF OLOF PALME"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 25)) ]
          + SVerticalBox::Slot().FillHeight(1.0f)
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 24.0f, 0.0f)
            [ SNew(SBox).WidthOverride(255.0f)[NavigationPanel] ]
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.025f, 0.035f, 0.055f, 0.96f))
              .Padding(24.0f)[PagePanel] ] ] ];
    ShowSection(CurrentSection);
    return RootWidget;
}

FReply UTMOPPauseMenuWidget::HandleSectionClicked(const ETMOPPauseHubSection Section)
{
    if (Section == ETMOPPauseHubSection::Sources)
    {
        SelectedSourceMainSection = NAME_None;
        SelectedSourceSeries = NAME_None;
    }
    ShowSection(Section);
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleSourceMainSectionClicked(const FName MainSectionId)
{
    SelectedSourceMainSection = MainSectionId;
    SelectedSourceSeries = NAME_None;
    if (ContentBox.IsValid()) ContentBox->ClearChildren();
    BuildSourcesPage();
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleSourceSeriesClicked(const FName SeriesId)
{
    SelectedSourceSeries = SeriesId;
    if (ContentBox.IsValid()) ContentBox->ClearChildren();
    BuildSourcesPage();
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleSourceBackClicked()
{
    if (!SelectedSourceSeries.IsNone()) SelectedSourceSeries = NAME_None;
    else SelectedSourceMainSection = NAME_None;
    if (ContentBox.IsValid()) ContentBox->ClearChildren();
    BuildSourcesPage();
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::ShowSection(const ETMOPPauseHubSection Section)
{
    CurrentSection = Section;
    if (!ContentBox.IsValid()) return;
    ContentBox->ClearChildren();
    if (SectionTitleText.IsValid()) SectionTitleText->SetText(SectionTitle(Section));
    SetStatus(FText::GetEmpty());
    switch (Section)
    {
    case ETMOPPauseHubSection::Inventory: BuildInventoryPage(); break;
    case ETMOPPauseHubSection::Evidence: BuildEvidencePage(); break;
    case ETMOPPauseHubSection::Sources: BuildSourcesPage(); break;
    case ETMOPPauseHubSection::Publications: BuildPublicationsPage(); break;
    case ETMOPPauseHubSection::Settings: BuildSettingsPage(); break;
    case ETMOPPauseHubSection::Controls: BuildControlsPage(); break;
    case ETMOPPauseHubSection::SaveLoad: BuildSaveLoadPage(); break;
    case ETMOPPauseHubSection::Quit: BuildQuitPage(); break;
    case ETMOPPauseHubSection::MoveInTime: BuildMoveInTimePage(); break;
    }
}

void UTMOPPauseMenuWidget::AddHeading(const FText& Text)
{ ContentBox->AddSlot().AutoHeight().Padding(2.0f, 10.0f)[ SNew(STextBlock).Text(Text).Font(FCoreStyle::GetDefaultFontStyle("Bold", 17)) ]; }
void UTMOPPauseMenuWidget::AddBody(const FText& Text)
{ ContentBox->AddSlot().AutoHeight().Padding(2.0f, 5.0f)[ SNew(STextBlock).Text(Text).AutoWrapText(true) ]; }

void UTMOPPauseMenuWidget::BuildInventoryPage()
{
    UTMOPInventoryComponent* Inventory = IsValid(PlayerCharacter)
        ? PlayerCharacter->Inventory.Get() : nullptr;
    if (!IsValid(Inventory) || Inventory->Items.IsEmpty())
    { AddBody(NSLOCTEXT("TMOP", "InventoryEmpty", "Inventory är tomt.")); return; }
    for (const FTMOPInventoryEntry& Entry : Inventory->Items)
    {
        UTMOPItemDefinition* Item = Entry.Item.Get();
        if (!IsValid(Item)) continue;
        const FText Label = FText::Format(NSLOCTEXT("TMOP", "InventoryLine", "{0}  ×{1}"),
            Item->DisplayName, FText::AsNumber(Entry.Quantity));
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 4.0f)
        [ SNew(SButton).Text(Label).IsEnabled(Item->bCanEquip)
          .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleEquipItem, Item) ];
    }
}

FReply UTMOPPauseMenuWidget::HandleEquipItem(UTMOPItemDefinition* Item)
{
    if (IsValid(PlayerCharacter) && IsValid(PlayerCharacter->Inventory) &&
        PlayerCharacter->Inventory->EquipItem(Item))
        SetStatus(FText::Format(NSLOCTEXT("TMOP", "EquippedItem", "Vald: {0}"), Item->DisplayName));
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::BuildEvidencePage()
{
    if (!IsValid(PlayerCharacter) || PlayerCharacter->DiscoveredEvidenceIds.IsEmpty())
    {
        AddBody(NSLOCTEXT("TMOP", "NoEvidenceYet", "Inga fynd eller observationer har lagts till i anteckningsboken ännu."));
        AddBody(NSLOCTEXT("TMOP", "EvidenceHint", "Poster visas här när spelet anropar Discover Evidence med ett stabilt Evidence ID."));
        return;
    }
    for (const FName Id : PlayerCharacter->DiscoveredEvidenceIds)
        AddBody(FText::FromName(Id));
}

void UTMOPPauseMenuWidget::BuildSourcesPage()
{
    if (!IsValid(UppslagTable))
    {
        AddBody(NSLOCTEXT("TMOP", "NoUppslagTable",
            "Ingen uppslagstabell är vald. Ange DT_TMOP_Uppslag_REGISTER i Class Defaults → TMOP → UI → Pause → Sources."));
        return;
    }

    TArray<FTMOPUppslagRow*> Rows;
    UppslagTable->GetAllRows(TEXT("Pause menu sources"), Rows);
    Rows.Sort([](const FTMOPUppslagRow& A, const FTMOPUppslagRow& B)
    {
        return UppslagIdNaturalLess(A.UppslagId, B.UppslagId);
    });

    AddHeading(NSLOCTEXT("TMOP", "PoliceSourcesHeading", "1. POLISUPPSLAG"));
    AddBody(NSLOCTEXT("TMOP", "SourcesCoverageIntro",
        "Varje rad motsvarar ett avsnitt i utredningen. Radens längd visar hur många dokument avsnittet innehåller i förhållande till det största avsnittet."));

    const auto MakeLegendEntry = [](const FLinearColor Color, const FText& Label)
    {
        return SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBorder).BorderBackgroundColor(Color).Padding(0.0f)
              [ SNew(SBox).WidthOverride(18.0f).HeightOverride(12.0f) ] ]
            + SHorizontalBox::Slot().AutoWidth().Padding(6.0f, 0.0f, 18.0f, 0.0f)
              .VAlign(VAlign_Center)
            [ SNew(STextBlock).Text(Label) ];
    };
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 7.0f, 2.0f, 12.0f)
    [ SNew(SWrapBox).UseAllottedSize(true)
      + SWrapBox::Slot()
      [ MakeLegendEntry(CoverageStateColor(ETMOPCoverageState::Added),
          NSLOCTEXT("TMOP", "CoverageAddedLegend", "Grönt: inlagt")) ]
      + SWrapBox::Slot()
      [ MakeLegendEntry(CoverageStateColor(ETMOPCoverageState::OnlineNotAdded),
          NSLOCTEXT("TMOP", "CoverageOnlineLegend", "Gult: online, ej inlagt")) ]
      + SWrapBox::Slot()
      [ MakeLegendEntry(CoverageStateColor(ETMOPCoverageState::PoliceOnly),
          NSLOCTEXT("TMOP", "CoveragePoliceLegend", "Grått: kvar hos polisen")) ]
      + SWrapBox::Slot()
      [ MakeLegendEntry(CoverageStateColor(ETMOPCoverageState::PoliceHighPriority),
          NSLOCTEXT("TMOP", "CoveragePriorityLegend",
              "Rött: kvar hos polisen, hög prioritet 23:00–23:45")) ] ];

    TMap<FName, TArray<FTMOPUppslagRow*>> RowsBySeries;
    TMap<FName, FString> SectionDescriptions;
    for (FTMOPUppslagRow* Row : Rows)
        if (Row != nullptr && Row->SourceCategory == ETMOPSourceCategory::PoliceUppslag &&
            !Row->SeriesId.IsNone())
        {
            if (Row->bIsSectionDefinition)
            {
                SectionDescriptions.FindOrAdd(Row->SeriesId) =
                    Row->SectionDescription.IsEmpty()
                    ? Row->Title.ToString() : Row->SectionDescription;
                RowsBySeries.FindOrAdd(Row->SeriesId);
            }
            else RowsBySeries.FindOrAdd(Row->SeriesId).Add(Row);
        }

    const auto MainSectionForSeries = [](const FName SeriesId)
    {
        const FString Value = SeriesId.ToString().ToUpper();
        FString Main;
        for (const TCHAR Character : Value)
        {
            if (!FChar::IsAlpha(Character)) break;
            Main.AppendChar(Character);
            break;
        }
        return Main.IsEmpty() ? NAME_None : FName(*Main);
    };
    const auto CoverageForRows = [](const TArray<FTMOPUppslagRow*>& SourceRows)
    {
        FTMOPCoverageCounts Result;
        for (const FTMOPUppslagRow* SourceRow : SourceRows)
            if (SourceRow != nullptr) Result.Add(ClassifyUppslagCoverage(*SourceRow));
        return Result;
    };
    const auto StatesForRows = [](const TArray<FTMOPUppslagRow*>& SourceRows)
    {
        TArray<uint8> Result;
        Result.Reserve(SourceRows.Num());
        for (const FTMOPUppslagRow* SourceRow : SourceRows)
            if (SourceRow != nullptr)
                Result.Add(static_cast<uint8>(ClassifyUppslagCoverage(*SourceRow)));
        return Result;
    };
    const auto StatisticsForCoverage = [](const FTMOPCoverageCounts& Coverage)
    {
        FString Result = FString::Printf(
            TEXT("%d uppslag inlagda\n%d tillgängliga online men ej inlagda\n")
            TEXT("%d ej utlämnade från polisen\n")
            TEXT("%d ej utlämnade men av stort intresse\n%d procent inlagt"),
            Coverage.Added, Coverage.OnlineNotAdded, Coverage.PoliceOnly,
            Coverage.PoliceHighPriority, Coverage.AddedPercent());
        if (Coverage.Unknown > 0)
            Result += FString::Printf(TEXT("\n%d ej klassificerade"), Coverage.Unknown);
        return Result;
    };
    const auto ResolveSectionDescription = [&SectionDescriptions](const FName SeriesId)
    {
        FString Description = SectionDescriptions.FindRef(SeriesId);
        if (!Description.TrimStartAndEnd().IsEmpty()) return Description;
        FString ParentId = SeriesId.ToString();
        while (ParentId.Len() > 1)
        {
            ParentId.LeftChopInline(1);
            const FString ParentDescription = SectionDescriptions.FindRef(FName(*ParentId));
            if (!ParentDescription.TrimStartAndEnd().IsEmpty()) return ParentDescription;
        }
        return FString::Printf(TEXT("Avsnitt %s – beskrivning saknas i registret."),
            *SeriesId.ToString());
    };

    struct FSectionCardData
    {
        FName Id = NAME_None;
        FText Label;
        FString Description;
        TArray<FTMOPUppslagRow*> Rows;
        int32 Span = 4;
        bool bMainSection = false;
    };

    const auto AssignSpans = [](TArray<FSectionCardData>& Cards)
    {
        int32 MaximumCount = 1;
        for (const FSectionCardData& Card : Cards)
            MaximumCount = FMath::Max(MaximumCount, Card.Rows.Num());
        for (FSectionCardData& Card : Cards)
        {
            const float RelativeSize = static_cast<float>(Card.Rows.Num()) / MaximumCount;
            Card.Span = RelativeSize <= 0.16f ? 3
                : RelativeSize <= 0.38f ? 4
                : RelativeSize <= 0.72f ? 6 : 12;
        }
    };
    const auto MakeCard = [this, &CoverageForRows, &StatesForRows,
        &StatisticsForCoverage](const FSectionCardData& Card) -> TSharedRef<SWidget>
    {
        const FTMOPCoverageCounts Coverage = CoverageForRows(Card.Rows);
        const TArray<uint8> States = StatesForRows(Card.Rows);
        const FText ButtonLabel = Card.Label;
        TSharedRef<SVerticalBox> CardContent = SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock).Text(ButtonLabel)
              .Font(FCoreStyle::GetDefaultFontStyle("Bold", Card.bMainSection ? 30 : 24)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 8.0f)
            [ SNew(STextBlock).Text(FText::FromString(Card.Description))
              .AutoWrapText(true).ColorAndOpacity(FLinearColor(0.78f, 0.80f, 0.84f, 1.0f)) ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STMOPUppslagCoverageBar).EntryStates(States).DesiredWidth(420.0f) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
            [ SNew(STextBlock).Text(FText::FromString(StatisticsForCoverage(Coverage)))
              .AutoWrapText(true) ];
        if (Card.bMainSection)
            return SNew(SBox).MinDesiredWidth(230.0f).HeightOverride(220.0f)
                [ SNew(SButton).ContentPadding(FMargin(14.0f))
                  .OnClicked_UObject(this,
                      &UTMOPPauseMenuWidget::HandleSourceMainSectionClicked, Card.Id)
                  [ CardContent ] ];
        return SNew(SBox).MinDesiredWidth(230.0f).HeightOverride(220.0f)
            [ SNew(SButton).ContentPadding(FMargin(14.0f))
              .OnClicked_UObject(this,
                  &UTMOPPauseMenuWidget::HandleSourceSeriesClicked, Card.Id)
              [ CardContent ] ];
    };
    const auto AddPackedCards = [this, &MakeCard](const TArray<FSectionCardData>& Cards)
    {
        TSharedPtr<SHorizontalBox> CurrentRow;
        int32 UsedColumns = 0;
        const auto BeginRow = [&CurrentRow, &UsedColumns]()
        {
            CurrentRow = SNew(SHorizontalBox);
            UsedColumns = 0;
        };
        const auto FinishRow = [this, &CurrentRow, &UsedColumns]()
        {
            if (!CurrentRow.IsValid() || UsedColumns <= 0) return;
            if (UsedColumns < 12)
                CurrentRow->AddSlot().FillWidth(static_cast<float>(12 - UsedColumns))
                    [ SNew(SSpacer) ];
            ContentBox->AddSlot().AutoHeight().Padding(0.0f, 4.0f)[ CurrentRow.ToSharedRef() ];
            CurrentRow.Reset();
            UsedColumns = 0;
        };
        BeginRow();
        for (const FSectionCardData& Card : Cards)
        {
            if (UsedColumns > 0 && UsedColumns + Card.Span > 12)
            {
                FinishRow();
                BeginRow();
            }
            CurrentRow->AddSlot().FillWidth(static_cast<float>(Card.Span))
                .Padding(4.0f)[ MakeCard(Card) ];
            UsedColumns += Card.Span;
        }
        FinishRow();
    };

    TMap<FName, TArray<FTMOPUppslagRow*>> RowsByMainSection;
    for (const TPair<FName, TArray<FTMOPUppslagRow*>>& Pair : RowsBySeries)
    {
        const FName MainId = MainSectionForSeries(Pair.Key);
        if (!MainId.IsNone()) RowsByMainSection.FindOrAdd(MainId).Append(Pair.Value);
    }

    if (SelectedSourceMainSection.IsNone())
    {
        AddBody(NSLOCTEXT("TMOP", "SourcesMainSectionHint",
            "Välj ett huvudavsnitt. Varje ruta summerar alla dess underavsnitt."));
        TArray<FName> MainIds;
        RowsByMainSection.GetKeys(MainIds);
        MainIds.Sort(FNameLexicalLess());
        TArray<FSectionCardData> Cards;
        for (const FName MainId : MainIds)
        {
            FSectionCardData& Card = Cards.AddDefaulted_GetRef();
            Card.Id = MainId;
            Card.Label = FText::FromName(MainId);
            Card.Description = ResolveSectionDescription(MainId);
            Card.Rows = RowsByMainSection.FindChecked(MainId);
            Card.bMainSection = true;
        }
        AssignSpans(Cards);
        AddPackedCards(Cards);

        const auto AddOtherSourceCategory = [this, &Rows](
            const ETMOPSourceCategory Category, const FText& Heading)
        {
            TArray<const FTMOPUppslagRow*> CategoryRows;
            for (const FTMOPUppslagRow* SourceRow : Rows)
                if (SourceRow != nullptr && !SourceRow->bIsSectionDefinition &&
                    SourceRow->SourceCategory == Category)
                    CategoryRows.Add(SourceRow);
            if (CategoryRows.IsEmpty()) return;
            AddHeading(Heading);
            for (const FTMOPUppslagRow* SourceRow : CategoryRows)
            {
                const FString DisplayTitle = SourceRow->Title.IsEmpty()
                    ? SourceRow->UppslagId.ToString() : SourceRow->Title.ToString();
                FString Details = SourceRow->bAddedToProject ? TEXT("Inlagt i projektet")
                    : SourceRow->bPartiallyAdded ? TEXT("Delvis inlagt")
                    : SourceRow->bRetrieved ? TEXT("Genomgången") : TEXT("Inte genomgången");
                if (!SourceRow->SourceUrl.IsEmpty()) Details += TEXT("\n") + SourceRow->SourceUrl;
                AddHeading(FText::FromString(DisplayTitle));
                AddBody(FText::FromString(Details));
            }
        };
        AddOtherSourceCategory(ETMOPSourceCategory::Book,
            NSLOCTEXT("TMOP", "BookSourcesHierarchyHeading", "2. BÖCKER"));
        AddOtherSourceCategory(ETMOPSourceCategory::Article,
            NSLOCTEXT("TMOP", "ArticleSourcesHierarchyHeading", "3. ARTIKLAR"));
        AddOtherSourceCategory(ETMOPSourceCategory::Other,
            NSLOCTEXT("TMOP", "OtherSourcesHierarchyHeading", "4. ANDRA UPPGIFTER"));
        return;
    }

    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 2.0f, 2.0f, 10.0f)
    [ SNew(SButton).Text(SelectedSourceSeries.IsNone()
        ? NSLOCTEXT("TMOP", "BackToMainSections", "← Alla huvudavsnitt")
        : FText::Format(NSLOCTEXT("TMOP", "BackToMainSection", "← Tillbaka till {0}"),
            FText::FromName(SelectedSourceMainSection)))
      .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleSourceBackClicked) ];

    if (SelectedSourceSeries.IsNone())
    {
        AddHeading(FText::Format(NSLOCTEXT("TMOP", "MainSectionHeading", "{0} – {1}"),
            FText::FromName(SelectedSourceMainSection),
            FText::FromString(ResolveSectionDescription(SelectedSourceMainSection))));
        const TArray<FTMOPUppslagRow*>& MainRows =
            RowsByMainSection.FindOrAdd(SelectedSourceMainSection);
        const FTMOPCoverageCounts MainCoverage = CoverageForRows(MainRows);
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 2.0f, 2.0f, 14.0f)
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()
          [ SNew(STMOPUppslagCoverageBar).EntryStates(StatesForRows(MainRows))
            .DesiredWidth(760.0f) ]
          + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 7.0f)
          [ SNew(STextBlock).Text(FText::FromString(StatisticsForCoverage(MainCoverage))) ] ];

        TArray<FName> ChildSeries;
        for (const TPair<FName, TArray<FTMOPUppslagRow*>>& Pair : RowsBySeries)
            if (MainSectionForSeries(Pair.Key) == SelectedSourceMainSection)
                ChildSeries.Add(Pair.Key);
        ChildSeries.Sort(FNameLexicalLess());
        TArray<FSectionCardData> Cards;
        for (const FName SeriesId : ChildSeries)
        {
            FSectionCardData& Card = Cards.AddDefaulted_GetRef();
            Card.Id = SeriesId;
            Card.Label = SeriesId == SelectedSourceMainSection
                ? FText::Format(NSLOCTEXT("TMOP", "MainSectionOther", "{0} – ÖVRIGA"),
                    FText::FromName(SeriesId))
                : FText::FromName(SeriesId);
            Card.Description = ResolveSectionDescription(SeriesId);
            Card.Rows = RowsBySeries.FindChecked(SeriesId);
        }
        AssignSpans(Cards);
        AddPackedCards(Cards);
        return;
    }

    const TArray<FTMOPUppslagRow*>* SelectedRows = RowsBySeries.Find(SelectedSourceSeries);
    if (SelectedRows == nullptr)
    {
        AddBody(NSLOCTEXT("TMOP", "MissingSelectedSeries", "Underavsnittet saknas i registret."));
        return;
    }
    AddHeading(FText::Format(NSLOCTEXT("TMOP", "SelectedSeriesHeading", "AVSNITT {0}"),
        FText::FromName(SelectedSourceSeries)));
    AddBody(FText::FromString(ResolveSectionDescription(SelectedSourceSeries)));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 4.0f, 2.0f, 12.0f)
    [ SNew(STMOPUppslagCoverageBar).EntryStates(StatesForRows(*SelectedRows))
      .DesiredWidth(760.0f) ];
    TArray<FTMOPUppslagRow*> DetailRows = *SelectedRows;
    DetailRows.Sort([](const FTMOPUppslagRow& A, const FTMOPUppslagRow& B)
    {
        return UppslagIdNaturalLess(A.UppslagId, B.UppslagId);
    });
    int32 VisibleDetailCount = 0;
    for (const FTMOPUppslagRow* Row : DetailRows)
    {
        if (Row == nullptr) continue;
        const FString Title = Row->Title.IsEmpty() ? TEXT("Utan titel") : Row->Title.ToString();
        AddHeading(FText::FromString(FString::Printf(TEXT("%s — %s"),
            *Row->UppslagId.ToString(), *Title)));
        FString Details = Row->bAddedToProject ? TEXT("Inlagt")
            : Row->bPartiallyAdded ? TEXT("Delvis inlagt") : TEXT("Ej inlagt");
        if (!Row->DocumentDate.IsEmpty()) Details += TEXT(" • ") + Row->DocumentDate;
        if (!Row->SourceUrl.IsEmpty()) Details += TEXT("\n") + Row->SourceUrl;
        AddBody(FText::FromString(Details));
        if (++VisibleDetailCount >= 300) break;
    }
    return;

    TArray<FName> SeriesIds;
    RowsBySeries.GetKeys(SeriesIds);
    SeriesIds.Sort(FNameLexicalLess());

    FTMOPCoverageCounts TotalCoverage;
    int32 MaximumSeriesCount = 0;
    for (const FName SeriesId : SeriesIds)
    {
        const TArray<FTMOPUppslagRow*>& SeriesRows = RowsBySeries.FindChecked(SeriesId);
        MaximumSeriesCount = FMath::Max(MaximumSeriesCount, SeriesRows.Num());
        for (const FTMOPUppslagRow* Row : SeriesRows)
            if (Row != nullptr) TotalCoverage.Add(ClassifyUppslagCoverage(*Row));
    }

    FString TotalStatistics = FString::Printf(
        TEXT("TOTALT: %d uppslag\n%d uppslag inlagda\n")
        TEXT("%d tillgängliga online men ej inlagda\n")
        TEXT("%d ej utlämnade från polisen\n")
        TEXT("%d ej utlämnade från polisen men av stort intresse för spelet\n")
        TEXT("%d procent inlagt"),
        TotalCoverage.Total, TotalCoverage.Added, TotalCoverage.OnlineNotAdded,
        TotalCoverage.PoliceOnly, TotalCoverage.PoliceHighPriority,
        TotalCoverage.AddedPercent());
    if (TotalCoverage.Unknown > 0)
        TotalStatistics += FString::Printf(TEXT("\n%d med ännu ej fastställd status"),
            TotalCoverage.Unknown);
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 4.0f, 2.0f, 14.0f)
    [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.055f, 0.07f, 0.095f, 1.0f))
      .Padding(12.0f)
      [ SNew(STextBlock).Text(FText::FromString(TotalStatistics))
        .Font(FCoreStyle::GetDefaultFontStyle("Bold", 15)).AutoWrapText(true) ] ];

    const auto ResolveSectionDescriptionLegacy = [&SectionDescriptions](const FName SeriesId)
    {
        FString Description = SectionDescriptions.FindRef(SeriesId);
        if (!Description.TrimStartAndEnd().IsEmpty()) return Description;

        FString ParentId = SeriesId.ToString();
        while (ParentId.Len() > 1)
        {
            ParentId.LeftChopInline(1);
            const FString ParentDescription = SectionDescriptions.FindRef(FName(*ParentId));
            if (!ParentDescription.TrimStartAndEnd().IsEmpty())
                return FString::Printf(TEXT("Delavsnitt inom %s: %s"),
                    *ParentId, *ParentDescription);
        }
        return FString::Printf(TEXT("Avsnitt %s – detaljerad beskrivning saknas i registret."),
            *SeriesId.ToString());
    };

    for (const FName SeriesId : SeriesIds)
    {
        TArray<FTMOPUppslagRow*>& SeriesRows = RowsBySeries.FindChecked(SeriesId);
        SeriesRows.Sort([](const FTMOPUppslagRow& A, const FTMOPUppslagRow& B)
        {
            return UppslagIdNaturalLess(A.UppslagId, B.UppslagId);
        });

        TArray<uint8> EntryStates;
        EntryStates.Reserve(SeriesRows.Num());
        FTMOPCoverageCounts Coverage;
        for (const FTMOPUppslagRow* Row : SeriesRows)
        {
            if (Row == nullptr) continue;
            const ETMOPCoverageState State = ClassifyUppslagCoverage(*Row);
            EntryStates.Add(static_cast<uint8>(State));
            Coverage.Add(State);
        }
        FString StatisticsString = FString::Printf(
            TEXT("%d uppslag inlagda\n")
            TEXT("%d tillgängliga online men ej inlagda\n")
            TEXT("%d ej utlämnade från polisen\n")
            TEXT("%d ej utlämnade från polisen men av stort intresse för spelet\n")
            TEXT("%d procent inlagt"),
            Coverage.Added, Coverage.OnlineNotAdded, Coverage.PoliceOnly,
            Coverage.PoliceHighPriority, Coverage.AddedPercent());
        if (Coverage.Unknown > 0)
            StatisticsString += FString::Printf(TEXT("\n%d ej klassificerade"),
                Coverage.Unknown);
        const FText Statistics = FText::FromString(StatisticsString);
        constexpr float MinimumBarWidth = 24.0f;
        constexpr float MaximumBarWidth = 620.0f;
        const float RelativeCount = MaximumSeriesCount > 0
            ? static_cast<float>(Coverage.Total) / MaximumSeriesCount : 0.0f;
        const float BarWidth = FMath::Lerp(MinimumBarWidth, MaximumBarWidth,
            RelativeCount);

        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 12.0f)
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(105.0f)
              [ SNew(STextBlock).Text(FText::FromName(SeriesId))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 28)) ] ]
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [ SNew(STextBlock).Text(FText::FromString(
                ResolveSectionDescriptionLegacy(SeriesId))).AutoWrapText(true) ] ]
          + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 7.0f, 0.0f, 0.0f)
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(105.0f, 0.0f, 18.0f, 0.0f)
              .VAlign(VAlign_Center)
            [ SNew(STMOPUppslagCoverageBar).EntryStates(EntryStates)
              .DesiredWidth(BarWidth) ]
            + SHorizontalBox::Slot().FillWidth(1.0f)[ SNew(SSpacer) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(360.0f)
              [ SNew(STextBlock).Text(Statistics).AutoWrapText(false) ] ] ] ];
    }

    AddHeading(NSLOCTEXT("TMOP", "SourcesDetails", "Polisuppslag – detaljer"));

    int32 VisibleCount = 0;
    constexpr int32 MaximumVisibleRows = 300;
    for (const FTMOPUppslagRow* Row : Rows)
    {
        if (Row == nullptr || Row->SourceCategory != ETMOPSourceCategory::PoliceUppslag ||
            Row->bIsSectionDefinition || !Row->bRelevantToGame) continue;
        const FString SourceDisplayTitle = Row->Title.IsEmpty() ? TEXT("Utan titel") : Row->Title.ToString();
        const FString ProcessingStateText = Row->bAddedToProject ? TEXT("Inlagt")
            : Row->bPartiallyAdded ? TEXT("Delvis inlagt") : TEXT("Ej inlagt");
        AddHeading(FText::FromString(FString::Printf(TEXT("%s — %s"),
            *Row->UppslagId.ToString(), *SourceDisplayTitle)));
        FString Details = ProcessingStateText;
        if (!Row->DocumentDate.IsEmpty()) Details += TEXT(" • ") + Row->DocumentDate;
        if (!Row->SourceUrl.IsEmpty()) Details += TEXT("\n") + Row->SourceUrl;
        AddBody(FText::FromString(Details));
        if (++VisibleCount >= MaximumVisibleRows) break;
    }
    if (VisibleCount == 0)
        AddBody(NSLOCTEXT("TMOP", "NoRelevantSources", "Tabellen innehåller inga uppslag markerade som relevanta för spelet."));
    else if (VisibleCount >= MaximumVisibleRows)
        AddBody(NSLOCTEXT("TMOP", "SourcesLimited", "Listan visar de första 300 relevanta uppslagen för att hålla menyn snabb."));

    const auto NameArrayText = [](const TArray<FName>& Values) -> FString
    {
        FString Result;
        for (const FName Value : Values)
        {
            if (!Result.IsEmpty()) Result += TEXT(", ");
            Result += Value.ToString();
        }
        return Result;
    };
    const auto ReliabilityText = [](const ETMOPSourceReliability Value) -> FString
    {
        switch (Value)
        {
        case ETMOPSourceReliability::PrimarySource: return TEXT("Primärkälla");
        case ETMOPSourceReliability::SecondarySource: return TEXT("Sekundärkälla");
        case ETMOPSourceReliability::Corroborated: return TEXT("Bekräftad av flera källor");
        case ETMOPSourceReliability::Disputed: return TEXT("Motsagd / omtvistad");
        default: return TEXT("Obekräftad");
        }
    };
    const auto BuildSourceCategory = [this, &Rows, &NameArrayText, &ReliabilityText](
        const ETMOPSourceCategory RequestedSourceCategory, const FText& Heading,
        const FText& EmptyMessage)
    {
        AddHeading(Heading);
        TArray<const FTMOPUppslagRow*> CategoryRows;
        for (const FTMOPUppslagRow* SourceRow : Rows)
            if (SourceRow != nullptr && !SourceRow->bIsSectionDefinition &&
                SourceRow->SourceCategory == RequestedSourceCategory)
                CategoryRows.Add(SourceRow);

        if (CategoryRows.IsEmpty())
        {
            AddBody(EmptyMessage);
            return;
        }

        TArray<uint8> EntryStates;
        int32 FullyAddedCount = 0;
        int32 PartiallyAddedCount = 0;
        for (const FTMOPUppslagRow* SourceRow : CategoryRows)
        {
            const bool bSourceIsPresent = SourceRow->bRetrieved ||
                SourceRow->bAddedToProject || SourceRow->bPartiallyAdded;
            EntryStates.Add(bSourceIsPresent ? uint8(1) : uint8(0));
            FullyAddedCount += SourceRow->bAddedToProject ? 1 : 0;
            PartiallyAddedCount += SourceRow->bPartiallyAdded ? 1 : 0;
        }
        AddBody(FText::FromString(FString::Printf(
            TEXT("%d källor • %d helt inlagda • %d delvis inlagda"),
            CategoryRows.Num(), FullyAddedCount, PartiallyAddedCount)));
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 3.0f, 2.0f, 12.0f)
        [ SNew(STMOPUppslagCoverageBar).EntryStates(EntryStates)
          .DesiredWidth(620.0f) ];

        for (const FTMOPUppslagRow* SourceRow : CategoryRows)
        {
            const FString SourceDisplayTitle = SourceRow->Title.IsEmpty()
                ? SourceRow->UppslagId.ToString() : SourceRow->Title.ToString();
            AddHeading(FText::FromString(SourceDisplayTitle));

            FString Details = SourceRow->bAddedToProject ? TEXT("Inlagt i projektet")
                : SourceRow->bPartiallyAdded ? TEXT("Delvis inlagt")
                : SourceRow->bRetrieved ? TEXT("Genomgången") : TEXT("Inte genomgången");
            Details += TEXT(" • ") + ReliabilityText(SourceRow->Reliability);
            if (!SourceRow->AuthorOrCreator.IsEmpty())
                Details += TEXT("\nFörfattare/uppgiftslämnare: ") + SourceRow->AuthorOrCreator;
            if (!SourceRow->PublicationOrPlatform.IsEmpty())
                Details += TEXT("\nPublikation/plattform: ") + SourceRow->PublicationOrPlatform;
            if (!SourceRow->DocumentDate.IsEmpty())
                Details += TEXT("\nDatum: ") + SourceRow->DocumentDate;
            if (!SourceRow->ISBNOrArchiveId.IsEmpty())
                Details += TEXT("\nISBN/arkiv-ID: ") + SourceRow->ISBNOrArchiveId;
            if (!SourceRow->PageOrLocation.IsEmpty())
                Details += TEXT("\nSida/plats: ") + SourceRow->PageOrLocation;
            if (!SourceRow->CitationText.IsEmpty())
                Details += TEXT("\nKällhänvisning: ") + SourceRow->CitationText;
            if (!SourceRow->ImplementedSummary.IsEmpty())
                Details += TEXT("\nInlagt innehåll: ") + SourceRow->ImplementedSummary;
            if (!SourceRow->SourceUrl.IsEmpty())
                Details += TEXT("\nLänk: ") + SourceRow->SourceUrl;

            const FString People = NameArrayText(SourceRow->PersonEntityIds);
            const FString Vehicles = NameArrayText(SourceRow->VehicleEntityIds);
            const FString Events = NameArrayText(SourceRow->SharedEventIds);
            const FString Observations = NameArrayText(SourceRow->ObservationIds);
            const FString Anchors = NameArrayText(SourceRow->AnchorIds);
            if (!People.IsEmpty()) Details += TEXT("\nPersoner: ") + People;
            if (!Vehicles.IsEmpty()) Details += TEXT("\nFordon: ") + Vehicles;
            if (!Events.IsEmpty()) Details += TEXT("\nHändelser: ") + Events;
            if (!Observations.IsEmpty()) Details += TEXT("\nObservationer: ") + Observations;
            if (!Anchors.IsEmpty()) Details += TEXT("\nPlatser: ") + Anchors;
            AddBody(FText::FromString(Details));
        }
    };

    BuildSourceCategory(ETMOPSourceCategory::Book,
        NSLOCTEXT("TMOP", "BookSourcesHeading", "2. BÖCKER"),
        NSLOCTEXT("TMOP", "NoBookSources", "Inga böcker har registrerats ännu."));
    BuildSourceCategory(ETMOPSourceCategory::Article,
        NSLOCTEXT("TMOP", "ArticleSourcesHeading", "3. ARTIKLAR"),
        NSLOCTEXT("TMOP", "NoArticleSources", "Inga artiklar har registrerats ännu."));
    BuildSourceCategory(ETMOPSourceCategory::Other,
        NSLOCTEXT("TMOP", "OtherSourcesHeading", "4. ANDRA UPPGIFTER"),
        NSLOCTEXT("TMOP", "NoOtherSources",
            "Inga andra uppgifter från forum, sociala medier eller manuella tips har registrerats ännu."));
}

void UTMOPPauseMenuWidget::BuildPublicationsPage()
{
    UTMOPInventoryComponent* Inventory = IsValid(PlayerCharacter)
        ? PlayerCharacter->Inventory.Get() : nullptr;
    int32 Count = 0;
    if (IsValid(Inventory))
        for (const FTMOPInventoryEntry& Entry : Inventory->Items)
            if (UTMOPNewspaperItemDefinition* Newspaper =
                Cast<UTMOPNewspaperItemDefinition>(Entry.Item.Get()))
            {
                ++Count;
                ContentBox->AddSlot().AutoHeight().Padding(2.0f, 4.0f)
                [ SNew(SButton).Text(Newspaper->DisplayName)
                  .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleOpenPublication, Newspaper) ];
            }
    if (Count == 0) AddBody(NSLOCTEXT("TMOP", "NoPublications", "Du har inte hittat några tidningar eller böcker ännu."));
}

FReply UTMOPPauseMenuWidget::HandleOpenPublication(UTMOPNewspaperItemDefinition* Newspaper)
{
    if (IsValid(PlayerCharacter) && IsValid(Newspaper))
    {
        PlayerCharacter->SetPauseMenuOpen(false);
        PlayerCharacter->OpenNewspaper(Newspaper);
    }
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::BuildSettingsPage()
{
    AddHeading(NSLOCTEXT("TMOP", "GraphicsQuality", "Grafikkvalitet"));
    TSharedRef<SHorizontalBox> Quality = SNew(SHorizontalBox);
    const TArray<FText> Labels = { FText::FromString(TEXT("Low")), FText::FromString(TEXT("Medium")), FText::FromString(TEXT("High")), FText::FromString(TEXT("Epic")) };
    for (int32 I = 0; I < Labels.Num(); ++I)
        Quality->AddSlot().AutoWidth().Padding(3.0f)[ SNew(SButton).Text(Labels[I]).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleGraphicsQuality, I) ];
    ContentBox->AddSlot().AutoHeight()[Quality];
    AddHeading(NSLOCTEXT("TMOP", "InterfaceSettings", "Gränssnitt"));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "ToggleLabels", "Visa/dölj namn och ikoner i världen")).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleWorldLabels) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "ToggleMinimap", "Visa/dölj minimap")).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleMinimap) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "ToggleVSync", "Växla VSync")).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleVSync) ];
}

FReply UTMOPPauseMenuWidget::HandleGraphicsQuality(const int32 Quality)
{
    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    { Settings->SetOverallScalabilityLevel(FMath::Clamp(Quality, 0, 3)); Settings->ApplySettings(false); Settings->SaveSettings(); }
    SetStatus(NSLOCTEXT("TMOP", "GraphicsApplied", "Grafikinställningen har tillämpats."));
    OnSettingsRequested.Broadcast(); return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleWorldLabels()
{
    bWorldLabelsVisible = !bWorldLabelsVisible;
    if (GetWorld())
    {
        for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It) It->SetNameLabelVisible(bWorldLabelsVisible);
        for (TActorIterator<ATMOPVehicleBase> It(GetWorld()); It; ++It) It->SetNameLabelVisible(bWorldLabelsVisible);
        for (TActorIterator<ATMOPFindingActor> It(GetWorld()); It; ++It)
            if (IsValid(It->FindingLabel)) It->FindingLabel->SetVisibility(bWorldLabelsVisible, true);
    }
    SetStatus(bWorldLabelsVisible ? NSLOCTEXT("TMOP", "LabelsShown", "Namn och ikoner visas.") : NSLOCTEXT("TMOP", "LabelsHidden", "Namn och ikoner är dolda."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleMinimap()
{
    if (IsValid(PlayerCharacter) && IsValid(PlayerCharacter->MapComponent))
    {
        PlayerCharacter->MapComponent->bShowMinimap = !PlayerCharacter->MapComponent->bShowMinimap;
        if (IsValid(PlayerCharacter->MinimapWidget))
            PlayerCharacter->MinimapWidget->SetMapVisible(PlayerCharacter->MapComponent->bShowMinimap);
        SetStatus(PlayerCharacter->MapComponent->bShowMinimap ? NSLOCTEXT("TMOP", "MinimapShown", "Minimap visas.") : NSLOCTEXT("TMOP", "MinimapHidden", "Minimap är dold."));
    }
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleVSync()
{
    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    { Settings->SetVSyncEnabled(!Settings->IsVSyncEnabled()); Settings->ApplySettings(false); Settings->SaveSettings(); SetStatus(Settings->IsVSyncEnabled() ? FText::FromString(TEXT("VSync: On")) : FText::FromString(TEXT("VSync: Off"))); }
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::BuildControlsPage()
{
    AddBody(NSLOCTEXT("TMOP", "ControlsIntro", "Aktiva standardkontroller. Listan visas i två kolumner."));
    const TArray<TPair<FString,FString>> Controls = {
        {TEXT("WASD"),TEXT("Rörelse / körning")},{TEXT("Mouse"),TEXT("Kamera")},{TEXT("E"),TEXT("Interagera / prata")},{TEXT("Enter"),TEXT("Pausmeny")},
        {TEXT("Escape"),TEXT("Stäng / tillbaka")},{TEXT("M"),TEXT("Stor karta")},{TEXT("Tab (håll)"),TEXT("Snabb-inventory")},{TEXT("Q / E"),TEXT("Föregående / nästa item")},
        {TEXT("Left Shift"),TEXT("Spring")},{TEXT("Left Ctrl + Shift"),TEXT("Extra snabbt")},{TEXT("G"),TEXT("Släpp item")},{TEXT("Space"),TEXT("Hoppa")},
        {TEXT("Gamepad Start"),TEXT("Pausmeny")},{TEXT("Gamepad B"),TEXT("Stäng / tillbaka")},{TEXT("Gamepad LB"),TEXT("Snabb-inventory")},{TEXT("Mushjul"),TEXT("Zoom på karta/tidning")}
    };
    TSharedRef<SUniformGridPanel> Grid = SNew(SUniformGridPanel).SlotPadding(FMargin(5.0f));
    for (int32 I=0; I<Controls.Num(); ++I)
    {
        const int32 Column = I % 2, Row = I / 2;
        Grid->AddSlot(Column, Row)[ SNew(SBorder).Padding(8.0f)
            [ SNew(STextBlock).Text(FText::FromString(Controls[I].Key + TEXT(" — ") + Controls[I].Value)).AutoWrapText(true) ] ];
    }
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,8.0f)[Grid];
}

void UTMOPPauseMenuWidget::BuildSaveLoadPage()
{
    AddBody(FText::Format(NSLOCTEXT("TMOP", "SaveSlotInfo", "Aktiv sparplats: {0}"), FText::FromString(SaveSlotName)));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,5.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "SaveGame", "SPARA SPELET")).OnClicked_UObject(this,&UTMOPPauseMenuWidget::HandleSaveClicked) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,5.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "LoadGame", "LADDA SPELET")).OnClicked_UObject(this,&UTMOPPauseMenuWidget::HandleLoadClicked) ];
}

FReply UTMOPPauseMenuWidget::HandleSaveClicked()
{
    if (!IsValid(PlayerCharacter)) return FReply::Handled();
    UTMOPMenuSaveGame* Save = Cast<UTMOPMenuSaveGame>(UGameplayStatics::CreateSaveGameObject(UTMOPMenuSaveGame::StaticClass()));
    if (!IsValid(Save)) return FReply::Handled();
    Save->PlayerTransform = PlayerCharacter->GetActorTransform();
    if (UTMOPClockSubsystem* Clock = PlayerCharacter->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()) Save->SavedTime = Clock->GetCurrentTime();
    if (IsValid(PlayerCharacter->Inventory))
        for (const FTMOPInventoryEntry& Entry : PlayerCharacter->Inventory->Items)
            if (IsValid(Entry.Item)) { Save->InventoryItemPaths.Add(FSoftObjectPath(Entry.Item->GetPathName())); Save->InventoryQuantities.Add(Entry.Quantity); }
    if (IsValid(PlayerCharacter->Inventory) && IsValid(PlayerCharacter->Inventory->EquippedItem)) Save->EquippedItemPath = FSoftObjectPath(PlayerCharacter->Inventory->EquippedItem->GetPathName());
    Save->DiscoveredEvidenceIds = PlayerCharacter->DiscoveredEvidenceIds;
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
    SetStatus(bSaved ? NSLOCTEXT("TMOP", "SaveSuccess", "Spelet sparades.") : NSLOCTEXT("TMOP", "SaveFailed", "Kunde inte spara spelet."));
    if (bSaved) OnSaveRequested.Broadcast(); return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleLoadClicked()
{
    UTMOPMenuSaveGame* Save = Cast<UTMOPMenuSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName,0));
    if (!IsValid(Save) || !IsValid(PlayerCharacter)) { SetStatus(NSLOCTEXT("TMOP", "NoSave", "Ingen giltig sparfil hittades.")); return FReply::Handled(); }
    bool bTimeLoaded = false;
    for (TActorIterator<ATMOPSimulationDebugDirector> It(GetWorld()); It; ++It) { bTimeLoaded = It->JumpToSimulationTime(Save->SavedTime); break; }
    if (!bTimeLoaded) { SetStatus(NSLOCTEXT("TMOP", "LoadNeedsDirector", "Laddning kräver TMOPSimulationDebugDirector i nivån.")); return FReply::Handled(); }
    PlayerCharacter->SetActorTransform(Save->PlayerTransform, false, nullptr, ETeleportType::TeleportPhysics);
    if (IsValid(PlayerCharacter->Inventory))
    {
        const TArray<FTMOPInventoryEntry> Existing = PlayerCharacter->Inventory->Items;
        for (const FTMOPInventoryEntry& Entry : Existing) if (IsValid(Entry.Item)) PlayerCharacter->Inventory->RemoveItem(Entry.Item, Entry.Quantity);
        for (int32 I=0; I<Save->InventoryItemPaths.Num(); ++I)
            if (UTMOPItemDefinition* Item = Cast<UTMOPItemDefinition>(Save->InventoryItemPaths[I].TryLoad())) PlayerCharacter->Inventory->AddItem(Item, Save->InventoryQuantities.IsValidIndex(I)?Save->InventoryQuantities[I]:1);
        if (UTMOPItemDefinition* Equipped = Cast<UTMOPItemDefinition>(Save->EquippedItemPath.TryLoad())) PlayerCharacter->Inventory->EquipItem(Equipped);
    }
    PlayerCharacter->DiscoveredEvidenceIds = Save->DiscoveredEvidenceIds;
    SetStatus(NSLOCTEXT("TMOP", "LoadSuccess", "Spelet laddades.")); OnLoadRequested.Broadcast(); return FReply::Handled();
}

void UTMOPPauseMenuWidget::BuildQuitPage()
{
    AddBody(NSLOCTEXT("TMOP", "QuitWarning", "Vill du avsluta spelet? Osparade framsteg försvinner."));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,8.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "ConfirmQuit", "AVSLUTA SPELET")).OnClicked_UObject(this,&UTMOPPauseMenuWidget::HandleQuitClicked) ];
}

FReply UTMOPPauseMenuWidget::HandleQuitClicked()
{ if (IsValid(PlayerController)) UKismetSystemLibrary::QuitGame(this,PlayerController,EQuitPreference::Quit,false); return FReply::Handled(); }

void UTMOPPauseMenuWidget::BuildMoveInTimePage()
{
    AddBody(NSLOCTEXT("TMOP", "MoveTimeInstructions", "Skriv HH:MM eller HH:MM:SS. Endast 23:00:00–23:45:00 godtas. Världen byggs om till det valda klockslaget."));
    FString Current = TEXT("23:00:00");
    if (IsValid(PlayerCharacter)) if (UTMOPClockSubsystem* Clock = PlayerCharacter->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()) Current = Clock->GetCurrentTime().ToDisplayString();
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,8.0f)[ SAssignNew(TimeEntryBox,SEditableTextBox).Text(FText::FromString(Current)).HintText(FText::FromString(TEXT("23:21:30"))) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,5.0f)[ SNew(SButton).Text(NSLOCTEXT("TMOP", "ApplyMoveTime", "FLYTTA TILL KLOCKSLAGET")).OnClicked_UObject(this,&UTMOPPauseMenuWidget::HandleMoveInTimeClicked) ];
}

FReply UTMOPPauseMenuWidget::HandleMoveInTimeClicked()
{
    if (!TimeEntryBox.IsValid()) return FReply::Handled();
    TArray<FString> Parts; TimeEntryBox->GetText().ToString().ParseIntoArray(Parts,TEXT(":"),false);
    int32 H=0,M=0,S=0;
    const bool bParsed = (Parts.Num()==2 || Parts.Num()==3) && LexTryParseString(H,*Parts[0]) && LexTryParseString(M,*Parts[1]) && (Parts.Num()==2 || LexTryParseString(S,*Parts[2]));
    const bool bRange = bParsed && H==23 && M>=0 && M<=45 && S>=0 && S<=59 && !(M==45 && S>0);
    if (!bRange) { SetStatus(NSLOCTEXT("TMOP", "InvalidMoveTime", "Ogiltig tid. Använd exempelvis 23:21:30 inom intervallet 23:00–23:45.")); return FReply::Handled(); }
    bool bMoved=false; for (TActorIterator<ATMOPSimulationDebugDirector> It(GetWorld()); It; ++It) { bMoved=It->JumpToSimulationTime(FTMOPTime(H,M,S)); break; }
    SetStatus(bMoved ? NSLOCTEXT("TMOP", "MoveTimeSuccess", "Världen flyttades till det nya klockslaget.") : NSLOCTEXT("TMOP", "MoveTimeFailed", "Ingen TMOPSimulationDebugDirector hittades, eller tiden avvisades."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleResumeClicked()
{ if (IsValid(PlayerCharacter)) PlayerCharacter->SetPauseMenuOpen(false); return FReply::Handled(); }

FReply UTMOPPauseMenuWidget::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
    const FKey Key=Event.GetKey();
    if (Key==EKeys::Enter || Key==EKeys::Escape ||
        Key==EKeys::Gamepad_Special_Right || Key==EKeys::Gamepad_FaceButton_Right)
        return HandleResumeClicked();
    return Super::NativeOnKeyDown(Geometry,Event);
}

void UTMOPPauseMenuWidget::SetStatus(const FText& Text)
{ if (StatusText.IsValid()) StatusText->SetText(Text); }
