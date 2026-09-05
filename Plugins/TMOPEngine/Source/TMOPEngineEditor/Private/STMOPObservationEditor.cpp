#include "STMOPObservationEditor.h"

#include "Anchors/TMOPHistoricalAnchor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Editor.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "IStructureDetailsView.h"
#include "InputCoreTypes.h"
#include "Misc/MessageDialog.h"
#include "Observations/TMOPObservationSignalementLibrary.h"
#include "People/TMOPPersonProfileTypes.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "UObject/StructOnScope.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SComboBox.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Input/SSlider.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"
#include "Traffic/TMOPTrafficLaneComponent.h"

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

bool IsUnknownPersonCategory(const FName CategoryId)
{
    const FString Category = CategoryId.ToString();
    return Category.Contains(TEXT("UNKNOWN"), ESearchCase::IgnoreCase) ||
        Category.Equals(TEXT("OBSERVED_PERSON"), ESearchCase::IgnoreCase);
}

FText GeoFilterDisplayName(const FName FilterId)
{
    if (FilterId == TEXT("Grand"))
        return LOCTEXT("GeoGrand", "Around Grand / Sandins");
    if (FilterId == TEXT("MurderScene"))
        return LOCTEXT("GeoMurderScene", "Around the murder scene");
    if (FilterId == TEXT("Kungsgatan"))
        return LOCTEXT("GeoKungsgatan", "Around Kungsgatan");
    if (FilterId == TEXT("Tegnersgatan"))
        return LOCTEXT("GeoTegnersgatan", "Around Tegnérgatan");
    if (FilterId == TEXT("Ridge"))
        return LOCTEXT("GeoRidge",
            "The ridge: David Bagares / Malmskillnads / Johannes / Regerings");
    if (FilterId == TEXT("East"))
        return LOCTEXT("GeoEast",
            "East: Birger Jarl / Smala gränd / Snickarbacken");
    return LOCTEXT("GeoAllPlayArea", "Entire play area");
}

TArray<FString> GeoFilterAnchorTokens(const FName FilterId)
{
    if (FilterId == TEXT("Grand"))
        return {TEXT("GRAND"), TEXT("SADIN"), TEXT("SANDIN")};
    if (FilterId == TEXT("MurderScene"))
        return {TEXT("MORD"), TEXT("SHOT"), TEXT("AFKXSVEA"),
            TEXT("TUNNELXSVEA"), TEXT("DEKORIMA")};
    if (FilterId == TEXT("Kungsgatan"))
        return {TEXT("KUNG"), TEXT("KING_CREOLE"), TEXT("KINGCREOLE")};
    if (FilterId == TEXT("Tegnersgatan"))
        return {TEXT("TEGNER"), TEXT("TEGNÉR")};
    if (FilterId == TEXT("Ridge"))
        return {TEXT("DAVID"), TEXT("MALMSKILL"), TEXT("JOHANNES"),
            TEXT("REGERING")};
    if (FilterId == TEXT("East"))
        return {TEXT("BIRGER"), TEXT("SMALA"), TEXT("SMÅLA"),
            TEXT("SNICKAR")};
    return {};
}

bool AnchorMatchesGeoTokens(const FName AnchorId,
    const TArray<FString>& Tokens)
{
    const FString Anchor = AnchorId.ToString();
    return Tokens.ContainsByPredicate([&Anchor](const FString& Token)
    {
        return Anchor.Contains(Token, ESearchCase::IgnoreCase);
    });
}

bool ParseEditorClockText(const FString& Input, int32& OutSecond)
{
    FString Value = Input;
    Value.TrimStartAndEndInline();
    int32 Hour = 0;
    int32 Minute = 0;
    int32 Second = 0;
    TArray<FString> Parts;
    Value.ParseIntoArray(Parts, TEXT(":"), true);
    if (Parts.Num() == 2 || Parts.Num() == 3)
    {
        Hour = FCString::Atoi(*Parts[0]);
        Minute = FCString::Atoi(*Parts[1]);
        Second = Parts.Num() == 3 ? FCString::Atoi(*Parts[2]) : 0;
    }
    else if (Parts.Num() == 1 && (Value.Len() == 4 || Value.Len() == 6))
    {
        Hour = FCString::Atoi(*Value.Left(2));
        Minute = FCString::Atoi(*Value.Mid(2, 2));
        Second = Value.Len() == 6 ? FCString::Atoi(*Value.Right(2)) : 0;
    }
    else
        return false;
    if (Hour < 0 || Hour > 23 || Minute < 0 || Minute > 59 ||
        Second < 0 || Second > 59)
        return false;
    OutSecond = Hour * 3600 + Minute * 60 + Second;
    return true;
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
DECLARE_DELEGATE_OneParam(FTMOPOnMapObservationSelected, FName);

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
        SLATE_EVENT(FTMOPOnMapObservationSelected, OnObservationSelected)
    SLATE_END_ARGS()
    void Construct(const FArguments& InArgs)
    {
        SelectionHandler = InArgs._OnObservationSelected;
    }

    struct FPoint
    {
        FName ObservationId;
        FVector2D Position;
        bool bLinked = false;
        bool bSelected = false;
        int32 Order = INDEX_NONE;
        ETMOPObservedEntityType EntityType = ETMOPObservedEntityType::Unknown;
    };

    void SetData(TArray<FPoint>&& InPoints,
        const TArray<TArray<FVector2D>>& InLanePolylines,
        const TOptional<FVector2D>& InPreviewPosition,
        const FString& InPreviewDescription)
    {
        Points = MoveTemp(InPoints);
        LanePolylines = InLanePolylines;
        PreviewPosition = InPreviewPosition;
        PreviewDescription = InPreviewDescription;
        Invalidate(EInvalidateWidgetReason::Paint);
    }

    virtual FReply OnMouseButtonDown(const FGeometry& Geometry,
        const FPointerEvent& Event) override
    {
        if (Event.GetEffectingButton() != EKeys::LeftMouseButton)
            return FReply::Unhandled();
        FBox2D Bounds(ForceInit);
        BuildBounds(Bounds);
        if (!Bounds.bIsValid) return FReply::Unhandled();
        const FVector2D LocalClick =
            Geometry.AbsoluteToLocal(Event.GetScreenSpacePosition());
        FName BestId = NAME_None;
        float BestDistanceSquared = FMath::Square(14.0f);
        for (const FPoint& Point : Points)
        {
            const float DistanceSquared = FVector2D::DistSquared(
                Project(Point.Position, Bounds, Geometry.GetLocalSize()),
                LocalClick);
            if (DistanceSquared <= BestDistanceSquared)
            {
                BestDistanceSquared = DistanceSquared;
                BestId = Point.ObservationId;
            }
        }
        if (!BestId.IsNone() && SelectionHandler.IsBound())
        {
            SelectionHandler.Execute(BestId);
            return FReply::Handled();
        }
        return FReply::Unhandled();
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
        BuildBounds(Bounds);
        if (!Bounds.bIsValid)
        {
            FSlateDrawElement::MakeText(Elements, Layer + 1,
                Geometry.ToPaintGeometry(FVector2D(14.0f), Geometry.GetLocalSize()),
                LOCTEXT("NoMapPoints", "No observation anchors found in the open level"),
                FAppStyle::GetFontStyle("NormalFont"), ESlateDrawEffect::None,
                FLinearColor(0.6f, 0.6f, 0.6f));
            return Layer + 1;
        }
        const FVector2D Size = Geometry.GetLocalSize();
        for (const TArray<FVector2D>& Lane : LanePolylines)
        {
            if (Lane.Num() < 2) continue;
            TArray<FVector2D> ProjectedLane;
            ProjectedLane.Reserve(Lane.Num());
            for (const FVector2D& Position : Lane)
                ProjectedLane.Add(Project(Position, Bounds, Size));
            FSlateDrawElement::MakeLines(Elements, Layer + 1,
                Geometry.ToPaintGeometry(), ProjectedLane,
                ESlateDrawEffect::None,
                FLinearColor(0.42f, 0.48f, 0.55f, 0.38f), true, 1.25f);
        }
        TArray<FPoint> Ordered = Points;
        Ordered.RemoveAll([](const FPoint& P) { return P.Order == INDEX_NONE; });
        Ordered.Sort([](const FPoint& A, const FPoint& B) { return A.Order < B.Order; });
        if (Ordered.Num() >= 2)
        {
            TArray<FVector2D> Track;
            for (const FPoint& P : Ordered)
                Track.Add(Project(P.Position, Bounds, Size));
            FSlateDrawElement::MakeLines(Elements, Layer + 2,
                Geometry.ToPaintGeometry(), Track, ESlateDrawEffect::None,
                FLinearColor(0.05f, 0.7f, 1.0f, 0.9f), true, 3.0f);
            for (int32 Index = 1; Index < Track.Num(); ++Index)
            {
                const FVector2D Direction =
                    (Track[Index] - Track[Index - 1]).GetSafeNormal();
                if (Direction.IsNearlyZero()) continue;
                const FVector2D Normal(-Direction.Y, Direction.X);
                const FVector2D Tip = Track[Index];
                TArray<FVector2D> Arrow = {
                    Tip - Direction * 10.0f + Normal * 5.0f,
                    Tip,
                    Tip - Direction * 10.0f - Normal * 5.0f
                };
                FSlateDrawElement::MakeLines(Elements, Layer + 3,
                    Geometry.ToPaintGeometry(), Arrow,
                    ESlateDrawEffect::None,
                    FLinearColor(0.05f, 0.7f, 1.0f, 0.95f),
                    false, 2.0f);
            }
        }
        for (const FPoint& Point : Points)
        {
            const FVector2D P = Project(Point.Position, Bounds, Size);
            const float Radius = Point.bSelected ? 7.0f : 4.0f;
            const FVector2D MarkerExtent =
                Point.EntityType == ETMOPObservedEntityType::Vehicle
                ? FVector2D(Radius * 1.65f, Radius)
                : FVector2D(Radius, Radius);
            const FLinearColor Color = Point.bSelected
                ? FLinearColor(0.05f, 0.8f, 1.0f)
                : Point.bLinked ? FLinearColor(0.2f, 0.85f, 0.35f)
                : FLinearColor(1.0f, 0.65f, 0.05f);
            FSlateDrawElement::MakeBox(Elements, Layer + 4,
                Geometry.ToPaintGeometry(P - MarkerExtent,
                    MarkerExtent * 2.0f), FAppStyle::GetBrush("WhiteBrush"),
                ESlateDrawEffect::None, Color);
        }
        if (PreviewPosition.IsSet())
        {
            const FVector2D P = Project(PreviewPosition.GetValue(), Bounds, Size);
            FSlateDrawElement::MakeBox(Elements, Layer + 5,
                Geometry.ToPaintGeometry(P - FVector2D(8.0f),
                    FVector2D(16.0f)), FAppStyle::GetBrush("WhiteBrush"),
                ESlateDrawEffect::None, FLinearColor::White);
            FSlateDrawElement::MakeBox(Elements, Layer + 6,
                Geometry.ToPaintGeometry(P - FVector2D(5.0f),
                    FVector2D(10.0f)), FAppStyle::GetBrush("WhiteBrush"),
                ESlateDrawEffect::None, FLinearColor(0.0f, 0.85f, 1.0f));
        }
        FSlateDrawElement::MakeText(Elements, Layer + 7,
            Geometry.ToPaintGeometry(FVector2D(10.0f), Size),
            FText::Format(LOCTEXT("MapCaption",
                "{0} visible points · {1} lanes · click a point for details"),
                FText::AsNumber(Points.Num()),
                FText::AsNumber(LanePolylines.Num())),
            FAppStyle::GetFontStyle("SmallFont"), ESlateDrawEffect::None,
            FLinearColor(0.75f, 0.78f, 0.82f));
        return Layer + 7;
    }

private:
    void BuildBounds(FBox2D& OutBounds) const
    {
        for (const TArray<FVector2D>& Lane : LanePolylines)
            for (const FVector2D& Position : Lane)
                OutBounds += Position;
        for (const FPoint& Point : Points) OutBounds += Point.Position;
        if (PreviewPosition.IsSet()) OutBounds += PreviewPosition.GetValue();
    }

    static FVector2D Project(const FVector2D& Position,
        const FBox2D& Bounds, const FVector2D& Size)
    {
        const FVector2D Extent = Bounds.GetSize();
        const float Padding = 24.0f;
        const float Scale = FMath::Min(
            (Size.X - Padding * 2.0f) / FMath::Max(Extent.X, 1.0f),
            (Size.Y - Padding * 2.0f) / FMath::Max(Extent.Y, 1.0f));
        return FVector2D(Padding + (Position.X - Bounds.Min.X) * Scale,
            Size.Y - Padding - (Position.Y - Bounds.Min.Y) * Scale);
    }

    TArray<FPoint> Points;
    TArray<TArray<FVector2D>> LanePolylines;
    TOptional<FVector2D> PreviewPosition;
    FString PreviewDescription;
    FTMOPOnMapObservationSelected SelectionHandler;
};

void STMOPObservationEditor::Construct(const FArguments& Args)
{
    GeoFilterItems = {
        MakeShared<FName>(TEXT("AllPlayArea")),
        MakeShared<FName>(TEXT("Grand")),
        MakeShared<FName>(TEXT("MurderScene")),
        MakeShared<FName>(TEXT("Kungsgatan")),
        MakeShared<FName>(TEXT("Tegnersgatan")),
        MakeShared<FName>(TEXT("Ridge")),
        MakeShared<FName>(TEXT("East"))
    };
    GeoFilterRadiusCm.Add(TEXT("Grand"), 18000.0f);
    GeoFilterRadiusCm.Add(TEXT("MurderScene"), 18000.0f);
    GeoFilterRadiusCm.Add(TEXT("Kungsgatan"), 12000.0f);
    GeoFilterRadiusCm.Add(TEXT("Tegnersgatan"), 12000.0f);
    GeoFilterRadiusCm.Add(TEXT("Ridge"), 16000.0f);
    GeoFilterRadiusCm.Add(TEXT("East"), 16000.0f);
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
            + SHorizontalBox::Slot().AutoWidth().Padding(4.0f, 0.0f)
            [ SNew(SButton).Text(LOCTEXT("ValidateAll", "Validate all"))
                .OnClicked(this, &STMOPObservationEditor::ValidateAll) ]
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
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 2)
                [ SNew(STextBlock).Text(LOCTEXT("FilterHeading",
                    "TIME + GEOGRAPHY FILTERS")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 3)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ SNew(SCheckBox)
                        .IsChecked_Lambda([this]()
                        {
                            return bAllTimes ? ECheckBoxState::Checked :
                                ECheckBoxState::Unchecked;
                        })
                        .OnCheckStateChanged(this,
                            &STMOPObservationEditor::OnAllTimesChanged)
                        [ SNew(STextBlock).Text(LOCTEXT("AllTimes", "All times")) ] ]
                    + SHorizontalBox::Slot().FillWidth(1.0f).Padding(6, 0, 2, 0)
                    [ SNew(SEditableTextBox)
                        .Text(FText::FromString(TEXT("22:55")))
                        .HintText(LOCTEXT("FromTime", "From HH:MM"))
                        .IsEnabled_Lambda([this]() { return !bAllTimes; })
                        .OnTextCommitted(this,
                            &STMOPObservationEditor::OnTimeStartCommitted) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ SNew(STextBlock).Text(FText::FromString(TEXT("–"))) ]
                    + SHorizontalBox::Slot().FillWidth(1.0f).Padding(2, 0, 0, 0)
                    [ SNew(SEditableTextBox)
                        .Text(FText::FromString(TEXT("23:55")))
                        .HintText(LOCTEXT("ToTime", "To HH:MM"))
                        .IsEnabled_Lambda([this]() { return !bAllTimes; })
                        .OnTextCommitted(this,
                            &STMOPObservationEditor::OnTimeEndCommitted) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 5)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.0f)
                    [
                        SAssignNew(GeoFilterCombo, SComboBox<FGeoFilterItem>)
                        .OptionsSource(&GeoFilterItems)
                        .OnGenerateWidget(this,
                            &STMOPObservationEditor::GenerateGeoFilterOption)
                        .OnSelectionChanged(this,
                            &STMOPObservationEditor::OnGeoFilterSelected)
                        [ SNew(STextBlock)
                            .Text(this,
                                &STMOPObservationEditor::GetSelectedGeoFilterText) ]
                    ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(4, 0, 0, 0)
                    .VAlign(VAlign_Center)
                    [ SNew(STextBlock).Text(LOCTEXT("RadiusLabel", "Radius m")) ]
                    + SHorizontalBox::Slot().AutoWidth().Padding(3, 0, 0, 0)
                    [ SNew(SSpinBox<float>)
                        .MinValue(20.0f).MaxValue(1000.0f)
                        .MinSliderValue(20.0f).MaxSliderValue(400.0f)
                        .Value(this,
                            &STMOPObservationEditor::GetGeoFilterRadiusMeters)
                        .IsEnabled_Lambda([this]()
                        {
                            return SelectedGeoFilter != TEXT("AllPlayArea");
                        })
                        .OnValueCommitted(this,
                            &STMOPObservationEditor::OnGeoFilterRadiusCommitted) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 5)
                [
                    SNew(SCheckBox)
                    .IsChecked(ECheckBoxState::Unchecked)
                    .OnCheckStateChanged(this,
                        &STMOPObservationEditor::OnMissingSignalementChanged)
                    [ SNew(STextBlock).Text(LOCTEXT("OnlyMissingSignalement",
                        "Only person observations needing signalement review")) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 3)
                [ SNew(STextBlock)
                    .Text(LOCTEXT("ObservationStatusLegend",
                        "● error   ▲ review   ✓ ready   green linked   blue known"))
                    .Font(FAppStyle::GetFontStyle("SmallFont"))
                    .ColorAndOpacity(FLinearColor(0.65f, 0.7f, 0.75f)) ]
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(LOCTEXT("Observations", "ALL OBSERVATIONS")) ]
                + SVerticalBox::Slot().FillHeight(1.0f)
                [ SAssignNew(ObservationList, SListView<FObservationItem>)
                    .ListItemsSource(&ObservationItems)
                    .OnGenerateRow(this, &STMOPObservationEditor::GenerateObservationRow)
                    .OnSelectionChanged(this,
                        &STMOPObservationEditor::OnObservationSelected) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 2)
                [ SNew(STextBlock).Text(LOCTEXT("IdentityHeading",
                    "SELECTED OBSERVATION: OBSERVED PERSON IDENTITY")) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 3)
                [ SNew(SSearchBox)
                    .HintText(LOCTEXT("KnownPersonSearch",
                        "Filter known people / witnesses"))
                    .OnTextChanged(this,
                        &STMOPObservationEditor::OnKnownPersonSearchChanged) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 3)
                [
                    SAssignNew(KnownPersonCombo, SComboBox<FPersonItem>)
                    .OptionsSource(&KnownPersonItems)
                    .MaxListHeight(420.0f)
                    .OnGenerateWidget(this,
                        &STMOPObservationEditor::GenerateKnownPersonOption)
                    .OnSelectionChanged(this,
                        &STMOPObservationEditor::OnKnownPersonSelected)
                    [ SNew(STextBlock)
                        .Text(this,
                            &STMOPObservationEditor::GetSelectedKnownPersonText) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 3)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().FillWidth(1.0f)
                    [ SNew(SButton)
                        .Text(LOCTEXT("MarkUnknown", "Mark UNKNOWN PERSON"))
                        .OnClicked(this,
                            &STMOPObservationEditor::MarkSelectedUnknownPerson) ]
                    + SHorizontalBox::Slot().FillWidth(1.0f).Padding(3, 0, 0, 0)
                    [ SNew(SButton)
                        .Text(LOCTEXT("MarkKnown", "Associate KNOWN PERSON"))
                        .OnClicked(this,
                            &STMOPObservationEditor::MarkSelectedKnownPerson) ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 3, 0, 2)
                [ SNew(STextBlock).Text(LOCTEXT("ObservationInfoHeading",
                    "SELECTED OBSERVATION DETAILS")) ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBox).HeightOverride(175.0f)
                    [
                        SNew(SBorder).Padding(5.0f)
                        [
                            SNew(SScrollBox)
                            + SScrollBox::Slot()
                            [ SAssignNew(ObservationInfoText, STextBlock)
                                .AutoWrapText(true)
                                .Text(LOCTEXT("NoObservationSelected",
                                    "Select an observation to see observers, subject, time, source, description, notes and signalement.")) ]
                        ]
                    ]
                ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 5)
                [ SNew(SButton).Text(LOCTEXT("AddSelected", "Add selected to link →"))
                    .OnClicked(this, &STMOPObservationEditor::AddSelectedObservation) ]
            ]
            + SSplitter::Slot().Value(0.42f)
            [
                SNew(SVerticalBox)
                + SVerticalBox::Slot().AutoHeight()
                [ SNew(STextBlock).Text(LOCTEXT("Map",
                    "OBSERVATION MAP · selected link only · lanes in background")) ]
                + SVerticalBox::Slot().FillHeight(0.56f)
                [ SAssignNew(Map, STMOPObservationMap)
                    .OnObservationSelected(this,
                        &STMOPObservationEditor::OnMapObservationSelected) ]
                + SVerticalBox::Slot().AutoHeight().Padding(0, 5, 0, 0)
                [
                    SNew(SHorizontalBox)
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ SNew(STextBlock).Text(LOCTEXT("PreviewLabel", "TRACK PREVIEW")) ]
                    + SHorizontalBox::Slot().FillWidth(1.0f).Padding(8, 0)
                    [ SNew(SSlider)
                        .Value(this,
                            &STMOPObservationEditor::GetPreviewSliderValue)
                        .OnValueChanged(this,
                            &STMOPObservationEditor::OnPreviewSliderChanged) ]
                    + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center)
                    [ SNew(STextBlock)
                        .Text(this,
                            &STMOPObservationEditor::GetPreviewTimeText) ]
                ]
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
                + SVerticalBox::Slot().AutoHeight().Padding(0, 6, 0, 2)
                [ SNew(STextBlock).Text(LOCTEXT("ValidationHeading",
                    "VALIDATION SUMMARY")) ]
                + SVerticalBox::Slot().AutoHeight()
                [
                    SNew(SBox).HeightOverride(155.0f)
                    [
                        SNew(SBorder).Padding(5.0f)
                        [
                            SNew(SScrollBox)
                            + SScrollBox::Slot()
                            [ SAssignNew(ValidationText, STextBlock)
                                .AutoWrapText(true)
                                .Text(LOCTEXT("ValidationNotRun",
                                    "Press Validate all to inspect observations and links.")) ]
                        ]
                    ]
                ]
                + SVerticalBox::Slot().FillHeight(0.46f).Padding(0, 6)
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
    PeopleTable = LoadObject<UDataTable>(nullptr, PeopleTablePath);
    if (ObservationTable.IsValid() && LinkTable.IsValid() &&
        PeopleTable.IsValid()) return;
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
        if (!PeopleTable.IsValid() &&
            Table->GetRowStruct() == FTMOPPersonProfileRow::StaticStruct())
            PeopleTable = Table;
    }
}

void STMOPObservationEditor::RefreshAll()
{
    ObservationsById.Reset();
    ObservationRowNamesById.Reset();
    if (UDataTable* Table = ObservationTable.Get())
    {
        for (const FName RowName : Table->GetRowNames())
        {
            const FTMOPObservationDefinition* Row =
                Table->FindRow<FTMOPObservationDefinition>(RowName,
                    TEXT("TMOPObservationEditor"), false);
            if (Row && !Row->ObservationId.IsNone())
            {
                ObservationsById.Add(Row->ObservationId, *Row);
                ObservationRowNamesById.Add(Row->ObservationId, RowName);
            }
        }
    }
    RefreshKnownPeople();
    RefreshLevelGeometry();
    RefreshLinks();
    RefreshObservations();
    RefreshMembers();
    RefreshObservationInfo();
    if (!ObservationTable.IsValid() || !LinkTable.IsValid() ||
        !PeopleTable.IsValid())
        SetStatus(LOCTEXT("MissingTables",
            "Create/import the observation, observation-link and people DataTables, then press Reload."),
            FLinearColor::Red);
}

void STMOPObservationEditor::RefreshLevelGeometry()
{
    AnchorPositions.Reset();
    CachedLanePolylines.Reset();
    GeoFilterSeedPositions.Reset();
    if (!GEditor) return;
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (World == nullptr) return;

    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        const FVector Location = It->GetActorLocation();
        const FVector2D Position(Location.X, Location.Y);
        const FName AnchorId = It->GetAnchorId();
        AnchorPositions.Add(AnchorId, Position);
        for (const FGeoFilterItem& FilterItem : GeoFilterItems)
        {
            if (!FilterItem || *FilterItem == TEXT("AllPlayArea")) continue;
            if (AnchorMatchesGeoTokens(
                    AnchorId, GeoFilterAnchorTokens(*FilterItem)))
                GeoFilterSeedPositions.FindOrAdd(*FilterItem).Add(Position);
        }
    }

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> LaneComponents;
        It->GetComponents<UTMOPTrafficLaneComponent>(LaneComponents);
        for (const UTMOPTrafficLaneComponent* Lane : LaneComponents)
        {
            if (!IsValid(Lane) || Lane->GetNumberOfSplinePoints() < 2)
                continue;
            const float Length = Lane->GetSplineLength();
            const int32 SegmentCount = FMath::Clamp(
                FMath::CeilToInt(Length / 400.0f), 1, 256);
            TArray<FVector2D>& Polyline =
                CachedLanePolylines.AddDefaulted_GetRef();
            Polyline.Reserve(SegmentCount + 1);
            for (int32 SegmentIndex = 0;
                SegmentIndex <= SegmentCount; ++SegmentIndex)
            {
                const FVector Location = Lane->GetLocationAtDistanceAlongSpline(
                    Length * static_cast<float>(SegmentIndex) /
                        static_cast<float>(SegmentCount),
                    ESplineCoordinateSpace::World);
                Polyline.Add(FVector2D(Location.X, Location.Y));
            }
        }
    }
}

void STMOPObservationEditor::RefreshKnownPeople()
{
    PersonDisplayNames.Reset();
    KnownPersonIds.Reset();
    if (UDataTable* Table = PeopleTable.Get())
    {
        static const FString Context(TEXT("TMOPObservationKnownPeople"));
        TArray<FTMOPPersonProfileRow*> Rows;
        Table->GetAllRows(Context, Rows);
        for (const FTMOPPersonProfileRow* Row : Rows)
        {
            if (Row == nullptr || Row->EntityId.IsNone() ||
                IsUnknownPersonCategory(Row->CategoryId))
                continue;
            KnownPersonIds.Add(Row->EntityId);
            PersonDisplayNames.Add(Row->EntityId,
                Row->FullName.IsEmpty() ? FText::FromName(Row->EntityId) :
                    Row->FullName);
        }
    }
    OnKnownPersonSearchChanged(FText::GetEmpty());
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
        if (!MatchesTimeFilter(Pair.Value) || !MatchesGeoFilter(Pair.Value))
            continue;
        const FString Haystack = BuildObservationSearchText(Pair.Value);
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
    RebuildMap();
}

bool STMOPObservationEditor::MatchesTimeFilter(
    const FTMOPObservationDefinition& Observation) const
{
    if (bAllTimes) return true;
    const int32 Second = Observation.CanonicalTime.ToSecondsFromMidnight();
    return TimeFilterStartSecond <= TimeFilterEndSecond
        ? Second >= TimeFilterStartSecond && Second <= TimeFilterEndSecond
        : Second >= TimeFilterStartSecond || Second <= TimeFilterEndSecond;
}

bool STMOPObservationEditor::CalculatePreviewPosition(
    FVector2D& OutPosition, FString& OutDescription) const
{
    if (WorkingLink.ObservationIds.IsEmpty()) return false;

    struct FPreviewPoint
    {
        FName ObservationId = NAME_None;
        const FTMOPObservationDefinition* Observation = nullptr;
        FVector2D Position = FVector2D::ZeroVector;
        int32 Second = 0;
    };
    TArray<FPreviewPoint> PreviewPoints;
    for (const FName ObservationId : WorkingLink.ObservationIds)
    {
        const FTMOPObservationDefinition* Observation =
            ObservationsById.Find(ObservationId);
        const FVector2D* Position = Observation != nullptr
            ? AnchorPositions.Find(Observation->ObservationAnchorId) : nullptr;
        if (Observation == nullptr || Position == nullptr) continue;
        FPreviewPoint& Point = PreviewPoints.AddDefaulted_GetRef();
        Point.ObservationId = ObservationId;
        Point.Observation = Observation;
        Point.Position = *Position;
        Point.Second = ResolveObservationSecond(ObservationId);
        if (PreviewPoints.Num() > 1 &&
            Point.Second < PreviewPoints[PreviewPoints.Num() - 2].Second)
            Point.Second += 24 * 3600;
    }
    if (PreviewPoints.IsEmpty()) return false;

    int32 EffectivePreviewSecond = PreviewSecond;
    if (PreviewPoints.Last().Second >= 24 * 3600 &&
        EffectivePreviewSecond < PreviewPoints[0].Second)
        EffectivePreviewSecond += 24 * 3600;
    if (EffectivePreviewSecond <= PreviewPoints[0].Second)
    {
        OutPosition = PreviewPoints[0].Position;
        OutDescription = TEXT("waiting for first observation");
        return true;
    }

    for (int32 Index = 0; Index + 1 < PreviewPoints.Num(); ++Index)
    {
        const FPreviewPoint& From = PreviewPoints[Index];
        const FPreviewPoint& To = PreviewPoints[Index + 1];
        const int32 FromEndSecond = From.Second +
            FMath::Max(1, From.Observation->ObservationDurationSeconds);
        double TravelStartSecond = FromEndSecond;
        const FTMOPObservationTrackSegment* Segment =
            WorkingLink.TrackSegments.FindByPredicate(
                [&From, &To](const FTMOPObservationTrackSegment& Candidate)
                {
                    return Candidate.FromObservationId == From.ObservationId &&
                        Candidate.ToObservationId == To.ObservationId;
                });
        ETMOPObservationSegmentMovementMode MovementMode = Segment != nullptr
            ? Segment->MovementMode
            : ETMOPObservationSegmentMovementMode::Automatic;
        if (MovementMode == ETMOPObservationSegmentMovementMode::Automatic)
            MovementMode = WorkingLink.LinkedEntityType ==
                    ETMOPObservedEntityType::Vehicle
                ? ETMOPObservationSegmentMovementMode::VehicleDirectInterpolation
                : ETMOPObservationSegmentMovementMode::Walk;
        float PreferredSpeed = Segment != nullptr &&
                Segment->PreferredTravelSpeedCmPerSecond > 0.0f
            ? Segment->PreferredTravelSpeedCmPerSecond
            : WorkingLink.PreferredTravelSpeedCmPerSecond;
        if (PreferredSpeed <= 0.0f)
            PreferredSpeed = MovementMode ==
                    ETMOPObservationSegmentMovementMode::Sprint ? 600.0f
                : MovementMode == ETMOPObservationSegmentMovementMode::Run ||
                  MovementMode == ETMOPObservationSegmentMovementMode::RunToVehicleAndBoard ||
                  MovementMode == ETMOPObservationSegmentMovementMode::ExitVehicleThenRun
                    ? 350.0f
                : MovementMode == ETMOPObservationSegmentMovementMode::VehicleLaneRoute ||
                  MovementMode == ETMOPObservationSegmentMovementMode::VehicleDirectInterpolation ||
                  MovementMode == ETMOPObservationSegmentMovementMode::RideInVehicle
                    ? 1200.0f : 140.0f;
        if (WorkingLink.TravelTimingMode ==
                ETMOPObservationTravelTimingMode::ArriveAtPreferredSpeed)
        {
            const double TravelDuration = FVector2D::Distance(
                From.Position, To.Position) / FMath::Max(PreferredSpeed, 1.0f);
            TravelStartSecond = FMath::Max(
                static_cast<double>(FromEndSecond),
                static_cast<double>(To.Second) - TravelDuration);
        }

        if (EffectivePreviewSecond <= TravelStartSecond)
        {
            OutPosition = From.Position;
            OutDescription = FString::Printf(TEXT("waiting at %s"),
                *From.Observation->ObservationAnchorId.ToString());
            return true;
        }
        if (EffectivePreviewSecond <= To.Second)
        {
            const double Duration = FMath::Max(
                0.001, static_cast<double>(To.Second) - TravelStartSecond);
            const float Alpha = FMath::Clamp(static_cast<float>(
                (EffectivePreviewSecond - TravelStartSecond) / Duration),
                0.0f, 1.0f);
            OutPosition = FMath::Lerp(From.Position, To.Position, Alpha);
            OutDescription = FString::Printf(TEXT("%s → %s · %.0f%%"),
                *From.Observation->ObservationAnchorId.ToString(),
                *To.Observation->ObservationAnchorId.ToString(), Alpha * 100.0f);
            return true;
        }
    }
    OutPosition = PreviewPoints.Last().Position;
    OutDescription = TEXT("track completed");
    return true;
}

bool STMOPObservationEditor::MatchesGeoFilter(
    const FTMOPObservationDefinition& Observation) const
{
    if (SelectedGeoFilter == TEXT("AllPlayArea")) return true;
    const FVector2D* ObservationPosition =
        AnchorPositions.Find(Observation.ObservationAnchorId);
    const TArray<FVector2D>* Seeds =
        GeoFilterSeedPositions.Find(SelectedGeoFilter);
    const float* Radius = GeoFilterRadiusCm.Find(SelectedGeoFilter);
    if (ObservationPosition != nullptr && Seeds != nullptr && Radius != nullptr)
    {
        const float RadiusSquared = FMath::Square(*Radius);
        if (Seeds->ContainsByPredicate(
            [ObservationPosition, RadiusSquared](const FVector2D& Seed)
            {
                return FVector2D::DistSquared(*ObservationPosition, Seed) <=
                    RadiusSquared;
            }))
            return true;
    }

    // Fallback keeps filters useful before the relevant level is open.
    return AnchorMatchesGeoTokens(Observation.ObservationAnchorId,
        GeoFilterAnchorTokens(SelectedGeoFilter));
}

FString STMOPObservationEditor::BuildObservationSearchText(
    const FTMOPObservationDefinition& Observation) const
{
    FString Result = Observation.ObservationId.ToString() + TEXT(" ") +
        Observation.DisplayName.ToString() + TEXT(" ") +
        Observation.ObservedEntityId.ToString() + TEXT(" ") +
        Observation.ObservationAnchorId.ToString() + TEXT(" ") +
        Observation.ObservedDescription + TEXT(" ") +
        Observation.SourceReference + TEXT(" ") + Observation.Notes;
    if (const FText* PersonName =
        PersonDisplayNames.Find(Observation.ObservedEntityId))
        Result += TEXT(" ") + PersonName->ToString();
    for (const FName ObserverId : Observation.ObserverEntityIds)
        Result += TEXT(" ") + ObserverId.ToString();
    for (const FTMOPObservationWitnessSignalement& Signalement :
        Observation.WitnessSignalements)
    {
        Result += TEXT(" ") + Signalement.ObserverEntityId.ToString() +
            TEXT(" ") + Signalement.OriginalSummary + TEXT(" ") +
            Signalement.ObservationConditions + TEXT(" ") +
            Signalement.SourceReference + TEXT(" ") +
            FString::FromInt(Signalement.EstimatedAgeMinimum) + TEXT(" ") +
            FString::FromInt(Signalement.EstimatedAgeMaximum) + TEXT(" ") +
            FString::SanitizeFloat(Signalement.EstimatedHeightMinimumCm) +
            TEXT(" ") +
            FString::SanitizeFloat(Signalement.EstimatedHeightMaximumCm);
        for (const FTMOPObservedSignalementTrait& Trait : Signalement.Traits)
        {
            Result += TEXT(" ") + Trait.OriginalText + TEXT(" ") +
                Trait.SourceReference + TEXT(" ") +
                StaticEnum<ETMOPSignalementTraitType>()->GetNameStringByValue(
                    static_cast<int64>(Trait.TraitType));
            for (const FName Value : Trait.NormalizedValues)
                Result += TEXT(" ") + Value.ToString();
            for (const FName Value : Trait.ExplicitlyExcludedValues)
                Result += TEXT(" ") + Value.ToString();
        }
    }
    return Result;
}

void STMOPObservationEditor::RefreshObservationInfo()
{
    if (!ObservationInfoText) return;
    const FTMOPObservationDefinition* Observation =
        ObservationsById.Find(SelectedObservationId);
    if (Observation == nullptr)
    {
        ObservationInfoText->SetText(LOCTEXT("NoObservationInfo",
            "Select an observation to see its complete information."));
        return;
    }

    FString Observers;
    for (const FName ObserverId : Observation->ObserverEntityIds)
    {
        if (!Observers.IsEmpty()) Observers += TEXT(", ");
        const FText* ObserverName = PersonDisplayNames.Find(ObserverId);
        Observers += ObserverName != nullptr
            ? FString::Printf(TEXT("%s [%s]"),
                *ObserverName->ToString(), *ObserverId.ToString())
            : ObserverId.ToString();
    }
    const TCHAR* IdentityText =
        Observation->ObservedPersonIdentityStatus ==
            ETMOPObservedPersonIdentityStatus::KnownPerson
            ? TEXT("KNOWN PERSON")
        : Observation->ObservedPersonIdentityStatus ==
            ETMOPObservedPersonIdentityStatus::UnknownPerson
            ? TEXT("UNKNOWN PERSON") : TEXT("UNCLASSIFIED");
    FString Info = FString::Printf(
        TEXT("%s\nTime: %s\nAnchor: %s\nObserved: %s\nIdentity: %s\nObservers: %s\n\nDescription:\n%s\n\nSource:\n%s"),
        *Observation->DisplayName.ToString(),
        *FormatObservationTime(*Observation),
        *Observation->ObservationAnchorId.ToString(),
        *Observation->ObservedEntityId.ToString(), IdentityText,
        Observers.IsEmpty() ? TEXT("None") : *Observers,
        *Observation->ObservedDescription,
        *Observation->SourceReference);
    if (!Observation->Notes.IsEmpty())
        Info += TEXT("\n\nNotes:\n") + Observation->Notes;
    for (int32 Index = 0;
        Index < Observation->WitnessSignalements.Num(); ++Index)
    {
        const FTMOPObservationWitnessSignalement& Signalement =
            Observation->WitnessSignalements[Index];
        Info += FString::Printf(TEXT("\n\nSignalement %d — %s:\n%s"),
            Index + 1, *Signalement.ObserverEntityId.ToString(),
            *Signalement.OriginalSummary);
        if (Signalement.EstimatedAgeMinimum > 0 ||
            Signalement.EstimatedAgeMaximum > 0)
            Info += FString::Printf(TEXT("\nEstimated age: %d–%d"),
                Signalement.EstimatedAgeMinimum,
                Signalement.EstimatedAgeMaximum);
        if (Signalement.EstimatedHeightMinimumCm > 0.0f ||
            Signalement.EstimatedHeightMaximumCm > 0.0f)
            Info += FString::Printf(TEXT("\nEstimated height: %.0f–%.0f cm"),
                Signalement.EstimatedHeightMinimumCm,
                Signalement.EstimatedHeightMaximumCm);
        if (!Signalement.ObservationConditions.IsEmpty())
            Info += TEXT("\nConditions: ") + Signalement.ObservationConditions;
        for (const FTMOPObservedSignalementTrait& Trait : Signalement.Traits)
        {
            Info += TEXT("\n[") +
                StaticEnum<ETMOPSignalementTraitType>()->GetNameStringByValue(
                    static_cast<int64>(Trait.TraitType)) + TEXT("]");
            if (!Trait.OriginalText.IsEmpty())
                Info += TEXT(" ") + Trait.OriginalText;
            if (!Trait.NormalizedValues.IsEmpty())
            {
                FString Values;
                for (const FName Value : Trait.NormalizedValues)
                {
                    if (!Values.IsEmpty()) Values += TEXT(", ");
                    Values += Value.ToString();
                }
                Info += TEXT("\n  Normalized: ") + Values;
            }
            for (const FName Excluded : Trait.ExplicitlyExcludedValues)
                Info += TEXT("\n  Excludes: ") + Excluded.ToString();
            if (!Trait.SourceReference.IsEmpty())
                Info += TEXT("\n  Trait source: ") + Trait.SourceReference;
        }
        if (!Signalement.SourceReference.IsEmpty())
            Info += TEXT("\nSignalement source: ") +
                Signalement.SourceReference;
    }
    ObservationInfoText->SetText(FText::FromString(Info));
}

void STMOPObservationEditor::RefreshValidationReport()
{
    if (!ValidationText) return;
    TArray<FString> MissingAnchors;
    TArray<FString> MissingSubjects;
    TArray<FString> MissingObservers;
    TArray<FString> IdentityProblems;
    TArray<FString> SignalementReview;
    TArray<FString> OutsidePlayTime;
    TArray<FString> LinkProblems;
    TArray<FString> MovementProblems;

    const bool bCanValidateLevelAnchors = !AnchorPositions.IsEmpty();
    for (const TPair<FName, FTMOPObservationDefinition>& Pair :
        ObservationsById)
    {
        const FTMOPObservationDefinition& Observation = Pair.Value;
        if (bCanValidateLevelAnchors &&
            !AnchorPositions.Contains(Observation.ObservationAnchorId))
            MissingAnchors.Add(Pair.Key.ToString() + TEXT(" → ") +
                Observation.ObservationAnchorId.ToString());
        if (Observation.ObservedEntityId.IsNone())
            MissingSubjects.Add(Pair.Key.ToString());
        if (Observation.ObserverEntityIds.IsEmpty() &&
            !Observation.bAllowUnattributedObservation)
            MissingObservers.Add(Pair.Key.ToString());
        if (Observation.ObservedEntityType ==
                ETMOPObservedEntityType::Person &&
            Observation.ObservedPersonIdentityStatus ==
                ETMOPObservedPersonIdentityStatus::Unclassified)
            IdentityProblems.Add(Pair.Key.ToString() +
                TEXT(" is not classified as known/unknown"));
        if (Observation.ObservedPersonIdentityStatus ==
                ETMOPObservedPersonIdentityStatus::KnownPerson &&
            !KnownPersonIds.Contains(Observation.ObservedEntityId))
            IdentityProblems.Add(Pair.Key.ToString() + TEXT(" → missing person ") +
                Observation.ObservedEntityId.ToString());
        if (Observation.ObservedEntityType ==
                ETMOPObservedEntityType::Person &&
            !UTMOPObservationSignalementLibrary::HasUsableSignalement(
                Observation) &&
            !Observation.bNoFurtherSignalementInSource)
            SignalementReview.Add(Pair.Key.ToString());
        const int32 Second = ResolveObservationSecond(Pair.Key);
        if (Second < 22 * 3600 + 55 * 60 ||
            Second > 23 * 3600 + 55 * 60)
            OutsidePlayTime.Add(Pair.Key.ToString() + TEXT(" @ ") +
                FormatObservationTime(Observation));
    }

    if (UDataTable* Table = LinkTable.Get())
    {
        for (const FName RowName : Table->GetRowNames())
        {
            const FTMOPObservationLinkDefinition* Link =
                Table->FindRow<FTMOPObservationLinkDefinition>(RowName,
                    TEXT("ObservationEditorValidateAll"), false);
            if (Link == nullptr) continue;
            TArray<FName> Members = Link->ObservationIds;
            if (Members.IsEmpty())
            {
                if (!Link->FromObservationId.IsNone())
                    Members.Add(Link->FromObservationId);
                if (!Link->ToObservationId.IsNone())
                    Members.AddUnique(Link->ToObservationId);
            }
            if (Members.Num() < 2)
            {
                LinkProblems.Add(RowName.ToString() +
                    TEXT(" has fewer than two observations"));
                continue;
            }
            for (const FName MemberId : Members)
            {
                const FTMOPObservationDefinition* Observation =
                    ObservationsById.Find(MemberId);
                if (Observation == nullptr)
                    LinkProblems.Add(RowName.ToString() + TEXT(" → missing ") +
                        MemberId.ToString());
                else if (Observation->ObservedPersonIdentityStatus ==
                    ETMOPObservedPersonIdentityStatus::KnownPerson)
                    LinkProblems.Add(RowName.ToString() + TEXT(" contains known person ") +
                        MemberId.ToString());
            }
            for (int32 Index = 0; Index + 1 < Members.Num(); ++Index)
            {
                const FTMOPObservationDefinition* From =
                    ObservationsById.Find(Members[Index]);
                const FTMOPObservationDefinition* To =
                    ObservationsById.Find(Members[Index + 1]);
                if (From == nullptr || To == nullptr) continue;
                const int32 FromSecond = ResolveObservationSecond(Members[Index]);
                int32 ToSecond = ResolveObservationSecond(Members[Index + 1]);
                if (ToSecond < FromSecond) ToSecond += 24 * 3600;
                const int32 AvailableSeconds = ToSecond -
                    (FromSecond + FMath::Max(1,
                        From->ObservationDurationSeconds));
                const FVector2D* FromPosition =
                    AnchorPositions.Find(From->ObservationAnchorId);
                const FVector2D* ToPosition =
                    AnchorPositions.Find(To->ObservationAnchorId);
                if (AvailableSeconds < 0 &&
                    From->ObservationAnchorId != To->ObservationAnchorId)
                    MovementProblems.Add(RowName.ToString() + TEXT(": ") +
                        Members[Index].ToString() + TEXT(" overlaps ") +
                        Members[Index + 1].ToString());
                else if (AvailableSeconds > 0 && FromPosition && ToPosition)
                {
                    const float RequiredSpeed = FVector2D::Distance(
                        *FromPosition, *ToPosition) / AvailableSeconds;
                    const bool bVehicle = Link->LinkedEntityType ==
                        ETMOPObservedEntityType::Vehicle;
                    const float PlausibleMaximum = bVehicle ? 7000.0f : 600.0f;
                    if (RequiredSpeed > PlausibleMaximum)
                        MovementProblems.Add(FString::Printf(
                            TEXT("%s: %s → %s requires %.1f km/h"),
                            *RowName.ToString(), *Members[Index].ToString(),
                            *Members[Index + 1].ToString(),
                            RequiredSpeed * 0.036f));
                }
                if (Link->bRequireSignalementCompatibility)
                {
                    const FTMOPSignalementComparison Comparison =
                        UTMOPObservationSignalementLibrary::CompareSignalements(
                            *From, *To);
                    if (!Comparison.bHasComparableEvidence ||
                        Comparison.CompatibilityScore <
                            Link->MinimumSignalementCompatibility)
                        LinkProblems.Add(RowName.ToString() +
                            TEXT(" has insufficient signalement match: ") +
                            Members[Index].ToString() + TEXT(" → ") +
                            Members[Index + 1].ToString());
                }
            }
        }
    }

    FString Report = FString::Printf(
        TEXT("%d observations · %d links\n"), ObservationsById.Num(),
        LinkTable.IsValid() ? LinkTable->GetRowNames().Num() : 0);
    if (!bCanValidateLevelAnchors)
        Report += TEXT("\n▲ Open the gameplay level to validate anchor positions.");
    auto AppendSection = [&Report](const TCHAR* Label,
        const TArray<FString>& Values)
    {
        Report += FString::Printf(TEXT("\n\n%s: %d"), Label, Values.Num());
        const int32 ExampleCount = FMath::Min(Values.Num(), 12);
        for (int32 Index = 0; Index < ExampleCount; ++Index)
            Report += TEXT("\n  • ") + Values[Index];
        if (Values.Num() > ExampleCount)
            Report += FString::Printf(TEXT("\n  …and %d more"),
                Values.Num() - ExampleCount);
    };
    AppendSection(TEXT("ERROR missing anchors"), MissingAnchors);
    AppendSection(TEXT("ERROR missing observed entities"), MissingSubjects);
    AppendSection(TEXT("ERROR missing observers"), MissingObservers);
    AppendSection(TEXT("ERROR link definitions"), LinkProblems);
    AppendSection(TEXT("ERROR movement feasibility"), MovementProblems);
    AppendSection(TEXT("REVIEW identity"), IdentityProblems);
    AppendSection(TEXT("REVIEW signalement"), SignalementReview);
    AppendSection(TEXT("INFO outside 22:55–23:55"), OutsidePlayTime);
    ValidationText->SetText(FText::FromString(Report));
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
    if (!Map) return;
    TArray<STMOPObservationMap::FPoint> Points;
    const bool bShowSelectedLinkOnly = !WorkingLink.ObservationIds.IsEmpty();
    TSet<FName> FilteredObservationIds;
    if (!bShowSelectedLinkOnly)
        for (const FObservationItem& Item : ObservationItems)
            if (Item) FilteredObservationIds.Add(*Item);
    for (const TPair<FName, FTMOPObservationDefinition>& Pair : ObservationsById)
    {
        if (bShowSelectedLinkOnly
                ? !WorkingLink.ObservationIds.Contains(Pair.Key)
                : !FilteredObservationIds.Contains(Pair.Key))
            continue;
        if (const FVector2D* Position =
            AnchorPositions.Find(Pair.Value.ObservationAnchorId))
        {
            STMOPObservationMap::FPoint Point;
            Point.ObservationId = Pair.Key;
            Point.Position = *Position;
            Point.bLinked = IsObservationLinked(Pair.Key);
            Point.Order = WorkingLink.ObservationIds.IndexOfByKey(Pair.Key);
            Point.bSelected = Point.Order != INDEX_NONE ||
                Pair.Key == SelectedObservationId;
            Point.EntityType = Pair.Value.ObservedEntityType;
            Points.Add(Point);
        }
    }
    FVector2D PreviewPosition;
    FString PreviewDescription;
    const TOptional<FVector2D> OptionalPreview =
        CalculatePreviewPosition(PreviewPosition, PreviewDescription)
        ? TOptional<FVector2D>(PreviewPosition) : TOptional<FVector2D>();
    Map->SetData(MoveTemp(Points), CachedLanePolylines,
        OptionalPreview, PreviewDescription);
}

TSharedRef<ITableRow> STMOPObservationEditor::GenerateObservationRow(
    FObservationItem Item, const TSharedRef<STableViewBase>& Owner)
{
    const FTMOPObservationDefinition* Observation = Item ? ObservationsById.Find(*Item) : nullptr;
    FString SignalementStatus;
    FString IdentityStatus;
    bool bHasError = Observation == nullptr;
    bool bNeedsReview = false;
    bool bDisabled = false;
    if (Observation && Observation->ObservedEntityType ==
        ETMOPObservedEntityType::Person)
    {
        switch (Observation->ObservedPersonIdentityStatus)
        {
        case ETMOPObservedPersonIdentityStatus::KnownPerson:
            IdentityStatus = TEXT("  ·  KNOWN PERSON");
            break;
        case ETMOPObservedPersonIdentityStatus::UnknownPerson:
            IdentityStatus = TEXT("  ·  UNKNOWN PERSON");
            break;
        default:
            IdentityStatus = TEXT("  ·  ID UNCLASSIFIED");
            break;
        }
        if (UTMOPObservationSignalementLibrary::HasUsableSignalement(*Observation))
            SignalementStatus = FString::Printf(TEXT("  ·  SIG %d witness record(s)"),
                Observation->WitnessSignalements.Num());
        else if (Observation->bNoFurtherSignalementInSource)
            SignalementStatus = TEXT("  ·  SIG reviewed: none in source");
        else
            SignalementStatus = Observation->bSignalementSourceReviewed
                ? TEXT("  ·  SIG reviewed: incomplete")
                : TEXT("  ·  SIG NEEDS REVIEW");
        bNeedsReview = Observation->ObservedPersonIdentityStatus ==
                ETMOPObservedPersonIdentityStatus::Unclassified ||
            (!UTMOPObservationSignalementLibrary::HasUsableSignalement(
                *Observation) && !Observation->bNoFurtherSignalementInSource);
    }
    if (Observation)
    {
        bDisabled = !Observation->bEnabled;
        bHasError = Observation->ObservedEntityId.IsNone() ||
            (Observation->ObserverEntityIds.IsEmpty() &&
                !Observation->bAllowUnattributedObservation) ||
            (!AnchorPositions.IsEmpty() &&
                !AnchorPositions.Contains(Observation->ObservationAnchorId));
        const int32 Second = ResolveObservationSecond(Observation->ObservationId);
        bNeedsReview = bNeedsReview || Second < 22 * 3600 + 55 * 60 ||
            Second > 23 * 3600 + 55 * 60 ||
            Observation->ObservedEntityType == ETMOPObservedEntityType::Unknown;
    }
    const FString StatusPrefix = bDisabled ? TEXT("○ DISABLED")
        : bHasError ? TEXT("● ERROR")
        : bNeedsReview ? TEXT("▲ REVIEW") : TEXT("✓ READY");
    const FString Label = Observation
        ? FString::Printf(TEXT("%s  %s  %s\n%s → %s%s%s"),
            *StatusPrefix,
            *FormatObservationTime(*Observation), *Observation->DisplayName.ToString(),
            *Observation->ObservedEntityId.ToString(), *Observation->ObservationAnchorId.ToString(),
            *IdentityStatus, *SignalementStatus)
        : TEXT("Missing observation");
    const bool bLinked = IsObservationLinked(Item ? *Item : NAME_None);
    const bool bKnown = Observation &&
        Observation->ObservedPersonIdentityStatus ==
            ETMOPObservedPersonIdentityStatus::KnownPerson;
    const FLinearColor RowColor = bDisabled
        ? FLinearColor(0.45f, 0.45f, 0.45f)
        : bHasError ? FLinearColor(1.0f, 0.25f, 0.2f)
        : bNeedsReview ? FLinearColor(1.0f, 0.72f, 0.15f)
        : bLinked ? FLinearColor(0.35f, 0.9f, 0.45f)
        : bKnown ? FLinearColor(0.35f, 0.78f, 1.0f)
        : FLinearColor::White;
    return SNew(STableRow<FObservationItem>, Owner)
        .OnDragDetected(this, &STMOPObservationEditor::HandleObservationDragDetected, Item)
        [ SNew(STextBlock).Text(FText::FromString(Label))
            .ColorAndOpacity(RowColor) ];
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

TSharedRef<SWidget> STMOPObservationEditor::GenerateKnownPersonOption(
    FPersonItem Item) const
{
    if (!Item) return SNew(STextBlock).Text(LOCTEXT("NoKnownPerson", "None"));
    const FText* DisplayName = PersonDisplayNames.Find(*Item);
    return SNew(STextBlock).Text(FText::Format(
        LOCTEXT("KnownPersonOption", "{0}  ·  {1}"),
        DisplayName != nullptr ? *DisplayName : FText::FromName(*Item),
        FText::FromName(*Item)));
}

TSharedRef<SWidget> STMOPObservationEditor::GenerateGeoFilterOption(
    FGeoFilterItem Item) const
{
    return SNew(STextBlock).Text(Item
        ? GeoFilterDisplayName(*Item)
        : GeoFilterDisplayName(TEXT("AllPlayArea")));
}

FText STMOPObservationEditor::GetSelectedKnownPersonText() const
{
    if (SelectedKnownPersonId.IsNone())
        return LOCTEXT("SelectKnownPerson", "Select known person / witness…");
    const FText* DisplayName = PersonDisplayNames.Find(SelectedKnownPersonId);
    return FText::Format(LOCTEXT("SelectedKnownPerson", "{0}  ·  {1}"),
        DisplayName != nullptr ? *DisplayName :
            FText::FromName(SelectedKnownPersonId),
        FText::FromName(SelectedKnownPersonId));
}

FText STMOPObservationEditor::GetSelectedGeoFilterText() const
{
    return GeoFilterDisplayName(SelectedGeoFilter);
}

float STMOPObservationEditor::GetGeoFilterRadiusMeters() const
{
    const float* RadiusCm = GeoFilterRadiusCm.Find(SelectedGeoFilter);
    return RadiusCm != nullptr ? *RadiusCm / 100.0f : 0.0f;
}

float STMOPObservationEditor::GetPreviewSliderValue() const
{
    const int32 StartSecond = bAllTimes
        ? 22 * 3600 + 55 * 60 : TimeFilterStartSecond;
    int32 EndSecond = bAllTimes
        ? 23 * 3600 + 55 * 60 : TimeFilterEndSecond;
    int32 EffectivePreview = PreviewSecond;
    if (EndSecond < StartSecond) EndSecond += 24 * 3600;
    if (EffectivePreview < StartSecond) EffectivePreview += 24 * 3600;
    return FMath::Clamp(static_cast<float>(EffectivePreview - StartSecond) /
        FMath::Max(1.0f, static_cast<float>(EndSecond - StartSecond)),
        0.0f, 1.0f);
}

FText STMOPObservationEditor::GetPreviewTimeText() const
{
    const int32 Normalized = ((PreviewSecond % (24 * 3600)) +
        24 * 3600) % (24 * 3600);
    FVector2D Position;
    FString Description;
    CalculatePreviewPosition(Position, Description);
    return FText::FromString(FString::Printf(TEXT("%02d:%02d:%02d  %s"),
        Normalized / 3600, (Normalized / 60) % 60, Normalized % 60,
        Description.IsEmpty() ? TEXT("select a link") : *Description));
}

void STMOPObservationEditor::OnObservationSelected(
    FObservationItem Item, ESelectInfo::Type)
{
    SelectedObservationId = Item ? *Item : NAME_None;
    SelectedKnownPersonId = NAME_None;
    const FTMOPObservationDefinition* Observation =
        ObservationsById.Find(SelectedObservationId);
    if (Observation != nullptr &&
        Observation->ObservedPersonIdentityStatus ==
            ETMOPObservedPersonIdentityStatus::KnownPerson &&
        KnownPersonIds.Contains(Observation->ObservedEntityId))
    {
        SelectedKnownPersonId = Observation->ObservedEntityId;
        for (const FPersonItem& Option : KnownPersonItems)
            if (Option && *Option == SelectedKnownPersonId)
            {
                if (KnownPersonCombo) KnownPersonCombo->SetSelectedItem(Option);
                break;
            }
    }
    RefreshObservationInfo();
    RebuildMap();
}

void STMOPObservationEditor::OnLinkSelected(FLinkItem Item, ESelectInfo::Type)
{
    if (Item) SelectLink(*Item);
}

void STMOPObservationEditor::OnKnownPersonSelected(
    FPersonItem Item, ESelectInfo::Type)
{
    SelectedKnownPersonId = Item ? *Item : NAME_None;
}

void STMOPObservationEditor::OnGeoFilterSelected(
    FGeoFilterItem Item, ESelectInfo::Type)
{
    SelectedGeoFilter = Item ? *Item : FName(TEXT("AllPlayArea"));
    RefreshObservations();
}

void STMOPObservationEditor::OnGeoFilterRadiusCommitted(
    const float Value, ETextCommit::Type)
{
    if (SelectedGeoFilter == TEXT("AllPlayArea")) return;
    GeoFilterRadiusCm.FindOrAdd(SelectedGeoFilter) =
        FMath::Clamp(Value, 20.0f, 1000.0f) * 100.0f;
    RefreshObservations();
}

void STMOPObservationEditor::OnPreviewSliderChanged(const float Value)
{
    CommitLinkDetails();
    const int32 StartSecond = bAllTimes
        ? 22 * 3600 + 55 * 60 : TimeFilterStartSecond;
    int32 EndSecond = bAllTimes
        ? 23 * 3600 + 55 * 60 : TimeFilterEndSecond;
    if (EndSecond < StartSecond) EndSecond += 24 * 3600;
    PreviewSecond = (StartSecond + FMath::RoundToInt(
        FMath::Clamp(Value, 0.0f, 1.0f) *
        static_cast<float>(EndSecond - StartSecond))) % (24 * 3600);
    RebuildMap();
}

void STMOPObservationEditor::OnMapObservationSelected(
    const FName ObservationId)
{
    SelectedObservationId = ObservationId;
    if (ObservationList)
    {
        for (const FObservationItem& Item : ObservationItems)
        {
            if (!Item || *Item != ObservationId) continue;
            ObservationList->SetSelection(Item);
            ObservationList->RequestScrollIntoView(Item);
            return;
        }
    }
    RefreshObservationInfo();
    RebuildMap();
}

void STMOPObservationEditor::OnAllTimesChanged(
    const ECheckBoxState NewState)
{
    bAllTimes = NewState == ECheckBoxState::Checked;
    RefreshObservations();
}

void STMOPObservationEditor::OnTimeStartCommitted(
    const FText& Text, ETextCommit::Type)
{
    int32 ParsedSecond = 0;
    if (!ParseEditorClockText(Text.ToString(), ParsedSecond))
    {
        SetStatus(LOCTEXT("InvalidTimeFilterStart",
            "Invalid start time. Use HH:MM or HH:MM:SS."),
            FLinearColor::Red);
        return;
    }
    TimeFilterStartSecond = ParsedSecond;
    bAllTimes = false;
    RefreshObservations();
}

void STMOPObservationEditor::OnTimeEndCommitted(
    const FText& Text, ETextCommit::Type)
{
    int32 ParsedSecond = 0;
    if (!ParseEditorClockText(Text.ToString(), ParsedSecond))
    {
        SetStatus(LOCTEXT("InvalidTimeFilterEnd",
            "Invalid end time. Use HH:MM or HH:MM:SS."),
            FLinearColor::Red);
        return;
    }
    TimeFilterEndSecond = ParsedSecond;
    bAllTimes = false;
    RefreshObservations();
}

void STMOPObservationEditor::OnKnownPersonSearchChanged(const FText& Text)
{
    KnownPersonSearch = Text.ToString();
    KnownPersonItems.Reset();
    for (const FName PersonId : KnownPersonIds)
    {
        const FText* DisplayName = PersonDisplayNames.Find(PersonId);
        const FString Haystack = PersonId.ToString() + TEXT(" ") +
            (DisplayName != nullptr ? DisplayName->ToString() : FString());
        if (KnownPersonSearch.IsEmpty() ||
            Haystack.Contains(KnownPersonSearch, ESearchCase::IgnoreCase))
            KnownPersonItems.Add(MakeShared<FName>(PersonId));
    }
    KnownPersonItems.Sort([this](const FPersonItem& First,
        const FPersonItem& Second)
    {
        const FText* FirstName = First ? PersonDisplayNames.Find(*First) : nullptr;
        const FText* SecondName = Second ? PersonDisplayNames.Find(*Second) : nullptr;
        const FString FirstText = FirstName != nullptr
            ? FirstName->ToString() : FString();
        const FString SecondText = SecondName != nullptr
            ? SecondName->ToString() : FString();
        return FirstText == SecondText
            ? First->LexicalLess(*Second) : FirstText < SecondText;
    });
    if (KnownPersonCombo) KnownPersonCombo->RefreshOptions();
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
    const FTMOPObservationDefinition* Observation =
        ObservationsById.Find(ObservationId);
    if (Observation == nullptr) return;
    if (Observation->ObservedPersonIdentityStatus ==
        ETMOPObservedPersonIdentityStatus::KnownPerson)
    {
        SetStatus(LOCTEXT("KnownPersonCannotBeLinked",
            "Known-person observations point directly to the People table and cannot be added to an ObservationLink."),
            FLinearColor::Red);
        return;
    }
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
    {
        const FTMOPObservationDefinition* Observation =
            ObservationsById.Find(Id);
        if (Observation == nullptr)
        {
            SetStatus(FText::Format(LOCTEXT("MissingMember", "Not saved: observation {0} does not exist."), FText::FromName(Id)), FLinearColor::Red);
            return FReply::Handled();
        }
        if (Observation->ObservedPersonIdentityStatus ==
            ETMOPObservedPersonIdentityStatus::KnownPerson)
        {
            SetStatus(FText::Format(LOCTEXT("KnownMember",
                "Not saved: {0} is marked as a known person and must point directly to the People table, not an ObservationLink."),
                FText::FromName(Id)), FLinearColor::Red);
            return FReply::Handled();
        }
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

FReply STMOPObservationEditor::ValidateAll()
{
    RefreshLevelGeometry();
    RefreshObservations();
    RefreshValidationReport();
    SetStatus(LOCTEXT("ValidationComplete",
        "Validation complete. See the grouped report on the right."),
        FLinearColor(0.35f, 0.85f, 1.0f));
    return FReply::Handled();
}

FReply STMOPObservationEditor::AddSelectedObservation()
{
    if (ObservationList && !ObservationList->GetSelectedItems().IsEmpty())
        AddObservation(*ObservationList->GetSelectedItems()[0]);
    return FReply::Handled();
}

FReply STMOPObservationEditor::MarkSelectedUnknownPerson()
{
    if (SelectedObservationId.IsNone())
    {
        SetStatus(LOCTEXT("SelectObservationForUnknown",
            "Select an observation first."), FLinearColor::Red);
        return FReply::Handled();
    }
    SaveObservationIdentity(
        ETMOPObservedPersonIdentityStatus::UnknownPerson, NAME_None);
    return FReply::Handled();
}

FReply STMOPObservationEditor::MarkSelectedKnownPerson()
{
    if (SelectedObservationId.IsNone() || SelectedKnownPersonId.IsNone())
    {
        SetStatus(LOCTEXT("SelectObservationAndKnownPerson",
            "Select both an observation and a known person / witness."),
            FLinearColor::Red);
        return FReply::Handled();
    }
    SaveObservationIdentity(
        ETMOPObservedPersonIdentityStatus::KnownPerson,
        SelectedKnownPersonId);
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

bool STMOPObservationEditor::IsKnownPersonEntity(const FName EntityId) const
{
    return KnownPersonIds.Contains(EntityId);
}

int32 STMOPObservationEditor::RemoveObservationFromLinks(
    const FName ObservationId)
{
    UDataTable* Table = LinkTable.Get();
    if (Table == nullptr) return 0;

    struct FLinkUpdate
    {
        FName RowName = NAME_None;
        FTMOPObservationLinkDefinition Definition;
        bool bDelete = false;
    };
    TArray<FLinkUpdate> Updates;
    for (const FName RowName : Table->GetRowNames())
    {
        const FTMOPObservationLinkDefinition* Existing =
            Table->FindRow<FTMOPObservationLinkDefinition>(RowName,
                TEXT("RemoveKnownObservationFromLinks"), false);
        if (Existing == nullptr) continue;
        FLinkUpdate Update;
        Update.RowName = RowName;
        Update.Definition = *Existing;
        if (Update.Definition.ObservationIds.IsEmpty())
        {
            if (!Update.Definition.FromObservationId.IsNone())
                Update.Definition.ObservationIds.Add(
                    Update.Definition.FromObservationId);
            if (!Update.Definition.ToObservationId.IsNone())
                Update.Definition.ObservationIds.AddUnique(
                    Update.Definition.ToObservationId);
        }
        if (Update.Definition.ObservationIds.Remove(ObservationId) == 0)
            continue;
        Update.Definition.FromObservationId = NAME_None;
        Update.Definition.ToObservationId = NAME_None;
        Update.Definition.TrackSegments.RemoveAll(
            [ObservationId](const FTMOPObservationTrackSegment& Segment)
            {
                return Segment.FromObservationId == ObservationId ||
                    Segment.ToObservationId == ObservationId;
            });
        Update.bDelete = Update.Definition.ObservationIds.Num() < 2;
        Updates.Add(MoveTemp(Update));
    }

    if (Updates.IsEmpty()) return 0;
    Table->Modify();
    for (const FLinkUpdate& Update : Updates)
    {
        if (Update.bDelete)
            Table->RemoveRow(Update.RowName);
        else
            Table->AddRow(Update.RowName, Update.Definition);
    }
    Table->MarkPackageDirty();
    Table->PostEditChange();
    return Updates.Num();
}

bool STMOPObservationEditor::SaveObservationIdentity(
    const ETMOPObservedPersonIdentityStatus NewStatus,
    const FName PersonEntityId)
{
    UDataTable* Table = ObservationTable.Get();
    const FName* RowName =
        ObservationRowNamesById.Find(SelectedObservationId);
    const FTMOPObservationDefinition* Existing =
        ObservationsById.Find(SelectedObservationId);
    if (Table == nullptr || RowName == nullptr || Existing == nullptr)
    {
        SetStatus(LOCTEXT("CannotSaveObservationIdentity",
            "The selected observation row could not be found."),
            FLinearColor::Red);
        return false;
    }
    if (NewStatus == ETMOPObservedPersonIdentityStatus::KnownPerson &&
        !IsKnownPersonEntity(PersonEntityId))
    {
        SetStatus(LOCTEXT("InvalidKnownPerson",
            "The selected person is not a known People-table row."),
            FLinearColor::Red);
        return false;
    }

    const bool bMustUnlink =
        NewStatus == ETMOPObservedPersonIdentityStatus::KnownPerson &&
        IsObservationLinked(SelectedObservationId);
    if (bMustUnlink)
    {
        const EAppReturnType::Type Answer = FMessageDialog::Open(
            EAppMsgType::YesNo,
            LOCTEXT("ConfirmKnownPersonUnlink",
                "This observation currently belongs to one or more ObservationLinks. A known person must point directly to the People table. Remove it from those links? Links left with fewer than two observations will be deleted."));
        if (Answer != EAppReturnType::Yes)
            return false;
    }

    const FScopedTransaction Transaction(LOCTEXT("SaveObservationIdentityTransaction",
        "Classify observed person identity"));
    const FName ObservationId = SelectedObservationId;
    const int32 ChangedLinkCount = bMustUnlink
        ? RemoveObservationFromLinks(ObservationId) : 0;
    FTMOPObservationDefinition Updated = *Existing;
    Updated.ObservedEntityType = ETMOPObservedEntityType::Person;
    Updated.ObservedPersonIdentityStatus = NewStatus;
    if (NewStatus == ETMOPObservedPersonIdentityStatus::KnownPerson)
        Updated.ObservedEntityId = PersonEntityId;

    Table->Modify();
    Table->AddRow(*RowName, Updated);
    Table->MarkPackageDirty();
    Table->PostEditChange();

    const FName LinkToReselect = SelectedLinkRow;
    RefreshAll();
    SelectedObservationId = ObservationId;
    if (!LinkToReselect.IsNone() && LinkTable.IsValid() &&
        LinkTable->GetRowMap().Contains(LinkToReselect))
        SelectLink(LinkToReselect);
    else if (!LinkToReselect.IsNone())
    {
        SelectedLinkRow = NAME_None;
        WorkingLink = FTMOPObservationLinkDefinition();
        RefreshLinkDetails();
        RefreshMembers();
        RebuildMap();
    }
    if (ObservationList)
        for (const FObservationItem& Item : ObservationItems)
            if (Item && *Item == ObservationId)
            {
                ObservationList->SetSelection(Item);
                break;
            }
    SetStatus(NewStatus == ETMOPObservedPersonIdentityStatus::KnownPerson
        ? FText::Format(LOCTEXT("KnownPersonSaved",
            "Observation now points directly to known person {0}. Removed from {1} link(s). Save the project."),
            FText::FromName(PersonEntityId), FText::AsNumber(ChangedLinkCount))
        : LOCTEXT("UnknownPersonSaved",
            "Observation marked as an unknown person and may be used in an ObservationLink. Save the project."),
        FLinearColor(0.35f, 1.0f, 0.4f));
    return true;
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
