#include "STMOPPeopleEditor.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "IStructureDetailsView.h"
#include "PropertyEditorModule.h"
#include "Styling/AppStyle.h"
#include "UObject/StructOnScope.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSearchableComboBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SSplitter.h"
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
            .Value(0.22f)
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
            .Value(0.40f)
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
                    .FillHeight(1.0f)
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
                ]
            ]

            + SSplitter::Slot()
            .Value(0.38f)
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
}

void STMOPPeopleEditor::LoadDefaultTable()
{
    PeopleTable = LoadObject<UDataTable>(
        nullptr, DefaultPeopleTablePath);
    EventTable = LoadObject<UDataTable>(
        nullptr, DefaultEventTablePath);
    VehicleTable = LoadObject<UDataTable>(
        nullptr, DefaultVehicleTablePath);
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
    SelectedTimelineIndex = INDEX_NONE;
    EntryStructData.Reset();
    EntryDetailsView->SetStructureData(nullptr);
    RefreshTimeline();
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

TSharedRef<ITableRow> STMOPPeopleEditor::GenerateTimelineRow(
    const FTimelineItem Item,
    const TSharedRef<STableViewBase>& OwnerTable)
{
    const int32 Index = Item.IsValid() ? *Item : INDEX_NONE;
    const FTMOPPersonTimelineEntry* Entry =
        WorkingRow.Timeline.IsValidIndex(Index)
        ? &WorkingRow.Timeline[Index] : nullptr;

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
            ]
            + SVerticalBox::Slot().AutoHeight().Padding(28.0f, 3.0f, 0.0f, 0.0f)
            [
                SNew(STextBlock)
                .Text(GetTimelineSummary(Index))
                .ColorAndOpacity(FSlateColor::UseSubduedForeground())
            ]
        ]
    ];
}

void STMOPPeopleEditor::HandlePersonSelectionChanged(
    const FPersonItem Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid()) SelectPerson(*Item);
}

void STMOPPeopleEditor::HandleTimelineSelectionChanged(
    const FTimelineItem Item, ESelectInfo::Type SelectInfo)
{
    if (Item.IsValid()) SelectTimelineEntry(*Item);
}

void STMOPPeopleEditor::HandlePersonSearchChanged(
    const FText& SearchText)
{
    PersonSearch = SearchText.ToString();
    RefreshPeople();
}

void STMOPPeopleEditor::RefreshReferenceOptions()
{
    AnchorReferenceItems.Reset();
    EntityReferenceItems.Reset();
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
    SortItems(SeatReferenceItems);
    SortItems(EventReferenceItems);

    if (AnchorReferenceCombo.IsValid())
        AnchorReferenceCombo->RefreshOptions();
    if (EntityReferenceCombo.IsValid())
        EntityReferenceCombo->RefreshOptions();
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
        break;
    case EReferenceField::TargetEntity:
        Entry->TargetEntityId = Id;
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
    if (SelectedTimelineIndex <= 0 ||
        !WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
    {
        SetStatus(
            LOCTEXT("CannotDeleteInitial",
                "Timeline[0] is the required initial placement and cannot be deleted."),
            FLinearColor(1.0f, 0.65f, 0.1f));
        return FReply::Handled();
    }
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
    CommitEntryEdits();
    UDataTable* Table = PeopleTable.Get();
    if (!IsValid(Table) || SelectedRowName.IsNone())
        return FReply::Handled();

    const TArray<FString> Errors = ValidateWorkingRow();
    if (!Errors.IsEmpty())
    {
        SetStatus(
            FText::FromString(TEXT("Not saved: ") + Errors[0]),
            FLinearColor::Red);
        return FReply::Handled();
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
            return FReply::Handled();
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
            return FReply::Handled();
        }
        *Existing = WorkingRow;
    }

    Table->MarkPackageDirty();
    Table->PostEditChange();

    RefreshPeople();
    RefreshTimeline();
    SetStatus(
        LOCTEXT("Saved",
            "Person saved to DT_TMOP_People. Save the project to write the asset to disk."),
        FLinearColor(0.4f, 1.0f, 0.4f));
    return FReply::Handled();
}

FReply STMOPPeopleEditor::ReloadPerson()
{
    if (!SelectedRowName.IsNone()) SelectPerson(SelectedRowName);
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
    FString Summary = ActionLabel(Entry.Action);
    if (!Entry.TargetAnchorId.IsNone())
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

FSlateColor STMOPPeopleEditor::GetTimelineColor(
    const int32 Index) const
{
    return EntryHasError(Index)
        ? FSlateColor(FLinearColor(1.0f, 0.25f, 0.2f))
        : FSlateColor(FLinearColor(0.35f, 0.75f, 1.0f));
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
    if (Index == 0 &&
        Entry.Action != ETMOPPersonTimelineAction::InitialPlacement &&
        Entry.Action != ETMOPPersonTimelineAction::Spawn)
        return Fail(TEXT("First entry must be Initial Placement or Spawn"));
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
    if ((Entry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
         Entry.Action == ETMOPPersonTimelineAction::ExitVehicle ||
         Entry.Action == ETMOPPersonTimelineAction::BeginDriving) &&
        Entry.TargetEntityId.IsNone())
        return Fail(TEXT("Vehicle action requires Target Entity ID"));
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

void STMOPPeopleEditor::SetStatus(
    const FText& Message, const FLinearColor& Color)
{
    if (!StatusText.IsValid()) return;
    StatusText->SetText(Message);
    StatusText->SetColorAndOpacity(Color);
}

#undef LOCTEXT_NAMESPACE
