#pragma once

#include "CoreMinimal.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPeopleEditorViewModels.h"
#include "Styling/SlateBrush.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SListView.h"

class IStructureDetailsView;
class SEditableTextBox;
class SSearchBox;
class SSearchableComboBox;
class STextBlock;
class UDataTable;
class FStructOnScope;

class STMOPPeopleEditor final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPPeopleEditor) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);

private:
    using FPersonItem = TSharedPtr<FName>;
    using FTimelineItem = TSharedPtr<int32>;
    using FReferenceItem = TSharedPtr<FString>;

    enum class EPeopleCategoryFilter : uint8
    {
        All,
        Police,
        Suspect
    };

    enum class EReferenceField : uint8
    {
        TargetAnchor,
        TargetEntity,
        TargetSeat,
        SharedEvent
    };

    static constexpr const TCHAR* DefaultPeopleTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_People.DT_TMOP_People");
    static constexpr const TCHAR* DefaultEventTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_HistoricalEvents.DT_TMOP_HistoricalEvents");
    static constexpr const TCHAR* DefaultVehicleTablePath =
        TEXT("/Game/TMOP/Vehicles/DT_TMOP_HistoricalVehicles.DT_TMOP_HistoricalVehicles");

    void LoadDefaultTable();
    void RefreshPeople();
    void RefreshTimeline();
    void SelectPerson(FName RowName);
    void SelectTimelineEntry(int32 Index);
    void CommitEntryEdits();
    void RefreshPersonDetailViews();
    void CommitPersonDetailEdits();
    void SetStatus(const FText& Message, const FLinearColor& Color);

    TSharedRef<ITableRow> GeneratePersonRow(
        FPersonItem Item,
        const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> GenerateTimelineRow(
        FTimelineItem Item,
        const TSharedRef<STableViewBase>& OwnerTable);
    void HandlePersonSelectionChanged(
        FPersonItem Item, ESelectInfo::Type SelectInfo);
    void HandleTimelineSelectionChanged(
        FTimelineItem Item, ESelectInfo::Type SelectInfo);
    void HandlePersonSearchChanged(const FText& SearchText);
    ECheckBoxState GetPeopleFilterCheckState(
        EPeopleCategoryFilter Filter) const;
    void HandlePeopleFilterChanged(
        ECheckBoxState NewState,
        EPeopleCategoryFilter Filter);
    void RefreshReferenceOptions();
    TSharedRef<SWidget> GenerateReferenceOption(FReferenceItem Item) const;
    void HandleReferenceSelected(
        FReferenceItem Item,
        ESelectInfo::Type SelectInfo,
        EReferenceField Field);
    FText GetReferenceFieldText(EReferenceField Field) const;
    FName GetReferenceId(FReferenceItem Item) const;
    const FSlateBrush* GetReferenceImageBrush() const;
    EVisibility GetReferenceImagePlaceholderVisibility() const;

    FReply AddTimelineEntry();
    FReply DuplicateTimelineEntry();
    FReply DeleteTimelineEntry();
    FReply MoveTimelineEntryUp();
    FReply MoveTimelineEntryDown();
    FReply SavePerson();
    FReply ReloadPerson();

    FText GetSelectedPersonTitle() const;
    FText GetSelectedPersonSubtitle() const;
    FText GetTimelineSummary(int32 Index) const;
    FText GetTimelineTimingText(const FTMOPPersonTimelineEntry& Entry) const;
    FSlateColor GetTimelineColor(int32 Index) const;
    bool EntryHasError(int32 Index, FString* OutMessage = nullptr) const;
    TArray<FString> ValidateWorkingRow() const;

    TWeakObjectPtr<UDataTable> PeopleTable;
    TWeakObjectPtr<UDataTable> EventTable;
    TWeakObjectPtr<UDataTable> VehicleTable;
    FName SelectedRowName = NAME_None;
    FTMOPPersonProfileRow WorkingRow;
    int32 SelectedTimelineIndex = INDEX_NONE;
    FString PersonSearch;
    EPeopleCategoryFilter PeopleCategoryFilter =
        EPeopleCategoryFilter::All;

    TArray<FPersonItem> PersonItems;
    TArray<FTimelineItem> TimelineItems;
    TSharedPtr<SListView<FPersonItem>> PersonListView;
    TSharedPtr<SListView<FTimelineItem>> TimelineListView;
    TSharedPtr<IStructureDetailsView> EntryDetailsView;
    TSharedPtr<FStructOnScope> EntryStructData;
    TSharedPtr<IStructureDetailsView> CharacteristicsDetailsView;
    TSharedPtr<IStructureDetailsView> GeneralDetailsView;
    TSharedPtr<FStructOnScope> CharacteristicsStructData;
    TSharedPtr<FStructOnScope> GeneralStructData;
    mutable FSlateBrush ReferenceImageBrush;
    TSharedPtr<STextBlock> StatusText;

    TArray<FReferenceItem> AnchorReferenceItems;
    TArray<FReferenceItem> EntityReferenceItems;
    TArray<FReferenceItem> SeatReferenceItems;
    TArray<FReferenceItem> EventReferenceItems;
    TMap<FString, FName> ReferenceIdsByLabel;
    TSharedPtr<SSearchableComboBox> AnchorReferenceCombo;
    TSharedPtr<SSearchableComboBox> EntityReferenceCombo;
    TSharedPtr<SSearchableComboBox> SeatReferenceCombo;
    TSharedPtr<SSearchableComboBox> EventReferenceCombo;
};

