#pragma once

#include "CoreMinimal.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class STextBlock;
class UTMOPTrafficLaneComponent;

/** Visual, transactional repair tool for the lane graph in the open editor world. */
class STMOPLaneRepairEditor final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPLaneRepairEditor) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& Args);

private:
    enum class ESuggestionKind : uint8
    {
        Safe,
        Restricted,
        Review
    };

    struct FLaneSuggestion
    {
        TWeakObjectPtr<UTMOPTrafficLaneComponent> From;
        TWeakObjectPtr<UTMOPTrafficLaneComponent> To;
        ESuggestionKind Kind = ESuggestionKind::Review;
        ETMOPTrafficTurnType TurnType = ETMOPTrafficTurnType::Straight;
        float GapCm = 0.0f;
        float HeadingChangeDegrees = 0.0f;
        bool bSelected = false;
        FString Reason;
    };

    using FSuggestionItem = TSharedPtr<FLaneSuggestion>;

    FReply ScanNetwork();
    FReply ApplySelected();
    FReply SelectSafe();
    FReply SelectRestricted();
    FReply ClearSelection();
    FReply AssignLaneNeighbours();
    FReply SnapConnectorEnds();
    void FocusSuggestion(FSuggestionItem Item);

    void CollectLanes(TArray<UTMOPTrafficLaneComponent*>& OutLanes) const;
    void BuildExistingRoadConnections(
        const TArray<UTMOPTrafficLaneComponent*>& Lanes,
        TSet<FString>& OutPairs) const;
    bool CreateConnector(const FLaneSuggestion& Suggestion,
        FString& OutFailure);
    void RefreshSummary();

    TSharedRef<ITableRow> GenerateSuggestionRow(
        FSuggestionItem Item, const TSharedRef<STableViewBase>& OwnerTable);
    FText GetKindText(ESuggestionKind Kind) const;
    FSlateColor GetKindColor(ESuggestionKind Kind) const;

    static bool IsRoadLane(const UTMOPTrafficLaneComponent* Lane);
    static bool IsRestrictedCorridor(const UTMOPTrafficLaneComponent* Lane);
    static FString MakePairKey(FName From, FName To);
    static FString MakeLanePairStem(FName LaneId);
    static FString TurnTypeToken(ETMOPTrafficTurnType TurnType);

    TArray<FSuggestionItem> Suggestions;
    TSharedPtr<SListView<FSuggestionItem>> SuggestionList;
    TSharedPtr<STextBlock> SummaryText;
    TSharedPtr<STextBlock> DiagnosticsText;
    TSharedPtr<STextBlock> StatusText;

    TArray<FString> Diagnostics;

    int32 RoadLaneCount = 0;
    int32 ConnectorLaneCount = 0;
    int32 IsolatedLaneCount = 0;
    int32 BrokenConnectorCount = 0;
    int32 ReversedPairCount = 0;
};
