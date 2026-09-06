#include "STMOPVehicleEditor.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "STMOPAppearancePreview.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Vehicles/TMOPVehicleTimeline.h"
#include "TMOPRuntimeValidationReader.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "DrawDebugHelpers.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "TMOPVehicleEditorObjects.h"
#include "IDetailsView.h"
#include "FileHelpers.h"
#include "ScopedTransaction.h"
#include "Templates/UnrealTemplate.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "InputCoreTypes.h"
#include "Misc/MessageDialog.h"
#include "NavigationSystem.h"
#include "People/TMOPPersonProfileTypes.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "Styling/AppStyle.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "UObject/StructOnScope.h"
#include "UObject/UnrealType.h"
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
#include "Vehicles/TMOPHistoricalVehicleDirector.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "Events/TMOPHistoricalEventDirector.h"
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
            Action == ETMOPHistoricalVehicleAction::Park ||
            Action == ETMOPHistoricalVehicleAction::OffscreenTransfer;
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


}

class STMOPVehicleRouteMap final : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPVehicleRouteMap) {}
    SLATE_END_ARGS()
    void Construct(const FArguments&) {}
    void SetRoute(TArray<TArray<FVector2D>>&& InNetwork, TArray<FName>&& InIds,
        TArray<TArray<FVector2D>>&& InRoute, const FString& InCaption,
        const TOptional<FVector2D>& InStart = TOptional<FVector2D>(), const TOptional<FVector2D>& InEnd = TOptional<FVector2D>(),
        const TOptional<FVector2D>& InPlacement = TOptional<FVector2D>())
    {
        Network = MoveTemp(InNetwork); NetworkIds = MoveTemp(InIds); Route = MoveTemp(InRoute);
        Caption = InCaption; Start = InStart; End = InEnd; Placement = InPlacement;
        Invalidate(EInvalidateWidgetReason::Paint);
    }
    void SetPlan(const FTMOPVehicleRoutePlan& Plan)
    {
        StartName = Plan.StartAnchorId.ToString(); EndName = Plan.EndAnchorId.ToString();
        if (!Plan.Samples.IsEmpty()) { StartPose = Plan.Samples[0]; EndPose = Plan.Samples.Last(); }
        bHasPlan = !Plan.Samples.IsEmpty();
    }
    void SetGhost(const FTransform& Pose)
    { Ghost = Pose; Invalidate(EInvalidateWidgetReason::Paint); }
    void Fit() { Zoom = 1.0; Pan = FVector2D::ZeroVector; Invalidate(EInvalidateWidgetReason::Paint); }
    void SetLaneClickedHandler(TFunction<void(FName, bool, bool, bool)>&& Handler)
    { LaneClicked = MoveTemp(Handler); }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(520, 330); }
    FBox2D Bounds() const
    {
        FBox2D Box(ForceInit);
        for (const auto& Line : Route.IsEmpty() ? Network : Route)
            for (const auto& Point : Line) Box += Point;
        if (Start.IsSet()) Box += Start.GetValue();
        if (End.IsSet()) Box += End.GetValue();
        if (Placement.IsSet()) Box += Placement.GetValue();
        return Box;
    }
    FVector2D Project(const FVector2D& Point, const FVector2D& Size) const
    {
        const FBox2D Box = Bounds();
        if (!Box.bIsValid) return Size * 0.5;
        const FVector2D Extent = Box.GetSize();
        const double Scale = FMath::Max(0.001, FMath::Min(
            (Size.X - 60.0) / FMath::Max(Extent.X, 1.0),
            (Size.Y - 70.0) / FMath::Max(Extent.Y, 1.0))) * Zoom;
        return (Point - Box.GetCenter()) * Scale + Size * 0.5 + Pan;
    }
    virtual FReply OnMouseWheel(const FGeometry& Geometry, const FPointerEvent& Event) override
    {
        const double OldZoom = Zoom;
        Zoom = FMath::Clamp(Zoom * FMath::Pow(1.2, Event.GetWheelDelta()), 0.15, 30.0);
        const FVector2D Mouse = Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition()) - Geometry.GetLocalSize() * 0.5;
        Pan = Mouse - (Mouse - Pan) * (Zoom / OldZoom);
        Invalidate(EInvalidateWidgetReason::Paint); return FReply::Handled();
    }
    virtual FReply OnMouseButtonDown(const FGeometry& Geometry, const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() == EKeys::MiddleMouseButton)
        { bPanning = true; return FReply::Handled().CaptureMouse(SharedThis(this)); }
        if (!LaneClicked || Network.Num() != NetworkIds.Num()) return FReply::Unhandled();
        const bool bVia = Event.GetEffectingButton() == EKeys::RightMouseButton;
        if (!Event.IsShiftDown() && !Event.IsControlDown() && !bVia) return FReply::Unhandled();
        const FVector2D Mouse = Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
        int32 ClosestIndex = INDEX_NONE; double Distance = 196.0;
        for (int32 LineIndex = 0; LineIndex < Network.Num(); ++LineIndex)
            for (int32 PointIndex = 1; PointIndex < Network[LineIndex].Num(); ++PointIndex)
            {
                const FVector2D Closest = FMath::ClosestPointOnSegment2D(Mouse,
                    Project(Network[LineIndex][PointIndex - 1], Geometry.GetLocalSize()),
                    Project(Network[LineIndex][PointIndex], Geometry.GetLocalSize()));
                const double Candidate = FVector2D::DistSquared(Mouse, Closest);
                if (Candidate < Distance) { Distance = Candidate; ClosestIndex = LineIndex; }
            }
        if (NetworkIds.IsValidIndex(ClosestIndex))
            LaneClicked(NetworkIds[ClosestIndex], Event.IsShiftDown(), Event.IsControlDown(), bVia);
        return FReply::Handled();
    }
    virtual FReply OnMouseMove(const FGeometry&, const FPointerEvent& Event) override
    {
        if (!bPanning || !HasMouseCapture()) return FReply::Unhandled();
        Pan += Event.GetCursorDelta(); Invalidate(EInvalidateWidgetReason::Paint); return FReply::Handled();
    }
    virtual FReply OnMouseButtonUp(const FGeometry&, const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() != EKeys::MiddleMouseButton) return FReply::Unhandled();
        bPanning = false; return FReply::Handled().ReleaseMouseCapture();
    }
    virtual int32 OnPaint(const FPaintArgs&, const FGeometry& Geometry, const FSlateRect&,
        FSlateWindowElementList& Elements, int32 Layer, const FWidgetStyle&, bool) const override
    {
        const FVector2D Size = Geometry.GetLocalSize();
        FSlateDrawElement::MakeBox(Elements, Layer, Geometry.ToPaintGeometry(),
            FAppStyle::GetBrush("Brushes.Recessed"), ESlateDrawEffect::None, FLinearColor(0.018f,0.022f,0.028f));
        auto Label = [&](const FVector2D& At, const FString& Text, FLinearColor Color)
        {
            FSlateDrawElement::MakeText(Elements, Layer + 4,
                Geometry.ToPaintGeometry(At, FVector2D(300, 18)), FText::FromString(Text),
                FAppStyle::GetFontStyle("SmallFont"), ESlateDrawEffect::None, Color);
        };
        auto Line = [&](const TArray<FVector2D>& Points, FLinearColor Color, float Width)
        {
            if (Points.Num() > 1) FSlateDrawElement::MakeLines(Elements, Layer + 2,
                Geometry.ToPaintGeometry(), Points, ESlateDrawEffect::None, Color, true, Width);
        };
        for (const auto& Points : Network)
        {
            TArray<FVector2D> Screen;
            for (const auto& Point : Points) Screen.Add(Project(Point, Size));
            Line(Screen, FLinearColor(0.18f,0.22f,0.27f), 1.0f);
        }
        for (const auto& Points : Route)
        {
            TArray<FVector2D> Screen;
            for (const auto& Point : Points) Screen.Add(Project(Point, Size));
            Line(Screen, FLinearColor(0.05f,0.55f,1.0f), 3.0f);
        }
        auto Marker = [&](const FTransform& Pose, const FString& Name, FLinearColor Color, bool bCar)
        {
            const FVector2D Center = Project(FVector2D(Pose.GetLocation()), Size);
            const FVector2D Forward = FVector2D(Pose.GetRotation().GetForwardVector()).GetSafeNormal();
            const FVector2D Right(-Forward.Y, Forward.X);
            TArray<FVector2D> Arrow = { Center - Forward * 8.0 + Right * 6.0,
                Center + Forward * 12.0, Center - Forward * 8.0 - Right * 6.0 };
            if (bCar)
            {
                // Add must not receive a reference into the array being modified.
                const FVector2D ClosingPoint = Arrow[0];
                Arrow.Add(ClosingPoint);
            }
            Line(Arrow, Color, bCar ? 3.0f : 2.0f);
            if (!Name.IsEmpty()) Label(Center + FVector2D(12,12), Name, Color);
        };
        if (bHasPlan)
        {
            Marker(StartPose, TEXT("FROM: ") + StartName, FLinearColor::Green, false);
            Marker(EndPose, TEXT("TO: ") + EndName, FLinearColor::Yellow, false);
            Marker(Ghost, TEXT(""), FLinearColor::White, true);
        }
        if (Placement.IsSet())
        {
            const FVector2D Point = Project(Placement.GetValue(), Size);
            FSlateDrawElement::MakeBox(Elements, Layer + 3,
                Geometry.ToPaintGeometry(Point - FVector2D(6), FVector2D(12)),
                FAppStyle::GetBrush("WhiteBrush"), ESlateDrawEffect::None, FLinearColor(0,0.85f,1));
        }
        Label(FVector2D(8,6), Caption, FLinearColor::White);
        Label(FVector2D(8, Size.Y-23), TEXT("Green=start  Yellow=end  White=preview | Wheel: zoom  Middle drag: pan"), FLinearColor(0.8f,0.8f,0.8f));
        return Layer + 4;
    }
private:
    TArray<TArray<FVector2D>> Network, Route;
    TArray<FName> NetworkIds;
    FString Caption, StartName, EndName;
    TOptional<FVector2D> Start, End, Placement;
    FTransform StartPose, EndPose, Ghost;
    bool bHasPlan = false, bPanning = false;
    double Zoom = 1.0;
    FVector2D Pan = FVector2D::ZeroVector;
    TFunction<void(FName, bool, bool, bool)> LaneClicked;
};

void STMOPVehicleEditor::Construct(const FArguments& Args)
{
    FDetailsViewArgs DetailsArgs;
    DetailsArgs.bAllowSearch = true;
    DetailsArgs.bHideSelectionTip = true;
    DetailsArgs.bLockable = false;
    DetailsArgs.bUpdatesFromSelection = false;
    DetailsArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
    FPropertyEditorModule& PropertyEditor =
        FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));
    VehicleDetails = PropertyEditor.CreateDetailView(DetailsArgs);
    EntryDetails = PropertyEditor.CreateDetailView(DetailsArgs);
    AccessoryDetails = PropertyEditor.CreateDetailView(DetailsArgs);
    const auto CanEdit = TAttribute<bool>::Create(
        TAttribute<bool>::FGetter::CreateSP(this, &STMOPVehicleEditor::CanEditDetails));
    VehicleDetails->SetEnabled(CanEdit);
    EntryDetails->SetEnabled(CanEdit);
    AccessoryDetails->SetEnabled(CanEdit);
    VehicleDetails->SetIsPropertyVisibleDelegate(
        FIsPropertyVisible::CreateLambda([](const FPropertyAndParent& Property)
        {
            return Property.Property.GetFName() !=
                GET_MEMBER_NAME_CHECKED(FTMOPHistoricalVehicleRow, Timeline);
        }));
    AccessoryDetails->OnFinishedChangingProperties().AddSP(this,
        &STMOPVehicleEditor::OnAccessoryDetailsChanged);
    EntryDetails->OnFinishedChangingProperties().AddSP(
        this, &STMOPVehicleEditor::OnDetailsChanged, false);
    VehicleDetails->OnFinishedChangingProperties().AddSP(
        this, &STMOPVehicleEditor::OnDetailsChanged, true);

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
            [ SNew(SButton).Text(LOCTEXT("ValidateAll", "Check Before Play"))
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
                        [ SNew(SButton).Text(LOCTEXT("QuickAddStop", "+ Add Stop"))
                            .ToolTipText(LOCTEXT("QuickAddStopTip", "Insert a stop after the selected row at its destination, relative to the previous entry +30 seconds."))
                            .IsEnabled_Lambda([this]() { return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex); })
                            .OnClicked(this, &STMOPVehicleEditor::AddStopEntry) ]
                        + SHorizontalBox::Slot().AutoWidth().Padding(3,0)
                        [ SNew(SButton).Text(LOCTEXT("QuickAddDespawn", "+ Add Despawn"))
                            .ToolTipText(LOCTEXT("QuickAddDespawnTip", "Insert a despawn after the selected row, relative to the previous entry +2 seconds."))
                            .IsEnabled_Lambda([this]() { return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex); })
                            .OnClicked(this, &STMOPVehicleEditor::AddDespawnEntry) ]
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
                SNew(SScrollBox)
                + SScrollBox::Slot()
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
                        .Visibility(this, &STMOPVehicleEditor::LaneVisibility)
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
                        .Visibility(this, &STMOPVehicleEditor::LaneVisibility)
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
                        .Visibility(this, &STMOPVehicleEditor::LaneVisibility)
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
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text(LOCTEXT("ClearStartAnchor", "Clear start anchor"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearRouteReference,
                            ERouteReferenceField::StartAnchor) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("ClearStartLane", "Clear start lane"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearRouteReference,
                            ERouteReferenceField::StartLane) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("ClearEndAnchor", "Clear end anchor"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearRouteReference,
                            ERouteReferenceField::DestinationAnchor) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("ClearEndLane", "Clear end lane"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearRouteReference,
                            ERouteReferenceField::DestinationLane) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text(LOCTEXT("ClearViaAnchors", "Clear via anchors"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearRouteReference,
                            ERouteReferenceField::ViaAnchor) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("ClearViaLanes", "Clear via lanes"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearRouteReference,
                            ERouteReferenceField::ViaLane) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4,0,0,0)
                    [ SNew(SButton).Text(LOCTEXT("ClearAllRouteFields", "Clear all route fields"))
                        .OnClicked(this, &STMOPVehicleEditor::ClearAllRouteReferences) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,5,7,2)
                [ SNew(STextBlock).Visibility(this, &STMOPVehicleEditor::PlacementVisibility).Text(LOCTEXT("EntryAnchor", "PLACEMENT ANCHOR"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2,7,6)
                [ SAssignNew(AnchorCombo, SSearchableComboBox)
                    .Visibility(this, &STMOPVehicleEditor::PlacementVisibility)
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
                + SVerticalBox::Slot().AutoHeight().Padding(7,5,7,2)
                [ SNew(STextBlock).Text(LOCTEXT("DepartureSharedEvent",
                    "DEPARTURE SHARED EVENT (TIME IS ARRIVAL)"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall"))
                    .Visibility_Lambda([this]()
                    {
                        if (!EntryStruct.IsValid()) return EVisibility::Collapsed;
                        const auto* Entry = reinterpret_cast<const
                            FTMOPHistoricalVehicleTimelineEntry*>(
                                EntryStruct->GetStructMemory());
                        return IsDriving(Entry->Action) && Entry->bTimeIsArrival
                            ? EVisibility::Visible : EVisibility::Collapsed;
                    }) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2,7,6)
                [ SAssignNew(DepartureEventCombo, SSearchableComboBox)
                    .OptionsSource(&EventItems)
                    .OnGenerateWidget(this, &STMOPVehicleEditor::GenerateEventOption)
                    .OnSelectionChanged(this,
                        &STMOPVehicleEditor::OnDepartureEventSelected)
                    .Visibility_Lambda([this]()
                    {
                        if (!EntryStruct.IsValid()) return EVisibility::Collapsed;
                        const auto* Entry = reinterpret_cast<const
                            FTMOPHistoricalVehicleTimelineEntry*>(
                                EntryStruct->GetStructMemory());
                        return IsDriving(Entry->Action) && Entry->bTimeIsArrival
                            ? EVisibility::Visible : EVisibility::Collapsed;
                    })
                    [ SNew(STextBlock).Text(this,
                        &STMOPVehicleEditor::GetSelectedDepartureEventText) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,4)
                [ BuildDrivingControls() ]
                + SVerticalBox::Slot().AutoHeight().Padding(4)
                [ SNew(SBox).HeightOverride(330)
                    [ SAssignNew(RouteMap, STMOPVehicleRouteMap).Clipping(EWidgetClipping::ClipToBounds) ] ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text_Lambda([this]() { return FText::FromString(bPreviewPlaying ? TEXT("Pause preview") : TEXT("Play preview")); })
                        .OnClicked(this, &STMOPVehicleEditor::TogglePreview) ]
                    + SHorizontalBox::Slot().FillWidth(1).Padding(8,0)
                    [ SNew(SSlider).Value_Lambda([this]() { return PreviewAlpha; })
                        .OnValueChanged(this, &STMOPVehicleEditor::SetPreviewAlpha) ]
                    + SHorizontalBox::Slot().AutoWidth()
                    [ SNew(SButton).Text(LOCTEXT("FitMap","Fit"))
                        .OnClicked_Lambda([this]() { RouteMap->Fit(); return FReply::Handled(); }) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text_Lambda([this]()
                    { return FText::FromString(FormatClockSecond(FMath::RoundToInt(
                        FMath::Lerp(float(PreviewDeparture), float(PreviewArrival), PreviewAlpha)))) ; }) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text(LOCTEXT("Occupants", "OCCUPANTS / BOARDING FEASIBILITY"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetLiveDriverStatusText)
                    .AutoWrapText(true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text(LOCTEXT("PlannedOccupantsLabel",
                    "PLANNED OCCUPANTS — timeline prediction, not proof of actual boarding"))
                    .AutoWrapText(true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(STextBlock).Text_Lambda([this]()
                    { return FText::FromString(CachedOccupants); })
                    .AutoWrapText(true) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,8,7,2)
                [ SNew(STextBlock).Text(LOCTEXT("Validation", "VALIDATION"))
                    .Font(FAppStyle::GetFontStyle("HeadingExtraSmall")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(7,2)
                [ SNew(SBox).HeightOverride(220)
                  [ SAssignNew(ValidationList, SListView<FValidationItemPtr>)
                    .ListItemsSource(&ValidationItems)
                    .OnGenerateRow(this, &STMOPVehicleEditor::GenerateValidationRow) ] ]
                ]
            ]
            + SSplitter::Slot().Value(0.28f)
            [
                SNew(SSplitter).Orientation(Orient_Vertical)
                + SSplitter::Slot().Value(0.32f)
                [ SNew(SBorder).Padding(5)[SAssignNew(AppearancePreview, STMOPAppearancePreview)] ]
                + SSplitter::Slot().Value(0.34f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [
                        SNew(SExpandableArea).InitiallyCollapsed(true)
                        .HeaderContent()[SNew(STextBlock).Text(LOCTEXT("EquipmentHeading", "ACCESSORIES / SOCKETS"))]
                        .BodyContent()[BuildAccessoryControls()]
                    ]
                    + SVerticalBox::Slot().FillHeight(1)
                    [ SNew(SBorder).Padding(5)[VehicleDetails.ToSharedRef()] ]
                ]
                + SSplitter::Slot().Value(0.34f)
                [ SNew(SBorder).Padding(5)[EntryDetails.ToSharedRef()] ]
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
    if (IsValid(Args._VehicleTableOverride))
    {
        VehicleTable = Args._VehicleTableOverride;
        RefreshVehicles();
    }
    else LoadTables();
    UE_LOG(LogTemp, Display, TEXT("TMOP Vehicle Editor: route clear controls 2026-09-06 r6"));
    LastRuntimeValidationRevision =
        TMOPRuntimeValidation::GetLatestReportRevision();
    RegisterActiveTimer(
        2.0f,
        FWidgetActiveTimerDelegate::CreateSP(
            this,
            &STMOPVehicleEditor::HandleRuntimeValidationRefresh));
}

void STMOPVehicleEditor::LoadTables()
{
    VehicleTable = LoadObject<UDataTable>(nullptr, VehicleTablePath);
    PeopleTable = LoadObject<UDataTable>(nullptr, PeopleTablePath);
    EventTable = LoadObject<UDataTable>(nullptr, EventTablePath);
    if (UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr)
    {
        for (TActorIterator<ATMOPHistoricalVehicleDirector> It(World); It; ++It)
            if (It->HistoricalVehicleTable) { VehicleTable = It->HistoricalVehicleTable.Get(); break; }
        for (TActorIterator<ATMOPPersonRegistryDirector> It(World); It; ++It)
            if (It->PersonProfileTable) { PeopleTable = It->PersonProfileTable.Get(); break; }
        for (TActorIterator<ATMOPHistoricalEventDirector> It(World); It; ++It)
            if (It->EventTable) { EventTable = It->EventTable.Get(); break; }
    }
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
    SetStatus(FText::FromString(TEXT("Loaded: ") + VehicleTable->GetPathName()), FLinearColor(0.4f,1,0.4f));
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
    SyncDetailsFromWorking();
    RebuildRoutePreview();
}

EActiveTimerReturnType STMOPVehicleEditor::HandleRuntimeValidationRefresh(
    const double, const float)
{
    const uint64 Revision =
        TMOPRuntimeValidation::GetLatestReportRevision();
    if (Revision != LastRuntimeValidationRevision)
    {
        LastRuntimeValidationRevision = Revision;
        if (TimelineList.IsValid())
            TimelineList->RequestListRefresh();
    }
    return EActiveTimerReturnType::Continue;
}

void STMOPVehicleEditor::SelectVehicle(const FName RowName)
{
    if (bChangingSelection) return;
    TGuardValue<bool> SelectionGuard(bChangingSelection, true);
    SyncVehicleListSelection();
    if (!ConfirmDiscardOrSave()) return;
    CommitEntry(); CommitVehicle();
    const UDataTable* Table = VehicleTable.Get();
    const FTMOPHistoricalVehicleRow* Row = IsValid(Table) ? Table->FindRow<FTMOPHistoricalVehicleRow>(RowName, TEXT("VehicleEditorSelect"), false) : nullptr;
    if (!Row) return;
    UE_LOG(LogTemp, Log, TEXT("TMOP Vehicle Editor select: row=%s vehicle=%s entries=%d"),
        *RowName.ToString(), *Row->VehicleId.ToString(), Row->Timeline.Num());
    SelectedRowName=RowName; WorkingRow=*Row; SavedRow=*Row; SelectedTimelineIndex=INDEX_NONE;
    SelectedAccessoryIndex = INDEX_NONE;
    SyncDetailsFromWorking();
    RefreshAppearancePreview();
    RefreshTimeline();
    if (!WorkingRow.Timeline.IsEmpty()) SelectTimelineEntry(0);
    RefreshAccessoryChoices();
    SyncVehicleListSelection();
}

void STMOPVehicleEditor::SelectTimelineEntry(const int32 Index)
{
    CommitEntry();
    if (!WorkingRow.Timeline.IsValidIndex(Index)) return;
    bPreviewPlaying = false;
    PreviewAlpha = 0.0f;
    if (RouteMap.IsValid()) RouteMap->Fit();
    SelectedTimelineIndex=Index;
    SyncDetailsFromWorking();
    RebuildValidation();
    RebuildRoutePreview();
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
    if (DepartureEventCombo.IsValid()) DepartureEventCombo->RefreshOptions();
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
    CommitEntry();
    SyncDetailsFromWorking();
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
    CommitEntry();
    SyncDetailsFromWorking();
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

void STMOPVehicleEditor::OnDepartureEventSelected(
    const FEventItem Item, ESelectInfo::Type)
{
    if (!Item.IsValid() || !EntryStruct.IsValid()) return;
    const FName* EventId = EventIdsByLabel.Find(*Item);
    if (EventId == nullptr) return;
    auto* Entry = reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    if (!IsDriving(Entry->Action)) return;
    Entry->bTimeIsArrival = true;
    Entry->bUseExplicitDepartureTime = true;
    Entry->DepartureTimingMode = ETMOPEventTimingMode::Relative;
    Entry->DepartureSharedEventId = *EventId;
    CommitEntry();
    SyncDetailsFromWorking();
    RebuildValidation();
    RebuildRoutePreview();
    SetStatus(FText::FromString(TEXT("Selected departure shared event: ") +
        EventId->ToString()), FLinearColor(.55f,.8f,.55f));
}

FText STMOPVehicleEditor::GetSelectedDepartureEventText() const
{
    if (!EntryStruct.IsValid())
        return LOCTEXT("SelectEntryForDepartureEvent",
            "Select a driving timeline entry");
    const auto* Entry = reinterpret_cast<const
        FTMOPHistoricalVehicleTimelineEntry*>(
            EntryStruct->GetStructMemory());
    return Entry->DepartureSharedEventId.IsNone()
        ? LOCTEXT("SearchDepartureEvent", "Type to search departure event...")
        : FText::FromName(Entry->DepartureSharedEventId);
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
        break;
    case ERouteReferenceField::StartLane:
        Entry->RouteStartLaneId = *Id;
        break;
    case ERouteReferenceField::DestinationAnchor:
        Entry->RouteDestinationAnchorId = *Id;
        break;
    case ERouteReferenceField::DestinationLane:
        Entry->RouteDestinationLaneId = *Id;
        break;
    case ERouteReferenceField::ViaAnchor:
        Entry->RouteViaAnchorIds.AddUnique(*Id);
        break;
    case ERouteReferenceField::ViaLane:
        Entry->RouteViaLaneIds.AddUnique(*Id);
        break;
    }
    CommitEntry();
    SyncDetailsFromWorking();
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
    CommitEntry();
    SyncDetailsFromWorking();
    return RecalculateRoute();
}

FReply STMOPVehicleEditor::ClearRouteReference(const ERouteReferenceField Field)
{
    if (!EntryStruct.IsValid()) return FReply::Handled();
    auto* Entry = reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    if (!IsDriving(Entry->Action)) return FReply::Handled();
    switch (Field)
    {
    case ERouteReferenceField::StartAnchor: Entry->RouteStartAnchorId = NAME_None; break;
    case ERouteReferenceField::StartLane: Entry->RouteStartLaneId = NAME_None; break;
    case ERouteReferenceField::DestinationAnchor: Entry->RouteDestinationAnchorId = NAME_None; break;
    case ERouteReferenceField::DestinationLane: Entry->RouteDestinationLaneId = NAME_None; break;
    case ERouteReferenceField::ViaAnchor: Entry->RouteViaAnchorIds.Reset(); break;
    case ERouteReferenceField::ViaLane: Entry->RouteViaLaneIds.Reset(); break;
    }
    CommitEntry();
    SyncDetailsFromWorking();
    return RecalculateRoute();
}

FReply STMOPVehicleEditor::ClearAllRouteReferences()
{
    if (!EntryStruct.IsValid()) return FReply::Handled();
    auto* Entry = reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(
        EntryStruct->GetStructMemory());
    if (!IsDriving(Entry->Action)) return FReply::Handled();
    Entry->RouteStartAnchorId = NAME_None;
    Entry->RouteStartLaneId = NAME_None;
    Entry->RouteDestinationAnchorId = NAME_None;
    Entry->RouteDestinationLaneId = NAME_None;
    Entry->RouteViaAnchorIds.Reset();
    Entry->RouteViaLaneIds.Reset();
    Entry->OrderedLaneIds.Reset();
    CommitEntry();
    SyncDetailsFromWorking();
    return RecalculateRoute();
}

FReply STMOPVehicleEditor::PreviewRouteInLevel()
{
    CommitEntry(); RebuildRoutePreview();
    if (PreviewPlan.Samples.IsEmpty())
    { SetStatus(LOCTEXT("NoPreview", "Resolve the route before previewing."), FLinearColor::Red); return FReply::Handled(); }
    bPreviewInLevel = !bPreviewInLevel;
    if (bPreviewInLevel && !bPreviewPlaying) TogglePreview();
    SetStatus(FText::FromString(bPreviewInLevel ? TEXT("Preview in level enabled: white box = vehicle footprint. Use the time slider.")
        : TEXT("Preview in level disabled.")), FLinearColor::White);
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
    }
    else if (bSetDestination)
    {
        Entry->RouteDestinationLaneId = LaneId;
    }
    else
    {
        SetStatus(FText::FromString(FString::Printf(
            TEXT("Lane %s — Shift+click=start, Ctrl+click=end, right-click=via."),
            *LaneId.ToString())), FLinearColor(.55f, .8f, 1.0f));
        return;
    }
    CommitEntry();
    SyncDetailsFromWorking();
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
    FTMOPVehicleRoutePlan Plan;
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!TMOPVehicleRoute::Build(World, WorkingRow, SelectedTimelineIndex, Plan, OutFailure, true))
    {
        // Keep the authored fields for correction, but never preview a stale successful route.
        PreviewPlan = FTMOPVehicleRoutePlan();
        RebuildRoutePreview();
        return false;
    }
    auto& Entry = WorkingRow.Timeline[SelectedTimelineIndex];
    if (!Plan.bAnchorManeuver) Entry.OrderedLaneIds = Plan.LaneIds;
    SyncDetailsFromWorking();
    RebuildValidation();
    RebuildRoutePreview();
    return true;
}


FText STMOPVehicleEditor::GetRouteEndpointsText() const { return CachedRouteEndpoints; }
FText STMOPVehicleEditor::BuildRouteEndpointsText() const
{
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) ||
        !IsDriving(WorkingRow.Timeline[SelectedTimelineIndex].Action))
        return LOCTEXT("ChooseDrive","Select a driving row to edit its route.");
    FTMOPVehicleRoutePlan Plan; FString Failure;
    if (!TMOPVehicleRoute::Build(GEditor ? GEditor->GetEditorWorldContext().World() : nullptr,
        WorkingRow, SelectedTimelineIndex, Plan, Failure))
        return FText::FromString(TEXT("Not ready: ") + Failure);
    const auto& Entry = WorkingRow.Timeline[SelectedTimelineIndex];
    return FText::FromString(FString::Printf(TEXT("FROM %s%s\nTO %s%s\n%s | %.1f m"),
        Plan.StartAnchorId.IsNone() ? TEXT("world/lane placement") : *Plan.StartAnchorId.ToString(),
        Entry.RouteStartAnchorId.IsNone() ? TEXT(" (inherited)") : TEXT(""),
        Plan.EndAnchorId.IsNone() ? TEXT("end of lane") : *Plan.EndAnchorId.ToString(),
        Entry.RouteDestinationAnchorId.IsNone() ? TEXT(" (inherited)") : TEXT(""),
        Plan.bAnchorManeuver ? TEXT("Free anchor maneuver") : TEXT("Follow lanes"),
        Plan.LengthCm / 100.0));
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
    FText ArrivalText; FText ArrivalToolTip;
    FLinearColor ArrivalColor = FLinearColor::Transparent;
    const bool bShowArrivalBadge = E && IsStop(E->Action) &&
        TMOPRuntimeValidation::BuildArrivalBadge(
            WorkingRow.VehicleId, E->EntryId, ArrivalText,
            ArrivalToolTip, ArrivalColor, GetEntryFingerprint(I));
    int32 Sec=0; FString Fail; const bool HasTime=E&&ResolveTime(WorkingRow,I,Sec,&Fail); FLinearColor Color(0.35f,0.75f,1);
    FString Badge; FString Tip;
    if (E&&IsDriving(E->Action)) { double D=0,K=0; int32 Departure=0,Arrival=0,S=0; if(CalculateDrive(WorkingRow,I,D,Departure,Arrival,S,K,Fail)){Badge=FString::Printf(TEXT("DEP %s  ARR %s  REQ %.1f km/h"),*FormatClockSecond(Departure),*FormatClockSecond(Arrival),K);Tip=FString::Printf(TEXT("%.2f km over %d seconds. The schedule controls speed; presets do not change arrival."),D/100000.0,S);Color=K<=50?FLinearColor(0.05f,.42f,.12f):K<=90?FLinearColor(.78f,.38f,.03f):FLinearColor(.75f,.05f,.03f);if(E->bIgnoreOneWayRestrictions)Tip+=TEXT(" Ignores restricted lane connections.");if(E->bRunRedLights)Tip+=TEXT(" May run red lights.");} else {Badge=TEXT("SPEED ?");Tip=Fail;Color=FLinearColor(.55f,.12f,.08f);} }
    FString Summary=E?VehicleActionLabel(E->Action):FString(); if(E&&IsDriving(E->Action)&&!E->RouteSegmentName.IsEmpty())Summary+=TEXT(" • ")+E->RouteSegmentName.ToString();if(E&&!IsDriving(E->Action)&&!E->PlacementAnchorId.IsNone()) Summary+=TEXT(" → ")+E->PlacementAnchorId.ToString(); if(E&&IsDriving(E->Action)&&E->VehicleRouteMode==ETMOPVehicleRouteMode::AnchorManeuver)Summary+=TEXT(" • ANCHOR MANEUVER • NO LANES");else if(E&&!E->OrderedLaneIds.IsEmpty()) Summary+=FString::Printf(TEXT(" • %d lanes"),E->OrderedLaneIds.Num());if(E&&IsDriving(E->Action)&&E->DrivingPreset!=ETMOPVehicleDrivingPreset::AutomaticFromTimeline)Summary+=TEXT(" • ")+DrivingPresetLabel(E->DrivingPreset);
    if (E && E->Action == ETMOPHistoricalVehicleAction::OffscreenTransfer)
        Summary += FString::Printf(TEXT(" • hidden %d s"),
            FMath::Max(0, E->OffscreenTransferDurationSeconds));
    if (E && E->TimingMode == ETMOPEventTimingMode::Relative)
        Summary += FString::Printf(TEXT(" • @ %s %+d s"),
            *E->SharedEventId.ToString(), E->EventOffsetSeconds);
    else if (E && E->TimingMode ==
        ETMOPEventTimingMode::RelativeToPreviousEntry)
        Summary += FString::Printf(TEXT(" • Previous %+d s"),
            E->EventOffsetSeconds);
    if (E && IsDriving(E->Action) && E->bTimeIsArrival)
        Summary += TEXT(" • TIME IS ARRIVAL");
    if (E && IsDriving(E->Action) && E->bTimeIsArrival &&
        E->bUseExplicitDepartureTime)
        Summary += TEXT(" • DEPARTURE SET ON ROW");
    FString DestinationText;
    FText DestinationToolTip;
    FLinearColor DestinationColor = FLinearColor(0.35f, 0.85f, 0.45f);
    const bool bShowDestination = E && IsDriving(E->Action);
    if (bShowDestination)
    {
        if (!E->RouteDestinationAnchorId.IsNone())
        {
            DestinationText = TEXT("END ANCHOR → ") +
                E->RouteDestinationAnchorId.ToString();
            DestinationToolTip = LOCTEXT("ExplicitRouteDestinationTip",
                "This driving row has an explicit Route Destination Anchor ID. The vehicle can use its exact transform for the final approach.");
        }
        else
        {
            FName FollowingStopAnchor = NAME_None;
            for (int32 NextIndex = I + 1;
                NextIndex < WorkingRow.Timeline.Num(); ++NextIndex)
            {
                const FTMOPHistoricalVehicleTimelineEntry& NextEntry =
                    WorkingRow.Timeline[NextIndex];
                if (!IsStop(NextEntry.Action))
                    continue;
                FollowingStopAnchor = NextEntry.PlacementAnchorId;
                break;
            }

            DestinationText = FollowingStopAnchor.IsNone() ? TEXT("END: lane endpoint (no anchor)")
                : TEXT("END (inherited Stop) → ") + FollowingStopAnchor.ToString();
            if (!E->RouteDestinationLaneId.IsNone())
                DestinationText += TEXT(" • bara lane: ") +
                    E->RouteDestinationLaneId.ToString();

            DestinationColor = FollowingStopAnchor.IsNone() ? FLinearColor(1.0f,0.65f,0.2f)
                : FLinearColor(0.35f,0.85f,0.45f);
            DestinationToolTip = LOCTEXT("MissingRouteDestinationTip",
                "The shared route planner inherits an empty end anchor from the following Stop/Park, including its local offset. With no following stop it ends at the selected lane endpoint.");
        }
    }
    const int32 N=FMath::Max(0,Sec)%(24*3600); const FString Time=HasTime?FString::Printf(TEXT("%02d:%02d:%02d"),N/3600,(N/60)%60,N%60):TEXT("TIME ?");
    return SNew(STableRow<FTimelineItem>,Owner)[SNew(SBorder).Padding(6).BorderImage(FAppStyle::GetBrush("Brushes.Panel"))[SNew(SVerticalBox)
        +SVerticalBox::Slot().AutoHeight()[SNew(SHorizontalBox)+SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::AsNumber(I)).ColorAndOpacity(FSlateColor::UseSubduedForeground())]+SHorizontalBox::Slot().FillWidth(1).Padding(7,0)[SNew(STextBlock).Text(E?FText::FromName(E->EntryId):FText::GetEmpty()).ColorAndOpacity(Color)]+SHorizontalBox::Slot().AutoWidth()[SNew(STextBlock).Text(FText::FromString(Time)).ColorAndOpacity(FLinearColor(1,.72f,.05f))]]
        +SVerticalBox::Slot().AutoHeight().Padding(22,2,0,0)[SNew(STextBlock).Text(FText::FromString(Summary)).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FSlateColor::UseSubduedForeground())]
        +SVerticalBox::Slot().AutoHeight().Padding(22,2,0,0)[SNew(STextBlock).Visibility(bShowDestination?EVisibility::Visible:EVisibility::Collapsed).Text(FText::FromString(DestinationText)).ToolTipText(DestinationToolTip).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(DestinationColor)]
        +SVerticalBox::Slot().AutoHeight().Padding(22,2,0,0)[SNew(STextBlock).Text(FText::FromString(Badge)).ToolTipText(FText::FromString(Tip)).ColorAndOpacity(Color)]
        +SVerticalBox::Slot().AutoHeight().Padding(22,2,0,0)[SNew(SBorder).Visibility(bShowArrivalBadge?EVisibility::Visible:EVisibility::Collapsed).Padding(FMargin(5,1)).BorderImage(FAppStyle::GetBrush("Brushes.Panel")).BorderBackgroundColor(ArrivalColor).ToolTipText(ArrivalToolTip)[SNew(STextBlock).Text(ArrivalText).Font(FAppStyle::GetFontStyle("SmallFont")).ColorAndOpacity(FLinearColor::White)]]]];
}

void STMOPVehicleEditor::OnVehicleSelected(FVehicleItem Item, ESelectInfo::Type)
{
    if (Item.IsValid() && *Item != SelectedRowName) SelectVehicle(*Item);
} void STMOPVehicleEditor::OnTimelineSelected(FTimelineItem I,ESelectInfo::Type){if(I.IsValid())SelectTimelineEntry(*I);} void STMOPVehicleEditor::OnSearchChanged(const FText& T){Search=T.ToString();RefreshVehicles();}

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

FReply STMOPVehicleEditor::AddDespawnEntry()
{
    CommitEntry();
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)) return FReply::Handled();
    const int32 NewIndex = SelectedTimelineIndex + 1;
    // Copy the ID before Insert can reallocate the timeline storage.
    const FString Base = WorkingRow.Timeline[SelectedTimelineIndex].EntryId.ToString() + TEXT("_DESPAWN");
    FTMOPHistoricalVehicleTimelineEntry Entry;
    Entry.Action = ETMOPHistoricalVehicleAction::Despawn;
    Entry.TimingMode = ETMOPEventTimingMode::RelativeToPreviousEntry;
    Entry.EventOffsetSeconds = 2;
    Entry.bTimeIsArrival = false;
    Entry.EntryId = FName(*Base);
    for (int32 Suffix = 1; WorkingRow.Timeline.ContainsByPredicate(
        [&Entry](const FTMOPHistoricalVehicleTimelineEntry& Existing) { return Existing.EntryId == Entry.EntryId; }); ++Suffix)
        Entry.EntryId = FName(*FString::Printf(TEXT("%s_%d"), *Base, Suffix));
    WorkingRow.Timeline.Insert(Entry, NewIndex);
    RefreshTimeline();
    SelectTimelineEntry(NewIndex);
    return FReply::Handled();
}

FReply STMOPVehicleEditor::AddStopEntry()
{
    CommitEntry();
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)) return FReply::Handled();
    // Own the source value before Insert can reallocate the timeline.
    const auto Source = WorkingRow.Timeline[SelectedTimelineIndex];
    const int32 NewIndex = SelectedTimelineIndex + 1;
    FTMOPHistoricalVehicleTimelineEntry Stop;
    Stop.EntryId = TMOPVehicleRoute::UniqueEntryId(WorkingRow,
        Source.EntryId.ToString() + TEXT("_STOP"));
    Stop.Action = ETMOPHistoricalVehicleAction::Stop;
    Stop.TimingMode = ETMOPEventTimingMode::RelativeToPreviousEntry;
    Stop.EventOffsetSeconds = 30;
    Stop.bTimeIsArrival = false;
    Stop.DriverEntityId = TMOPVehicleRoute::Driver(WorkingRow, Source);
    Stop.PassengerEntityIds = Source.PassengerEntityIds;
    Stop.PassengerEntityIds.Remove(Stop.DriverEntityId);
    if (TMOPVehicleRoute::IsDriving(Source.Action))
    {
        Stop.PlacementMode = ETMOPHistoricalVehiclePlacementMode::Anchor;
        Stop.PlacementAnchorId = Source.RouteDestinationAnchorId;
        // Match the same following-stop anchor/offset used by the route builder.
        for (int32 Next = NewIndex; Next < WorkingRow.Timeline.Num(); ++Next)
        {
            const auto& Following = WorkingRow.Timeline[Next];
            if (TMOPVehicleRoute::IsDriving(Following.Action) ||
                Following.Action == ETMOPHistoricalVehicleAction::Despawn) break;
            if (!TMOPVehicleRoute::IsStop(Following.Action)) continue;
            if (Stop.PlacementAnchorId.IsNone() || Stop.PlacementAnchorId == Following.PlacementAnchorId)
            {
                Stop.PlacementMode = Following.PlacementMode;
                Stop.PlacementAnchorId = Following.PlacementAnchorId;
                Stop.AnchorLocalOffset = Following.AnchorLocalOffset;
                Stop.WorldTransform = Following.WorldTransform;
            }
            break;
        }
    }
    else if (TMOPVehicleRoute::HasPlacement(Source.Action))
    {
        Stop.PlacementMode = Source.PlacementMode;
        Stop.PlacementAnchorId = Source.PlacementAnchorId;
        Stop.AnchorLocalOffset = Source.AnchorLocalOffset;
        Stop.WorldTransform = Source.WorldTransform;
    }
    else
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("QuickStopNoPosition", "Select a driving or placement row with a destination first."));
        return FReply::Handled();
    }
    if (Stop.PlacementMode == ETMOPHistoricalVehiclePlacementMode::Anchor && Stop.PlacementAnchorId.IsNone())
    {
        FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("QuickStopNeedsAnchor", "This route ends on a lane without a destination anchor. Set its end anchor before adding a stop."));
        return FReply::Handled();
    }
    WorkingRow.Timeline.Insert(Stop, NewIndex);
    RefreshTimeline();
    SelectTimelineEntry(NewIndex);
    return FReply::Handled();
}

FReply STMOPVehicleEditor::AddEntry()
{
    if (SelectedRowName.IsNone()) return FReply::Handled();

    CommitEntry();

    FTMOPHistoricalVehicleTimelineEntry NewEntry;
    NewEntry.EntryId = TMOPVehicleRoute::UniqueEntryId(
        WorkingRow, WorkingRow.VehicleId.ToString() + TEXT("_ENTRY"));
    NewEntry.bAutoStartFromVehicleTimeline = true;

    // Occupants describe who is expected to be in the vehicle for a timeline
    // segment. Carry the most recently authored values forward so adding a
    // Stop followed by another drive does not require re-entering everybody.
    // Copy into NewEntry before modifying Timeline: never retain references to
    // TArray elements across Add/Insert, because those operations may reallocate.
    bool bFoundDriver = false;
    bool bFoundPassengers = false;
    for (int32 Index = WorkingRow.Timeline.Num() - 1;
         Index >= 0 && (!bFoundDriver || !bFoundPassengers);
         --Index)
    {
        const FTMOPHistoricalVehicleTimelineEntry& PreviousEntry =
            WorkingRow.Timeline[Index];

        if (!bFoundDriver && !PreviousEntry.DriverEntityId.IsNone())
        {
            NewEntry.DriverEntityId = PreviousEntry.DriverEntityId;
            bFoundDriver = true;
        }

        if (!bFoundPassengers && !PreviousEntry.PassengerEntityIds.IsEmpty())
        {
            NewEntry.PassengerEntityIds = PreviousEntry.PassengerEntityIds;
            bFoundPassengers = true;
        }
    }

    if (!bFoundDriver)
    {
        NewEntry.DriverEntityId = WorkingRow.KnownDriverEntityId;
    }

    // Keep the driver out of the passenger array and clean up accidental
    // duplicates while the data is being inherited.
    TArray<FName> CleanPassengers;
    CleanPassengers.Reserve(NewEntry.PassengerEntityIds.Num());
    for (const FName PassengerId : NewEntry.PassengerEntityIds)
    {
        if (!PassengerId.IsNone() && PassengerId != NewEntry.DriverEntityId)
        {
            CleanPassengers.AddUnique(PassengerId);
        }
    }
    NewEntry.PassengerEntityIds = MoveTemp(CleanPassengers);

    WorkingRow.Timeline.Add(MoveTemp(NewEntry));
    RefreshTimeline();
    SelectTimelineEntry(WorkingRow.Timeline.Num() - 1);
    return FReply::Handled();
}
FReply STMOPVehicleEditor::DuplicateEntry(){CommitEntry();if(WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)){auto E=WorkingRow.Timeline[SelectedTimelineIndex];E.EntryId=TMOPVehicleRoute::UniqueEntryId(WorkingRow, E.EntryId.ToString()+TEXT("_COPY"));WorkingRow.Timeline.Insert(E,SelectedTimelineIndex+1);RefreshTimeline();SelectTimelineEntry(SelectedTimelineIndex+1);}return FReply::Handled();}
FReply STMOPVehicleEditor::DeleteEntry(){CommitEntry();if(WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)){WorkingRow.Timeline.RemoveAt(SelectedTimelineIndex);SelectedTimelineIndex=INDEX_NONE;EntryStruct.Reset();RefreshTimeline();}return FReply::Handled();}
FReply STMOPVehicleEditor::MoveEntry(const int32 D){CommitEntry();const int32 N=SelectedTimelineIndex+D;if(WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)&&WorkingRow.Timeline.IsValidIndex(N)){WorkingRow.Timeline.Swap(SelectedTimelineIndex,N);SelectedTimelineIndex=INDEX_NONE;RefreshTimeline();SelectTimelineEntry(N);}return FReply::Handled();}

FReply STMOPVehicleEditor::SaveVehicle()
{
    CommitEntry();
    CommitVehicle();
    UDataTable* Table = VehicleTable.Get();
    if (!IsValid(Table) || SelectedRowName.IsNone()) return FReply::Handled();
    if (!Table->GetRowMap().Contains(SelectedRowName))
    {
        SetStatus(LOCTEXT("VehicleRemovedBeforeSave", "The selected row no longer exists. Reload the table before saving."), FLinearColor::Red);
        return FReply::Handled();
    }

    CurrentErrors = ValidateRow(WorkingRow);
    const FString* BlockingError = CurrentErrors.FindByPredicate(
        [](const FString& Message) { return Message.StartsWith(TEXT("ERROR")); });
    bool bSavedWithValidationErrors = false;
    if (BlockingError)
    {
        const FString Prompt = FString::Printf(
            TEXT("The vehicle has validation errors.\n\nFirst error:\n%s\n\nSave anyway?"),
            **BlockingError);
        if (FMessageDialog::Open(EAppMsgType::YesNo,
            FText::FromString(Prompt)) != EAppReturnType::Yes)
        {
            SetStatus(FText::FromString(TEXT("Not saved: ") + *BlockingError),
                FLinearColor::Red);
            return FReply::Handled();
        }
        bSavedWithValidationErrors = true;
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

    const FName PreviousRowName = SelectedRowName;
    const FTMOPHistoricalVehicleRow* ExistingRow =
        Table->FindRow<FTMOPHistoricalVehicleRow>(SelectedRowName, TEXT("SaveBackup"), false);
    if (!ExistingRow)
    {
        SetStatus(LOCTEXT("VehicleRemovedBeforeSave", "The selected row no longer exists. Reload the table before saving."), FLinearColor::Red);
        return FReply::Handled();
    }
    const FTMOPHistoricalVehicleRow PreviousTableRow = *ExistingRow;
    const FScopedTransaction SaveTransaction(LOCTEXT("SaveVehicleTransaction", "Edit vehicle timeline"));
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
    TArray<UPackage*> Packages = { Table->GetOutermost() };
    TArray<UPackage*> FailedPackages;
    const auto SaveResult = FEditorFileUtils::PromptForCheckoutAndSave(
        Packages, false, false, &FailedPackages, false, false);
    if (SaveResult != FEditorFileUtils::EPromptReturnCode::PR_Success || !FailedPackages.IsEmpty())
    {
        Table->RemoveRow(NewRowName);
        Table->AddRow(PreviousRowName, PreviousTableRow);
        SelectedRowName = PreviousRowName;
        SetStatus(LOCTEXT("SaveFailedDisk",
            "Changes are still open. The table could not be saved to disk; retry Save Vehicle."),
            FLinearColor::Red);
        return FReply::Handled();
    }
    SavedRow = WorkingRow;
    RefreshVehicles();
    RefreshTimeline();
    SetStatus(bSavedWithValidationErrors
        ? LOCTEXT("SavedWithErrors", "Table saved to disk with validation errors.")
        : LOCTEXT("Saved", "Table saved to disk."),
        CurrentErrors.IsEmpty() ? FLinearColor(.4f,1,.4f) : FLinearColor(1,.65f,.1f));
    return FReply::Handled();
}
FReply STMOPVehicleEditor::ReloadVehicle()
{
    if (!SelectedRowName.IsNone()) SelectVehicle(SelectedRowName);
    RefreshAnchorOptions(); RefreshLaneOptions(); RefreshEventOptions();
    RefreshTimeline(); return FReply::Handled();
}
FReply STMOPVehicleEditor::ValidateAll()
{
    CommitEntry(); CommitVehicle();
    const UDataTable* Table = VehicleTable.Get();
    if (!Table) return FReply::Handled();
    ValidationItems.Reset(); int32 Vehicles = 0; int32 Errors = 0; int32 Warnings = 0;
    for (FName RowName : Table->GetRowNames())
    {
        const auto* Row = RowName == SelectedRowName ? &WorkingRow :
            Table->FindRow<FTMOPHistoricalVehicleRow>(RowName, TEXT("ValidateAll"), false);
        if (!Row) continue;
        ++Vehicles;
        for (const FString& Message : ValidateRow(*Row))
        {
            if (Message.StartsWith(TEXT("ERROR"))) ++Errors;
            else if (Message.StartsWith(TEXT("WARNING"))) ++Warnings;
            auto Item = MakeShared<FValidationItem>(); Item->VehicleRow = RowName;
            Item->Message = Message + TEXT(" | ") + RowName.ToString();
            const int32 Start = Message.Find(TEXT("Timeline["));
            if (Start != INDEX_NONE) Item->EntryIndex = FCString::Atoi(*Message.Mid(Start + 9));
            ValidationItems.Add(Item);
        }
    }
    ValidationList->RequestListRefresh();
    SetStatus(FText::FromString(FString::Printf(TEXT("PRE-PLAY: %d vehicles, %d errors, %d warnings. Click an issue for its row. Static checks only; live boarding/traffic still require a test."),
        Vehicles, Errors, Warnings)), Errors > 0 ? FLinearColor(1,0.25f,0.15f) : FLinearColor(1.0f,0.7f,0.3f));
    return FReply::Handled();
}

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

    const bool bResolved = TMOPVehicleTimeline::ResolveEntry(Row, I, ResolveEvent, Out);
    if (!bResolved && Failure && Failure->IsEmpty()) *Failure = TEXT("Timeline time could not be resolved.");
    return bResolved;
}

bool STMOPVehicleEditor::ResolveEntryCompletionTime(
    const FTMOPHistoricalVehicleRow& Row, const int32 Index,
    int32& OutSecond, FString* Failure) const
{
    if (!ResolveTime(Row, Index, OutSecond, Failure)) return false;
    OutSecond += TMOPVehicleRoute::CompletionDelay(Row.Timeline[Index]);
    return true;
}

bool STMOPVehicleEditor::ResolveDrivingDepartureTime(
    const FTMOPHistoricalVehicleRow& Row, const int32 Index,
    int32& OutSecond, FString* Failure) const
{
    auto EventTime = [this, Failure](FName Id, int32& Second)
    {
        FTMOPHistoricalVehicleRow EventRow; FTMOPHistoricalVehicleTimelineEntry Entry;
        Entry.TimingMode = ETMOPEventTimingMode::Relative; Entry.SharedEventId = Id;
        EventRow.Timeline.Add(Entry);
        return ResolveTime(EventRow, 0, Second, Failure);
    };
    return TMOPVehicleTimeline::ResolveDeparture(Row, Index, EventTime, OutSecond);
}

bool STMOPVehicleEditor::CalculateDrive(
    const FTMOPHistoricalVehicleRow& Row, const int32 Index,
    double& Distance, int32& Departure, int32& Arrival,
    int32& Duration, double& Kmh, FString& Failure) const
{
    Distance = 0.0; Departure = Arrival = Duration = 0; Kmh = 0.0;
    if (!Row.Timeline.IsValidIndex(Index) || !IsDriving(Row.Timeline[Index].Action))
    { Failure = TEXT("Select a driving row."); return false; }
    if (!ResolveDrivingDepartureTime(Row, Index, Departure, &Failure)) return false;
    bool bArrival = false;
    if (Row.Timeline[Index].bTimeIsArrival)
        bArrival = ResolveTime(Row, Index, Arrival, &Failure);
    else for (int32 Next = Index + 1; Next < Row.Timeline.Num(); ++Next)
    {
        if (IsDriving(Row.Timeline[Next].Action)) break;
        if (IsStop(Row.Timeline[Next].Action))
        { bArrival = ResolveTime(Row, Next, Arrival, &Failure); break; }
    }
    if (!bArrival || Arrival <= Departure)
    { Failure = TEXT("Arrival must be later than departure. Set both times on the driving row."); return false; }
    FTMOPVehicleRoutePlan Plan;
    if (!TMOPVehicleRoute::Build(GEditor ? GEditor->GetEditorWorldContext().World() : nullptr,
        Row, Index, Plan, Failure)) return false;
    Distance = Plan.LengthCm; Duration = Arrival - Departure;
    Kmh = Distance * 0.036 / Duration;
    return true;
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

        if (HasPlacement(Entry.Action) &&
            Entry.PlacementMode == ETMOPHistoricalVehiclePlacementMode::Anchor &&
            Entry.PlacementAnchorId.IsNone())
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: placement anchor is missing."), Index));
        else if (HasPlacement(Entry.Action) && Entry.PlacementMode == ETMOPHistoricalVehiclePlacementMode::Anchor &&
            !Entry.PlacementAnchorId.IsNone() &&
            !KnownAnchors.Contains(Entry.PlacementAnchorId))
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: anchor '%s' is not present in the open level."),
                Index, *Entry.PlacementAnchorId.ToString()));

        if (Entry.Action == ETMOPHistoricalVehicleAction::OffscreenTransfer &&
            Entry.OffscreenTransferDurationSeconds < 0)
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: offscreen transfer duration cannot be negative."),
                Index));
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

        if (Entry.Action == ETMOPHistoricalVehicleAction::OffscreenTransfer &&
            ResolvedSecond != INDEX_NONE)
        {
            const int32 RevealSecond = ResolvedSecond +
                FMath::Max(0, Entry.OffscreenTransferDurationSeconds);
            for (int32 LaterIndex = Index + 1;
                LaterIndex < Row.Timeline.Num(); ++LaterIndex)
            {
                const FTMOPHistoricalVehicleTimelineEntry& Later =
                    Row.Timeline[LaterIndex];
                if (!IsDriving(Later.Action)) continue;
                int32 DepartureSecond = INDEX_NONE;
                if (ResolveDrivingDepartureTime(Row, LaterIndex, DepartureSecond) &&
                    DepartureSecond < RevealSecond)
                    Results.Add(FString::Printf(
                        TEXT("ERROR Timeline[%d]: driving begins at %s, before offscreen transfer [%d] finishes at %s."),
                        LaterIndex, *FormatClockSecond(DepartureSecond), Index,
                        *FormatClockSecond(RevealSecond)));
                break;
            }
        }

        if (!IsDriving(Entry.Action)) continue;
        if (Entry.bTimeIsArrival && Index == 0 &&
            (!Entry.bUseExplicitDepartureTime ||
             Entry.DepartureTimingMode ==
                ETMOPEventTimingMode::RelativeToPreviousEntry))
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
            if (Entry.VehicleRouteMode != ETMOPVehicleRouteMode::AnchorManeuver && !LaneId.IsNone() && !Lanes.Contains(LaneId))
                Results.Add(FString::Printf(
                    TEXT("ERROR Timeline[%d]: route lane '%s' is missing."),
                    Index, *LaneId.ToString()));
        if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute &&
            Entry.OrderedLaneIds.IsEmpty())
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: driving route has no lanes."), Index));

        if (Entry.VehicleRouteMode ==
                ETMOPVehicleRouteMode::AnchorManeuver &&
            Entry.RouteStartAnchorId.IsNone())
            Results.Add(FString::Printf(
                TEXT("WARNING Timeline[%d]: Anchor Maneuver inherits its start from the previous placement; set Route Start Anchor ID explicitly for clarity."),
                Index));

        for (int32 LaneIndex = 0;
            Entry.VehicleRouteMode == ETMOPVehicleRouteMode::ManualLaneRoute &&
            LaneIndex + 1 < Entry.OrderedLaneIds.Num(); ++LaneIndex)
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

        if (TMOPVehicleRoute::Driver(Row, Entry).IsNone())
            Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: no driver on this row or in the vehicle defaults."), Index));
        if (Arrival > Departure)
        {
            if (Index > 0)
            {
                int32 PreviousCompletion;
                if (ResolveEntryCompletionTime(Row, Index-1, PreviousCompletion) && Departure < PreviousCompletion)
                    Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: departure %s precedes the previous row's completion %s."),
                        Index, *FormatClockSecond(Departure), *FormatClockSecond(PreviousCompletion)));
            }
            for (int32 Next = Index + 1; Next < Row.Timeline.Num(); ++Next)
            {
                const auto& Following = Row.Timeline[Next];
                if (IsDriving(Following.Action))
                {
                    int32 NextDeparture;
                    if (ResolveDrivingDepartureTime(Row, Next, NextDeparture) && NextDeparture < Arrival)
                        Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: overlaps the previous driving route, which arrives at %s."),
                            Next, *FormatClockSecond(Arrival)));
                    break;
                }
                if (!IsStop(Following.Action)) continue;
                int32 StopTime;
                if (ResolveTime(Row, Next, StopTime) && StopTime < Arrival)
                    Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: stop interrupts the route before arrival %s."),
                        Next, *FormatClockSecond(Arrival)));
                if (!Entry.RouteDestinationAnchorId.IsNone() &&
                    Following.PlacementMode == ETMOPHistoricalVehiclePlacementMode::Anchor &&
                    Entry.RouteDestinationAnchorId != Following.PlacementAnchorId)
                    Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: stop anchor '%s' differs from the route end '%s'."),
                        Next, *Following.PlacementAnchorId.ToString(), *Entry.RouteDestinationAnchorId.ToString()));
                break;
            }
        }
        if (Entry.VehicleRouteMode == ETMOPVehicleRouteMode::AnchorManeuver)
        {
            FTMOPVehicleRoutePlan Plan; FString RouteFailure; FHitResult Hit;
            UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
            if (TMOPVehicleRoute::Build(World, Row, Index, Plan, RouteFailure) &&
                TMOPVehicleRoute::FindObstacle(World, Plan, FVector(202.5,90,50), Hit))
                Results.Add(FString::Printf(TEXT("WARNING Timeline[%d]: maneuver footprint visually intersects '%s'. Static scenery is ignored by default during authored driving; adjust the curve only if you want to avoid visual clipping."),
                    Index, *GetNameSafe(Hit.GetActor())));
        }

        // Compare intended end/start positions. A successful earlier arrival is assumed;
        // this cannot certify that the vehicle will actually get there during play.
        {
            UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
            FTMOPVehicleRoutePlan CurrentPlan; FString PlanFailure;
            if (TMOPVehicleRoute::Build(World, Row, Index, CurrentPlan, PlanFailure) &&
                !CurrentPlan.Samples.IsEmpty())
            {
                TOptional<FVector> PreviousPosition;
                for (int32 Earlier = Index - 1; Earlier >= 0; --Earlier)
                {
                    const auto& Prior = Row.Timeline[Earlier];
                    if (Prior.Action == ETMOPHistoricalVehicleAction::Despawn)
                    {
                        Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: driving follows Despawn without a new placement/spawn. FIX: add Spawn before this route."), Index));
                        break;
                    }
                    if (IsDriving(Prior.Action))
                    {
                        FTMOPVehicleRoutePlan PriorPlan; FString PriorFailure;
                        if (TMOPVehicleRoute::Build(World, Row, Earlier, PriorPlan, PriorFailure))
                            PreviousPosition = PriorPlan.Destination.GetLocation();
                        else Results.Add(FString::Printf(TEXT("WARNING Timeline[%d]: previous route [%d] is invalid; start continuity is unverified. FIX: repair that earlier route first."), Index, Earlier));
                        break;
                    }
                    if (HasPlacement(Prior.Action))
                    {
                        if (Prior.PlacementMode == ETMOPHistoricalVehiclePlacementMode::WorldTransform)
                            PreviousPosition = Prior.WorldTransform.GetLocation();
                        else if (World)
                            for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
                                if (It->GetAnchorId() == Prior.PlacementAnchorId)
                                {
                                    PreviousPosition = (Prior.AnchorLocalOffset * FTransform(
                                        It->GetAnchorRotation(), It->GetAnchorLocation())).GetLocation();
                                    break;
                                }
                        break;
                    }
                }
                if (PreviousPosition.IsSet())
                {
                    const double Gap = FVector::Distance(PreviousPosition.GetValue(), CurrentPlan.Samples[0].GetLocation());
                    if (Gap > 300.0)
                        Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: planned start is %.2f m from the previous endpoint/placement; runtime rejects starts beyond 3 m. FIX: align the start anchor and offsets, or add a connecting maneuver."), Index, Gap / 100.0));
                }
            }
        }

        if (!PeopleTable.IsValid())
        {
            Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: no People table loaded; driver and boarding cannot be validated."), Index));
            continue;
        }
        if (Departure <= 0 || Arrival <= Departure) continue;
        TMap<FName, FName> SeatOwners;
        TSet<FName> PeopleInside;
        TSet<FName> EnabledPeople;
        TArray<FName> DriversAtDeparture;
        const UDataTable* People = PeopleTable.Get();
        for (const FName PersonRowName : People->GetRowNames())
        {
            const FTMOPPersonProfileRow* Person =
                People->FindRow<FTMOPPersonProfileRow>(PersonRowName,
                    TEXT("VehicleEditorOccupantValidation"), false);
            if (Person == nullptr || !Person->bSpawnInSimulation) continue;
            const FName PersonId = Person->EntityId.IsNone()
                ? PersonRowName : Person->EntityId;
            EnabledPeople.Add(PersonId);
            bool bInside = false;
            bool bTargetsVehicleBoarding = false;
            int32 LatestBoardingSecond = INDEX_NONE;
            FName SeatId = NAME_None;
            for (int32 PersonIndex = 0;
                PersonIndex < Person->Timeline.Num(); ++PersonIndex)
            {
                const FTMOPPersonTimelineEntry& PersonEntry =
                    Person->Timeline[PersonIndex];
                int32 PersonSecond = INDEX_NONE;
                if (!ResolvePersonTime(*Person, PersonIndex, PersonSecond))
                    continue;
                if (PersonSecond <= Departure)
                {
                    if (PersonEntry.Action == ETMOPPersonTimelineAction::Despawn)
                    { bInside = false; SeatId = NAME_None; continue; }
                    if (PersonEntry.Action == ETMOPPersonTimelineAction::ExitVehicle)
                    { bInside = false; SeatId = NAME_None; }
                    else if ((PersonEntry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
                              PersonEntry.LocationType == ETMOPPersonLocationType::VehicleSeat) &&
                             PersonEntry.TargetEntityId != Row.VehicleId)
                    { bInside = false; SeatId = NAME_None; }
                }
                if (PersonEntry.TargetEntityId != Row.VehicleId) continue;
                if (PersonEntry.Action == ETMOPPersonTimelineAction::EnterVehicle)
                {
                    bTargetsVehicleBoarding = true;
                    if (PersonSecond <= Departure)
                        LatestBoardingSecond = FMath::Max(
                            LatestBoardingSecond, PersonSecond);
                }
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
            bool bAlreadyCarriedByEarlierSegment = false;
            if (bInside && LatestBoardingSecond != INDEX_NONE)
            {
                for (int32 EarlierIndex = 0; EarlierIndex < Index;
                    ++EarlierIndex)
                {
                    if (!IsDriving(Row.Timeline[EarlierIndex].Action)) continue;
                    int32 EarlierDeparture = INDEX_NONE;
                    if (ResolveDrivingDepartureTime(Row, EarlierIndex,
                        EarlierDeparture) &&
                        EarlierDeparture >= LatestBoardingSecond &&
                        EarlierDeparture < Departure)
                    {
                        bAlreadyCarriedByEarlierSegment = true;
                        break;
                    }
                }
            }
            FBoardingFeasibility Boarding;
            if (bTargetsVehicleBoarding && !bAlreadyCarriedByEarlierSegment &&
                CalculateBoardingFeasibility(Row, Index, *Person, Boarding))
            {
                if (Boarding.MarginSeconds < -10 &&
                    !Boarding.bUsedStraightLineFallback)
                    Results.Add(FString::Printf(
                        TEXT("WARNING Timeline[%d]: '%s' cannot reach the car by departure; needs %d s, has %d s (%d s late)."),
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
        const FName RequiredDriver = TMOPVehicleRoute::Driver(Row, Entry);
        if (!RequiredDriver.IsNone() && !EnabledPeople.Contains(RequiredDriver))
            Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: driver '%s' is missing from the loaded People table or disabled for simulation."),
                Index, *RequiredDriver.ToString()));
        const FName* FrontLeft = SeatOwners.Find(FName(TEXT("FRONT_LEFT")));
        if (!RequiredDriver.IsNone() && PeopleInside.Contains(RequiredDriver) &&
            (!FrontLeft || *FrontLeft != RequiredDriver))
            Results.Add(FString::Printf(TEXT("ERROR Timeline[%d]: driver '%s' is not assigned to FRONT_LEFT."), Index, *RequiredDriver.ToString()));
        if (!RequiredDriver.IsNone() && !PeopleInside.Contains(RequiredDriver))
            Results.Add(FString::Printf(
                TEXT("ERROR Timeline[%d]: driver '%s' is not inside the vehicle at departure %s; this route cannot start."),
                Index, *RequiredDriver.ToString(), *FormatClockSecond(Departure)));
        if (!RequiredDriver.IsNone() && !DriversAtDeparture.IsEmpty() &&
            !DriversAtDeparture.Contains(RequiredDriver))
            Results.Add(FString::Printf(
                TEXT("WARNING Timeline[%d]: Begin Driving belongs to another person, expected '%s'."),
                Index, *RequiredDriver.ToString()));
        for (const FName PassengerId : Entry.PassengerEntityIds)
            if (!PassengerId.IsNone() && !PeopleInside.Contains(PassengerId))
                Results.Add(FString::Printf(
                    TEXT("%s Timeline[%d]: listed passenger '%s' is not seated at departure%s."),
                    Entry.bWaitForListedOccupants ? TEXT("ERROR") : TEXT("WARNING"),
                    Index, *PassengerId.ToString(), Entry.bWaitForListedOccupants
                        ? TEXT("; this route waits for that person") : TEXT("")));
    }
    for (FString& Issue : Results)
    {
        if (Issue.Contains(TEXT("FIX:"))) continue;
        if (Issue.Contains(TEXT("driver"), ESearchCase::IgnoreCase) || Issue.Contains(TEXT("FRONT_LEFT")))
            Issue += TEXT(" FIX: check Driver Entity Id, enable the person, and set Enter Vehicle to this Vehicle Id / FRONT_LEFT before departure; check later Exit Vehicle or Despawn rows.");
        else if (Issue.Contains(TEXT("boarding"), ESearchCase::IgnoreCase) || Issue.Contains(TEXT("seated")) || Issue.Contains(TEXT("seat '")))
            Issue += TEXT(" FIX: inspect the person's Enter/Exit Vehicle target, seat and timing; allow enough travel time and avoid duplicate seat assignments.");
        else if (Issue.Contains(TEXT("maneuver footprint")))
            Issue += TEXT(" FIX (visual only with Ignore Static Scenery enabled): adjust the curve or accept the scenery overlap. This approximate check does not certify dynamic obstacle clearance.");
        else if (Issue.Contains(TEXT("anchor"), ESearchCase::IgnoreCase))
            Issue += TEXT(" FIX: verify the Anchor ID in the open level (not only its Outliner label), select it again, and recalculate the route. Check stop and route-end offsets.");
        else if (Issue.Contains(TEXT("lane"), ESearchCase::IgnoreCase))
            Issue += TEXT(" FIX: select valid lanes in the correct direction, check their allowed connectors, then recalculate the route.");
        else if (Issue.Contains(TEXT("speed"), ESearchCase::IgnoreCase))
            Issue += TEXT(" FIX: allow more travel time or shorten the route; check for an unintended detour.");
        else if (Issue.Contains(TEXT("time"), ESearchCase::IgnoreCase) || Issue.Contains(TEXT("arrival"), ESearchCase::IgnoreCase) || Issue.Contains(TEXT("departure"), ESearchCase::IgnoreCase) || Issue.Contains(TEXT("overlap")))
            Issue += TEXT(" FIX: check departure, arrival, relative offsets and shared events. The next drive must not start before the previous action completes.");
        else if (Issue.Contains(TEXT("Entry ID")))
            Issue += TEXT(" FIX: give this timeline row a non-empty unique Entry ID.");
    }
    return Results;
}

void STMOPVehicleEditor::RebuildValidation()
{
    // Validation must not commit stale buffers during a selection/command refresh.
    CachedFingerprints.Reset();
    CurrentErrors = ValidateRow(WorkingRow);
    CachedDrivingSummary = BuildDrivingSummary();
    CachedRouteEndpoints = BuildRouteEndpointsText();
    CachedOccupants = BuildOccupantsText(SelectedTimelineIndex);
    RefreshValidationItems();
}

void STMOPVehicleEditor::RebuildRoutePreview()
{
    if (!RouteMap.IsValid()) return;
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    TArray<TArray<FVector2D>> Network, Route;
    TArray<FName> NetworkIds;
    if (World) for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Components; It->GetComponents(Components);
        for (auto* Lane : Components)
        {
            if (!Lane || Lane->LaneId.IsNone()) continue;
            TArray<FVector2D> Points;
            const int32 Steps = FMath::Max(2, FMath::CeilToInt(Lane->GetSplineLength()/300.0f));
            for (int32 Step = 0; Step <= Steps; ++Step)
                Points.Add(FVector2D(Lane->GetLaneLocationAtDistance(Lane->GetSplineLength()*float(Step)/Steps)));
            Network.Add(MoveTemp(Points)); NetworkIds.Add(Lane->LaneId);
        }
    }
    int32 DriveIndex = SelectedTimelineIndex;
    while (WorkingRow.Timeline.IsValidIndex(DriveIndex) && !IsDriving(WorkingRow.Timeline[DriveIndex].Action)) --DriveIndex;
    FString Failure; TOptional<FVector2D> Start, End, Placement;
    PreviewPlan = FTMOPVehicleRoutePlan();
    if (TMOPVehicleRoute::Build(World, WorkingRow, DriveIndex, PreviewPlan, Failure))
    {
        TArray<FVector2D> Points;
        for (const auto& Pose : PreviewPlan.Samples) Points.Add(FVector2D(Pose.GetLocation()));
        Route.Add(MoveTemp(Points));
        Start = FVector2D(PreviewPlan.Samples[0].GetLocation()); End = FVector2D(PreviewPlan.Destination.GetLocation());
        double Distance, Kmh; int32 Duration;
        CalculateDrive(WorkingRow, DriveIndex, Distance, PreviewDeparture, PreviewArrival, Duration, Kmh, Failure);
    }
    else { PreviewPlan = FTMOPVehicleRoutePlan(); PreviewDeparture = PreviewArrival = 0; bPreviewPlaying = false; }
    const auto* Selected = WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)
        ? &WorkingRow.Timeline[SelectedTimelineIndex] : nullptr;
    if (Selected && HasPlacement(Selected->Action))
    {
        if (Selected->PlacementMode == ETMOPHistoricalVehiclePlacementMode::WorldTransform)
            Placement = FVector2D(Selected->WorldTransform.GetLocation());
        else if (World) for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
            if (It->GetAnchorId() == Selected->PlacementAnchorId)
                Placement = FVector2D((Selected->AnchorLocalOffset * FTransform(
                    It->GetAnchorRotation(), It->GetAnchorLocation())).GetLocation());
    }
    RouteMap->SetRoute(MoveTemp(Network), MoveTemp(NetworkIds), MoveTemp(Route),
        Failure.IsEmpty() ? TEXT("Shift-click: start lane | Ctrl-click: end lane | Right-click: via lane") : Failure,
        Start, End, Placement);
    RouteMap->SetPlan(PreviewPlan);
    SetPreviewAlpha(PreviewAlpha);
}

FText STMOPVehicleEditor::GetLiveDriverStatusText() const
{
    if (SelectedRowName.IsNone())
        return LOCTEXT("DriverSelectVehicle", "DRIVER CHECK: select a vehicle.");
    const FTMOPHistoricalVehicleTimelineEntry EmptyEntry;
    const auto& Entry = WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)
        ? WorkingRow.Timeline[SelectedTimelineIndex] : EmptyEntry;
    const FName Expected = TMOPVehicleRoute::Driver(WorkingRow, Entry);
    if (Expected.IsNone())
        return LOCTEXT("DriverNotSpecified", "DRIVER CHECK: no expected driver specified for this row/vehicle.");
    UWorld* PlayWorld = GEditor ? GEditor->PlayWorld : nullptr;
    if (!PlayWorld)
        return FText::FromString(FString::Printf(TEXT(
            "DRIVER CHECK: expected %s. Actual seating is NOT VERIFIED. Run Play/Simulate; the list below is a timeline prediction."),
            *Expected.ToString()));
    ATMOPVehicleBase* Vehicle = nullptr;
    for (TActorIterator<ATMOPHistoricalVehicleDirector> It(PlayWorld); It; ++It)
    {
        Vehicle = It->FindHistoricalVehicle(WorkingRow.VehicleId);
        if (IsValid(Vehicle)) break;
    }
    if (!IsValid(Vehicle))
        return LOCTEXT("DriverVehicleAbsent", "LIVE NOW: vehicle is not spawned/found. Driver seating cannot be checked.");
    ATMOPHistoricalAgent* Driver = Vehicle->GetDriverAgent();
    if (!IsValid(Driver))
        return FText::FromString(FString::Printf(TEXT(
            "LIVE NOW: DRIVER SEAT EMPTY — expected %s. The route cannot start in this state."), *Expected.ToString()));
    if (!IsValid(Driver->EntityIdentity) || Driver->EntityIdentity->EntityId.IsNone())
        return LOCTEXT("DriverIdentityUnknown", "LIVE NOW: driver seat occupied, but the occupant has no valid EntityId. Driver match cannot be verified.");
    const FName Actual = Driver->EntityIdentity->EntityId;
    if (Actual == Expected)
        return FText::FromString(FString::Printf(
            TEXT("LIVE NOW: MATCH — %s is registered in the driver seat. This checks the current play time, not a future departure or route clearance."),
            *Actual.ToString()));
    return FText::FromString(FString::Printf(
        TEXT("LIVE NOW: WRONG DRIVER — seated %s; expected %s for the selected row."),
        *Actual.ToString(), *Expected.ToString()));
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
        int32 LatestBoardingSecond = INDEX_NONE;
        FName SeatId = NAME_None;
        for (int32 Index = 0; Index < Person->Timeline.Num(); ++Index)
        {
            const FTMOPPersonTimelineEntry& Entry = Person->Timeline[Index];
            if (Entry.TargetEntityId != WorkingRow.VehicleId) continue;
            int32 Second = 0;
            if (!ResolvePersonTime(*Person, Index, Second)) continue;
            if (Entry.Action == ETMOPPersonTimelineAction::EnterVehicle)
            {
                bHasBoarding = true;
                if (Second <= At)
                    LatestBoardingSecond = FMath::Max(
                        LatestBoardingSecond, Second);
            }
            if (Second > At) continue;
            if (Entry.Action == ETMOPPersonTimelineAction::EnterVehicle ||
                Entry.LocationType == ETMOPPersonLocationType::VehicleSeat)
            { bInside = true; SeatId = Entry.TargetSeatId; }
            if (Entry.Action == ETMOPPersonTimelineAction::ExitVehicle)
            { bInside = false; SeatId = NAME_None; }
        }
        FString FeasibilityText;
        bool bAlreadyCarriedByEarlierSegment = false;
        if (bDriving && bInside && LatestBoardingSecond != INDEX_NONE)
        {
            for (int32 EarlierIndex = 0; EarlierIndex < TimelineIndex;
                ++EarlierIndex)
            {
                if (!IsDriving(WorkingRow.Timeline[EarlierIndex].Action))
                    continue;
                int32 EarlierDeparture = INDEX_NONE;
                if (ResolveDrivingDepartureTime(WorkingRow, EarlierIndex,
                    EarlierDeparture) &&
                    EarlierDeparture >= LatestBoardingSecond &&
                    EarlierDeparture < At)
                {
                    bAlreadyCarriedByEarlierSegment = true;
                    break;
                }
            }
        }
        if (bDriving && bHasBoarding && !bAlreadyCarriedByEarlierSegment)
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

FText STMOPVehicleEditor::GetTitle() const
{
    const FText Name = SelectedRowName.IsNone() ? LOCTEXT("Title", "TMOP Vehicle Editor")
        : (!WorkingRow.DisplayName.IsEmpty() ? WorkingRow.DisplayName : FText::FromName(WorkingRow.VehicleId));
    return FText::Format(LOCTEXT("VehicleEditorR6Title", "{0} [R6]"), Name);
}
FText STMOPVehicleEditor::GetSubtitle()const{return FText::FromString(WorkingRow.VehicleId.ToString()+FString::Printf(TEXT(" • %d timeline entries%s"),WorkingRow.Timeline.Num(), FTMOPHistoricalVehicleRow::StaticStruct()->CompareScriptStruct(&WorkingRow,&SavedRow,0)?TEXT(""):TEXT(" • UNSAVED")));}
FText STMOPVehicleEditor::GetValidationText()const{return CurrentErrors.IsEmpty()?LOCTEXT("NoErrors","No errors detected."):FText::FromString(FString::Join(CurrentErrors,TEXT("\n")));}
void STMOPVehicleEditor::SetStatus(const FText&Text,const FLinearColor&Color){if(StatusText.IsValid()){StatusText->SetText(Text);StatusText->SetColorAndOpacity(Color);}}


FReply STMOPVehicleEditor::OnKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
    if (Event.IsControlDown() && Event.GetKey() == EKeys::S) return SaveVehicle();
    return SCompoundWidget::OnKeyDown(Geometry, Event);
}
bool STMOPVehicleEditor::HasUnsavedChanges()
{
    CommitEntry(); CommitVehicle();
    return !SelectedRowName.IsNone() && !FTMOPHistoricalVehicleRow::StaticStruct()->CompareScriptStruct(
        &WorkingRow, &SavedRow, 0);
}
bool STMOPVehicleEditor::ConfirmDiscardOrSave()
{
    if (bConfirmingUnsavedChanges) return false;
    TGuardValue<bool> DialogGuard(bConfirmingUnsavedChanges, true);
    if (!HasUnsavedChanges()) return true;
    bPreviewPlaying = false;
    RefreshAppearancePreview();
    const EAppReturnType::Type Choice = FMessageDialog::Open(EAppMsgType::YesNoCancel,
        FText::Format(LOCTEXT("UnsavedNamedVehicle",
            "{0}\nRow: {1}\n\nThis vehicle has unsaved changes.\nYes: save to disk\nNo: discard changes\nCancel: keep editing"),
            GetTitle(), FText::FromName(SelectedRowName)));
    if (Choice == EAppReturnType::Cancel) return false;
    if (Choice == EAppReturnType::Yes) { SaveVehicle(); return !HasUnsavedChanges(); }
    WorkingRow = SavedRow; SyncDetailsFromWorking();
    return true;
}
bool STMOPVehicleEditor::CanClose()
{
    const bool bClose = ConfirmDiscardOrSave();
    if (bClose) bPreviewPlaying = false;
    return bClose;
}
void STMOPVehicleEditor::RefreshCommandBuffersFromWorking()
{
    // These snapshots are never bound to PropertyEditor. Commands still read
    // them, so keep them current without replacing any visible UObject.
    if (!SelectedRowName.IsNone())
    {
        auto Next = MakeShared<FStructOnScope>(FTMOPHistoricalVehicleRow::StaticStruct());
        auto* Data = reinterpret_cast<FTMOPHistoricalVehicleRow*>(Next->GetStructMemory());
        *Data = WorkingRow;
        Data->Timeline.Reset();
        VehicleStruct = Next;
    }
    else VehicleStruct.Reset();
    if (WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
    {
        auto Next = MakeShared<FStructOnScope>(FTMOPHistoricalVehicleTimelineEntry::StaticStruct());
        *reinterpret_cast<FTMOPHistoricalVehicleTimelineEntry*>(Next->GetStructMemory()) =
            WorkingRow.Timeline[SelectedTimelineIndex];
        EntryStruct = Next;
    }
    else EntryStruct.Reset();
}
void STMOPVehicleEditor::SyncDetailsFromWorking()
{
    RefreshCommandBuffersFromWorking();
    QueueDetailsRefresh();
}
void STMOPVehicleEditor::OnDetailsChanged(const FPropertyChangedEvent& Event, bool bVehicleDetails)
{
    if (bSynchronizingDetails || SelectedRowName.IsNone()) return;
    UObject* EditedObject = bVehicleDetails
        ? static_cast<UObject*>(VehicleDetailsObject.Get())
        : static_cast<UObject*>(EntryDetailsObject.Get());
    if (!EditedObject) return;
    const UObject* EventObject = Event.GetObjectBeingEdited(0);
    if (EventObject && EventObject != EditedObject) return;
    if (bPendingDetailsRefresh && PendingEditedObject.Get() != EditedObject) return;
    if (Event.ChangeType & EPropertyChangeType::Interactive) return;
    CommitEntry();
    if (bVehicleDetails && VehicleDetailsObject)
    {
        auto Details = VehicleDetailsObject->Data;
        Details.Timeline = WorkingRow.Timeline;
        WorkingRow = MoveTemp(Details);
    }
    else if (!bVehicleDetails && EntryDetailsObject && WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        WorkingRow.Timeline[SelectedTimelineIndex] = EntryDetailsObject->Data;
    bPreviewPlaying = false;
    SyncDetailsFromWorking();
    PendingEditedObject = EditedObject;
}
void STMOPVehicleEditor::RefreshAppearancePreview()
{
    if (!AppearancePreview) return;
    if (SelectedRowName.IsNone()) { AppearancePreview->Clear(TEXT("Select a vehicle.")); return; }
    AppearancePreview->ShowVehicle(WorkingRow);
    TGuardValue<bool> Guard(bRefreshingAccessories, true);
    AccessorySockets.Reset();
    for (const FName Socket : AppearancePreview->GetSockets())
        AccessorySockets.Add(MakeShared<FString>(Socket.ToString()));
    if (AccessorySocketCombo) AccessorySocketCombo->RefreshOptions();
}

TSharedRef<SWidget> STMOPVehicleEditor::BuildAccessoryControls()
{
    return SNew(SVerticalBox)
        .IsEnabled_Lambda([this]{return !SelectedRowName.IsNone();})
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("AddTaxiEquipment","+ Taxi"))
                .OnClicked(this,&STMOPVehicleEditor::AddAccessory,ETMOPRoofAccessoryType::TaxiSign) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("AddPoliceEquipment","+ Police"))
                .OnClicked(this,&STMOPVehicleEditor::AddAccessory,ETMOPRoofAccessoryType::PoliceLightbar) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("AddCustomEquipment","+ Custom"))
                .OnClicked(this,&STMOPVehicleEditor::AddAccessory,ETMOPRoofAccessoryType::Custom) ]
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("RemoveEquipment","Remove"))
                .OnClicked(this,&STMOPVehicleEditor::RemoveAccessory) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0,3)
        [ SAssignNew(AccessoryCombo,SComboBox<TSharedPtr<int32>>)
            .OptionsSource(&AccessoryChoices)
            .OnSelectionChanged(this,&STMOPVehicleEditor::SelectAccessory)
            .OnGenerateWidget_Lambda([this](TSharedPtr<int32> Item)->TSharedRef<SWidget>
            {
                const FString Label=Item.IsValid() && WorkingRow.AdditionalAccessories.IsValidIndex(*Item)
                    ? WorkingRow.AdditionalAccessories[*Item].AccessoryId.ToString() : TEXT("Roof Accessory (existing)");
                return SNew(STextBlock).Text(FText::FromString(Label));
            })
            [ SNew(STextBlock).Text_Lambda([this]
                {
                    return FText::FromString(WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex)
                        ? WorkingRow.AdditionalAccessories[SelectedAccessoryIndex].AccessoryId.ToString()
                        : TEXT("Roof Accessory (existing)"));
                }) ] ]
        + SVerticalBox::Slot().AutoHeight().Padding(0,0,0,3)
        [ SAssignNew(AccessorySocketCombo,SSearchableComboBox)
            .OptionsSource(&AccessorySockets)
            .OnGenerateWidget_Lambda([](TSharedPtr<FString> Item)->TSharedRef<SWidget>
                {return SNew(STextBlock).Text(FText::FromString(Item.IsValid()?*Item:FString()));})
            .OnSelectionChanged(this,&STMOPVehicleEditor::SelectAccessorySocket)
            [ SNew(STextBlock).Text_Lambda([this]
                {
                    const FName Socket=WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex)
                        ?WorkingRow.AdditionalAccessories[SelectedAccessoryIndex].SocketName:WorkingRow.RoofAccessory.SocketName;
                    return FText::FromString(TEXT("Socket: ")+Socket.ToString());
                }) ] ]
        + SVerticalBox::Slot().AutoHeight()
        [ SNew(SBox).HeightOverride(210)[AccessoryDetails.ToSharedRef()] ];
}

void STMOPVehicleEditor::RefreshAccessoryChoices()
{
    {
    TGuardValue<bool> Guard(bRefreshingAccessories,true);
    AccessoryChoices.Reset();
    AccessoryChoices.Add(MakeShared<int32>(INDEX_NONE));
    for(int32 Index=0;Index<WorkingRow.AdditionalAccessories.Num();++Index)
        AccessoryChoices.Add(MakeShared<int32>(Index));
    if(!WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex))SelectedAccessoryIndex=INDEX_NONE;
    if(AccessoryCombo)AccessoryCombo->RefreshOptions();
    }
    SelectAccessory(MakeShared<int32>(SelectedAccessoryIndex),ESelectInfo::Direct);
}

void STMOPVehicleEditor::SelectAccessory(TSharedPtr<int32> Item, ESelectInfo::Type)
{
    if (bRefreshingAccessories || !Item.IsValid()) return;
    TGuardValue<bool> Guard(bRefreshingAccessories, true);
    SelectedAccessoryIndex = *Item;
    QueueDetailsRefresh();
}
void STMOPVehicleEditor::OnAccessoryDetailsChanged(const FPropertyChangedEvent& Event)
{
    if (bSynchronizingDetails || bRefreshingAccessories || SelectedRowName.IsNone()) return;
    UObject* EditedObject = AccessoryDetailsObject
        ? static_cast<UObject*>(AccessoryDetailsObject.Get()) : static_cast<UObject*>(RoofDetailsObject.Get());
    if (!EditedObject) return;
    const UObject* EventObject = Event.GetObjectBeingEdited(0);
    if (EventObject && EventObject != EditedObject) return;
    if (bPendingDetailsRefresh && PendingEditedObject.Get() != EditedObject) return;
    if (Event.ChangeType & EPropertyChangeType::Interactive) return;
    CommitEntry(); CommitVehicle();
    if (AccessoryDetailsObject && WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex))
        WorkingRow.AdditionalAccessories[SelectedAccessoryIndex] = AccessoryDetailsObject->Data;
    else if (RoofDetailsObject) WorkingRow.RoofAccessory = RoofDetailsObject->Data;
    // The active transform widget owns handles into EditedObject until its
    // notification stack has completely unwound. Do not queue SetObject here:
    // the visible UObject already contains the new value. Only refresh the
    // unbound command snapshots and the separate 3D preview.
    RefreshCommandBuffersFromWorking();
    RefreshAppearancePreview();
}
FReply STMOPVehicleEditor::AddAccessory(ETMOPRoofAccessoryType Type)
{
    if(SelectedRowName.IsNone())return FReply::Handled();
    CommitEntry();CommitVehicle();
    FTMOPVehicleAccessoryVisual NewPart;
    NewPart.Type=Type;
    const FString Prefix=Type==ETMOPRoofAccessoryType::TaxiSign?TEXT("TaxiSign"):
        Type==ETMOPRoofAccessoryType::PoliceLightbar?TEXT("PoliceLightbar"):TEXT("Accessory");
    int32 Suffix=1;
    do {NewPart.AccessoryId=FName(*FString::Printf(TEXT("%s_%02d"),*Prefix,Suffix++));}
    while(WorkingRow.AdditionalAccessories.ContainsByPredicate([&](const FTMOPVehicleAccessoryVisual& Part)
        {return Part.AccessoryId==NewPart.AccessoryId;}));
    SelectedAccessoryIndex=WorkingRow.AdditionalAccessories.Add(NewPart);
    SyncDetailsFromWorking();RefreshAppearancePreview();RefreshAccessoryChoices();
    SetStatus(LOCTEXT("ChooseEquipmentMesh","Accessory added. Choose Mesh, Socket and Local Transform; then Save Vehicle."),
        FLinearColor(0.9f,0.8f,0.3f));
    return FReply::Handled();
}
FReply STMOPVehicleEditor::RemoveAccessory()
{
    if(SelectedRowName.IsNone())return FReply::Handled();
    CommitEntry();CommitVehicle();
    if(WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex))
        WorkingRow.AdditionalAccessories.RemoveAt(SelectedAccessoryIndex);
    else WorkingRow.RoofAccessory=FTMOPRoofAccessoryVisual();
    SelectedAccessoryIndex=INDEX_NONE;
    SyncDetailsFromWorking();RefreshAppearancePreview();RefreshAccessoryChoices();
    return FReply::Handled();
}
void STMOPVehicleEditor::SelectAccessorySocket(TSharedPtr<FString> Item,ESelectInfo::Type)
{
    if(bRefreshingAccessories||!Item.IsValid()||SelectedRowName.IsNone())return;
    CommitEntry();CommitVehicle();
    if(WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex))
        WorkingRow.AdditionalAccessories[SelectedAccessoryIndex].SocketName=FName(**Item);
    else WorkingRow.RoofAccessory.SocketName=FName(**Item);
    SyncDetailsFromWorking();RefreshAppearancePreview();RefreshAccessoryChoices();
}

EVisibility STMOPVehicleEditor::DrivingVisibility() const
{
    return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) &&
        IsDriving(WorkingRow.Timeline[SelectedTimelineIndex].Action) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility STMOPVehicleEditor::LaneVisibility() const
{
    return DrivingVisibility() == EVisibility::Visible &&
        WorkingRow.Timeline[SelectedTimelineIndex].VehicleRouteMode != ETMOPVehicleRouteMode::AnchorManeuver
        ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility STMOPVehicleEditor::PlacementVisibility() const
{
    return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) &&
        HasPlacement(WorkingRow.Timeline[SelectedTimelineIndex].Action) ? EVisibility::Visible : EVisibility::Collapsed;
}
EVisibility STMOPVehicleEditor::StopVisibility() const
{
    return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) &&
        IsStop(WorkingRow.Timeline[SelectedTimelineIndex].Action) ? EVisibility::Visible : EVisibility::Collapsed;
}
// MSVC can report C4702 inside this nested Slate declarative expression even
// though its widget branches are all reachable. Keep the suppression local.
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable: 4702)
#endif
TSharedRef<SWidget> STMOPVehicleEditor::BuildDrivingControls()
{
    return SNew(SVerticalBox)
    + SVerticalBox::Slot().AutoHeight()
    [
        SNew(SVerticalBox).Visibility(this, &STMOPVehicleEditor::DrivingVisibility)
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth()
            [ SNew(SButton).Text(LOCTEXT("FollowLanes","Follow lanes"))
                .OnClicked_Lambda([this]()
                {
                    CommitEntry();
                    WorkingRow.Timeline[SelectedTimelineIndex].VehicleRouteMode = ETMOPVehicleRouteMode::AutomaticToAnchor;
                    SyncDetailsFromWorking(); FString Failure; RecalculateSelectedRoute(Failure); RefreshTimeline();
                    return FReply::Handled();
                }) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(4,0)
            [ SNew(SButton).Text(LOCTEXT("FreeManeuver","Free anchor maneuver"))
                .OnClicked_Lambda([this]()
                {
                    CommitEntry();
                    WorkingRow.Timeline[SelectedTimelineIndex].VehicleRouteMode = ETMOPVehicleRouteMode::AnchorManeuver;
                    SyncDetailsFromWorking(); RefreshTimeline(); return FReply::Handled();
                }) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0,4)
        [ SNew(STextBlock).Text(LOCTEXT("VehicleOwnsRouteAlways",
            "Starts automatically at departure when driver and required passengers are seated")) ]
        + SVerticalBox::Slot().AutoHeight()
        [
            SNew(SHorizontalBox)
            + SHorizontalBox::Slot().AutoWidth().Padding(0,0,5,0)
            [ SNew(STextBlock).Text(LOCTEXT("DepartureClock","Depart")) ]
            + SHorizontalBox::Slot().FillWidth(1)
            [ SNew(SEditableTextBox).Text(this, &STMOPVehicleEditor::GetClockField, false)
                .HintText(LOCTEXT("ClockHint","HH:MM:SS"))
                .ToolTipText(LOCTEXT("SetAbsDeparture","Entering a clock time changes departure to Absolute. Shared-event settings remain available in Details."))
                .OnTextCommitted(this, &STMOPVehicleEditor::SetClockField, false) ]
            + SHorizontalBox::Slot().AutoWidth().Padding(8,0,5,0)
            [ SNew(STextBlock).Text(LOCTEXT("ArrivalClock","Arrive")) ]
            + SHorizontalBox::Slot().FillWidth(1)
            [ SNew(SEditableTextBox).Text(this, &STMOPVehicleEditor::GetClockField, true)
                .HintText(LOCTEXT("ClockHint","HH:MM:SS"))
                .ToolTipText(LOCTEXT("SetAbsArrival","Entering a clock time changes arrival to Absolute. To remain linked to an event, use its offset in Details."))
                .OnTextCommitted(this, &STMOPVehicleEditor::SetClockField, true) ]
        ]
        + SVerticalBox::Slot().AutoHeight().Padding(0,3)
        [ SNew(STextBlock).Text(this, &STMOPVehicleEditor::GetDrivingSummary).AutoWrapText(true) ]
    ]
    + SVerticalBox::Slot().AutoHeight()
    [
        SNew(SHorizontalBox).Visibility(this, &STMOPVehicleEditor::StopVisibility)
        + SHorizontalBox::Slot().AutoWidth()
        [ SNew(SCheckBox)
            .IsChecked_Lambda([this]() { return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) &&
                WorkingRow.Timeline[SelectedTimelineIndex].bUseStopDuration ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
            .OnCheckStateChanged_Lambda([this](ECheckBoxState State)
            {
                if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)) return;
                CommitEntry(); auto& Entry = WorkingRow.Timeline[SelectedTimelineIndex];
                Entry.bUseStopDuration = State == ECheckBoxState::Checked;
                const int32 Seconds = Entry.StopDurationSeconds; SyncDetailsFromWorking();
                if (State == ECheckBoxState::Checked) SetStopDuration(Seconds); else RefreshTimeline();
            })
            [ SNew(STextBlock).Text(LOCTEXT("StopFor","Stand still for")) ] ]
        + SHorizontalBox::Slot().AutoWidth().Padding(5,0)
        [ SNew(SBox).WidthOverride(75)
            [ SNew(SSpinBox<int32>).MinValue(0).MaxValue(86400)
                .Value_Lambda([this]() { return WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) ?
                    WorkingRow.Timeline[SelectedTimelineIndex].StopDurationSeconds : 10; })
                .OnValueCommitted_Lambda([this](int32 Seconds, ETextCommit::Type) { SetStopDuration(Seconds); }) ] ]
        + SHorizontalBox::Slot().AutoWidth()
        [ SNew(STextBlock).Text(LOCTEXT("StopSeconds","seconds; next drive inherits departure")) ]
    ];
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
FText STMOPVehicleEditor::GetClockField(bool bArrival) const
{
    if (!WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex)) return FText::GetEmpty();
    int32 Seconds = INDEX_NONE;
    if (!bArrival) ResolveDrivingDepartureTime(WorkingRow, SelectedTimelineIndex, Seconds);
    else if (WorkingRow.Timeline[SelectedTimelineIndex].bTimeIsArrival)
        ResolveTime(WorkingRow, SelectedTimelineIndex, Seconds);
    else for (int32 Next = SelectedTimelineIndex + 1; Next < WorkingRow.Timeline.Num(); ++Next)
    {
        if (IsDriving(WorkingRow.Timeline[Next].Action)) break;
        if (IsStop(WorkingRow.Timeline[Next].Action)) { ResolveTime(WorkingRow, Next, Seconds); break; }
    }
    return Seconds == INDEX_NONE ? FText::GetEmpty() : FText::FromString(FormatClockSecond(Seconds));
}
void STMOPVehicleEditor::SetClockField(const FText& Text, ETextCommit::Type CommitType, bool bArrival)
{
    if (CommitType == ETextCommit::OnCleared || DrivingVisibility() != EVisibility::Visible) return;
    TArray<FString> Parts; Text.ToString().TrimStartAndEnd().ParseIntoArray(Parts, TEXT(":"));
    if (Parts.Num() != 3 || !Parts[0].IsNumeric() || !Parts[1].IsNumeric() || !Parts[2].IsNumeric())
    { SetStatus(LOCTEXT("InvalidClock","Use HH:MM:SS, for example 23:21:25."), FLinearColor::Red); return; }
    for (const FString& Part : Parts)
        for (TCHAR Character : Part)
            if (!FChar::IsDigit(Character))
            { SetStatus(LOCTEXT("ClockDigits","Use whole hours, minutes and seconds."), FLinearColor::Red); return; }
    const int32 Hour = FCString::Atoi(*Parts[0]), Minute = FCString::Atoi(*Parts[1]), Second = FCString::Atoi(*Parts[2]);
    if (Hour < 0 || Hour > 23 || Minute < 0 || Minute > 59 || Second < 0 || Second > 59)
    { SetStatus(LOCTEXT("ClockRange","Clock time must be between 00:00:00 and 23:59:59."), FLinearColor::Red); return; }
    CommitEntry();
    auto& Entry = WorkingRow.Timeline[SelectedTimelineIndex];
    const bool bWasArrival = Entry.bTimeIsArrival;
    int32 ExistingStopIndex = INDEX_NONE, ExistingStopTime = INDEX_NONE;
    if (!bWasArrival) for (int32 Next = SelectedTimelineIndex + 1; Next < WorkingRow.Timeline.Num(); ++Next)
    {
        if (IsDriving(WorkingRow.Timeline[Next].Action)) break;
        if (IsStop(WorkingRow.Timeline[Next].Action))
        {
            if (ResolveTime(WorkingRow, Next, ExistingStopTime)) ExistingStopIndex = Next;
            break;
        }
    }
    Entry.bTimeIsArrival = true;
    if (!bWasArrival)
    {
        Entry.bUseExplicitDepartureTime = true;
        Entry.DepartureTimingMode = Entry.TimingMode;
        Entry.DepartureTime = Entry.Time;
        Entry.DepartureSharedEventId = Entry.SharedEventId;
        Entry.DepartureOffsetSeconds = Entry.EventOffsetSeconds;
        // Preserve the next stop's arrival when converting a legacy departure row.
        for (int32 Next = SelectedTimelineIndex + 1; Next < WorkingRow.Timeline.Num(); ++Next)
        {
            if (IsDriving(WorkingRow.Timeline[Next].Action)) break;
            if (IsStop(WorkingRow.Timeline[Next].Action))
            {
                int32 ExistingArrival;
                if (ResolveTime(WorkingRow, Next, ExistingArrival))
                { Entry.Time = FTMOPTime::FromSecondsFromMidnight(ExistingArrival); Entry.TimingMode = ETMOPEventTimingMode::Absolute; }
                break;
            }
        }
    }
    if (bArrival)
    {
        Entry.Time = FTMOPTime(Hour, Minute, Second);
        Entry.TimingMode = ETMOPEventTimingMode::Absolute;
    }
    else
    {
        Entry.bUseExplicitDepartureTime = true;
        Entry.DepartureTime = FTMOPTime(Hour, Minute, Second);
        Entry.DepartureTimingMode = ETMOPEventTimingMode::Absolute;
    }
    if (!bWasArrival && WorkingRow.Timeline.IsValidIndex(ExistingStopIndex) &&
        ExistingStopIndex == SelectedTimelineIndex + 1)
    {
        auto& FollowingStop = WorkingRow.Timeline[ExistingStopIndex];
        if (FollowingStop.TimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
            FollowingStop.EventOffsetSeconds = 0; // It already supplied the legacy arrival.
    }
    SyncDetailsFromWorking(); RefreshTimeline();
}
void STMOPVehicleEditor::SetStopDuration(int32 Seconds)
{
    if (StopVisibility() != EVisibility::Visible) return;
    CommitEntry();
    auto& Stop = WorkingRow.Timeline[SelectedTimelineIndex];
    Stop.bUseStopDuration = true; Stop.StopDurationSeconds = FMath::Max(0, Seconds);
    const int32 Next = SelectedTimelineIndex + 1;
    if (WorkingRow.Timeline.IsValidIndex(Next))
    {
        auto& Drive = WorkingRow.Timeline[Next];
        if (!IsDriving(Drive.Action))
        {
            SetStatus(LOCTEXT("StopNeedsAdjacentDrive", "Place the driving row immediately below this stop to inherit its duration."),
                FLinearColor(1.0f,0.5f,0.0f));
        }
        else if (Drive.bTimeIsArrival)
        {
            Drive.bUseExplicitDepartureTime = true;
            Drive.DepartureTimingMode = ETMOPEventTimingMode::RelativeToPreviousEntry;
            Drive.DepartureOffsetSeconds = 0;
        }
        else
        {
            Drive.TimingMode = ETMOPEventTimingMode::RelativeToPreviousEntry;
            // The shared resolver adds the preceding stop duration.
            Drive.EventOffsetSeconds = 0;
        }
    }
    SyncDetailsFromWorking(); RefreshTimeline();
}
FText STMOPVehicleEditor::GetDrivingSummary() const { return CachedDrivingSummary; }
FText STMOPVehicleEditor::BuildDrivingSummary() const
{
    if (DrivingVisibility() != EVisibility::Visible) return FText::GetEmpty();
    double Distance, Kmh; int32 Departure, Arrival, Duration; FString Failure;
    const auto& Entry = WorkingRow.Timeline[SelectedTimelineIndex];
    const FName DriverId = TMOPVehicleRoute::Driver(WorkingRow, Entry);
    FString Text = FString::Printf(TEXT("Driver: %s%s | %s"),
        *DriverId.ToString(), Entry.DriverEntityId.IsNone() ? TEXT(" (vehicle default)") : TEXT(""),
        TEXT("Vehicle timeline (automatic)"));
    if (CalculateDrive(WorkingRow, SelectedTimelineIndex, Distance, Departure, Arrival, Duration, Kmh, Failure))
        Text += FString::Printf(TEXT("\n%d s | %.1f m | %.1f km/h average. Timeline controls arrival."),
            Duration, Distance / 100.0, Kmh);
    else Text += TEXT("\nNot ready: ") + Failure;
    if (DriverId.IsNone()) Text += TEXT("\nSet a driver on the vehicle or this row.");
    return FText::FromString(Text);
}
void STMOPVehicleEditor::SetPreviewAlpha(float Alpha)
{
    PreviewAlpha = FMath::Clamp(Alpha, 0.0f, 1.0f);
    if (PreviewPlan.Samples.IsEmpty()) return;
    const bool bStopVia = WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex) &&
        WorkingRow.Timeline[SelectedTimelineIndex].bStopAtViaAnchors;
    const FTransform Pose = PreviewPlan.Sample(TMOPVehicleRoute::DistanceAtTime(PreviewPlan, PreviewAlpha, bStopVia));
    if (RouteMap.IsValid()) RouteMap->SetGhost(Pose);
    if (bPreviewInLevel && GEditor)
    {
        UWorld* World = GEditor->GetEditorWorldContext().World();
        if (!World) return;
        const float Lifetime = bPreviewPlaying ? 0.06f : 1.0f;
        DrawDebugBox(World, Pose.GetLocation() + FVector(0,0,80), FVector(225,90,65),
            Pose.GetRotation(), FColor::White, false, Lifetime, 0, 4.0f);
        DrawDebugDirectionalArrow(World, Pose.GetLocation()+FVector(0,0,150),
            Pose.GetLocation()+FVector(0,0,150)+Pose.GetRotation().GetForwardVector()*300,
            75, FColor::Cyan, false, Lifetime, 0, 4.0f);
        for (int32 Index = 1; Index < PreviewPlan.Samples.Num(); ++Index)
            DrawDebugLine(World, PreviewPlan.Samples[Index-1].GetLocation()+FVector(0,0,15),
                PreviewPlan.Samples[Index].GetLocation()+FVector(0,0,15), FColor::Cyan, false, Lifetime);
        GEditor->RedrawLevelEditingViewports();
    }
}
FReply STMOPVehicleEditor::TogglePreview()
{
    if (PreviewPlan.Samples.IsEmpty()) return FReply::Handled();
    bPreviewPlaying = !bPreviewPlaying;
    if (bPreviewPlaying)
    {
        if (PreviewAlpha >= 1.0f) PreviewAlpha = 0.0f;
        if (!bPreviewTimerRegistered)
        {
            bPreviewTimerRegistered = true;
            RegisterActiveTimer(0.033f, FWidgetActiveTimerDelegate::CreateSP(this, &STMOPVehicleEditor::TickPreview));
        }
    }
    return FReply::Handled();
}
EActiveTimerReturnType STMOPVehicleEditor::TickPreview(double, float DeltaTime)
{
    if (!bPreviewPlaying) { bPreviewTimerRegistered = false; return EActiveTimerReturnType::Stop; }
    SetPreviewAlpha(PreviewAlpha + DeltaTime / FMath::Max(1, PreviewArrival - PreviewDeparture));
    if (PreviewAlpha >= 1.0f) { bPreviewPlaying = false; bPreviewTimerRegistered = false; return EActiveTimerReturnType::Stop; }
    return EActiveTimerReturnType::Continue;
}
FString STMOPVehicleEditor::GetEntryFingerprint(int32 Index) const
{
    for (; Index >= 0 && WorkingRow.Timeline.IsValidIndex(Index); --Index)
        if (IsDriving(WorkingRow.Timeline[Index].Action))
        {
            if (const FString* Cached = CachedFingerprints.Find(Index)) return *Cached;
            double Distance, Kmh; int32 Departure, Arrival, Duration; FString Failure;
            FTMOPVehicleRoutePlan Plan;
            if (CalculateDrive(WorkingRow, Index, Distance, Departure, Arrival, Duration, Kmh, Failure) &&
                TMOPVehicleRoute::Build(GEditor ? GEditor->GetEditorWorldContext().World() : nullptr, WorkingRow, Index, Plan, Failure))
            {
                const FString Fingerprint = TMOPVehicleRoute::Fingerprint(WorkingRow, Plan, Departure, Arrival);
                CachedFingerprints.Add(Index, Fingerprint); return Fingerprint;
            }
            CachedFingerprints.Add(Index, TEXT("invalid-current-route"));
            return TEXT("invalid-current-route");
        }
    return TEXT("no-driving-row");
}
void STMOPVehicleEditor::RefreshValidationItems()
{
    ValidationItems.Reset();
    for (const FString& Message : CurrentErrors)
    {
        auto Item = MakeShared<FValidationItem>(); Item->VehicleRow = SelectedRowName; Item->Message = Message;
        const int32 Start = Message.Find(TEXT("Timeline["));
        if (Start != INDEX_NONE) Item->EntryIndex = FCString::Atoi(*Message.Mid(Start + 9));
        ValidationItems.Add(Item);
    }
    if (ValidationItems.IsEmpty())
    {
        auto Item = MakeShared<FValidationItem>(); Item->VehicleRow = SelectedRowName;
        Item->Message = TEXT("No static issues detected. Actual boarding and collision-free execution are NOT verified; run a validation test."); ValidationItems.Add(Item);
    }
    if (ValidationList.IsValid()) ValidationList->RequestListRefresh();
}
TSharedRef<ITableRow> STMOPVehicleEditor::GenerateValidationRow(
    FValidationItemPtr Item, const TSharedRef<STableViewBase>& Owner)
{
    return SNew(STableRow<FValidationItemPtr>, Owner)
    [
        SNew(SButton).ButtonStyle(FAppStyle::Get(), "SimpleButton")
        .OnClicked_Lambda([this, Item]()
        {
            if (!Item.IsValid()) return FReply::Handled();
            if (Item->VehicleRow != SelectedRowName) SelectVehicle(Item->VehicleRow);
            if (Item->VehicleRow == SelectedRowName && WorkingRow.Timeline.IsValidIndex(Item->EntryIndex))
            {
                SelectTimelineEntry(Item->EntryIndex);
                if (TimelineItems.IsValidIndex(Item->EntryIndex))
                { TimelineList->SetSelection(TimelineItems[Item->EntryIndex]); TimelineList->RequestScrollIntoView(TimelineItems[Item->EntryIndex]); }
            }
            return FReply::Handled();
        })
        [ SNew(STextBlock).Text(FText::FromString(Item.IsValid() ? Item->Message : FString()))
            .AutoWrapText(true)
            .ColorAndOpacity(Item.IsValid() && Item->Message.StartsWith(TEXT("ERROR")) ?
                FLinearColor(1,0.25f,0.15f) : FLinearColor(1,0.75f,0.3f)) ]
    ];
}


STMOPVehicleEditor::~STMOPVehicleEditor()
{
    bSynchronizingDetails = true;
    for (const auto& View : {VehicleDetails, EntryDetails, AccessoryDetails})
        if (View.IsValid())
        {
            View->OnFinishedChangingProperties().RemoveAll(this);
            View->SetObject(nullptr);
        }
    VehicleStruct.Reset(); EntryStruct.Reset();
}
void STMOPVehicleEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
    UDataTable* Vehicles = VehicleTable.Get();
    UDataTable* People = PeopleTable.Get();
    UDataTable* Events = EventTable.Get();
    Collector.AddReferencedObject(Vehicles);
    Collector.AddReferencedObject(People);
    Collector.AddReferencedObject(Events);
    Collector.AddReferencedObject(VehicleDetailsObject);
    Collector.AddReferencedObject(EntryDetailsObject);
    Collector.AddReferencedObject(AccessoryDetailsObject);
    Collector.AddReferencedObject(RoofDetailsObject);
    Collector.AddPropertyReferencesWithStructARO(FTMOPHistoricalVehicleRow::StaticStruct(), &WorkingRow, nullptr);
    Collector.AddPropertyReferencesWithStructARO(FTMOPHistoricalVehicleRow::StaticStruct(), &SavedRow, nullptr);
    for (const auto& Data : {VehicleStruct, EntryStruct})
        if (Data.IsValid()) Data->AddReferencedObjects(Collector);
}
void STMOPVehicleEditor::SyncVehicleListSelection()
{
    if (!VehicleList.IsValid()) return;
    TGuardValue<bool> SelectionGuard(bChangingSelection, true);
    for (const auto& Item : VehicleItems)
        if (Item.IsValid() && *Item == SelectedRowName)
        {
            VehicleList->SetSelection(Item, ESelectInfo::Direct);
            return;
        }
    VehicleList->ClearSelection();
}
void STMOPVehicleEditor::QueueDetailsRefresh()
{
    if (bSynchronizingDetails) return;
    PendingEditedObject.Reset();
    if (bPendingDetailsRefresh) return;
    bPendingDetailsRefresh = true;
    RegisterActiveTimer(0.0f, FWidgetActiveTimerDelegate::CreateSP(
        this, &STMOPVehicleEditor::ApplyPendingDetailsRefresh));
}
EActiveTimerReturnType STMOPVehicleEditor::ApplyPendingDetailsRefresh(double, float)
{
    // A modal loop can pump timers while the original input handler is active.
    if (bChangingSelection || bConfirmingUnsavedChanges) return EActiveTimerReturnType::Continue;
    TGuardValue<bool> Guard(bSynchronizingDetails, true);
    UE_LOG(LogTemp, Log, TEXT("TMOP Vehicle Editor refresh begin: row=%s entry=%d"), *SelectedRowName.ToString(), SelectedTimelineIndex);
    RefreshTimeline();
    RefreshAppearancePreview();
    RefreshAccessoryChoices();
    VehicleDetailsObject = nullptr;
    EntryDetailsObject = nullptr;
    AccessoryDetailsObject = nullptr;
    RoofDetailsObject = nullptr;
    if (!SelectedRowName.IsNone())
    {
        VehicleDetailsObject = NewObject<UTMOPVehicleDetailsObject>(GetTransientPackage());
        VehicleDetailsObject->Data = WorkingRow;
        VehicleDetailsObject->Data.Timeline.Reset();
        if (WorkingRow.Timeline.IsValidIndex(SelectedTimelineIndex))
        {
            EntryDetailsObject = NewObject<UTMOPVehicleEntryDetailsObject>(GetTransientPackage());
            EntryDetailsObject->Data = WorkingRow.Timeline[SelectedTimelineIndex];
        }
        if (WorkingRow.AdditionalAccessories.IsValidIndex(SelectedAccessoryIndex))
        {
            AccessoryDetailsObject = NewObject<UTMOPVehicleAccessoryDetailsObject>(GetTransientPackage());
            AccessoryDetailsObject->Data = WorkingRow.AdditionalAccessories[SelectedAccessoryIndex];
        }
        else
        {
            RoofDetailsObject = NewObject<UTMOPVehicleRoofDetailsObject>(GetTransientPackage());
            RoofDetailsObject->Data = WorkingRow.RoofAccessory;
        }
    }
    VehicleDetails->SetObject(VehicleDetailsObject.Get());
    EntryDetails->SetObject(EntryDetailsObject.Get());
    AccessoryDetails->SetObject(AccessoryDetailsObject
        ? static_cast<UObject*>(AccessoryDetailsObject.Get()) : static_cast<UObject*>(RoofDetailsObject.Get()));
    UE_LOG(LogTemp, Log, TEXT("TMOP Vehicle Editor refresh complete: row=%s"), *SelectedRowName.ToString());
    bPendingDetailsRefresh = false;
    PendingEditedObject.Reset();
    return EActiveTimerReturnType::Stop;
}

#undef LOCTEXT_NAMESPACE
