#include "STMOPVehicleEditor.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "IStructureDetailsView.h"
#include "InputCoreTypes.h"
#include "NavigationSystem.h"
#include "People/TMOPPersonProfileTypes.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "UObject/StructOnScope.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSearchableComboBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Venues/TMOPCinemaSeatComponent.h"

#define LOCTEXT_NAMESPACE "STMOPVehicleEditor"

namespace
{
    FString VehicleActionLabel(const ETMOPHistoricalVehicleAction Action)
    {
        if (const UEnum* Enum = StaticEnum<ETMOPHistoricalVehicleAction>())
            return Enum->GetDisplayNameTextByValue(
                static_cast<int64>(Action)).ToString();
        return TEXT("Unknown");
    }

    bool IsDriving(const ETMOPHistoricalVehicleAction Action)
    {
        return Action == ETMOPHistoricalVehicleAction::BeginDriving ||
            Action == ETMOPHistoricalVehicleAction::EnterTrafficRoute;
    }

    bool IsStop(const ETMOPHistoricalVehicleAction Action)
    {
        return Action == ETMOPHistoricalVehicleAction::Stop ||
            Action == ETMOPHistoricalVehicleAction::Park ||
            Action == ETMOPHistoricalVehicleAction::ExitTrafficRoute;
    }

    bool HasPlacement(const ETMOPHistoricalVehicleAction Action)
    {
        return Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
            Action == ETMOPHistoricalVehicleAction::Spawn ||
            Action == ETMOPHistoricalVehicleAction::Stop ||
            Action == ETMOPHistoricalVehicleAction::Park;
    }

    FString FormatClockSecond(const int32 Second)
    {
        const int32 Normalized = FMath::Max(0, Second) % (24 * 3600);
        return FString::Printf(TEXT("%02d:%02d:%02d"),
            Normalized / 3600, (Normalized / 60) % 60, Normalized % 60);
    }

    FString DrivingPresetLabel(const ETMOPVehicleDrivingPreset Preset)
    {
        if (const UEnum* Enum = StaticEnum<ETMOPVehicleDrivingPreset>())
            return Enum->GetDisplayNameTextByValue(
                static_cast<int64>(Preset)).ToString();
        return TEXT("Automatic From Timeline");
    }

    float DrivingPresetSpeedKmh(const ETMOPVehicleDrivingPreset Preset)
    {
        switch (Preset)
        {
        case ETMOPVehicleDrivingPreset::Parking: return 8.0f;
        case ETMOPVehicleDrivingPreset::SlowCity: return 20.0f;
        case ETMOPVehicleDrivingPreset::NormalCity: return 35.0f;
        case ETMOPVehicleDrivingPreset::Fast: return 60.0f;
        case ETMOPVehicleDrivingPreset::Emergency: return 90.0f;
        case ETMOPVehicleDrivingPreset::Fleeing: return 110.0f;
        default: return 0.0f;
        }
    }
}

class STMOPVehicleRouteMap final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPVehicleRouteMap) {}
    SLATE_END_ARGS()
    void Construct(const FArguments&) {}

    void SetRoute(TArray<TArray<FVector2D>>&& InNetwork,
        TArray<FName>&& InNetworkLaneIds,
        TArray<TArray<FVector2D>>&& InRoute,
        const FString& InCaption,
        const TOptional<FVector2D>& InStartAnchor = TOptional<FVector2D>(),
        const TOptional<FVector2D>& InEndAnchor = TOptional<FVector2D>(),
        const TOptional<FVector2D>& InSelectedPlacement =
            TOptional<FVector2D>())
    {
        Network = MoveTemp(InNetwork);
        NetworkLaneIds = MoveTemp(InNetworkLaneIds);
        Route = MoveTemp(InRoute);
        Caption = InCaption;
        StartAnchor = InStartAnchor;
        EndAnchor = InEndAnchor;
        SelectedPlacement = InSelectedPlacement;
        Invalidate(EInvalidateWidgetReason::Paint);
    }

    void SetLaneClickedHandler(
        TFunction<void(FName, bool, bool, bool)>&& InHandler)
    {
        LaneClickedHandler = MoveTemp(InHandler);
    }

    virtual FReply OnMouseButtonDown(const FGeometry& Geometry,
        const FPointerEvent& Event) override
    {
        if (!LaneClickedHandler || Network.Num() != NetworkLaneIds.Num())
            return FReply::Unhandled();
        FBox2D Bounds(ForceInit);
        const TArray<TArray<FVector2D>>& BoundsLines =
            Route.IsEmpty() ? Network : Route;
        for (const TArray<FVector2D>& Line : BoundsLines)
            for (const FVector2D& Point : Line) Bounds += Point;
        if (!Bounds.bIsValid) return FReply::Unhandled();
        const FVector2D Size = Geometry.GetLocalSize();
        const FVector2D Extent = Bounds.GetSize();
        const float Padding = 22.0f;
        const float Scale = FMath::Min(
            (Size.X - Padding * 2.0f) / FMath::Max(Extent.X, 1.0f),
            (Size.Y - Padding * 2.0f) / FMath::Max(Extent.Y, 1.0f));
        auto Project = [&](const FVector2D& Point)
        {
            return FVector2D(Padding + (Point.X - Bounds.Min.X) * Scale,
                Padding + (Point.Y - Bounds.Min.Y) * Scale);
        };
        const FVector2D Mouse = Geometry.AbsoluteToLocal(
            Event.GetScreenSpacePosition());
        int32 BestIndex = INDEX_NONE;
        double BestDistance = 14.0 * 14.0;
        for (int32 LineIndex = 0; LineIndex < Network.Num(); ++LineIndex)
            for (int32 PointIndex = 0;
                PointIndex + 1 < Network[LineIndex].Num(); ++PointIndex)
            {
                const FVector2D Closest = FMath::ClosestPointOnSegment2D(Mouse,
                    Project(Network[LineIndex][PointIndex]),
                    Project(Network[LineIndex][PointIndex + 1]));
                const double Distance = FVector2D::DistSquared(Mouse, Closest);
                if (Distance < BestDistance)
                { BestDistance = Distance; BestIndex = LineIndex; }
            }
        if (!NetworkLaneIds.IsValidIndex(BestIndex)) return FReply::Unhandled();
        const bool bRight = Event.GetEffectingButton() == EKeys::RightMouseButton;
        LaneClickedHandler(NetworkLaneIds[BestIndex], Event.IsShiftDown(),
            Event.IsControlDown(), bRight);
        return FReply::Handled();
    }

    virtual FVector2D ComputeDesiredSize(float) const override
    {
        return FVector2D(520.0f, 330.0f);
    }

    virtual int32 OnPaint(const FPaintArgs& Args,
        const FGeometry& Geometry, const FSlateRect& CullingRect,
        FSlateWindowElementList& DrawElements, int32 Layer,
        const FWidgetStyle& Style, bool bParentEnabled) const override
    {
        const FVector2D Size = Geometry.GetLocalSize();
        FSlateDrawElement::MakeBox(DrawElements, Layer,
            Geometry.ToPaintGeometry(),
            FAppStyle::GetBrush("Brushes.Recessed"),
            ESlateDrawEffect::None,
            FLinearColor(0.018f, 0.022f, 0.028f, 1.0f));

        FBox2D Bounds(ForceInit);
        const TArray<TArray<FVector2D>>& BoundsLines =
            Route.IsEmpty() ? Network : Route;
        for (const TArray<FVector2D>& Line : BoundsLines)
            for (const FVector2D& Point : Line) Bounds += Point;
        if (StartAnchor.IsSet()) Bounds += StartAnchor.GetValue();
        if (EndAnchor.IsSet()) Bounds += EndAnchor.GetValue();
        if (SelectedPlacement.IsSet()) Bounds += SelectedPlacement.GetValue();
        if (!Bounds.bIsValid)
        {
            FSlateDrawElement::MakeText(DrawElements, Layer + 1,
                Geometry.ToPaintGeometry(FVector2D(12, 12), Size),
                LOCTEXT("MapEmpty", "Select a driving entry with lane IDs"),
                FAppStyle::GetFontStyle("NormalFont"),
                ESlateDrawEffect::None, FLinearColor(0.6f, 0.6f, 0.6f));
            return Layer + 1;
        }

        const FVector2D Extent = Bounds.GetSize();
        const float Padding = 22.0f;
        const float Scale = FMath::Min(
            (Size.X - Padding * 2.0f) / FMath::Max(Extent.X, 1.0f),
            (Size.Y - Padding * 2.0f) / FMath::Max(Extent.Y, 1.0f));
        auto Project = [&](const FVector2D& P)
        {
            return FVector2D(
                Padding + (P.X - Bounds.Min.X) * Scale,
                Padding + (P.Y - Bounds.Min.Y) * Scale);
        };
        auto Draw = [&](const TArray<TArray<FVector2D>>& Lines,
                        const FLinearColor& Color, const float Width,
                        const int32 DrawLayer)
        {
            for (const TArray<FVector2D>& Line : Lines)
            {
                TArray<FVector2D> Points;
                for (const FVector2D& P : Line) Points.Add(Project(P));
                if (Points.Num() >= 2)
                    FSlateDrawElement::MakeLines(DrawElements, DrawLayer,
                        Geometry.ToPaintGeometry(), Points,
                        ESlateDrawEffect::None, Color, true, Width);
            }
        };
        Draw(Network, FLinearColor(0.16f, 0.19f, 0.23f, 0.75f),
            1.0f, Layer + 1);
        Draw(Route, FLinearColor(0.05f, 0.55f, 1.0f, 1.0f),
            4.0f, Layer + 2);
        if (!Route.IsEmpty() && !Route[0].IsEmpty())
        {
            const FVector2D Start = Project(Route[0][0]);
            const TArray<FVector2D>& LastLine = Route.Last();
            const FVector2D End = Project(LastLine.Last());
            FSlateDrawElement::MakeBox(DrawElements, Layer + 3,
                Geometry.ToPaintGeometry(Start - FVector2D(5), FVector2D(10)),
                FAppStyle::GetBrush("WhiteBrush"), ESlateDrawEffect::None,
                FLinearColor(0.05f, 0.9f, 0.18f));
            FSlateDrawElement::MakeBox(DrawElements, Layer + 3,
                Geometry.ToPaintGeometry(End - FVector2D(5), FVector2D(10)),
                FAppStyle::GetBrush("WhiteBrush"), ESlateDrawEffect::None,
                FLinearColor(1.0f, 0.65f, 0.02f));
        }
        if (StartAnchor.IsSet())
        {
            const FVector2D Point = Project(StartAnchor.GetValue());
            FSlateDrawElement::MakeBox(DrawElements, Layer + 3,
                Geometry.ToPaintGeometry(Point - FVector2D(7), FVector2D(14)),
                FAppStyle::GetBrush("WhiteBrush"), ESlateDrawEffect::None,
                FLinearColor(0.05f, 1.0f, 0.25f));
        }
        if (EndAnchor.IsSet())
        {
            const FVector2D Point = Project(EndAnchor.GetValue());
            FSlateDrawElement::MakeBox(DrawElements, Layer + 3,
                Geometry.ToPaintGeometry(Point - FVector2D(7), FVector2D(14)),
                FAppStyle::GetBrush("WhiteBrush"), ESlateDrawEffect::None,
                FLinearColor(1.0f, 0.72f, 0.02f));
        }
        if (SelectedPlacement.IsSet())
        {
            const FVector2D Point = Project(SelectedPlacement.GetValue());
            FSlateDrawElement::MakeBox(DrawElements, Layer + 4,
                Geometry.ToPaintGeometry(Point - FVector2D(8), FVector2D(16)),
                FAppStyle::GetBrush("WhiteBrush"), ESlateDrawEffect::None,
                FLinearColor(0.0f, 0.85f, 1.0f));
        }
        FSlateDrawElement::MakeText(DrawElements, Layer + 4,
            Geometry.ToPaintGeometry(FVector2D(10, 7), Size),
            FText::FromString(Caption),
            FAppStyle::GetFontStyle("SmallFont"),
            ESlateDrawEffect::None, FLinearColor::White);
        return Layer + 4;
    }

private:
    TArray<TArray<FVector2D>> Network;
    TArray<FName> NetworkLaneIds;
    TArray<TArray<FVector2D>> Route;
    FString Caption;
    TOptional<FVector2D> StartAnchor;
    TOptional<FVector2D> EndAnchor;
    TOptional<FVector2D> SelectedPlacement;
    TFunction<void(FName, bool, bool, bool)> LaneClickedHandler;
};

void STMOPVehicleEditor::Construct(const FArguments& Args)
{
    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = true;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.bLockable = false;
    DetailsArgs.bUpdatesFromSelection = false;
    DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    FStructureDetailsViewArgs StructureArgs;
    StructureArgs.bShowObjects = false;
    StructureArgs.bShowAssets = true;
    StructureArgs.bShowClasses = true;
    StructureArgs.bShowInterfaces = false;
    FPropertyEditorModule& PropertyEditor =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>(
            TEXT("PropertyEditor"));
    VehicleDetails = PropertyEditor.CreateStructureDetailView(
        DetailsArgs, StructureArgs, nullptr);
    EntryDetails = PropertyEditor.CreateStructureDetailView(
        DetailsArgs, StructureArgs, nullptr);

    ChildSlot
    [
        SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(8)
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetTitle)
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetSubtitle)
                    .ColorAndOpacity(FSlateColor::UseSubduedForeground()) ]
            ]
            + SHorizontalBox::Slot().AutoWidth().Padding(3,0)
            [ SNew(SButton).Text(LOCTEXT("Reload", "Reload"))
                .OnClicked(this, &STMOPVehicleEditor::ReloadVehicle) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(3,0)
            [ SNew(SButton).Text(LOCTEXT("ValidateAll", "Validate All"))
                .OnClicked(this, &STMOPVehicleEditor::ValidateAll) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("Save", "Save Vehicle"))
                .ButtonStyle(FAppStyle::Get(), "PrimaryButton")
                .OnClicked(this, &STMOPVehicleEditor::SaveVehicle) ]
        ]
        + SVerticalBox::Slot().FillHeight(1)
        [
            SNew(SSplitter)
            + SSplitter::Slot().Value(0.16f)
            [
                SNew(SBorder).Padding(6)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)
                    [ SNew(SSearchBox).HintText(LOCTEXT("Search", "Search vehicles"))
                        .OnTextChanged(this, &STMOPVehicleEditor::OnSearchChanged) ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,3)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
                        [ SNew(SCheckBox)
                            .IsChecked(this, &STMOPVehicleEditor::GetVehicleFilterCheckState,
                                EVehicleListFilter::AllCars)
                            .OnCheckStateChanged(this, &STMOPVehicleEditor::HandleVehicleFilterChanged,
                                EVehicleListFilter::AllCars)
                            [ SNew(STextBlock).Text(LOCTEXT("FilterAllCars", "All cars")) ] ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ SNew(SCheckBox)
                            .IsChecked(this, &STMOPVehicleEditor::GetVehicleFilterCheckState,
                                EVehicleListFilter::SpawnedCars)
                            .OnCheckStateChanged(this, &STMOPVehicleEditor::HandleVehicleFilterChanged,
                                EVehicleListFilter::SpawnedCars)
                            [ SNew(STextBlock).Text(LOCTEXT("FilterSpawnedCars", "Only spawned cars")) ] ]
                    ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,3)
                    [ SNew(SCheckBox)
                        .IsChecked(this, &STMOPVehicleEditor::GetVehicleFilterCheckState,
                            EVehicleListFilter::SpawnedCarsWithTimelines)
                        .OnCheckStateChanged(this, &STMOPVehicleEditor::HandleVehicleFilterChanged,
                            EVehicleListFilter::SpawnedCarsWithTimelines)
                        [ SNew(STextBlock).Text(LOCTEXT("FilterSpawnedCarsWithTimelines",
                            "Only spawned cars with timelines")) ] ]
                    + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,6)
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth().Padding(0,0,8,0)
                        [ SNew(SCheckBox)
                            .IsChecked(this, &STMOPVehicleEditor::GetVehicleFilterCheckState,
                                EVehicleListFilter::MainWitnessCars)
                            .OnCheckStateChanged(this, &STMOPVehicleEditor::HandleVehicleFilterChanged,
                                EVehicleListFilter::MainWitnessCars)
                            [ SNew(STextBlock).Text(LOCTEXT("FilterMainWitnessCars", "Main witnesses cars")) ] ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ SNew(SCheckBox)
                            .IsChecked(this, &STMOPVehicleEditor::GetVehicleFilterCheckState,
                                EVehicleListFilter::PoliceCars)
                            .OnCheckStateChanged(this, &STMOPVehicleEditor::HandleVehicleFilterChanged,
                                EVehicleListFilter::PoliceCars)
                            [ SNew(STextBlock).Text(LOCTEXT("FilterPoliceCars", "Police cars")) ] ]
                    ]
                    + SVerticalBox::Slot().FillHeight(1)
                    [ SAssignNew(VehicleList, SListView<FVehicleItem>)
                        .ListItemsSource(&VehicleItems)
                        .OnGenerateRow(this, &STMOPVehicleEditor::GenerateVehicleRow)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnVehicleSelected) ]
                ]
            ]
            + SSplitter::Slot().Value(0.25f)
            [
                SNew(SBorder).Padding(6)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SHorizontalBox)
                        + SHorizontalBox::Slot().AutoWidth()
                        [ SNew(SButton).Text(LOCTEXT("Add", "+ Add"))
                            .OnClicked(this, &STMOPVehicleEditor::AddEntry) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(3,0)
                        [ SNew(SButton).Text(LOCTEXT("Duplicate", "Duplicate"))
                            .OnClicked(this, &STMOPVehicleEditor::DuplicateEntry) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ SNew(SButton).Text(LOCTEXT("Delete", "Delete"))
                            .OnClicked(this, &STMOPVehicleEditor::DeleteEntry) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(8,0,2,0)
                        [ SNew(SButton).Text(FText::FromString(TEXT("↑")))
                            .OnClicked(this, &STMOPVehicleEditor::MoveEntry, -1) ]
                        + SHorizontalBox::Slot().AutoWidth()
                        [ SNew(SButton).Text(FText::FromString(TEXT("↓")))
                            .OnClicked(this, &STMOPVehicleEditor::MoveEntry, 1) ]
                    ]
                    + SVerticalBox::Slot().FillHeight(1).Padding(0,5,0,0)
                    [ SAssignNew(TimelineList, SListView<FTimelineItem>)
                        .ListItemsSource(&TimelineItems)
                        .OnGenerateRow(this, &STMOPVehicleEditor::GenerateTimelineRow)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnTimelineSelected) ]
                ]
            ]
            + SSplitter::Slot().Value(0.31f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text(LOCTEXT("RouteEndpoints", "ROUTE START / DESTINATION"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetRouteEndpointsText)
                    .AutoWrapText(true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,3,0)
                    [ SAssignNew(StartAnchorCombo, SSearchableComboBox)
                        .OptionsSource(&AnchorItems)
                        .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateAnchorOption)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnRouteReferenceSelected,
                            ERouteReferenceField::StartAnchor)
                        [ SNew(STextBlock).Text(this,
                            &STMOPVehicleEditor::GetRouteReferenceText,
                            ERouteReferenceField::StartAnchor) ] ]
                    + SHorizontalBox::Slot().FillWidth(1).Padding(3,0,0,0)
                    [ SAssignNew(StartLaneCombo, SSearchableComboBox)
                        .OptionsSource(&LaneItems)
                        .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateLaneOption)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnRouteReferenceSelected,
                            ERouteReferenceField::StartLane)
                        [ SNew(STextBlock).Text(this,
                            &STMOPVehicleEditor::GetRouteReferenceText,
                            ERouteReferenceField::StartLane) ] ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,3,0)
                    [ SAssignNew(DestinationAnchorCombo, SSearchableComboBox)
                        .OptionsSource(&AnchorItems)
                        .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateAnchorOption)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnRouteReferenceSelected,
                            ERouteReferenceField::DestinationAnchor)
                        [ SNew(STextBlock).Text(this,
                            &STMOPVehicleEditor::GetRouteReferenceText,
                            ERouteReferenceField::DestinationAnchor) ] ]
                    + SHorizontalBox::Slot().FillWidth(1).Padding(3,0,0,0)
                    [ SAssignNew(DestinationLaneCombo, SSearchableComboBox)
                        .OptionsSource(&LaneItems)
                        .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateLaneOption)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnRouteReferenceSelected,
                            ERouteReferenceField::DestinationLane)
                        [ SNew(STextBlock).Text(this,
                            &STMOPVehicleEditor::GetRouteReferenceText,
                            ERouteReferenceField::DestinationLane) ] ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1).Padding(0,0,3,0)
                    [ SAssignNew(ViaAnchorCombo, SSearchableComboBox)
                        .OptionsSource(&AnchorItems)
                        .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateAnchorOption)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnRouteReferenceSelected,
                            ERouteReferenceField::ViaAnchor)
                        [ SNew(STextBlock).Text(this,
                            &STMOPVehicleEditor::GetRouteReferenceText,
                            ERouteReferenceField::ViaAnchor) ] ]
                    + SHorizontalBox::Slot().FillWidth(1).Padding(3,0,0,0)
                    [ SAssignNew(ViaLaneCombo, SSearchableComboBox)
                        .OptionsSource(&LaneItems)
                        .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateLaneOption)
                        .OnSelectionChanged(this, &STMOPVehicleEditor::OnRouteReferenceSelected,
                            ERouteReferenceField::ViaLane)
                        [ SNew(STextBlock).Text(this,
                            &STMOPVehicleEditor::GetRouteReferenceText,
                            ERouteReferenceField::ViaLane) ] ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text(LOCTEXT("RecalculateRoute", "Recalculate route"))
                        .OnClicked(this, &STMOPVehicleEditor::RecalculateRoute) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("ClearVia", "Clear via points"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearViaPoints) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(5,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("PreviewRouteLevel", "Preview in level"))
                        .ToolTipText(LOCTEXT("PreviewRouteLevelTip",
                            "Draw the selected route and direction arrows in the open level for 20 seconds."))
                        .OnClicked(this, &STMOPVehicleEditor::PreviewRouteInLevel) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,5,7,2)
                [ SNew(STextBlock).Text(LOCTEXT("EntryAnchor", "ANCHOR FOR SELECTED ENTRY"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2,7,6)
                [ SAssignNew(AnchorCombo, SSearchableComboBox)
                    .OptionsSource(&AnchorItems)
                    .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateAnchorOption)
                    .OnSelectionChanged(this, &STMOPVehicleEditor::OnAnchorSelected)
                    [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetSelectedAnchorText) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,5,7,2)
                [ SNew(STextBlock).Text(LOCTEXT("EntrySharedEvent",
                    "SHARED EVENT FOR SELECTED ENTRY"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2,7,6)
                [ SAssignNew(EventCombo, SSearchableComboBox)
                    .OptionsSource(&EventItems)
                    .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateEventOption)
                    .OnSelectionChanged(this, &STMOPVehicleEditor::OnEventSelected)
                    [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetSelectedEventText) ] ]
                + SVerticalBox::Slot().FillHeight(0.48f).Padding(4)
                [ SAssignNew(RouteMap, STMOPVehicleRouteMap) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text(LOCTEXT("Occupants", "OCCUPANTS / BOARDING FEASIBILITY"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text_Lambda([this]()
                    { return FText::FromString(BuildOccupantsText(SelectedTimelineIndex)); })
                    .AutoWrapText(true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,8,7,2)
                [ SNew(STextBlock).Text(LOCTEXT("Validation", "VALIDATION"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().FillHeight(0.52f).Padding(7,2)
                [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetValidationText)
                    .AutoWrapText(true) ]
            ]
            + SSplitter::Slot().Value(0.28f)
            [
                SNew(SSplitter).Orientation(Orient_Vertical)
                + SSplitter::Slot().Value(0.48f)
                [ SNew(SBorder).Padding(5)[VehicleDetails->GetWidget().ToSharedRef()] ]
                + SSplitter::Slot().Value(0.52f)
                [ SNew(SBorder).Padding(5)[EntryDetails->GetWidget().ToSharedRef()] ]
            ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(8,4)
        [ SAssignNew(StatusText, STextBlock).Text(LOCTEXT("Ready", "Ready")) ]
    ];
    if (RouteMap.IsValid())
        RouteMap->SetLaneClickedHandler(
            [this](const FName LaneId, const bool bStart,
                const bool bDestination, const bool bVia)
            { HandleMapLaneClicked(LaneId, bStart, bDestination, bVia); });
    LoadTables();
}

void STMOPVehicleEditor::LoadTables()
{
    VehicleTable = LoadObject<UDataTable>(nullptr, VehicleTablePath);
    PeopleTable = LoadObject<UDataTable>(nullptr, PeopleTablePath);
    EventTable = LoadObject<UDataTable>(nullptr, EventTablePath);
    RefreshAnchorOptions();
    RefreshLaneOptions();
    RefreshEventOptions();
    if (!VehicleTable.IsValid() || VehicleTable->GetRowStruct() !=
        FTMOPHistoricalVehicleRow::StaticStruct())
    {
        SetStatus(LOCTEXT("MissingVehicles", "DT_TMOP_HistoricalVehicles could not be loaded."), FLinearColor::Red);
        return;
    }
    RefreshVehicles();
    SetStatus(LOCTEXT("Loaded", "DT_TMOP_HistoricalVehicles loaded."), FLinearColor(0.4f,1,0.4f));
}

void STMOPVehicleEditor::RefreshVehicles()
{
    VehicleItems.Reset();
    const UDataTable* Table = VehicleTable.Get();
    if (!IsValid(Table)) return;
    for (const FName RowName : Table->GetRowNames())
    {
        const FTMOPHistoricalVehicleRow* Row = Table->FindRow<FTMOPHistoricalVehicleRow>(RowName, TEXT("VehicleEditorList"), false);
        if (!Row) continue;
        const FString Haystack = RowName.ToString() + TEXT(" ") + Row->VehicleId.ToString() + TEXT(" ") + Row->DisplayName.ToString() + TEXT(" ") + Row->RegistrationNumber;
        if (!Search.IsEmpty() &&
            !Haystack.Contains(Search, ESearchCase::IgnoreCase))
            continue;
        if (!PassesVehicleFilter(*Row)) continue;
        VehicleItems.Add(MakeShared<FName>(RowName));
    }
    VehicleItems.Sort([](const FVehicleItem& A, const FVehicleItem& B){ return A->ToString() < B->ToString(); });
    if (VehicleList.IsValid()) VehicleList->RequestListRefresh();
}

void STMOPVehicleEditor::RefreshTimeline()
{
    TimelineItems.Reset();
    for (int32 I=0; I<WorkingRow.Timeline.Num(); ++I) TimelineItems.Add(MakeShared<int32>(I));
    if (TimelineList.IsValid()) TimelineList->RequestListRefresh();
    RebuildValidation();
    RebuildRoutePreview();
}

void STMOPVehicleEditor::SelectVehicle(const FName RowName)
{
    CommitEntry(); CommitVehicle();
    const UDataTable* Table = VehicleTable.Get();
    const FTMOPHistoricalVehicleRow* Row = IsValid(Table) ? Table->FindRow<FTMOPHistoricalVehicleRow>(RowName, TEXT("VehicleEditorSelect"), false) : nullptr;
    if (!Row) return;
    SelectedRowName=RowName; WorkingRow=*Row; SavedRow=*Row; SelectedTimelineIndex=INDEX_NONE;
    VehicleStruct=MakeShared<FStructOnScope>(FTMOPHistoricalVehicleRow::StaticStruct());
    *reinterpret_cast<FTMOPHistoricalVehicleRow*>(VehicleStruct->GetStructMemory())=WorkingRow;
    VehicleDetails->SetStructureData(VehicleStruct);
    EntryStruct.Reset(); EntryDetails->SetStructureData(nullptr); RefreshTimeline();
    if (!WorkingRow.Timeline.IsEmpty()) SelectTimelineEntry(0);
}

void STMOPVehicleEditor::SelectTimelineEntry(const int32 Index)
{
    CommitEntry();
    if (!WorkingRow.Timeline.IsValidIndex(Index)) return;
    SelectedTimelineIndex=Index;
    EntryStruct=MakeShared<FStructOnScope>(FTMOPHistoricalVehicleTimelineEntry::StaticStruct());
    *reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(EntryStruct->GetStructMemory())=WorkingRow.Timeline[Index];
    EntryDetails->SetStructureData(EntryStruct); RebuildValidation(); RebuildRoutePreview();
}

void STMOPVehicleEditor::RefreshAnchorOptions()
{
    AnchorItems.Reset();
    AnchorIdsByLabel.Reset();
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World)
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        {
            const FName Id = It->GetAnchorId();
            if (Id.IsNone()) continue;
            FString Label = Id.ToString();
            if (!It->DisplayName.IsEmpty())
                Label += TEXT("  •  ") + It->DisplayName.ToString();
            AnchorIdsByLabel.Add(Label, Id);
            AnchorItems.Add(MakeShared<FString>(MoveTemp(Label)));
        }
    AnchorItems.Sort([](const FAnchorItem& A, const FAnchorItem& B)
        { return A.IsValid() && B.IsValid() ? *A < *B : A.IsValid(); });
    if (AnchorCombo.IsValid()) AnchorCombo->RefreshOptions();
    if (StartAnchorCombo.IsValid()) StartAnchorCombo->RefreshOptions();
    if (DestinationAnchorCombo.IsValid()) DestinationAnchorCombo->RefreshOptions();
    if (ViaAnchorCombo.IsValid()) ViaAnchorCombo->RefreshOptions();
}

void STMOPVehicleEditor::RefreshLaneOptions()
{
    LaneItems.Reset();
    LaneIdsByLabel.Reset();
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World)
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Components;
            It->GetComponents<UTMOPTrafficLaneComponent>(Components);
            for (UTMOPTrafficLaneComponent* Lane : Components)
            {
                if (!IsValid(Lane) || Lane->LaneId.IsNone()) continue;
                FString Label = Lane->LaneId.ToString();
                if (!Lane->RoadId.IsNone())
                    Label += TEXT("  •  ") + Lane->RoadId.ToString();
                LaneIdsByLabel.Add(Label, Lane->LaneId);
                LaneItems.Add(MakeShared<FString>(MoveTemp(Label)));
            }
        }
    LaneItems.Sort([](const FLaneItem& A, const FLaneItem& B)
        { return A.IsValid() && B.IsValid() ? *A < *B : A.IsValid(); });
    if (StartLaneCombo.IsValid()) StartLaneCombo->RefreshOptions();
    if (DestinationLaneCombo.IsValid()) DestinationLaneCombo->RefreshOptions();
    if (ViaLaneCombo.IsValid()) ViaLaneCombo->RefreshOptions();
}

void STMOPVehicleEditor::RefreshEventOptions()
{
    EventItems.Reset();
    EventIdsByLabel.Reset();
    const UDataTable* Table = EventTable.Get();
    if (IsValid(Table) &&
        Table->GetRowStruct() == FTMOPHistoricalEventDefinition::StaticStruct())
    {
        for (const FName RowName : Table->GetRowNames())
        {
            const FTMOPHistoricalEventDefinition* Event =
                Table->FindRow<FTMOPHistoricalEventDefinition>(
                    RowName, TEXT("VehicleEditorEvents"), false);
            if (Event == nullptr) continue;
            const FName EventId = Event->EventId.IsNone()
                ? RowName : Event->EventId;
            FString Label = EventId.ToString();
            if (!Event->DisplayName.IsEmpty())
                Label += TEXT("  •  ") + Event->DisplayName.ToString();
            EventIdsByLabel.Add(Label, EventId);
            EventItems.Add(MakeShared<FString>(MoveTemp(Label)));
        }
    }
    EventItems.Sort([](const FEventItem& A, const FEventItem& B)
        { return A.IsValid() && B.IsValid() ? *A < *B : A.IsValid(); });
    if (EventCombo.IsValid()) EventCombo->RefreshOptions();
}

TSharedRef<SWidget> STMOPVehicleEditor::GenerateAnchorOption(
    const FAnchorItem Item) const
{
    return SNew(STextBlock).Text(Item.IsValid()
        ? FText::FromString(*Item) : FText::GetEmpty());
}

void STMOPVehicleEditor::OnAnchorSelected(
    const FAnchorItem Item, ESelectInfo::Type)
{
    if (!Item.IsValid() || !EntryStruct.IsValid()) return;
    const FName* Id = AnchorIdsByLabel.Find(*Item);
    if (!Id) return;
    FTMOPHistoricalVehicleTimelineEntry* Entry =
        reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
            EntryStruct->GetStructMemory());
    Entry->PlacementMode = ETMOPHistoricalVehiclePlacementMode::Anchor;
    Entry->PlacementAnchorId = *Id;
    EntryDetails->SetStructureData(EntryStruct);
    CommitEntry();
    RebuildValidation();
    RebuildRoutePreview();
    SetStatus(FText::FromString(TEXT("Selected anchor: ") + Id->ToString()),
        FLinearColor(.55f,.8f,.55f));
}

FText STMOPVehicleEditor::GetSelectedAnchorText() const
{
    if (!EntryStruct.IsValid())
        return LOCTEXT("SelectEntryForAnchor", "Select a timeline entry");
    const auto* Entry = reinterpret_cast<const FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    return Entry->PlacementAnchorId.IsNone()
        ? LOCTEXT("SearchVehicleAnchor", "Type to search known anchors...")
        : FText::FromName(Entry->PlacementAnchorId);
}

TSharedRef<SWidget> STMOPVehicleEditor::GenerateEventOption(
    const FEventItem Item) const
{
    return SNew(STextBlock)
        .Text(Item.IsValid() ? FText::FromString(*Item) : FText::GetEmpty())
        .ToolTipText(Item.IsValid()
            ? FText::FromString(*Item) : FText::GetEmpty());
}

void STMOPVehicleEditor::OnEventSelected(
    const FEventItem Item, ESelectInfo::Type)
{
    if (!Item.IsValid() || !EntryStruct.IsValid()) return;
    const FName* EventId = EventIdsByLabel.Find(*Item);
    if (EventId == nullptr) return;
    FTMOPHistoricalVehicleTimelineEntry* Entry =
        reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
            EntryStruct->GetStructMemory());
    Entry->TimingMode = ETMOPEventTimingMode::Relative;
    Entry->SharedEventId = *EventId;
    EntryDetails->SetStructureData(EntryStruct);
    CommitEntry();
    RebuildValidation();
    RebuildRoutePreview();
    SetStatus(FText::FromString(TEXT("Selected shared event: ") +
        EventId->ToString()), FLinearColor(.55f,.8f,.55f));
}

FText STMOPVehicleEditor::GetSelectedEventText() const
{
    if (!EntryStruct.IsValid())
        return LOCTEXT("SelectEntryForEvent", "Select a timeline entry");
    const auto* Entry =
        reinterpret_cast<const FTMOPHistoricalVehicleTimelineEntry*>(
            EntryStruct->GetStructMemory());
    return Entry->SharedEventId.IsNone()
        ? LOCTEXT("SearchVehicleEvent", "Type to search shared events...")
        : FText::FromName(Entry->SharedEventId);
}

TSharedRef<SWidget> STMOPVehicleEditor::GenerateLaneOption(
    const FLaneItem Item) const
{
    return SNew(STextBlock).Text(Item.IsValid()
        ? FText::FromString(*Item) : FText::GetEmpty());
}

void STMOPVehicleEditor::OnRouteReferenceSelected(
    const TSharedPtr<FString> Item, ESelectInfo::Type,
    const ERouteReferenceField Field)
{
    if (!Item.IsValid() || !EntryStruct.IsValid()) return;
    auto* Entry = reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    if (!IsDriving(Entry->Action))
    {
        SetStatus(LOCTEXT("SelectDrivingEntry", "Select a driving entry first."),
            FLinearColor(1.0f, .55f, .1f));
        return;
    }
    const bool bAnchor = Field == ERouteReferenceField::StartAnchor ||
        Field == ERouteReferenceField::DestinationAnchor ||
        Field == ERouteReferenceField::ViaAnchor;
    const FName* Id = bAnchor
        ? AnchorIdsByLabel.Find(*Item) : LaneIdsByLabel.Find(*Item);
    if (Id == nullptr) return;

    switch (Field)
    {
    case ERouteReferenceField::StartAnchor:
        Entry->RouteStartAnchorId = *Id;
        Entry->RouteStartLaneId = NAME_None;
        break;
    case ERouteReferenceField::StartLane:
        Entry->RouteStartLaneId = *Id;
        Entry->RouteStartAnchorId = NAME_None;
        break;
    case ERouteReferenceField::DestinationAnchor:
        Entry->RouteDestinationAnchorId = *Id;
        Entry->RouteDestinationLaneId = NAME_None;
        break;
    case ERouteReferenceField::DestinationLane:
        Entry->RouteDestinationLaneId = *Id;
        Entry->RouteDestinationAnchorId = NAME_None;
        break;
    case ERouteReferenceField::ViaAnchor:
        Entry->RouteViaAnchorIds.AddUnique(*Id);
        break;
    case ERouteReferenceField::ViaLane:
        Entry->RouteViaLaneIds.AddUnique(*Id);
        break;
    }
    EntryDetails->SetStructureData(EntryStruct);
    CommitEntry();
    FString Failure;
    if (RecalculateSelectedRoute(Failure))
        SetStatus(LOCTEXT("RouteRecalculated", "Route recalculated and map updated."),
            FLinearColor(.4f, 1.0f, .4f));
    else
        SetStatus(FText::FromString(TEXT("Route not calculated: ") + Failure),
            FLinearColor(1.0f, .55f, .1f));
    RefreshTimeline();
}

FText STMOPVehicleEditor::GetRouteReferenceText(
    const ERouteReferenceField Field) const
{
    if (!EntryStruct.IsValid())
        return LOCTEXT("SelectRouteEntry", "Select driving entry");
    const auto* Entry =
        reinterpret_cast<const FTMOPHistoricalVehicleTimelineEntry*>(
            EntryStruct->GetStructMemory());
    FName Id = NAME_None;
    FString Empty;
    switch (Field)
    {
    case ERouteReferenceField::StartAnchor:
        Id = Entry->RouteStartAnchorId; Empty = TEXT("Start anchor..."); break;
    case ERouteReferenceField::StartLane:
        Id = Entry->RouteStartLaneId; Empty = TEXT("Start lane..."); break;
    case ERouteReferenceField::DestinationAnchor:
        Id = Entry->RouteDestinationAnchorId; Empty = TEXT("End anchor..."); break;
    case ERouteReferenceField::DestinationLane:
        Id = Entry->RouteDestinationLaneId; Empty = TEXT("End lane..."); break;
    case ERouteReferenceField::ViaAnchor:
        Empty = FString::Printf(TEXT("Add via anchor... (%d)"),
            Entry->RouteViaAnchorIds.Num()); break;
    case ERouteReferenceField::ViaLane:
        Empty = FString::Printf(TEXT("Add via lane... (%d)"),
            Entry->RouteViaLaneIds.Num()); break;
    }
    return Id.IsNone() ? FText::FromString(Empty) : FText::FromName(Id);
}

FReply STMOPVehicleEditor::RecalculateRoute()
{
    CommitEntry();
    FString Failure;
    if (RecalculateSelectedRoute(Failure))
    {
        RefreshTimeline();
        if (EntryStruct.IsValid() &&
            WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
            *reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
                EntryStruct->GetStructMemory()) =
                WorkingRow.Timeline[SelectedTimelineIndex];
        SetStatus(LOCTEXT("RouteRecalculatedButton",
            "Route recalculated and minimap updated."),
            FLinearColor(.4f, 1.0f, .4f));
    }
    else
        SetStatus(FText::FromString(TEXT("Route failed: ") + Failure),
            FLinearColor::Red);
    return FReply::Handled();
}

FReply STMOPVehicleEditor::ClearViaPoints()
{
    if (!EntryStruct.IsValid()) return FReply::Handled();
    auto* Entry = reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    Entry->RouteViaAnchorIds.Reset();
    Entry->RouteViaLaneIds.Reset();
    EntryDetails->SetStructureData(EntryStruct);
    return RecalculateRoute();
}

FReply STMOPVehicleEditor::PreviewRouteInLevel()
{
    CommitEntry();
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) ||
        !IsDriving(WorkingRow.Timeline[SelectedTimelineIndex].Action))
    {
        SetStatus(LOCTEXT("PreviewNeedsRoute",
            "Select a driving segment before previewing."), FLinearColor::Red);
        return FReply::Handled();
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World == nullptr) return FReply::Handled();
    TMap<FName, UTMOPTrafficLaneComponent*> Lanes;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Components;
        It->GetComponents<UTMOPTrafficLaneComponent>(Components);
        for (UTMOPTrafficLaneComponent* Lane : Components)
            if (IsValid(Lane) && !Lane->LaneId.IsNone())
                Lanes.Add(Lane->LaneId, Lane);
    }
    int32 Drawn = 0;
    for (const FName LaneId :
        WorkingRow.Timeline[SelectedTimelineIndex].OrderedLaneIds)
    {
        UTMOPTrafficLaneComponent* const* Found = Lanes.Find(LaneId);
        if (Found == nullptr || !IsValid(*Found)) continue;
        UTMOPTrafficLaneComponent* Lane = *Found;
        const float Length = Lane->GetSplineLength();
        const int32 Samples = FMath::Max(2, FMath::CeilToInt(Length / 250.0f));
        FVector Previous = Lane->GetLocationAtDistanceAlongSpline(
            0.0f, ESplineCoordinateSpace::World) + FVector(0, 0, 35);
        for (int32 Sample = 1; Sample <= Samples; ++Sample)
        {
            const FVector Current = Lane->GetLocationAtDistanceAlongSpline(
                Length * Sample / Samples,
                ESplineCoordinateSpace::World) + FVector(0, 0, 35);
            DrawDebugLine(World, Previous, Current,
                FColor(0, 170, 255), false, 20.0f, 0, 12.0f);
            Previous = Current;
        }
        const FVector ArrowStart = Lane->GetLocationAtDistanceAlongSpline(
            Length * .42f, ESplineCoordinateSpace::World) + FVector(0,0,55);
        const FVector ArrowEnd = Lane->GetLocationAtDistanceAlongSpline(
            Length * .58f, ESplineCoordinateSpace::World) + FVector(0,0,55);
        DrawDebugDirectionalArrow(World, ArrowStart, ArrowEnd, 120.0f,
            FColor::Yellow, false, 20.0f, 0, 18.0f);
        ++Drawn;
    }
    SetStatus(FText::FromString(FString::Printf(
        TEXT("Previewing %d route lanes in the level for 20 seconds."), Drawn)),
        Drawn > 0 ? FLinearColor(.4f, 1.0f, .4f) : FLinearColor::Red);
    return FReply::Handled();
}

void STMOPVehicleEditor::HandleMapLaneClicked(const FName LaneId,
    const bool bSetStart, const bool bSetDestination, const bool bAddVia)
{
    if (!EntryStruct.IsValid() || LaneId.IsNone()) return;
    auto* Entry = reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    if (!IsDriving(Entry->Action))
    {
        SetStatus(LOCTEXT("MapSelectDriving",
            "Select a driving segment before choosing lanes on the map."),
            FLinearColor(1.0f, .55f, .1f));
        return;
    }
    if (bAddVia)
        Entry->RouteViaLaneIds.AddUnique(LaneId);
    else if (bSetStart)
    {
        Entry->RouteStartLaneId = LaneId;
        Entry->RouteStartAnchorId = NAME_None;
    }
    else if (bSetDestination)
    {
        Entry->RouteDestinationLaneId = LaneId;
        Entry->RouteDestinationAnchorId = NAME_None;
    }
    else
    {
        SetStatus(FText::FromString(FString::Printf(
            TEXT("Lane %s — Shift+click=start, Ctrl+click=end, right-click=via."),
            *LaneId.ToString())), FLinearColor(.55f, .8f, 1.0f));
        return;
    }
    EntryDetails->SetStructureData(EntryStruct);
    CommitEntry();
    FString Failure;
    if (RecalculateSelectedRoute(Failure))
        SetStatus(FText::FromString(FString::Printf(
            TEXT("Lane %s selected; route and minimap updated."),
            *LaneId.ToString())), FLinearColor(.4f, 1.0f, .4f));
    else
        SetStatus(FText::FromString(TEXT("Lane selected; route incomplete: ") +
            Failure), FLinearColor(1.0f, .55f, .1f));
    RefreshTimeline();
}

bool STMOPVehicleEditor::RecalculateSelectedRoute(FString& OutFailure)
{
    OutFailure.Reset();
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) ||
        !IsDriving(WorkingRow.Timeline[SelectedTimelineIndex].Action))
    {
        OutFailure = TEXT("select a Begin Driving / Enter Traffic Route entry");
        return false;
    }
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World == nullptr)
    {
        OutFailure = TEXT("open the level containing the traffic lanes");
        return false;
    }

    FTMOPHistoricalVehicleTimelineEntry& Entry =
        WorkingRow.Timeline[SelectedTimelineIndex];
    TMap<FName, UTMOPTrafficLaneComponent*> Lanes;
    TMap<FName, FVector> Anchors;
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        if (!It->GetAnchorId().IsNone())
            Anchors.Add(It->GetAnchorId(), It->GetAnchorLocation());
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Components;
        It->GetComponents<UTMOPTrafficLaneComponent>(Components);
        for (UTMOPTrafficLaneComponent* Lane : Components)
            if (IsValid(Lane) && !Lane->LaneId.IsNone())
                Lanes.Add(Lane->LaneId, Lane);
    }
    if (Lanes.IsEmpty())
    {
        OutFailure = TEXT("no traffic lanes exist in the open level");
        return false;
    }

    auto NearestLane = [&Lanes](const FVector& Point, FName& OutLane)
    {
        double Best = TNumericLimits<double>::Max();
        for (const TPair<FName, UTMOPTrafficLaneComponent*>& Pair : Lanes)
        {
            const float Key = Pair.Value->FindInputKeyClosestToWorldLocation(Point);
            const FVector LanePoint = Pair.Value->GetLocationAtSplineInputKey(
                Key, ESplineCoordinateSpace::World);
            const double Distance = FVector::DistSquared(Point, LanePoint);
            if (Distance < Best) { Best = Distance; OutLane = Pair.Key; }
        }
        return !OutLane.IsNone();
    };
    auto LaneForAnchor = [&Anchors, &NearestLane](const FName AnchorId,
        FName& OutLane)
    {
        const FVector* Point = Anchors.Find(AnchorId);
        return Point != nullptr && NearestLane(*Point, OutLane);
    };
    auto PlacementPoint = [&Anchors](
        const FTMOPHistoricalVehicleTimelineEntry& Placement,
        FVector& OutPoint)
    {
        if (Placement.PlacementMode ==
            ETMOPHistoricalVehiclePlacementMode::WorldTransform)
        {
            OutPoint = Placement.WorldTransform.GetLocation();
            return true;
        }
        const FVector* AnchorPoint = Anchors.Find(Placement.PlacementAnchorId);
        if (AnchorPoint == nullptr) return false;
        OutPoint = *AnchorPoint + Placement.AnchorLocalOffset.GetLocation();
        return true;
    };

    FName StartLane = Entry.RouteStartLaneId;
    if (StartLane.IsNone() && !Entry.RouteStartAnchorId.IsNone())
        LaneForAnchor(Entry.RouteStartAnchorId, StartLane);
    if (StartLane.IsNone())
        for (int32 Index = SelectedTimelineIndex - 1; Index >= 0; --Index)
        {
            if (!HasPlacement(WorkingRow.Timeline[Index].Action)) continue;
            FVector Point;
            if (PlacementPoint(WorkingRow.Timeline[Index], Point))
                NearestLane(Point, StartLane);
            break;
        }

    FName EndLane = Entry.RouteDestinationLaneId;
    if (EndLane.IsNone() && !Entry.RouteDestinationAnchorId.IsNone())
        LaneForAnchor(Entry.RouteDestinationAnchorId, EndLane);
    if (EndLane.IsNone())
        for (int32 Index = SelectedTimelineIndex + 1;
            Index < WorkingRow.Timeline.Num(); ++Index)
        {
            if (!IsStop(WorkingRow.Timeline[Index].Action)) continue;
            FVector Point;
            if (PlacementPoint(WorkingRow.Timeline[Index], Point))
                NearestLane(Point, EndLane);
            break;
        }
    if (!Lanes.Contains(StartLane) || !Lanes.Contains(EndLane))
    {
        OutFailure = TEXT("set a valid start and destination anchor/lane");
        return false;
    }

    TArray<FName> Waypoints;
    Waypoints.Add(StartLane);
    for (const FName AnchorId : Entry.RouteViaAnchorIds)
    {
        FName LaneId;
        if (!LaneForAnchor(AnchorId, LaneId))
        {
            OutFailure = FString::Printf(TEXT("via anchor '%s' cannot resolve to a lane"),
                *AnchorId.ToString());
            return false;
        }
        Waypoints.Add(LaneId);
    }
    for (const FName LaneId : Entry.RouteViaLaneIds)
    {
        if (!Lanes.Contains(LaneId))
        {
            OutFailure = FString::Printf(TEXT("via lane '%s' is missing"),
                *LaneId.ToString());
            return false;
        }
        Waypoints.Add(LaneId);
    }
    Waypoints.Add(EndLane);

    auto FindSegment = [&Lanes, &Entry](const FName From, const FName To,
        TArray<FName>& OutSegment)
    {
        OutSegment.Reset();
        if (From == To) { OutSegment.Add(From); return true; }
        TMap<FName, double> Distances;
        TMap<FName, FName> Previous;
        TSet<FName> Open;
        for (const TPair<FName, UTMOPTrafficLaneComponent*>& Pair : Lanes)
        {
            Distances.Add(Pair.Key, TNumericLimits<double>::Max());
            Open.Add(Pair.Key);
        }
        Distances[From] = 0.0;
        while (!Open.IsEmpty())
        {
            FName Current = NAME_None;
            double Best = TNumericLimits<double>::Max();
            for (const FName Candidate : Open)
                if (const double* Value = Distances.Find(Candidate))
                    if (*Value < Best)
                    { Best = *Value; Current = Candidate; }
            if (Current.IsNone() || Best == TNumericLimits<double>::Max()) break;
            Open.Remove(Current);
            if (Current == To) break;
            UTMOPTrafficLaneComponent* const* Lane = Lanes.Find(Current);
            if (Lane == nullptr || !IsValid(*Lane)) continue;
            for (const FTMOPLaneConnection& Connection : (*Lane)->NextLanes)
            {
                if (!Connection.bAllowed && !Entry.bIgnoreOneWayRestrictions)
                    continue;
                UTMOPTrafficLaneComponent* const* Next =
                    Lanes.Find(Connection.TargetLaneId);
                if (Next == nullptr || !Open.Contains(Connection.TargetLaneId))
                    continue;
                const double NewDistance = Best + (*Next)->GetSplineLength();
                double* Existing = Distances.Find(Connection.TargetLaneId);
                if (Existing != nullptr && NewDistance < *Existing)
                {
                    *Existing = NewDistance;
                    Previous.Add(Connection.TargetLaneId, Current);
                }
            }
        }
        if (!Previous.Contains(To)) return false;
        for (FName At = To; !At.IsNone(); At = Previous.FindRef(At))
        {
            OutSegment.Insert(At, 0);
            if (At == From) return true;
        }
        return false;
    };

    TArray<FName> NewRoute;
    for (int32 Index = 0; Index + 1 < Waypoints.Num(); ++Index)
    {
        TArray<FName> Segment;
        if (!FindSegment(Waypoints[Index], Waypoints[Index + 1], Segment))
        {
            OutFailure = FString::Printf(TEXT("no connected route from '%s' to '%s'%s"),
                *Waypoints[Index].ToString(), *Waypoints[Index + 1].ToString(),
                Entry.bIgnoreOneWayRestrictions
                    ? TEXT(" even with restricted connections enabled") : TEXT(""));
            return false;
        }
        if (!NewRoute.IsEmpty() && !Segment.IsEmpty() &&
            NewRoute.Last() == Segment[0])
            Segment.RemoveAt(0);
        NewRoute.Append(Segment);
    }
    Entry.OrderedLaneIds = MoveTemp(NewRoute);
    if (EntryStruct.IsValid())
        *reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
            EntryStruct->GetStructMemory()) = Entry;
    RebuildValidation();
    RebuildRoutePreview();
    return true;
}

FText STMOPVehicleEditor::GetRouteEndpointsText() const
{
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        return LOCTEXT("NoRouteEndpoints", "Select a timeline entry.");
    int32 DriveIndex = SelectedTimelineIndex;
    if (!IsDriving(WorkingRow.Timeline[DriveIndex].Action))
        for (int32 Index = SelectedTimelineIndex - 1; Index >= 0; --Index)
            if (IsDriving(WorkingRow.Timeline[Index].Action))
            { DriveIndex = Index; break; }
    if (!IsDriving(WorkingRow.Timeline[DriveIndex].Action))
        return LOCTEXT("NoDriveForEndpoints", "No driving route selected.");

    const FTMOPHistoricalVehicleTimelineEntry& Drive =
        WorkingRow.Timeline[DriveIndex];
    FName Start = Drive.RouteStartAnchorId;
    FName Destination = Drive.RouteDestinationAnchorId;
    for (int32 Index = DriveIndex - 1; Index >= 0; --Index)
        if (Start.IsNone() &&
            !WorkingRow.Timeline[Index].PlacementAnchorId.IsNone())
        { Start = WorkingRow.Timeline[Index].PlacementAnchorId; break; }
    for (int32 Index = DriveIndex + 1; Index < WorkingRow.Timeline.Num(); ++Index)
        if (Destination.IsNone() && IsStop(WorkingRow.Timeline[Index].Action) &&
            !WorkingRow.Timeline[Index].PlacementAnchorId.IsNone())
        { Destination = WorkingRow.Timeline[Index].PlacementAnchorId; break; }
    TArray<FString> ViaAnchors;
    for (const FName Id : Drive.RouteViaAnchorIds) ViaAnchors.Add(Id.ToString());
    TArray<FString> ViaLanes;
    for (const FName Id : Drive.RouteViaLaneIds) ViaLanes.Add(Id.ToString());
    return FText::FromString(FString::Printf(
        TEXT("Start anchor: %s | lane: %s\nEnd anchor: %s | lane: %s\nVia anchors: %s | via lanes: %s"),
        Start.IsNone() ? TEXT("not set") : *Start.ToString(),
        Drive.RouteStartLaneId.IsNone() ? TEXT("auto") :
            *Drive.RouteStartLaneId.ToString(),
        Destination.IsNone() ? TEXT("not set") : *Destination.ToString(),
        Drive.RouteDestinationLaneId.IsNone() ? TEXT("auto") :
            *Drive.RouteDestinationLaneId.ToString(),
        ViaAnchors.IsEmpty() ? TEXT("none") : *FString::Join(ViaAnchors, TEXT(", ")),
        ViaLanes.IsEmpty() ? TEXT("none") : *FString::Join(ViaLanes, TEXT(", "))));
}

void STMOPVehicleEditor::CommitEntry()
{
    if (EntryStruct.IsValid() && WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        WorkingRow.Timeline[SelectedTimelineIndex]=*reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(EntryStruct->GetStructMemory());
}

void STMOPVehicleEditor::CommitVehicle()
{
    if (VehicleStruct.IsValid())
    {
        FTMOPHistoricalVehicleRow Details=*reinterpret_cast<FTMOPHistoricalVehicleRow*>(VehicleStruct->GetStructMemory());
        Details.Timeline=WorkingRow.Timeline; WorkingRow=MoveTemp(Details);
    }
}

TSharedRef<ITableRow> STMOPVehicleEditor::GenerateVehicleRow(const FVehicleItem Item, const TSharedRef<STableViewBase>& Owner)
{
    const UDataTable* T=VehicleTable.Get(); const FTMOPHistoricalVehicleRow* R=IsValid(T)&&Item.IsValid()?T->FindRow<FTMOPHistoricalVehicleRow>(*Item,TEXT("VehicleEditorRow"),false):nullptr;
    return SNew(STableRow<FVehicleItem>,Owner)[SNew(SVerticalBox)
        +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(R&&!R->DisplayName.IsEmpty()?R->DisplayName:FText::FromName(Item.IsValid()?*Item:NAME_None))]
        +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(R?FText::FromString(R->VehicleId.ToString()+TEXT(" • ")+R->RegistrationNumber):FText::GetEmpty()).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FSlateColor::UseSubduedForeground())]];
}

TSharedRef<ITableRow> STMOPVehicleEditor::GenerateTimelineRow(const FTimelineItem Item, const TSharedRef<STableViewBase>& Owner)
{
    const int32 I=Item.IsValid()?*Item:INDEX_NONE; const FTMOPHistoricalVehicleTimelineEntry* E=WorkingRow.Timeline.IsValidIndex(I)?&WorkingRow.Timeline[I]:nullptr;
    int32 Sec=0; FString Fail; const bool HasTime=E&&ResolveTime(WorkingRow,I,Sec,&Fail); FLinearColor Color(0.35f,0.75f,1);
    FString Badge; FString Tip;
    if (E&&IsDriving(E->Action)) { double D=0,K=0; int32 Departure=0,Arrival=0,S=0; if(CalculateDrive(WorkingRow,I,D,Departure,Arrival,S,K,Fail)){Badge=FString::Printf(TEXT("DEP %s  ARR %s  REQ %.1f km/h"),*FormatClockSecond(Departure),*FormatClockSecond(Arrival),K);const float SetKmh=E->CruiseSpeedOverrideKmh>0?E->CruiseSpeedOverrideKmh:DrivingPresetSpeedKmh(E->DrivingPreset);if(SetKmh>0){const int32 EstimatedSeconds=FMath::RoundToInt((D/100000.0)/(SetKmh/3600.0));const int32 EstimatedArrival=Departure+EstimatedSeconds;Badge+=FString::Printf(TEXT("  SET %.1f  ETA %s"),SetKmh,*FormatClockSecond(EstimatedArrival));Tip=FString::Printf(TEXT("%.2f km. Selected speed estimates arrival %+d seconds versus plan."),D/100000.0,EstimatedArrival-Arrival);}else Tip=FString::Printf(TEXT("%.2f km over %d seconds."),D/100000.0,S);Color=K<=50?FLinearColor(0.05f,.42f,.12f):K<=90?FLinearColor(.78f,.38f,.03f):FLinearColor(.75f,.05f,.03f);if(E->bIgnoreOneWayRestrictions)Tip+=TEXT(" Ignores restricted lane connections.");if(E->bRunRedLights)Tip+=TEXT(" May run red lights.");} else {Badge=TEXT("SPEED ?");Tip=Fail;Color=FLinearColor(.55f,.12f,.08f);} }
    FString Summary=E?VehicleActionLabel(E->Action):FString(); if(E&&IsDriving(E->Action)&&!E->RouteSegmentName.IsEmpty())Summary+=TEXT(" • ")+E->RouteSegmentName.ToString();if(E&&!E->PlacementAnchorId.IsNone()) Summary+=TEXT(" → ")+E->PlacementAnchorId.ToString(); if(E&&!E->OrderedLaneIds.IsEmpty()) Summary+=FString::Printf(TEXT(" • %d lanes"),E->OrderedLaneIds.Num());if(E&&IsDriving(E->Action)&&E->DrivingPreset!=ETMOPVehicleDrivingPreset::AutomaticFromTimeline)Summary+=TEXT(" • ")+DrivingPresetLabel(E->DrivingPreset);
    if (E && E->TimingMode == ETMOPEventTimingMode::Relative)
        Summary += FString::Printf(TEXT(" • @ %s %+d s"),
            *E->SharedEventId.ToString(), E->EventOffsetSeconds);
    else if (E && E->TimingMode ==
        ETMOPEventTimingMode::RelativeToPreviousEntry)
        Summary += FString::Printf(TEXT(" • Previous %+d s"),
            E->EventOffsetSeconds);
    if (E && IsDriving(E->Action) && E->bTimeIsArrival)
        Summary += TEXT(" • TIME IS ARRIVAL");
    const int32 N=FMath::Max(0,Sec)%(24*3600); const FString Time=HasTime?FString::Printf(TEXT("%02d:%02d:%02d"),N/3600,(N/60)%60,N%60):TEXT("TIME ?");
    return SNew(STableRow<FTimelineItem>,Owner)[SNew(SBorder).Padding(6).BorderImage(FAppStyle::GetBrush("Brushes.Panel"))[SNew(SVerticalBox)
        +SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::AsNumber(I)).ColorAndOpacity(FSlateColor::UseSubduedForeground())]+SHorizontalBox::Slot().FillWidth(1).Padding(7,0)[SNew(STextBlock).Text(E?FText::FromName(E->EntryId):FText::GetEmpty()).ColorAndOpacity(Color)]+SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::FromString(Time)).ColorAndOpacity(FLinearColor(1,.72f,.05f))]]
        +SVerticalBox::Slot().AutoHeight().Padding(22,2,0,0)[SNew(STextBlock).Text(FText::FromString(Summary)).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
        +SVerticalBox::Slot().AutoHeight().Padding(22,2,0,0)[SNew(STextBlock).Text(FText::FromString(Badge)).ToolTipText(FText::FromString(Tip)).ColorAndOpacity(Color)]]];
}

void STMOPVehicleEditor::OnVehicleSelected(FVehicleItem I,ESelectInfo::Type){if(I.IsValid())SelectVehicle(*I);} void STMOPVehicleEditor::OnTimelineSelected(FTimelineItem I,ESelectInfo::Type){if(I.IsValid())SelectTimelineEntry(*I);} void STMOPVehicleEditor::OnSearchChanged(const FText& T){Search=T.ToString();RefreshVehicles();}

ECheckBoxState STMOPVehicleEditor::GetVehicleFilterCheckState(
    const EVehicleListFilter Filter) const
{
    return VehicleListFilter == Filter
        ? ECheckBoxState::Checked
        : ECheckBoxState::Unchecked;
}

void STMOPVehicleEditor::HandleVehicleFilterChanged(
    const ECheckBoxState NewState, const EVehicleListFilter Filter)
{
    // Match the People Editor: exactly one list filter is active at a time.
    if (NewState != ECheckBoxState::Checked) return;
    VehicleListFilter = Filter;
    RefreshVehicles();
}

bool STMOPVehicleEditor::PassesVehicleFilter(
    const FTMOPHistoricalVehicleRow& Row) const
{
    switch (VehicleListFilter)
    {
    case EVehicleListFilter::SpawnedCars:
        return Row.bSpawnInSimulation;
    case EVehicleListFilter::SpawnedCarsWithTimelines:
        return Row.bSpawnInSimulation && !Row.Timeline.IsEmpty();
    case EVehicleListFilter::MainWitnessCars:
        return Row.CategoryId == FName(TEXT("MAIN_WITNESSES"));
    case EVehicleListFilter::PoliceCars:
    {
        if (Row.VehicleCategory == ETMOPVehicleCategory::Police) return true;
        const FString Category = Row.CategoryId.ToString().TrimStartAndEnd();
        return Category.Equals(TEXT("POLICE"), ESearchCase::IgnoreCase) ||
            Category.Equals(TEXT("POLIS"), ESearchCase::IgnoreCase) ||
            Category.EndsWith(TEXT("_POLICE"), ESearchCase::IgnoreCase) ||
            Category.EndsWith(TEXT("_POLIS"), ESearchCase::IgnoreCase);
    }
    case EVehicleListFilter::AllCars:
    default:
        return true;
    }
}

FReply STMOPVehicleEditor::AddEntry(){CommitEntry();FTMOPHistoricalVehicleTimelineEntry E;E.EntryId=FName(*FString::Printf(TEXT("%s_ENTRY_%02d"),*WorkingRow.VehicleId.ToString(),WorkingRow.Timeline.Num()));WorkingRow.Timeline.Add(E);RefreshTimeline();SelectTimelineEntry(WorkingRow.Timeline.Num()-1);return FReply::Handled();}
FReply STMOPVehicleEditor::DuplicateEntry(){CommitEntry();if(WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)){auto E=WorkingRow.Timeline[SelectedTimelineIndex];E.EntryId=FName(*(E.EntryId.ToString()+TEXT("_COPY")));WorkingRow.Timeline.Insert(E,SelectedTimelineIndex+1);RefreshTimeline();SelectTimelineEntry(SelectedTimelineIndex+1);}return FReply::Handled();}
FReply STMOPVehicleEditor::DeleteEntry(){CommitEntry();if(WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)){WorkingRow.Timeline.RemoveAt(SelectedTimelineIndex);SelectedTimelineIndex=INDEX_NONE;EntryStruct.Reset();EntryDetails->SetStructureData(nullptr);RefreshTimeline();}return FReply::Handled();}
FReply STMOPVehicleEditor::MoveEntry(const int32 D){CommitEntry();const int32 N=SelectedTimelineIndex+D;if(WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)&&WorkingRow.Timeline.IsValidIndex(N)){WorkingRow.Timeline.Swap(SelectedTimelineIndex,N);SelectedTimelineIndex=INDEX_NONE;RefreshTimeline();SelectTimelineEntry(N);}return FReply::Handled();}

FReply STMOPVehicleEditor::SaveVehicle()
{
    CommitEntry();
    CommitVehicle();
    UDataTable* Table = VehicleTable.Get();
    if (!IsValid(Table) || SelectedRowName.IsNone()) return FReply::Handled();

    CurrentErrors = ValidateRow(WorkingRow);
    const FString* BlockingError = CurrentErrors.FindByPredicate(
        [](const FString& Message) { return Message.StartsWith(TEXT("ERROR")); });
    if (BlockingError)
    {
        SetStatus(FText::FromString(TEXT("Not saved: ") + *BlockingError),
            FLinearColor::Red);
        return FReply::Handled();
    }

    const FName NewRowName = WorkingRow.VehicleId;
    if (NewRowName.IsNone()) return FReply::Handled();
    if (NewRowName != SelectedRowName && Table->GetRowMap().Contains(NewRowName))
    {
        SetStatus(FText::FromString(FString::Printf(
            TEXT("Not saved: row '%s' already exists."), *NewRowName.ToString())),
            FLinearColor::Red);
        return FReply::Handled();
    }

    Table->Modify();
    if (NewRowName != SelectedRowName)
    {
        Table->AddRow(NewRowName, WorkingRow);
        Table->RemoveRow(SelectedRowName);
        SelectedRowName = NewRowName;
    }
    else if (FTMOPHistoricalVehicleRow* Row =
        Table->FindRow<FTMOPHistoricalVehicleRow>(SelectedRowName,
            TEXT("VehicleEditorSave"), false))
    {
        *Row = WorkingRow;
    }
    Table->MarkPackageDirty();
    Table->PostEditChange();
    SavedRow = WorkingRow;
    RefreshVehicles();
    RefreshTimeline();
    SetStatus(LOCTEXT("Saved", "Vehicle saved. Save the project to write the asset to disk."),
        CurrentErrors.IsEmpty() ? FLinearColor(.4f,1,.4f) : FLinearColor(1,.65f,.1f));
    return FReply::Handled();
}
FReply STMOPVehicleEditor::ReloadVehicle(){if(!SelectedRowName.IsNone())SelectVehicle(SelectedRowName);return FReply::Handled();}
FReply STMOPVehicleEditor::ValidateAll(){const UDataTable* T=VehicleTable.Get();int32 Errors=0,Vehicles=0;if(IsValid(T))for(const FName N:T->GetRowNames())if(const auto*R=T->FindRow<FTMOPHistoricalVehicleRow>(N,TEXT("VehicleEditorValidateAll"),false)){++Vehicles;Errors+=ValidateRow(*R).Num();}SetStatus(FText::FromString(FString::Printf(TEXT("Validated %d vehicles: %d error(s)/warning(s)."),Vehicles,Errors)),Errors?FLinearColor(1,.55f,.1f):FLinearColor(.4f,1,.4f));return FReply::Handled();}

bool STMOPVehicleEditor::ResolveTime(const FTMOPHistoricalVehicleRow& Row,const int32 I,int32& Out,FString* Failure)const
{
    if (Failure) Failure->Reset();
    auto Fail = [Failure](const FString& Message)
    {
        if (Failure && Failure->IsEmpty()) *Failure = Message;
        return false;
    };
    if (!Row.Timeline.IsValidIndex(I))
        return Fail(TEXT("Timeline entry does not exist."));

    const UDataTable* Events = EventTable.Get();
    TSet<FName> ResolvingEvents;
    TFunction<bool(FName, int32&)> ResolveEvent;
    ResolveEvent = [&](const FName EventId, int32& EventSecond)
    {
        if (EventId.IsNone())
            return Fail(TEXT("Shared Event ID is empty."));
        if (!IsValid(Events))
            return Fail(TEXT("Historical event table unavailable."));
        if (ResolvingEvents.Contains(EventId))
            return Fail(FString::Printf(
                TEXT("Shared-event cycle at '%s'."), *EventId.ToString()));
        const FTMOPHistoricalEventDefinition* Definition =
            Events->FindRow<FTMOPHistoricalEventDefinition>(
                EventId, TEXT("VehicleEditorTime"), false);
        if (Definition == nullptr)
            for (const FName RowName : Events->GetRowNames())
                if (const FTMOPHistoricalEventDefinition* Candidate =
                    Events->FindRow<FTMOPHistoricalEventDefinition>(
                        RowName, TEXT("VehicleEditorTimeById"), false))
                    if (Candidate->EventId == EventId)
                    { Definition = Candidate; break; }
        if (Definition == nullptr)
            return Fail(FString::Printf(TEXT("Shared event '%s' not found."),
                *EventId.ToString()));

        ResolvingEvents.Add(EventId);
        bool bResolved = true;
        switch (Definition->TimingMode)
        {
        case ETMOPEventTimingMode::Absolute:
            EventSecond = Definition->AbsoluteTime.ToSecondsFromMidnight();
            break;
        case ETMOPEventTimingMode::Window:
            EventSecond = Definition->PreferredTime.ToSecondsFromMidnight();
            break;
        case ETMOPEventTimingMode::Relative:
        {
            int32 TriggerSecond = 0;
            bResolved = ResolveEvent(Definition->TriggerEventId, TriggerSecond);
            if (bResolved)
                EventSecond = TriggerSecond +
                    Definition->PreferredDelaySeconds;
            break;
        }
        case ETMOPEventTimingMode::RelativeToPreviousEntry:
        default:
            bResolved = Fail(FString::Printf(
                TEXT("Shared event '%s' has an unsupported timing mode."),
                *EventId.ToString()));
            break;
        }
        ResolvingEvents.Remove(EventId);
        return bResolved;
    };

    TSet<int32> ResolvingEntries;
    TFunction<bool(int32, int32&)> ResolveEntry;
    ResolveEntry = [&](const int32 Index, int32& EntrySecond)
    {
        if (!Row.Timeline.IsValidIndex(Index))
            return Fail(TEXT("Previous timeline entry is missing."));
        if (ResolvingEntries.Contains(Index))
            return Fail(TEXT("Timeline timing cycle detected."));
        ResolvingEntries.Add(Index);
        const FTMOPHistoricalVehicleTimelineEntry& Entry = Row.Timeline[Index];
        bool bResolved = true;
        if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
        {
            int32 EventSecond = 0;
            bResolved = ResolveEvent(Entry.SharedEventId, EventSecond);
            if (bResolved)
                EntrySecond = EventSecond + Entry.EventOffsetSeconds;
        }
        else if (Entry.TimingMode ==
            ETMOPEventTimingMode::RelativeToPreviousEntry)
        {
            int32 PreviousSecond = 0;
            bResolved = Index > 0
                ? ResolveEntry(Index - 1, PreviousSecond)
                : Fail(TEXT("First timeline entry cannot be relative to the previous entry."));
            if (bResolved)
                EntrySecond = PreviousSecond + Entry.EventOffsetSeconds;
        }
        else
        {
            EntrySecond = Entry.Time.ToSecondsFromMidnight();
        }
        ResolvingEntries.Remove(Index);
        return bResolved;
    };
    return ResolveEntry(I, Out);
}

bool STMOPVehicleEditor::CalculateDrive(const FTMOPHistoricalVehicleRow& Row,const int32 I,double& Distance,int32& Departure,int32& Arrival,int32& Duration,double& Kmh,FString& Failure)const
{
    Distance=0;Departure=0;Arrival=0;Duration=0;Kmh=0;Failure.Reset();if(!Row.Timeline.IsValidIndex(I)||!IsDriving(Row.Timeline[I].Action)){Failure=TEXT("Not a driving entry.");return false;}const auto&E=Row.Timeline[I];if(E.OrderedLaneIds.IsEmpty()){Failure=TEXT("No Ordered Lane IDs.");return false;}UWorld*W=GEditor?GEditor->GetEditorWorldContext().World():nullptr;if(!W){Failure=TEXT("Open the level containing the lanes.");return false;}TMap<FName,UTMOPTrafficLaneComponent*>Lanes;for(TActorIterator<AActor>It(W);It;++It){TArray<UTMOPTrafficLaneComponent*>Cs;It->GetComponents<UTMOPTrafficLaneComponent>(Cs);for(auto*C:Cs)if(IsValid(C)&&!C->LaneId.IsNone())Lanes.Add(C->LaneId,C);}for(const FName L:E.OrderedLaneIds){auto**C=Lanes.Find(L);if(!C||!IsValid(*C)){Failure=FString::Printf(TEXT("Lane '%s' is missing."),*L.ToString());return false;}Distance+=(*C)->GetSplineLength();}if(E.bTimeIsArrival){if(I<=0||!ResolveTime(Row,I-1,Departure)){Failure=TEXT("Time Is Arrival needs a valid previous entry as departure.");return false;}if(!ResolveTime(Row,I,Arrival)){Failure=TEXT("Arrival time cannot be resolved.");return false;}}else{if(!ResolveTime(Row,I,Departure)){Failure=TEXT("Departure time cannot be resolved.");return false;}int32 StopIndex=INDEX_NONE;for(int32 N=I+1;N<Row.Timeline.Num();++N)if(IsStop(Row.Timeline[N].Action)){StopIndex=N;break;}if(StopIndex==INDEX_NONE||!ResolveTime(Row,StopIndex,Arrival)){Failure=TEXT("No later Stop/Park with a valid arrival time.");return false;}}Duration=Arrival-Departure;if(Duration<=0){Failure=TEXT("Arrival occurs before departure.");return false;}Kmh=(Distance/100000.0)/(Duration/3600.0);return true;
}

bool STMOPVehicleEditor::ResolvePersonTime(
    const FTMOPPersonProfileRow& Person, const int32 Index,
    int32& OutSecond) const
{
    TSet<int32> ResolvingEntries;
    TSet<FName> ResolvingEvents;
    TFunction<bool(FName, int32&)> ResolveEvent;
    ResolveEvent = [&](const FName EventId, int32& Second)
    {
        if (EventId.IsNone() || ResolvingEvents.Contains(EventId) ||
            !EventTable.IsValid()) return false;
        const UDataTable* Events = EventTable.Get();
        const FTMOPHistoricalEventDefinition* Event =
            Events->FindRow<FTMOPHistoricalEventDefinition>(EventId,
                TEXT("VehicleBoardingEvent"), false);
        if (Event == nullptr)
            for (const FName RowName : Events->GetRowNames())
                if (const FTMOPHistoricalEventDefinition* Candidate =
                    Events->FindRow<FTMOPHistoricalEventDefinition>(RowName,
                        TEXT("VehicleBoardingEventById"), false))
                    if (Candidate->EventId == EventId)
                    { Event = Candidate; break; }
        if (Event == nullptr) return false;
        ResolvingEvents.Add(EventId);
        bool bResolved = true;
        if (Event->TimingMode == ETMOPEventTimingMode::Absolute)
            Second = Event->AbsoluteTime.ToSecondsFromMidnight();
        else if (Event->TimingMode == ETMOPEventTimingMode::Window)
            Second = Event->PreferredTime.ToSecondsFromMidnight();
        else if (Event->TimingMode == ETMOPEventTimingMode::Relative)
        {
            int32 TriggerSecond = 0;
            bResolved = ResolveEvent(Event->TriggerEventId, TriggerSecond);
            if (bResolved)
                Second = TriggerSecond + Event->PreferredDelaySeconds;
        }
        else bResolved = false;
        ResolvingEvents.Remove(EventId);
        return bResolved;
    };

    TFunction<bool(int32, int32&)> ResolveEntry;
    ResolveEntry = [&](const int32 At, int32& Second)
    {
        if (!Person.Timeline.IsValidIndex(At) ||
            ResolvingEntries.Contains(At)) return false;
        ResolvingEntries.Add(At);
        const FTMOPPersonTimelineEntry& Entry = Person.Timeline[At];
        bool bResolved = true;
        if (Entry.TimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
        {
            int32 PreviousSecond = 0;
            bResolved = At > 0 && ResolveEntry(At - 1, PreviousSecond);
            if (bResolved) Second = PreviousSecond + Entry.EventOffsetSeconds;
        }
        else if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
        {
            int32 EventSecond = 0;
            bResolved = ResolveEvent(Entry.SharedEventId, EventSecond);
            if (bResolved) Second = EventSecond + Entry.EventOffsetSeconds;
        }
        else Second = Entry.Time.ToSecondsFromMidnight();
        ResolvingEntries.Remove(At);
        return bResolved;
    };
    return ResolveEntry(Index, OutSecond);
}

bool STMOPVehicleEditor::CalculateBoardingFeasibility(
    const FTMOPHistoricalVehicleRow& Vehicle, const int32 DrivingIndex,
    const FTMOPPersonProfileRow& Person,
    FBoardingFeasibility& OutResult) const
{
    OutResult = FBoardingFeasibility();
    if (!Vehicle.Timeline.IsValidIndex(DrivingIndex) ||
        !IsDriving(Vehicle.Timeline[DrivingIndex].Action))
    { OutResult.Failure = TEXT("not a driving segment"); return false; }
    const FTMOPHistoricalVehicleTimelineEntry& Drive =
        Vehicle.Timeline[DrivingIndex];
    double DriveDistance = 0.0, RequiredKmh = 0.0;
    int32 Arrival = 0, Duration = 0;
    if (!CalculateDrive(Vehicle, DrivingIndex, DriveDistance,
        OutResult.DepartureSecond, Arrival, Duration, RequiredKmh,
        OutResult.Failure)) return false;

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World == nullptr)
    { OutResult.Failure = TEXT("open the level to calculate walking paths"); return false; }
    TMap<FName, ATMOPHistoricalAnchor*> Anchors;
    TMap<FName, UTMOPTrafficLaneComponent*> Lanes;
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        if (!It->GetAnchorId().IsNone()) Anchors.Add(It->GetAnchorId(), *It);
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Components;
        It->GetComponents<UTMOPTrafficLaneComponent>(Components);
        for (UTMOPTrafficLaneComponent* Lane : Components)
            if (IsValid(Lane) && !Lane->LaneId.IsNone())
                Lanes.Add(Lane->LaneId, Lane);
    }
    auto AnchorLocation = [&Anchors](const FName Id, const FVector& Offset,
        const ETMOPAnchorOffsetSpace Space, FVector& OutLocation)
    {
        ATMOPHistoricalAnchor* const* Found = Anchors.Find(Id);
        if (Found == nullptr || !IsValid(*Found)) return false;
        OutLocation = (*Found)->GetAnchorLocation();
        OutLocation += Space == ETMOPAnchorOffsetSpace::AnchorLocal
            ? (*Found)->GetActorQuat().RotateVector(Offset) : Offset;
        return true;
    };
    auto VehiclePlacement = [&Vehicle, DrivingIndex, &Anchors, &Lanes,
        &Drive](FVector& OutLocation)
    {
        for (int32 Index = DrivingIndex - 1; Index >= 0; --Index)
        {
            const FTMOPHistoricalVehicleTimelineEntry& Entry =
                Vehicle.Timeline[Index];
            if (!HasPlacement(Entry.Action)) continue;
            if (Entry.PlacementMode ==
                ETMOPHistoricalVehiclePlacementMode::WorldTransform)
            { OutLocation = Entry.WorldTransform.GetLocation(); return true; }
            ATMOPHistoricalAnchor* const* Anchor =
                Anchors.Find(Entry.PlacementAnchorId);
            if (Anchor != nullptr && IsValid(*Anchor))
            {
                OutLocation = (Entry.AnchorLocalOffset * FTransform(
                    (*Anchor)->GetAnchorRotation(),
                    (*Anchor)->GetAnchorLocation())).GetLocation();
                return true;
            }
            break;
        }
        if (ATMOPHistoricalAnchor* const* Anchor =
            Anchors.Find(Drive.RouteStartAnchorId))
            if (IsValid(*Anchor))
            { OutLocation = (*Anchor)->GetAnchorLocation(); return true; }
        if (UTMOPTrafficLaneComponent* const* Lane =
            Lanes.Find(Drive.RouteStartLaneId))
            if (IsValid(*Lane))
            {
                OutLocation = (*Lane)->GetLocationAtDistanceAlongSpline(
                    0.0f, ESplineCoordinateSpace::World);
                return true;
            }
        return false;
    };
    auto SeatLocation = [World](const FName SeatId, const FName VehicleId,
        FVector& OutLocation)
    {
        if (SeatId.IsNone()) return false;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            TArray<UTMOPCinemaSeatComponent*> CinemaSeats;
            It->GetComponents<UTMOPCinemaSeatComponent>(CinemaSeats);
            for (const UTMOPCinemaSeatComponent* Seat : CinemaSeats)
                if (IsValid(Seat) && Seat->SeatId == SeatId)
                { OutLocation = Seat->GetComponentLocation(); return true; }
            TArray<UTMOPVehicleSeatComponent*> VehicleSeats;
            It->GetComponents<UTMOPVehicleSeatComponent>(VehicleSeats);
            const ATMOPVehicleBase* VehicleActor =
                Cast<ATMOPVehicleBase>(*It);
            if (!VehicleId.IsNone() && (!IsValid(VehicleActor) ||
                VehicleActor->VehicleId != VehicleId)) continue;
            for (const UTMOPVehicleSeatComponent* Seat : VehicleSeats)
                if (IsValid(Seat) && Seat->SeatId == SeatId)
                { OutLocation = Seat->GetComponentLocation(); return true; }
        }
        return false;
    };
    auto PersonLocation = [&AnchorLocation, &SeatLocation](
        const FTMOPPersonTimelineEntry& Entry, FVector& OutLocation)
    {
        if (Entry.LocationType == ETMOPPersonLocationType::WorldTransform)
        {
            const bool bPhysical = Entry.Action ==
                ETMOPPersonTimelineAction::InitialPlacement ||
                Entry.Action == ETMOPPersonTimelineAction::Spawn ||
                Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor;
            if (bPhysical)
            { OutLocation = Entry.WorldTransform.GetLocation(); return true; }
        }
        if (Entry.LocationType == ETMOPPersonLocationType::Anchor &&
            !Entry.TargetAnchorId.IsNone() &&
            (Entry.Action == ETMOPPersonTimelineAction::InitialPlacement ||
             Entry.Action == ETMOPPersonTimelineAction::Spawn ||
             Entry.Action == ETMOPPersonTimelineAction::MoveToAnchor))
            return AnchorLocation(Entry.TargetAnchorId, Entry.AnchorOffsetCm,
                Entry.AnchorOffsetSpace, OutLocation);
        return SeatLocation(Entry.TargetSeatId, Entry.TargetEntityId,
            OutLocation);
    };

    int32 BoardingIndex = INDEX_NONE;
    int32 BestDifference = MAX_int32;
    for (int32 Index = 0; Index < Person.Timeline.Num(); ++Index)
    {
        const FTMOPPersonTimelineEntry& Entry = Person.Timeline[Index];
        if (Entry.Action != ETMOPPersonTimelineAction::EnterVehicle ||
            Entry.TargetEntityId != Vehicle.VehicleId) continue;
        int32 Second = 0;
        if (!ResolvePersonTime(Person, Index, Second)) continue;
        const int32 Difference = FMath::Abs(Second - OutResult.DepartureSecond);
        if (Difference < BestDifference)
        { BestDifference = Difference; BoardingIndex = Index;
          OutResult.BoardingSecond = Second; OutResult.SeatId = Entry.TargetSeatId; }
    }
    if (BoardingIndex == INDEX_NONE || BestDifference > 15 * 60 ||
        OutResult.BoardingSecond > OutResult.DepartureSecond + 60)
    { OutResult.Failure = TEXT("no Enter Vehicle entry targets this car"); return false; }
    OutResult.bFoundBoardingEntry = true;

    int32 WalkIndex = INDEX_NONE;
    for (int32 Index = BoardingIndex - 1; Index >= 0; --Index)
        if (Person.Timeline[Index].Action ==
            ETMOPPersonTimelineAction::MoveToAnchor)
        { WalkIndex = Index; break; }
    const int32 SearchBefore = WalkIndex != INDEX_NONE
        ? WalkIndex : BoardingIndex;
    FVector StartLocation;
    int32 StartIndex = INDEX_NONE;
    for (int32 Index = SearchBefore - 1; Index >= 0; --Index)
        if (PersonLocation(Person.Timeline[Index], StartLocation))
        { StartIndex = Index; break; }
    if (StartIndex == INDEX_NONE)
    { OutResult.Failure = TEXT("no earlier physical person position was resolved"); return false; }
    int32 StartSecond = 0;
    if (!ResolvePersonTime(Person, StartIndex, StartSecond))
    { OutResult.Failure = TEXT("the earlier position time could not be resolved"); return false; }
    if (WalkIndex != INDEX_NONE &&
        !Person.Timeline[WalkIndex].bTimeIsArrival &&
        !ResolvePersonTime(Person, WalkIndex, StartSecond))
    { OutResult.Failure = TEXT("the final walk departure time could not be resolved"); return false; }
    FVector CarLocation;
    if (!VehiclePlacement(CarLocation))
    { OutResult.Failure = TEXT("the car's boarding position could not be resolved"); return false; }

    TArray<FVector> RoutePoints;
    if (WalkIndex != INDEX_NONE)
    {
        const FTMOPPersonTimelineEntry& Walk = Person.Timeline[WalkIndex];
        for (const FName ViaId : Walk.PassAnchorIds)
        {
            FVector Point;
            if (!AnchorLocation(ViaId, FVector::ZeroVector,
                ETMOPAnchorOffsetSpace::World, Point))
            { OutResult.Failure = FString::Printf(TEXT("pass anchor '%s' is missing"),
                *ViaId.ToString()); return false; }
            RoutePoints.Add(Point);
        }
        FVector WalkTarget;
        if (!Walk.TargetAnchorId.IsNone() && AnchorLocation(Walk.TargetAnchorId,
            Walk.AnchorOffsetCm, Walk.AnchorOffsetSpace, WalkTarget))
            RoutePoints.Add(WalkTarget);
    }
    RoutePoints.Add(CarLocation);

    FVector SegmentStart = StartLocation;
    for (const FVector& SegmentEnd : RoutePoints)
    {
        double SegmentDistance = FVector::Dist2D(SegmentStart, SegmentEnd);
        double NavDistance = SegmentDistance;
        const ENavigationQueryResult::Type Result =
            UNavigationSystemV1::GetPathLength(World, SegmentStart, SegmentEnd,
                NavDistance, nullptr, nullptr);
        if (Result == ENavigationQueryResult::Success &&
            NavDistance > KINDA_SMALL_NUMBER)
            SegmentDistance = NavDistance;
        else if (SegmentDistance > 100.0)
            OutResult.bUsedStraightLineFallback = true;
        OutResult.DistanceCm += SegmentDistance;
        SegmentStart = SegmentEnd;
    }
    const FTMOPMovementProfile& Movement = Person.MovementProfile;
    float ActivitySpeed = Movement.NormalWalkSpeed;
    if (WalkIndex != INDEX_NONE)
    {
        const FTMOPPersonTimelineEntry& Walk = Person.Timeline[WalkIndex];
        switch (Walk.ActivityState)
        {
        case ETMOPAgentActivityState::FastWalking:
            ActivitySpeed = Movement.FastWalkSpeed; break;
        case ETMOPAgentActivityState::Jogging:
            ActivitySpeed = Movement.JogSpeed; break;
        case ETMOPAgentActivityState::Running:
        case ETMOPAgentActivityState::Fleeing:
            ActivitySpeed = Movement.RunSpeed; break;
        case ETMOPAgentActivityState::Sprinting:
            ActivitySpeed = Movement.SprintSpeed; break;
        default: break;
        }
        if (Walk.TravelSpeedOverrideCmPerSecond > 0.0f)
            ActivitySpeed = Walk.TravelSpeedOverrideCmPerSecond /
                FMath::Max(Movement.PersonalSpeedMultiplier, KINDA_SMALL_NUMBER);
    }
    OutResult.SpeedCmPerSecond = ActivitySpeed * FMath::Max(
        Movement.PersonalSpeedMultiplier, KINDA_SMALL_NUMBER);
    if (OutResult.SpeedCmPerSecond <= KINDA_SMALL_NUMBER)
    { OutResult.Failure = TEXT("walking speed is zero"); return false; }
    const int32 Deadline = FMath::Min(OutResult.BoardingSecond,
        OutResult.DepartureSecond) - FMath::Max(0, Drive.BoardingBufferSeconds);
    OutResult.AvailableSeconds = Deadline - StartSecond;
    OutResult.RequiredSeconds = FMath::CeilToInt(
        OutResult.DistanceCm / OutResult.SpeedCmPerSecond);
    OutResult.MarginSeconds = OutResult.AvailableSeconds -
        OutResult.RequiredSeconds;
    if (OutResult.BoardingSecond > OutResult.DepartureSecond)
        OutResult.MarginSeconds = FMath::Min(OutResult.MarginSeconds,
            OutResult.DepartureSecond - OutResult.BoardingSecond);
    OutResult.bRouteResolved = true;
    return true;
}

TArray<FString> STMOPVehicleEditor::ValidateRow(const FTMOPHistoricalVehicleRow& Row)const
{
    TArray<FString> Results;
    if (Row.VehicleId.IsNone()) Results.Add(TEXT("ERROR: Vehicle ID is empty."));
    if (Row.bSpawnInSimulation && Row.Timeline.IsEmpty())
        Results.Add(TEXT("ERROR: Spawned vehicle has no timeline."));

    TMap<FName, UTMOPTrafficLaneComponent*> Lanes;
    TSet<FName> KnownAnchors;
    if (UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
    {
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
            if (!It->GetAnchorId().IsNone()) KnownAnchors.Add(It->GetAnchorId());
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Components;
            It->GetComponents<UTMOPTrafficLaneComponent>(Components);
            for (UTMOPTrafficLaneComponent* Component : Components)
                if (IsValid(Component) && !Component->LaneId.IsNone())
                    Lanes.Add(Component->LaneId, Component);
        }
    }

    TSet<FName> EntryIds;
    for (int32 Index = 0; Index < Row.Timeline.Num(); ++Index)
    {
        const FTMOPHistoricalVehicleTimelineEntry& Entry = Row.Timeline[Index];
        if (Entry.EntryId.IsNone())
            Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: Entry ID is empty."), Index));
        else if (EntryIds.Contains(Entry.EntryId))
            Results.Add(FString::Printf(TEXT("ERROR: Duplicate Entry ID '%s'."),
                *Entry.EntryId.ToString()));
        EntryIds.Add(Entry.EntryId);

        if ((Entry.Action == ETMOPHistoricalVehicleAction::InitialPlacement ||
             Entry.Action == ETMOPHistoricalVehicleAction::Spawn || IsStop(Entry.Action)) &&
            Entry.PlacementMode == ETMOPHistoricalVehiclePlacementMode::Anchor &&
            Entry.PlacementAnchorId.IsNone())
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: placement anchor is missing."), Index));
        else if (Entry.PlacementMode == ETMOPHistoricalVehiclePlacementMode::Anchor &&
            !Entry.PlacementAnchorId.IsNone() &&
            !KnownAnchors.Contains(Entry.PlacementAnchorId))
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: anchor '%s' is not present in the open level."),
                Index, *Entry.PlacementAnchorId.ToString()));

        int32 ResolvedSecond = INDEX_NONE;
        FString TimingFailure;
        if (!ResolveTime(Row, Index, ResolvedSecond, &TimingFailure))
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: %s"), Index, *TimingFailure));
        else if (Index > 0)
        {
            int32 PreviousSecond = INDEX_NONE;
            if (ResolveTime(Row, Index - 1, PreviousSecond) &&
                ResolvedSecond < PreviousSecond)
                Results.Add(FString::Printf(
                    TEXT("ERROR Timeline[%d]: resolved time is before the previous entry."),
                    Index));
        }

        if (!IsDriving(Entry.Action)) continue;
        if (Entry.bTimeIsArrival && Index == 0)
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: Time Is Arrival needs a previous departure entry."),
                Index));
        if (!Entry.RouteStartAnchorId.IsNone() &&
            !KnownAnchors.Contains(Entry.RouteStartAnchorId))
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: start anchor '%s' is missing."),
                Index, *Entry.RouteStartAnchorId.ToString()));
        if (!Entry.RouteDestinationAnchorId.IsNone() &&
            !KnownAnchors.Contains(Entry.RouteDestinationAnchorId))
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: destination anchor '%s' is missing."),
                Index, *Entry.RouteDestinationAnchorId.ToString()));
        for (const FName AnchorId : Entry.RouteViaAnchorIds)
            if (!KnownAnchors.Contains(AnchorId))
                Results.Add(FString::Printf(
                    TEXT("ERROR Timeline[%d]: via anchor '%s' is missing."),
                    Index, *AnchorId.ToString()));
        TArray<FName> ReferencedLanes = Entry.RouteViaLaneIds;
        ReferencedLanes.Add(Entry.RouteStartLaneId);
        ReferencedLanes.Add(Entry.RouteDestinationLaneId);
        for (const FName LaneId : ReferencedLanes)
            if (!LaneId.IsNone() && !Lanes.Contains(LaneId))
                Results.Add(FString::Printf(
                    TEXT("ERROR Timeline[%d]: route lane '%s' is missing."),
                    Index, *LaneId.ToString()));
        if (Entry.OrderedLaneIds.IsEmpty())
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: driving route has no lanes."), Index));

        for (int32 LaneIndex = 0; LaneIndex + 1 < Entry.OrderedLaneIds.Num(); ++LaneIndex)
        {
            UTMOPTrafficLaneComponent* const* Lane =
                Lanes.Find(Entry.OrderedLaneIds[LaneIndex]);
            if (!Lane || !IsValid(*Lane)) continue;
            const FName NextId = Entry.OrderedLaneIds[LaneIndex + 1];
            const bool bConnected = (*Lane)->NextLanes.ContainsByPredicate(
                [NextId, &Entry](const FTMOPLaneConnection& Connection)
                { return (Connection.bAllowed || Entry.bIgnoreOneWayRestrictions) &&
                    Connection.TargetLaneId == NextId; });
            if (!bConnected)
                Results.Add(FString::Printf(
                    TEXT("ERROR Timeline[%d]: lane '%s' does not connect to '%s'."),
                    Index, *Entry.OrderedLaneIds[LaneIndex].ToString(),
                    *NextId.ToString()));
        }

        double Distance = 0.0, Kmh = 0.0;
        int32 Departure = 0, Arrival = 0, Duration = 0;
        FString Failure;
        if (!CalculateDrive(Row, Index, Distance, Departure, Arrival,
            Duration, Kmh, Failure))
            Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: %s"),
                Index, *Failure));
        else if (Kmh > 90.0)
            Results.Add(FString::Printf(
                TEXT("WARNING Timeline[%d]: required average speed %.1f km/h is very high."),
                Index, Kmh));
        else if (Kmh > 50.0)
            Results.Add(FString::Printf(
                TEXT("WARNING Timeline[%d]: required average speed is %.1f km/h."),
                Index, Kmh));
        const float SelectedSpeed = Entry.CruiseSpeedOverrideKmh > 0.0f
            ? Entry.CruiseSpeedOverrideKmh
            : DrivingPresetSpeedKmh(Entry.DrivingPreset);
        if (SelectedSpeed > 0.0f && Distance > 0.0 && Arrival > Departure)
        {
            const int32 EstimatedArrival = Departure + FMath::RoundToInt(
                (Distance / 100000.0) / (SelectedSpeed / 3600.0));
            const int32 Difference = EstimatedArrival - Arrival;
            if (FMath::Abs(Difference) > 10)
                Results.Add(FString::Printf(
                    TEXT("WARNING Timeline[%d]: selected %.1f km/h estimates arrival %s (%+d s versus plan)."),
                    Index, SelectedSpeed, *FormatClockSecond(EstimatedArrival),
                    Difference));
        }

        if (!PeopleTable.IsValid() || Departure <= 0 || Arrival <= Departure)
            continue;
        TMap<FName, FName> SeatOwners;
        TSet<FName> PeopleInside;
        TArray<FName> DriversAtDeparture;
        const UDataTable* People = PeopleTable.Get();
        for (const FName PersonRowName : People->GetRowNames())
        {
            const FTMOPPersonProfileRow* Person =
                People->FindRow<FTMOPPersonProfileRow>(PersonRowName,
                    TEXT("VehicleEditorOccupantValidation"), false);
            if (Person == nullptr) continue;
            const FName PersonId = Person->EntityId.IsNone()
                ? PersonRowName : Person->EntityId;
            bool bInside = false;
            bool bTargetsVehicleBoarding = false;
            FName SeatId = NAME_None;
            for (int32 PersonIndex = 0;
                PersonIndex < Person->Timeline.Num(); ++PersonIndex)
            {
                const FTMOPPersonTimelineEntry& PersonEntry =
                    Person->Timeline[PersonIndex];
                int32 PersonSecond = INDEX_NONE;
                if (!ResolvePersonTime(*Person, PersonIndex, PersonSecond))
                    continue;
                if (PersonEntry.TargetEntityId != Row.VehicleId) continue;
                if (PersonEntry.Action == ETMOPPersonTimelineAction::EnterVehicle)
                    bTargetsVehicleBoarding = true;
                if (PersonEntry.Action == ETMOPPersonTimelineAction::BeginDriving &&
                    FMath::Abs(PersonSecond - Departure) <= 10)
                {
                    DriversAtDeparture.AddUnique(PersonId);
                    if (!PersonEntry.OrderedLaneIds.IsEmpty())
                        Results.Add(FString::Printf(
                            TEXT("WARNING Timeline[%d]: person '%s' has legacy lane IDs; the vehicle segment is authoritative."),
                            Index, *PersonId.ToString()));
                    if (!PersonEntry.DrivingDestinationAnchorId.IsNone() &&
                        !Entry.RouteDestinationAnchorId.IsNone() &&
                        PersonEntry.DrivingDestinationAnchorId !=
                            Entry.RouteDestinationAnchorId)
                        Results.Add(FString::Printf(
                            TEXT("WARNING Timeline[%d]: person '%s' has a conflicting driving destination; the vehicle destination wins."),
                            Index, *PersonId.ToString()));
                }
                if (PersonEntry.Action == ETMOPPersonTimelineAction::ExitVehicle)
                {
                    if (PersonSecond > Departure && PersonSecond < Arrival)
                        Results.Add(FString::Printf(
                            TEXT("WARNING Timeline[%d]: '%s' exits at %s while the segment is still driving."),
                            Index, *PersonId.ToString(),
                            *FormatClockSecond(PersonSecond)));
                    if (PersonSecond <= Departure) { bInside = false; SeatId = NAME_None; }
                }
                else if (PersonSecond <= Departure &&
                    (PersonEntry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
                     PersonEntry.LocationType == ETMOPPersonLocationType::VehicleSeat))
                {
                    bInside = true;
                    SeatId = PersonEntry.TargetSeatId;
                }
            }
            FBoardingFeasibility Boarding;
            if (bTargetsVehicleBoarding &&
                CalculateBoardingFeasibility(Row, Index, *Person, Boarding))
            {
                if (Boarding.MarginSeconds < -10 &&
                    !Boarding.bUsedStraightLineFallback)
                    Results.Add(FString::Printf(
                        TEXT("ERROR Timeline[%d]: '%s' cannot reach the car by departure; needs %d s, has %d s (%d s late)."),
                        Index, *PersonId.ToString(), Boarding.RequiredSeconds,
                        Boarding.AvailableSeconds, -Boarding.MarginSeconds));
                else if (Boarding.MarginSeconds < 0)
                    Results.Add(FString::Printf(
                        TEXT("WARNING Timeline[%d]: '%s' is estimated %d s late for boarding%s."),
                        Index, *PersonId.ToString(), -Boarding.MarginSeconds,
                        Boarding.bUsedStraightLineFallback
                            ? TEXT(" (straight-line fallback; check NavMesh)") : TEXT("")));
                else if (Boarding.MarginSeconds < 10)
                    Results.Add(FString::Printf(
                        TEXT("WARNING Timeline[%d]: '%s' has only %d s boarding margin."),
                        Index, *PersonId.ToString(), Boarding.MarginSeconds));
            }
            else if (Boarding.bFoundBoardingEntry)
                Results.Add(FString::Printf(
                    TEXT("WARNING Timeline[%d]: boarding feasibility for '%s' could not be calculated: %s."),
                    Index, *PersonId.ToString(), *Boarding.Failure));
            if (!bInside) continue;
            PeopleInside.Add(PersonId);
            if (!SeatId.IsNone())
            {
                if (const FName* Existing = SeatOwners.Find(SeatId))
                    Results.Add(FString::Printf(
                        TEXT("WARNING Timeline[%d]: seat '%s' is assigned to both '%s' and '%s'."),
                        Index, *SeatId.ToString(), *Existing->ToString(),
                        *PersonId.ToString()));
                else SeatOwners.Add(SeatId, PersonId);
            }
        }
        FName RequiredDriver = Entry.DriverEntityId;
        if (RequiredDriver.IsNone()) RequiredDriver = Row.KnownDriverEntityId;
        if (!RequiredDriver.IsNone() && !PeopleInside.Contains(RequiredDriver))
            Results.Add(FString::Printf(
                TEXT("WARNING Timeline[%d]: driver '%s' is not inside the vehicle at departure %s."),
                Index, *RequiredDriver.ToString(), *FormatClockSecond(Departure)));
        if (!RequiredDriver.IsNone() && !DriversAtDeparture.IsEmpty() &&
            !DriversAtDeparture.Contains(RequiredDriver))
            Results.Add(FString::Printf(
                TEXT("WARNING Timeline[%d]: Begin Driving belongs to another person, expected '%s'."),
                Index, *RequiredDriver.ToString()));
        for (const FName PassengerId : Entry.PassengerEntityIds)
            if (!PassengerId.IsNone() && !PeopleInside.Contains(PassengerId))
                Results.Add(FString::Printf(
                    TEXT("WARNING Timeline[%d]: listed passenger '%s' is not seated at departure."),
                    Index, *PassengerId.ToString()));
    }
    return Results;
}

void STMOPVehicleEditor::RebuildValidation(){CommitEntry();CurrentErrors=ValidateRow(WorkingRow);}

void STMOPVehicleEditor::RebuildRoutePreview()
{
    if (!RouteMap.IsValid()) return;
    TArray<TArray<FVector2D>> Network;
    TArray<FName> NetworkLaneIds;
    TArray<TArray<FVector2D>> Route;
    TMap<FName, TArray<FVector2D>> LinesByLaneId;
    TMap<FName, FTransform> AnchorTransforms;
    TOptional<FVector2D> StartAnchor;
    TOptional<FVector2D> EndAnchor;
    TOptional<FVector2D> SelectedPlacement;
    FString Caption = TEXT("No route selected");

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (World)
    {
        for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
        {
            AnchorTransforms.Add(It->GetAnchorId(), FTransform(
                It->GetAnchorRotation(), It->GetAnchorLocation(),
                FVector::OneVector));
        }
        auto ResolvePlacementPoint = [&AnchorTransforms](
            const FTMOPHistoricalVehicleTimelineEntry& Entry,
            FVector2D& OutPoint)
        {
            if (Entry.PlacementMode ==
                ETMOPHistoricalVehiclePlacementMode::WorldTransform)
            {
                const FVector Location = Entry.WorldTransform.GetLocation();
                OutPoint = FVector2D(Location.X, Location.Y);
                return true;
            }
            const FTransform* AnchorTransform =
                AnchorTransforms.Find(Entry.PlacementAnchorId);
            if (AnchorTransform == nullptr) return false;
            const FVector Location =
                (Entry.AnchorLocalOffset * *AnchorTransform).GetLocation();
            OutPoint = FVector2D(Location.X, Location.Y);
            return true;
        };
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Components;
            It->GetComponents<UTMOPTrafficLaneComponent>(Components);
            for (UTMOPTrafficLaneComponent* Component : Components)
            {
                if (!IsValid(Component)) continue;
                TArray<FVector2D> Line;
                const int32 Samples = FMath::Max(2,
                    FMath::CeilToInt(Component->GetSplineLength() / 500.0f));
                for (int32 Sample = 0; Sample <= Samples; ++Sample)
                {
                    const FVector Point = Component->GetLocationAtDistanceAlongSpline(
                        Component->GetSplineLength() * Sample / Samples,
                        ESplineCoordinateSpace::World);
                    Line.Add(FVector2D(Point.X, Point.Y));
                }
                Network.Add(Line);
                NetworkLaneIds.Add(Component->LaneId);
                if (!Component->LaneId.IsNone())
                    LinesByLaneId.Add(Component->LaneId, MoveTemp(Line));
            }
        }

        int32 DriveIndex = SelectedTimelineIndex;
        if (WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        {
            const FTMOPHistoricalVehicleTimelineEntry& Selected =
                WorkingRow.Timeline[SelectedTimelineIndex];
            const bool bHasPlacement = HasPlacement(Selected.Action);
            FVector2D Point;
            if (bHasPlacement && ResolvePlacementPoint(Selected, Point))
                SelectedPlacement = Point;
        }
        if (WorkingRow.Timeline.IsValidIndex(DriveIndex) &&
            !IsDriving(WorkingRow.Timeline[DriveIndex].Action))
            for (int32 Index = DriveIndex - 1; Index >= 0; --Index)
                if (IsDriving(WorkingRow.Timeline[Index].Action))
                { DriveIndex = Index; break; }
        if (WorkingRow.Timeline.IsValidIndex(DriveIndex) &&
            IsDriving(WorkingRow.Timeline[DriveIndex].Action))
        {
            const FTMOPHistoricalVehicleTimelineEntry& DriveEntry =
                WorkingRow.Timeline[DriveIndex];
            for (const FName LaneId : DriveEntry.OrderedLaneIds)
                if (const TArray<FVector2D>* Line = LinesByLaneId.Find(LaneId))
                    Route.Add(*Line);
            if (const FTransform* Transform =
                AnchorTransforms.Find(DriveEntry.RouteStartAnchorId))
            {
                const FVector Location = Transform->GetLocation();
                StartAnchor = FVector2D(Location.X, Location.Y);
            }
            else if (const TArray<FVector2D>* Line =
                LinesByLaneId.Find(DriveEntry.RouteStartLaneId))
                if (!Line->IsEmpty()) StartAnchor = (*Line)[0];
            if (!StartAnchor.IsSet())
                for (int32 Index = DriveIndex - 1; Index >= 0; --Index)
                {
                    if (!HasPlacement(WorkingRow.Timeline[Index].Action))
                        continue;
                    FVector2D Point;
                    if (ResolvePlacementPoint(
                            WorkingRow.Timeline[Index], Point))
                    { StartAnchor = Point; break; }
                }
            if (const FTransform* Transform =
                AnchorTransforms.Find(DriveEntry.RouteDestinationAnchorId))
            {
                const FVector Location = Transform->GetLocation();
                EndAnchor = FVector2D(Location.X, Location.Y);
            }
            else if (const TArray<FVector2D>* Line =
                LinesByLaneId.Find(DriveEntry.RouteDestinationLaneId))
                if (!Line->IsEmpty()) EndAnchor = Line->Last();
            if (!EndAnchor.IsSet())
                for (int32 Index = DriveIndex + 1;
                    Index < WorkingRow.Timeline.Num(); ++Index)
                    if (IsStop(WorkingRow.Timeline[Index].Action))
                    {
                        FVector2D Point;
                        if (ResolvePlacementPoint(
                                WorkingRow.Timeline[Index], Point))
                        { EndAnchor = Point; break; }
                    }
        }

        Caption = FString::Printf(
            TEXT("%d lanes • Shift+click=start • Ctrl+click=end • right-click=via"),
            Route.Num());
    }
    RouteMap->SetRoute(MoveTemp(Network), MoveTemp(NetworkLaneIds),
        MoveTemp(Route), Caption,
        StartAnchor, EndAnchor, SelectedPlacement);
}

FString STMOPVehicleEditor::BuildOccupantsText(const int32 TimelineIndex)const
{
    if (!WorkingRow.Timeline.IsValidIndex(TimelineIndex) ||
        !PeopleTable.IsValid()) return TEXT("Select a timeline event.");
    const bool bDriving = IsDriving(WorkingRow.Timeline[TimelineIndex].Action);
    int32 At = 0;
    if (bDriving)
    {
        double Distance = 0.0, Kmh = 0.0;
        int32 Arrival = 0, Duration = 0;
        FString Failure;
        if (!CalculateDrive(WorkingRow, TimelineIndex, Distance, At,
            Arrival, Duration, Kmh, Failure))
            return TEXT("Departure unavailable: ") + Failure;
    }
    else if (!ResolveTime(WorkingRow, TimelineIndex, At))
        return TEXT("Event time unavailable.");

    TArray<FString> Lines;
    TSet<FName> ShownPeople;
    const UDataTable* Table = PeopleTable.Get();
    for (const FName RowName : Table->GetRowNames())
    {
        const FTMOPPersonProfileRow* Person =
            Table->FindRow<FTMOPPersonProfileRow>(RowName,
                TEXT("VehicleEditorOccupants"), false);
        if (Person == nullptr) continue;
        const FName PersonId = Person->EntityId.IsNone()
            ? RowName : Person->EntityId;
        const FString PersonName = Person->FullName.IsEmpty()
            ? PersonId.ToString() : Person->FullName.ToString();
        bool bInside = false;
        bool bHasBoarding = false;
        FName SeatId = NAME_None;
        for (int32 Index = 0; Index < Person->Timeline.Num(); ++Index)
        {
            const FTMOPPersonTimelineEntry& Entry = Person->Timeline[Index];
            if (Entry.TargetEntityId != WorkingRow.VehicleId) continue;
            int32 Second = 0;
            if (!ResolvePersonTime(*Person, Index, Second)) continue;
            if (Entry.Action == ETMOPPersonTimelineAction::EnterVehicle)
                bHasBoarding = true;
            if (Second > At) continue;
            if (Entry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
                Entry.LocationType == ETMOPPersonLocationType::VehicleSeat)
            { bInside = true; SeatId = Entry.TargetSeatId; }
            if (Entry.Action == ETMOPPersonTimelineAction::ExitVehicle)
            { bInside = false; SeatId = NAME_None; }
        }
        FString FeasibilityText;
        if (bDriving && bHasBoarding)
        {
            FBoardingFeasibility Boarding;
            if (CalculateBoardingFeasibility(
                WorkingRow, TimelineIndex, *Person, Boarding))
            {
                const FString State = Boarding.MarginSeconds < 0
                    ? FString::Printf(TEXT("LATE %d s"), -Boarding.MarginSeconds)
                    : Boarding.MarginSeconds < 10
                        ? FString::Printf(TEXT("TIGHT +%d s"), Boarding.MarginSeconds)
                        : FString::Printf(TEXT("OK +%d s"), Boarding.MarginSeconds);
                FeasibilityText = FString::Printf(
                    TEXT("  •  %s  •  walk %.0f m, needs %d s / has %d s%s"),
                    *State, Boarding.DistanceCm / 100.0,
                    Boarding.RequiredSeconds, Boarding.AvailableSeconds,
                    Boarding.bUsedStraightLineFallback
                        ? TEXT("  •  CHECK NAVMESH") : TEXT(""));
            }
            else if (Boarding.bFoundBoardingEntry)
                FeasibilityText = TEXT("  •  CHECK: ") + Boarding.Failure;
        }
        if (bInside || !FeasibilityText.IsEmpty())
        {
            Lines.Add(FString::Printf(TEXT("%s  •  %s%s"), *PersonName,
                SeatId.IsNone() ? TEXT("seat not set") : *SeatId.ToString(),
                *FeasibilityText));
            ShownPeople.Add(PersonId);
        }
    }
    if (bDriving)
    {
        const FTMOPHistoricalVehicleTimelineEntry& Drive =
            WorkingRow.Timeline[TimelineIndex];
        TArray<FName> Expected = Drive.PassengerEntityIds;
        if (!Drive.DriverEntityId.IsNone()) Expected.AddUnique(Drive.DriverEntityId);
        for (const FName PersonId : Expected)
            if (!PersonId.IsNone() && !ShownPeople.Contains(PersonId))
                Lines.Add(FString::Printf(TEXT("%s  •  MISSING AT DEPARTURE"),
                    *PersonId.ToString()));
    }
    return Lines.IsEmpty()
        ? TEXT("No occupants resolved from DT_TMOP_People.")
        : FString::Join(Lines, TEXT("\n"));
}

FText STMOPVehicleEditor::GetTitle()const{return SelectedRowName.IsNone()?LOCTEXT("Title","TMOP Vehicle Editor"):(!WorkingRow.DisplayName.IsEmpty()?WorkingRow.DisplayName:FText::FromName(WorkingRow.VehicleId));}
FText STMOPVehicleEditor::GetSubtitle()const{return FText::FromString(WorkingRow.VehicleId.ToString()+FString::Printf(TEXT(" • %d timeline entries"),WorkingRow.Timeline.Num()));}
FText STMOPVehicleEditor::GetValidationText()const{return CurrentErrors.IsEmpty()?LOCTEXT("NoErrors","No errors detected."):FText::FromString(FString::Join(CurrentErrors,TEXT("\n")));}
void STMOPVehicleEditor::SetStatus(const FText&Text,const FLinearColor&Color){if(StatusText.IsValid()){StatusText->SetText(Text);StatusText->SetColorAndOpacity(Color);}}

#undef LOCTEXT_NAMESPACE
