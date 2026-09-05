#include "STMOPObservationEditor.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "IStructureDetailsView.h"
#include "InputCoreTypes.h"
#include "Observations/TMOPObservationSignalementLibrary.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "UObject/StructOnScope.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "STMOPObservationEditor"

namespace
{
FString FormatObservationTime(const FTMOPObservationDefinition& Observation)
{
    if (Observation.TimingMode == ETMOPObservationTimingMode::RelativeToSharedEvent)
        return FString::Printf(TEXT("@ %s %+d s"),
            *Observation.ReferenceSharedEventId.ToString(),
            Observation.ReferenceOffsetSeconds);
    const int32 Seconds = Observation.CanonicalTime.ToSecondsFromMidnight();
    return FString::Printf(TEXT("%02d:%02d:%02d"), Seconds / 3600,
        (Seconds / 60) % 60, Seconds % 60);
}

class FTMOPObservationDragDropOp final : public FDecoratedDragDropOp
{
public:
    DRAG_DROP_OPERATOR_TYPE(FTMOPObservationDragDropOp, FDecoratedDragDropOp)
    FName ObservationId = NAME_None;
    static TSharedRef<FTMOPObservationDragDropOp> New(const FName Id)
    {
        TSharedRef<FTMOPObservationDragDropOp> Operation =
            MakeShared<FTMOPObservationDragDropOp>();
        Operation->ObservationId = Id;
        Operation->DefaultHoverText = FText::Format(
            LOCTEXT("DragObservation", "Add observation {0} to link"),
            FText::FromName(Id));
        Operation->Construct();
        return Operation;
    }
};

DECLARE_DELEGATE_RetVal_TwoParams(FReply, FTMOPOnObservationDrop,
    const FGeometry&, const FDragDropEvent&);

/** SBorder has no OnDrop Slate argument, so use a real drop-aware widget. */
class STMOPObservationDropTarget final : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPObservationDropTarget) {}
        SLATE_DEFAULT_SLOT(FArguments, Content)
        SLATE_EVENT(FTMOPOnObservationDrop, OnObservationDrop)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs)
    {
        DropHandler = InArgs._OnObservationDrop;
        ChildSlot
        [
            InArgs._Content.Widget
        ];
    }

    virtual FReply OnDrop(const FGeometry& Geometry,
        const FDragDropEvent& Event) override
    {
        return DropHandler.IsBound()
            ? DropHandler.Execute(Geometry, Event)
            : FReply::Unhandled();
    }

private:
    FTMOPOnObservationDrop DropHandler;
};
}

class STMOPObservationMap final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPObservationMap) {}
    SLATE_END_ARGS()
    void Construct(const FArguments&) {}

    struct FPoint
    {
        FName ObservationId;
        FVector2D Position;
        bool bLinked = false;
        bool bSelected = false;
        int32 Order = INDEX_NONE;
        ETMOPObservedEntityType EntityType = ETMOPObservedEntityType::Unknown;
    };

    void SetPoints(TArray<FPoint>&& InPoints)
    {
        Points = MoveTemp(InPoints);
        Invalidate(EInvalidateWidgetReason::Paint);
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return FVector2D(640.0f, 430.0f);
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& Geometry,
        const FSlateRect& CullingRect, FSlateWindowElementList& Elements,
        int32 Layer, const FWidgetStyle& Style, bool bParentEnabled) const override
    {
        FSlateDrawElement::MakeBox(Elements, Layer, Geometry.ToPaintGeometry(),
            FAppStyle::GetBrush("Brushes.Recessed"), ESlateDrawEffect::None,
            FLinearColor(0.015f, 0.02f, 0.028f, 1.0f));
        FBox2D Bounds(ForceInit);
        for (const FPoint& Point : Points) Bounds += Point.Position;
        if (!Bounds.bIsValid)
        {
            FSlateDrawElement::MakeText(Elements, Layer + 1,
                Geometry.ToPaintGeometry(FVector2D(14.0f), Geometry.GetLocalSize()),
                LOCTEXT("NoMapPoints", "No observation anchors found in the open level"),
                FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None,
                FLinearColor(0.6f, 0.6f, 0.6f));
            return Layer + 1;
        }
        const FVector2D Extent = Bounds.GetSize();
        const FVector2D Size = Geometry.GetLocalSize();
        const float Padding = 24.0f;
        const float Scale = FMath::Min(
            (Size.X - Padding * 2.0f) / FMath::Max(Extent.X, 1.0f),
            (Size.Y - Padding * 2.0f) / FMath::Max(Extent.Y, 1.0f));
        auto Project = [&](const FVector2D& P)
        {
            return FVector2D(Padding + (P.X - Bounds.Min.X) * Scale,
                Size.Y - Padding - (P.Y - Bounds.Min.Y) * Scale);
        };
        TArray<FPoint> Ordered = Points;
        Ordered.RemoveAll([](const FPoint& P) { return P.Order == INDEX_NONE; });
        Ordered.Sort([](const FPoint& A, const FPoint& B) { return A.Order < B.Order; });
        if (Ordered.Num() >= 2)
        {
            TArray<FVector2D> Track;
            for (const FPoint& P : Ordered) Track.Add(Project(P.Position));
            FSlateDrawElement::MakeLines(Elements, Layer + 1,
                Geometry.ToPaintGeometry(), Track, ESlateDrawEffect::None,
                FLinearColor(0.05f, 0.7f, 1.0f, 0.9f), true, 3.0f);
        }
        for (const FPoint& Point : Points)
        {
            const FVector2D P = Project(Point.Position);
            const float Radius = Point.bSelected ? 7.0f : 4.0f;
            const FVector2D MarkerExtent =
                Point.EntityType == ETMOPObservedEntityType::Vehicle
                ? FVector2D(Radius * 1.65f, Radius)
                : FVector2D(Radius, Radius);
            const FLinearColor Color = Point.bSelected
                ? FLinearColor(0.05f, 0.8f, 1.0f)
                : Point.bLinked ? FLinearColor(0.2f, 0.85f, 0.35f)
                : FLinearColor(1.0f, 0.65f, 0.05f);
            FSlateDrawElement::MakeBox(Elements, Layer + 2,
                Geometry.ToPaintGeometry(P - MarkerExtent,
                    MarkerExtent * 2.0f), FAppStyle::GetBrush("WhiteBrush"),
                ESlateDrawEffect::None, Color);
        }
        FSlateDrawElement::MakeText(Elements, Layer + 3,
            Geometry.ToPaintGeometry(FVector2D(10.0f), Size),
            FText::Format(LOCTEXT("MapCaption",
                "{0} points · wide marker vehicle · orange unlinked · green linked · blue selected track"),
                FText::AsNumber(Points.Num())),
            FAppStyle::GetFontStyle("SmallFont"), ESlateDrawEffect::None,
            FLinearColor(0.75f, 0.78f, 0.82f));
        return Layer + 3;
    }

private:
    TArray<FPoint> Points;
};

void STMOPObservationEditor::Construct(const FArguments& Args)
{
    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = true;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.bUpdatesFromSelection = false;
    DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    FStructureDetailsViewArgs StructureArgs;
    StructureArgs.bShowObjects = false;
    StructureArgs.bShowAssets = false;
    StructureArgs.bShowClasses = false;
    StructureArgs.bShowInterfaces = false;
    FPropertyEditorModule& PropertyEditor =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    LinkDetails = PropertyEditor.CreateStructureDetailView(
        DetailsArgs, StructureArgs, nullptr);

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1.0f)
            [ SNew(STextBlock).Text(LOCTEXT("Title", "TMOP OBSERVATION EDITOR"))
                .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
            [ SNew(SButton).Text(LOCTEXT("Reload", "Reload"))
                .OnClicked(this, &STMOPObservationEditor::Reload) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
            [ SNew(SButton).Text(LOCTEXT("NewLink", "+ New link"))
                .OnClicked(this, &STMOPObservationEditor::NewLink) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
            [ SNew(SButton).Text(LOCTEXT("SaveLink", "Save link"))
                .OnClicked(this, &STMOPObservationEditor::SaveLink) ]
        ]
        + SVerticalBox::Slot().FillHeight(1.0f).Padding(8.0f, 0.0f)
        [
            SNew(SSplitter)
            + SSplitter::Slot().Value(0.25f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 5)
                [ SNew(SSearchBox).HintText(LOCTEXT("Search", "Search observations"))
                    .OnTextChanged(this, &STMOPObservationEditor::OnSearchChanged) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 5)
                [
                    SNew(SCheckBox)
                    .IsChecked(ECheckBoxState::Unchecked)
                    .OnCheckStateChanged(this,
                        &STMOPObservationEditor::OnMissingSignalementChanged)
                    [ SNew(STextBlock).Text(LOCTEXT("OnlyMissingSignalement",
                        "Only person observations needing signalement review")) ]
                ]
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(LOCTEXT("Observations", "ALL OBSERVATIONS")) ]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [ SAssignNew(ObservationList, SListView<FObservationItem>)
                    .ListItemsSource(&ObservationItems)
                    .OnGenerateRow(this, &STMOPObservationEditor::GenerateObservationRow) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 5)
                [ SNew(SButton).Text(LOCTEXT("AddSelected", "Add selected to link →"))
                    .OnClicked(this, &STMOPObservationEditor::AddSelectedObservation) ]
            ]
            + SSplitter::Slot().Value(0.42f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(LOCTEXT("Map", "OBSERVATION MAP")) ]
                + SVerticalBox::Slot().FillHeight(0.62f)
                [ SAssignNew(Map, STMOPObservationMap) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 2)
                [ SNew(STextBlock).Text(LOCTEXT("Members", "OBSERVATIONS IN SELECTED LINK (PLAYBACK ORDER)")) ]
                + SVerticalBox::Slot().FillHeight(0.38f)
                [
                    SNew(STMOPObservationDropTarget)
                    .OnObservationDrop(this,
                        &STMOPObservationEditor::HandleMemberDrop)
                    [
                        SNew(SBorder).Padding(3.0f)
                        [ SAssignNew(MemberList, SListView<FMemberItem>)
                            .ListItemsSource(&MemberItems)
                            .OnGenerateRow(this,
                                &STMOPObservationEditor::GenerateMemberRow) ]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text(LOCTEXT("Sort", "Sort by time"))
                        .OnClicked(this, &STMOPObservationEditor::SortMembersByTime) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                    [ SNew(SButton).Text(FText::FromString(TEXT("↑")))
                        .OnClicked(this, &STMOPObservationEditor::MoveSelectedMember, -1) ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text(FText::FromString(TEXT("↓")))
                        .OnClicked(this, &STMOPObservationEditor::MoveSelectedMember, 1) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
                    [ SNew(SButton).Text(LOCTEXT("Remove", "Remove"))
                        .OnClicked(this, &STMOPObservationEditor::RemoveSelectedMember) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 4)
                [ SNew(SButton)
                    .Text(LOCTEXT("BuildSegments", "Build / update movement segments"))
                    .ToolTipText(LOCTEXT("BuildSegmentsTip",
                        "Creates one editable Track Segment between each consecutive observation. Existing movement, vehicle, seat and lane settings are preserved."))
                    .OnClicked(this, &STMOPObservationEditor::BuildTrackSegments) ]
            ]
            + SSplitter::Slot().Value(0.33f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(LOCTEXT("Links", "OBSERVATION LINKS")) ]
                + SVerticalBox::Slot().FillHeight(0.34f)
                [ SAssignNew(LinkList, SListView<FLinkItem>)
                    .ListItemsSource(&LinkItems)
                    .OnGenerateRow(this, &STMOPObservationEditor::GenerateLinkRow)
                    .OnSelectionChanged(this, &STMOPObservationEditor::OnLinkSelected) ]
                + SVerticalBox::Slot().FillHeight(0.61f).Padding(0, 6)
                [ LinkDetails->GetWidget().ToSharedRef() ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 5)
                [ SNew(STextBlock)
                    .AutoWrapText(true)
                    .Text(LOCTEXT("MovementHelp",
                        "Track Segments can walk, run, board, ride, exit, or drive a lane route. A vehicle route needs a driver and lanes/destination; boarding refuses excessive distance."))
                    .ColorAndOpacity(FLinearColor(0.65f, 0.72f, 0.8f)) ]
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(SButton).Text(LOCTEXT("Delete", "Delete selected link"))
                    .OnClicked(this, &STMOPObservationEditor::DeleteLink) ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8.0f)
        [ SAssignNew(StatusText, STextBlock)
            .Text(LOCTEXT("Ready", "Drag an observation into the middle list to build a track.")) ]
    ];
    LoadTables();
    RefreshAll();
}

void STMOPObservationEditor::LoadTables()
{
    ObservationTable = LoadObject<UDataTable>(nullptr, ObservationTablePath);
    LinkTable = LoadObject<UDataTable>(nullptr, LinkTablePath);
    if (ObservationTable.IsValid() && LinkTable.IsValid()) return;
    FAssetRegistryModule& Registry = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> Assets;
    Registry.Get().GetAssetsByClass(UDataTable::StaticClass()->GetClassPathName(), Assets);
    for (const FAssetData& Asset : Assets)
    {
        UDataTable* Table = Cast<UDataTable>(Asset.GetAsset());
        if (!IsValid(Table)) continue;
        if (!ObservationTable.IsValid() &&
            Table->GetRowStruct() == FTMOPObservationDefinition::StaticStruct())
            ObservationTable = Table;
        if (!LinkTable.IsValid() &&
            Table->GetRowStruct() == FTMOPObservationLinkDefinition::StaticStruct())
            LinkTable = Table;
    }
}

void STMOPObservationEditor::RefreshAll()
{
    ObservationsById.Reset();
    if (UDataTable* Table = ObservationTable.Get())
    {
        static const FString Context(TEXT("TMOPObservationEditor"));
        TArray<FTMOPObservationDefinition*> Rows;
        Table->GetAllRows(Context, Rows);
        for (const FTMOPObservationDefinition* Row : Rows)
            if (Row && !Row->ObservationId.IsNone())
                ObservationsById.Add(Row->ObservationId, *Row);
    }
    RefreshLinks();
    RefreshObservations();
    RefreshMembers();
    RebuildMap();
    if (!ObservationTable.IsValid() || !LinkTable.IsValid())
        SetStatus(LOCTEXT("MissingTables", "Create/import both observation DataTables, then press Reload."), FLinearColor::Red);
}

void STMOPObservationEditor::RefreshObservations()
{
    ObservationItems.Reset();
    for (const TPair<FName, FTMOPObservationDefinition>& Pair : ObservationsById)
    {
        const bool bNeedsSignalementReview =
            Pair.Value.ObservedEntityType == ETMOPObservedEntityType::Person &&
            !UTMOPObservationSignalementLibrary::HasUsableSignalement(Pair.Value) &&
            !Pair.Value.bNoFurtherSignalementInSource;
        if (bShowOnlyMissingSignalement && !bNeedsSignalementReview)
            continue;
        const FString Haystack = Pair.Key.ToString() + TEXT(" ") +
            Pair.Value.DisplayName.ToString() + TEXT(" ") +
            Pair.Value.ObservedEntityId.ToString();
        if (Search.IsEmpty() || Haystack.Contains(Search, ESearchCase::IgnoreCase))
            ObservationItems.Add(MakeShared<FName>(Pair.Key));
    }
    ObservationItems.Sort([this](const FObservationItem& A, const FObservationItem& B)
    {
        const int32 TA = ResolveObservationSecond(*A);
        const int32 TB = ResolveObservationSecond(*B);
        return TA == TB ? A->LexicalLess(*B) : TA < TB;
    });
    if (ObservationList) ObservationList->RequestListRefresh();
}

void STMOPObservationEditor::RefreshLinks()
{
    LinkItems.Reset();
    if (UDataTable* Table = LinkTable.Get())
        for (const FName RowName : Table->GetRowNames())
            LinkItems.Add(MakeShared<FName>(RowName));
    LinkItems.Sort([](const FLinkItem& A, const FLinkItem& B) { return A->LexicalLess(*B); });
    if (LinkList) LinkList->RequestListRefresh();
}

void STMOPObservationEditor::RefreshMembers()
{
    MemberItems.Reset();
    for (const FName Id : WorkingLink.ObservationIds)
        MemberItems.Add(MakeShared<FName>(Id));
    if (MemberList) MemberList->RequestListRefresh();
}

void STMOPObservationEditor::RebuildMap()
{
    if (!Map || !GEditor) return;
    TMap<FName, FVector2D> AnchorPositions;
    if (UWorld* World = GEditor->GetEditorWorldContext().World())
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        {
            const FVector Location = It->GetActorLocation();
            AnchorPositions.Add(It->GetAnchorId(), FVector2D(Location.X, Location.Y));
        }
    TArray<STMOPObservationMap::FPoint> Points;
    for (const TPair<FName, FTMOPObservationDefinition>& Pair : ObservationsById)
        if (const FVector2D* Position = AnchorPositions.Find(Pair.Value.ObservationAnchorId))
        {
            STMOPObservationMap::FPoint Point;
            Point.ObservationId = Pair.Key;
            Point.Position = *Position;
            Point.bLinked = IsObservationLinked(Pair.Key);
            Point.Order = WorkingLink.ObservationIds.IndexOfByKey(Pair.Key);
            Point.bSelected = Point.Order != INDEX_NONE;
            Point.EntityType = Pair.Value.ObservedEntityType;
            Points.Add(Point);
        }
    Map->SetPoints(MoveTemp(Points));
}

TSharedRef<ITableRow> STMOPObservationEditor::GenerateObservationRow(
    FObservationItem Item, const TSharedRef<STableViewBase>& Owner)
{
    const FTMOPObservationDefinition* Observation = Item ? ObservationsById.Find(*Item) : nullptr;
    FString SignalementStatus;
    if (Observation && Observation->ObservedEntityType ==
        ETMOPObservedEntityType::Person)
    {
        if (UTMOPObservationSignalementLibrary::HasUsableSignalement(*Observation))
            SignalementStatus = FString::Printf(TEXT("  ·  SIG %d witness record(s)"),
                Observation->WitnessSignalements.Num());
        else if (Observation->bNoFurtherSignalementInSource)
            SignalementStatus = TEXT("  ·  SIG reviewed: none in source");
        else
            SignalementStatus = Observation->bSignalementSourceReviewed
                ? TEXT("  ·  SIG reviewed: incomplete")
                : TEXT("  ·  SIG NEEDS REVIEW");
    }
    const FString Label = Observation
        ? FString::Printf(TEXT("%s  %s\n%s → %s%s"),
            *FormatObservationTime(*Observation), *Observation->DisplayName.ToString(),
            *Observation->ObservedEntityId.ToString(), *Observation->ObservationAnchorId.ToString(),
            *SignalementStatus)
        : TEXT("Missing observation");
    return SNew(STableRow<FObservationItem>, Owner)
        .OnDragDetected(this, &STMOPObservationEditor::HandleObservationDragDetected, Item)
        [ SNew(STextBlock).Text(FText::FromString(Label))
            .ColorAndOpacity(IsObservationLinked(Item ? *Item : NAME_None)
                ? FLinearColor(0.35f, 0.9f, 0.45f) : FLinearColor::White) ];
}

TSharedRef<ITableRow> STMOPObservationEditor::GenerateLinkRow(
    FLinkItem Item, const TSharedRef<STableViewBase>& Owner)
{
    const FTMOPObservationLinkDefinition* Row = LinkTable.IsValid() && Item
        ? LinkTable->FindRow<FTMOPObservationLinkDefinition>(*Item, TEXT("ObservationLinkRow"), false)
        : nullptr;
    const int32 Count = Row ? (Row->ObservationIds.IsEmpty()
        ? (!Row->FromObservationId.IsNone() && !Row->ToObservationId.IsNone() ? 2 : 0)
        : Row->ObservationIds.Num()) : 0;
    const FString Id = Item ? Item->ToString() : TEXT("None");
    return SNew(STableRow<FLinkItem>, Owner)
        [ SNew(STextBlock).Text(FText::FromString(FString::Printf(
            TEXT("%s  ·  %d observations"), *Id, Count))) ];
}

TSharedRef<ITableRow> STMOPObservationEditor::GenerateMemberRow(
    FMemberItem Item, const TSharedRef<STableViewBase>& Owner)
{
    const FTMOPObservationDefinition* Observation = Item ? ObservationsById.Find(*Item) : nullptr;
    const FString Id = Item ? Item->ToString() : TEXT("None");
    FString ComparisonText;
    const int32 MemberIndex = Item
        ? WorkingLink.ObservationIds.IndexOfByKey(*Item) : INDEX_NONE;
    if (Observation && MemberIndex > 0)
    {
        if (const FTMOPObservationDefinition* Previous =
            ObservationsById.Find(WorkingLink.ObservationIds[MemberIndex - 1]))
        {
            const FTMOPSignalementComparison Comparison =
                UTMOPObservationSignalementLibrary::CompareSignalements(
                    *Previous, *Observation);
            ComparisonText = Comparison.bHasComparableEvidence
                ? FString::Printf(TEXT("\nSignalement match with previous: %.0f%%"),
                    Comparison.CompatibilityScore * 100.0f)
                : TEXT("\nSignalement match with previous: no comparable evidence");
        }
    }
    const FString Label = Observation
        ? FString::Printf(TEXT("%s  %s\n%s%s"), *FormatObservationTime(*Observation),
            *Id, *Observation->ObservationAnchorId.ToString(), *ComparisonText)
        : FString::Printf(TEXT("MISSING  %s"), *Id);
    return SNew(STableRow<FMemberItem>, Owner)
        [ SNew(STextBlock).Text(FText::FromString(Label)) ];
}

void STMOPObservationEditor::OnLinkSelected(FLinkItem Item, ESelectInfo::Type)
{
    if (Item) SelectLink(*Item);
}

void STMOPObservationEditor::SelectLink(const FName RowName)
{
    if (!LinkTable.IsValid()) return;
    const FTMOPObservationLinkDefinition* Row =
        LinkTable->FindRow<FTMOPObservationLinkDefinition>(RowName,
            TEXT("ObservationEditorSelect"), false);
    if (!Row) return;
    SelectedLinkRow = RowName;
    WorkingLink = *Row;
    if (WorkingLink.ObservationIds.IsEmpty())
    {
        if (!WorkingLink.FromObservationId.IsNone())
            WorkingLink.ObservationIds.Add(WorkingLink.FromObservationId);
        if (!WorkingLink.ToObservationId.IsNone())
            WorkingLink.ObservationIds.AddUnique(WorkingLink.ToObservationId);
    }
    RefreshLinkDetails();
    RefreshMembers();
    RebuildMap();
}

void STMOPObservationEditor::OnSearchChanged(const FText& Text)
{
    Search = Text.ToString();
    RefreshObservations();
}

void STMOPObservationEditor::OnMissingSignalementChanged(
    const ECheckBoxState NewState)
{
    bShowOnlyMissingSignalement = NewState == ECheckBoxState::Checked;
    RefreshObservations();
}

FReply STMOPObservationEditor::HandleObservationDragDetected(
    const FGeometry&, const FPointerEvent& Event, FObservationItem Item)
{
    if (Item && Event.IsMouseButtonDown(EKeys::LeftMouseButton))
        return FReply::Handled().BeginDragDrop(FTMOPObservationDragDropOp::New(*Item));
    return FReply::Unhandled();
}

FReply STMOPObservationEditor::HandleMemberDrop(const FGeometry&,
    const FDragDropEvent& Event)
{
    if (TSharedPtr<FTMOPObservationDragDropOp> Operation =
        Event.GetOperationAs<FTMOPObservationDragDropOp>())
    {
        AddObservation(Operation->ObservationId);
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

void STMOPObservationEditor::AddObservation(const FName ObservationId)
{
    if (!ObservationsById.Contains(ObservationId)) return;
    if (SelectedLinkRow.IsNone()) NewLink();
    CommitLinkDetails();
    WorkingLink.ObservationIds.AddUnique(ObservationId);
    RefreshLinkDetails();
    RefreshMembers();
    RebuildMap();
}

FReply STMOPObservationEditor::NewLink()
{
    SelectedLinkRow = NAME_None;
    WorkingLink = FTMOPObservationLinkDefinition();
    WorkingLink.LinkId = FName(*FString::Printf(TEXT("LINK_NEW_%lld"),
        FDateTime::UtcNow().GetTicks()));
    WorkingLink.SimulationMode = ETMOPObservationTrackSimulationMode::ValidateOnly;
    RefreshLinkDetails();
    RefreshMembers();
    RebuildMap();
    SetStatus(LOCTEXT("NewReady", "New link ready. Drag at least two observations into it."), FLinearColor::White);
    return FReply::Handled();
}

FReply STMOPObservationEditor::SaveLink()
{
    CommitLinkDetails();
    UDataTable* Table = LinkTable.Get();
    if (!Table || WorkingLink.LinkId.IsNone() || WorkingLink.ObservationIds.Num() < 2)
    {
        SetStatus(LOCTEXT("CannotSave", "Not saved: choose a Link ID and add at least two observations."), FLinearColor::Red);
        return FReply::Handled();
    }
    if (WorkingLink.SimulationMode ==
            ETMOPObservationTrackSimulationMode::InterpolateExistingActor &&
        WorkingLink.LinkedEntityId.IsNone())
    {
        SetStatus(LOCTEXT("MissingPlaybackActor",
            "Not saved: an interpolated link needs one Linked Entity ID."),
            FLinearColor::Red);
        return FReply::Handled();
    }
    for (const FName Id : WorkingLink.ObservationIds)
        if (!ObservationsById.Contains(Id))
        {
            SetStatus(FText::Format(LOCTEXT("MissingMember", "Not saved: observation {0} does not exist."), FText::FromName(Id)), FLinearColor::Red);
            return FReply::Handled();
        }
    if (WorkingLink.bRequireSignalementCompatibility)
    {
        for (int32 MemberIndex = 0;
            MemberIndex + 1 < WorkingLink.ObservationIds.Num(); ++MemberIndex)
        {
            const FName FirstId = WorkingLink.ObservationIds[MemberIndex];
            const FName SecondId = WorkingLink.ObservationIds[MemberIndex + 1];
            const FTMOPObservationDefinition* FirstObservation =
                ObservationsById.Find(FirstId);
            const FTMOPObservationDefinition* SecondObservation =
                ObservationsById.Find(SecondId);
            if (FirstObservation == nullptr || SecondObservation == nullptr)
                continue;
            const FTMOPSignalementComparison Comparison =
                UTMOPObservationSignalementLibrary::CompareSignalements(
                    *FirstObservation, *SecondObservation);
            if (!Comparison.bHasComparableEvidence)
            {
                SetStatus(FText::Format(LOCTEXT("MissingComparableSignalement",
                    "Not saved: {0} and {1} have no comparable structured signalement evidence."),
                    FText::FromName(FirstId), FText::FromName(SecondId)),
                    FLinearColor::Red);
                return FReply::Handled();
            }
            if (Comparison.CompatibilityScore <
                WorkingLink.MinimumSignalementCompatibility)
            {
                SetStatus(FText::Format(LOCTEXT("SignalementMismatch",
                    "Not saved: signalement compatibility between {0} and {1} is {2}%, below this link's minimum."),
                    FText::FromName(FirstId), FText::FromName(SecondId),
                    FText::AsNumber(FMath::RoundToInt(
                        Comparison.CompatibilityScore * 100.0f))),
                    FLinearColor::Red);
                return FReply::Handled();
            }
        }
    }
    for (const FTMOPObservationTrackSegment& Segment : WorkingLink.TrackSegments)
    {
        if (!WorkingLink.ObservationIds.Contains(Segment.FromObservationId) ||
            !WorkingLink.ObservationIds.Contains(Segment.ToObservationId) ||
            Segment.FromObservationId == Segment.ToObservationId)
        {
            SetStatus(LOCTEXT("InvalidSegmentMembers",
                "Not saved: a Track Segment references observations outside this link. Rebuild movement segments."),
                FLinearColor::Red);
            return FReply::Handled();
        }
        const bool bPersonVehicleTransition =
            Segment.MovementMode == ETMOPObservationSegmentMovementMode::WalkToVehicleAndBoard ||
            Segment.MovementMode == ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard ||
            Segment.MovementMode == ETMOPObservationSegmentMovementMode::RideInVehicle ||
            Segment.MovementMode == ETMOPObservationSegmentMovementMode::ExitVehicleThenWalk ||
            Segment.MovementMode == ETMOPObservationSegmentMovementMode::ExitVehicleThenRun;
        ETMOPObservedEntityType EffectiveLinkType = WorkingLink.LinkedEntityType;
        if (EffectiveLinkType == ETMOPObservedEntityType::Unknown &&
            !WorkingLink.ObservationIds.IsEmpty())
            if (const FTMOPObservationDefinition* FirstObservation =
                ObservationsById.Find(WorkingLink.ObservationIds[0]))
                EffectiveLinkType = FirstObservation->ObservedEntityType;
        if (bPersonVehicleTransition &&
            EffectiveLinkType != ETMOPObservedEntityType::Person)
        {
            SetStatus(LOCTEXT("WrongSegmentEntityType",
                "Not saved: board, ride, and exit modes require a Person link."),
                FLinearColor::Red);
            return FReply::Handled();
        }
        if (bPersonVehicleTransition && Segment.VehicleEntityId.IsNone())
        {
            SetStatus(LOCTEXT("MissingSegmentVehicle",
                "Not saved: every board, ride, or exit segment needs Vehicle Entity ID."),
                FLinearColor::Red);
            return FReply::Handled();
        }
        if (Segment.MovementMode ==
            ETMOPObservationSegmentMovementMode::VehicleLaneRoute)
        {
            if (Segment.DriverEntityId.IsNone())
            {
                SetStatus(LOCTEXT("MissingSegmentDriver",
                    "Not saved: every Vehicle Lane Route needs Driver Entity ID."),
                    FLinearColor::Red);
                return FReply::Handled();
            }
            if (Segment.VehicleRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute &&
                Segment.OrderedLaneIds.IsEmpty())
            {
                SetStatus(LOCTEXT("MissingSegmentLanes",
                    "Not saved: a Manual Lane Route needs Ordered Lane IDs."),
                    FLinearColor::Red);
                return FReply::Handled();
            }
            if (Segment.VehicleRouteMode != ETMOPVehicleRouteMode::ManualLaneRoute &&
                Segment.VehicleDestinationAnchorId.IsNone())
            {
                SetStatus(LOCTEXT("MissingSegmentDestination",
                    "Not saved: an automatic vehicle route needs Vehicle Destination Anchor ID."),
                    FLinearColor::Red);
                return FReply::Handled();
            }
        }
    }
    if (WorkingLink.SimulationMode ==
        ETMOPObservationTrackSimulationMode::InterpolateExistingActor)
    {
        static const FString OwnerContext(TEXT("ObservationEditorOwnerCheck"));
        TArray<FTMOPObservationLinkDefinition*> ExistingLinks;
        Table->GetAllRows(OwnerContext, ExistingLinks);
        for (const FTMOPObservationLinkDefinition* Existing : ExistingLinks)
            if (Existing != nullptr && Existing->LinkId != SelectedLinkRow &&
                Existing->SimulationMode ==
                    ETMOPObservationTrackSimulationMode::InterpolateExistingActor &&
                Existing->LinkedEntityId == WorkingLink.LinkedEntityId)
            {
                SetStatus(FText::Format(LOCTEXT("DuplicateActorOwner",
                    "Not saved: {0} is already controlled by interpolated link {1}."),
                    FText::FromName(WorkingLink.LinkedEntityId),
                    FText::FromName(Existing->LinkId)), FLinearColor::Red);
                return FReply::Handled();
            }
    }
    const FScopedTransaction Transaction(LOCTEXT("SaveTransaction", "Save observation link"));
    Table->Modify();
    const FName NewRowName = WorkingLink.LinkId;
    if (SelectedLinkRow != NewRowName && Table->GetRowMap().Contains(NewRowName))
    {
        SetStatus(LOCTEXT("DuplicateLink", "Not saved: that Link ID already exists."), FLinearColor::Red);
        return FReply::Handled();
    }
    if (!SelectedLinkRow.IsNone() && SelectedLinkRow != NewRowName)
        Table->RemoveRow(SelectedLinkRow);
    WorkingLink.FromObservationId = NAME_None;
    WorkingLink.ToObservationId = NAME_None;
    Table->AddRow(NewRowName, WorkingLink);
    Table->MarkPackageDirty();
    Table->PostEditChange();
    SelectedLinkRow = NewRowName;
    RefreshAll();
    SetStatus(LOCTEXT("Saved", "Observation link saved. Save the project to write the asset to disk."), FLinearColor(0.35f, 1.0f, 0.4f));
    return FReply::Handled();
}

FReply STMOPObservationEditor::Reload()
{
    LoadTables();
    RefreshAll();
    if (!SelectedLinkRow.IsNone()) SelectLink(SelectedLinkRow);
    return FReply::Handled();
}

FReply STMOPObservationEditor::AddSelectedObservation()
{
    if (ObservationList && !ObservationList->GetSelectedItems().IsEmpty())
        AddObservation(*ObservationList->GetSelectedItems()[0]);
    return FReply::Handled();
}

FReply STMOPObservationEditor::RemoveSelectedMember()
{
    CommitLinkDetails();
    if (MemberList && !MemberList->GetSelectedItems().IsEmpty())
        WorkingLink.ObservationIds.Remove(*MemberList->GetSelectedItems()[0]);
    RefreshLinkDetails();
    RefreshMembers();
    RebuildMap();
    return FReply::Handled();
}

FReply STMOPObservationEditor::MoveSelectedMember(const int32 Direction)
{
    CommitLinkDetails();
    if (!MemberList || MemberList->GetSelectedItems().IsEmpty()) return FReply::Handled();
    const FName Id = *MemberList->GetSelectedItems()[0];
    const int32 OldIndex = WorkingLink.ObservationIds.IndexOfByKey(Id);
    const int32 NewIndex = FMath::Clamp(OldIndex + Direction, 0,
        WorkingLink.ObservationIds.Num() - 1);
    if (OldIndex != INDEX_NONE && OldIndex != NewIndex)
        WorkingLink.ObservationIds.Swap(OldIndex, NewIndex);
    RefreshLinkDetails();
    RefreshMembers();
    RebuildMap();
    return FReply::Handled();
}

FReply STMOPObservationEditor::SortMembersByTime()
{
    CommitLinkDetails();
    WorkingLink.ObservationIds.Sort([this](const FName A, const FName B)
    {
        const int32 TA = ResolveObservationSecond(A);
        const int32 TB = ResolveObservationSecond(B);
        return TA == TB ? A.LexicalLess(B) : TA < TB;
    });
    RefreshLinkDetails();
    RefreshMembers();
    RebuildMap();
    return FReply::Handled();
}

FReply STMOPObservationEditor::BuildTrackSegments()
{
    CommitLinkDetails();
    if (WorkingLink.ObservationIds.Num() < 2)
    {
        SetStatus(LOCTEXT("NeedMembersForSegments",
            "Add at least two observations before building movement segments."),
            FLinearColor::Red);
        return FReply::Handled();
    }

    TArray<FTMOPObservationTrackSegment> NewSegments;
    NewSegments.Reserve(WorkingLink.ObservationIds.Num() - 1);
    for (int32 Index = 0; Index + 1 < WorkingLink.ObservationIds.Num(); ++Index)
    {
        const FName FromId = WorkingLink.ObservationIds[Index];
        const FName ToId = WorkingLink.ObservationIds[Index + 1];
        if (const FTMOPObservationTrackSegment* Existing =
            WorkingLink.TrackSegments.FindByPredicate(
                [FromId, ToId](const FTMOPObservationTrackSegment& Candidate)
                {
                    return Candidate.FromObservationId == FromId &&
                        Candidate.ToObservationId == ToId;
                }))
            NewSegments.Add(*Existing);
        else
        {
            FTMOPObservationTrackSegment& Segment = NewSegments.AddDefaulted_GetRef();
            Segment.FromObservationId = FromId;
            Segment.ToObservationId = ToId;
        }
    }
    WorkingLink.TrackSegments = MoveTemp(NewSegments);
    RefreshLinkDetails();
    SetStatus(FText::Format(LOCTEXT("SegmentsBuilt",
        "Built {0} movement segments. Expand Track Segments on the right to set walking, running, boarding, riding, seats and vehicle lanes."),
        FText::AsNumber(WorkingLink.TrackSegments.Num())),
        FLinearColor(0.35f, 0.85f, 1.0f));
    return FReply::Handled();
}

FReply STMOPObservationEditor::DeleteLink()
{
    if (LinkTable.IsValid() && !SelectedLinkRow.IsNone())
    {
        const FScopedTransaction Transaction(LOCTEXT("DeleteTransaction", "Delete observation link"));
        LinkTable->Modify();
        LinkTable->RemoveRow(SelectedLinkRow);
        LinkTable->MarkPackageDirty();
        LinkTable->PostEditChange();
        SelectedLinkRow = NAME_None;
        WorkingLink = FTMOPObservationLinkDefinition();
        LinkDetails->SetStructureData(nullptr);
        RefreshAll();
    }
    return FReply::Handled();
}

bool STMOPObservationEditor::IsObservationLinked(const FName ObservationId) const
{
    if (!LinkTable.IsValid()) return false;
    static const FString Context(TEXT("ObservationLinkedLookup"));
    TArray<FTMOPObservationLinkDefinition*> Rows;
    LinkTable->GetAllRows(Context, Rows);
    for (const FTMOPObservationLinkDefinition* Row : Rows)
        if (Row && (Row->ObservationIds.Contains(ObservationId) ||
            Row->FromObservationId == ObservationId || Row->ToObservationId == ObservationId))
            return true;
    return false;
}

int32 STMOPObservationEditor::ResolveObservationSecond(const FName ObservationId) const
{
    const FTMOPObservationDefinition* Row = ObservationsById.Find(ObservationId);
    return Row ? Row->CanonicalTime.ToSecondsFromMidnight() : MAX_int32;
}

void STMOPObservationEditor::CommitLinkDetails()
{
    if (LinkStruct.IsValid())
        WorkingLink = *reinterpret_cast<FTMOPObservationLinkDefinition*>(
            LinkStruct->GetStructMemory());
}

void STMOPObservationEditor::RefreshLinkDetails()
{
    LinkStruct = MakeShared<FStructOnScope>(
        FTMOPObservationLinkDefinition::StaticStruct());
    LinkStruct->SetPackage(LinkTable.IsValid()
        ? LinkTable->GetOutermost() : GetTransientPackage());
    *reinterpret_cast<FTMOPObservationLinkDefinition*>(
        LinkStruct->GetStructMemory()) = WorkingLink;
    LinkDetails->SetStructureData(LinkStruct);
}

void STMOPObservationEditor::SetStatus(const FText& Text,
    const FLinearColor& Color)
{
    if (StatusText)
    {
        StatusText->SetText(Text);
        StatusText->SetColorAndOpacity(Color);
    }
}

#undef LOCTEXT_NAMESPACE
