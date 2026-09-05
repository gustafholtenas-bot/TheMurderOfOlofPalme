#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "UObject/StrongObjectPtr.h"
#include "Addresses/TMOPAddressRegistryTypes.h"

class IStructureDetailsView;
class FStructOnScope;
class SScrollBox;
class SEditableTextBox;
class STMOPAddressMap;
class ATMOPHistoricalAnchor;
struct FAssetData;

class STMOPAddressEditor : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPAddressEditor) {}
    SLATE_END_ARGS()
    void Construct(const FArguments&);
    bool CanClose();
private:
    TStrongObjectPtr<UDataTable> Table;
    TSharedPtr<FStructOnScope> Draft;
    TSharedPtr<IStructureDetailsView> Details;
    TSharedPtr<SScrollBox> Rows;
    TSharedPtr<SEditableTextBox> JsonPath;
    TSharedPtr<STMOPAddressMap> Map;
    FName SelectedRow;
    FString Filter, Status;
    bool bDirty = false;
    void Load(const FAssetData& Asset);
    void Refresh();
    void Select(FName Name);
    bool ResolveDraft();
    bool Apply();
    FReply Import();
    FReply Save();
    FReply Connect();
    FReply BindSelected();
    bool Bind(ATMOPHistoricalAnchor* Anchor, FName Name);
    FTMOPAddressRegistryRow* Current() const;
};

void RegisterTMOPAddressEditor();
void UnregisterTMOPAddressEditor();
void AddTMOPAddressEditorMenu();
