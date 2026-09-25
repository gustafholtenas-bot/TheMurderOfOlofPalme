#include "UI/STMOPNotebookPanel.h"
#include "Localization/TMOPLocalization.h"
#include "Observations/TMOPNotebookTypes.h"
#include "Observations/TMOPNotebookPresentation.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "EngineUtils.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "UObject/GCObject.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScaleBox.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/STableRow.h"

namespace
{
    const FLinearColor ColumnGray(0.36f, 0.36f, 0.36f, 1);
    const FLinearColor CardGray(0.20f, 0.20f, 0.20f, 1);
    TSharedRef<SWidget> Frame(TSharedRef<SWidget> Child, FLinearColor Fill, float Padding = 8)
    {
        return SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
            .BorderBackgroundColor(FLinearColor::Black).Padding(2)
            [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
                .BorderBackgroundColor(Fill).Padding(Padding)[Child]];
    }
    TSharedRef<STextBlock> Text(const FText& Value, int32 Size = 12, bool bBold = false,
        FLinearColor Color = FLinearColor::White)
    {
        return SNew(STextBlock).Text(FTMOPLocalization::Text(Value)).AutoWrapText(true).ColorAndOpacity(Color)
            .Font(FCoreStyle::GetDefaultFontStyle(bBold ? "Bold" : "Regular", Size));
    }

    class SNotebookCard : public SCompoundWidget, public FGCObject
    {
    public:
        SLATE_BEGIN_ARGS(SNotebookCard) {}
            SLATE_ARGUMENT(FTMOPNotebookObservation, Entry)
            SLATE_EVENT(FOnClicked, OnInspect)
        SLATE_END_ARGS()
        void Construct(const FArguments& Args)
        {
            Entry = Args._Entry;
            if (!Entry.ModelPreviewPng.IsEmpty())
                ModelTexture = FImageUtils::ImportBufferAsTexture2D(Entry.ModelPreviewPng);
            SetBrush(ModelBrush, ModelTexture);
            ShowEvidence(0);
            TSharedRef<SVerticalBox> Body = SNew(SVerticalBox);
            Body->AddSlot().AutoHeight()[SNew(SButton).OnClicked(Args._OnInspect)
                .IsEnabled(Entry.Kind == ETMOPNotebookEntityKind::Person)
                [Text(Entry.DisplayName, 13, true)]];
            TArray<FString> Places;
            for (const auto& Point : Entry.Locations)
                Places.AddUnique(FTMOPTime::FromSecondsFromMidnight(Point.Second).ToDisplayString() + TEXT(" — ") + Point.Address.ToString() + (Point.bPlayerObservation ? NSLOCTEXT("TMOP", "STMOPNotebookPanel.b2aa15e0ed32de6d", " (egen observation)").ToString() : TEXT("")));
            Body->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Text(FText::FromString(Places.IsEmpty() ? NSLOCTEXT("TMOP", "STMOPNotebookPanel.a31bbcb8a7b0dc04", "Observerad vid: plats ej fastställd").ToString() : NSLOCTEXT("TMOP", "STMOPNotebookPanel.75d746ff9d70c08c", "Observerad vid: ").ToString() + FString::Join(Places, TEXT("; "))))];
            Body->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Text(Entry.Summary.IsEmpty()
                ? NSLOCTEXT("TMOP", "NotebookNoEventSummary", "Ingen händelsebeskrivning registrerad.") : Entry.Summary)];
            Body->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Text(FTMOPLocalization::Format(
                NSLOCTEXT("TMOP", "NotebookSignalement", "Signalement: {0}"), Entry.Signalement.IsEmpty()
                    ? NSLOCTEXT("TMOP", "NotebookUnspecified", "ej angivet") : Entry.Signalement))];
            TArray<FString> Names;
            for (const auto& Name : Entry.ObserverNames) Names.Add(Name.ToString());
            Body->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Text(FTMOPLocalization::Format(
                NSLOCTEXT("TMOP", "NotebookSeenBy", "Sedd av: {0}"), Names.IsEmpty()
                    ? NSLOCTEXT("TMOP", "NotebookNoWitnesses", "inget namngivet vittne registrerat")
                    : FText::FromString(FString::Join(Names, TEXT(", ")))))];
            Body->AddSlot().AutoHeight().Padding(0, 6, 0, 0)[Text(FTMOPLocalization::Format(
                NSLOCTEXT("TMOP", "NotebookCollectedTime", "Insamlad {0}"),
                FText::FromString(FTMOPTime::FromSecondsFromMidnight(Entry.DiscoveredSecond).ToDisplayString())), 10)];

            TSharedRef<SHorizontalBox> Row = SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Top)
                [SNew(SVerticalBox)
                    + SVerticalBox::Slot().AutoHeight()
                    [SNew(SButton).OnClicked(Args._OnInspect).ContentPadding(0)
                        .IsEnabled(Entry.Kind == ETMOPNotebookEntityKind::Person)
                        [Frame(SNew(SBox).WidthOverride(76).HeightOverride(104)
                        [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
                            [ModelTexture ? StaticCastSharedRef<SWidget>(SNew(SImage).Image(&ModelBrush))
                                : StaticCastSharedRef<SWidget>(Text(NSLOCTEXT("TMOP", "NotebookMissingModel", "Modellbild\nsaknas"), 10))]], ColumnGray, 2)]]
                    + SVerticalBox::Slot().AutoHeight().Padding(0, 3)
                    [Text(NSLOCTEXT("TMOP", "NotebookModelImage", "3D-modell"), 10)]]
                + SHorizontalBox::Slot().FillWidth(1).Padding(8, 0)[Body];
            if (Entry.Kind == ETMOPNotebookEntityKind::Person)
                Row->AddSlot().AutoWidth().VAlign(VAlign_Top)[SNew(SBox).WidthOverride(94)
                    [SNew(SVerticalBox)
                        + SVerticalBox::Slot().AutoHeight()
                        [Frame(SNew(SBox).WidthOverride(86).HeightOverride(104)
                            [SNew(SScaleBox).Stretch(EStretch::ScaleToFit)
                                [SNew(SImage).Image(&EvidenceBrush)
                                    .Visibility_Lambda([this]() { return EvidenceTexture ? EVisibility::Visible : EVisibility::Collapsed; })]], ColumnGray, 2)]
                        + SVerticalBox::Slot().AutoHeight().Padding(0, 3)
                        [SNew(STextBlock).Text(FTMOPLocalization::Bind([this]() { return EvidenceCaption(); })).AutoWrapText(true)
                            .Font(FCoreStyle::GetDefaultFontStyle("Regular", 10)).ColorAndOpacity(FLinearColor::White)]
                        + SVerticalBox::Slot().AutoHeight()
                        [SNew(SHorizontalBox)
                            .Visibility(Entry.EvidenceImages.Num() > 1 ? EVisibility::Visible : EVisibility::Collapsed)
                            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("‹"))))
                                .OnClicked_Lambda([this]() { ShowEvidence(EvidenceIndex - 1); return FReply::Handled(); })]
                            + SHorizontalBox::Slot().FillWidth(1).HAlign(HAlign_Center).VAlign(VAlign_Center)
                            [SNew(STextBlock).Text(FTMOPLocalization::Bind([this]() { return FText::FromString(FString::Printf(TEXT("%d/%d"),
                                EvidenceIndex + 1, Entry.EvidenceImages.Num())); })).Font(FCoreStyle::GetDefaultFontStyle("Regular", 10))]
                            + SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(FTMOPLocalization::Text(FText::FromString(TEXT("›"))))
                                .OnClicked_Lambda([this]() { ShowEvidence(EvidenceIndex + 1); return FReply::Handled(); })]]]];
            ChildSlot[Frame(Row, CardGray, 8)];
        }
        virtual void AddReferencedObjects(FReferenceCollector& Collector) override
        {
            Collector.AddReferencedObject(ModelTexture);
            Collector.AddReferencedObject(EvidenceTexture);
        }
        virtual FString GetReferencerName() const override { return TEXT("STMOPNotebookCard"); }
    private:
        FTMOPNotebookObservation Entry;
        FSlateBrush ModelBrush, EvidenceBrush;
        UTexture2D* ModelTexture = nullptr;
        UTexture2D* EvidenceTexture = nullptr;
        int32 EvidenceIndex = 0;
        static void SetBrush(FSlateBrush& Brush, UTexture2D* Texture)
        {
            Brush.SetResourceObject(Texture);
            Brush.DrawAs = Texture ? ESlateBrushDrawType::Image : ESlateBrushDrawType::NoDrawType;
            Brush.ImageSize = Texture ? FVector2D(Texture->GetSizeX(), Texture->GetSizeY()) : FVector2D(76, 104);
        }
        void ShowEvidence(int32 Index)
        {
            EvidenceTexture = nullptr;
            if (!Entry.EvidenceImages.IsEmpty())
            {
                EvidenceIndex = (Index % Entry.EvidenceImages.Num() + Entry.EvidenceImages.Num()) % Entry.EvidenceImages.Num();
                EvidenceTexture = Cast<UTexture2D>(Entry.EvidenceImages[EvidenceIndex].ImagePath.TryLoad());
            }
            SetBrush(EvidenceBrush, EvidenceTexture);
            Invalidate(EInvalidateWidgetReason::Paint);
        }
        FText EvidenceCaption() const
        {
            if (Entry.EvidenceImages.IsEmpty()) return NSLOCTEXT("TMOP", "NotebookNoSketch", "Ingen fantombild/skiss");
            if (!EvidenceTexture) return NSLOCTEXT("TMOP", "NotebookMissingSketchAsset", "Bilden kunde inte läsas");
            const auto& Image = Entry.EvidenceImages[EvidenceIndex];
            FString Caption = Image.Caption.IsEmpty() ? NSLOCTEXT("TMOP", "STMOPNotebookPanel.815c692e8001b8c9", "Fantombild/skiss").ToString() : FTMOPLocalization::String(Image.Caption);
            if (!Image.Source.IsEmpty()) Caption += TEXT("\n") + FTMOPLocalization::String(Image.Source);
            return FText::FromString(Caption);
        }
    };
}

void STMOPNotebookPanel::Construct(const FArguments& Args)
{
    Player = Args._Player;
    const FText PersonHeadings[] = {
        NSLOCTEXT("TMOP", "NotebookVisualShooter", "Gärningsmannen"),
        NSLOCTEXT("TMOP", "NotebookVisualHigh", "Mycket misstänkta"),
        NSLOCTEXT("TMOP", "NotebookVisualRadio", "Män med walkie-talkies"),
        NSLOCTEXT("TMOP", "NotebookVisualOther", "Andra misstänkta"),
        NSLOCTEXT("TMOP", "NotebookVisualLow", "Mindre misstänkta")
    };
    auto AddGroup = [this](TArray<FItemPtr>& Items, FText Heading, ETMOPNotebookEntityKind Kind, int32 Category)
    {
        auto Header = MakeShared<FItem>(); Header->Heading = Heading; Items.Add(Header);
        int32 Count = 0;
        if (auto* P = Player.Get()) for (int32 I = 0; I < P->NotebookObservations.Num(); ++I)
        {
            const auto& Entry = P->NotebookObservations[I];
            if (Entry.Kind != Kind) continue;
            const int32 Value = Kind == ETMOPNotebookEntityKind::Vehicle ? static_cast<int32>(Entry.VehicleSuspicion)
                : static_cast<int32>(Entry.Category == ETMOPNotebookCategory::Automatic ? ETMOPNotebookCategory::OtherSuspicious : Entry.Category);
            if (Value != Category) continue;
            auto Item = MakeShared<FItem>(); Item->EntryIndex = I; Items.Add(Item); ++Count;
        }
        if (!Count) { auto Empty = MakeShared<FItem>(); Empty->bEmpty = true; Items.Add(Empty); }
    };
    for (int32 I = 0; I < 5; ++I) AddGroup(PeopleItems, PersonHeadings[I], ETMOPNotebookEntityKind::Person, I + 1);
    AddGroup(VehicleItems, PersonHeadings[1], ETMOPNotebookEntityKind::Vehicle, static_cast<int32>(ETMOPNotebookVehicleSuspicion::HighlySuspicious));
    AddGroup(VehicleItems, PersonHeadings[4], ETMOPNotebookEntityKind::Vehicle, static_cast<int32>(ETMOPNotebookVehicleSuspicion::LessSuspicious));
    // Keep text/image columns legible at their design width; shrink the whole
    // page in smaller/split-screen viewports instead of crushing the text cells.
    ChildSlot[SNew(SScaleBox).Stretch(EStretch::ScaleToFit).StretchDirection(EStretchDirection::DownOnly)
        [SNew(SBox).WidthOverride(1200).HeightOverride(850)
        [SNew(SBorder).BorderImage(FCoreStyle::Get().GetBrush("WhiteBrush"))
        .BorderBackgroundColor(FLinearColor::White).Padding(10)
        [SNew(SVerticalBox)
            + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Center).Padding(0, 0, 0, 12)
            [Text(NSLOCTEXT("TMOP", "NotebookVisualTitle", "Mina observationer"), 32, false, FLinearColor::Black)]
            + SVerticalBox::Slot().FillHeight(1)
            [SNew(SHorizontalBox)
                + SHorizontalBox::Slot().FillWidth(1).Padding(0, 0, 7, 0)
                [MakeColumn(NSLOCTEXT("TMOP", "NotebookPeopleColumn", "PERSONER"), PeopleItems)]
                + SHorizontalBox::Slot().FillWidth(1).Padding(7, 0, 0, 0)
                [MakeColumn(NSLOCTEXT("TMOP", "NotebookVehiclesColumn", "FORDON"), VehicleItems)]]]]]];
}

TSharedRef<SWidget> STMOPNotebookPanel::MakeColumn(const FText& Title, TArray<FItemPtr>& Items)
{
    auto Scrollbar = SNew(SScrollBar).AlwaysShowScrollbar(true);
    return Frame(SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(8, 4, 8, 8)[Text(Title, 25, true, FLinearColor::Black)]
        + SVerticalBox::Slot().FillHeight(1)
        [SNew(SHorizontalBox)
            + SHorizontalBox::Slot().FillWidth(1)[SNew(SListView<FItemPtr>)
                .ListItemsSource(&Items).SelectionMode(ESelectionMode::None).ExternalScrollbar(Scrollbar)
                .OnGenerateRow(this, &STMOPNotebookPanel::MakeRow)]
            + SHorizontalBox::Slot().AutoWidth().Padding(3, 0, 0, 0)[Scrollbar]], ColumnGray, 8);
}

TSharedRef<ITableRow> STMOPNotebookPanel::MakeRow(FItemPtr Item, const TSharedRef<STableViewBase>& Owner)
{
    TSharedRef<SVerticalBox> Content = SNew(SVerticalBox);
    if (Item->EntryIndex != INDEX_NONE && Player.IsValid() && Player->NotebookObservations.IsValidIndex(Item->EntryIndex))
    {
        auto& Entry = Player->NotebookObservations[Item->EntryIndex];
        if ((Entry.ModelPreviewPng.IsEmpty() || Entry.ModelPreviewVersion < 2) && !PreviewAttempted.Contains(Item->EntryIndex))
        {
            PreviewAttempted.Add(Item->EntryIndex);
            AActor* Source = nullptr;
            if (Entry.Kind == ETMOPNotebookEntityKind::Person)
            {
                for (TActorIterator<ATMOPHistoricalAgent> It(Player->GetWorld()); It; ++It)
                    if (IsValid(It->EntityIdentity) && It->EntityIdentity->GetEntityId() == Entry.EntityId)
                        { Source = *It; break; }
            }
            else for (TActorIterator<ATMOPVehicleBase> It(Player->GetWorld()); It; ++It)
                if (It->VehicleId == Entry.EntityId) { Source = *It; break; }
            if (Source) FTMOPNotebookPresentation::Populate(Entry, Player->GetWorld(), Source);
        }
        FTMOPNotebookPresentation::CollectLocations(Entry, Player->GetWorld());
        const FName EntityId = Entry.EntityId;
        const TWeakObjectPtr<ATMOPPlayerCharacter> OwnerPlayer = Player;
        Content->AddSlot().AutoHeight().Padding(0, 3, 0, 9)
            [SNew(SNotebookCard).Entry(FTMOPNotebookPresentation::LocalizedView(Entry, Player->GetWorld())).OnInspect(FOnClicked::CreateLambda([OwnerPlayer, EntityId]()
            {
                if (OwnerPlayer.IsValid()) OwnerPlayer->InspectNotebookPerson(EntityId);
                return FReply::Handled();
            }))];
    }
    else if (Item->bEmpty)
        Content->AddSlot().AutoHeight().Padding(8, 4, 8, 14)
            [Text(NSLOCTEXT("TMOP", "NotebookVisualEmpty", "Inga insamlade observationer ännu."), 12, false, FLinearColor::Black)];
    else Content->AddSlot().AutoHeight().Padding(8, 5, 8, 10)[Text(Item->Heading, 22, true, FLinearColor::Black)];
    return SNew(STableRow<FItemPtr>, Owner).Padding(0)[Content];
}
