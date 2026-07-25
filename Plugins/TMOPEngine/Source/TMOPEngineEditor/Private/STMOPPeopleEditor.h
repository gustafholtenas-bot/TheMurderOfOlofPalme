#pragma once

#include "CoreMinimal.h"
#include "People/TMOPPersonProfileTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class IStructureDetailsView;
class SEditableTextBox;
class SSearchBox;
class STextBlock;
class UDataTable;
struct FStructOnScope;

class STMOPPeopleEditor final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPPeopleEditor) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);

private:
    using FPersonItem = TSharedPtr<FName>;
    using FTimelineItem = TSharedPtr<int32>;

    static constexpr const TCHAR* DefaultPeopleTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_People.DT_TMOP_People");
    static constexpr const TCHAR* DefaultEventTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_HistoricalEvents.DT_TMOP_HistoricalEvents");

    void LoadDefaultTable();
    void RefreshPeople();
    void RefreshTimeline();
    void SelectPerson(FName RowName);
    void SelectTimelineEntry(int32 Index);
    void CommitEntryEdits();
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
    FName SelectedRowName = NAME_None;
    FTMOPPersonProfileRow WorkingRow;
    int32 SelectedTimelineIndex = INDEX_NONE;
    FString PersonSearch;

    TArray<FPersonItem> PersonItems;
    TArray<FTimelineItem> TimelineItems;
    TSharedPtr<SListView<FPersonItem>> PersonListView;
    TSharedPtr<SListView<FTimelineItem>> TimelineListView;
    TSharedPtr<IStructureDetailsView> EntryDetailsView;
    TSharedPtr<FStructOnScope> EntryStructData;
    TSharedPtr<STextBlock> StatusText;
};
