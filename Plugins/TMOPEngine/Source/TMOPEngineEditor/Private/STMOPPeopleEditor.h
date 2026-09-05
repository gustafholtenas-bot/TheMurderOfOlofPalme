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
        Suspect,
        Spawned,
        NonSpawned,
        MainCharacters
    };

    enum class EReferenceField : uint8
    {
        TargetAnchor,
        TargetEntity,
        TargetGroup,
        TargetSeat,
        SharedEvent
    };

    static constexpr const TCHAR* DefaultPeopleTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_People.DT_TMOP_People");
    static constexpr const TCHAR* DefaultEventTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_HistoricalEvents.DT_TMOP_HistoricalEvents");
    static constexpr const TCHAR* DefaultVehicleTablePath =
        TEXT("/Game/TMOP/Vehicles/DT_TMOP_HistoricalVehicles.DT_TMOP_HistoricalVehicles");
    static constexpr const TCHAR* DefaultAppearanceTablePath =
        TEXT("/Game/TMOP/Characters/Appearance/Data/DT_TMOP_AppearanceAssets.DT_TMOP_AppearanceAssets");

    void LoadDefaultTable();
    void RefreshPeople();
    void RefreshTimeline();
    EActiveTimerReturnType HandleRuntimeValidationRefresh(
        double CurrentTime, float DeltaTime);
    void RefreshComparisonPeople();
    void SelectDefaultComparisonPerson();
    void RefreshComparisonTimeline();
    void SelectPerson(FName RowName);
    void SelectTimelineEntry(int32 Index);
    void CommitEntryEdits();
    void RefreshPersonDetailViews();
    void CommitPersonDetailEdits();
    bool HasUnsavedPersonChanges();
    bool SaveCurrentPerson();
    void RestorePersonListSelection();
    void SetStatus(const FText& Message, const FLinearColor& Color);

    TSharedRef<ITableRow> GeneratePersonRow(
        FPersonItem Item,
        const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> GenerateTimelineRow(
        FTimelineItem Item,
        const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<ITableRow> GenerateComparisonTimelineRow(
        FTimelineItem Item,
        const TSharedRef<STableViewBase>& OwnerTable);
    TSharedRef<SWidget> GenerateComparisonPersonOption(
        FReferenceItem Item) const;
    void HandleComparisonPersonSelected(
        FReferenceItem Item, ESelectInfo::Type SelectInfo);
    FText GetComparisonPersonText() const;
    void HandlePersonSelectionChanged(
        FPersonItem Item, ESelectInfo::Type SelectInfo);
    void HandleTimelineSelectionChanged(
        FTimelineItem Item, ESelectInfo::Type SelectInfo);
    void HandleComparisonTimelineSelectionChanged(
        FTimelineItem Item, ESelectInfo::Type SelectInfo);
    TSharedPtr<SWidget> BuildComparisonTimelineContextMenu();
    void ApplyReferenceTimeToNearestTimelineEntry();
    int32 FindClosestWorkingTimelineIndex(
        int32 ReferenceIndex,
        int32* OutDeltaSeconds = nullptr) const;
    void HandlePersonSearchChanged(const FText& SearchText);
    ECheckBoxState GetPeopleFilterCheckState(
        EPeopleCategoryFilter Filter) const;
    void HandlePeopleFilterChanged(
        ECheckBoxState NewState,
        EPeopleCategoryFilter Filter);
    bool IsMainCharacter(const FTMOPPersonProfileRow& Row) const;
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
    FReply CopyPersonTimeline();
    FReply PastePersonTimeline();
    FReply SavePerson();
    FReply ReloadPerson();
    FReply ResolveCurrentAppearance();
    FReply ValidateAllAppearances();

    FText GetSelectedPersonTitle() const;
    FText GetSelectedPersonSubtitle() const;
    FText GetCopiedTimelineStatusText() const;
    FText GetTimelineSummary(int32 Index) const;
    FText GetTimelineTimingText(const FTMOPPersonTimelineEntry& Entry) const;
    bool ResolveTimelineDisplaySecond(
        int32 Index,
        int32& OutSecond,
        FString* OutFailureReason = nullptr) const;
    bool ResolveTimelineDisplaySecondForRow(
        const FTMOPPersonProfileRow& Row,
        int32 Index,
        int32& OutSecond,
        FString* OutFailureReason = nullptr) const;
    bool ResolveTimelinePositionKey(
        const FTMOPPersonProfileRow& Row,
        int32 Index,
        FString& OutKey,
        FString& OutLabel) const;
    bool BuildTimelineSpeedBadge(
        int32 Index,
        FText& OutText,
        FText& OutToolTip,
        FLinearColor& OutColor) const;
    bool BuildVehicleTimelineSpeedBadge(
        int32 Index,
        FText& OutText,
        FText& OutToolTip,
        FLinearColor& OutColor) const;
    FSlateColor GetTimelineColor(int32 Index) const;
    bool EntryHasError(int32 Index, FString* OutMessage = nullptr) const;
    TArray<FString> ValidateWorkingRow() const;
    TArray<FString> ValidateAppearanceRow(
        const FTMOPPersonProfileRow& Row, UDataTable* Catalog,
        int32* OutUnknownPartCount = nullptr) const;

    TWeakObjectPtr<UDataTable> PeopleTable;
    TWeakObjectPtr<UDataTable> EventTable;
    TWeakObjectPtr<UDataTable> VehicleTable;
    TWeakObjectPtr<UDataTable> AppearanceTable;
    FName SelectedRowName = NAME_None;
    FTMOPPersonProfileRow WorkingRow;
    FTMOPPersonProfileRow LastSavedRow;
    bool bHasLastSavedRow = false;
    bool bRestoringPersonSelection = false;
    int32 SelectedTimelineIndex = INDEX_NONE;
    int32 SelectedComparisonTimelineIndex = INDEX_NONE;
    TArray<FTMOPPersonTimelineEntry> CopiedTimeline;
    FName CopiedTimelineSourceRow = NAME_None;
    FString CopiedTimelineSourceLabel;
    bool bHasCopiedTimeline = false;
    FString PersonSearch;
    EPeopleCategoryFilter PeopleCategoryFilter =
        EPeopleCategoryFilter::All;

    TArray<FPersonItem> PersonItems;
    TArray<FTimelineItem> TimelineItems;
    TArray<FReferenceItem> ComparisonPersonItems;
    TArray<FTimelineItem> ComparisonTimelineItems;
    TSharedPtr<SListView<FPersonItem>> PersonListView;
    TSharedPtr<SListView<FTimelineItem>> TimelineListView;
    TSharedPtr<SListView<FTimelineItem>> ComparisonTimelineListView;
    TSharedPtr<SSearchableComboBox> ComparisonPersonCombo;
    TMap<FString, FName> ComparisonRowNamesByLabel;
    FName ComparisonRowName = NAME_None;
    FTMOPPersonProfileRow ComparisonRow;
    bool bHasComparisonRow = false;
    TSharedPtr<IStructureDetailsView> EntryDetailsView;
    TSharedPtr<FStructOnScope> EntryStructData;
    TSharedPtr<IStructureDetailsView> CharacteristicsDetailsView;
    TSharedPtr<IStructureDetailsView> GeneralDetailsView;
    TSharedPtr<FStructOnScope> CharacteristicsStructData;
    TSharedPtr<FStructOnScope> GeneralStructData;
    mutable FSlateBrush ReferenceImageBrush;
    TSharedPtr<STextBlock> StatusText;
    uint64 LastRuntimeValidationRevision = 0;

    TArray<FReferenceItem> AnchorReferenceItems;
    TArray<FReferenceItem> EntityReferenceItems;
    TArray<FReferenceItem> GroupReferenceItems;
    TArray<FReferenceItem> SeatReferenceItems;
    TArray<FReferenceItem> EventReferenceItems;
    TMap<FString, FName> ReferenceIdsByLabel;
    TSharedPtr<SSearchableComboBox> AnchorReferenceCombo;
    TSharedPtr<SSearchableComboBox> EntityReferenceCombo;
    TSharedPtr<SSearchableComboBox> GroupReferenceCombo;
    TSharedPtr<SSearchableComboBox> SeatReferenceCombo;
    TSharedPtr<SSearchableComboBox> EventReferenceCombo;
};
