#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Vehicles/TMOPVehicleRoutePlan.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"

class IStructureDetailsView;
class STMOPAppearancePreview;
class SEditableTextBox;
class SSearchBox;
class SSearchableComboBox;
class STextBlock;
class UDataTable;
class FStructOnScope;
class STMOPVehicleRouteMap;
template<typename OptionType> class SComboBox;
struct FTMOPPersonProfileRow;

class STMOPVehicleEditor final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPVehicleEditor) {}
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
    bool CanClose();
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    bool HasUnsavedChanges();


private:
    bool ConfirmDiscardOrSave();
    void OnDetailsChanged(const FPropertyChangedEvent& Event, bool bVehicleDetails);
    void SyncDetailsFromWorking();
    void QueueStructureData(const TSharedPtr<IStructureDetailsView>& View,
        const TSharedPtr<FStructOnScope>& Data);
    bool FlushStructureData(float DeltaTime);
    struct FPendingStructureUpdate
    {
        TSharedPtr<IStructureDetailsView> View;
        TSharedPtr<FStructOnScope> Data;
    };
    TArray<FPendingStructureUpdate> PendingStructureUpdates;
    TArray<FPendingStructureUpdate> DisplayedStructures;
    bool bStructureUpdateQueued = false;
    void RefreshAppearancePreview();
    TSharedRef<SWidget> BuildAccessoryControls();
    void RefreshAccessoryChoices();
    void SelectAccessory(TSharedPtr<int32> Item, ESelectInfo::Type);
    void OnAccessoryDetailsChanged(const FPropertyChangedEvent& Event);
    FReply AddAccessory(ETMOPRoofAccessoryType Type);
    FReply RemoveAccessory();
    void SelectAccessorySocket(TSharedPtr<FString> Item, ESelectInfo::Type);
    TSharedPtr<STMOPAppearancePreview> AppearancePreview;
    TSharedPtr<IStructureDetailsView> AccessoryDetails;
    TSharedPtr<FStructOnScope> AccessoryStruct;
    TArray<TSharedPtr<int32>> AccessoryChoices;
    TArray<TSharedPtr<FString>> AccessorySockets;
    TSharedPtr<SComboBox<TSharedPtr<int32>>> AccessoryCombo;
    TSharedPtr<SSearchableComboBox> AccessorySocketCombo;
    int32 SelectedAccessoryIndex = INDEX_NONE;
    bool bRefreshingAccessories = false;
    TSharedRef<SWidget> BuildDrivingControls();
    FText GetDrivingSummary() const;
    FText BuildDrivingSummary() const;
    FText BuildRouteEndpointsText() const;
    FText GetClockField(bool bArrival) const;
    void SetClockField(const FText& Text, ETextCommit::Type CommitType, bool bArrival);
    void SetStopDuration(int32 Seconds);
    EVisibility DrivingVisibility() const;
    EVisibility LaneVisibility() const;
    EVisibility PlacementVisibility() const;
    EVisibility StopVisibility() const;
    void SetPreviewAlpha(float Alpha);
    FReply TogglePreview();
    EActiveTimerReturnType TickPreview(double Now, float DeltaTime);
    FString GetEntryFingerprint(int32 Index) const;
    void RefreshValidationItems();
    struct FValidationItem
    {
        FName VehicleRow;
        int32 EntryIndex = INDEX_NONE;
        FString Message;
    };
    using FValidationItemPtr = TSharedPtr<FValidationItem>;
    TSharedRef<ITableRow> GenerateValidationRow(
        FValidationItemPtr Item, const TSharedRef<STableViewBase>& Owner);

    struct FBoardingFeasibility
    {
        bool bFoundBoardingEntry = false;
        bool bUsedStraightLineFallback = false;
        bool bRouteResolved = false;
        int32 BoardingSecond = INDEX_NONE;
        int32 DepartureSecond = INDEX_NONE;
        int32 AvailableSeconds = 0;
        int32 RequiredSeconds = 0;
        int32 MarginSeconds = 0;
        double DistanceCm = 0.0;
        float SpeedCmPerSecond = 0.0f;
        FName SeatId = NAME_None;
        FString Failure;
    };

    using FVehicleItem = TSharedPtr<FName>;
    using FTimelineItem = TSharedPtr<int32>;
    using FAnchorItem = TSharedPtr<FString>;
    using FEventItem = TSharedPtr<FString>;
    using FLaneItem = TSharedPtr<FString>;

    enum class ERouteReferenceField : uint8
    {
        StartAnchor,
        StartLane,
        DestinationAnchor,
        DestinationLane,
        ViaAnchor,
        ViaLane
    };

    enum class EVehicleListFilter : uint8
    {
        AllCars,
        SpawnedCars,
        SpawnedCarsWithTimelines,
        MainWitnessCars,
        PoliceCars
    };

    static constexpr const TCHAR* VehicleTablePath =
        TEXT("/Game/TMOP/Vehicles/DT_TMOP_HistoricalVehicles.DT_TMOP_HistoricalVehicles");
    static constexpr const TCHAR* PeopleTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_People.DT_TMOP_People");
    static constexpr const TCHAR* EventTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_HistoricalEvents.DT_TMOP_HistoricalEvents");

    void LoadTables();
    void RefreshVehicles();
    void RefreshTimeline();
    EActiveTimerReturnType HandleRuntimeValidationRefresh(
        double CurrentTime, float DeltaTime);
    void SelectVehicle(FName RowName);
    void SelectTimelineEntry(int32 Index);
    void CommitEntry();
    void CommitVehicle();
    void RebuildRoutePreview();
    void RefreshAnchorOptions();
    void RefreshLaneOptions();
    void RefreshEventOptions();
    void RebuildValidation();
    void SetStatus(const FText& Text, const FLinearColor& Color);

    TSharedRef<ITableRow> GenerateVehicleRow(
        FVehicleItem Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<ITableRow> GenerateTimelineRow(
        FTimelineItem Item, const TSharedRef<STableViewBase>& Owner);
    void OnVehicleSelected(FVehicleItem Item, ESelectInfo::Type);
    void OnTimelineSelected(FTimelineItem Item, ESelectInfo::Type);
    void OnSearchChanged(const FText& Text);
    ECheckBoxState GetVehicleFilterCheckState(EVehicleListFilter Filter) const;
    void HandleVehicleFilterChanged(
        ECheckBoxState NewState, EVehicleListFilter Filter);
    bool PassesVehicleFilter(const FTMOPHistoricalVehicleRow& Row) const;
    TSharedRef<SWidget> GenerateAnchorOption(FAnchorItem Item) const;
    void OnAnchorSelected(FAnchorItem Item, ESelectInfo::Type SelectInfo);
    FText GetSelectedAnchorText() const;
    TSharedRef<SWidget> GenerateEventOption(FEventItem Item) const;
    void OnEventSelected(FEventItem Item, ESelectInfo::Type SelectInfo);
    FText GetSelectedEventText() const;
    void OnDepartureEventSelected(FEventItem Item,
        ESelectInfo::Type SelectInfo);
    FText GetSelectedDepartureEventText() const;
    TSharedRef<SWidget> GenerateLaneOption(FLaneItem Item) const;
    void OnRouteReferenceSelected(TSharedPtr<FString> Item,
        ESelectInfo::Type SelectInfo, ERouteReferenceField Field);
    FText GetRouteReferenceText(ERouteReferenceField Field) const;
    FText GetRouteEndpointsText() const;
    FReply RecalculateRoute();
    FReply ClearViaPoints();
    FReply PreviewRouteInLevel();
    void HandleMapLaneClicked(FName LaneId, bool bSetStart,
        bool bSetDestination, bool bAddVia);
    bool RecalculateSelectedRoute(FString& OutFailure);
    FReply AddEntry();
    FReply DuplicateEntry();
    FReply DeleteEntry();
    FReply MoveEntry(int32 Direction);
    FReply SaveVehicle();
    FReply ReloadVehicle();
    FReply ValidateAll();

    bool ResolveTime(const FTMOPHistoricalVehicleRow& Row,
        int32 Index, int32& OutSecond, FString* Failure = nullptr) const;
    bool ResolveEntryCompletionTime(const FTMOPHistoricalVehicleRow& Row,
        int32 Index, int32& OutSecond, FString* Failure = nullptr) const;
    bool ResolveDrivingDepartureTime(const FTMOPHistoricalVehicleRow& Row,
        int32 Index, int32& OutSecond, FString* Failure = nullptr) const;
    bool CalculateDrive(const FTMOPHistoricalVehicleRow& Row,
        int32 Index, double& OutDistanceCm,
        int32& OutDepartureSecond, int32& OutArrivalSecond,
        int32& OutDurationSeconds, double& OutKmh,
        FString& OutFailure) const;
    TArray<FString> ValidateRow(const FTMOPHistoricalVehicleRow& Row) const;
    bool CalculateBoardingFeasibility(
        const FTMOPHistoricalVehicleRow& Vehicle,
        int32 DrivingIndex,
        const FTMOPPersonProfileRow& Person,
        FBoardingFeasibility& OutResult) const;
    bool ResolvePersonTime(const FTMOPPersonProfileRow& Person,
        int32 Index, int32& OutSecond) const;
    FString BuildOccupantsText(int32 TimelineIndex) const;
    FText GetTitle() const;
    FText GetSubtitle() const;
    FText GetValidationText() const;

    TWeakObjectPtr<UDataTable> VehicleTable;
    TWeakObjectPtr<UDataTable> PeopleTable;
    TWeakObjectPtr<UDataTable> EventTable;
    FName SelectedRowName = NAME_None;
    int32 SelectedTimelineIndex = INDEX_NONE;
    FString Search;
    EVehicleListFilter VehicleListFilter = EVehicleListFilter::AllCars;
    FTMOPHistoricalVehicleRow WorkingRow;
    FTMOPHistoricalVehicleRow SavedRow;
    TArray<FString> CurrentErrors;

    TArray<FVehicleItem> VehicleItems;
    TArray<FTimelineItem> TimelineItems;
    TArray<FAnchorItem> AnchorItems;
    TArray<FEventItem> EventItems;
    TArray<FLaneItem> LaneItems;
    TMap<FString, FName> AnchorIdsByLabel;
    TMap<FString, FName> EventIdsByLabel;
    TMap<FString, FName> LaneIdsByLabel;
    TSharedPtr<SListView<FVehicleItem>> VehicleList;
    TSharedPtr<SListView<FTimelineItem>> TimelineList;
    TSharedPtr<IStructureDetailsView> VehicleDetails;
    TSharedPtr<IStructureDetailsView> EntryDetails;
    TSharedPtr<FStructOnScope> VehicleStruct;
    TSharedPtr<FStructOnScope> EntryStruct;
    TSharedPtr<STMOPVehicleRouteMap> RouteMap;
    TSharedPtr<SSearchableComboBox> AnchorCombo;
    TSharedPtr<SSearchableComboBox> EventCombo;
    TSharedPtr<SSearchableComboBox> DepartureEventCombo;
    TSharedPtr<SSearchableComboBox> StartAnchorCombo;
    TSharedPtr<SSearchableComboBox> StartLaneCombo;
    TSharedPtr<SSearchableComboBox> DestinationAnchorCombo;
    TSharedPtr<SSearchableComboBox> DestinationLaneCombo;
    TSharedPtr<SSearchableComboBox> ViaAnchorCombo;
    TSharedPtr<SSearchableComboBox> ViaLaneCombo;
    TSharedPtr<STextBlock> StatusText;
    uint64 LastRuntimeValidationRevision = 0;
    bool bChangingSelection = false;
    bool bSynchronizingDetails = false;
    bool bPreviewPlaying = false;
    bool bPreviewTimerRegistered = false;
    bool bPreviewInLevel = false;
    float PreviewAlpha = 0.0f;
    int32 PreviewDeparture = 0;
    int32 PreviewArrival = 0;
    FTMOPVehicleRoutePlan PreviewPlan;
    FString CachedOccupants;
    FText CachedDrivingSummary;
    FText CachedRouteEndpoints;
    mutable TMap<int32, FString> CachedFingerprints;
    TArray<FValidationItemPtr> ValidationItems;
    TSharedPtr<SListView<FValidationItemPtr>> ValidationList;
};
