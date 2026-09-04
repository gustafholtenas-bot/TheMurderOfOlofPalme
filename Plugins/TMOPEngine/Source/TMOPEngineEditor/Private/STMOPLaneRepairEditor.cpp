#include "STMOPLaneRepairEditor.h"

#include "Components/SceneComponent.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Misc/MessageDialog.h"
#include "ScopedTransaction.h"
#include "Styling/CoreStyle.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "STMOPLaneRepairEditor"

namespace TMOPLaneRepair
{
constexpr float MaximumCandidateGapCm = 1800.0f;
constexpr float MaximumStraightGapCm = 2500.0f;
constexpr float SafeStraightGapCm = 800.0f;
constexpr float SafeStraightAngleDegrees = 25.0f;
constexpr float MaximumTurnAngleDegrees = 155.0f;
constexpr float MaximumSnapGapCm = 500.0f;
constexpr float BrokenConnectorGapCm = 100.0f;

FString NormalizedToken(const FName Value)
{
    FString Result = Value.ToString().ToUpper();
    Result.ReplaceInline(TEXT("Ä"), TEXT("A"));
    Result.ReplaceInline(TEXT("Å"), TEXT("A"));
    Result.ReplaceInline(TEXT("Ö"), TEXT("O"));
    return Result;
}

float SignedHeadingChange(const FVector& FromDirection,
    const FVector& ToDirection)
{
    const FVector A = FVector(FromDirection.X, FromDirection.Y, 0.0f).GetSafeNormal();
    const FVector B = FVector(ToDirection.X, ToDirection.Y, 0.0f).GetSafeNormal();
    if (A.IsNearlyZero() || B.IsNearlyZero()) return 180.0f;
    return FMath::RadiansToDegrees(FMath::Atan2(
        FVector::CrossProduct(A, B).Z,
        FVector::DotProduct(A, B)));
}

bool HasConnectionTo(const UTMOPTrafficLaneComponent* Lane, FName Target)
{
    return IsValid(Lane) && Lane->NextLanes.ContainsByPredicate(
        [Target](const FTMOPLaneConnection& Connection)
        {
            return Connection.TargetLaneId == Target;
        });
}
}

void STMOPLaneRepairEditor::Construct(const FArguments& Args)
{
    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Title", "TMOP LANE REPAIR"))
            .Font(FCoreStyle::GetDefaultFontStyle("Bold", 18))
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f, 8.0f, 8.0f)
        [
            SNew(STextBlock)
            .Text(LOCTEXT("Explanation",
                "Scans the open level. Safe connections are exact continuations; restricted connections are disabled for normal traffic and require Ignore One-Way. Review suggestions are never selected automatically."))
            .AutoWrapText(true)
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 0.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("Scan", "1. Scan Network")).OnClicked(this, &STMOPLaneRepairEditor::ScanNetwork) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("SelectSafe", "Select Safe")).OnClicked(this, &STMOPLaneRepairEditor::SelectSafe) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("SelectRestricted", "Select Restricted")).OnClicked(this, &STMOPLaneRepairEditor::SelectRestricted) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("Clear", "Clear Selection")).OnClicked(this, &STMOPLaneRepairEditor::ClearSelection) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("Apply", "2. Apply Checked Connectors")).OnClicked(this, &STMOPLaneRepairEditor::ApplySelected) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 4.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("Neighbours", "Assign R1/R2 Neighbours")).OnClicked(this, &STMOPLaneRepairEditor::AssignLaneNeighbours) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(2.0f)
            [ SNew(SButton).Text(LOCTEXT("Snap", "Snap Connector Ends <= 5 m")).OnClicked(this, &STMOPLaneRepairEditor::SnapConnectorEnds) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(10.0f, 4.0f)
        [ SAssignNew(SummaryText, STextBlock).AutoWrapText(true) ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f, 2.0f)
        [
            SNew(SBox).HeightOverride(130.0f)
            [
                SNew(SBorder).Padding(5.0f)
                [
                    SNew(SScrollBox)
                    + SScrollBox::Slot()
                    [ SAssignNew(DiagnosticsText, STextBlock).AutoWrapText(true) ]
                ]
            ]
        ]
        + SVerticalBox::Slot().FillHeight(1.0f).Padding(8.0f)
        [
            SNew(SBorder)
            .Padding(2.0f)
            [
                SAssignNew(SuggestionList, SListView<FSuggestionItem>)
                .ListItemsSource(&Suggestions)
                .SelectionMode(ESelectionMode::Single)
                .OnGenerateRow(this, &STMOPLaneRepairEditor::GenerateSuggestionRow)
                .OnMouseButtonDoubleClick(this, &STMOPLaneRepairEditor::FocusSuggestion)
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
        [
            SAssignNew(StatusText, STextBlock)
            .Text(LOCTEXT("InitialStatus", "Open the traffic level and press Scan Network."))
            .AutoWrapText(true)
        ]
    ];
    RefreshSummary();
}

void STMOPLaneRepairEditor::CollectLanes(
    TArray<UTMOPTrafficLaneComponent*>& OutLanes) const
{
    OutLanes.Reset();
    if (GEditor == nullptr) return;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (World == nullptr) return;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TInlineComponentArray<UTMOPTrafficLaneComponent*> Components(*It);
        for (UTMOPTrafficLaneComponent* Lane : Components)
            if (IsValid(Lane) && !Lane->LaneId.IsNone()) OutLanes.Add(Lane);
    }
}

bool STMOPLaneRepairEditor::IsRoadLane(
    const UTMOPTrafficLaneComponent* Lane)
{
    return IsValid(Lane) && !Lane->LaneId.IsNone() &&
        !Lane->LaneId.ToString().StartsWith(TEXT("X_"));
}

bool STMOPLaneRepairEditor::IsRestrictedCorridor(
    const UTMOPTrafficLaneComponent* Lane)
{
    if (!IsRoadLane(Lane)) return false;
    const FString Key = TMOPLaneRepair::NormalizedToken(Lane->RoadId) +
        TEXT("|") + TMOPLaneRepair::NormalizedToken(Lane->DirectionId);
    static const TSet<FString> Restricted =
    {
        TEXT("ADOLFFREDRIKSKYRKOG|W"), TEXT("APELBERGSGATAN|W"),
        TEXT("DAVIDBAGARESG|E"), TEXT("DOBELSGATAN|N"),
        TEXT("DOBELSGATAN|W"), TEXT("HOLLANDAREGATAN|S"),
        TEXT("JOHANNESGATAN|N"), TEXT("KAMMAKAREGATAN|E"),
        TEXT("LUNTMAKARGATAN|N"), TEXT("MALMSKILLNADSVAG|N"),
        TEXT("OLOFGATAN|N"), TEXT("REGERINGSGATAN|S"),
        TEXT("ROSENGATAN|E"), TEXT("SALTMASTAREGATAN|N"),
        TEXT("TEGNERGATAN|W")
    };
    return Restricted.Contains(Key);
}

FString STMOPLaneRepairEditor::MakePairKey(FName From, FName To)
{
    return From.ToString() + TEXT("|") + To.ToString();
}

FString STMOPLaneRepairEditor::MakeLanePairStem(FName LaneId)
{
    FString Text = LaneId.ToString();
    int32 Underscore = INDEX_NONE;
    if (!Text.FindLastChar(TEXT('_'), Underscore)) return Text;
    const FString Suffix = Text.Mid(Underscore + 1);
    if (Suffix.Len() < 2 || Suffix[0] != TEXT('R')) return Text;
    for (int32 Index = 1; Index < Suffix.Len(); ++Index)
        if (!FChar::IsDigit(Suffix[Index])) return Text;
    return Text.Left(Underscore);
}

FString STMOPLaneRepairEditor::TurnTypeToken(
    ETMOPTrafficTurnType TurnType)
{
    switch (TurnType)
    {
    case ETMOPTrafficTurnType::Left: return TEXT("LEFT");
    case ETMOPTrafficTurnType::Right: return TEXT("RIGHT");
    case ETMOPTrafficTurnType::UTurn: return TEXT("UTURN");
    default: return TEXT("STRAIGHT");
    }
}

void STMOPLaneRepairEditor::BuildExistingRoadConnections(
    const TArray<UTMOPTrafficLaneComponent*>& Lanes,
    TSet<FString>& OutPairs) const
{
    OutPairs.Reset();
    TMap<FName, UTMOPTrafficLaneComponent*> ById;
    for (UTMOPTrafficLaneComponent* Lane : Lanes) ById.Add(Lane->LaneId, Lane);
    for (UTMOPTrafficLaneComponent* Source : Lanes)
    {
        if (!IsRoadLane(Source)) continue;
        for (const FTMOPLaneConnection& First : Source->NextLanes)
        {
            UTMOPTrafficLaneComponent* const* Middle = ById.Find(First.TargetLaneId);
            if (Middle == nullptr) continue;
            if (IsRoadLane(*Middle))
            {
                OutPairs.Add(MakePairKey(Source->LaneId, (*Middle)->LaneId));
                continue;
            }
            for (const FTMOPLaneConnection& Second : (*Middle)->NextLanes)
                if (UTMOPTrafficLaneComponent* const* Target = ById.Find(Second.TargetLaneId))
                    if (IsRoadLane(*Target))
                        OutPairs.Add(MakePairKey(Source->LaneId, (*Target)->LaneId));
        }
    }
}

FReply STMOPLaneRepairEditor::ScanNetwork()
{
    Suggestions.Reset();
    Diagnostics.Reset();
    RoadLaneCount = ConnectorLaneCount = IsolatedLaneCount = 0;
    BrokenConnectorCount = ReversedPairCount = 0;
    TArray<UTMOPTrafficLaneComponent*> Lanes;
    CollectLanes(Lanes);
    if (Lanes.IsEmpty())
    {
        StatusText->SetText(LOCTEXT("NoLanes", "No TMOP traffic lanes were found in the open editor world."));
        RefreshSummary();
        return FReply::Handled();
    }

    TMap<FName, UTMOPTrafficLaneComponent*> ById;
    TMap<FName, int32> Incoming;
    for (UTMOPTrafficLaneComponent* Lane : Lanes)
    {
        ById.Add(Lane->LaneId, Lane);
        if (IsRoadLane(Lane)) ++RoadLaneCount; else ++ConnectorLaneCount;
        Incoming.FindOrAdd(Lane->LaneId) = 0;
    }
    for (UTMOPTrafficLaneComponent* Lane : Lanes)
        for (const FTMOPLaneConnection& Connection : Lane->NextLanes)
            ++Incoming.FindOrAdd(Connection.TargetLaneId);
    for (UTMOPTrafficLaneComponent* Lane : Lanes)
        if (IsRoadLane(Lane) && Lane->NextLanes.IsEmpty() &&
            Incoming.FindRef(Lane->LaneId) == 0)
        {
            ++IsolatedLaneCount;
            Diagnostics.Add(FString::Printf(TEXT("ISOLATED: %s"),
                *Lane->LaneId.ToString()));
        }

    TSet<FString> ExistingPairs;
    BuildExistingRoadConnections(Lanes, ExistingPairs);

    // Existing connector geometry diagnostics.
    for (UTMOPTrafficLaneComponent* Source : Lanes)
    {
        for (const FTMOPLaneConnection& First : Source->NextLanes)
        {
            UTMOPTrafficLaneComponent* const* Target = ById.Find(First.TargetLaneId);
            if (Target == nullptr || !IsValid(*Target)) continue;
            const float Gap = FVector::Dist(
                Source->GetLocationAtSplinePoint(Source->GetNumberOfSplinePoints() - 1,
                    ESplineCoordinateSpace::World),
                (*Target)->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World));
            if (Gap > TMOPLaneRepair::BrokenConnectorGapCm)
            {
                ++BrokenConnectorCount;
                Diagnostics.Add(FString::Printf(
                    TEXT("GAP %.1f m: %s -> %s"), Gap / 100.0f,
                    *Source->LaneId.ToString(), *(*Target)->LaneId.ToString()));
            }
        }
    }

    TMap<FString, TArray<UTMOPTrafficLaneComponent*>> PairGroups;
    for (UTMOPTrafficLaneComponent* Lane : Lanes)
        if (IsRoadLane(Lane)) PairGroups.FindOrAdd(MakeLanePairStem(Lane->LaneId)).Add(Lane);
    for (const TPair<FString, TArray<UTMOPTrafficLaneComponent*>>& Pair : PairGroups)
    {
        if (Pair.Value.Num() != 2) continue;
        const FVector A = Pair.Value[0]->GetDirectionAtSplinePoint(0,
            ESplineCoordinateSpace::World).GetSafeNormal2D();
        const FVector B = Pair.Value[1]->GetDirectionAtSplinePoint(0,
            ESplineCoordinateSpace::World).GetSafeNormal2D();
        if (FVector::DotProduct(A, B) < -0.8f)
        {
            ++ReversedPairCount;
            Diagnostics.Add(FString::Printf(TEXT("REVERSED R-PAIR: %s / %s"),
                *Pair.Value[0]->LaneId.ToString(), *Pair.Value[1]->LaneId.ToString()));
        }
    }

    TArray<UTMOPTrafficLaneComponent*> Roads;
    for (UTMOPTrafficLaneComponent* Lane : Lanes)
        if (IsRoadLane(Lane)) Roads.Add(Lane);

    for (UTMOPTrafficLaneComponent* From : Roads)
    {
        if (From->GetNumberOfSplinePoints() < 2) continue;
        const FVector FromEnd = From->GetLocationAtSplinePoint(
            From->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
        const FVector FromDirection = From->GetDirectionAtSplinePoint(
            From->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
        for (UTMOPTrafficLaneComponent* To : Roads)
        {
            if (From == To || ExistingPairs.Contains(MakePairKey(From->LaneId, To->LaneId)) ||
                To->GetNumberOfSplinePoints() < 2) continue;
            const FVector ToStart = To->GetLocationAtSplinePoint(0,
                ESplineCoordinateSpace::World);
            const float Gap = FVector::Dist2D(FromEnd, ToStart);
            const bool bSameCorridor = From->RoadId == To->RoadId &&
                From->DirectionId == To->DirectionId;
            const float MaxGap = bSameCorridor
                ? TMOPLaneRepair::MaximumStraightGapCm
                : TMOPLaneRepair::MaximumCandidateGapCm;
            if (Gap > MaxGap || FMath::Abs(FromEnd.Z - ToStart.Z) > 400.0f) continue;

            const FVector ToDirection = To->GetDirectionAtSplinePoint(0,
                ESplineCoordinateSpace::World);
            const float Heading = TMOPLaneRepair::SignedHeadingChange(
                FromDirection, ToDirection);
            const float AbsoluteHeading = FMath::Abs(Heading);
            if (AbsoluteHeading > TMOPLaneRepair::MaximumTurnAngleDegrees) continue;

            const bool bRestricted = IsRestrictedCorridor(From) ||
                IsRestrictedCorridor(To);
            const bool bSafe = bSameCorridor &&
                Gap <= TMOPLaneRepair::SafeStraightGapCm &&
                AbsoluteHeading <= TMOPLaneRepair::SafeStraightAngleDegrees;
            FSuggestionItem Item = MakeShared<FLaneSuggestion>();
            Item->From = From;
            Item->To = To;
            Item->GapCm = Gap;
            Item->HeadingChangeDegrees = Heading;
            Item->TurnType = AbsoluteHeading < 25.0f
                ? ETMOPTrafficTurnType::Straight
                : Heading > 0.0f
                    ? ETMOPTrafficTurnType::Left
                    : ETMOPTrafficTurnType::Right;
            Item->Kind = bRestricted ? ESuggestionKind::Restricted
                : bSafe ? ESuggestionKind::Safe : ESuggestionKind::Review;
            Item->bSelected = Item->Kind == ESuggestionKind::Safe;
            Item->Reason = bRestricted
                ? TEXT("Touches a known reverse/one-way corridor; generated connection will be disabled for normal traffic.")
                : bSafe ? TEXT("Close continuation on the same road and direction.")
                : TEXT("Geometrically possible turn; verify signs, lane choice and 1986 traffic rules.");
            Suggestions.Add(Item);
        }
    }

    Suggestions.Sort([](const FSuggestionItem& A, const FSuggestionItem& B)
    {
        if (A->Kind != B->Kind) return static_cast<uint8>(A->Kind) < static_cast<uint8>(B->Kind);
        if (!FMath::IsNearlyEqual(A->GapCm, B->GapCm)) return A->GapCm < B->GapCm;
        return A->From->LaneId.LexicalLess(B->From->LaneId);
    });
    if (!Diagnostics.IsEmpty())
        UE_LOG(LogTemp, Warning, TEXT("TMOP Lane Repair diagnostics:\n%s"),
            *FString::Join(Diagnostics, TEXT("\n")));
    SuggestionList->RequestListRefresh();
    RefreshSummary();
    StatusText->SetText(FText::Format(LOCTEXT("ScanComplete",
        "Scan complete: {0} suggestions. Double-click a row to select its two lane actors in the level."),
        FText::AsNumber(Suggestions.Num())));
    return FReply::Handled();
}

TSharedRef<ITableRow> STMOPLaneRepairEditor::GenerateSuggestionRow(
    FSuggestionItem Item, const TSharedRef<STableViewBase>& OwnerTable)
{
    const FString From = Item.IsValid() && Item->From.IsValid()
        ? Item->From->LaneId.ToString() : TEXT("missing");
    const FString To = Item.IsValid() && Item->To.IsValid()
        ? Item->To->LaneId.ToString() : TEXT("missing");
    return SNew(STableRow<FSuggestionItem>, OwnerTable)
    [
        SNew(SHorizontalBox)
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.0f)
        [
            SNew(SCheckBox)
            .IsChecked_Lambda([Item]() { return Item->bSelected ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([Item](ECheckBoxState State) { Item->bSelected = State == ECheckBoxState::Checked; })
        ]
        + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(4.0f)
        [ SNew(STextBlock).Text(GetKindText(Item->Kind)).ColorAndOpacity(GetKindColor(Item->Kind)) ]
        + SHorizontalBox::Slot().FillWidth(1.0f).VAlign(VAlign_Center).Padding(4.0f)
        [
            SNew(STextBlock)
            .Text(FText::FromString(FString::Printf(TEXT("%s  ->  %s   %s   gap %.1f m   heading %+.0f deg"),
                *From, *To, *TurnTypeToken(Item->TurnType), Item->GapCm / 100.0f,
                Item->HeadingChangeDegrees)))
            .ToolTipText(FText::FromString(Item->Reason))
        ]
    ];
}

FText STMOPLaneRepairEditor::GetKindText(ESuggestionKind Kind) const
{
    switch (Kind)
    {
    case ESuggestionKind::Safe: return LOCTEXT("Safe", "SAFE");
    case ESuggestionKind::Restricted: return LOCTEXT("Restricted", "RESTRICTED");
    default: return LOCTEXT("Review", "REVIEW");
    }
}

FSlateColor STMOPLaneRepairEditor::GetKindColor(ESuggestionKind Kind) const
{
    switch (Kind)
    {
    case ESuggestionKind::Safe: return FLinearColor(0.15f, 0.8f, 0.3f);
    case ESuggestionKind::Restricted: return FLinearColor(1.0f, 0.45f, 0.05f);
    default: return FLinearColor(1.0f, 0.85f, 0.2f);
    }
}

FReply STMOPLaneRepairEditor::SelectSafe()
{
    for (const FSuggestionItem& Item : Suggestions)
        Item->bSelected = Item->Kind == ESuggestionKind::Safe;
    SuggestionList->RequestListRefresh();
    RefreshSummary();
    return FReply::Handled();
}

FReply STMOPLaneRepairEditor::SelectRestricted()
{
    for (const FSuggestionItem& Item : Suggestions)
        Item->bSelected = Item->Kind == ESuggestionKind::Restricted;
    SuggestionList->RequestListRefresh();
    RefreshSummary();
    return FReply::Handled();
}

FReply STMOPLaneRepairEditor::ClearSelection()
{
    for (const FSuggestionItem& Item : Suggestions) Item->bSelected = false;
    SuggestionList->RequestListRefresh();
    RefreshSummary();
    return FReply::Handled();
}

void STMOPLaneRepairEditor::FocusSuggestion(FSuggestionItem Item)
{
    if (!Item.IsValid() || GEditor == nullptr) return;
    GEditor->SelectNone(false, true, false);
    if (Item->From.IsValid() && IsValid(Item->From->GetOwner()))
        GEditor->SelectActor(Item->From->GetOwner(), true, false, true);
    if (Item->To.IsValid() && IsValid(Item->To->GetOwner()))
        GEditor->SelectActor(Item->To->GetOwner(), true, false, true);
    GEditor->NoteSelectionChange();
    TArray<AActor*> Actors;
    if (Item->From.IsValid() && IsValid(Item->From->GetOwner()))
        Actors.Add(Item->From->GetOwner());
    if (Item->To.IsValid() && IsValid(Item->To->GetOwner()))
        Actors.AddUnique(Item->To->GetOwner());
    if (!Actors.IsEmpty()) GEditor->MoveViewportCamerasToActor(Actors, false);
}

bool STMOPLaneRepairEditor::CreateConnector(
    const FLaneSuggestion& Suggestion, FString& OutFailure)
{
    UTMOPTrafficLaneComponent* From = Suggestion.From.Get();
    UTMOPTrafficLaneComponent* To = Suggestion.To.Get();
    if (!IsValid(From) || !IsValid(To) || GEditor == nullptr)
    {
        OutFailure = TEXT("source or destination lane no longer exists");
        return false;
    }
    UWorld* World = From->GetWorld();
    if (World == nullptr || To->GetWorld() != World)
    {
        OutFailure = TEXT("lanes are not in the same editor world");
        return false;
    }
    FString ConnectorText = FString::Printf(TEXT("X_%s_TO_%s_%s"),
        *From->LaneId.ToString(), *To->LaneId.ToString(),
        *TurnTypeToken(Suggestion.TurnType));
    ConnectorText.ReplaceInline(TEXT(" "), TEXT("_"));
    FName ConnectorId(*ConnectorText);
    TArray<UTMOPTrafficLaneComponent*> Existing;
    CollectLanes(Existing);
    for (UTMOPTrafficLaneComponent* Lane : Existing)
        if (Lane->LaneId == ConnectorId)
        {
            OutFailure = FString::Printf(TEXT("connector '%s' already exists"), *ConnectorText);
            return false;
        }

    FActorSpawnParameters Parameters;
    Parameters.OverrideLevel = World->GetCurrentLevel();
    if (AActor* SourceOwner = From->GetOwner())
    {
        Parameters.OverrideLevel = SourceOwner->GetLevel();
    }
    Parameters.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AActor* Actor = World->SpawnActor<AActor>(
        AActor::StaticClass(), FTransform::Identity, Parameters);
    if (!IsValid(Actor))
    {
        OutFailure = TEXT("could not create connector actor");
        return false;
    }
    Actor->Modify();
    Actor->SetActorLabel(ConnectorText);
    Actor->SetFolderPath(TEXT("TMOP Traffic Network/Auto Connectors"));
    Actor->Tags.AddUnique(TEXT("TMOPAutoGeneratedConnector"));

    USceneComponent* Root = NewObject<USceneComponent>(
        Actor, TEXT("Root"), RF_Transactional);
    Actor->SetRootComponent(Root);
    Actor->AddInstanceComponent(Root);
    Root->RegisterComponent();

    UTMOPTrafficLaneComponent* Connector =
        NewObject<UTMOPTrafficLaneComponent>(
            Actor, TEXT("LaneSpline"), RF_Transactional);
    Actor->AddInstanceComponent(Connector);
    Connector->SetupAttachment(Root);
    Connector->RegisterComponent();
    Connector->LaneId = ConnectorId;
    Connector->RoadId = TEXT("CROSSING");
    Connector->DirectionId = TEXT("CROSSING");
    Connector->LaneIndexFromRight = 1;
    Connector->LaneCountSameDirection = 1;
    Connector->ClearSplinePoints(false);

    const int32 FromLast = From->GetNumberOfSplinePoints() - 1;
    const FVector Start = From->GetLocationAtSplinePoint(
        FromLast, ESplineCoordinateSpace::World);
    const FVector End = To->GetLocationAtSplinePoint(
        0, ESplineCoordinateSpace::World);
    const FVector StartDirection = From->GetDirectionAtSplinePoint(
        FromLast, ESplineCoordinateSpace::World).GetSafeNormal();
    const FVector EndDirection = To->GetDirectionAtSplinePoint(
        0, ESplineCoordinateSpace::World).GetSafeNormal();
    const float Handle = FMath::Clamp(FVector::Dist(Start, End) * 0.35f,
        100.0f, 800.0f);
    Connector->AddSplinePoint(Start, ESplineCoordinateSpace::World, false);
    Connector->AddSplinePoint(Start + StartDirection * Handle,
        ESplineCoordinateSpace::World, false);
    Connector->AddSplinePoint(End - EndDirection * Handle,
        ESplineCoordinateSpace::World, false);
    Connector->AddSplinePoint(End, ESplineCoordinateSpace::World, false);
    for (int32 Index = 0; Index < Connector->GetNumberOfSplinePoints(); ++Index)
        Connector->SetSplinePointType(Index, ESplinePointType::Curve, false);
    Connector->UpdateSpline();
    Connector->SetDrawDebug(true);

    const bool bAllowed = Suggestion.Kind != ESuggestionKind::Restricted;
    FTMOPLaneConnection IntoConnector;
    IntoConnector.TargetLaneId = ConnectorId;
    IntoConnector.TurnType = Suggestion.TurnType;
    IntoConnector.bAllowed = bAllowed;
    From->Modify();
    From->NextLanes.Add(IntoConnector);

    FTMOPLaneConnection OutOfConnector;
    OutOfConnector.TargetLaneId = To->LaneId;
    OutOfConnector.TurnType = ETMOPTrafficTurnType::Straight;
    OutOfConnector.bAllowed = bAllowed;
    Connector->NextLanes.Add(OutOfConnector);
    From->MarkPackageDirty();
    Connector->MarkPackageDirty();
    Actor->MarkPackageDirty();
    return true;
}

FReply STMOPLaneRepairEditor::ApplySelected()
{
    int32 SelectedCount = 0;
    bool bHasReview = false;
    bool bHasRestricted = false;
    for (const FSuggestionItem& Item : Suggestions)
        if (Item->bSelected)
        {
            ++SelectedCount;
            bHasReview |= Item->Kind == ESuggestionKind::Review;
            bHasRestricted |= Item->Kind == ESuggestionKind::Restricted;
        }
    if (SelectedCount == 0)
    {
        StatusText->SetText(LOCTEXT("NothingSelected", "No connector suggestions are checked."));
        return FReply::Handled();
    }
    if (bHasReview && FMessageDialog::Open(EAppMsgType::YesNo,
        LOCTEXT("ReviewConfirmation",
            "The selection contains REVIEW connections. Have you verified their direction, lane choice and traffic rules in the level?")) != EAppReturnType::Yes)
        return FReply::Handled();
    if (bHasRestricted && FMessageDialog::Open(EAppMsgType::YesNo,
        LOCTEXT("RestrictedConfirmation",
            "The selection contains RESTRICTED connectors. They will use bAllowed=false and only vehicles with Ignore One-Way can traverse them. Apply them?")) != EAppReturnType::Yes)
        return FReply::Handled();

    const FScopedTransaction Transaction(
        LOCTEXT("ApplyTransaction", "Apply TMOP Lane Repair Suggestions"));
    int32 Created = 0;
    TArray<FString> Failures;
    for (const FSuggestionItem& Item : Suggestions)
    {
        if (!Item->bSelected) continue;
        FString Failure;
        if (CreateConnector(*Item, Failure)) ++Created;
        else Failures.Add(FString::Printf(TEXT("%s -> %s: %s"),
            Item->From.IsValid() ? *Item->From->LaneId.ToString() : TEXT("missing"),
            Item->To.IsValid() ? *Item->To->LaneId.ToString() : TEXT("missing"),
            *Failure));
    }
    if (GEditor != nullptr) GEditor->RedrawLevelEditingViewports(true);
    StatusText->SetText(FText::FromString(FString::Printf(
        TEXT("Created %d connector(s). %d failed. Scan again before applying another batch."),
        Created, Failures.Num())));
    if (!Failures.IsEmpty())
        UE_LOG(LogTemp, Warning, TEXT("TMOP Lane Repair failures:\n%s"),
            *FString::Join(Failures, TEXT("\n")));
    return ScanNetwork();
}

FReply STMOPLaneRepairEditor::AssignLaneNeighbours()
{
    TArray<UTMOPTrafficLaneComponent*> Lanes;
    CollectLanes(Lanes);
    TMap<FString, TArray<UTMOPTrafficLaneComponent*>> Groups;
    for (UTMOPTrafficLaneComponent* Lane : Lanes)
        if (IsRoadLane(Lane)) Groups.FindOrAdd(MakeLanePairStem(Lane->LaneId)).Add(Lane);

    const FScopedTransaction Transaction(
        LOCTEXT("NeighbourTransaction", "Assign TMOP Lane Neighbours"));
    int32 Changed = 0;
    for (TPair<FString, TArray<UTMOPTrafficLaneComponent*>>& Pair : Groups)
    {
        TArray<UTMOPTrafficLaneComponent*>& Group = Pair.Value;
        if (Group.Num() < 2) continue;
        const FVector ReferenceDirection = Group[0]->GetDirectionAtSplinePoint(
            0, ESplineCoordinateSpace::World).GetSafeNormal2D();
        const bool bDirectionsAgree = Group.ContainsByPredicate(
            [&ReferenceDirection](const UTMOPTrafficLaneComponent* Candidate)
            {
                return !IsValid(Candidate) || FVector::DotProduct(
                    ReferenceDirection,
                    Candidate->GetDirectionAtSplinePoint(0,
                        ESplineCoordinateSpace::World).GetSafeNormal2D()) < 0.8f;
            }) == false;
        if (!bDirectionsAgree)
        {
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Lane Repair: skipped neighbour group '%s' because its lanes face different directions."),
                *Pair.Key);
            continue;
        }
        Group.Sort([](const UTMOPTrafficLaneComponent& A,
            const UTMOPTrafficLaneComponent& B)
        {
            return A.LaneIndexFromRight < B.LaneIndexFromRight;
        });
        for (int32 Index = 0; Index < Group.Num(); ++Index)
        {
            UTMOPTrafficLaneComponent* Lane = Group[Index];
            const FName NewRight = Index > 0 ? Group[Index - 1]->LaneId : NAME_None;
            const FName NewLeft = Index + 1 < Group.Num() ? Group[Index + 1]->LaneId : NAME_None;
            if (Lane->RightNeighborLaneId == NewRight && Lane->LeftNeighborLaneId == NewLeft) continue;
            Lane->Modify();
            Lane->RightNeighborLaneId = NewRight;
            Lane->LeftNeighborLaneId = NewLeft;
            Lane->LaneCountSameDirection = Group.Num();
            Lane->MarkPackageDirty();
            ++Changed;
        }
    }
    StatusText->SetText(FText::Format(LOCTEXT("NeighboursAssigned",
        "Assigned or corrected neighbours on {0} lane component(s)."), FText::AsNumber(Changed)));
    return FReply::Handled();
}

FReply STMOPLaneRepairEditor::SnapConnectorEnds()
{
    TArray<UTMOPTrafficLaneComponent*> Lanes;
    CollectLanes(Lanes);
    TMap<FName, UTMOPTrafficLaneComponent*> ById;
    for (UTMOPTrafficLaneComponent* Lane : Lanes) ById.Add(Lane->LaneId, Lane);
    const FScopedTransaction Transaction(
        LOCTEXT("SnapTransaction", "Snap TMOP Connector Ends"));
    int32 Snapped = 0;
    for (UTMOPTrafficLaneComponent* Source : Lanes)
    {
        if (!IsRoadLane(Source) || Source->GetNumberOfSplinePoints() < 2) continue;
        for (const FTMOPLaneConnection& First : Source->NextLanes)
        {
            UTMOPTrafficLaneComponent* const* ConnectorPtr = ById.Find(First.TargetLaneId);
            if (ConnectorPtr == nullptr || IsRoadLane(*ConnectorPtr)) continue;
            UTMOPTrafficLaneComponent* Connector = *ConnectorPtr;
            if (Connector->GetNumberOfSplinePoints() < 2) continue;
            const FVector SourceEnd = Source->GetLocationAtSplinePoint(
                Source->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
            const FVector ConnectorStart = Connector->GetLocationAtSplinePoint(
                0, ESplineCoordinateSpace::World);
            const float StartGap = FVector::Dist(SourceEnd, ConnectorStart);
            if (StartGap > 1.0f && StartGap <= TMOPLaneRepair::MaximumSnapGapCm)
            {
                Connector->Modify();
                Connector->SetLocationAtSplinePoint(0, SourceEnd,
                    ESplineCoordinateSpace::World, false);
                ++Snapped;
            }
            for (const FTMOPLaneConnection& Second : Connector->NextLanes)
            {
                UTMOPTrafficLaneComponent* const* TargetPtr = ById.Find(Second.TargetLaneId);
                if (TargetPtr == nullptr || !IsRoadLane(*TargetPtr)) continue;
                const int32 Last = Connector->GetNumberOfSplinePoints() - 1;
                const FVector ConnectorEnd = Connector->GetLocationAtSplinePoint(
                    Last, ESplineCoordinateSpace::World);
                const FVector TargetStart = (*TargetPtr)->GetLocationAtSplinePoint(
                    0, ESplineCoordinateSpace::World);
                const float EndGap = FVector::Dist(ConnectorEnd, TargetStart);
                if (EndGap > 1.0f && EndGap <= TMOPLaneRepair::MaximumSnapGapCm)
                {
                    Connector->Modify();
                    Connector->SetLocationAtSplinePoint(Last, TargetStart,
                        ESplineCoordinateSpace::World, false);
                    ++Snapped;
                }
            }
            Connector->UpdateSpline();
            Connector->MarkPackageDirty();
        }
    }
    if (GEditor != nullptr) GEditor->RedrawLevelEditingViewports(true);
    StatusText->SetText(FText::Format(LOCTEXT("Snapped",
        "Snapped {0} connector endpoint(s). Gaps larger than 5 m were left for manual review."),
        FText::AsNumber(Snapped)));
    return FReply::Handled();
}

void STMOPLaneRepairEditor::RefreshSummary()
{
    int32 Safe = 0, Restricted = 0, Review = 0, Checked = 0;
    for (const FSuggestionItem& Item : Suggestions)
    {
        if (Item->Kind == ESuggestionKind::Safe) ++Safe;
        else if (Item->Kind == ESuggestionKind::Restricted) ++Restricted;
        else ++Review;
        if (Item->bSelected) ++Checked;
    }
    if (SummaryText.IsValid()) SummaryText->SetText(FText::FromString(FString::Printf(
        TEXT("%d road lanes | %d connectors | %d isolated | %d connector gaps > 1 m | %d reversed R-pairs\nSuggestions: %d safe, %d restricted, %d review | %d checked"),
        RoadLaneCount, ConnectorLaneCount, IsolatedLaneCount,
        BrokenConnectorCount, ReversedPairCount, Safe, Restricted, Review, Checked)));
    if (DiagnosticsText.IsValid())
    {
        constexpr int32 MaximumVisibleDiagnostics = 80;
        TArray<FString> Visible = Diagnostics;
        if (Visible.Num() > MaximumVisibleDiagnostics)
        {
            const int32 Hidden = Visible.Num() - MaximumVisibleDiagnostics;
            Visible.SetNum(MaximumVisibleDiagnostics);
            Visible.Add(FString::Printf(TEXT("... plus %d more; see Output Log."), Hidden));
        }
        DiagnosticsText->SetText(Visible.IsEmpty()
            ? LOCTEXT("NoDiagnostics", "No geometry or isolation diagnostics yet. Press Scan Network.")
            : FText::FromString(FString::Join(Visible, TEXT("\n"))));
    }
}

#undef LOCTEXT_NAMESPACE
