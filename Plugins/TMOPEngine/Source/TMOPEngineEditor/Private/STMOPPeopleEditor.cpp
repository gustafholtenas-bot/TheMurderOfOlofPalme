#include "STMOPPeopleEditor.h"
#include "TMOPRuntimeValidationReader.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "EngineUtils.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "IStructureDetailsView.h"
#include "Misc/MessageDialog.h"
#include "NavigationSystem.h"
#include "People/TMOPAppearanceResolver.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "UObject/StructOnScope.h"
#include "UObject/UObjectGlobals.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSearchableComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "STMOPPeopleEditor"

namespace
{
    FString ActionLabel(const ETMOPPersonTimelineAction Action)
    {
        if (const UEnum* Enum = StaticEnum<ETMOPPersonTimelineAction>())
            return Enum->GetDisplayNameTextByValue(
                static_cast<int64>(Action)).ToString();
        return TEXT("Unknown");
    }
}

void STMOPPeopleEditor::Construct(const FArguments& Args)
{
    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = true;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.bLockable = false;
    DetailsArgs.bUpdatesFromSelection = false;
    DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

    FStructureDetailsViewArgs StructureArgs;
    StructureArgs.bShowObjects = false;
    StructureArgs.bShowAssets = false;
    StructureArgs.bShowClasses = false;
    StructureArgs.bShowInterfaces = false;

    FPropertyEditorModule& PropertyEditor =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
            TEXT("PropertyEditor"));
    EntryDetailsView = PropertyEditor.CreateStructureDetailView(
        DetailsArgs, StructureArgs, nullptr);
    CharacteristicsDetailsView = PropertyEditor.CreateStructureDetailView(
        DetailsArgs, StructureArgs, nullptr);
    GeneralDetailsView = PropertyEditor.CreateStructureDetailView(
        DetailsArgs, StructureArgs, nullptr);

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot()
            .FillWidth(1.0f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(this, &STMOPPeopleEditor::GetSelectedPersonTitle)
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                [
                    SNew(STextBlock)
                    .Text(this, &STMOPPeopleEditor::GetSelectedPersonSubtitle)
                    .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                ]
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.0f, 0.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("Reload", "Reload"))
                .OnClicked(this, &STMOPPeopleEditor::ReloadPerson)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.0f, 0.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("ResolveAppearance", "Generate Appearance"))
                .ToolTipText(LOCTEXT("ResolveAppearanceTip",
                    "Resolve the selected person's body, face and clothing from the evidence fields."))
                .OnClicked(this, &STMOPPeopleEditor::ResolveCurrentAppearance)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            .Padding(4.0f, 0.0f)
            [
                SNew(SButton)
                .Text(LOCTEXT("ValidateAllAppearances", "Validate All Appearances"))
                .ToolTipText(LOCTEXT("ValidateAllAppearancesTip",
                    "Resolve every person and report known descriptions that lack a matching asset."))
                .OnClicked(this, &STMOPPeopleEditor::ValidateAllAppearances)
            ]
            + SHorizontalBox::Slot()
            .AutoWidth()
            [
                SNew(SButton)
                .Text(LOCTEXT("SavePerson", "Save Person"))
                .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
                .OnClicked(this, &STMOPPeopleEditor::SavePerson)
            ]
        ]

        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SNew(SSplitter)
            .PhysicalSplitterHandleSize(5.0f)

            + SSplitter::Slot()
            .Value(0.13f)
            [
                SNew(SBorder)
                .Padding(6.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(SSearchBox)
                        .HintText(LOCTEXT(
                            "SearchPeople", "Search people, ID or category"))
                        .OnTextChanged(
                            this,
                            &STMOPPeopleEditor::HandlePersonSearchChanged)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0.0f, 0.0f, 8.0f, 0.0f)
                        [
                            SNew(SCheckBox)
                            .IsChecked(this,
                                &STMOPPeopleEditor::GetPeopleFilterCheckState,
                                EPeopleCategoryFilter::All)
                            .OnCheckStateChanged(this,
                                &STMOPPeopleEditor::HandlePeopleFilterChanged,
                                EPeopleCategoryFilter::All)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("FilterAll", "All"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0.0f, 0.0f, 8.0f, 0.0f)
                        [
                            SNew(SCheckBox)
                            .IsChecked(this,
                                &STMOPPeopleEditor::GetPeopleFilterCheckState,
                                EPeopleCategoryFilter::Police)
                            .OnCheckStateChanged(this,
                                &STMOPPeopleEditor::HandlePeopleFilterChanged,
                                EPeopleCategoryFilter::Police)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("FilterPolice", "Police"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SCheckBox)
                            .IsChecked(this,
                                &STMOPPeopleEditor::GetPeopleFilterCheckState,
                                EPeopleCategoryFilter::Suspect)
                            .OnCheckStateChanged(this,
                                &STMOPPeopleEditor::HandlePeopleFilterChanged,
                                EPeopleCategoryFilter::Suspect)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("FilterSuspect", "Suspect"))
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        .Padding(0.0f, 0.0f, 8.0f, 0.0f)
                        [
                            SNew(SCheckBox)
                            .IsChecked(this,
                                &STMOPPeopleEditor::GetPeopleFilterCheckState,
                                EPeopleCategoryFilter::Spawned)
                            .OnCheckStateChanged(this,
                                &STMOPPeopleEditor::HandlePeopleFilterChanged,
                                EPeopleCategoryFilter::Spawned)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("FilterSpawned", "Spawned"))
                            ]
                        ]
                        + SHorizontalBox::Slot()
                        .AutoWidth()
                        [
                            SNew(SCheckBox)
                            .IsChecked(this,
                                &STMOPPeopleEditor::GetPeopleFilterCheckState,
                                EPeopleCategoryFilter::NonSpawned)
                            .OnCheckStateChanged(this,
                                &STMOPPeopleEditor::HandlePeopleFilterChanged,
                                EPeopleCategoryFilter::NonSpawned)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("FilterNonSpawned", "Non-spawned"))
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(SCheckBox)
                        .IsChecked(this,
                            &STMOPPeopleEditor::GetPeopleFilterCheckState,
                            EPeopleCategoryFilter::MainCharacters)
                        .OnCheckStateChanged(this,
                            &STMOPPeopleEditor::HandlePeopleFilterChanged,
                            EPeopleCategoryFilter::MainCharacters)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT(
                                "FilterMainCharacters", "Main Characters"))
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        SAssignNew(PersonListView, SListView<FPersonItem>)
                        .ListItemsSource(&PersonItems)
                        .OnGenerateRow(
                            this, &STMOPPeopleEditor::GeneratePersonRow)
                        .OnSelectionChanged(
                            this,
                            &STMOPPeopleEditor::HandlePersonSelectionChanged)
                        .SelectionMode(ESelectionMode::Single)
                    ]
                ]
            ]

            + SSplitter::Slot()
            .Value(0.20f)
            [
                SNew(SBorder)
                .Padding(6.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth()
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("AddEntry", "+ Add"))
                            .OnClicked(
                                this, &STMOPPeopleEditor::AddTimelineEntry)
                        ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(3.0f, 0.0f)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("DuplicateEntry", "Duplicate"))
                            .OnClicked(
                                this,
                                &STMOPPeopleEditor::DuplicateTimelineEntry)
                        ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("DeleteEntry", "Delete"))
                            .OnClicked(
                                this, &STMOPPeopleEditor::DeleteTimelineEntry)
                        ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(8.0f, 0.0f, 3.0f, 0.0f)
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("MoveUp", "↑"))
                            .OnClicked(
                                this, &STMOPPeopleEditor::MoveTimelineEntryUp)
                        ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [
                            SNew(SButton)
                            .Text(LOCTEXT("MoveDown", "↓"))
                            .OnClicked(
                                this, &STMOPPeopleEditor::MoveTimelineEntryDown)
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(0.56f)
                    [
                        SAssignNew(TimelineListView, SListView<FTimelineItem>)
                        .ListItemsSource(&TimelineItems)
                        .OnGenerateRow(
                            this, &STMOPPeopleEditor::GenerateTimelineRow)
                        .OnSelectionChanged(
                            this,
                            &STMOPPeopleEditor::HandleTimelineSelectionChanged)
                        .SelectionMode(ESelectionMode::Single)
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 8.0f, 0.0f, 4.0f)
                    [
                        SNew(SBorder)
                        .Padding(6.0f)
                        .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("ComparisonTimelineTitle",
                                    "REFERENCE TIMELINE"))
                                .Font(FAppStyle::GetFontStyle(
                                    "HeadingExtraSmall"))
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            .Padding(0.0f, 4.0f, 0.0f, 0.0f)
                            [
                                SAssignNew(ComparisonPersonCombo,
                                    SSearchableComboBox)
                                .OptionsSource(&ComparisonPersonItems)
                                .OnGenerateWidget(this,
                                    &STMOPPeopleEditor::
                                        GenerateComparisonPersonOption)
                                .OnSelectionChanged(this,
                                    &STMOPPeopleEditor::
                                        HandleComparisonPersonSelected)
                                [
                                    SNew(STextBlock)
                                    .Text(this,
                                        &STMOPPeopleEditor::
                                            GetComparisonPersonText)
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(0.44f)
                    [
                        SAssignNew(ComparisonTimelineListView,
                            SListView<FTimelineItem>)
                        .ListItemsSource(&ComparisonTimelineItems)
                        .OnGenerateRow(this,
                            &STMOPPeopleEditor::
                                GenerateComparisonTimelineRow)
                        .OnSelectionChanged(this,
                            &STMOPPeopleEditor::
                                HandleComparisonTimelineSelectionChanged)
                        .OnContextMenuOpening(this,
                            &STMOPPeopleEditor::
                                BuildComparisonTimelineContextMenu)
                        .SelectionMode(ESelectionMode::Single)
                    ]
                ]
            ]

            + SSplitter::Slot()
            .Value(0.25f)
            [
                SNew(SBorder)
                .Padding(6.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 7.0f)
                    [
                        SNew(SBorder)
                        .Padding(7.0f)
                        .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
                        [
                            SNew(SVerticalBox)
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("ReferenceSearchTitle",
                                    "Search timeline references"))
                                .Font(FAppStyle::GetFontStyle(
                                    "HeadingExtraSmall"))
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 2)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("AnchorReference", "Target Anchor"))
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SAssignNew(AnchorReferenceCombo, SSearchableComboBox)
                                .OptionsSource(&AnchorReferenceItems)
                                .OnGenerateWidget(this,
                                    &STMOPPeopleEditor::GenerateReferenceOption)
                                .OnSelectionChanged(this,
                                    &STMOPPeopleEditor::HandleReferenceSelected,
                                    EReferenceField::TargetAnchor)
                                [
                                    SNew(STextBlock)
                                    .Text(this,
                                        &STMOPPeopleEditor::GetReferenceFieldText,
                                        EReferenceField::TargetAnchor)
                                ]
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 2)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("EntityReference",
                                    "Target Person / Vehicle"))
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SAssignNew(EntityReferenceCombo, SSearchableComboBox)
                                .OptionsSource(&EntityReferenceItems)
                                .OnGenerateWidget(this,
                                    &STMOPPeopleEditor::GenerateReferenceOption)
                                .OnSelectionChanged(this,
                                    &STMOPPeopleEditor::HandleReferenceSelected,
                                    EReferenceField::TargetEntity)
                                [
                                    SNew(STextBlock)
                                    .Text(this,
                                        &STMOPPeopleEditor::GetReferenceFieldText,
                                    EReferenceField::TargetEntity)
                                ]
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 2)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("GroupReference", "Target Group"))
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SAssignNew(GroupReferenceCombo, SSearchableComboBox)
                                .OptionsSource(&GroupReferenceItems)
                                .OnGenerateWidget(this,
                                    &STMOPPeopleEditor::GenerateReferenceOption)
                                .OnSelectionChanged(this,
                                    &STMOPPeopleEditor::HandleReferenceSelected,
                                    EReferenceField::TargetGroup)
                                [
                                    SNew(STextBlock)
                                    .Text(this,
                                        &STMOPPeopleEditor::GetReferenceFieldText,
                                        EReferenceField::TargetGroup)
                                ]
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 2)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("SeatReference", "Target Seat"))
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SAssignNew(SeatReferenceCombo, SSearchableComboBox)
                                .OptionsSource(&SeatReferenceItems)
                                .OnGenerateWidget(this,
                                    &STMOPPeopleEditor::GenerateReferenceOption)
                                .OnSelectionChanged(this,
                                    &STMOPPeopleEditor::HandleReferenceSelected,
                                    EReferenceField::TargetSeat)
                                [
                                    SNew(STextBlock)
                                    .Text(this,
                                        &STMOPPeopleEditor::GetReferenceFieldText,
                                        EReferenceField::TargetSeat)
                                ]
                            ]
                            + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 2)
                            [
                                SNew(STextBlock)
                                .Text(LOCTEXT("EventReference", "Shared Event"))
                            ]
                            + SVerticalBox::Slot().AutoHeight()
                            [
                                SAssignNew(EventReferenceCombo, SSearchableComboBox)
                                .OptionsSource(&EventReferenceItems)
                                .OnGenerateWidget(this,
                                    &STMOPPeopleEditor::GenerateReferenceOption)
                                .OnSelectionChanged(this,
                                    &STMOPPeopleEditor::HandleReferenceSelected,
                                    EReferenceField::SharedEvent)
                                [
                                    SNew(STextBlock)
                                    .Text(this,
                                        &STMOPPeopleEditor::GetReferenceFieldText,
                                        EReferenceField::SharedEvent)
                                ]
                            ]
                        ]
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        EntryDetailsView->GetWidget().ToSharedRef()
                    ]
                ]
            ]

            + SSplitter::Slot()
            .Value(0.24f)
            [
                SNew(SBorder)
                .Padding(6.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("CharacteristicsTitle", "CHARACTERISTICS"))
                        .Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
                        .Justification(ETextJustify::Center)
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        CharacteristicsDetailsView->GetWidget().ToSharedRef()
                    ]
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(4.0f, 8.0f, 4.0f, 4.0f)
                    [
                        SNew(SVerticalBox)
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .Padding(0.0f, 0.0f, 0.0f, 4.0f)
                        [
                            SNew(STextBlock)
                            .Text(LOCTEXT("ImageReferenceTitle", "IMAGE REFERENCE"))
                            .Justification(ETextJustify::Center)
                            .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                        ]
                        + SVerticalBox::Slot()
                        .AutoHeight()
                        .HAlign(HAlign_Center)
                        [
                            SNew(SBox)
                            .WidthOverride(220.0f)
                            .HeightOverride(260.0f)
                            [
                                SNew(SBorder)
                                .Padding(3.0f)
                                .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
                                [
                                    SNew(SOverlay)
                                    + SOverlay::Slot()
                                    [
                                        SNew(SScaleBox)
                                        .Stretch(EStretch::ScaleToFit)
                                        [
                                            SNew(SImage)
                                            .Image(this,
                                                &STMOPPeopleEditor::GetReferenceImageBrush)
                                        ]
                                    ]
                                    + SOverlay::Slot()
                                    .HAlign(HAlign_Center)
                                    .VAlign(VAlign_Center)
                                    [
                                        SNew(STextBlock)
                                        .Text(LOCTEXT("NoReferenceImage",
                                            "No reference image selected"))
                                        .Visibility(this,
                                            &STMOPPeopleEditor::
                                                GetReferenceImagePlaceholderVisibility)
                                        .ColorAndOpacity(
                                            FSlateColor::UseSubduedForeground())
                                    ]
                                ]
                            ]
                        ]
                    ]
                ]
            ]

            + SSplitter::Slot()
            .Value(0.18f)
            [
                SNew(SBorder)
                .Padding(6.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(0.0f, 0.0f, 0.0f, 6.0f)
                    [
                        SNew(STextBlock)
                        .Text(LOCTEXT("DataTitle", "DATA"))
                        .Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
                        .Justification(ETextJustify::Center)
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        GeneralDetailsView->GetWidget().ToSharedRef()
                    ]
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(8.0f, 5.0f)
        [
            SAssignNew(StatusText, STextBlock)
            .Text(LOCTEXT("Ready", "Ready"))
            .ColorAndOpacity(FLinearColor(0.55f, 0.8f, 0.55f))
        ]
    ];

    LoadDefaultTable();
    LastRuntimeValidationRevision =
        TMOPRuntimeValidation::GetLatestReportRevision();
    RegisterActiveTimer(
        2.0f,
        FWidgetActiveTimerDelegate::CreateSP(
            this,
            &STMOPPeopleEditor::HandleRuntimeValidationRefresh));
}

void STMOPPeopleEditor::LoadDefaultTable()
{
    PeopleTable = LoadObject<UDataTable>(
        nullptr, DefaultPeopleTablePath);
    EventTable = LoadObject<UDataTable>(
        nullptr, DefaultEventTablePath);
    VehicleTable = LoadObject<UDataTable>(
        nullptr, DefaultVehicleTablePath);
    AppearanceTable = LoadObject<UDataTable>(
        nullptr, DefaultAppearanceTablePath);
    if (!PeopleTable.IsValid() ||
        PeopleTable->GetRowStruct() !=
            FTMOPPersonProfileRow::StaticStruct())
    {
        SetStatus(
            FText::Format(
                LOCTEXT("MissingTable",
                    "Could not load {0} with the expected row type."),
                FText::FromString(DefaultPeopleTablePath)),
            FLinearColor::Red);
        return;
    }
    RefreshPeople();
    RefreshReferenceOptions();
    SetStatus(
        LOCTEXT("TableLoaded", "DT_TMOP_People loaded."),
        FLinearColor(0.55f, 0.8f, 0.55f));
}

void STMOPPeopleEditor::RefreshPeople()
{
    PersonItems.Reset();
    UDataTable* Table = PeopleTable.Get();
    if (!IsValid(Table)) return;

    for (const FName RowName : Table->GetRowNames())
    {
        const FTMOPPersonProfileRow* Row =
            Table->FindRow<FTMOPPersonProfileRow>(
                RowName, TEXT("TMOPPeopleEditorList"), false);
        if (Row == nullptr) continue;
        const FString Haystack =
            RowName.ToString() + TEXT(" ") +
            Row->EntityId.ToString() + TEXT(" ") +
            Row->FullName.ToString() + TEXT(" ") +
            Row->CategoryId.ToString();
        if (!PersonSearch.IsEmpty() &&
            !Haystack.Contains(PersonSearch, ESearchCase::IgnoreCase))
            continue;

        const FString Category = Row->CategoryId.ToString().TrimStartAndEnd();
        const bool bPoliceCategory =
            Category.Equals(TEXT("Police"), ESearchCase::IgnoreCase) ||
            Category.Equals(TEXT("Polis"), ESearchCase::IgnoreCase) ||
            Category.EndsWith(TEXT("_Police"), ESearchCase::IgnoreCase) ||
            Category.EndsWith(TEXT("_Polis"), ESearchCase::IgnoreCase);
        const bool bSuspectCategory =
            Category.Equals(TEXT("Suspect"), ESearchCase::IgnoreCase) ||
            Category.EndsWith(TEXT("_Suspect"), ESearchCase::IgnoreCase);

        if (PeopleCategoryFilter == EPeopleCategoryFilter::Police &&
            !bPoliceCategory)
            continue;
        if (PeopleCategoryFilter == EPeopleCategoryFilter::Suspect &&
            !bSuspectCategory)
            continue;
        if (PeopleCategoryFilter == EPeopleCategoryFilter::Spawned &&
            !Row->bSpawnInSimulation)
            continue;
        if (PeopleCategoryFilter == EPeopleCategoryFilter::NonSpawned &&
            Row->bSpawnInSimulation)
            continue;
        if (PeopleCategoryFilter ==
                EPeopleCategoryFilter::MainCharacters &&
            !IsMainCharacter(*Row))
            continue;

        PersonItems.Add(MakeShared<FName>(RowName));
    }
    PersonItems.Sort([](const FPersonItem& A, const FPersonItem& B)
    {
        return A->ToString() < B->ToString();
    });
    if (PersonListView.IsValid())
        PersonListView->RequestListRefresh();
}

void STMOPPeopleEditor::RefreshTimeline()
{
    TimelineItems.Reset();
    for (int32 Index = 0; Index < WorkingRow.Timeline.Num(); ++Index)
        TimelineItems.Add(MakeShared<int32>(Index));
    if (TimelineListView.IsValid())
        TimelineListView->RequestListRefresh();
    if (ComparisonTimelineListView.IsValid())
        ComparisonTimelineListView->RequestListRefresh();
}

EActiveTimerReturnType STMOPPeopleEditor::HandleRuntimeValidationRefresh(
    const double, const float)
{
    const uint64 Revision =
        TMOPRuntimeValidation::GetLatestReportRevision();
    if (Revision != LastRuntimeValidationRevision)
    {
        LastRuntimeValidationRevision = Revision;
        if (TimelineListView.IsValid())
            TimelineListView->RequestListRefresh();
    }
    return EActiveTimerReturnType::Continue;
}

void STMOPPeopleEditor::RefreshComparisonPeople()
{
    ComparisonPersonItems.Reset();
    ComparisonRowNamesByLabel.Reset();
    const UDataTable* Table = PeopleTable.Get();
    if (!IsValid(Table)) return;

    struct FComparisonCandidate
    {
        int32 Priority = 2;
        FString Label;
        FName RowName = NAME_None;
    };
    TArray<FComparisonCandidate> Candidates;
    for (const FName RowName : Table->GetRowNames())
    {
        const FTMOPPersonProfileRow* Row =
            Table->FindRow<FTMOPPersonProfileRow>(
                RowName, TEXT("TMOPComparisonPersonList"), false);
        if (Row == nullptr || RowName == SelectedRowName) continue;

        const bool bSameGroup = !WorkingRow.SocialGroupId.IsNone() &&
            Row->SocialGroupId == WorkingRow.SocialGroupId;
        const bool bLeader = bSameGroup &&
            ((!WorkingRow.GroupLeaderEntityId.IsNone() &&
              Row->EntityId == WorkingRow.GroupLeaderEntityId) ||
             (!Row->GroupLeaderEntityId.IsNone() &&
              Row->EntityId == Row->GroupLeaderEntityId));
        const FString Name = !Row->FullName.IsEmpty()
            ? Row->FullName.ToString() : Row->EntityId.ToString();
        const FString Prefix = bLeader
            ? TEXT("[GROUP LEADER] ")
            : bSameGroup ? TEXT("[SAME GROUP] ") : FString();
        FComparisonCandidate Candidate;
        Candidate.Priority = bLeader ? 0 : bSameGroup ? 1 : 2;
        Candidate.Label = Prefix + Name + TEXT("  •  ") +
            Row->EntityId.ToString();
        Candidate.RowName = RowName;
        Candidates.Add(MoveTemp(Candidate));
    }
    Candidates.Sort([](const FComparisonCandidate& A,
                       const FComparisonCandidate& B)
    {
        return A.Priority != B.Priority
            ? A.Priority < B.Priority
            : A.Label < B.Label;
    });
    for (const FComparisonCandidate& Candidate : Candidates)
    {
        ComparisonPersonItems.Add(
            MakeShared<FString>(Candidate.Label));
        ComparisonRowNamesByLabel.Add(
            Candidate.Label, Candidate.RowName);
    }
    if (ComparisonPersonCombo.IsValid())
        ComparisonPersonCombo->RefreshOptions();
}

void STMOPPeopleEditor::SelectDefaultComparisonPerson()
{
    ComparisonRowName = NAME_None;
    bHasComparisonRow = false;
    ComparisonTimelineItems.Reset();
    if (ComparisonPersonCombo.IsValid())
        ComparisonPersonCombo->ClearSelection();

    FName PreferredLeaderId = WorkingRow.GroupLeaderEntityId;
    const UDataTable* Table = PeopleTable.Get();
    if (PreferredLeaderId.IsNone() && IsValid(Table) &&
        !WorkingRow.SocialGroupId.IsNone())
        for (const FName RowName : Table->GetRowNames())
            if (const FTMOPPersonProfileRow* Candidate =
                Table->FindRow<FTMOPPersonProfileRow>(
                    RowName, TEXT("TMOPFindGroupLeader"), false))
                if (Candidate->SocialGroupId == WorkingRow.SocialGroupId &&
                    !Candidate->GroupLeaderEntityId.IsNone())
                {
                    PreferredLeaderId = Candidate->GroupLeaderEntityId;
                    break;
                }

    FReferenceItem PreferredItem;
    FReferenceItem SameGroupItem;
    for (const FReferenceItem& Item : ComparisonPersonItems)
    {
        if (!Item.IsValid()) continue;
        const FName* RowName = ComparisonRowNamesByLabel.Find(*Item);
        const FTMOPPersonProfileRow* Candidate =
            IsValid(Table) && RowName != nullptr
            ? Table->FindRow<FTMOPPersonProfileRow>(
                *RowName, TEXT("TMOPChooseComparisonPerson"), false)
            : nullptr;
        if (Candidate == nullptr) continue;
        if (!PreferredLeaderId.IsNone() &&
            Candidate->EntityId == PreferredLeaderId &&
            Candidate->EntityId != WorkingRow.EntityId)
        {
            PreferredItem = Item;
            break;
        }
        if (!WorkingRow.SocialGroupId.IsNone() &&
            Candidate->SocialGroupId == WorkingRow.SocialGroupId &&
            !SameGroupItem.IsValid())
            SameGroupItem = Item;
    }
    if (!PreferredItem.IsValid()) PreferredItem = SameGroupItem;
    if (PreferredItem.IsValid() && ComparisonPersonCombo.IsValid())
        ComparisonPersonCombo->SetSelectedItem(PreferredItem);
    else
        RefreshComparisonTimeline();
}

void STMOPPeopleEditor::RefreshComparisonTimeline()
{
    SelectedComparisonTimelineIndex = INDEX_NONE;
    ComparisonTimelineItems.Reset();
    if (bHasComparisonRow)
        for (int32 Index = 0;
            Index < ComparisonRow.Timeline.Num(); ++Index)
            ComparisonTimelineItems.Add(MakeShared<int32>(Index));
    if (ComparisonTimelineListView.IsValid())
        ComparisonTimelineListView->RequestListRefresh();
}

void STMOPPeopleEditor::SelectPerson(const FName RowName)
{
    CommitEntryEdits();
    UDataTable* Table = PeopleTable.Get();
    const FTMOPPersonProfileRow* Row = IsValid(Table)
        ? Table->FindRow<FTMOPPersonProfileRow>(
            RowName, TEXT("TMOPPeopleEditorSelect"), false)
        : nullptr;
    if (Row == nullptr) return;

    SelectedRowName = RowName;
    WorkingRow = *Row;
    LastSavedRow = *Row;
    bHasLastSavedRow = true;
    RefreshPersonDetailViews();
    SelectedTimelineIndex = INDEX_NONE;
    EntryStructData.Reset();
    EntryDetailsView->SetStructureData(nullptr);
    RefreshTimeline();
    RefreshComparisonPeople();
    SelectDefaultComparisonPerson();
    if (!WorkingRow.Timeline.IsEmpty())
        SelectTimelineEntry(0);
}

void STMOPPeopleEditor::SelectTimelineEntry(const int32 Index)
{
    CommitEntryEdits();
    if (!WorkingRow.Timeline.IsValidIndex(Index)) return;
    SelectedTimelineIndex = Index;
    EntryStructData =
        MakeShared<FStructOnScope>(
            FTMOPPersonTimelineEntry::StaticStruct());
    EntryStructData->SetPackage(
        PeopleTable.IsValid()
            ? PeopleTable->GetOutermost()
            : GetTransientPackage());
    *reinterpret_cast<FTMOPPersonTimelineEntry*>(
        EntryStructData->GetStructMemory()) =
            WorkingRow.Timeline[Index];
    EntryDetailsView->SetStructureData(EntryStructData);
    RefreshReferenceOptions();
}

void STMOPPeopleEditor::CommitEntryEdits()
{
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) ||
        !EntryStructData.IsValid())
        return;
    WorkingRow.Timeline[SelectedTimelineIndex] =
        *reinterpret_cast<FTMOPPersonTimelineEntry*>(
            EntryStructData->GetStructMemory());
    if (TimelineListView.IsValid())
        TimelineListView->RequestListRefresh();
    if (ComparisonTimelineListView.IsValid())
        ComparisonTimelineListView->RequestListRefresh();
}

void STMOPPeopleEditor::RefreshPersonDetailViews()
{
    CharacteristicsStructData = MakeShared<FStructOnScope>(
        FTMOPPersonCharacteristicsEditorData::StaticStruct());
    CharacteristicsStructData->SetPackage(
        PeopleTable.IsValid()
            ? PeopleTable->GetOutermost()
            : GetTransientPackage());
    FTMOPPersonCharacteristicsEditorData* Characteristics =
        reinterpret_cast<FTMOPPersonCharacteristicsEditorData*>(
            CharacteristicsStructData->GetStructMemory());
    Characteristics->AgeAtEvent = WorkingRow.AgeAtEvent;
    Characteristics->HeightCentimeters = WorkingRow.HeightCentimeters;
    Characteristics->AppearanceProfile = WorkingRow.AppearanceProfile;
    Characteristics->ReferenceImage = WorkingRow.ReferenceImage;
    Characteristics->HairColorCategory = WorkingRow.HairColorCategory;
    Characteristics->HeadwearCategory = WorkingRow.HeadwearCategory;
    Characteristics->FacialHairCategory = WorkingRow.FacialHairCategory;
    Characteristics->Hair = WorkingRow.Hair;
    Characteristics->BeardOrMustache = WorkingRow.BeardOrMustache;
    Characteristics->FaceShape = WorkingRow.FaceShape;
    Characteristics->Nose = WorkingRow.Nose;
    Characteristics->Scarf = WorkingRow.Scarf;
    Characteristics->Glasses = WorkingRow.Glasses;
    Characteristics->Headwear = WorkingRow.Headwear;
    Characteristics->BodyBuildCategory = WorkingRow.BodyBuildCategory;
    Characteristics->OuterwearCategory = WorkingRow.OuterwearCategory;
    Characteristics->BodyBuild = WorkingRow.BodyBuild;
    Characteristics->JacketOrCoat = WorkingRow.JacketOrCoat;
    Characteristics->ShirtOrSweater = WorkingRow.ShirtOrSweater;
    Characteristics->Trousers = WorkingRow.Trousers;
    Characteristics->Shoes = WorkingRow.Shoes;
    Characteristics->OtherCharacteristics =
        WorkingRow.OtherCharacteristics;
    CharacteristicsDetailsView->SetStructureData(
        CharacteristicsStructData);

    GeneralStructData = MakeShared<FStructOnScope>(
        FTMOPPersonGeneralEditorData::StaticStruct());
    GeneralStructData->SetPackage(
        PeopleTable.IsValid()
            ? PeopleTable->GetOutermost()
            : GetTransientPackage());
    FTMOPPersonGeneralEditorData* General =
        reinterpret_cast<FTMOPPersonGeneralEditorData*>(
            GeneralStructData->GetStructMemory());
    General->EntityId = WorkingRow.EntityId;
    General->CategoryId = WorkingRow.CategoryId;
    General->FullName = WorkingRow.FullName;
    General->FirstName = WorkingRow.FirstName;
    General->LastName = WorkingRow.LastName;
    General->Gender = WorkingRow.Gender;
    General->Nationality = WorkingRow.Nationality;
    General->Occupation = WorkingRow.Occupation;
    General->HistoricalAddress = WorkingRow.HistoricalAddress;
    General->BirthYear = WorkingRow.BirthYear;
    General->GeneralSourceReference = WorkingRow.GeneralSourceReference;
    General->Uppslag = WorkingRow.Uppslag;
    General->AgentClass = WorkingRow.AgentClass;
    General->bSpawnInSimulation = WorkingRow.bSpawnInSimulation;
    General->bMainCharacter = WorkingRow.bMainCharacter;
    General->MovementProfile = WorkingRow.MovementProfile;
    General->AssociatedVehicleIds = WorkingRow.AssociatedVehicleIds;
    General->SocialGroupId = WorkingRow.SocialGroupId;
    General->GroupLeaderEntityId = WorkingRow.GroupLeaderEntityId;
    General->GroupFormation = WorkingRow.GroupFormation;
    General->GroupFormationSpacingCm =
        WorkingRow.GroupFormationSpacingCm;
    General->bFollowGroupLeaderSchedule =
        WorkingRow.bFollowGroupLeaderSchedule;
    General->Dialog = WorkingRow.Dialog;
    General->AutomaticSpeech = WorkingRow.AutomaticSpeech;
    General->Notes = WorkingRow.Notes;
    GeneralDetailsView->SetStructureData(GeneralStructData);
}

void STMOPPeopleEditor::CommitPersonDetailEdits()
{
    if (CharacteristicsStructData.IsValid())
    {
        const FTMOPPersonCharacteristicsEditorData* Characteristics =
            reinterpret_cast<const FTMOPPersonCharacteristicsEditorData*>(
                CharacteristicsStructData->GetStructMemory());
        WorkingRow.AgeAtEvent = Characteristics->AgeAtEvent;
        WorkingRow.HeightCentimeters = Characteristics->HeightCentimeters;
        WorkingRow.AppearanceProfile = Characteristics->AppearanceProfile;
        WorkingRow.ReferenceImage = Characteristics->ReferenceImage;
        WorkingRow.HairColorCategory = Characteristics->HairColorCategory;
        WorkingRow.HeadwearCategory = Characteristics->HeadwearCategory;
        WorkingRow.FacialHairCategory =
            Characteristics->FacialHairCategory;
        WorkingRow.Hair = Characteristics->Hair;
        WorkingRow.BeardOrMustache = Characteristics->BeardOrMustache;
        WorkingRow.FaceShape = Characteristics->FaceShape;
        WorkingRow.Nose = Characteristics->Nose;
        WorkingRow.Scarf = Characteristics->Scarf;
        WorkingRow.Glasses = Characteristics->Glasses;
        WorkingRow.Headwear = Characteristics->Headwear;
        WorkingRow.BodyBuildCategory =
            Characteristics->BodyBuildCategory;
        WorkingRow.OuterwearCategory =
            Characteristics->OuterwearCategory;
        WorkingRow.BodyBuild = Characteristics->BodyBuild;
        WorkingRow.JacketOrCoat = Characteristics->JacketOrCoat;
        WorkingRow.ShirtOrSweater = Characteristics->ShirtOrSweater;
        WorkingRow.Trousers = Characteristics->Trousers;
        WorkingRow.Shoes = Characteristics->Shoes;
        WorkingRow.OtherCharacteristics =
            Characteristics->OtherCharacteristics;
    }

    if (GeneralStructData.IsValid())
    {
        const FTMOPPersonGeneralEditorData* General =
            reinterpret_cast<const FTMOPPersonGeneralEditorData*>(
                GeneralStructData->GetStructMemory());
        WorkingRow.EntityId = General->EntityId;
        WorkingRow.CategoryId = General->CategoryId;
        WorkingRow.FullName = General->FullName;
        WorkingRow.FirstName = General->FirstName;
        WorkingRow.LastName = General->LastName;
        WorkingRow.Gender = General->Gender;
        WorkingRow.Nationality = General->Nationality;
        WorkingRow.Occupation = General->Occupation;
        WorkingRow.HistoricalAddress = General->HistoricalAddress;
        WorkingRow.BirthYear = General->BirthYear;
        WorkingRow.GeneralSourceReference =
            General->GeneralSourceReference;
        WorkingRow.Uppslag = General->Uppslag;
        WorkingRow.AgentClass = General->AgentClass;
        WorkingRow.bSpawnInSimulation = General->bSpawnInSimulation;
        WorkingRow.bMainCharacter = General->bMainCharacter;
        WorkingRow.MovementProfile = General->MovementProfile;
        WorkingRow.AssociatedVehicleIds =
            General->AssociatedVehicleIds;
        WorkingRow.SocialGroupId = General->SocialGroupId;
        WorkingRow.GroupLeaderEntityId =
            General->GroupLeaderEntityId;
        WorkingRow.GroupFormation = General->GroupFormation;
        WorkingRow.GroupFormationSpacingCm =
            General->GroupFormationSpacingCm;
        WorkingRow.bFollowGroupLeaderSchedule =
            General->bFollowGroupLeaderSchedule;
        WorkingRow.Dialog = General->Dialog;
        WorkingRow.AutomaticSpeech = General->AutomaticSpeech;
        WorkingRow.Notes = General->Notes;
    }
}

bool STMOPPeopleEditor::HasUnsavedPersonChanges()
{
    if (!bHasLastSavedRow || SelectedRowName.IsNone()) return false;

    CommitEntryEdits();
    CommitPersonDetailEdits();
    return !FTMOPPersonProfileRow::StaticStruct()->CompareScriptStruct(
        &WorkingRow, &LastSavedRow, 0);
}

void STMOPPeopleEditor::RestorePersonListSelection()
{
    if (!PersonListView.IsValid()) return;

    TGuardValue<bool> GuardSelectionCallback(
        bRestoringPersonSelection, true);
    for (const FPersonItem& PersonItem : PersonItems)
    {
        if (PersonItem.IsValid() && *PersonItem == SelectedRowName)
        {
            PersonListView->SetSelection(PersonItem);
            PersonListView->RequestScrollIntoView(PersonItem);
            return;
        }
    }
    PersonListView->ClearSelection();
}

const FSlateBrush* STMOPPeopleEditor::GetReferenceImageBrush() const
{
    if (!CharacteristicsStructData.IsValid())
        return FAppStyle::GetBrush("Brushes.Recessed");

    const FTMOPPersonCharacteristicsEditorData* Characteristics =
        reinterpret_cast<const FTMOPPersonCharacteristicsEditorData*>(
            CharacteristicsStructData->GetStructMemory());
    UTexture2D* Texture = Characteristics->ReferenceImage.LoadSynchronous();
    if (!IsValid(Texture))
        return FAppStyle::GetBrush("Brushes.Recessed");

    ReferenceImageBrush.SetResourceObject(Texture);
    ReferenceImageBrush.DrawAs = ESlateBrushDrawType::Image;
    ReferenceImageBrush.SetImageSize(
        FVector2D(Texture->GetSizeX(), Texture->GetSizeY()));
    return &ReferenceImageBrush;
}

EVisibility
STMOPPeopleEditor::GetReferenceImagePlaceholderVisibility() const
{
    if (!CharacteristicsStructData.IsValid())
        return EVisibility::Visible;
    const FTMOPPersonCharacteristicsEditorData* Characteristics =
        reinterpret_cast<const FTMOPPersonCharacteristicsEditorData*>(
            CharacteristicsStructData->GetStructMemory());
    return IsValid(Characteristics->ReferenceImage.LoadSynchronous())
        ? EVisibility::Collapsed
        : EVisibility::Visible;
}

TSharedRef<ITableRow> STMOPPeopleEditor::GeneratePersonRow(
    const FPersonItem Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const UDataTable* Table = PeopleTable.Get();
    const FTMOPPersonProfileRow* Row =
        IsValid(Table) && Item.IsValid()
        ? Table->FindRow<FTMOPPersonProfileRow>(
            *Item, TEXT("TMOPPeopleEditorPersonRow"), false)
        : nullptr;
    const FText Primary = Row != nullptr && !Row->FullName.IsEmpty()
        ? Row->FullName
        : FText::FromString(
            (Item.IsValid() ? *Item : NAME_None).ToString());
    const FText Secondary = Row != nullptr
        ? FText::FromString(Row->EntityId.ToString() +
            TEXT("  •  ") + Row->CategoryId.ToString())
        : FText::GetEmpty();

    return SNew(STableRow<FPersonItem>, OwnerTable)
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(STextBlock).Text(Primary)
        ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(STextBlock)
            .Text(Secondary)
            .ColorAndOpacity(FSlateColor::UseSubduedForeground())
            .Font(FAppStyle::GetFontStyle("SmallFont"))
        ]
    ];
}

TSharedRef<SWidget> STMOPPeopleEditor::GenerateComparisonPersonOption(
    const FReferenceItem Item) const
{
    return SNew(STextBlock)
        .Text(Item.IsValid()
            ? FText::FromString(*Item) : FText::GetEmpty());
}

void STMOPPeopleEditor::HandleComparisonPersonSelected(
    const FReferenceItem Item,
    const ESelectInfo::Type SelectInfo)
{
    if (!Item.IsValid() ||
        SelectInfo == ESelectInfo::Direct &&
        !ComparisonRowNamesByLabel.Contains(*Item))
        return;
    const FName* RowName = ComparisonRowNamesByLabel.Find(*Item);
    const UDataTable* Table = PeopleTable.Get();
    const FTMOPPersonProfileRow* Row =
        IsValid(Table) && RowName != nullptr
        ? Table->FindRow<FTMOPPersonProfileRow>(
            *RowName, TEXT("TMOPSelectComparisonPerson"), false)
        : nullptr;
    if (Row == nullptr) return;
    ComparisonRowName = *RowName;
    ComparisonRow = *Row;
    bHasComparisonRow = true;
    RefreshComparisonTimeline();
}

FText STMOPPeopleEditor::GetComparisonPersonText() const
{
    if (!bHasComparisonRow)
        return LOCTEXT("NoComparisonPerson",
            "Choose person (group leader is automatic)");
    const FString Name = !ComparisonRow.FullName.IsEmpty()
        ? ComparisonRow.FullName.ToString()
        : ComparisonRow.EntityId.ToString();
    const bool bIsLeader =
        (!WorkingRow.GroupLeaderEntityId.IsNone() &&
         ComparisonRow.EntityId == WorkingRow.GroupLeaderEntityId) ||
        (!ComparisonRow.GroupLeaderEntityId.IsNone() &&
         ComparisonRow.EntityId == ComparisonRow.GroupLeaderEntityId);
    return FText::FromString(
        (bIsLeader ? TEXT("GROUP LEADER: ") : TEXT("REFERENCE: ")) +
        Name);
}

TSharedRef<ITableRow>
STMOPPeopleEditor::GenerateComparisonTimelineRow(
    const FTimelineItem Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const int32 Index = Item.IsValid() ? *Item : INDEX_NONE;
    const FTMOPPersonTimelineEntry* Entry =
        bHasComparisonRow && ComparisonRow.Timeline.IsValidIndex(Index)
        ? &ComparisonRow.Timeline[Index] : nullptr;

    int32 ReferenceSecond = 0;
    FString Failure;
    const bool bHasTime = Entry != nullptr &&
        ResolveTimelineDisplaySecondForRow(
            ComparisonRow, Index, ReferenceSecond, &Failure);
    const int32 NormalizedSecond =
        FMath::Max(0, ReferenceSecond) % (24 * 3600);
    const FString TimeLabel = bHasTime
        ? FString::Printf(TEXT("%02d:%02d:%02d"),
            NormalizedSecond / 3600,
            (NormalizedSecond / 60) % 60,
            NormalizedSecond % 60)
        : TEXT("TIME ?");

    FString ComparisonLabel = TEXT("NO TIME MATCH");
    FString ComparisonToolTip = bHasTime
        ? TEXT("No editable timeline row has exactly this resolved time.")
        : Failure;
    FLinearColor ComparisonColor(0.28f, 0.28f, 0.28f);
    int32 ClosestDelta = TNumericLimits<int32>::Max();
    if (bHasTime)
    {
        FString ReferencePositionKey;
        FString ReferencePositionLabel;
        const bool bHasReferencePosition = ResolveTimelinePositionKey(
            ComparisonRow, Index,
            ReferencePositionKey, ReferencePositionLabel);
        bool bFoundSameTime = false;
        bool bFoundSamePosition = false;
        FString CurrentPositionLabel;
        for (int32 CurrentIndex = 0;
            CurrentIndex < WorkingRow.Timeline.Num(); ++CurrentIndex)
        {
            int32 CurrentSecond = 0;
            if (!ResolveTimelineDisplaySecondForRow(
                WorkingRow, CurrentIndex, CurrentSecond))
                continue;
            ClosestDelta = FMath::Min(
                ClosestDelta, FMath::Abs(CurrentSecond - ReferenceSecond));
            if (CurrentSecond != ReferenceSecond) continue;
            bFoundSameTime = true;
            FString CurrentPositionKey;
            if (ResolveTimelinePositionKey(
                WorkingRow, CurrentIndex,
                CurrentPositionKey, CurrentPositionLabel) &&
                bHasReferencePosition &&
                CurrentPositionKey == ReferencePositionKey)
            {
                bFoundSamePosition = true;
                break;
            }
        }
        if (bFoundSamePosition)
        {
            ComparisonLabel = TEXT("SAME TIME + POSITION");
            ComparisonToolTip = FString::Printf(
                TEXT("Both timelines resolve to %s at %s."),
                *TimeLabel, *ReferencePositionLabel);
            ComparisonColor = FLinearColor(0.05f, 0.42f, 0.12f);
        }
        else if (bFoundSameTime)
        {
            ComparisonLabel = TEXT("SAME TIME / CHECK POSITION");
            ComparisonToolTip = bHasReferencePosition
                ? FString::Printf(
                    TEXT("Both timelines resolve to %s, but no matching position was found. Reference position: %s. Editable position: %s."),
                    *TimeLabel, *ReferencePositionLabel,
                    CurrentPositionLabel.IsEmpty()
                        ? TEXT("unknown") : *CurrentPositionLabel)
                : FString::Printf(
                    TEXT("Both timelines resolve to %s, but the reference position is unknown."),
                    *TimeLabel);
            ComparisonColor = FLinearColor(0.78f, 0.38f, 0.03f);
        }
        else if (ClosestDelta != TNumericLimits<int32>::Max())
        {
            ComparisonLabel = FString::Printf(
                TEXT("NEAREST %d s"), ClosestDelta);
            ComparisonToolTip = FString::Printf(
                TEXT("The closest editable timeline event is %d second(s) from this reference event."),
                ClosestDelta);
        }
    }

    FString Summary = Entry != nullptr
        ? ActionLabel(Entry->Action) : FString();
    if (Entry != nullptr)
    {
        if (!Entry->TargetAnchorId.IsNone())
            Summary += TEXT("  →  ") + Entry->TargetAnchorId.ToString();
        else if (!Entry->TargetSeatId.IsNone())
            Summary += TEXT("  →  ") + Entry->TargetSeatId.ToString();
        else if (!Entry->TargetEntityId.IsNone())
            Summary += TEXT("  →  ") + Entry->TargetEntityId.ToString();
    }

    return SNew(STableRow<FTimelineItem>, OwnerTable)
    [
        SNew(SBorder)
        .Padding(FMargin(7.0f, 5.0f))
        .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(FString::Printf(
                        TEXT("%02d"), Index)))
                    .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                ]
                + SHorizontalBox::Slot().FillWidth(1.0f)
                .Padding(8.0f, 0.0f)
                [
                    SNew(STextBlock)
                    .Text(Entry != nullptr
                        ? FText::FromString(Entry->EntryId.ToString())
                        : FText::GetEmpty())
                    .ColorAndOpacity(FLinearColor(0.55f, 0.75f, 1.0f))
                ]
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(TimeLabel))
                    .ColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.05f))
                ]
            ]
            + SVerticalBox::Slot().AutoHeight()
            .Padding(28.0f, 3.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(FText::FromString(Summary))
                .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                .Font(FAppStyle::GetFontStyle("SmallFont"))
            ]
            + SVerticalBox::Slot().AutoHeight()
            .Padding(28.0f, 3.0f, 0.0f, 0.0f)
            [
                SNew(SBorder)
                .Padding(FMargin(5.0f, 1.0f))
                .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
                .BorderBackgroundColor(ComparisonColor)
                .ToolTipText(FText::FromString(ComparisonToolTip))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(ComparisonLabel))
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
    ];
}

TSharedRef<ITableRow> STMOPPeopleEditor::GenerateTimelineRow(
    const FTimelineItem Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const int32 Index = Item.IsValid() ? *Item : INDEX_NONE;
    const FTMOPPersonTimelineEntry* Entry =
        WorkingRow.Timeline.IsValidIndex(Index)
        ? &WorkingRow.Timeline[Index] : nullptr;
    FText SpeedText;
    FText SpeedToolTip;
    FLinearColor SpeedColor = FLinearColor(0.45f, 0.45f, 0.45f);
    const bool bShowSpeedBadge = BuildTimelineSpeedBadge(
        Index, SpeedText, SpeedToolTip, SpeedColor);
    FText ArrivalText;
    FText ArrivalToolTip;
    FLinearColor ArrivalColor = FLinearColor::Transparent;
    const bool bShowArrivalBadge = Entry != nullptr &&
        TMOPRuntimeValidation::BuildArrivalBadge(
            WorkingRow.EntityId,
            Entry->EntryId,
            ArrivalText,
            ArrivalToolTip,
            ArrivalColor);
    int32 ResolvedSecond = 0;
    FString ResolveFailureReason;
    const bool bResolvedTime = ResolveTimelineDisplaySecond(
        Index, ResolvedSecond, &ResolveFailureReason);
    const int32 DisplaySecond = FMath::Max(0, ResolvedSecond) % (24 * 3600);
    const FText ResolvedTimeText = bResolvedTime
        ? FText::FromString(FString::Printf(
            TEXT("%02d:%02d:%02d"),
            DisplaySecond / 3600,
            (DisplaySecond / 60) % 60,
            DisplaySecond % 60))
        : LOCTEXT("TimelineResolvedTimeUnavailable", "TIME ?");
    const FText ResolvedTimeToolTip = bResolvedTime
        ? FText::FromString(FString::Printf(
            TEXT("Resolved timeline time: %s. This is the effective time after applying shared-event or previous-entry offsets."),
            *ResolvedTimeText.ToString()))
        : FText::FromString(ResolveFailureReason);

    return SNew(STableRow<FTimelineItem>, OwnerTable)
    [
        SNew(SBorder)
        .Padding(FMargin(7.0f, 5.0f))
        .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
        [
            SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight()
            [
                SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(
                        FString::Printf(TEXT("%02d"), Index)))
                    .ColorAndOpacity(FSlateColor::UseSubduedForeground())
                ]
                + SHorizontalBox::Slot().FillWidth(1.0f).Padding(8.0f, 0.0f)
                [
                    SNew(STextBlock)
                    .Text(Entry != nullptr
                        ? FText::FromString(Entry->EntryId.ToString())
                        : FText::GetEmpty())
                    .ColorAndOpacity(GetTimelineColor(Index))
                ]
                + SHorizontalBox::Slot().AutoWidth()
                [
                    SNew(STextBlock)
                    .Text(Entry != nullptr
                        ? GetTimelineTimingText(*Entry)
                        : FText::GetEmpty())
                ]
                + SHorizontalBox::Slot()
                .AutoWidth()
                .Padding(8.0f, 0.0f, 0.0f, 0.0f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SBorder)
                        .Visibility(bShowSpeedBadge
                            ? EVisibility::Visible
                            : EVisibility::Collapsed)
                        .Padding(FMargin(5.0f, 1.0f))
                        .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
                        .BorderBackgroundColor(SpeedColor)
                        .ToolTipText(SpeedToolTip)
                        [
                            SNew(STextBlock)
                            .Text(SpeedText)
                            .Font(FAppStyle::GetFontStyle("SmallFont"))
                            .ColorAndOpacity(FLinearColor::White)
                        ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0.0f, 2.0f, 0.0f, 0.0f)
                    [
                        SNew(STextBlock)
                        .Text(ResolvedTimeText)
                        .ToolTipText(ResolvedTimeToolTip)
                        .Justification(ETextJustify::Right)
                        .Font(FAppStyle::GetFontStyle("SmallFont"))
                        .ColorAndOpacity(FLinearColor(1.0f, 0.72f, 0.05f))
                    ]
                ]
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(28.0f, 3.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(GetTimelineSummary(Index))
                .ColorAndOpacity(FSlateColor::UseSubduedForeground())
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(28.0f, 3.0f, 0.0f, 0.0f)
            [
                SNew(SBorder)
                .Visibility(bShowArrivalBadge
                    ? EVisibility::Visible
                    : EVisibility::Collapsed)
                .Padding(FMargin(5.0f, 1.0f))
                .BorderImage(FAppStyle::GetBrush("Brushes.Panel"))
                .BorderBackgroundColor(ArrivalColor)
                .ToolTipText(ArrivalToolTip)
                [
                    SNew(STextBlock)
                    .Text(ArrivalText)
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                    .ColorAndOpacity(FLinearColor::White)
                ]
            ]
        ]
    ];
}

void STMOPPeopleEditor::HandlePersonSelectionChanged(
    const FPersonItem Item, ESelectInfo::Type SelectInfo)
{
    if (bRestoringPersonSelection || !Item.IsValid() ||
        *Item == SelectedRowName)
        return;

    const FName RequestedRowName = *Item;
    if (HasUnsavedPersonChanges())
    {
        const EAppReturnType::Type Choice = FMessageDialog::Open(
            EAppMsgType::YesNoCancel,
            FText::Format(
                LOCTEXT("SaveBeforeChangingPerson",
                    "You have unsaved changes to {0}.\n\nDo you want to save this person?"),
                GetSelectedPersonTitle()),
            LOCTEXT("SavePersonPromptTitle", "Unsaved Person Changes"));

        if (Choice == EAppReturnType::Cancel)
        {
            RestorePersonListSelection();
            return;
        }
        if (Choice == EAppReturnType::Yes && !SaveCurrentPerson())
        {
            RestorePersonListSelection();
            return;
        }
    }

    SelectPerson(RequestedRowName);
    RestorePersonListSelection();
}

void STMOPPeopleEditor::HandleTimelineSelectionChanged(
    const FTimelineItem Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid()) SelectTimelineEntry(*Item);
}

void STMOPPeopleEditor::HandleComparisonTimelineSelectionChanged(
    const FTimelineItem Item,
    const ESelectInfo::Type SelectInfo)
{
    SelectedComparisonTimelineIndex = Item.IsValid()
        ? *Item : INDEX_NONE;
}

int32 STMOPPeopleEditor::FindClosestWorkingTimelineIndex(
    const int32 ReferenceIndex,
    int32* OutDeltaSeconds) const
{
    if (OutDeltaSeconds != nullptr)
        *OutDeltaSeconds = TNumericLimits<int32>::Max();
    if (!bHasComparisonRow ||
        !ComparisonRow.Timeline.IsValidIndex(ReferenceIndex))
        return INDEX_NONE;

    int32 ReferenceSecond = 0;
    if (!ResolveTimelineDisplaySecondForRow(
        ComparisonRow, ReferenceIndex, ReferenceSecond))
        return INDEX_NONE;

    int32 ClosestIndex = INDEX_NONE;
    int32 ClosestDelta = TNumericLimits<int32>::Max();
    for (int32 Index = 0; Index < WorkingRow.Timeline.Num(); ++Index)
    {
        int32 CurrentSecond = 0;
        if (!ResolveTimelineDisplaySecondForRow(
            WorkingRow, Index, CurrentSecond))
            continue;
        const int32 Delta = FMath::Abs(
            CurrentSecond - ReferenceSecond);
        if (Delta < ClosestDelta)
        {
            ClosestDelta = Delta;
            ClosestIndex = Index;
        }
    }
    if (OutDeltaSeconds != nullptr) *OutDeltaSeconds = ClosestDelta;
    return ClosestIndex;
}

TSharedPtr<SWidget>
STMOPPeopleEditor::BuildComparisonTimelineContextMenu()
{
    int32 DeltaSeconds = 0;
    const int32 TargetIndex = FindClosestWorkingTimelineIndex(
        SelectedComparisonTimelineIndex, &DeltaSeconds);
    if (TargetIndex == INDEX_NONE || DeltaSeconds <= 0)
        return nullptr;

    FMenuBuilder MenuBuilder(true, nullptr);
    MenuBuilder.AddMenuEntry(
        LOCTEXT("SyncNearestTimelineTime",
            "Ändra personens tid till referenspersonens tid"),
        FText::Format(
            LOCTEXT("SyncNearestTimelineTimeTip",
                "Ändra Timeline[{0}] med {1} sekunder så att den får exakt samma lösta tid som den valda referensraden."),
            FText::AsNumber(TargetIndex),
            FText::AsNumber(DeltaSeconds)),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateSP(
            this,
            &STMOPPeopleEditor::
                ApplyReferenceTimeToNearestTimelineEntry)));
    return MenuBuilder.MakeWidget();
}

void STMOPPeopleEditor::ApplyReferenceTimeToNearestTimelineEntry()
{
    CommitEntryEdits();
    int32 DeltaSeconds = 0;
    const int32 TargetIndex = FindClosestWorkingTimelineIndex(
        SelectedComparisonTimelineIndex, &DeltaSeconds);
    if (TargetIndex == INDEX_NONE || DeltaSeconds <= 0 ||
        !ComparisonRow.Timeline.IsValidIndex(
            SelectedComparisonTimelineIndex))
        return;

    int32 ReferenceSecond = 0;
    int32 CurrentSecond = 0;
    if (!ResolveTimelineDisplaySecondForRow(
            ComparisonRow, SelectedComparisonTimelineIndex,
            ReferenceSecond) ||
        !ResolveTimelineDisplaySecondForRow(
            WorkingRow, TargetIndex, CurrentSecond))
        return;

    const FTMOPPersonTimelineEntry& ReferenceEntry =
        ComparisonRow.Timeline[SelectedComparisonTimelineIndex];
    FTMOPPersonTimelineEntry& TargetEntry =
        WorkingRow.Timeline[TargetIndex];
    const int32 SignedAdjustment = ReferenceSecond - CurrentSecond;
    const int32 NormalizedReference =
        FMath::Max(0, ReferenceSecond) % (24 * 3600);
    const FString ReferenceTime = FString::Printf(
        TEXT("%02d:%02d:%02d"),
        NormalizedReference / 3600,
        (NormalizedReference / 60) % 60,
        NormalizedReference % 60);
    const EAppReturnType::Type Choice = FMessageDialog::Open(
        EAppMsgType::YesNo,
        FText::FromString(FString::Printf(
            TEXT("Ändra '%s' i Timeline[%d] till referensraden '%s' (%s)?\n\nDen lösta tiden flyttas %s%d sekunder. Tidsmodellen behålls."),
            *TargetEntry.EntryId.ToString(),
            TargetIndex,
            *ReferenceEntry.EntryId.ToString(),
            *ReferenceTime,
            SignedAdjustment >= 0 ? TEXT("+") : TEXT(""),
            SignedAdjustment)));
    if (Choice != EAppReturnType::Yes) return;

    switch (TargetEntry.TimingMode)
    {
    case ETMOPEventTimingMode::Absolute:
    case ETMOPEventTimingMode::Window:
        TargetEntry.Time =
            FTMOPTime::FromSecondsFromMidnight(ReferenceSecond);
        break;
    case ETMOPEventTimingMode::Relative:
        TargetEntry.EventOffsetSeconds += SignedAdjustment;
        break;
    case ETMOPEventTimingMode::RelativeToPreviousEntry:
    {
        int32 PreviousSecond = 0;
        if (TargetIndex <= 0 ||
            !ResolveTimelineDisplaySecondForRow(
                WorkingRow, TargetIndex - 1, PreviousSecond))
            return;
        TargetEntry.EventOffsetSeconds =
            ReferenceSecond - PreviousSecond;
        break;
    }
    default:
        return;
    }

    if (SelectedTimelineIndex == TargetIndex &&
        EntryStructData.IsValid())
    {
        *reinterpret_cast<FTMOPPersonTimelineEntry*>(
            EntryStructData->GetStructMemory()) = TargetEntry;
        EntryDetailsView->SetStructureData(EntryStructData);
    }
    RefreshTimeline();
    SetStatus(FText::FromString(FString::Printf(
        TEXT("Timeline[%d] synkad till %s från %s. Spara personen för att skriva ändringen till DataTable."),
        TargetIndex, *ReferenceTime,
        *ComparisonRow.EntityId.ToString())),
        FLinearColor(0.4f, 1.0f, 0.4f));
}

void STMOPPeopleEditor::HandlePersonSearchChanged(
    const FText& SearchText)
{
    PersonSearch = SearchText.ToString();
    RefreshPeople();
}

ECheckBoxState STMOPPeopleEditor::GetPeopleFilterCheckState(
    const EPeopleCategoryFilter Filter) const
{
    return PeopleCategoryFilter == Filter
        ? ECheckBoxState::Checked
        : ECheckBoxState::Unchecked;
}

void STMOPPeopleEditor::HandlePeopleFilterChanged(
    const ECheckBoxState NewState,
    const EPeopleCategoryFilter Filter)
{
    // One filter is always active. Clicking another checkbox selects it.
    if (NewState != ECheckBoxState::Checked) return;
    PeopleCategoryFilter = Filter;
    RefreshPeople();
}

bool STMOPPeopleEditor::IsMainCharacter(
    const FTMOPPersonProfileRow& Row) const
{
    if (Row.bMainCharacter) return true;

    // These roles remain central even while an alternative scenario
    // deliberately keeps the observed killer unspawned.
    static const TSet<FName> CentralEntityIds = {
        TEXT("OLOF_PALME"),
        TEXT("LISBET_PALME"),
        TEXT("THE_KILLER"),
        TEXT("ANDERS_BJORKMAN")
    };
    if (CentralEntityIds.Contains(Row.EntityId)) return true;
    if (!Row.bSpawnInSimulation) return false;

    // Source-backed people at or immediately around the murder scene.
    static const TSet<FName> SceneCategories = {
        TEXT("MAIN_WITNESSES"),
        TEXT("MURDER_SCENE"),
        TEXT("ANNE_HAGE_COMPANY"),
        TEXT("HANS_JOHANSSON_COMPANY"),
        TEXT("INGE_MORELIUS_COMPANY")
    };
    if (SceneCategories.Contains(Row.CategoryId)) return true;

    const FString Category = Row.CategoryId.ToString();
    const bool bFirstResponder =
        Category.Equals(TEXT("POLICE"), ESearchCase::IgnoreCase) ||
        Category.Equals(TEXT("POLIS"), ESearchCase::IgnoreCase) ||
        Category.Equals(TEXT("AMBULANCE"), ESearchCase::IgnoreCase);
    if (!bFirstResponder) return false;

    // Police and ambulance personnel are included only when their own
    // timeline actually references arrival or work at the crime scene.
    for (const FTMOPPersonTimelineEntry& Entry : Row.Timeline)
    {
        FString SceneReference = Entry.EntryId.ToString() + TEXT(" ") +
            Entry.SharedEventId.ToString() + TEXT(" ") +
            Entry.TargetAnchorId.ToString();
        for (const FName PassAnchorId : Entry.PassAnchorIds)
            SceneReference += TEXT(" ") + PassAnchorId.ToString();

        if (SceneReference.Contains(
                TEXT("CRIME_SCENE"), ESearchCase::IgnoreCase) ||
            SceneReference.Contains(
                TEXT("CRIMESCENE"), ESearchCase::IgnoreCase) ||
            SceneReference.Contains(
                TEXT("MORDPLATS"), ESearchCase::IgnoreCase) ||
            SceneReference.Contains(
                TEXT("DEKORIMA"), ESearchCase::IgnoreCase))
            return true;
    }
    return false;
}

void STMOPPeopleEditor::RefreshReferenceOptions()
{
    AnchorReferenceItems.Reset();
    EntityReferenceItems.Reset();
    GroupReferenceItems.Reset();
    SeatReferenceItems.Reset();
    EventReferenceItems.Reset();
    ReferenceIdsByLabel.Reset();

    auto AddOption = [this](
        TArray<FReferenceItem>& Items,
        const FName Id,
        const FString& Details)
    {
        if (Id.IsNone()) return;
        FString Label = Id.ToString();
        if (!Details.IsEmpty())
        {
            Label += TEXT("  |  ") + Details;
        }
        ReferenceIdsByLabel.Add(Label, Id);
        Items.Add(MakeShared<FString>(MoveTemp(Label)));
    };
    TSet<FName> KnownGroupIds;
    auto AddGroupOption = [this, &AddOption, &KnownGroupIds](
        const FName GroupId, const FString& Details)
    {
        if (GroupId.IsNone() || KnownGroupIds.Contains(GroupId)) return;
        KnownGroupIds.Add(GroupId);
        AddOption(GroupReferenceItems, GroupId, Details);
    };

    UWorld* EditorWorld = GEditor != nullptr
        ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (EditorWorld != nullptr)
    {
        for (TActorIterator<ATMOPHistoricalAnchor> It(EditorWorld); It; ++It)
        {
            const ATMOPHistoricalAnchor* Anchor = *It;
            FString Details = Anchor->DisplayName.ToString();
            if (const UEnum* Enum = StaticEnum<ETMOPAnchorCategory>())
            {
                const FString Category = Enum->GetDisplayNameTextByValue(
                    static_cast<int64>(Anchor->AnchorCategory)).ToString();
                Details = Details.IsEmpty()
                    ? Category : Details + TEXT("  •  ") + Category;
            }
            AddOption(
                AnchorReferenceItems, Anchor->GetAnchorId(), Details);
        }

        for (TActorIterator<AActor> It(EditorWorld); It; ++It)
        {
            TArray<UTMOPCinemaSeatComponent*> CinemaSeats;
            It->GetComponents<UTMOPCinemaSeatComponent>(CinemaSeats);
            for (const UTMOPCinemaSeatComponent* Seat : CinemaSeats)
            {
                AddOption(SeatReferenceItems, Seat->SeatId,
                    FString::Printf(TEXT("Cinema  •  %s"),
                        *It->GetActorLabel()));
            }

            TArray<UTMOPVehicleSeatComponent*> VehicleSeats;
            It->GetComponents<UTMOPVehicleSeatComponent>(VehicleSeats);
            for (const UTMOPVehicleSeatComponent* Seat : VehicleSeats)
            {
                FString Role = TEXT("Vehicle");
                if (const UEnum* Enum =
                    StaticEnum<ETMOPVehicleSeatRole>())
                {
                    Role = Enum->GetDisplayNameTextByValue(
                        static_cast<int64>(Seat->SeatRole)).ToString();
                }
                AddOption(SeatReferenceItems, Seat->SeatId,
                    FString::Printf(TEXT("%s  •  %s"),
                        *Role, *It->GetActorLabel()));
            }
        }
    }

    if (const UDataTable* Table = PeopleTable.Get())
    {
        for (const FName RowName : Table->GetRowNames())
        {
            if (const FTMOPPersonProfileRow* Row =
                Table->FindRow<FTMOPPersonProfileRow>(
                    RowName, TEXT("TMOPReferencePeople"), false))
            {
                AddOption(EntityReferenceItems, Row->EntityId,
                    Row->FullName.ToString() + TEXT("  •  Person"));
                AddGroupOption(Row->SocialGroupId, TEXT("Social group"));
                for (const FTMOPPersonTimelineEntry& TimelineEntry :
                    Row->Timeline)
                {
                    AddGroupOption(
                        TimelineEntry.TargetGroupId,
                        TEXT("Timeline group"));
                    AddGroupOption(
                        TimelineEntry.GroupDefinition.GroupId,
                        TEXT("Created group"));
                    for (const FTMOPGroupDefinition& SplitGroup :
                        TimelineEntry.SplitGroupDefinitions)
                        AddGroupOption(
                            SplitGroup.GroupId,
                            TEXT("Split group"));
                }
            }
        }
    }

    if (const UDataTable* Table = VehicleTable.Get())
    {
        if (Table->GetRowStruct() ==
            FTMOPHistoricalVehicleRow::StaticStruct())
        {
            for (const FName RowName : Table->GetRowNames())
            {
                if (const FTMOPHistoricalVehicleRow* Row =
                    Table->FindRow<FTMOPHistoricalVehicleRow>(
                        RowName, TEXT("TMOPReferenceVehicles"), false))
                {
                    AddOption(EntityReferenceItems, Row->VehicleId,
                        Row->DisplayName.ToString() +
                        TEXT("  •  Vehicle"));
                }
            }
        }
    }

    if (const UDataTable* Table = EventTable.Get())
    {
        if (Table->GetRowStruct() ==
            FTMOPHistoricalEventDefinition::StaticStruct())
        {
            for (const FName RowName : Table->GetRowNames())
            {
                if (const FTMOPHistoricalEventDefinition* Row =
                    Table->FindRow<FTMOPHistoricalEventDefinition>(
                        RowName, TEXT("TMOPReferenceEvents"), false))
                {
                    AddOption(EventReferenceItems, Row->EventId,
                        Row->DisplayName.ToString());
                }
            }
        }
    }

    auto SortItems = [](TArray<FReferenceItem>& Items)
    {
        Items.Sort([](
            const FReferenceItem& A,
            const FReferenceItem& B)
        {
            return A.IsValid() && B.IsValid()
                ? *A < *B : A.IsValid();
        });
    };
    SortItems(AnchorReferenceItems);
    SortItems(EntityReferenceItems);
    SortItems(GroupReferenceItems);
    SortItems(SeatReferenceItems);
    SortItems(EventReferenceItems);

    if (AnchorReferenceCombo.IsValid())
        AnchorReferenceCombo->RefreshOptions();
    if (EntityReferenceCombo.IsValid())
        EntityReferenceCombo->RefreshOptions();
    if (GroupReferenceCombo.IsValid())
        GroupReferenceCombo->RefreshOptions();
    if (SeatReferenceCombo.IsValid())
        SeatReferenceCombo->RefreshOptions();
    if (EventReferenceCombo.IsValid())
        EventReferenceCombo->RefreshOptions();
}

TSharedRef<SWidget> STMOPPeopleEditor::GenerateReferenceOption(
    const FReferenceItem Item) const
{
    return SNew(STextBlock)
        .Text(Item.IsValid()
            ? FText::FromString(*Item)
            : FText::GetEmpty())
        .ToolTipText(Item.IsValid()
            ? FText::FromString(*Item)
            : FText::GetEmpty());
}

FName STMOPPeopleEditor::GetReferenceId(
    const FReferenceItem Item) const
{
    if (!Item.IsValid()) return NAME_None;
    if (const FName* Id = ReferenceIdsByLabel.Find(*Item))
        return *Id;
    return NAME_None;
}

void STMOPPeopleEditor::HandleReferenceSelected(
    const FReferenceItem Item,
    const ESelectInfo::Type SelectInfo,
    const EReferenceField Field)
{
    if (!EntryStructData.IsValid()) return;
    const FName Id = GetReferenceId(Item);
    if (Id.IsNone()) return;

    FTMOPPersonTimelineEntry* Entry =
        reinterpret_cast<FTMOPPersonTimelineEntry*>(
            EntryStructData->GetStructMemory());
    switch (Field)
    {
    case EReferenceField::TargetAnchor:
        Entry->TargetAnchorId = Id;
        if (Entry->Action == ETMOPPersonTimelineAction::Interact ||
            Entry->Action ==
                ETMOPPersonTimelineAction::PlayUniqueAnimation ||
            Entry->Action == ETMOPPersonTimelineAction::LookAtAnchor)
            Entry->ConversationTargetMode =
                ETMOPConversationTargetMode::Anchor;
        break;
    case EReferenceField::TargetEntity:
        Entry->TargetEntityId = Id;
        if (Entry->Action == ETMOPPersonTimelineAction::Interact ||
            Entry->Action ==
                ETMOPPersonTimelineAction::PlayUniqueAnimation ||
            Entry->Action == ETMOPPersonTimelineAction::LookAtAnchor)
            Entry->ConversationTargetMode =
                ETMOPConversationTargetMode::SpecificPerson;
        break;
    case EReferenceField::TargetGroup:
        Entry->TargetGroupId = Id;
        if (Entry->Action == ETMOPPersonTimelineAction::Interact ||
            Entry->Action ==
                ETMOPPersonTimelineAction::PlayUniqueAnimation ||
            Entry->Action == ETMOPPersonTimelineAction::LookAtAnchor)
            Entry->ConversationTargetMode =
                ETMOPConversationTargetMode::Group;
        break;
    case EReferenceField::TargetSeat:
        Entry->TargetSeatId = Id;
        break;
    case EReferenceField::SharedEvent:
        Entry->SharedEventId = Id;
        break;
    }
    if (EntryDetailsView.IsValid())
    {
        EntryDetailsView->SetStructureData(EntryStructData);
    }
    SetStatus(
        FText::Format(
            LOCTEXT("ReferenceSelected", "Selected {0}."),
            FText::FromName(Id)),
        FLinearColor(0.55f, 0.8f, 0.55f));
}

FText STMOPPeopleEditor::GetReferenceFieldText(
    const EReferenceField Field) const
{
    if (!EntryStructData.IsValid())
        return LOCTEXT("NoTimelineEntry", "Select a timeline entry");
    const FTMOPPersonTimelineEntry* Entry =
        reinterpret_cast<const FTMOPPersonTimelineEntry*>(
            EntryStructData->GetStructMemory());
    FName Id = NAME_None;
    switch (Field)
    {
    case EReferenceField::TargetAnchor:
        Id = Entry->TargetAnchorId;
        break;
    case EReferenceField::TargetEntity:
        Id = Entry->TargetEntityId;
        break;
    case EReferenceField::TargetGroup:
        Id = Entry->TargetGroupId;
        break;
    case EReferenceField::TargetSeat:
        Id = Entry->TargetSeatId;
        break;
    case EReferenceField::SharedEvent:
        Id = Entry->SharedEventId;
        break;
    }
    return Id.IsNone()
        ? LOCTEXT("SearchReference", "Type to search...")
        : FText::FromName(Id);
}

FReply STMOPPeopleEditor::AddTimelineEntry()
{
    if (SelectedRowName.IsNone()) return FReply::Handled();
    CommitEntryEdits();
    FTMOPPersonTimelineEntry Entry;
    Entry.Action = WorkingRow.Timeline.IsEmpty()
        ? ETMOPPersonTimelineAction::InitialPlacement
        : ETMOPPersonTimelineAction::MoveToAnchor;
    Entry.LocationType = ETMOPPersonLocationType::Anchor;
    Entry.ActivityState = Entry.Action ==
        ETMOPPersonTimelineAction::MoveToAnchor
        ? ETMOPAgentActivityState::Walking
        : ETMOPAgentActivityState::Idle;
    Entry.EntryId = FName(*FString::Printf(
        TEXT("%s_ENTRY_%02d"),
        *WorkingRow.EntityId.ToString(),
        WorkingRow.Timeline.Num()));
    WorkingRow.Timeline.Add(Entry);
    RefreshTimeline();
    SelectTimelineEntry(WorkingRow.Timeline.Num() - 1);
    return FReply::Handled();
}

FReply STMOPPeopleEditor::DuplicateTimelineEntry()
{
    CommitEntryEdits();
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        return FReply::Handled();
    FTMOPPersonTimelineEntry Copy =
        WorkingRow.Timeline[SelectedTimelineIndex];
    Copy.EntryId = FName(*(Copy.EntryId.ToString() + TEXT("_COPY")));
    const int32 NewIndex = SelectedTimelineIndex + 1;
    WorkingRow.Timeline.Insert(Copy, NewIndex);
    RefreshTimeline();
    SelectTimelineEntry(NewIndex);
    return FReply::Handled();
}

FReply STMOPPeopleEditor::DeleteTimelineEntry()
{
    CommitEntryEdits();
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        return FReply::Handled();
    WorkingRow.Timeline.RemoveAt(SelectedTimelineIndex);
    const int32 NextIndex = FMath::Min(
        SelectedTimelineIndex,
        WorkingRow.Timeline.Num() - 1);
    SelectedTimelineIndex = INDEX_NONE;
    EntryStructData.Reset();
    EntryDetailsView->SetStructureData(nullptr);
    RefreshTimeline();
    if (WorkingRow.Timeline.IsValidIndex(NextIndex))
        SelectTimelineEntry(NextIndex);
    return FReply::Handled();
}

FReply STMOPPeopleEditor::MoveTimelineEntryUp()
{
    CommitEntryEdits();
    if (SelectedTimelineIndex <= 1 ||
        !WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        return FReply::Handled();
    const int32 NewIndex = SelectedTimelineIndex - 1;
    WorkingRow.Timeline.Swap(SelectedTimelineIndex, NewIndex);
    SelectedTimelineIndex = INDEX_NONE;
    EntryStructData.Reset();
    EntryDetailsView->SetStructureData(nullptr);
    RefreshTimeline();
    SelectTimelineEntry(NewIndex);
    return FReply::Handled();
}

FReply STMOPPeopleEditor::MoveTimelineEntryDown()
{
    CommitEntryEdits();
    if (SelectedTimelineIndex <= 0 ||
        !WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex + 1))
        return FReply::Handled();
    const int32 NewIndex = SelectedTimelineIndex + 1;
    WorkingRow.Timeline.Swap(SelectedTimelineIndex, NewIndex);
    SelectedTimelineIndex = INDEX_NONE;
    EntryStructData.Reset();
    EntryDetailsView->SetStructureData(nullptr);
    RefreshTimeline();
    SelectTimelineEntry(NewIndex);
    return FReply::Handled();
}

FReply STMOPPeopleEditor::SavePerson()
{
    SaveCurrentPerson();
    return FReply::Handled();
}

bool STMOPPeopleEditor::SaveCurrentPerson()
{
    CommitEntryEdits();
    CommitPersonDetailEdits();
    UDataTable* Table = PeopleTable.Get();
    if (!IsValid(Table) || SelectedRowName.IsNone())
        return false;

    const TArray<FString> Errors = ValidateWorkingRow();
    if (!Errors.IsEmpty())
    {
        SetStatus(
            FText::FromString(TEXT("Not saved: ") + Errors[0]),
            FLinearColor::Red);
        return false;
    }

    Table->Modify();
    if (WorkingRow.EntityId != SelectedRowName)
    {
        if (Table->GetRowMap().Contains(WorkingRow.EntityId))
        {
            SetStatus(
                FText::Format(
                    LOCTEXT("DuplicateEntityId",
                        "Not saved: a row named '{0}' already exists."),
                    FText::FromString(WorkingRow.EntityId.ToString())),
                FLinearColor::Red);
            return false;
        }

        Table->AddRow(WorkingRow.EntityId, WorkingRow);
        Table->RemoveRow(SelectedRowName);
        SelectedRowName = WorkingRow.EntityId;
    }
    else
    {
        FTMOPPersonProfileRow* Existing =
            Table->FindRow<FTMOPPersonProfileRow>(
                SelectedRowName, TEXT("TMOPPeopleEditorSave"), false);
        if (Existing == nullptr)
        {
            SetStatus(
                LOCTEXT("MissingSelectedRow",
                    "Not saved: the selected DataTable row no longer exists."),
                FLinearColor::Red);
            return false;
        }
        *Existing = WorkingRow;
    }

    Table->MarkPackageDirty();
    Table->PostEditChange();
    LastSavedRow = WorkingRow;
    bHasLastSavedRow = true;

    RefreshPeople();
    RefreshTimeline();
    SetStatus(
        LOCTEXT("Saved",
            "Person saved to DT_TMOP_People. Save the project to write the asset to disk."),
        FLinearColor(0.4f, 1.0f, 0.4f));
    return true;
}

FReply STMOPPeopleEditor::ReloadPerson()
{
    if (!SelectedRowName.IsNone()) SelectPerson(SelectedRowName);
    return FReply::Handled();
}

FReply STMOPPeopleEditor::ResolveCurrentAppearance()
{
    if (SelectedRowName.IsNone())
    {
        SetStatus(LOCTEXT("NoAppearancePersonSelected",
            "Select a person before generating an appearance."),
            FLinearColor::Red);
        return FReply::Handled();
    }
    CommitEntryEdits();
    CommitPersonDetailEdits();
    if (!AppearanceTable.IsValid())
        AppearanceTable = LoadObject<UDataTable>(
            nullptr, DefaultAppearanceTablePath);
    UDataTable* Catalog = AppearanceTable.Get();
    if (!IsValid(Catalog) || Catalog->GetRowStruct() !=
        FTMOPAppearanceAssetRow::StaticStruct())
    {
        SetStatus(LOCTEXT("MissingAppearanceCatalog",
            "Appearance catalog is missing. Run Setup/05_DataTableBuilder first."),
            FLinearColor::Red);
        return FReply::Handled();
    }

    FTMOPResolvedAppearance Resolved;
    UTMOPAppearanceResolver::ResolveAppearance(WorkingRow, Catalog, Resolved);
    int32 UnknownParts = 0;
    const TArray<FString> Warnings =
        ValidateAppearanceRow(WorkingRow, Catalog, &UnknownParts);
    const FString WarningSuffix = Warnings.IsEmpty()
        ? FString() : FString::Printf(TEXT(" | WARNING: %s"), *Warnings[0]);
    const FString Summary = FString::Printf(
        TEXT("Appearance: body %s | face %s | coat %s | upper %s | trousers %s | shoes %s | %d unknown part(s)%s"),
        *Resolved.Body.CatalogId.ToString(), *Resolved.Face.CatalogId.ToString(),
        *Resolved.Outerwear.CatalogId.ToString(),
        *Resolved.UpperBody.CatalogId.ToString(),
        *Resolved.Trousers.CatalogId.ToString(),
        *Resolved.Footwear.CatalogId.ToString(), UnknownParts,
        *WarningSuffix);
    SetStatus(FText::FromString(Summary), Warnings.IsEmpty()
        ? FLinearColor(0.4f, 1.0f, 0.4f)
        : FLinearColor(1.0f, 0.65f, 0.15f));
    return FReply::Handled();
}

FReply STMOPPeopleEditor::ValidateAllAppearances()
{
    if (!AppearanceTable.IsValid())
        AppearanceTable = LoadObject<UDataTable>(
            nullptr, DefaultAppearanceTablePath);
    UDataTable* Table = PeopleTable.Get();
    UDataTable* Catalog = AppearanceTable.Get();
    if (!IsValid(Table) || !IsValid(Catalog) ||
        Catalog->GetRowStruct() != FTMOPAppearanceAssetRow::StaticStruct())
    {
        SetStatus(LOCTEXT("CannotValidateAppearances",
            "Cannot validate appearances: people table or appearance catalog is missing."),
            FLinearColor::Red);
        return FReply::Handled();
    }

    int32 AffectedPeople = 0;
    int32 WarningCount = 0;
    int32 UnknownPartCount = 0;
    for (const FName RowName : Table->GetRowNames())
    {
        const FTMOPPersonProfileRow* Row =
            Table->FindRow<FTMOPPersonProfileRow>(
                RowName, TEXT("TMOP appearance batch validation"), false);
        if (Row == nullptr) continue;
        int32 PersonUnknownParts = 0;
        const TArray<FString> Warnings =
            ValidateAppearanceRow(*Row, Catalog, &PersonUnknownParts);
        UnknownPartCount += PersonUnknownParts;
        if (!Warnings.IsEmpty())
        {
            ++AffectedPeople;
            WarningCount += Warnings.Num();
            for (const FString& Warning : Warnings)
                UE_LOG(LogTemp, Warning, TEXT("TMOP Appearance [%s]: %s"),
                    *RowName.ToString(), *Warning);
        }
    }

    const FString Summary = FString::Printf(
        TEXT("Appearance validation: %d people, %d warning(s) affecting %d people, %d source-unknown parts. Details are in Output Log."),
        Table->GetRowNames().Num(), WarningCount, AffectedPeople,
        UnknownPartCount);
    SetStatus(FText::FromString(Summary), WarningCount == 0
        ? FLinearColor(0.4f, 1.0f, 0.4f)
        : FLinearColor(1.0f, 0.65f, 0.15f));
    return FReply::Handled();
}

FText STMOPPeopleEditor::GetSelectedPersonTitle() const
{
    if (SelectedRowName.IsNone())
        return LOCTEXT("NoPerson", "TMOP People Editor");
    return !WorkingRow.FullName.IsEmpty()
        ? WorkingRow.FullName
        : FText::FromString(WorkingRow.EntityId.ToString());
}

FText STMOPPeopleEditor::GetSelectedPersonSubtitle() const
{
    if (SelectedRowName.IsNone())
        return FText::FromString(DefaultPeopleTablePath);
    return FText::FromString(
        WorkingRow.EntityId.ToString() + TEXT("  •  ") +
        WorkingRow.CategoryId.ToString() + TEXT("  •  ") +
        FString::Printf(TEXT("%d timeline entries"),
            WorkingRow.Timeline.Num()));
}

FText STMOPPeopleEditor::GetTimelineSummary(const int32 Index) const
{
    if (!WorkingRow.Timeline.IsValidIndex(Index))
        return FText::GetEmpty();
    const FTMOPPersonTimelineEntry& Entry =
        WorkingRow.Timeline[Index];
    FString Summary = Entry.Usage == ETMOPPersonTimelineUsage::DocumentationOnly
        ? TEXT("[DOC] ") : Entry.Usage == ETMOPPersonTimelineUsage::DocumentationAndSimulation
        ? TEXT("[DOC+SIM] ") : TEXT("[SIM] ");
    if (Entry.AnchorReferenceMode == ETMOPAnchorReferenceMode::PlannedFuture)
        Summary += TEXT("[PLANNED] ");
    Summary += ActionLabel(Entry.Action);
    if ((Entry.Action == ETMOPPersonTimelineAction::Interact ||
         Entry.Action == ETMOPPersonTimelineAction::PlayUniqueAnimation ||
         Entry.Action == ETMOPPersonTimelineAction::LookAtAnchor) &&
        Entry.ConversationTargetMode ==
            ETMOPConversationTargetMode::Group &&
        !Entry.TargetGroupId.IsNone())
        Summary += TEXT("  → group ") + Entry.TargetGroupId.ToString();
    else if (Entry.Action == ETMOPPersonTimelineAction::LookAtAnchor &&
        (Entry.ConversationTargetMode ==
            ETMOPConversationTargetMode::SpecificPerson ||
         (Entry.ConversationTargetMode ==
            ETMOPConversationTargetMode::Automatic &&
          !Entry.TargetEntityId.IsNone())) &&
        !Entry.TargetEntityId.IsNone())
        Summary += TEXT("  →  ") + Entry.TargetEntityId.ToString();
    else if (!Entry.TargetAnchorId.IsNone())
        Summary += TEXT("  →  ") + Entry.TargetAnchorId.ToString();
    else if (!Entry.TargetSeatId.IsNone())
        Summary += TEXT("  →  ") + Entry.TargetSeatId.ToString();
    else if (!Entry.TargetEntityId.IsNone())
        Summary += TEXT("  →  ") + Entry.TargetEntityId.ToString();
    if (!Entry.PassAnchorIds.IsEmpty())
        Summary += FString::Printf(
            TEXT("  •  via %d anchor(s)"),
            Entry.PassAnchorIds.Num());
    FString Error;
    if (EntryHasError(Index, &Error))
        Summary += TEXT("  ⚠  ") + Error;
    return FText::FromString(Summary);
}

FText STMOPPeopleEditor::GetTimelineTimingText(
    const FTMOPPersonTimelineEntry& Entry) const
{
    if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
    {
        FString Result = TEXT("@ ") + Entry.SharedEventId.ToString();
        if (Entry.EventOffsetSeconds != 0)
            Result += FString::Printf(
                TEXT(" %s%d s"),
                Entry.EventOffsetSeconds > 0 ? TEXT("+") : TEXT(""),
                Entry.EventOffsetSeconds);
        if (Entry.bTimeIsArrival) Result += TEXT("  ARRIVAL");
        return FText::FromString(Result);
    }
    if (Entry.TimingMode ==
        ETMOPEventTimingMode::RelativeToPreviousEntry)
    {
        FString Result = TEXT("Previous");
        if (Entry.EventOffsetSeconds != 0)
            Result += FString::Printf(
                TEXT(" %s%d s"),
                Entry.EventOffsetSeconds > 0 ? TEXT("+") : TEXT(""),
                Entry.EventOffsetSeconds);
        if (Entry.bTimeIsArrival) Result += TEXT("  ARRIVAL");
        return FText::FromString(Result);
    }
    FString Result = Entry.Time.ToDisplayString();
    if (Entry.bTimeIsArrival) Result += TEXT("  ARRIVAL");
    return FText::FromString(Result);
}

bool STMOPPeopleEditor::ResolveTimelineDisplaySecond(
    const int32 Index,
    int32& OutSecond,
    FString* OutFailureReason) const
{
    return ResolveTimelineDisplaySecondForRow(
        WorkingRow, Index, OutSecond, OutFailureReason);
}

bool STMOPPeopleEditor::ResolveTimelineDisplaySecondForRow(
    const FTMOPPersonProfileRow& Row,
    const int32 Index,
    int32& OutSecond,
    FString* OutFailureReason) const
{
    if (OutFailureReason != nullptr) OutFailureReason->Reset();
    auto Fail = [OutFailureReason](const FString& Reason)
    {
        if (OutFailureReason != nullptr && OutFailureReason->IsEmpty())
            *OutFailureReason = Reason;
        return false;
    };

    if (!Row.Timeline.IsValidIndex(Index))
        return Fail(TEXT("The timeline row does not exist."));

    const UDataTable* Events = EventTable.Get();
    TMap<FName, int32> EventSecondCache;
    TSet<FName> ResolvingEvents;
    TFunction<bool(FName, int32&)> ResolveEventSecond;
    ResolveEventSecond = [&](const FName EventId, int32& EventSecond)
    {
        if (const int32* Cached = EventSecondCache.Find(EventId))
        {
            EventSecond = *Cached;
            return true;
        }
        if (EventId.IsNone())
            return Fail(TEXT("The row has no Shared Event ID."));
        if (!IsValid(Events))
            return Fail(TEXT("DT_TMOP_HistoricalEvents could not be loaded."));
        if (ResolvingEvents.Contains(EventId))
            return Fail(FString::Printf(
                TEXT("Shared-event cycle detected at '%s'."),
                *EventId.ToString()));

        const FTMOPHistoricalEventDefinition* Definition =
            Events->FindRow<FTMOPHistoricalEventDefinition>(
                EventId, TEXT("TMOPTimelineResolvedTimeEvent"), false);
        if (Definition == nullptr)
            for (const FName RowName : Events->GetRowNames())
                if (const FTMOPHistoricalEventDefinition* Candidate =
                    Events->FindRow<FTMOPHistoricalEventDefinition>(
                        RowName,
                        TEXT("TMOPTimelineResolvedTimeEventById"), false))
                    if (Candidate->EventId == EventId)
                    {
                        Definition = Candidate;
                        break;
                    }
        if (Definition == nullptr)
            return Fail(FString::Printf(
                TEXT("Shared event '%s' was not found in DT_TMOP_HistoricalEvents."),
                *EventId.ToString()));

        ResolvingEvents.Add(EventId);
        bool bResolved = true;
        switch (Definition->TimingMode)
        {
        case ETMOPEventTimingMode::Absolute:
            EventSecond =
                Definition->AbsoluteTime.ToSecondsFromMidnight();
            break;
        case ETMOPEventTimingMode::Window:
            EventSecond =
                Definition->PreferredTime.ToSecondsFromMidnight();
            break;
        case ETMOPEventTimingMode::Relative:
        {
            int32 TriggerSecond = 0;
            bResolved = ResolveEventSecond(
                Definition->TriggerEventId, TriggerSecond);
            if (bResolved)
                EventSecond = TriggerSecond +
                    Definition->PreferredDelaySeconds;
            break;
        }
        case ETMOPEventTimingMode::RelativeToPreviousEntry:
        default:
            bResolved = Fail(FString::Printf(
                TEXT("Shared event '%s' has no editor-resolvable time mode."),
                *EventId.ToString()));
            break;
        }
        ResolvingEvents.Remove(EventId);
        if (bResolved)
            EventSecondCache.Add(EventId, EventSecond);
        return bResolved;
    };

    TMap<int32, int32> TimelineSecondCache;
    TSet<int32> ResolvingEntries;
    TFunction<bool(int32, int32&)> ResolveEntrySecond;
    ResolveEntrySecond = [&](const int32 EntryIndex, int32& EntrySecond)
    {
        if (!Row.Timeline.IsValidIndex(EntryIndex))
            return Fail(TEXT("A preceding timeline row is missing."));
        if (const int32* Cached = TimelineSecondCache.Find(EntryIndex))
        {
            EntrySecond = *Cached;
            return true;
        }
        if (ResolvingEntries.Contains(EntryIndex))
            return Fail(FString::Printf(
                TEXT("Timeline timing cycle detected at row %d."),
                EntryIndex));

        ResolvingEntries.Add(EntryIndex);
        const FTMOPPersonTimelineEntry& Entry =
            Row.Timeline[EntryIndex];
        bool bResolved = true;
        switch (Entry.TimingMode)
        {
        case ETMOPEventTimingMode::Absolute:
        case ETMOPEventTimingMode::Window:
            EntrySecond = Entry.Time.ToSecondsFromMidnight();
            break;
        case ETMOPEventTimingMode::Relative:
        {
            int32 EventSecond = 0;
            bResolved = ResolveEventSecond(
                Entry.SharedEventId, EventSecond);
            if (bResolved)
                EntrySecond = EventSecond + Entry.EventOffsetSeconds;
            break;
        }
        case ETMOPEventTimingMode::RelativeToPreviousEntry:
        {
            int32 PreviousSecond = 0;
            bResolved = EntryIndex > 0
                ? ResolveEntrySecond(EntryIndex - 1, PreviousSecond)
                : Fail(TEXT(
                    "The first timeline row cannot be relative to a previous row."));
            if (bResolved)
                EntrySecond = PreviousSecond + Entry.EventOffsetSeconds;
            break;
        }
        default:
            bResolved = Fail(FString::Printf(
                TEXT("Timeline row %d has an unsupported timing mode."),
                EntryIndex));
            break;
        }
        ResolvingEntries.Remove(EntryIndex);
        if (bResolved)
            TimelineSecondCache.Add(EntryIndex, EntrySecond);
        return bResolved;
    };

    return ResolveEntrySecond(Index, OutSecond);
}

bool STMOPPeopleEditor::ResolveTimelinePositionKey(
    const FTMOPPersonProfileRow& Row,
    const int32 Index,
    FString& OutKey,
    FString& OutLabel) const
{
    OutKey.Reset();
    OutLabel.Reset();
    if (!Row.Timeline.IsValidIndex(Index)) return false;

    for (int32 CandidateIndex = Index;
        CandidateIndex >= 0; --CandidateIndex)
    {
        const FTMOPPersonTimelineEntry& Entry =
            Row.Timeline[CandidateIndex];
        if (Entry.LocationType == ETMOPPersonLocationType::NotPresent ||
            Entry.Action == ETMOPPersonTimelineAction::Despawn)
        {
            OutKey = TEXT("NOT_PRESENT");
            OutLabel = TEXT("not present");
            return true;
        }
        if (Entry.LocationType ==
            ETMOPPersonLocationType::WorldTransform)
        {
            const FVector Location = Entry.WorldTransform.GetLocation();
            const int32 X = FMath::RoundToInt(Location.X / 100.0f);
            const int32 Y = FMath::RoundToInt(Location.Y / 100.0f);
            const int32 Z = FMath::RoundToInt(Location.Z / 100.0f);
            OutKey = FString::Printf(
                TEXT("WORLD:%d:%d:%d"), X, Y, Z);
            OutLabel = FString::Printf(
                TEXT("world position %.0f, %.0f, %.0f cm"),
                Location.X, Location.Y, Location.Z);
            return true;
        }

        const bool bAnchorChangesPosition =
            Entry.Action ==
                ETMOPPersonTimelineAction::InitialPlacement ||
            Entry.Action == ETMOPPersonTimelineAction::Spawn ||
            Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor;
        if (bAnchorChangesPosition && !Entry.TargetAnchorId.IsNone())
        {
            // Group members may intentionally have different formation
            // offsets.  The shared anchor is the comparable position.
            OutKey = TEXT("ANCHOR:") +
                Entry.TargetAnchorId.ToString();
            OutLabel = Entry.TargetAnchorId.ToString();
            if (!Entry.AnchorOffsetCm.IsNearlyZero())
                OutLabel += TEXT(" + offset ") +
                    Entry.AnchorOffsetCm.ToCompactString();
            return true;
        }
        if (!Entry.TargetSeatId.IsNone())
        {
            OutKey = TEXT("SEAT:") + Entry.TargetSeatId.ToString();
            if (!Entry.TargetEntityId.IsNone())
                OutKey += TEXT(":") + Entry.TargetEntityId.ToString();
            OutLabel = Entry.TargetSeatId.ToString();
            if (!Entry.TargetEntityId.IsNone())
                OutLabel += TEXT(" in ") +
                    Entry.TargetEntityId.ToString();
            return true;
        }
        if ((Entry.LocationType == ETMOPPersonLocationType::VehicleSeat ||
             Entry.LocationType == ETMOPPersonLocationType::BusSeat ||
             Entry.LocationType ==
                ETMOPPersonLocationType::StandingInVehicle ||
             Entry.Action == ETMOPPersonTimelineAction::BeginDriving) &&
            !Entry.TargetEntityId.IsNone())
        {
            OutKey = TEXT("VEHICLE:") +
                Entry.TargetEntityId.ToString();
            OutLabel = Entry.TargetEntityId.ToString();
            return true;
        }
    }
    return false;
}

bool STMOPPeopleEditor::BuildTimelineSpeedBadge(
    const int32 Index,
    FText& OutText,
    FText& OutToolTip,
    FLinearColor& OutColor) const
{
    OutText = FText::GetEmpty();
    OutToolTip = FText::GetEmpty();
    OutColor = FLinearColor(0.35f, 0.35f, 0.35f);
    if (!WorkingRow.Timeline.IsValidIndex(Index)) return false;

    const FTMOPPersonTimelineEntry& Entry = WorkingRow.Timeline[Index];
    if (Entry.Action == ETMOPPersonTimelineAction::BeginDriving)
        return BuildVehicleTimelineSpeedBadge(
            Index, OutText, OutToolTip, OutColor);
    if (Entry.Action != ETMOPPersonTimelineAction::MoveToAnchor)
        return false;

    auto SetUnavailable = [&](const FString& Reason)
    {
        OutText = LOCTEXT("TimelineSpeedUnavailable", "SPEED ?");
        OutToolTip = FText::FromString(Reason);
        OutColor = FLinearColor(0.28f, 0.28f, 0.28f);
        return true;
    };

    UWorld* EditorWorld = GEditor != nullptr
        ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (EditorWorld == nullptr)
        return SetUnavailable(
            TEXT("Open a level containing the route anchors to calculate speed."));

    auto FindAnchorLocation = [EditorWorld](
        const FName AnchorId,
        const FVector& OffsetCm,
        const ETMOPAnchorOffsetSpace OffsetSpace,
        FVector& OutLocation)
    {
        if (AnchorId.IsNone()) return false;
        for (TActorIterator<ATMOPHistoricalAnchor> It(EditorWorld); It; ++It)
            if (It->GetAnchorId() == AnchorId)
            {
                OutLocation = It->GetAnchorLocation();
                if (!OffsetCm.IsNearlyZero())
                    OutLocation += OffsetSpace ==
                        ETMOPAnchorOffsetSpace::AnchorLocal
                        ? It->GetActorQuat().RotateVector(OffsetCm)
                        : OffsetCm;
                return true;
            }
        return false;
    };

    auto FindSeatLocation = [EditorWorld](
        const FName SeatId, FVector& OutLocation)
    {
        if (SeatId.IsNone()) return false;
        for (TActorIterator<AActor> It(EditorWorld); It; ++It)
        {
            TArray<UTMOPCinemaSeatComponent*> CinemaSeats;
            It->GetComponents<UTMOPCinemaSeatComponent>(CinemaSeats);
            for (const UTMOPCinemaSeatComponent* Seat : CinemaSeats)
                if (IsValid(Seat) && Seat->SeatId == SeatId)
                {
                    OutLocation = Seat->GetComponentLocation();
                    return true;
                }

            TArray<UTMOPVehicleSeatComponent*> VehicleSeats;
            It->GetComponents<UTMOPVehicleSeatComponent>(VehicleSeats);
            for (const UTMOPVehicleSeatComponent* Seat : VehicleSeats)
                if (IsValid(Seat) && Seat->SeatId == SeatId)
                {
                    OutLocation = Seat->GetComponentLocation();
                    return true;
                }
        }
        return false;
    };

    auto ResolveLocation = [&](const FTMOPPersonTimelineEntry& Candidate,
                               FVector& OutLocation)
    {
        if (Candidate.LocationType == ETMOPPersonLocationType::WorldTransform)
        {
            OutLocation = Candidate.WorldTransform.GetLocation();
            return true;
        }
        const bool bChangesPhysicalAnchorLocation =
            Candidate.Action ==
                ETMOPPersonTimelineAction::InitialPlacement ||
            Candidate.Action == ETMOPPersonTimelineAction::Spawn ||
            Candidate.Action == ETMOPPersonTimelineAction::MoveToAnchor;
        if (bChangesPhysicalAnchorLocation &&
            Candidate.LocationType == ETMOPPersonLocationType::Anchor &&
            !Candidate.TargetAnchorId.IsNone() &&
            FindAnchorLocation(
                Candidate.TargetAnchorId,
                Candidate.AnchorOffsetCm,
                Candidate.AnchorOffsetSpace,
                OutLocation))
            return true;
        if (!Candidate.TargetSeatId.IsNone() &&
            FindSeatLocation(Candidate.TargetSeatId, OutLocation))
            return true;
        return false;
    };

    FVector SegmentStart = FVector::ZeroVector;
    int32 PreviousLocationIndex = INDEX_NONE;
    for (int32 Previous = Index - 1; Previous >= 0; --Previous)
        if (ResolveLocation(WorkingRow.Timeline[Previous], SegmentStart))
        {
            PreviousLocationIndex = Previous;
            break;
        }
    if (PreviousLocationIndex == INDEX_NONE)
        return SetUnavailable(
            TEXT("No earlier timeline position or seat could be resolved."));

    TArray<FName> RouteAnchorIds = Entry.PassAnchorIds;
    RouteAnchorIds.Add(Entry.TargetAnchorId);
    if (Entry.TargetAnchorId.IsNone())
        return SetUnavailable(TEXT("The movement has no target anchor."));

    double RouteLengthCm = 0.0;
    bool bUsedStraightLineFallback = false;
    for (int32 RouteIndex = 0;
        RouteIndex < RouteAnchorIds.Num(); ++RouteIndex)
    {
        const FName RouteAnchorId = RouteAnchorIds[RouteIndex];
        FVector SegmentEnd;
        const bool bFinalAnchor =
            RouteIndex == RouteAnchorIds.Num() - 1;
        if (!FindAnchorLocation(
                RouteAnchorId,
                bFinalAnchor ? Entry.AnchorOffsetCm : FVector::ZeroVector,
                Entry.AnchorOffsetSpace,
                SegmentEnd))
            return SetUnavailable(FString::Printf(
                TEXT("Route anchor '%s' is not present in the open level."),
                *RouteAnchorId.ToString()));

        double SegmentLengthCm = FVector::Dist2D(SegmentStart, SegmentEnd);
        double NavPathLengthCm = SegmentLengthCm;
        const ENavigationQueryResult::Type PathResult =
            UNavigationSystemV1::GetPathLength(
                EditorWorld, SegmentStart, SegmentEnd,
                NavPathLengthCm, nullptr, nullptr);
        if (PathResult == ENavigationQueryResult::Success &&
            NavPathLengthCm > KINDA_SMALL_NUMBER)
        {
            SegmentLengthCm = NavPathLengthCm;
        }
        else
        {
            bUsedStraightLineFallback = true;
        }
        RouteLengthCm += SegmentLengthCm;
        SegmentStart = SegmentEnd;
    }

    if (RouteLengthCm <= KINDA_SMALL_NUMBER)
        return SetUnavailable(TEXT("The calculated route length is zero."));

    const FTMOPMovementProfile& Profile = WorkingRow.MovementProfile;
    const float PersonalMultiplier = FMath::Max(
        Profile.PersonalSpeedMultiplier, KINDA_SMALL_NUMBER);

    float ActivitySpeedCmPerSecond = Profile.NormalWalkSpeed;
    switch (Entry.ActivityState)
    {
    case ETMOPAgentActivityState::FastWalking:
        ActivitySpeedCmPerSecond = Profile.FastWalkSpeed;
        break;
    case ETMOPAgentActivityState::Jogging:
        ActivitySpeedCmPerSecond = Profile.JogSpeed;
        break;
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Fleeing:
        ActivitySpeedCmPerSecond = Profile.RunSpeed;
        break;
    case ETMOPAgentActivityState::Sprinting:
        ActivitySpeedCmPerSecond = Profile.SprintSpeed;
        break;
    default:
        break;
    }
    const float ConfiguredSpeedCmPerSecond =
        Entry.TravelSpeedOverrideCmPerSecond > 0.0f
        ? Entry.TravelSpeedOverrideCmPerSecond
        : ActivitySpeedCmPerSecond * PersonalMultiplier;

    float DisplaySpeedCmPerSecond = ConfiguredSpeedCmPerSecond;
    int32 AvailableSeconds = 0;
    if (Entry.bTimeIsArrival)
    {
        int32 ArrivalSecond = 0;
        int32 PreviousSecond = 0;
        if (!ResolveTimelineDisplaySecond(Index, ArrivalSecond) ||
            Index <= 0 ||
            !ResolveTimelineDisplaySecond(Index - 1, PreviousSecond))
            return SetUnavailable(
                TEXT("The arrival time or preceding timeline time could not be resolved."));
        AvailableSeconds = ArrivalSecond - PreviousSecond;
        if (AvailableSeconds <= 0)
        {
            const auto FormatTimelineSecond = [](const int32 Second)
            {
                const int32 Normalized = FMath::Max(0, Second);
                return FString::Printf(TEXT("%02d:%02d:%02d"),
                    Normalized / 3600,
                    (Normalized / 60) % 60,
                    Normalized % 60);
            };
            OutText = FText::FromString(FString::Printf(
                TEXT("TIME %s%d s"),
                AvailableSeconds > 0 ? TEXT("+") : TEXT(""),
                AvailableSeconds));
            OutToolTip = FText::FromString(FString::Printf(
                TEXT("Timeline conflict: this arrival resolves to %s, while the preceding row resolves to %s. The movement therefore has %d second(s), so no speed can be calculated. Relative/shared-event rows ignore the editable Absolute Time fields."),
                *FormatTimelineSecond(ArrivalSecond),
                *FormatTimelineSecond(PreviousSecond),
                AvailableSeconds));
            OutColor = FLinearColor(0.85f, 0.12f, 0.08f);
            return true;
        }
        DisplaySpeedCmPerSecond =
            static_cast<float>(RouteLengthCm / AvailableSeconds);
    }

    const float Slow = Profile.SlowWalkSpeed * PersonalMultiplier;
    const float GroupTalking =
        Profile.GroupTalkingWalkSpeed * PersonalMultiplier;
    const float Talking = Profile.TalkingWalkSpeed * PersonalMultiplier;
    const float Normal = Profile.NormalWalkSpeed * PersonalMultiplier;
    const float Fast = Profile.FastWalkSpeed * PersonalMultiplier;
    const float Jog = Profile.JogSpeed * PersonalMultiplier;
    const float Run = Profile.RunSpeed * PersonalMultiplier;
    const float Sprint = Profile.SprintSpeed * PersonalMultiplier;
    const float MidSlowGroupTalking = (Slow + GroupTalking) * 0.5f;
    const float MidGroupTalkingTalking =
        (GroupTalking + Talking) * 0.5f;
    const float MidTalkingNormal = (Talking + Normal) * 0.5f;
    const float MidNormalFast = (Normal + Fast) * 0.5f;
    const float MidFastJog = (Fast + Jog) * 0.5f;
    const float MidJogRun = (Jog + Run) * 0.5f;
    const float MidRunSprint = (Run + Sprint) * 0.5f;

    FString Category;
    if (DisplaySpeedCmPerSecond < Slow * 0.65f)
        Category = TEXT("VERY SLOW");
    else if (DisplaySpeedCmPerSecond < MidSlowGroupTalking)
        Category = TEXT("SLOW WALK");
    else if (DisplaySpeedCmPerSecond < MidGroupTalkingTalking)
        Category = TEXT("GROUP TALKING PACE");
    else if (DisplaySpeedCmPerSecond < MidTalkingNormal)
        Category = TEXT("TALKING / SOCIAL PACE");
    else if (DisplaySpeedCmPerSecond < MidNormalFast)
        Category = TEXT("NORMAL WALK");
    else if (DisplaySpeedCmPerSecond < MidFastJog)
        Category = TEXT("FAST WALK");
    else if (DisplaySpeedCmPerSecond < MidJogRun)
        Category = TEXT("JOG");
    else if (DisplaySpeedCmPerSecond < MidRunSprint)
        Category = TEXT("RUN");
    else if (DisplaySpeedCmPerSecond <= Sprint * 1.10f)
        Category = TEXT("SPRINT");
    else
        Category = TEXT("TOO FAST");

    const float DisplaySpeedMetersPerSecond =
        DisplaySpeedCmPerSecond / 100.0f;
    OutText = FText::FromString(FString::Printf(
        TEXT("%s %.1f m/s  %s"),
        Entry.bTimeIsArrival ? TEXT("REQ") : TEXT("SET"),
        DisplaySpeedMetersPerSecond, *Category));

    const FString RouteMethod = bUsedStraightLineFallback
        ? TEXT("NavMesh where available; straight-line fallback")
        : TEXT("NavMesh path");
    if (Entry.bTimeIsArrival)
    {
        const bool bUsesFixedOverride =
            Entry.TravelSpeedOverrideCmPerSecond > 0.0f;
        const bool bFitsConfiguredSpeed = bUsesFixedOverride
            ? DisplaySpeedCmPerSecond <=
                ConfiguredSpeedCmPerSecond * 1.05f
            : DisplaySpeedCmPerSecond <= Sprint;
        const bool bBeyondSprint =
            DisplaySpeedCmPerSecond > Sprint;
        const float RuntimeTargetSpeed = bUsesFixedOverride
            ? ConfiguredSpeedCmPerSecond
            : FMath::Min(DisplaySpeedCmPerSecond, Sprint);
        OutColor = bBeyondSprint
            ? FLinearColor(0.75f, 0.05f, 0.03f)
            : bFitsConfiguredSpeed
                ? FLinearColor(0.05f, 0.42f, 0.12f)
                : FLinearColor(0.78f, 0.38f, 0.03f);
        OutToolTip = FText::FromString(FString::Printf(
            TEXT("Required speed to this ARRIVAL: %.2f m/s. Available time begins at Timeline[%d]; the route begins at the last known position in Timeline[%d]. Route: %.1f m over %d s (%s; %d pass anchor(s)). Runtime target speed: %.2f m/s. %s"),
            DisplaySpeedMetersPerSecond,
            Index - 1,
            PreviousLocationIndex,
            RouteLengthCm / 100.0,
            AvailableSeconds,
            *RouteMethod,
            Entry.PassAnchorIds.Num(),
            RuntimeTargetSpeed / 100.0f,
            bFitsConfiguredSpeed
                ? bUsesFixedOverride
                    ? TEXT("The fixed override can meet the arrival time.")
                    : TEXT("Runtime will select this required gait automatically.")
                : bUsesFixedOverride
                    ? TEXT("The fixed override is too slow for the arrival time.")
                    : TEXT("The required speed exceeds this person's sprint speed.")));
    }
    else
    {
        const int32 EstimatedTravelSeconds =
            ConfiguredSpeedCmPerSecond > KINDA_SMALL_NUMBER
            ? FMath::CeilToInt(
                RouteLengthCm / ConfiguredSpeedCmPerSecond)
            : 0;
        OutColor = FLinearColor(0.08f, 0.30f, 0.52f);
        OutToolTip = FText::FromString(FString::Printf(
            TEXT("Departure-timed movement. Configured speed: %.2f m/s. Route: %.1f m (%s; %d pass anchor(s)). Estimated travel time: %d s. There is no fixed arrival time to validate."),
            ConfiguredSpeedCmPerSecond / 100.0f,
            RouteLengthCm / 100.0,
            *RouteMethod,
            Entry.PassAnchorIds.Num(),
            EstimatedTravelSeconds));
    }
    return true;
}

bool STMOPPeopleEditor::BuildVehicleTimelineSpeedBadge(
    const int32 Index,
    FText& OutText,
    FText& OutToolTip,
    FLinearColor& OutColor) const
{
    OutText = FText::GetEmpty();
    OutToolTip = FText::GetEmpty();
    OutColor = FLinearColor(0.35f, 0.35f, 0.35f);
    if (!WorkingRow.Timeline.IsValidIndex(Index)) return false;

    const FTMOPPersonTimelineEntry& Entry = WorkingRow.Timeline[Index];
    if (Entry.Action != ETMOPPersonTimelineAction::BeginDriving)
        return false;

    auto SetUnavailable = [&](const FString& Reason)
    {
        OutText = LOCTEXT("VehicleTimelineSpeedUnavailable", "SPEED ?");
        OutToolTip = FText::FromString(Reason);
        OutColor = FLinearColor(0.28f, 0.28f, 0.28f);
        return true;
    };

    UWorld* EditorWorld = GEditor != nullptr
        ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (EditorWorld == nullptr)
        return SetUnavailable(
            TEXT("Open the driving level to calculate vehicle speed."));

    auto FindLane = [EditorWorld](const FName LaneId)
        -> UTMOPTrafficLaneComponent*
    {
        if (LaneId.IsNone()) return nullptr;
        for (TActorIterator<AActor> It(EditorWorld); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Lanes;
            It->GetComponents<UTMOPTrafficLaneComponent>(Lanes);
            for (UTMOPTrafficLaneComponent* Lane : Lanes)
                if (IsValid(Lane) && Lane->LaneId == LaneId)
                    return Lane;
        }
        return nullptr;
    };

    TArray<FName> RouteLaneIds = Entry.OrderedLaneIds;
    FString RouteSource = TEXT("person timeline");
    if (RouteLaneIds.IsEmpty())
    {
        const UDataTable* Vehicles = VehicleTable.Get();
        if (!IsValid(Vehicles) || Vehicles->GetRowStruct() !=
            FTMOPHistoricalVehicleRow::StaticStruct())
            return SetUnavailable(
                TEXT("DT_TMOP_HistoricalVehicles could not be loaded."));

        const FTMOPHistoricalVehicleRow* Vehicle =
            Vehicles->FindRow<FTMOPHistoricalVehicleRow>(
                Entry.TargetEntityId,
                TEXT("TMOPVehicleTimelineSpeedByRowName"), false);
        if (Vehicle == nullptr)
            for (const FName RowName : Vehicles->GetRowNames())
                if (const FTMOPHistoricalVehicleRow* Candidate =
                    Vehicles->FindRow<FTMOPHistoricalVehicleRow>(
                        RowName,
                        TEXT("TMOPVehicleTimelineSpeedById"), false))
                    if (Candidate->VehicleId == Entry.TargetEntityId)
                    {
                        Vehicle = Candidate;
                        break;
                    }
        if (Vehicle == nullptr)
            return SetUnavailable(FString::Printf(
                TEXT("Vehicle '%s' was not found in DT_TMOP_HistoricalVehicles."),
                *Entry.TargetEntityId.ToString()));

        TArray<const FTMOPHistoricalVehicleTimelineEntry*> DrivingEntries;
        for (const FTMOPHistoricalVehicleTimelineEntry& VehicleEntry :
            Vehicle->Timeline)
        {
            const bool bDrivingAction =
                VehicleEntry.Action ==
                    ETMOPHistoricalVehicleAction::BeginDriving ||
                VehicleEntry.Action ==
                    ETMOPHistoricalVehicleAction::EnterTrafficRoute;
            const bool bDriverMatches =
                VehicleEntry.DriverEntityId.IsNone() ||
                VehicleEntry.DriverEntityId == WorkingRow.EntityId;
            if (bDrivingAction && bDriverMatches &&
                !VehicleEntry.OrderedLaneIds.IsEmpty())
                DrivingEntries.Add(&VehicleEntry);
        }

        const FTMOPHistoricalVehicleTimelineEntry* SelectedDrivingEntry =
            nullptr;
        for (const FTMOPHistoricalVehicleTimelineEntry* Candidate :
            DrivingEntries)
            if (Candidate->EntryId == Entry.EntryId)
            {
                SelectedDrivingEntry = Candidate;
                break;
            }
        if (SelectedDrivingEntry == nullptr && !DrivingEntries.IsEmpty())
        {
            int32 DrivingOrdinal = 0;
            for (int32 Previous = 0; Previous < Index; ++Previous)
                if (WorkingRow.Timeline[Previous].Action ==
                    ETMOPPersonTimelineAction::BeginDriving &&
                    WorkingRow.Timeline[Previous].TargetEntityId ==
                        Entry.TargetEntityId)
                    ++DrivingOrdinal;
            SelectedDrivingEntry = DrivingEntries[
                FMath::Min(DrivingOrdinal, DrivingEntries.Num() - 1)];
        }
        if (SelectedDrivingEntry == nullptr)
            return SetUnavailable(FString::Printf(
                TEXT("Vehicle '%s' has no lane route for this driver."),
                *Entry.TargetEntityId.ToString()));
        RouteLaneIds = SelectedDrivingEntry->OrderedLaneIds;
        RouteSource = TEXT("vehicle timeline");
    }

    RouteLaneIds.RemoveAll(
        [](const FName LaneId) { return LaneId.IsNone(); });
    if (RouteLaneIds.IsEmpty())
        return SetUnavailable(TEXT("The driving route has no lane IDs."));

    double RouteLengthCm = 0.0;
    for (int32 LaneIndex = 0; LaneIndex < RouteLaneIds.Num(); ++LaneIndex)
    {
        UTMOPTrafficLaneComponent* Lane =
            FindLane(RouteLaneIds[LaneIndex]);
        if (!IsValid(Lane))
            return SetUnavailable(FString::Printf(
                TEXT("Traffic lane '%s' is not present in the open level."),
                *RouteLaneIds[LaneIndex].ToString()));

        double LaneLengthCm = Lane->GetSplineLength();
        if (LaneIndex == 0)
            LaneLengthCm -= FMath::Clamp(
                static_cast<double>(
                    Entry.VehicleStartDistanceAlongFirstLaneCm),
                0.0, LaneLengthCm);

        if (LaneIndex == RouteLaneIds.Num() - 1 &&
            !Entry.DrivingDestinationAnchorId.IsNone())
        {
            for (TActorIterator<ATMOPHistoricalAnchor> It(EditorWorld);
                It; ++It)
                if (It->GetAnchorId() ==
                    Entry.DrivingDestinationAnchorId)
                {
                    const float InputKey =
                        Lane->FindInputKeyClosestToWorldLocation(
                            It->GetAnchorLocation());
                    const double DestinationDistanceCm =
                        Lane->GetDistanceAlongSplineAtSplineInputKey(
                            InputKey);
                    LaneLengthCm = LaneIndex == 0
                        ? FMath::Max(0.0,
                            DestinationDistanceCm -
                            Entry.VehicleStartDistanceAlongFirstLaneCm)
                        : DestinationDistanceCm;
                    break;
                }
        }
        RouteLengthCm += FMath::Max(0.0, LaneLengthCm);
    }
    if (RouteLengthCm <= KINDA_SMALL_NUMBER)
        return SetUnavailable(
            TEXT("The calculated vehicle route length is zero."));

    int32 StartSecond = 0;
    if (!ResolveTimelineDisplaySecond(Index, StartSecond))
        return SetUnavailable(
            TEXT("The driving start time could not be resolved."));

    int32 EndSecond = 0;
    int32 EndIndex = INDEX_NONE;
    for (int32 Next = Index + 1;
        Next < WorkingRow.Timeline.Num(); ++Next)
        if (ResolveTimelineDisplaySecond(Next, EndSecond) &&
            EndSecond > StartSecond)
        {
            EndIndex = Next;
            break;
        }
    if (EndIndex == INDEX_NONE)
        return SetUnavailable(
            TEXT("Add a later timeline entry to define when this drive ends."));

    const int32 AvailableSeconds = EndSecond - StartSecond;
    const double SpeedKmh =
        (RouteLengthCm / 100000.0) /
        (static_cast<double>(AvailableSeconds) / 3600.0);
    OutText = FText::FromString(FString::Printf(
        TEXT("AVG %.1f km/h"), SpeedKmh));
    OutColor = SpeedKmh <= 50.0
        ? FLinearColor(0.05f, 0.42f, 0.12f)
        : SpeedKmh <= 70.0
            ? FLinearColor(0.78f, 0.38f, 0.03f)
            : FLinearColor(0.75f, 0.05f, 0.03f);
    OutToolTip = FText::FromString(FString::Printf(
        TEXT("Average vehicle speed: %.2f km/h. Route: %.1f km over %d s, from Timeline[%d] to Timeline[%d], using %d lane(s) from the %s. Traffic, braking and acceleration can make the instantaneous runtime speed differ."),
        SpeedKmh,
        RouteLengthCm / 100000.0,
        AvailableSeconds,
        Index,
        EndIndex,
        RouteLaneIds.Num(),
        *RouteSource));
    return true;
}

FSlateColor STMOPPeopleEditor::GetTimelineColor(
    const int32 Index) const
{
    if (EntryHasError(Index))
        return FSlateColor(FLinearColor(1.0f, 0.25f, 0.2f));
    const FTMOPPersonTimelineEntry& Entry = WorkingRow.Timeline[Index];
    if (Entry.Usage == ETMOPPersonTimelineUsage::DocumentationOnly)
        return FSlateColor(FLinearColor(0.65f, 0.65f, 0.65f));
    if (Entry.AnchorReferenceMode == ETMOPAnchorReferenceMode::PlannedFuture)
        return FSlateColor(FLinearColor(0.75f, 0.45f, 1.0f));
    return FSlateColor(FLinearColor(0.35f, 0.75f, 1.0f));
}

bool STMOPPeopleEditor::EntryHasError(
    const int32 Index, FString* OutMessage) const
{
    if (!WorkingRow.Timeline.IsValidIndex(Index))
        return false;
    const FTMOPPersonTimelineEntry& Entry =
        WorkingRow.Timeline[Index];
    auto Fail = [OutMessage](const FString& Message)
    {
        if (OutMessage != nullptr) *OutMessage = Message;
        return true;
    };
    if (Entry.EntryId.IsNone())
        return Fail(TEXT("Missing Entry ID"));
    if (Entry.Usage != ETMOPPersonTimelineUsage::DocumentationOnly)
    {
        bool bEarlierSimulationEntry = false;
        for (int32 Previous = 0; Previous < Index; ++Previous)
            if (WorkingRow.Timeline[Previous].Usage !=
                ETMOPPersonTimelineUsage::DocumentationOnly)
            {
                bEarlierSimulationEntry = true;
                break;
            }
        if (!bEarlierSimulationEntry &&
            Entry.Action != ETMOPPersonTimelineAction::InitialPlacement &&
            Entry.Action != ETMOPPersonTimelineAction::Spawn)
            return Fail(TEXT("First simulation entry must be Initial Placement or Spawn"));
    }
    if (Entry.TimingMode == ETMOPEventTimingMode::Relative &&
        Entry.SharedEventId.IsNone())
        return Fail(TEXT("Relative timing requires Shared Event ID"));
    if (Entry.TimingMode ==
            ETMOPEventTimingMode::RelativeToPreviousEntry &&
        Index == 0)
        return Fail(TEXT("First entry cannot be relative to previous entry"));
    if (Entry.TimingMode == ETMOPEventTimingMode::Relative &&
        !Entry.SharedEventId.IsNone() && EventTable.IsValid())
    {
        bool bEventExists = EventTable->GetRowMap().Contains(
            Entry.SharedEventId);
        if (!bEventExists)
            for (const FName EventRowName : EventTable->GetRowNames())
                if (const FTMOPHistoricalEventDefinition* Event =
                    EventTable->FindRow<FTMOPHistoricalEventDefinition>(
                        EventRowName,
                        TEXT("TMOPPeopleEditorEventValidation"),
                        false))
                    if (Event->EventId == Entry.SharedEventId)
                    {
                        bEventExists = true;
                        break;
                    }
        if (!bEventExists)
            return Fail(FString::Printf(
                TEXT("Shared event '%s' does not exist"),
                *Entry.SharedEventId.ToString()));
    }
    if (Entry.bTimeIsArrival &&
        Entry.Action != ETMOPPersonTimelineAction::MoveToAnchor)
        return Fail(TEXT("Arrival time requires Move To Anchor"));
    if (Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor &&
        Entry.TargetAnchorId.IsNone())
        return Fail(TEXT("Move To Anchor requires Target Anchor ID"));
    if (Entry.Action == ETMOPPersonTimelineAction::LookAtAnchor)
    {
        const bool bPersonTarget =
            Entry.ConversationTargetMode ==
                ETMOPConversationTargetMode::SpecificPerson ||
            (Entry.ConversationTargetMode ==
                ETMOPConversationTargetMode::Automatic &&
             !Entry.TargetEntityId.IsNone());
        const bool bGroupTarget =
            Entry.ConversationTargetMode ==
                ETMOPConversationTargetMode::Group;
        if (bPersonTarget && Entry.TargetEntityId.IsNone())
            return Fail(TEXT("Look At Person requires Target Entity ID"));
        if (bGroupTarget && Entry.TargetGroupId.IsNone() &&
            WorkingRow.SocialGroupId.IsNone())
            return Fail(TEXT("Look At Group requires Target Group ID"));
        if (!bPersonTarget && !bGroupTarget &&
            Entry.TargetAnchorId.IsNone())
            return Fail(TEXT("Look At Anchor requires Target Anchor ID"));
    }
    if ((Entry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
         Entry.Action == ETMOPPersonTimelineAction::ExitVehicle ||
         Entry.Action == ETMOPPersonTimelineAction::BeginDriving) &&
        Entry.TargetEntityId.IsNone())
        return Fail(TEXT("Vehicle action requires Target Entity ID"));
    if ((Entry.Action == ETMOPPersonTimelineAction::JoinGroup ||
         Entry.Action == ETMOPPersonTimelineAction::SplitGroup ||
         Entry.Action == ETMOPPersonTimelineAction::DissolveGroup ||
         Entry.Action == ETMOPPersonTimelineAction::SetGroupLeader) &&
        Entry.TargetGroupId.IsNone())
        return Fail(TEXT("Group action requires Target Group ID"));
    if (Entry.Action == ETMOPPersonTimelineAction::CreateGroup &&
        (Entry.GroupDefinition.GroupId.IsNone() ||
         Entry.GroupDefinition.MemberEntityIds.IsEmpty()))
        return Fail(TEXT("Create Group requires a complete Group Definition"));
    if (Entry.Action == ETMOPPersonTimelineAction::SplitGroup &&
        Entry.SplitGroupDefinitions.Num() < 2)
        return Fail(TEXT("Split Group requires at least two child groups"));
    if (Entry.Action == ETMOPPersonTimelineAction::SetGroupLeader &&
        Entry.NewGroupLeaderEntityId.IsNone())
        return Fail(TEXT("Set Group Leader requires a new leader Entity ID"));
    if (Entry.Action == ETMOPPersonTimelineAction::PlayUniqueAnimation &&
        Entry.AnimationAsset.IsNull())
        return Fail(TEXT("Play Unique Animation requires an Animation Asset"));
    const bool bConversationAction =
        Entry.Action == ETMOPPersonTimelineAction::Interact ||
        Entry.Action == ETMOPPersonTimelineAction::PlayUniqueAnimation;
    if (bConversationAction &&
        Entry.ConversationTargetMode ==
            ETMOPConversationTargetMode::SpecificPerson &&
        Entry.TargetEntityId.IsNone())
        return Fail(TEXT("Specific Person conversation requires Target Entity ID"));
    if (bConversationAction &&
        Entry.ConversationTargetMode ==
            ETMOPConversationTargetMode::Group &&
        Entry.TargetGroupId.IsNone() && WorkingRow.SocialGroupId.IsNone())
        return Fail(TEXT("Group conversation requires Target Group ID or the person's own Social Group ID"));
    if (bConversationAction &&
        Entry.ConversationTargetMode ==
            ETMOPConversationTargetMode::Anchor &&
        Entry.TargetAnchorId.IsNone())
        return Fail(TEXT("Anchor conversation requires Target Anchor ID"));
    if (Index > 0 &&
        Entry.TimingMode == ETMOPEventTimingMode::Absolute)
    {
        for (int32 Previous = Index - 1; Previous >= 0; --Previous)
            if (WorkingRow.Timeline[Previous].TimingMode ==
                ETMOPEventTimingMode::Absolute)
                return WorkingRow.Timeline[Previous].Time
                    .ToSecondsFromMidnight() >
                    Entry.Time.ToSecondsFromMidnight()
                    ? Fail(TEXT("Absolute time is earlier than previous absolute entry"))
                    : false;
    }
    return false;
}

TArray<FString> STMOPPeopleEditor::ValidateWorkingRow() const
{
    TArray<FString> Errors;
    if (WorkingRow.EntityId.IsNone())
        Errors.Add(TEXT("Entity ID is missing."));
    if (WorkingRow.Timeline.IsEmpty())
        Errors.Add(TEXT("Timeline is empty."));
    if (WorkingRow.bSpawnInSimulation)
    {
        const bool bHasSimulationEntry = WorkingRow.Timeline.ContainsByPredicate(
            [](const FTMOPPersonTimelineEntry& Entry)
            {
                return Entry.Usage != ETMOPPersonTimelineUsage::DocumentationOnly;
            });
        if (!bHasSimulationEntry)
            Errors.Add(TEXT("Simulation is enabled but the timeline contains only documentation entries."));
    }
    TSet<FName> EntryIds;
    for (int32 Index = 0; Index < WorkingRow.Timeline.Num(); ++Index)
    {
        FString Error;
        if (EntryHasError(Index, &Error))
            Errors.Add(FString::Printf(
                TEXT("Timeline[%d]: %s"), Index, *Error));
        const FName EntryId = WorkingRow.Timeline[Index].EntryId;
        if (!EntryId.IsNone() && EntryIds.Contains(EntryId))
            Errors.Add(FString::Printf(
                TEXT("Timeline[%d]: duplicate Entry ID '%s'."),
                Index, *EntryId.ToString()));
        EntryIds.Add(EntryId);
    }
    return Errors;
}

TArray<FString> STMOPPeopleEditor::ValidateAppearanceRow(
    const FTMOPPersonProfileRow& Row, UDataTable* Catalog,
    int32* OutUnknownPartCount) const
{
    TArray<FString> Warnings;
    if (OutUnknownPartCount != nullptr) *OutUnknownPartCount = 0;
    if (!IsValid(Catalog) || Catalog->GetRowStruct() !=
        FTMOPAppearanceAssetRow::StaticStruct())
    {
        Warnings.Add(TEXT("Appearance catalog is missing or uses the wrong Row Struct."));
        return Warnings;
    }

    if (Row.AppearanceProfile.GenerationMode ==
            ETMOPAppearanceGenerationMode::MetaHuman &&
        Row.AgentClass == nullptr)
    {
        Warnings.Add(TEXT("MetaHuman mode requires a bespoke Agent Class so the correct head, body and groom components are spawned."));
    }

    FTMOPResolvedAppearance Resolved;
    UTMOPAppearanceResolver::ResolveAppearance(Row, Catalog, Resolved);
    struct FNamedPart
    {
        const TCHAR* Label;
        const FTMOPResolvedAppearancePart* Part;
    };
    const FNamedPart Parts[] = {
        { TEXT("Body"), &Resolved.Body }, { TEXT("Face"), &Resolved.Face },
        { TEXT("Hair"), &Resolved.Hair },
        { TEXT("Facial hair"), &Resolved.FacialHair },
        { TEXT("Outerwear"), &Resolved.Outerwear },
        { TEXT("Upper body"), &Resolved.UpperBody },
        { TEXT("Trousers"), &Resolved.Trousers },
        { TEXT("Footwear"), &Resolved.Footwear },
        { TEXT("Gloves"), &Resolved.Gloves },
        { TEXT("Headwear"), &Resolved.Headwear },
        { TEXT("Scarf"), &Resolved.Scarf },
        { TEXT("Glasses"), &Resolved.Glasses } };
    for (const FNamedPart& Named : Parts)
    {
        const FTMOPResolvedAppearancePart& Part = *Named.Part;
        if (Part.bIntentionallyEmpty) continue;
        if (Part.bSourceWasUnknown && Part.PartType != ETMOPAppearancePartType::Body &&
            OutUnknownPartCount != nullptr)
            ++(*OutUnknownPartCount);
        if (Part.bUsesObscuredFallback && !Part.bSourceWasUnknown)
            Warnings.Add(FString::Printf(
                TEXT("%s has source evidence but no catalog asset matched it."),
                Named.Label));
        if (!Part.bUsesObscuredFallback &&
            Part.PartType != ETMOPAppearancePartType::Body &&
            Part.CatalogId.IsNone() && Part.Mesh.IsNull() &&
            Part.StaticMesh.IsNull())
            Warnings.Add(FString::Printf(
                TEXT("%s resolved without an asset or fallback."), Named.Label));
        if (Part.PartType == ETMOPAppearancePartType::Headwear &&
            Part.StaticMesh.IsNull() && !Part.Mesh.IsNull())
            Warnings.Add(FString::Printf(TEXT(
                "%s still uses legacy Skeletal Mesh; assign StaticMesh for socket attachment."),
                Named.Label));
    }
    return Warnings;
}

void STMOPPeopleEditor::SetStatus(
    const FText& Message, const FLinearColor& Color)
{
    if (!StatusText.IsValid()) return;
    StatusText->SetText(Message);
    StatusText->SetColorAndOpacity(Color);
}

#undef LOCTEXT_NAMESPACE
