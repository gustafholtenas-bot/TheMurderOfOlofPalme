#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Input/SButton.h"
#include "Research/TMOPTheoryTypes.h"

class ATMOPPlayerCharacter;
class SVerticalBox;
class STMOPTheoryCanvas;

/** Runtime Slate editor; no UnrealEd/GraphEditor dependencies in packaged games. */
class STMOPTheoryBuilder : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPTheoryBuilder) {}
        SLATE_ARGUMENT(ATMOPPlayerCharacter*, Player)
        SLATE_EVENT(FOnClicked, OnSave)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);

private:
    friend class STMOPTheoryCanvas;
    struct FHistory
    {
        TArray<FTMOPTheoryTree> Trees;
        FGuid Active;
    };
    TWeakObjectPtr<ATMOPPlayerCharacter> Player;
    FOnClicked Save;
    TSharedPtr<SVerticalBox> Details;
    TSharedPtr<STMOPTheoryCanvas> Canvas;
    TArray<FHistory> UndoStack, RedoStack;
    FGuid SelectedNode, SelectedLink, LinkStart;
    FString Status, EditingField;
    bool bLinkMode = false;
    bool bConfirmDeleteTree = false;

    FTMOPTheoryTree* Tree() const;
    FTMOPTheoryNode* Node() const;
    FTMOPTheoryLink* Link() const;
    void Remember();
    void RememberField(const FString& Field);
    void RefreshDetails();
    void SelectNode(FGuid Id);
    void SelectLink(FGuid Id);
    void ClearSelection();
    void DeleteSelection();
    void Undo(bool bRedo);
    void NewTree(int32 TemplateIndex);
    void AddNode(ETMOPTheoryNodeKind Kind);
    TSharedRef<SWidget> TreeMenu();
    TSharedRef<SWidget> ObservationMenu();
};
