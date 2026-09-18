#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

class ATMOPPlayerCharacter;

/** Two independently virtualized lists; only visible cards decode their images. */
class STMOPNotebookPanel : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPNotebookPanel) {}
        SLATE_ARGUMENT(ATMOPPlayerCharacter*, Player)
    SLATE_END_ARGS()
    void Construct(const FArguments& Args);
private:
    struct FItem
    {
        FText Heading;
        int32 EntryIndex = INDEX_NONE;
        bool bEmpty = false;
    };
    using FItemPtr = TSharedPtr<FItem>;
    TWeakObjectPtr<ATMOPPlayerCharacter> Player;
    TArray<FItemPtr> PeopleItems, VehicleItems;
    TSet<int32> PreviewAttempted;
    TSharedRef<ITableRow> MakeRow(FItemPtr Item, const TSharedRef<STableViewBase>& Owner);
    TSharedRef<SWidget> MakeColumn(const FText& Title, TArray<FItemPtr>& Items);
};
