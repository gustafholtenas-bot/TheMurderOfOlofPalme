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
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
/** Draws one entire investigation-section coverage strip in a single widget. */
class STMOPUppslagCoverageBar final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPUppslagCoverageBar) {}
        SLATE_ARGUMENT(TArray<bool>, AvailableEntries)
    SLATE_END_ARGS()

    void Construct(const FArguments& Arguments)
    {
        AvailableEntries = Arguments._AvailableEntries;
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return FVector2D(620.0f, 24.0f);
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
        FSlateDrawElement::MakeBox(DrawElements, LayerId,
            Geometry.ToPaintGeometry(), WhiteBrush, ESlateDrawEffect::None,
            FLinearColor(0.16f, 0.145f, 0.12f, 1.0f));

        if (!AvailableEntries.IsEmpty() && BarWidth > 2.0f)
        {
            const float Step = BarWidth / static_cast<float>(AvailableEntries.Num());
            for (int32 Index = 0; Index < AvailableEntries.Num(); ++Index)
            {
                if (!AvailableEntries[Index]) continue;
                const float X = FMath::Clamp((Index + 0.5f) * Step,
                    1.0f, BarWidth - 1.0f);
                TArray<FVector2f> Marker;
                Marker.Add(FVector2f(X, 1.0f));
                Marker.Add(FVector2f(X, BarHeight - 1.0f));
                FSlateDrawElement::MakeLines(DrawElements, LayerId + 1,
                    Geometry.ToPaintGeometry(), Marker, ESlateDrawEffect::None,
                    FLinearColor(0.95f, 0.08f, 0.06f, 1.0f), true,
                    FMath::Clamp(Step * 0.72f, 1.0f, 3.0f));
            }
        }

        const FLinearColor BorderColor(0.95f, 0.08f, 0.06f, 1.0f);
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
    TArray<bool> AvailableEntries;
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
{ ShowSection(Section); return FReply::Handled(); }

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
        return A.UppslagId.LexicalLess(B.UppslagId);
    });

    AddBody(NSLOCTEXT("TMOP", "SourcesCoverageIntro",
        "Varje rad motsvarar ett avsnitt i utredningen. En röd markering visar ett uppslag som finns i projektets källmaterial."));

    TMap<FName, TArray<FTMOPUppslagRow*>> RowsBySeries;
    for (FTMOPUppslagRow* Row : Rows)
        if (Row != nullptr && !Row->SeriesId.IsNone())
            RowsBySeries.FindOrAdd(Row->SeriesId).Add(Row);

    TArray<FName> SeriesIds;
    RowsBySeries.GetKeys(SeriesIds);
    SeriesIds.Sort(FNameLexicalLess());
    for (const FName SeriesId : SeriesIds)
    {
        TArray<FTMOPUppslagRow*>& SeriesRows = RowsBySeries.FindChecked(SeriesId);
        SeriesRows.Sort([](const FTMOPUppslagRow& A, const FTMOPUppslagRow& B)
        {
            return A.UppslagId.LexicalLess(B.UppslagId);
        });

        TArray<bool> AvailableEntries;
        AvailableEntries.Reserve(SeriesRows.Num());
        int32 AvailableCount = 0;
        int32 AddedCount = 0;
        for (const FTMOPUppslagRow* Row : SeriesRows)
        {
            const bool bAvailable = Row != nullptr &&
                (Row->bRetrieved || Row->bAddedToProject || Row->bPartiallyAdded);
            AvailableEntries.Add(bAvailable);
            AvailableCount += bAvailable ? 1 : 0;
            AddedCount += Row != nullptr && Row->bAddedToProject ? 1 : 0;
        }
        const int32 TotalCount = SeriesRows.Num();
        const int32 AddedPercent = TotalCount > 0
            ? FMath::RoundToInt(100.0f * AddedCount / TotalCount) : 0;
        const FText Statistics = FText::FromString(FString::Printf(
            TEXT("%d/%d uppslag\n%d procent inlagt"),
            AvailableCount, TotalCount, AddedPercent));

        ContentBox->AddSlot().AutoHeight().Padding(2.0f, 12.0f)
        [ SNew(SHorizontalBox)
          + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
          [ SNew(SBox).WidthOverride(105.0f)
            [ SNew(STextBlock).Text(FText::FromName(SeriesId))
              .Font(FCoreStyle::GetDefaultFontStyle("Bold", 28)) ] ]
          + SHorizontalBox::Slot().FillWidth(1.0f).Padding(10.0f, 0.0f, 18.0f, 0.0f)
            .VAlign(VAlign_Center)
          [ SNew(STMOPUppslagCoverageBar).AvailableEntries(AvailableEntries) ]
          + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
          [ SNew(SBox).WidthOverride(180.0f)
            [ SNew(STextBlock).Text(Statistics).AutoWrapText(false) ] ] ];
    }

    AddHeading(NSLOCTEXT("TMOP", "SourcesDetails", "Uppslagsdetaljer"));

    int32 VisibleCount = 0;
    constexpr int32 MaximumVisibleRows = 300;
    for (const FTMOPUppslagRow* Row : Rows)
    {
        if (Row == nullptr || !Row->bRelevantToGame) continue;
        const FString Title = Row->Title.IsEmpty() ? TEXT("Utan titel") : Row->Title.ToString();
        const FString State = Row->bAddedToProject ? TEXT("Inlagt")
            : Row->bPartiallyAdded ? TEXT("Delvis inlagt") : TEXT("Ej inlagt");
        AddHeading(FText::FromString(FString::Printf(TEXT("%s — %s"),
            *Row->UppslagId.ToString(), *Title)));
        FString Details = State;
        if (!Row->DocumentDate.IsEmpty()) Details += TEXT(" • ") + Row->DocumentDate;
        if (!Row->SourceUrl.IsEmpty()) Details += TEXT("\n") + Row->SourceUrl;
        AddBody(FText::FromString(Details));
        if (++VisibleCount >= MaximumVisibleRows) break;
    }
    if (VisibleCount == 0)
        AddBody(NSLOCTEXT("TMOP", "NoRelevantSources", "Tabellen innehåller inga uppslag markerade som relevanta för spelet."));
    else if (VisibleCount >= MaximumVisibleRows)
        AddBody(NSLOCTEXT("TMOP", "SourcesLimited", "Listan visar de första 300 relevanta uppslagen för att hålla menyn snabb."));
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
