#pragma once

#include "CoreMinimal.h"
#include "Observations/TMOPObservationTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Views/SListView.h"

class IStructureDetailsView;
class STextBlock;
class UDataTable;
class FStructOnScope;
class STMOPObservationMap;
template<typename OptionType> class SComboBox;

/** Visual authoring tool for observation hypotheses and interpolated tracks. */
class STMOPObservationEditor final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPObservationEditor) {}
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);

private:
    using FObservationItem = TSharedPtr<FName>;
    using FLinkItem = TSharedPtr<FName>;
    using FMemberItem = TSharedPtr<FName>;
    using FPersonItem = TSharedPtr<FName>;
    using FGeoFilterItem = TSharedPtr<FName>;

    static constexpr const TCHAR* ObservationTablePath =
        TEXT("/Game/TMOP/Observations/DT_TMOP_Observations.DT_TMOP_Observations");
    static constexpr const TCHAR* LinkTablePath =
        TEXT("/Game/TMOP/Observations/DT_TMOP_ObservationLinks.DT_TMOP_ObservationLinks");
    static constexpr const TCHAR* PeopleTablePath =
        TEXT("/Game/TMOP/Agents/People/DT_TMOP_People.DT_TMOP_People");

    void LoadTables();
    void RefreshAll();
    void RefreshObservations();
    void RefreshLinks();
    void RefreshMembers();
    void RefreshKnownPeople();
    void RefreshObservationInfo();
    void RefreshLevelGeometry();
    void RefreshValidationReport();
    void RebuildMap();
    void SelectLink(FName RowName);
    void CommitLinkDetails();
    void RefreshLinkDetails();
    void AddObservation(FName ObservationId);
    bool IsObservationLinked(FName ObservationId) const;
    bool IsKnownPersonEntity(FName EntityId) const;
    bool MatchesGeoFilter(const FTMOPObservationDefinition& Observation) const;
    bool MatchesTimeFilter(const FTMOPObservationDefinition& Observation) const;
    bool CalculatePreviewPosition(FVector2D& OutPosition,
        FString& OutDescription) const;
    FString BuildObservationSearchText(
        const FTMOPObservationDefinition& Observation) const;
    int32 RemoveObservationFromLinks(FName ObservationId);
    bool SaveObservationIdentity(
        ETMOPObservedPersonIdentityStatus NewStatus, FName PersonEntityId);
    int32 ResolveObservationSecond(FName ObservationId) const;
    void SetStatus(const FText& Text, const FLinearColor& Color);

    TSharedRef<ITableRow> GenerateObservationRow(
        FObservationItem Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<ITableRow> GenerateLinkRow(
        FLinkItem Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<ITableRow> GenerateMemberRow(
        FMemberItem Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<SWidget> GenerateKnownPersonOption(FPersonItem Item) const;
    TSharedRef<SWidget> GenerateGeoFilterOption(FGeoFilterItem Item) const;
    FText GetSelectedKnownPersonText() const;
    FText GetSelectedGeoFilterText() const;
    FText GetPreviewTimeText() const;
    float GetGeoFilterRadiusMeters() const;
    float GetPreviewSliderValue() const;
    void OnObservationSelected(FObservationItem Item, ESelectInfo::Type);
    void OnLinkSelected(FLinkItem Item, ESelectInfo::Type);
    void OnKnownPersonSelected(FPersonItem Item, ESelectInfo::Type);
    void OnGeoFilterSelected(FGeoFilterItem Item, ESelectInfo::Type);
    void OnGeoFilterRadiusCommitted(float Value, ETextCommit::Type CommitType);
    void OnPreviewSliderChanged(float Value);
    void OnMapObservationSelected(FName ObservationId);
    void OnKnownPersonSearchChanged(const FText& Text);
    void OnAllTimesChanged(ECheckBoxState NewState);
    void OnTimeStartCommitted(const FText& Text, ETextCommit::Type CommitType);
    void OnTimeEndCommitted(const FText& Text, ETextCommit::Type CommitType);
    void OnSearchChanged(const FText& Text);
    void OnMissingSignalementChanged(ECheckBoxState NewState);
    FReply HandleObservationDragDetected(const FGeometry& Geometry,
        const FPointerEvent& Event, FObservationItem Item);
    FReply HandleMemberDrop(const FGeometry& Geometry,
        const FDragDropEvent& Event);
    FReply NewLink();
    FReply SaveLink();
    FReply Reload();
    FReply ValidateAll();
    FReply AddSelectedObservation();
    FReply MarkSelectedUnknownPerson();
    FReply MarkSelectedKnownPerson();
    FReply RemoveSelectedMember();
    FReply MoveSelectedMember(int32 Direction);
    FReply SortMembersByTime();
    FReply BuildTrackSegments();
    FReply DeleteLink();

    TWeakObjectPtr<UDataTable> ObservationTable;
    TWeakObjectPtr<UDataTable> LinkTable;
    TWeakObjectPtr<UDataTable> PeopleTable;
    TMap<FName, FTMOPObservationDefinition> ObservationsById;
    TMap<FName, FName> ObservationRowNamesById;
    TMap<FName, FText> PersonDisplayNames;
    TSet<FName> KnownPersonIds;
    FName SelectedLinkRow = NAME_None;
    FName SelectedObservationId = NAME_None;
    FName SelectedKnownPersonId = NAME_None;
    FTMOPObservationLinkDefinition WorkingLink;
    FString Search;
    FString KnownPersonSearch;
    bool bShowOnlyMissingSignalement = false;
    bool bAllTimes = true;
    int32 TimeFilterStartSecond = 22 * 3600 + 55 * 60;
    int32 TimeFilterEndSecond = 23 * 3600 + 55 * 60;
    FName SelectedGeoFilter = TEXT("AllPlayArea");
    int32 PreviewSecond = 23 * 3600;

    TMap<FName, FVector2D> AnchorPositions;
    TArray<TArray<FVector2D>> CachedLanePolylines;
    TMap<FName, TArray<FVector2D>> GeoFilterSeedPositions;
    TMap<FName, float> GeoFilterRadiusCm;

    TArray<FObservationItem> ObservationItems;
    TArray<FLinkItem> LinkItems;
    TArray<FMemberItem> MemberItems;
    TArray<FPersonItem> KnownPersonItems;
    TArray<FGeoFilterItem> GeoFilterItems;
    TSharedPtr<SListView<FObservationItem>> ObservationList;
    TSharedPtr<SListView<FLinkItem>> LinkList;
    TSharedPtr<SListView<FMemberItem>> MemberList;
    TSharedPtr<SComboBox<FPersonItem>> KnownPersonCombo;
    TSharedPtr<SComboBox<FGeoFilterItem>> GeoFilterCombo;
    TSharedPtr<IStructureDetailsView> LinkDetails;
    TSharedPtr<FStructOnScope> LinkStruct;
    TSharedPtr<STMOPObservationMap> Map;
    TSharedPtr<STextBlock> ObservationInfoText;
    TSharedPtr<STextBlock> ValidationText;
    TSharedPtr<STextBlock> StatusText;
};
