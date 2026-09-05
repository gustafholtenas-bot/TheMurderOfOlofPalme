#pragma once

#include "CoreMinimal.h"
#include "Observations/TMOPObservationTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class IStructureDetailsView;
class STextBlock;
class UDataTable;
class FStructOnScope;
class STMOPObservationMap;

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

    static constexpr const TCHAR* ObservationTablePath =
        TEXT("/Game/TMOP/Observations/DT_TMOP_Observations.DT_TMOP_Observations");
    static constexpr const TCHAR* LinkTablePath =
        TEXT("/Game/TMOP/Observations/DT_TMOP_ObservationLinks.DT_TMOP_ObservationLinks");

    void LoadTables();
    void RefreshAll();
    void RefreshObservations();
    void RefreshLinks();
    void RefreshMembers();
    void RebuildMap();
    void SelectLink(FName RowName);
    void CommitLinkDetails();
    void RefreshLinkDetails();
    void AddObservation(FName ObservationId);
    bool IsObservationLinked(FName ObservationId) const;
    int32 ResolveObservationSecond(FName ObservationId) const;
    void SetStatus(const FText& Text, const FLinearColor& Color);

    TSharedRef<ITableRow> GenerateObservationRow(
        FObservationItem Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<ITableRow> GenerateLinkRow(
        FLinkItem Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<ITableRow> GenerateMemberRow(
        FMemberItem Item, const TSharedRef<STableViewBase>& Owner);
    void OnLinkSelected(FLinkItem Item, ESelectInfo::Type);
    void OnSearchChanged(const FText& Text);
    void OnMissingSignalementChanged(ECheckBoxState NewState);
    FReply HandleObservationDragDetected(const FGeometry& Geometry,
        const FPointerEvent& Event, FObservationItem Item);
    FReply HandleMemberDrop(const FGeometry& Geometry,
        const FDragDropEvent& Event);
    FReply NewLink();
    FReply SaveLink();
    FReply Reload();
    FReply AddSelectedObservation();
    FReply RemoveSelectedMember();
    FReply MoveSelectedMember(int32 Direction);
    FReply SortMembersByTime();
    FReply BuildTrackSegments();
    FReply DeleteLink();

    TWeakObjectPtr<UDataTable> ObservationTable;
    TWeakObjectPtr<UDataTable> LinkTable;
    TMap<FName, FTMOPObservationDefinition> ObservationsById;
    FName SelectedLinkRow = NAME_None;
    FTMOPObservationLinkDefinition WorkingLink;
    FString Search;
    bool bShowOnlyMissingSignalement = false;

    TArray<FObservationItem> ObservationItems;
    TArray<FLinkItem> LinkItems;
    TArray<FMemberItem> MemberItems;
    TSharedPtr<SListView<FObservationItem>> ObservationList;
    TSharedPtr<SListView<FLinkItem>> LinkList;
    TSharedPtr<SListView<FMemberItem>> MemberList;
    TSharedPtr<IStructureDetailsView> LinkDetails;
    TSharedPtr<FStructOnScope> LinkStruct;
    TSharedPtr<STMOPObservationMap> Map;
    TSharedPtr<STextBlock> StatusText;
};
