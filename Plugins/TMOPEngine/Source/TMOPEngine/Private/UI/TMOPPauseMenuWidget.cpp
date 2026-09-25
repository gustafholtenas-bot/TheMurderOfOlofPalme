#include "UI/TMOPPauseMenuWidget.h"
#include "WorldAtlas/STMOPWorldAtlas.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/SNullWidget.h"
#include "Localization/TMOPLocalization.h"
#include "UI/TMOPLanguageSelector.h"
#include "UI/STMOPTheoryBuilder.h"
#include "UI/STMOPNotebookPanel.h"
#include "Observations/TMOPNotebookPresentation.h"
#include "UI/TMOPLocalPanel.h"
#include "UI/TMOPControlsPanel.h"
#include "UI/TMOPPlayerAppearancePanel.h"
#include "UI/TMOPControlUIHelpers.h"

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
#include "Observations/TMOPObservationDirector.h"
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
#include "UI/TMOPSaveGameService.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "World/TMOPFindingActor.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "UI/TMOPTypographyDirector.h"

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
    case ETMOPPauseHubSection::Inventory: return NSLOCTEXT("TMOP", "HubInventory", "INVENTARIE");
    case ETMOPPauseHubSection::Evidence: return NSLOCTEXT("TMOP", "HubEvidence", "ANTECKNINGSBOK / FYND");
    case ETMOPPauseHubSection::Sources: return NSLOCTEXT("TMOP", "HubSources", "KÄLLOR / UPPSLAG");
    case ETMOPPauseHubSection::MyObservations: return NSLOCTEXT("TMOP", "HubMyObservations", "MINA OBSERVATIONER");
    case ETMOPPauseHubSection::MurderKnowledge: return NSLOCTEXT("TMOP", "HubMurderKnowledge", "VETSKAPER OM MORDET");
    case ETMOPPauseHubSection::Publications: return NSLOCTEXT("TMOP", "HubPublications", "TIDNINGAR");
    case ETMOPPauseHubSection::Map: return NSLOCTEXT("TMOP", "HubMap", "KARTA");
    case ETMOPPauseHubSection::Settings: return NSLOCTEXT("TMOP", "HubSettings", "INSTÄLLNINGAR");
    case ETMOPPauseHubSection::Controls: return NSLOCTEXT("TMOP", "HubControls", "KONTROLLINSTÄLLNINGAR");
    case ETMOPPauseHubSection::SaveLoad: return NSLOCTEXT("TMOP", "HubSaveLoad", "SPARA/LADDA");
    case ETMOPPauseHubSection::Quit: return NSLOCTEXT("TMOP", "HubQuit", "AVSLUTA SPELET");
    case ETMOPPauseHubSection::MoveInTime: return NSLOCTEXT("TMOP", "HubMoveTime", "FÖRFLYTTA I TID");
    case ETMOPPauseHubSection::TheoryBuilder:
    case ETMOPPauseHubSection::Theories: return NSLOCTEXT("TMOP", "HubTheories", "MINA TEORIER");
    case ETMOPPauseHubSection::MurderDayMysteries: return NSLOCTEXT("TMOP", "HubMysteries", "MYSTERIER PÅ MORDDAGEN");
    case ETMOPPauseHubSection::AfterMurderEvents: return NSLOCTEXT("TMOP", "HubAfterMurder", "HÄNDELSER EFTER MORDET");
    case ETMOPPauseHubSection::WorldGroups: return NSLOCTEXT("TMOP", "HubWorldGroups", "GRUPPERINGAR I VÄRLDEN");
    case ETMOPPauseHubSection::SwedenGroups: return NSLOCTEXT("TMOP", "HubSwedenGroups", "GRUPPERINGAR I SVERIGE");
    default: return FText::GetEmpty();
    }
}
}

UTMOPPauseMenuWidget::UTMOPPauseMenuWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
    WorldGlobeMesh = Sphere.Object;
}

void UTMOPPauseMenuWidget::ReleaseSlateResources(bool bReleaseChildren)
{
    Super::ReleaseSlateResources(bReleaseChildren);
    ContentBox.Reset(); PageContentHost.Reset(); SectionTitleText.Reset(); StatusText.Reset(); TimeEntryBox.Reset();
}

void UTMOPPauseMenuWidget::InitializePauseMenu(APlayerController* InController,
    ATMOPPlayerCharacter* InCharacter)
{
    PlayerController = InController;
    PlayerCharacter = InCharacter;
    SetIsFocusable(true);
    // Use the owning player's full viewport, not a centered desired-size slot.
    SetAnchorsInViewport(FAnchors(0.0f, 0.0f, 1.0f, 1.0f));
    SetAlignmentInViewport(FVector2D::ZeroVector);
    SetPositionInViewport(FVector2D::ZeroVector, false);
    SetDesiredSizeInViewport(FVector2D::ZeroVector);
}

void UTMOPPauseMenuWidget::SetMenuVisible(const bool bVisible)
{
    SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    if (bVisible)
    {
        SetStatus(FText::GetEmpty());
        ShowSection(CurrentSection);
    }
    else if (CurrentSection == ETMOPPauseHubSection::WorldGroups && PageContentHost.IsValid())
    {
        PageContentHost->SetContent(SNullWidget::NullWidget);
        ContentBox.Reset(); // release the preview scene and its render target on close
    }
}

TSharedRef<SWidget> UTMOPPauseMenuWidget::RebuildWidget()
{
    const FTMOPMenuColorPalette MenuColors =
        ATMOPTypographyDirector::ResolveMenuColors(this);
    auto MakeNavigationButton = [this, &MenuColors](const FText& Label,
        const ETMOPPauseHubSection Section)
    {
        return SNew(SButton)
            .HAlign(HAlign_Left)
            .ButtonColorAndOpacity(MenuColors.ButtonBackground)
            .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleSectionClicked, Section)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(Label))
              .Font(ATMOPTypographyDirector::ResolveFont(this,
                  TEXT("PauseMenuNavigation"),
                  FCoreStyle::GetDefaultFontStyle("Regular", 16)))
              .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                  TEXT("PauseMenuNavigation"),
                  MenuColors.PauseMenuButtonText)) ];
    };
    TSharedRef<SVerticalBox> NavigationPanel = SNew(SVerticalBox);
    auto AddNavigationEntry = [&NavigationPanel, &MakeNavigationButton](
        const ETMOPPauseHubSection Section)
    {
        NavigationPanel->AddSlot().AutoHeight().Padding(3.0f)
        [ MakeNavigationButton(SectionTitle(Section), Section) ];
    };
    auto AddNavigationGap = [&NavigationPanel](float Height)
    {
        NavigationPanel->AddSlot().AutoHeight()
        [ SNew(SSpacer).Size(FVector2D(1.0f, Height)) ];
    };
    AddNavigationEntry(ETMOPPauseHubSection::Inventory);
    AddNavigationEntry(ETMOPPauseHubSection::Publications);
    AddNavigationEntry(ETMOPPauseHubSection::Map);
    AddNavigationGap(20.0f);
    AddNavigationEntry(ETMOPPauseHubSection::MyObservations);
    AddNavigationEntry(ETMOPPauseHubSection::Theories);
    AddNavigationGap(20.0f);
    AddNavigationEntry(ETMOPPauseHubSection::MurderDayMysteries);
    AddNavigationEntry(ETMOPPauseHubSection::MurderKnowledge);
    AddNavigationEntry(ETMOPPauseHubSection::AfterMurderEvents);
    AddNavigationGap(20.0f);
    AddNavigationEntry(ETMOPPauseHubSection::WorldGroups);
    AddNavigationEntry(ETMOPPauseHubSection::SwedenGroups);
    AddNavigationGap(64.0f);
    AddNavigationEntry(ETMOPPauseHubSection::Sources);
    AddNavigationEntry(ETMOPPauseHubSection::Settings);
    AddNavigationEntry(ETMOPPauseHubSection::Controls);
    AddNavigationEntry(ETMOPPauseHubSection::SaveLoad);
    AddNavigationEntry(ETMOPPauseHubSection::Quit);
    // Scrollable navigation keeps these larger reference-image gaps usable
    // in small windows and local multiplayer viewports.
    AddNavigationGap(76.0f);
    NavigationPanel->AddSlot().AutoHeight().Padding(3.0f, 12.0f, 3.0f, 3.0f)
    [ MakeNavigationButton(SectionTitle(ETMOPPauseHubSection::MoveInTime),
        ETMOPPauseHubSection::MoveInTime) ];
    AddNavigationGap(32.0f);
    NavigationPanel->AddSlot().AutoHeight().Padding(3.0f)
    [ SNew(SButton).HAlign(HAlign_Left).ButtonColorAndOpacity(MenuColors.ButtonBackground)
      .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleResumeClicked)
      [ SNew(STextBlock).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "HubResume", "FORTSÄTT SPELA")))
        .Font(ATMOPTypographyDirector::ResolveFont(this,
            TEXT("PauseMenuNavigation"),
            FCoreStyle::GetDefaultFontStyle("Regular", 16)))
        .ColorAndOpacity(MenuColors.PauseMenuButtonText) ] ];

    TSharedRef<SVerticalBox> PagePanel = SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 14.0f)
        [ SAssignNew(SectionTitleText, STextBlock).Font(
            ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuSectionTitle"),
                FCoreStyle::GetDefaultFontStyle("Bold", 21)))
          .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
              TEXT("PauseMenuSectionTitle"), MenuColors.AccentText)) ]
        + SVerticalBox::Slot().FillHeight(1.0f)
        [ SAssignNew(PageContentHost, SBox).Clipping(EWidgetClipping::ClipToBounds) ]
        + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 14.0f, 0.0f, 0.0f)
        [ SAssignNew(StatusText, STextBlock)
          .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuStatus"),
              FCoreStyle::GetDefaultFontStyle("Regular", 14)))
          .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
              TEXT("PauseMenuStatus"), MenuColors.StatusText)) ];

    TSharedRef<SWidget> RootWidget = SNew(SBorder)
        .BorderBackgroundColor(MenuColors.MenuBackground).Padding(34.0f)
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f, 8.0f, 20.0f)
          [ SNew(STextBlock).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "PauseHubTitle", "THE MURDER OF OLOF PALME")))
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuMainTitle"),
                FCoreStyle::GetDefaultFontStyle("Bold", 25)))
            .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                TEXT("PauseMenuMainTitle"), FLinearColor::White)) ]
          + SVerticalBox::Slot().FillHeight(1.0f)
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 24.0f, 0.0f)
            [ SNew(SBox).WidthOverride(340.0f)
              [ SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(3.0f, 0.0f, 3.0f, 12.0f)
                [ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "PinnedResume", "FORTSÄTT SPELA / STÄNG")))
                  .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleResumeClicked) ]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [ SNew(SScrollBox) + SScrollBox::Slot()[NavigationPanel] ] ] ]
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [ SNew(SBorder).BorderBackgroundColor(MenuColors.PanelBackground)
              .Padding(24.0f)[PagePanel] ] ] ];
    ShowSection(CurrentSection);
    // Expand to the local viewport instead of leaving a fixed-size panel at the top left.
    // Keep the content bounded so wide source cards cannot push navigation off-screen.
    return TMOPFillLocalPanel(this, RootWidget);
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
    if (!PageContentHost.IsValid()) return;
    ContentBox = SNew(SVerticalBox);
    PageContentHost->SetContent(SNew(SScrollBox) + SScrollBox::Slot()[ContentBox.ToSharedRef()]);
    if (SectionTitleText.IsValid()) SectionTitleText->SetVisibility(
        Section == ETMOPPauseHubSection::MyObservations ? EVisibility::Collapsed : EVisibility::Visible);
    if (SectionTitleText.IsValid()) SectionTitleText->SetText(FTMOPLocalization::Text(SectionTitle(Section)));
    SetStatus(FText::GetEmpty());
    switch (Section)
    {
    case ETMOPPauseHubSection::Map: BuildMapPage(); break;
    case ETMOPPauseHubSection::Inventory: BuildInventoryPage(); break;
    case ETMOPPauseHubSection::Evidence: BuildEvidencePage(); break;
    case ETMOPPauseHubSection::Sources: BuildSourcesPage(); break;
    case ETMOPPauseHubSection::Publications: BuildPublicationsPage(); break;
    case ETMOPPauseHubSection::Settings: BuildSettingsPage(); break;
    case ETMOPPauseHubSection::Controls: BuildControlsPage(); break;
    case ETMOPPauseHubSection::SaveLoad: BuildSaveLoadPage(); break;
    case ETMOPPauseHubSection::Quit: BuildQuitPage(); break;
    case ETMOPPauseHubSection::MoveInTime: BuildMoveInTimePage(); break;
    case ETMOPPauseHubSection::MyObservations:
        BuildNotebookPage();
        break;
    case ETMOPPauseHubSection::TheoryBuilder:
    case ETMOPPauseHubSection::Theories: BuildTheoryBuilderPage(); break;
    case ETMOPPauseHubSection::MurderKnowledge: BuildChronologyPage(MurderKnowledgeTable, true); break;
    case ETMOPPauseHubSection::AfterMurderEvents: BuildChronologyPage(AfterMurderEventsTable, false); break;
    case ETMOPPauseHubSection::MurderDayMysteries: BuildChronologyPage(MurderDayMysteriesTable, false, true); break;
    case ETMOPPauseHubSection::WorldGroups:
        // This page manages its own scrolling. Fill the available local-player panel.
        PageContentHost->SetContent(MakeTMOPWorldAtlas(WorldGlobeMesh, WorldGlobeMaterial,
            WorldGlobeAlignment, bWorldGlobeCoastlines));
        break;
    case ETMOPPauseHubSection::SwedenGroups:
        // Independent pages reserved for content specified later.
        break;
    }
}

void UTMOPPauseMenuWidget::AddHeading(const FText& Text)
{ ContentBox->AddSlot().AutoHeight().Padding(2.0f, 10.0f)[ SNew(STextBlock).Text(FTMOPLocalization::Text(Text)).Font(
    ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuSectionHeading"),
        FCoreStyle::GetDefaultFontStyle("Bold", 17)))
    .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
        TEXT("PauseMenuSectionHeading"), FLinearColor::White)) ]; }
void UTMOPPauseMenuWidget::AddBody(const FText& Text)
{ ContentBox->AddSlot().AutoHeight().Padding(2.0f, 5.0f)[ SNew(STextBlock).Text(FTMOPLocalization::Text(Text))
    .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuBody"),
        FCoreStyle::GetDefaultFontStyle("Regular", 16)))
    .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
        TEXT("PauseMenuBody"), FLinearColor::White)).AutoWrapText(true) ]; }

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
        const FText Label = FTMOPLocalization::Format(NSLOCTEXT("TMOP", "InventoryLine", "{0}  ×{1}"),
            Item->DisplayName, FText::AsNumber(Entry.Quantity));
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 4.0f)
        [ SNew(SButton).Text(FTMOPLocalization::Text(Label)).IsEnabled(Item->bCanEquip)
          .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleEquipItem, Item) ];
    }
}

FReply UTMOPPauseMenuWidget::HandleEquipItem(UTMOPItemDefinition* Item)
{
    if (IsValid(PlayerCharacter) && IsValid(PlayerCharacter->Inventory) &&
        PlayerCharacter->Inventory->EquipItem(Item))
        SetStatus(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "EquippedItem", "Vald: {0}"), Item->DisplayName));
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

void UTMOPPauseMenuWidget::BuildNotebookPage()
{
    // Upgrade only already collected entries; the notebook never grants new discoveries.
    if (IsValid(PlayerCharacter))
        for (auto& Entry : PlayerCharacter->NotebookObservations)
            if (Entry.PresentationVersion == 0)
            {
                AActor* Actor = nullptr;
                if (Entry.Kind == ETMOPNotebookEntityKind::Vehicle)
                    for (TActorIterator<ATMOPVehicleBase> It(GetWorld()); It; ++It)
                        if (It->VehicleId == Entry.EntityId) { Actor = *It; break; }
                FTMOPNotebookPresentation::Populate(Entry, GetWorld(), Actor, nullptr, false);
            }
    if (PageContentHost.IsValid())
        PageContentHost->SetContent(SNew(STMOPNotebookPanel).Player(PlayerCharacter.Get()));
}

void UTMOPPauseMenuWidget::BuildTheoryBuilderPage()
{
    ContentBox->AddSlot().AutoHeight()
    [SNew(STMOPTheoryBuilder).Player(PlayerCharacter.Get())
        .OnSave(FOnClicked::CreateUObject(this, &UTMOPPauseMenuWidget::HandleCreateNewSaveClicked))];
    AddHeading(NSLOCTEXT("TMOP", "TheoryInformation", "INFORMATION"));
    TArray<FTMOPTheoryInformationRow> ViewStorage;
    TArray<FTMOPTheoryInformationRow*> Rows;
    UDataTable* Table = TheoryInformationTable;
    if (!IsValid(Table))
        Table = LoadObject<UDataTable>(nullptr,
            TEXT("/Game/TMOP/Data/DT_TMOP_TheoryInformation.DT_TMOP_TheoryInformation"));
    if (IsValid(Table))
    {
        if (Table->GetRowStruct() != FTMOPTheoryInformationRow::StaticStruct())
        {
            AddBody(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.e31d800a1621d2b9", "Teorilistan har fel radtyp. Importera den som TMOPTheoryInformationRow."));
            return;
        }
        FTMOPLocalization::TableViews(Table, ViewStorage, Rows);
    }
    else for (auto& Entry : TheoryInformationEntries) Rows.Add(&Entry);
    Rows.Sort([](const FTMOPTheoryInformationRow& A, const FTMOPTheoryInformationRow& B)
    { return A.SortOrder == B.SortOrder ? FTMOPLocalization::String(A.Title) < FTMOPLocalization::String(B.Title) : A.SortOrder < B.SortOrder; });
    for (int32 Track = 0; Track < 2; ++Track)
    {
        AddHeading(Track == 0 ? NSLOCTEXT("TMOP", "TheoryLoneGunman", "ENSAM GÄRNINGSMAN")
            : NSLOCTEXT("TMOP", "TheoryConspiracy", "KONSPIRATION"));
        bool bAny = false;
        for (const auto* Row : Rows)
        {
            if (static_cast<int32>(Row->Track) != Track || Row->Title.IsEmpty()) continue;
            bAny = true;
            const FText Body = Row->Body.IsEmpty() ? NSLOCTEXT("TMOP", "TheoryMissingText", "Texten är inte inlagd ännu.") : Row->Body;
            ContentBox->AddSlot().AutoHeight().Padding(0, 3)
            [SNew(SExpandableArea).InitiallyCollapsed(true)
                .HeaderContent()[SNew(STextBlock).Text(FTMOPLocalization::Text(Row->Title)).AutoWrapText(true)]
                .BodyContent()[SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(12, 8)[SNew(STextBlock).Text(FTMOPLocalization::Text(Body)).AutoWrapText(true)]
                    + SVerticalBox::Slot().AutoHeight().Padding(12, 0, 12, 8)
                    [SNew(STextBlock).Text(FTMOPLocalization::Text(Row->Source)).AutoWrapText(true)
                        .Visibility(Row->Source.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)]]];
        }
        if (!bAny) AddBody(NSLOCTEXT("TMOP", "TheoryNoInformation", "Inga uppgifter tillagda ännu."));
    }
}

void UTMOPPauseMenuWidget::BuildChronologyPage(UDataTable* Table, bool bKnowledge, bool bMysteries)
{
    // Optional conventional paths let the native menu work without a widget Blueprint.
    if (!IsValid(Table))
        Table = LoadObject<UDataTable>(nullptr, bMysteries
            ? TEXT("/Game/TMOP/Data/DT_TMOP_MurderDayMysteries.DT_TMOP_MurderDayMysteries")
            : bKnowledge
            ? TEXT("/Game/TMOP/Data/DT_TMOP_MurderKnowledge.DT_TMOP_MurderKnowledge")
            : TEXT("/Game/TMOP/Data/DT_TMOP_AfterMurderEvents.DT_TMOP_AfterMurderEvents"));
    if (!IsValid(Table) || Table->GetRowStruct() != FTMOPChronologyRow::StaticStruct())
    {
        AddBody(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.c8e627fe249280ba", "Ingen kronologilista är vald. Importera JSON som TMOPChronologyRow och välj tabellen under Pause → Chronology."));
        return;
    }
    TArray<FTMOPChronologyRow> ViewStorage;
    TArray<FTMOPChronologyRow*> Rows;
    FTMOPLocalization::TableViews(Table, ViewStorage, Rows);
    Rows.RemoveAll([](const auto* R) { return !R || !R->bPublished || R->Title.IsEmpty(); });
    const auto DateKey = [](const FTMOPChronologyRow& R) -> int64
    {
        const int64 Y = R.Year > 0 ? R.Year : 999999;
        return Y * 10000 + (R.Month > 0 ? R.Month : 13) * 100 + (R.Day > 0 ? R.Day : 32);
    };
    Rows.StableSort([bKnowledge, &DateKey](const FTMOPChronologyRow& A, const FTMOPChronologyRow& B)
    {
        if (bKnowledge && A.bBeforeMurder != B.bBeforeMurder) return A.bBeforeMurder;
        if (DateKey(A) != DateKey(B)) return DateKey(A) < DateKey(B);
        if (A.SortOrder != B.SortOrder) return A.SortOrder < B.SortOrder;
        return FTMOPLocalization::String(A.Title) < FTMOPLocalization::String(B.Title);
    });
    FString PreviousGroup;
    for (const auto* R : Rows)
    {
        const FString Period = bKnowledge ? (R->bBeforeMurder ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.a28d788facf2998a", "INNAN MORDET").ToString() : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.e518cc84531cc8aa", "EFTER MORDET").ToString()) : TEXT("");
        const FString Year = R->Year > 0 ? FString::FromInt(R->Year) : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.568fca539d6395d1", "DATUM EJ FASTSTÄLLT").ToString();
        const FString Group = Period + TEXT(" ") + Year;
        if (Group != PreviousGroup)
        {
            AddHeading(FText::FromString(Group.TrimStartAndEnd()));
            PreviousGroup = Group;
        }
        FString Date = R->Year > 0 ? FString::FromInt(R->Year) : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.fa7a332763e21821", "Datum ej fastställt").ToString();
        if (R->Year > 0 && R->Month > 0) Date += FString::Printf(TEXT("-%02d"), R->Month);
        if (R->Year > 0 && R->Month > 0 && R->Day > 0) Date += FString::Printf(TEXT("-%02d"), R->Day);
        FString Details = Date;
        if (!R->DateNote.IsEmpty()) Details += TEXT(" — ") + FTMOPLocalization::String(R->DateNote);
        if (!R->EvidenceStatus.IsEmpty()) Details += TEXT("\n") + FTMOPLocalization::String(R->EvidenceStatus);
        Details += TEXT("\n\n") + FTMOPLocalization::String(R->Body);
        if (!R->Source.IsEmpty()) Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.287df351360d3405", "\n\nKälla: ").ToString() + FTMOPLocalization::String(R->Source);
        if (!R->SourceUrl.IsEmpty()) Details += TEXT("\n") + R->SourceUrl;
        ContentBox->AddSlot().AutoHeight().Padding(0, 5)
        [ SNew(SExpandableArea).InitiallyCollapsed(true)
            .HeaderContent()[SNew(STextBlock).Text(FTMOPLocalization::Text(R->Title)).AutoWrapText(true)
                .Font(FCoreStyle::GetDefaultFontStyle("Regular", 18)).ColorAndOpacity(FLinearColor::White)]
            .BodyContent()[SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(Details))).AutoWrapText(true)
                .Margin(FMargin(14, 10)).Font(FCoreStyle::GetDefaultFontStyle("Regular", 16))
                .ColorAndOpacity(FLinearColor::White)] ];
    }
    if (Rows.IsEmpty()) AddBody(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.02bb4b16ea1039d0", "Inga publicerade poster ännu."));
}

void UTMOPPauseMenuWidget::BuildSourcesPage()
{
    if (!IsValid(UppslagTable))
    {
        AddBody(NSLOCTEXT("TMOP", "NoUppslagTable",
            "Ingen uppslagstabell är vald. Ange DT_TMOP_Uppslag_REGISTER i Class Defaults → TMOP → UI → Pause → Sources."));
        return;
    }

    TArray<FTMOPUppslagRow> ViewStorage;
    TArray<FTMOPUppslagRow*> Rows;
    FTMOPLocalization::TableViews(UppslagTable, ViewStorage, Rows);
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
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(Label)) ];
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
                    FTMOPLocalization::String(Row->SectionDescription).IsEmpty()
                    ? FTMOPLocalization::String(Row->Title) : FTMOPLocalization::String(Row->SectionDescription);
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
        FString Result = FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.c67f135591e01ece", "{0} uppslag inlagda\n{1} tillgängliga online men ej inlagda\n{2} ej utlämnade från polisen\n{3} ej utlämnade men av stort intresse\n{4} procent inlagt"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.Added)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.OnlineNotAdded)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.PoliceOnly)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.PoliceHighPriority)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.AddedPercent()))).ToString();
        if (Coverage.Unknown > 0)
            Result += FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1e83c03507ba0f63", "\n{0} ej klassificerade"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.Unknown))).ToString();
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
        return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.d6dd356100946f09", "Avsnitt {0} – beskrivning saknas i registret."), FTMOPLocalization::Text(FString(SeriesId.ToString()))).ToString();
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
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(ButtonLabel))
              .Font(FCoreStyle::GetDefaultFontStyle("Bold", Card.bMainSection ? 30 : 24)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 3.0f, 0.0f, 8.0f)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(Card.Description)))
              .AutoWrapText(true).ColorAndOpacity(FLinearColor(0.78f, 0.80f, 0.84f, 1.0f)) ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STMOPUppslagCoverageBar).EntryStates(States).DesiredWidth(420.0f) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 8.0f, 0.0f, 0.0f)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(StatisticsForCoverage(Coverage))))
              .AutoWrapText(true) ];
        if (Card.bMainSection)
            return SNew(SBox).HeightOverride(220.0f)
                [ SNew(SButton).ContentPadding(FMargin(14.0f))
                  .OnClicked_UObject(this,
                      &UTMOPPauseMenuWidget::HandleSourceMainSectionClicked, Card.Id)
                  [ CardContent ] ];
        return SNew(SBox).HeightOverride(220.0f)
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
                    ? SourceRow->UppslagId.ToString() : FTMOPLocalization::String(SourceRow->Title);
                FString Details = SourceRow->bAddedToProject ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.6265ce0096115cb3", "Inlagt i projektet").ToString()
                    : SourceRow->bPartiallyAdded ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.8b0c5f8dcfc057b5", "Delvis inlagt").ToString()
                    : SourceRow->bRetrieved ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1cdd033700dd1dd7", "Genomgången").ToString() : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1c53a8da7f5916ca", "Inte genomgången").ToString();
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
    [ SNew(SButton).Text(FTMOPLocalization::Text(SelectedSourceSeries.IsNone()
        ? NSLOCTEXT("TMOP", "BackToMainSections", "← Alla huvudavsnitt")
        : FTMOPLocalization::Format(NSLOCTEXT("TMOP", "BackToMainSection", "← Tillbaka till {0}"),
            FText::FromName(SelectedSourceMainSection))))
      .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleSourceBackClicked) ];

    if (SelectedSourceSeries.IsNone())
    {
        AddHeading(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MainSectionHeading", "{0} – {1}"),
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
          [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(StatisticsForCoverage(MainCoverage)))) ] ];

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
                ? FTMOPLocalization::Format(NSLOCTEXT("TMOP", "MainSectionOther", "{0} – ÖVRIGA"),
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
    AddHeading(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "SelectedSeriesHeading", "AVSNITT {0}"),
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
        const FString Title = Row->Title.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.318cee16bf3d25ac", "Utan titel").ToString() : FTMOPLocalization::String(Row->Title);
        const FLinearColor RowColor = ClassifyUppslagCoverage(*Row) ==
            ETMOPCoverageState::Unknown
            ? FLinearColor(0.78f, 0.80f, 0.84f, 1.0f)
            : CoverageStateColor(ClassifyUppslagCoverage(*Row));
        const FText RowHeading = FText::FromString(FString::Printf(TEXT("%s — %s"),
            *Row->UppslagId.ToString(), *Title));
        FString Details = Row->bAddedToProject ? TEXT("Inlagt")
            : Row->bPartiallyAdded ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.8b0c5f8dcfc057b5", "Delvis inlagt").ToString() : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.e00f2a51a946330f", "Ej inlagt").ToString();
        if (!Row->DocumentDate.IsEmpty()) Details += TEXT(" • ") + Row->DocumentDate;
        if (!Row->SourceUrl.IsEmpty()) Details += TEXT("\n") + Row->SourceUrl;
        // One source is one color block: title, state, date and URL all use the
        // exact same classification as the coverage bar and legend above.
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 8.0f, 2.0f, 5.0f)
        [ SNew(SVerticalBox)
          + SVerticalBox::Slot().AutoHeight()
          [ SNew(STextBlock).Text(FTMOPLocalization::Text(RowHeading)).ColorAndOpacity(RowColor)
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuSourceHeading"),
                FCoreStyle::GetDefaultFontStyle("Bold", 17))) ]
          + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 5.0f, 0.0f, 0.0f)
          [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(Details)))
            .ColorAndOpacity(RowColor).AutoWrapText(true)
            .Font(ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuSourceDetails"),
                FCoreStyle::GetDefaultFontStyle("Regular", 16))) ] ];
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

    FString TotalStatistics = FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.c0e187fb172a654a", "TOTALT: {0} uppslag\n{1} uppslag inlagda\n{2} tillgängliga online men ej inlagda\n{3} ej utlämnade från polisen\n{4} ej utlämnade från polisen men av stort intresse för spelet\n{5} procent inlagt"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.Total)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.Added)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.OnlineNotAdded)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.PoliceOnly)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.PoliceHighPriority)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.AddedPercent()))).ToString();
    if (TotalCoverage.Unknown > 0)
        TotalStatistics += FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1830e563a39ed943", "\n{0} med ännu ej fastställd status"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), TotalCoverage.Unknown))).ToString();
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 4.0f, 2.0f, 14.0f)
    [ SNew(SBorder).BorderBackgroundColor(FLinearColor(0.055f, 0.07f, 0.095f, 1.0f))
      .Padding(12.0f)
      [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(TotalStatistics)))
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
                return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.803e3793d0222799", "Delavsnitt inom {0}: {1}"), FTMOPLocalization::Text(FString(ParentId)), FTMOPLocalization::Text(FString(ParentDescription))).ToString();
        }
        return FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.627078f31fa8db6a", "Avsnitt {0} – detaljerad beskrivning saknas i registret."), FTMOPLocalization::Text(FString(SeriesId.ToString()))).ToString();
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
        FString StatisticsString = FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.b1df6fd737d3e146", "{0} uppslag inlagda\n{1} tillgängliga online men ej inlagda\n{2} ej utlämnade från polisen\n{3} ej utlämnade från polisen men av stort intresse för spelet\n{4} procent inlagt"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.Added)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.OnlineNotAdded)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.PoliceOnly)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.PoliceHighPriority)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.AddedPercent()))).ToString();
        if (Coverage.Unknown > 0)
            StatisticsString += FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1e83c03507ba0f63", "\n{0} ej klassificerade"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), Coverage.Unknown))).ToString();
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
              [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromName(SeriesId)))
                .Font(FCoreStyle::GetDefaultFontStyle("Bold", 28)) ] ]
            + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(
                ResolveSectionDescriptionLegacy(SeriesId)))).AutoWrapText(true) ] ]
          + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 7.0f, 0.0f, 0.0f)
          [ SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(105.0f, 0.0f, 18.0f, 0.0f)
              .VAlign(VAlign_Center)
            [ SNew(STMOPUppslagCoverageBar).EntryStates(EntryStates)
              .DesiredWidth(BarWidth) ]
            + SHorizontalBox::Slot().FillWidth(1.0f)[ SNew(SSpacer) ]
            + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
            [ SNew(SBox).WidthOverride(360.0f)
              [ SNew(STextBlock).Text(FTMOPLocalization::Text(Statistics)).AutoWrapText(false) ] ] ] ];
    }

    AddHeading(NSLOCTEXT("TMOP", "SourcesDetails", "Polisuppslag – detaljer"));

    int32 VisibleCount = 0;
    constexpr int32 MaximumVisibleRows = 300;
    for (const FTMOPUppslagRow* Row : Rows)
    {
        if (Row == nullptr || Row->SourceCategory != ETMOPSourceCategory::PoliceUppslag ||
            Row->bIsSectionDefinition || !Row->bRelevantToGame) continue;
        const FString SourceDisplayTitle = Row->Title.IsEmpty() ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.318cee16bf3d25ac", "Utan titel").ToString() : FTMOPLocalization::String(Row->Title);
        const FString ProcessingStateText = Row->bAddedToProject ? TEXT("Inlagt")
            : Row->bPartiallyAdded ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.8b0c5f8dcfc057b5", "Delvis inlagt").ToString() : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.e00f2a51a946330f", "Ej inlagt").ToString();
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
        case ETMOPSourceReliability::PrimarySource: return NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.fab9c43d25ae6319", "Primärkälla").ToString();
        case ETMOPSourceReliability::SecondarySource: return NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1e32e036671eb37a", "Sekundärkälla").ToString();
        case ETMOPSourceReliability::Corroborated: return NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.52513cb9da26fb0c", "Bekräftad av flera källor").ToString();
        case ETMOPSourceReliability::Disputed: return NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.0cbedb9ba091aaff", "Motsagd / omtvistad").ToString();
        default: return NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.275c1a860623393b", "Obekräftad").ToString();
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
        AddBody(FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1da50de0e22697e4", "{0} källor • {1} helt inlagda • {2} delvis inlagda"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), CategoryRows.Num())), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), FullyAddedCount)), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), PartiallyAddedCount))));
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 3.0f, 2.0f, 12.0f)
        [ SNew(STMOPUppslagCoverageBar).EntryStates(EntryStates)
          .DesiredWidth(620.0f) ];

        for (const FTMOPUppslagRow* SourceRow : CategoryRows)
        {
            const FString SourceDisplayTitle = SourceRow->Title.IsEmpty()
                ? SourceRow->UppslagId.ToString() : FTMOPLocalization::String(SourceRow->Title);
            AddHeading(FText::FromString(SourceDisplayTitle));

            FString Details = SourceRow->bAddedToProject ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.6265ce0096115cb3", "Inlagt i projektet").ToString()
                : SourceRow->bPartiallyAdded ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.8b0c5f8dcfc057b5", "Delvis inlagt").ToString()
                : SourceRow->bRetrieved ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1cdd033700dd1dd7", "Genomgången").ToString() : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.1c53a8da7f5916ca", "Inte genomgången").ToString();
            Details += TEXT(" • ") + ReliabilityText(SourceRow->Reliability);
            if (!SourceRow->AuthorOrCreator.IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.ce04a7a085ef312b", "\nFörfattare/uppgiftslämnare: ").ToString() + SourceRow->AuthorOrCreator;
            if (!SourceRow->PublicationOrPlatform.IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.a41bccbaafdeefa7", "\nPublikation/plattform: ").ToString() + SourceRow->PublicationOrPlatform;
            if (!SourceRow->DocumentDate.IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.3b7ad75f7651297c", "\nDatum: ").ToString() + SourceRow->DocumentDate;
            if (!SourceRow->ISBNOrArchiveId.IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.791b324cac7d6d10", "\nISBN/arkiv-ID: ").ToString() + SourceRow->ISBNOrArchiveId;
            if (!FTMOPLocalization::String(SourceRow->PageOrLocation).IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.95a31b40ffd7aa9d", "\nSida/plats: ").ToString() + FTMOPLocalization::String(SourceRow->PageOrLocation);
            if (!FTMOPLocalization::String(SourceRow->CitationText).IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.60e15a1c15302da1", "\nKällhänvisning: ").ToString() + FTMOPLocalization::String(SourceRow->CitationText);
            if (!FTMOPLocalization::String(SourceRow->ImplementedSummary).IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.c34936283d5ffe34", "\nInlagt innehåll: ").ToString() + FTMOPLocalization::String(SourceRow->ImplementedSummary);
            if (!SourceRow->SourceUrl.IsEmpty())
                Details += NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.e28473035d31f059", "\nLänk: ").ToString() + SourceRow->SourceUrl;

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
                [ SNew(SButton).Text(FTMOPLocalization::Text(Newspaper->DisplayName))
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
    AddHeading(NSLOCTEXT("TMOP", "LanguageHeading", "Språk / Language"));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 8.0f)
    [ MakeTMOPLanguageSelector(GetGameInstance()) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 8.0f)
    [SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "SettingsAppearance", "Spelarens utseende")))
        .OnClicked_Lambda([this] {
            if (PageContentHost.IsValid())
                PageContentHost->SetContent(SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()[SNew(SButton)
                        .Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "SettingsBack", "◀ Inställningar")))
                        .OnClicked_Lambda([this] { ShowSection(ETMOPPauseHubSection::Settings); return FReply::Handled(); })]
                    + SVerticalBox::Slot().FillHeight(1)[SNew(STMOPPlayerAppearancePanel).Player(PlayerCharacter.Get())]);
            return FReply::Handled();
        })];
    AddHeading(NSLOCTEXT("TMOP", "GraphicsQuality", "Grafikkvalitet"));
    TSharedRef<SHorizontalBox> Quality = SNew(SHorizontalBox);
    const TArray<FText> Labels = { NSLOCTEXT("TMOP", "QualityLow", "Låg"), NSLOCTEXT("TMOP", "QualityMedium", "Medel"), NSLOCTEXT("TMOP", "QualityHigh", "Hög"), NSLOCTEXT("TMOP", "QualityEpic", "Episk") };
    for (int32 I = 0; I < Labels.Num(); ++I)
        Quality->AddSlot().AutoWidth().Padding(3.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(Labels[I])).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleGraphicsQuality, I) ];
    ContentBox->AddSlot().AutoHeight()[Quality];
    AddHeading(NSLOCTEXT("TMOP", "InterfaceSettings", "Gränssnitt"));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ToggleLabels", "Visa/dölj namn och ikoner i världen"))).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleWorldLabels) ];
    AddBody(NSLOCTEXT("TMOP", "PersonLabelTextSizeHeading",
        "Textstorlek ovanför personer"));
    TSharedRef<SHorizontalBox> PersonTextSize = SNew(SHorizontalBox);
    const TArray<FText> PersonTextSizeLabels = {
        NSLOCTEXT("TMOP", "PersonLabelSmall", "Liten"),
        NSLOCTEXT("TMOP", "PersonLabelMedium", "Medium"),
        NSLOCTEXT("TMOP", "PersonLabelLarge", "Stor") };
    for (int32 I = 0; I < PersonTextSizeLabels.Num(); ++I)
        PersonTextSize->AddSlot().AutoWidth().Padding(3.0f)
        [ SNew(SButton).Text(FTMOPLocalization::Text(PersonTextSizeLabels[I]))
          .OnClicked_UObject(this,
              &UTMOPPauseMenuWidget::HandlePersonLabelTextSize, I) ];
    ContentBox->AddSlot().AutoHeight().Padding(0.0f, 0.0f, 0.0f, 4.0f)
    [ PersonTextSize ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ToggleMinimap", "Visa/dölj minimap"))).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleMinimap) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ToggleOlofLocationLine", "Visa/dölj röd positionslinje över Olof Palme"))).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleOlofLocationLine) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ToggleObservationLines", "Visa/dölj aktiva observationslinjer"))).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleObservationLines) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ToggleVSync", "Växla VSync"))).OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleToggleVSync) ];
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

FReply UTMOPPauseMenuWidget::HandlePersonLabelTextSize(const int32 SizeIndex)
{
    const ETMOPPersonLabelTextSize Size =
        static_cast<ETMOPPersonLabelTextSize>(FMath::Clamp(SizeIndex, 0, 2));
    ATMOPHistoricalAgent::SaveNameLabelTextSize(Size);
    if (GetWorld() != nullptr)
        for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It)
            It->RefreshNameLabel();

    const FText SizeName = Size == ETMOPPersonLabelTextSize::Small
        ? NSLOCTEXT("TMOP", "PersonLabelSmallApplied", "liten")
        : Size == ETMOPPersonLabelTextSize::Medium
            ? NSLOCTEXT("TMOP", "PersonLabelMediumApplied", "medium")
            : NSLOCTEXT("TMOP", "PersonLabelLargeApplied", "stor");
    SetStatus(FTMOPLocalization::Format(
        NSLOCTEXT("TMOP", "PersonLabelSizeApplied",
            "Textstorlek ovanför personer: {0}."),
        SizeName));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleMinimap()
{
    if (IsValid(PlayerCharacter) && IsValid(PlayerCharacter->MapComponent))
    {
        PlayerCharacter->MapComponent->bShowMinimap = !PlayerCharacter->MapComponent->bShowMinimap;
        if (IsValid(PlayerCharacter->MinimapWidget))
            PlayerCharacter->MinimapWidget->SetMapVisible(
                PlayerCharacter->IsGameplayHUDVisible() &&
                PlayerCharacter->MapComponent->bShowMinimap);
        SetStatus(PlayerCharacter->MapComponent->bShowMinimap ? NSLOCTEXT("TMOP", "MinimapShown", "Minimap visas.") : NSLOCTEXT("TMOP", "MinimapHidden", "Minimap är dold."));
    }
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleOlofLocationLine()
{
    const bool bShow =
        !ATMOPObservationDirector::GetSavedShowOlofLocationLine();
    ATMOPObservationDirector::SaveShowOlofLocationLine(bShow);
    if (GetWorld() != nullptr)
        for (TActorIterator<ATMOPObservationDirector> It(GetWorld()); It; ++It)
            It->bShowOlofLocationLine = bShow;
    SetStatus(bShow
        ? NSLOCTEXT("TMOP", "OlofLocationLineShown",
            "Den röda positionslinjen över Olof Palme visas.")
        : NSLOCTEXT("TMOP", "OlofLocationLineHidden",
            "Den röda positionslinjen över Olof Palme är dold."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleObservationLines()
{
    const bool bShow =
        !ATMOPObservationDirector::GetSavedShowActiveObservationLines();
    ATMOPObservationDirector::SaveShowActiveObservationLines(bShow);
    if (GetWorld() != nullptr)
        for (TActorIterator<ATMOPObservationDirector> It(GetWorld()); It; ++It)
            It->bShowActiveObservationLines = bShow;
    SetStatus(bShow
        ? NSLOCTEXT("TMOP", "ObservationLinesShown",
            "Linjer för aktiva observationer visas.")
        : NSLOCTEXT("TMOP", "ObservationLinesHidden",
            "Linjer för aktiva observationer är dolda."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleToggleVSync()
{
    if (UGameUserSettings* Settings = UGameUserSettings::GetGameUserSettings())
    { Settings->SetVSyncEnabled(!Settings->IsVSyncEnabled()); Settings->ApplySettings(false); Settings->SaveSettings(); SetStatus(Settings->IsVSyncEnabled() ? NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.0dc4842eea1ab3f1", "VSync: On") : NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.3913f5eb6c5a99a0", "VSync: Off")); }
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::OpenMapPage()
{
    ShowSection(ETMOPPauseHubSection::Map);
}

void UTMOPPauseMenuWidget::BuildMapPage()
{
    if (!IsValid(PlayerCharacter) || !IsValid(PlayerCharacter->MapComponent))
    {
        AddBody(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.e46b75f9f7c8d932", "Kartkomponenten saknas."));
        return;
    }
    if (!IsValid(EmbeddedMapWidget))
    {
        EmbeddedMapWidget = CreateWidget<UTMOPMapWidget>(GetOwningPlayer());
        if (!IsValid(EmbeddedMapWidget)) return;
        EmbeddedMapWidget->InitializeMap(PlayerCharacter->MapComponent, PlayerCharacter, false);
        EmbeddedMapWidget->SetEmbeddedInMenu(true);
    }
    EmbeddedMapWidget->SetMapVisible(true);
    PageContentHost->SetContent(EmbeddedMapWidget->TakeWidget());
}

void UTMOPPauseMenuWidget::BuildControlsPage()
{
    PageContentHost->SetContent(SNew(SScrollBox)
        + SScrollBox::Slot().Padding(2.0f, 8.0f)
        [ SNew(STMOPControlsPanel).PlayerCharacter(PlayerCharacter) ]);
}

void UTMOPPauseMenuWidget::BuildSaveLoadPage()
{
    AddBody(NSLOCTEXT("TMOP", "SaveLoadIntro",
        "Skapa en ny sparning eller välj en tidigare sparning. Varje post visar var och när spelet sparades."));
    const FString FreeSlot = FTMOPSaveGameService::FindFirstFreeManualSlot(
        ManualSaveSlotPrefix, ManualSaveSlotCount);
    ContentBox->AddSlot().AutoHeight().Padding(2.0f, 7.0f, 2.0f, 14.0f)
    [ SNew(SButton)
      .Text(FTMOPLocalization::Text(FreeSlot.IsEmpty()
          ? NSLOCTEXT("TMOP", "SaveSlotsFull", "ALLA SPARPLATSER ÄR UPPTAGNA")
          : NSLOCTEXT("TMOP", "CreateNewSave", "+ SKAPA NY SPARNING")))
      .IsEnabled(!FreeSlot.IsEmpty())
      .OnClicked_UObject(this, &UTMOPPauseMenuWidget::HandleCreateNewSaveClicked) ];

    const TArray<FTMOPSaveSlotInfo> Slots = FTMOPSaveGameService::FindSaveSlots(
        ManualSaveSlotPrefix, ManualSaveSlotCount, SaveSlotName);
    if (Slots.IsEmpty())
    {
        AddBody(NSLOCTEXT("TMOP", "NoSavedGames", "Det finns inga sparade spel ännu."));
        return;
    }
    for (const FTMOPSaveSlotInfo& Info : Slots)
    {
        const FString Detail = FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.01e8a8aeac3fa85a", "Plats: {0}   •   Nivå: {1}   •   Sparad: {2}"), FTMOPLocalization::Text(FString(Info.LocationName)), FTMOPLocalization::Text(FString(Info.MapName.IsEmpty() ? TEXT("Okänd") : *Info.MapName)), FTMOPLocalization::Text(FString(Info.SavedAtText.IsEmpty() ? TEXT("Äldre sparfil") : *Info.SavedAtText))).ToString();
        const FText Title = FText::FromString(FString::Printf(TEXT("%s   —   %s"),
            *Info.DisplayName, *Info.GameTime.ToDisplayString()));
        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 5.0f)
        [ SNew(SBorder).Padding(12.0f)
          [ SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(Title)).Font(
                ATMOPTypographyDirector::ResolveFont(this, TEXT("PauseMenuSaveTitle"),
                    FCoreStyle::GetDefaultFontStyle("Bold", 17)))
              .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                  TEXT("PauseMenuSaveTitle"), FLinearColor::White)) ]
            + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 4.0f, 0.0f, 8.0f)
            [ SNew(STextBlock).Text(FTMOPLocalization::Text(FText::FromString(Detail)))
              .Font(ATMOPTypographyDirector::ResolveFont(this,
                  TEXT("PauseMenuSaveDetails"),
                  FCoreStyle::GetDefaultFontStyle("Regular", 14)))
              .ColorAndOpacity(ATMOPTypographyDirector::ResolveColor(this,
                  TEXT("PauseMenuSaveDetails"),
                  FLinearColor(0.8f, 0.82f, 0.85f))) ]
            + SVerticalBox::Slot().AutoHeight()
            [ SNew(SHorizontalBox)
              + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
              [ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "LoadSelectedSave", "LADDA")))
                .OnClicked_UObject(this,
                    &UTMOPPauseMenuWidget::HandleLoadSaveSlotClicked, Info.SlotName) ]
              + SHorizontalBox::Slot().AutoWidth().Padding(0.0f, 0.0f, 8.0f, 0.0f)
              [ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "OverwriteSelectedSave", "SKRIV ÖVER")))
                .OnClicked_UObject(this,
                    &UTMOPPauseMenuWidget::HandleOverwriteSaveClicked, Info.SlotName) ]
              + SHorizontalBox::Slot().AutoWidth()
              [ SNew(SButton)
                .Text(FTMOPLocalization::Text(PendingDeleteSaveSlot == Info.SlotName
                    ? NSLOCTEXT("TMOP", "ConfirmDeleteSave", "BEKRÄFTA RADERING")
                    : NSLOCTEXT("TMOP", "DeleteSelectedSave", "RADERA")))
                .OnClicked_UObject(this,
                    &UTMOPPauseMenuWidget::HandleDeleteSaveSlotClicked, Info.SlotName) ] ] ] ];
    }
}

FReply UTMOPPauseMenuWidget::HandleSaveClicked()
{
    return HandleCreateNewSaveClicked();
}

FReply UTMOPPauseMenuWidget::HandleLoadClicked()
{
    LoadQuickSave(SaveSlotName);
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::OpenSettingsPage()
{
    ShowSection(ETMOPPauseHubSection::Settings);
}

bool UTMOPPauseMenuWidget::LoadQuickSave(const FString& SlotName)
{
    FText Status;
    const bool bLoaded = FTMOPSaveGameService::LoadPlayer(
        GetWorld(), PlayerCharacter, SlotName, Status);
    SetStatus(Status);
    if (bLoaded) OnLoadRequested.Broadcast();
    return bLoaded;
}

FReply UTMOPPauseMenuWidget::HandleCreateNewSaveClicked()
{
    const FString NewSaveSlot = FTMOPSaveGameService::FindFirstFreeManualSlot(
        ManualSaveSlotPrefix, ManualSaveSlotCount);
    if (NewSaveSlot.IsEmpty())
    {
        SetStatus(NSLOCTEXT("TMOP", "NoFreeSaveSlot",
            "Alla sparplatser används. Skriv över eller radera en befintlig sparning."));
        return FReply::Handled();
    }
    int32 SlotIndex = 1;
    for (; SlotIndex <= ManualSaveSlotCount; ++SlotIndex)
        if (FTMOPSaveGameService::MakeManualSlotName(
            ManualSaveSlotPrefix, SlotIndex) == NewSaveSlot) break;
    FText Status;
    const bool bSaved = FTMOPSaveGameService::SavePlayer(GetWorld(),
        PlayerCharacter, NewSaveSlot,
        FTMOPLocalization::Format(NSLOCTEXT("TMOP", "TMOPPauseMenuWidget.d32d4c66ce8ae8fc", "Manuell sparning {0}"), FText::AsCultureInvariant(FString::Printf(TEXT("%d"), SlotIndex))).ToString(),
        ETMOPMenuSaveKind::Manual, Status);
    PendingDeleteSaveSlot.Reset();
    if (CurrentSection == ETMOPPauseHubSection::SaveLoad && ContentBox.IsValid())
        { ContentBox->ClearChildren(); BuildSaveLoadPage(); }
    SetStatus(Status);
    if (bSaved) OnSaveRequested.Broadcast();
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleOverwriteSaveClicked(FString SlotName)
{
    UTMOPMenuSaveGame* Existing = Cast<UTMOPMenuSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    const FString DisplayName = IsValid(Existing) && !Existing->SlotDisplayName.IsEmpty()
        ? Existing->SlotDisplayName : TEXT("Manuell sparning");
    FText Status;
    const bool bSaved = FTMOPSaveGameService::SavePlayer(GetWorld(),
        PlayerCharacter, SlotName, DisplayName, ETMOPMenuSaveKind::Manual, Status);
    PendingDeleteSaveSlot.Reset();
    if (ContentBox.IsValid()) { ContentBox->ClearChildren(); BuildSaveLoadPage(); }
    SetStatus(Status);
    if (bSaved) OnSaveRequested.Broadcast();
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleLoadSaveSlotClicked(FString SlotName)
{
    if (LoadQuickSave(SlotName) && IsValid(PlayerCharacter))
        PlayerCharacter->SetPauseMenuOpen(false);
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleDeleteSaveSlotClicked(FString SlotName)
{
    if (PendingDeleteSaveSlot != SlotName)
    {
        PendingDeleteSaveSlot = SlotName;
        if (ContentBox.IsValid()) { ContentBox->ClearChildren(); BuildSaveLoadPage(); }
        SetStatus(NSLOCTEXT("TMOP", "DeleteSaveConfirmStatus",
            "Klicka på BEKRÄFTA RADERING för att ta bort sparningen permanent."));
        return FReply::Handled();
    }
    const bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, 0);
    PendingDeleteSaveSlot.Reset();
    if (ContentBox.IsValid()) { ContentBox->ClearChildren(); BuildSaveLoadPage(); }
    SetStatus(bDeleted
        ? NSLOCTEXT("TMOP", "DeleteSaveSuccess", "Sparningen raderades.")
        : NSLOCTEXT("TMOP", "DeleteSaveFailed", "Sparningen kunde inte raderas."));
    return FReply::Handled();
}

void UTMOPPauseMenuWidget::BuildQuitPage()
{
    AddBody(NSLOCTEXT("TMOP", "QuitWarning", "Vill du avsluta spelet? Osparade framsteg försvinner."));
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,8.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ConfirmQuit", "AVSLUTA SPELET"))).OnClicked_UObject(this,&UTMOPPauseMenuWidget::HandleQuitClicked) ];
}

FReply UTMOPPauseMenuWidget::HandleQuitClicked()
{ if (IsValid(PlayerController)) UKismetSystemLibrary::QuitGame(this,PlayerController,EQuitPreference::Quit,false); return FReply::Handled(); }

void UTMOPPauseMenuWidget::BuildMoveInTimePage()
{
    AddBody(NSLOCTEXT("TMOP", "MoveTimeInstructions", "Skriv HH:MM eller HH:MM:SS. Endast 23:00:00–23:45:00 godtas. Tiden avrundas till närmaste femsekunderssteg. En giltig historisk bake krävs."));
    FString Current = TEXT("23:00:00");
    if (IsValid(PlayerCharacter)) if (UTMOPClockSubsystem* Clock = PlayerCharacter->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()) Current = Clock->GetCurrentTime().ToDisplayString();
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,8.0f)[ SAssignNew(TimeEntryBox,SEditableTextBox).Text(FTMOPLocalization::Text(FText::FromString(Current))).HintText(FText::FromString(TEXT("23:21:30"))) ];
    ContentBox->AddSlot().AutoHeight().Padding(2.0f,5.0f)[ SNew(SButton).Text(FTMOPLocalization::Text(NSLOCTEXT("TMOP", "ApplyMoveTime", "FLYTTA TILL KLOCKSLAGET"))).OnClicked_UObject(this,&UTMOPPauseMenuWidget::HandleMoveInTimeClicked) ];
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
    SetStatus(bMoved ? NSLOCTEXT("TMOP", "MoveTimeSuccess", "Tidsförflyttningen förbereds. Spelet behåller menypausen när den är klar.") : NSLOCTEXT("TMOP", "MoveTimeFailed", "Tidsförflyttning kunde inte starta. Kontrollera att en giltig historisk bake är laddad."));
    return FReply::Handled();
}

FReply UTMOPPauseMenuWidget::HandleResumeClicked()
{ if (IsValid(PlayerCharacter)) PlayerCharacter->SetPauseMenuOpen(false); return FReply::Handled(); }

FReply UTMOPPauseMenuWidget::NativeOnKeyDown(const FGeometry& Geometry,const FKeyEvent& Event)
{
    const FKey Key=Event.GetKey();
    const bool bProfileClose = (IsMapPage() &&
        TMOPMatchesControl(this, Key, ETMOPControlAction::WorldMap)) ||
        TMOPMatchesControl(this, Key, ETMOPControlAction::Pause) ||
        TMOPMatchesControl(this, Key, ETMOPControlAction::Cancel) ||
        TMOPMatchesControl(this, Key, ETMOPControlAction::MenuBack);
    const bool bLegacyClose = !TMOPHasControlProfiles(this) &&
        (Key==EKeys::Enter || Key==EKeys::Escape ||
         Key==EKeys::Gamepad_Special_Right || Key==EKeys::Gamepad_FaceButton_Right);
    if (bProfileClose || bLegacyClose)
        return HandleResumeClicked();
    return Super::NativeOnKeyDown(Geometry,Event);
}

void UTMOPPauseMenuWidget::SetStatus(const FText& Text)
{ if (StatusText.IsValid()) StatusText->SetText(FTMOPLocalization::Text(Text)); }
