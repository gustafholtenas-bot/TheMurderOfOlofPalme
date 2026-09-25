#include "UI/STMOPTheoryBuilder.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "InputCoreTypes.h"
#include "Rendering/DrawElements.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SMultiLineEditableTextBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
    FString ShortLine(FString Text, int32 Max = 28)
    {
        Text.ReplaceInline(TEXT("\r"), TEXT(" "));
        Text.ReplaceInline(TEXT("\n"), TEXT(" "));
        return Text.Len() > Max ? Text.Left(Max - 1) + TEXT("…") : Text;
    }
}

class STMOPTheoryCanvas : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPTheoryCanvas) {}
        SLATE_ARGUMENT(TWeakPtr<STMOPTheoryBuilder>, Editor)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args)
    {
        Editor = Args._Editor;
        SetClipping(EWidgetClipping::ClipToBoundsAlways);
    }
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(640, 510); }
    FVector2D ToBoard(const FVector2D& P, const FTMOPTheoryTree& T) const { return (P - T.Pan) / T.Zoom; }
    void Fit()
    {
        auto E = Editor.Pin();
        auto* T = E ? E->Tree() : nullptr;
        if (!T || T->Nodes.IsEmpty()) return;
        FVector2D Min = T->Nodes[0].Position - FVector2D(20, 30), Max = T->Nodes[0].Position + T->Nodes[0].Size;
        for (const auto& N : T->Nodes)
        {
            Min.X = FMath::Min(Min.X, N.Position.X - 20); Min.Y = FMath::Min(Min.Y, N.Position.Y - 30);
            Max.X = FMath::Max(Max.X, N.Position.X + N.Size.X + 20); Max.Y = FMath::Max(Max.Y, N.Position.Y + N.Size.Y + 20);
        }
        const FVector2D View = GetCachedGeometry().GetLocalSize();
        if (View.X < 40 || View.Y < 40) return;
        T->Zoom = FMath::Clamp(static_cast<float>(FMath::Min((View.X - 40) / (Max.X - Min.X),
            (View.Y - 40) / (Max.Y - Min.Y))), 0.15f, 1.5f);
        T->Pan = (View - (Max - Min) * T->Zoom) * 0.5 - Min * T->Zoom;
    }

    virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& G, const FSlateRect& Cull,
        FSlateWindowElementList& Out, int32 Layer, const FWidgetStyle& Style, bool bEnabled) const override
    {
        const auto* Brush = FCoreStyle::Get().GetBrush("WhiteBrush");
        FSlateDrawElement::MakeBox(Out, Layer, G.ToPaintGeometry(), Brush, ESlateDrawEffect::None, FLinearColor::White);
        auto E = Editor.Pin();
        const auto* T = E ? E->Tree() : nullptr;
        if (!T) return Layer;
        const auto GeometryAt = [&G](FVector2D Pos, FVector2D Size)
            { return G.MakeChild(Size, FSlateLayoutTransform(Pos)).ToPaintGeometry(); };
        const auto DrawText = [&](const FString& Text, FVector2D Pos, int32 Size, int32 AtLayer, FLinearColor Color)
        {
            FSlateDrawElement::MakeText(Out, AtLayer, GeometryAt(Pos, FVector2D(240, 32)),
                Text, FCoreStyle::GetDefaultFontStyle("Regular", Size), ESlateDrawEffect::None, Color);
        };
        for (const auto& L : T->Links)
        {
            const auto* A = T->Nodes.FindByPredicate([&](const auto& N) { return N.Id == L.From; });
            const auto* B = T->Nodes.FindByPredicate([&](const auto& N) { return N.Id == L.To; });
            if (!A || !B) continue;
            FVector2D Start = (A->Position + A->Size * 0.5) * T->Zoom + T->Pan;
            FVector2D End = (B->Position + B->Size * 0.5) * T->Zoom + T->Pan;
            TArray<FVector2f> Points; Points.Add(FVector2f(Start)); Points.Add(FVector2f(End));
            const bool bSelected = L.Id == E->SelectedLink;
            FSlateDrawElement::MakeLines(Out, Layer + 1, G.ToPaintGeometry(), Points,
                ESlateDrawEffect::None, bSelected ? FLinearColor(1, 0.75, 0.2) : FLinearColor(0.55, 0.65, 0.73), true, bSelected ? 4 : 2);
            if (!L.Label.IsEmpty())
                DrawText(ShortLine(L.Label, 24), (Start + End) * 0.5 + FVector2D(4, -18), 12, Layer + 2, FLinearColor::Black);
        }
        for (const auto& N : T->Nodes)
        {
            const FVector2D P = N.Position * T->Zoom + T->Pan;
            const FVector2D Size = N.Size * T->Zoom;
            const bool bSelected = N.Id == E->SelectedNode || N.Id == E->LinkStart;
            const FLinearColor Color = bSelected ? FLinearColor(0.08, 0.35, 0.75) : FLinearColor::Black;
            if (!N.bHeading || bSelected)
            {
                FSlateDrawElement::MakeBox(Out, Layer + 3, GeometryAt(P, Size), Brush, ESlateDrawEffect::None, Color);
                const FLinearColor Fill = N.EntityId.IsNone() ? FLinearColor::White : FLinearColor(0.87, 0.95, 0.89);
                FSlateDrawElement::MakeBox(Out, Layer + 4, GeometryAt(P + FVector2D(2, 2), Size - FVector2D(4, 4)),
                    Brush, ESlateDrawEffect::None, Fill);
            }
            const int32 FontSize = FMath::Max(4, FMath::RoundToInt(13 * T->Zoom));
            DrawText(N.Role, P + FVector2D(0, -24) * T->Zoom, FontSize, Layer + 5, Color);
            // Short wrapped preview; selecting a slot exposes its complete name/notes.
            const int32 Chars = FMath::Max(6, FMath::FloorToInt((Size.X - 10) / (FontSize * 0.62f)));
            FString Remaining = N.Title;
            const int32 Lines = N.bHeading ? 2 : N.Kind == ETMOPTheoryNodeKind::Vehicle ? 2 : 4;
            for (int32 I = 0; I < Lines && !Remaining.IsEmpty(); ++I)
            {
                FString Line = Remaining.Left(Chars);
                if (Remaining.Len() > Chars && I + 1 < Lines)
                {
                    int32 Space;
                    if (Line.FindLastChar(TEXT(' '), Space) && Space > 2) Line = Line.Left(Space);
                }
                Remaining = Remaining.Mid(Line.Len()).TrimStart();
                if (I + 1 == Lines && !Remaining.IsEmpty()) Line = Line.Left(FMath::Max(1, Chars - 1)) + TEXT("…");
                DrawText(Line, P + FVector2D(5, 6 + I * (FontSize + 2)), FontSize, Layer + 5, FLinearColor::Black);
            }
            if (!N.Notes.IsEmpty() && !N.bHeading)
                DrawText(TEXT("✎"), P + FVector2D(5, Size.Y - FontSize - 5), FontSize, Layer + 5, Color);
        }
        return Layer + 5;
    }

    virtual FReply OnMouseButtonDown(const FGeometry& G, const FPointerEvent& Event) override
    {
        auto E = Editor.Pin(); auto* T = E ? E->Tree() : nullptr;
        if (!T) return FReply::Unhandled();
        LastMouse = G.AbsoluteToLocal(Event.GetScreenSpacePosition());
        if (Event.GetEffectingButton() == EKeys::MiddleMouseButton || Event.GetEffectingButton() == EKeys::RightMouseButton)
        {
            bPan = true;
            return FReply::Handled().CaptureMouse(SharedThis(this)).SetUserFocus(SharedThis(this), EFocusCause::Mouse);
        }
        if (Event.GetEffectingButton() != EKeys::LeftMouseButton) return FReply::Unhandled();
        const FVector2D Board = ToBoard(LastMouse, *T);
        for (int32 I = T->Nodes.Num() - 1; I >= 0; --I)
        {
            const auto& N = T->Nodes[I];
            if (Board.X < N.Position.X || Board.Y < N.Position.Y ||
                Board.X > N.Position.X + N.Size.X || Board.Y > N.Position.Y + N.Size.Y) continue;
            const FGuid Id = N.Id;
            E->SelectNode(Id);
            if (!E->bLinkMode) { DragNode = Id; bRememberedDrag = false; }
            return FReply::Handled().CaptureMouse(SharedThis(this)).SetUserFocus(SharedThis(this), EFocusCause::Mouse);
        }
        for (const auto& L : T->Links)
        {
            const auto* A = T->Nodes.FindByPredicate([&](const auto& N) { return N.Id == L.From; });
            const auto* B = T->Nodes.FindByPredicate([&](const auto& N) { return N.Id == L.To; });
            if (!A || !B) continue;
            const FVector2D Start = (A->Position + A->Size * 0.5) * T->Zoom + T->Pan;
            const FVector2D End = (B->Position + B->Size * 0.5) * T->Zoom + T->Pan;
            const FVector2D Delta = End - Start;
            const double Alpha = FMath::Clamp(FVector2D::DotProduct(LastMouse - Start, Delta) /
                FMath::Max(1.0, Delta.SizeSquared()), 0.0, 1.0);
            if ((LastMouse - (Start + Delta * Alpha)).SizeSquared() < 64)
            {
                E->SelectLink(L.Id);
                return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::Mouse);
            }
        }
        E->ClearSelection();
        return FReply::Handled().SetUserFocus(SharedThis(this), EFocusCause::Mouse);
    }
    virtual FReply OnMouseMove(const FGeometry& G, const FPointerEvent& Event) override
    {
        if (!HasMouseCapture()) return FReply::Unhandled();
        auto E = Editor.Pin(); auto* T = E ? E->Tree() : nullptr;
        if (!T) return FReply::Unhandled();
        const FVector2D Mouse = G.AbsoluteToLocal(Event.GetScreenSpacePosition());
        const FVector2D Delta = Mouse - LastMouse;
        LastMouse = Mouse;
        if (bPan) T->Pan += Delta;
        else if (DragNode.IsValid() && !Delta.IsNearlyZero())
        {
            if (auto* N = T->Nodes.FindByPredicate([this](const auto& Node) { return Node.Id == DragNode; }))
            {
                if (!bRememberedDrag) { E->Remember(); bRememberedDrag = true; }
                N->Position += Delta / T->Zoom;
            }
        }
        return FReply::Handled();
    }
    virtual FReply OnMouseButtonUp(const FGeometry&, const FPointerEvent& Event) override
    {
        if (!HasMouseCapture()) return FReply::Unhandled();
        bPan = false; DragNode.Invalidate();
        return FReply::Handled().ReleaseMouseCapture();
    }
    virtual void OnMouseCaptureLost(const FCaptureLostEvent& Event) override
    {
        bPan = false; DragNode.Invalidate();
        SLeafWidget::OnMouseCaptureLost(Event);
    }
    virtual FReply OnMouseWheel(const FGeometry& G, const FPointerEvent& Event) override
    {
        auto E = Editor.Pin(); auto* T = E ? E->Tree() : nullptr;
        if (!T) return FReply::Unhandled();
        const FVector2D Mouse = G.AbsoluteToLocal(Event.GetScreenSpacePosition());
        const FVector2D Board = ToBoard(Mouse, *T);
        T->Zoom = FMath::Clamp(T->Zoom * FMath::Pow(1.15f, Event.GetWheelDelta()), 0.15f, 2.0f);
        T->Pan = Mouse - Board * T->Zoom;
        return FReply::Handled();
    }
    virtual FReply OnKeyDown(const FGeometry&, const FKeyEvent& Event) override
    {
        auto E = Editor.Pin(); if (!E) return FReply::Unhandled();
        if (Event.GetKey() == EKeys::Delete) E->DeleteSelection();
        else if (Event.IsControlDown() && Event.GetKey() == EKeys::Z) E->Undo(Event.IsShiftDown());
        else if (Event.IsControlDown() && Event.GetKey() == EKeys::Y) E->Undo(true);
        else return FReply::Unhandled();
        return FReply::Handled();
    }
private:
    TWeakPtr<STMOPTheoryBuilder> Editor;
    FVector2D LastMouse = FVector2D::ZeroVector;
    FGuid DragNode;
    bool bPan = false, bRememberedDrag = false;
};

FTMOPTheoryTree* STMOPTheoryBuilder::Tree() const
{
    auto* P = Player.Get();
    return P ? P->TheoryTrees.FindByPredicate([P](const auto& T) { return T.Id == P->ActiveTheoryTreeId; }) : nullptr;
}
FTMOPTheoryNode* STMOPTheoryBuilder::Node() const
{
    auto* T = Tree(); return T ? T->Nodes.FindByPredicate([this](const auto& N) { return N.Id == SelectedNode; }) : nullptr;
}
FTMOPTheoryLink* STMOPTheoryBuilder::Link() const
{
    auto* T = Tree(); return T ? T->Links.FindByPredicate([this](const auto& L) { return L.Id == SelectedLink; }) : nullptr;
}

void STMOPTheoryBuilder::Construct(const FArguments& Args)
{
    Player = Args._Player; Save = Args._OnSave;
    if (auto* P = Player.Get())
    {
        if (P->TheoryTrees.IsEmpty())
        {
            auto Initial = TMOPTheory::CreateTemplate(0);
            P->ActiveTheoryTreeId = Initial.Id;
            P->TheoryTrees.Add(MoveTemp(Initial));
        }
        P->SynchronizeTheoryShooters();
        if (!Tree() && !P->TheoryTrees.IsEmpty()) P->ActiveTheoryTreeId = P->TheoryTrees[0].Id;
    }
    auto Button = [](const FText& Label, TFunction<FReply()> Action)
    {
        return SNew(SButton).Text(Label).OnClicked_Lambda(MoveTemp(Action));
    };
    TSharedRef<SVerticalBox> Templates = SNew(SVerticalBox);
    for (int32 I = 0; I < 4; ++I)
        Templates->AddSlot().AutoHeight().HAlign(HAlign_Left).Padding(0, 2, 8, 4)
        [SNew(SButton).Text_Lambda([this, I]() { return FText::FromString(
            (Tree() && Tree()->TemplateIndex == I ? TEXT("●  ") : TEXT("○  ")) + TMOPTheory::TemplateName(I)); })
         .OnClicked_Lambda([this, I]() { NewTree(I); return FReply::Handled(); })];
    TSharedRef<SWrapBox> Toolbar = SNew(SWrapBox).UseAllottedSize(true);
    Toolbar->AddSlot().Padding(2)[Button(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.485c1dd561e7db54", "+ Person"), [this]() { AddNode(ETMOPTheoryNodeKind::Person); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[Button(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.468f5208fa0ca42b", "+ Fordon"), [this]() { AddNode(ETMOPTheoryNodeKind::Vehicle); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[Button(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.d5ab26d9a19c8f76", "+ Anteckning"), [this]() { AddNode(ETMOPTheoryNodeKind::Note); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[SNew(SButton)
        .Text_Lambda([this]() { return FText::FromString(bLinkMode ? NSLOCTEXT("TMOP", "STMOPTheoryBuilder.60f80f97e84cb6a4", "Avsluta linjer").ToString() : NSLOCTEXT("TMOP", "STMOPTheoryBuilder.6dadf155ba79d8eb", "Dra linjer").ToString()); })
        .OnClicked_Lambda([this]() { bLinkMode = !bLinkMode; LinkStart.Invalidate(); Status = bLinkMode
            ? NSLOCTEXT("TMOP", "TheorySelectTwoNodes", "Klicka på två rutor för att koppla ihop dem.").ToString() : TEXT(""); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[Button(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.facd3929ad01c9f5", "Ta bort markerad"), [this]() { DeleteSelection(); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[SNew(SButton).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.42564357b89b190b", "Ångra"))
        .IsEnabled_Lambda([this]() { return !UndoStack.IsEmpty(); })
        .OnClicked_Lambda([this]() { Undo(false); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[SNew(SButton).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.af106b6c1ae7d020", "Gör om"))
        .IsEnabled_Lambda([this]() { return !RedoStack.IsEmpty(); })
        .OnClicked_Lambda([this]() { Undo(true); return FReply::Handled(); })];
    Toolbar->AddSlot().Padding(2)[Button(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.ce5874f50b9c7da8", "Visa hela trädet"), [this]() { if (Canvas) Canvas->Fit(); return FReply::Handled(); })];

    ChildSlot[SNew(SVerticalBox)
        + SVerticalBox::Slot().AutoHeight().Padding(0, 0, 0, 8)
        [SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.17ab7b2b7672695e", "Börja med en mall. Varje klick skapar ett nytt träd. Mallarna är arbetsstrukturer, inte slutsatser om mordet.")).AutoWrapText(true)]
        + SVerticalBox::Slot().AutoHeight()[Templates]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 6)
        [SNew(SWrapBox).UseAllottedSize(true)
            + SWrapBox::Slot().Padding(2)[SNew(SComboButton).OnGetMenuContent(this, &STMOPTheoryBuilder::TreeMenu)
                .ButtonContent()[SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(Tree() ? Tree()->Title : NSLOCTEXT("TMOP", "STMOPTheoryBuilder.3b16a68dd56a6c7d", "Välj ett träd").ToString()); })]]
            + SWrapBox::Slot().Padding(2)[Button(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.e9a91ec596c9614c", "Duplicera träd"), [this]() {
                if (auto* T = Tree()) { Remember(); auto Copy = *T; Copy.Id = FGuid::NewGuid(); Copy.Title += TEXT(" — kopia");
                    Player->TheoryTrees.Add(Copy); Player->ActiveTheoryTreeId = Copy.Id; ClearSelection(); }
                return FReply::Handled(); })]
            + SWrapBox::Slot().Padding(2)[SNew(SButton)
                .Text_Lambda([this]() { return FText::FromString(bConfirmDeleteTree ? NSLOCTEXT("TMOP", "STMOPTheoryBuilder.cc97b00077b562dc", "Bekräfta radering av träd").ToString() : NSLOCTEXT("TMOP", "STMOPTheoryBuilder.9bef578d2c8fdfed", "Radera träd").ToString()); })
                .OnClicked_Lambda([this]() {
                    if (!Tree()) return FReply::Handled();
                    if (!bConfirmDeleteTree) { bConfirmDeleteTree = true; Status = NSLOCTEXT("TMOP", "STMOPTheoryBuilder.f06a054bad79ee98", "Klicka igen för att radera det valda trädet. Ångra kan återställa det.").ToString(); return FReply::Handled(); }
                    Remember(); const FGuid Id = Player->ActiveTheoryTreeId;
                    Player->TheoryTrees.RemoveAll([Id](const auto& T) { return T.Id == Id; });
                    Player->ActiveTheoryTreeId = Player->TheoryTrees.IsEmpty() ? FGuid() : Player->TheoryTrees[0].Id;
                    ClearSelection(); return FReply::Handled(); })]
            + SWrapBox::Slot().Padding(2)[SNew(SButton).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.a9f876dcf3c8b7be", "Spara träd och spel"))
                .OnClicked_Lambda([this]() { return Save.IsBound() ? Save.Execute() : FReply::Handled(); })]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 4)
        [SNew(SEditableTextBox).HintText(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.e7234cefb5c77b43", "Trädets namn"))
            .Text_Lambda([this]() { return FText::FromString(Tree() ? Tree()->Title : TEXT("")); })
            .IsEnabled_Lambda([this]() { return Tree() != nullptr; })
            .OnTextChanged_Lambda([this](const FText& Text) { if (auto* T = Tree()) {
                const FString Value = Text.ToString().Left(120); if (T->Title == Value) return;
                RememberField(TEXT("tree_title")); T->Title = Value; } })]
        + SVerticalBox::Slot().AutoHeight()[Toolbar]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 5)
        [SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.af0b4d50d1666c5c", "Klicka på en ruta och välj en insamlad observation under trädet. Dra rutor med vänster musknapp. Panorera med höger/mittknapp. Zooma med mushjulet. Klicka på en linje för att namnge eller ta bort den.")).AutoWrapText(true)]
        + SVerticalBox::Slot().AutoHeight()
        [SNew(SBox).HeightOverride(510)[SAssignNew(Canvas, STMOPTheoryCanvas).Editor(SharedThis(this))]]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 6)
        [SNew(STextBlock).Text_Lambda([this]() { return FText::FromString(Status); }).AutoWrapText(true)
            .ColorAndOpacity(FLinearColor(1, 0.8, 0.35))]
        + SVerticalBox::Slot().AutoHeight()[SAssignNew(Details, SVerticalBox)]
        + SVerticalBox::Slot().AutoHeight().Padding(0, 8)
        [SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.ca9f986215c20593", "Träden följer med när du sparar spelet. Knappen ovan skapar en ny manuell sparning. Ångra/gör om gäller under detta besök i Mina teorier.")).AutoWrapText(true)]
    ];
    RefreshDetails();
}

void STMOPTheoryBuilder::Remember()
{
    if (!Player.IsValid()) return;
    UndoStack.Add({Player->TheoryTrees, Player->ActiveTheoryTreeId});
    if (UndoStack.Num() > 50) UndoStack.RemoveAt(0);
    RedoStack.Reset(); EditingField.Reset(); bConfirmDeleteTree = false;
}
void STMOPTheoryBuilder::RememberField(const FString& Field)
{
    if (EditingField == Field) return;
    Remember(); EditingField = Field;
}
void STMOPTheoryBuilder::Undo(bool bRedo)
{
    if (!Player.IsValid()) return;
    auto& Source = bRedo ? RedoStack : UndoStack;
    auto& Destination = bRedo ? UndoStack : RedoStack;
    if (Source.IsEmpty()) return;
    Destination.Add({Player->TheoryTrees, Player->ActiveTheoryTreeId});
    const auto State = Source.Pop();
    Player->RestoreTheories(State.Trees, State.Active);
    ClearSelection(); Status = bRedo ? NSLOCTEXT("TMOP", "STMOPTheoryBuilder.46c8f3304d2a634e", "Gjorde om ändringen.").ToString() : NSLOCTEXT("TMOP", "STMOPTheoryBuilder.01a3b811054cbc02", "Ångrade ändringen.").ToString();
}
void STMOPTheoryBuilder::NewTree(int32 TemplateIndex)
{
    if (!Player.IsValid()) return;
    Remember(); auto New = TMOPTheory::CreateTemplate(TemplateIndex);
    TMOPTheory::SynchronizeShooter(New, Player->NotebookObservations);
    Player->TheoryTrees.Add(New); Player->ActiveTheoryTreeId = New.Id;
    ClearSelection(); if (Canvas) Canvas->Fit();
    Status = NSLOCTEXT("TMOP", "STMOPTheoryBuilder.8e4335da61c9a8b2", "Nytt träd skapat. Välj en ruta och fyll den med en observation.").ToString();
}
void STMOPTheoryBuilder::AddNode(ETMOPTheoryNodeKind Kind)
{
    auto* T = Tree(); if (!T) { Status = NSLOCTEXT("TMOP", "STMOPTheoryBuilder.9555cbc42cade81f", "Välj en av de fyra mallarna först.").ToString(); return; }
    Remember();
    const FVector2D View = Canvas->GetCachedGeometry().GetLocalSize();
    const FVector2D Pos = (View * 0.5 - T->Pan) / T->Zoom - FVector2D(40, 52);
    SelectedNode = TMOPTheory::AddNode(*T, Kind, Pos, Kind == ETMOPTheoryNodeKind::Note
        ? TEXT("Ny anteckning") : Kind == ETMOPTheoryNodeKind::Vehicle ? TEXT("Välj fordon") : TEXT("Välj person"));
    SelectedLink.Invalidate(); bLinkMode = false; LinkStart.Invalidate(); RefreshDetails();
}
void STMOPTheoryBuilder::ClearSelection()
{
    SelectedNode.Invalidate(); SelectedLink.Invalidate(); LinkStart.Invalidate();
    EditingField.Reset(); bConfirmDeleteTree = false; bLinkMode = false; Status.Reset(); RefreshDetails();
}
void STMOPTheoryBuilder::SelectNode(FGuid Id)
{
    EditingField.Reset(); bConfirmDeleteTree = false;
    SelectedNode = Id; SelectedLink.Invalidate();
    if (bLinkMode)
    {
        if (!LinkStart.IsValid()) { LinkStart = Id; Status = NSLOCTEXT("TMOP", "STMOPTheoryBuilder.95cbf391f2866fa9", "Välj rutan i andra änden av linjen.").ToString(); }
        else if (auto* T = Tree())
        {
            Remember(); const bool bAdded = TMOPTheory::AddLink(*T, LinkStart, Id); LinkStart.Invalidate();
            Status = bAdded ? NSLOCTEXT("TMOP", "STMOPTheoryBuilder.38f5c5f3a50ab4be", "Linje tillagd. Välj två nya rutor eller avsluta linjeläget.").ToString() : NSLOCTEXT("TMOP", "STMOPTheoryBuilder.400797cd732b2442", "Välj två olika rutor utan befintlig linje.").ToString();
        }
    }
    RefreshDetails();
}
void STMOPTheoryBuilder::SelectLink(FGuid Id)
{
    SelectedLink = Id; SelectedNode.Invalidate(); LinkStart.Invalidate(); EditingField.Reset();
    bConfirmDeleteTree = false; RefreshDetails();
}
void STMOPTheoryBuilder::DeleteSelection()
{
    auto* T = Tree(); if (!T) return;
    if (Node() && Node()->bShooter) { Status = NSLOCTEXT("TMOP", "STMOPTheoryBuilder.d2f00b73afdf0334", "Skyttens plats är gemensam för alla mallar och kan inte tas bort.").ToString(); return; }
    if (!Node() && !Link()) return;
    Remember();
    if (Node()) TMOPTheory::RemoveNode(*T, SelectedNode);
    else T->Links.RemoveAll([this](const auto& L) { return L.Id == SelectedLink; });
    ClearSelection();
}
TSharedRef<SWidget> STMOPTheoryBuilder::TreeMenu()
{
    FMenuBuilder Menu(true, nullptr);
    if (auto* P = Player.Get()) for (const auto& T : P->TheoryTrees)
    {
        const FGuid Id = T.Id;
        Menu.AddMenuEntry(FText::FromString(T.Title), FText::GetEmpty(), FSlateIcon(), FUIAction(
            FExecuteAction::CreateSPLambda(this, [this, Id]() { if (!Player.IsValid()) return;
                Player->ActiveTheoryTreeId = Id; ClearSelection(); })));
    }
    if (!Player.IsValid() || Player->TheoryTrees.IsEmpty())
        Menu.AddWidget(SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.688134d7ab984128", "Skapa ett träd med en mall ovan.")), FText::GetEmpty());
    return Menu.MakeWidget();
}
TSharedRef<SWidget> STMOPTheoryBuilder::ObservationMenu()
{
    FMenuBuilder Menu(true, nullptr);
    const auto* N = Node(); int32 Count = 0;
    if (N && Player.IsValid() && !N->bShooter && N->Kind != ETMOPTheoryNodeKind::Note)
    {
        const auto Kind = N->Kind == ETMOPTheoryNodeKind::Vehicle ? ETMOPNotebookEntityKind::Vehicle : ETMOPNotebookEntityKind::Person;
        const FGuid NodeId = N->Id;
        for (const auto& O : Player->NotebookObservations)
        {
            if (O.Kind != Kind || (Kind == ETMOPNotebookEntityKind::Person && O.Category == ETMOPNotebookCategory::Shooter)) continue;
            if (Tree()->Nodes.ContainsByPredicate([&](const auto& Existing) {
                return Existing.Id != NodeId && Existing.Kind == N->Kind && Existing.EntityId == O.EntityId; })) continue;
            ++Count; const FName EntityId = O.EntityId;
            Menu.AddMenuEntry(O.DisplayName, O.Summary, FSlateIcon(), FUIAction(FExecuteAction::CreateSPLambda(this,
                [this, NodeId, EntityId]() { if (auto* T = Tree()) { Remember();
                    if (TMOPTheory::AssignObservation(*T, NodeId, EntityId, Player->NotebookObservations)) Status = NSLOCTEXT("TMOP", "STMOPTheoryBuilder.45b214b2eddcc382", "Observation tillagd i trädet.").ToString();
                    RefreshDetails(); } })));
        }
    }
    if (!Count) Menu.AddWidget(SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.8cfe65a4e2f1c970", "Inga fler insamlade observationer av denna typ.")), FText::GetEmpty());
    return Menu.MakeWidget();
}
void STMOPTheoryBuilder::RefreshDetails()
{
    if (!Details) return;
    Details->ClearChildren();
    if (const auto* N = Node())
    {
        const FGuid Id = N->Id;
        Details->AddSlot().AutoHeight().Padding(0, 4)[SNew(STextBlock)
            .Text(FText::FromString(N->bShooter ? NSLOCTEXT("TMOP", "STMOPTheoryBuilder.cdcf76a29bd80da7", "SKYTTEN — fylls automatiskt när personen har hittats").ToString() : NSLOCTEXT("TMOP", "STMOPTheoryBuilder.50a258183e1a70fd", "MARKERAD RUTA").ToString()))];
        if (!N->bShooter && N->Kind != ETMOPTheoryNodeKind::Note)
            Details->AddSlot().AutoHeight()[SNew(SHorizontalBox)
                + SHorizontalBox::Slot().AutoWidth()[SNew(SComboButton)
                    .OnGetMenuContent(this, &STMOPTheoryBuilder::ObservationMenu)
                    .ButtonContent()[SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.10d36d0ecbe3aa29", "Välj från Mina observationer"))]]
                + SHorizontalBox::Slot().AutoWidth().Padding(8, 0)[SNew(SButton).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.2620769e2acb5509", "Töm rutan"))
                    .OnClicked_Lambda([this]() { if (auto* Selected = Node()) { Remember(); Selected->EntityId = NAME_None;
                        Selected->Title = Selected->Kind == ETMOPTheoryNodeKind::Vehicle ? TEXT("Välj fordon") : TEXT("Välj person"); RefreshDetails(); }
                        return FReply::Handled(); })]];
        if (!N->bShooter && !N->bHeading)
            Details->AddSlot().AutoHeight().Padding(0, 4)[SNew(SEditableTextBox)
                .Text(FText::FromString(N->Role)).HintText(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.e701f54c45fcbe2a", "Roll i teorin"))
                .OnTextChanged_Lambda([this, Id](const FText& Text) { if (auto* Selected = Node()) if (Selected->Id == Id) {
                    RememberField(TEXT("node_role")); Selected->Role = Text.ToString().Left(60); } })];
        Details->AddSlot().AutoHeight().Padding(0, 4)[SNew(SEditableTextBox)
            .Text(FText::FromString(N->Title)).IsReadOnly(N->bShooter || !N->EntityId.IsNone())
            .HintText(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.78f6332c61b6e96f", "Rubrik / roll"))
            .OnTextChanged_Lambda([this, Id](const FText& Text) { if (auto* Selected = Node()) if (Selected->Id == Id) {
                RememberField(TEXT("node_title")); Selected->Title = Text.ToString().Left(120); } })];
        Details->AddSlot().AutoHeight()[SNew(SBox).HeightOverride(100)
            [SNew(SMultiLineEditableTextBox).Text(FText::FromString(N->Notes)).AutoWrapText(true)
                .HintText(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.fc3a3203fdc50906", "Egna anteckningar om denna ruta…"))
                .OnTextChanged_Lambda([this, Id](const FText& Text) { if (auto* Selected = Node()) if (Selected->Id == Id) {
                    RememberField(TEXT("node_notes")); Selected->Notes = Text.ToString().Left(4000); } })]];
    }
    else if (const auto* L = Link())
    {
        const FGuid Id = L->Id;
        Details->AddSlot().AutoHeight().Padding(0, 4)[SNew(STextBlock).Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.34378eb1bbdf38ec", "MARKERAD LINJE — beskriv sambandet"))];
        Details->AddSlot().AutoHeight()[SNew(SEditableTextBox).Text(FText::FromString(L->Label))
            .HintText(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.b0583bb43ab24f9c", "Till exempel: möjlig förare"))
            .OnTextChanged_Lambda([this, Id](const FText& Text) { if (auto* Selected = Link()) if (Selected->Id == Id) {
                RememberField(TEXT("link_label")); Selected->Label = Text.ToString().Left(120); } })];
    }
    else Details->AddSlot().AutoHeight()[SNew(STextBlock)
        .Text(NSLOCTEXT("TMOP", "STMOPTheoryBuilder.87cb1938d6246499", "Välj en ruta för att lägga in en observation eller skriva anteckningar.")).AutoWrapText(true)];
}
