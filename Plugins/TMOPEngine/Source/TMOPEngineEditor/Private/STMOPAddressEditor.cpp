#include "STMOPAddressEditor.h"
#include "Addresses/TMOPAddressComponent.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Engine/Selection.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Framework/Docking/TabManager.h"
#include "IStructureDetailsView.h"
#include "InputCoreTypes.h"
#include "Misc/FileHelper.h"
#include "Misc/MessageDialog.h"
#include "PropertyCustomizationHelpers.h"
#include "PropertyEditorModule.h"
#include "Rendering/DrawElements.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "UObject/Package.h"
#include "UObject/StructOnScope.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/SLeafWidget.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
FText Txt(const FString& S) { return FText::FromString(S); }
FString Key(FString S)
{
    S = S.ToLower();
    FString Out;
    S.ReplaceInline(TEXT("å"),TEXT("a")); S.ReplaceInline(TEXT("ä"),TEXT("a")); S.ReplaceInline(TEXT("ö"),TEXT("o"));
    for (int32 I=0;I<S.Len();++I)
    {
        const TCHAR C=S[I];
        if (FChar::IsAlnum(C) || C==TEXT('-')) Out.AppendChar(C);
        // Preserve number ranges: 5_7 / 5-7 must never become house 57.
        else if (C==TEXT('_') && I>0 && I+1<S.Len() && FChar::IsDigit(S[I-1]) && FChar::IsDigit(S[I+1])) Out.AppendChar(TEXT('-'));
    }
    return Out;
}
bool Matches(const ATMOPHistoricalAnchor* A, const FTMOPAddressRegistryRow& R)
{
    if (!A->GetAnchorId().IsNone() && (A->GetAnchorId() == R.EntranceAnchorId || A->GetAnchorId() == R.BuildingAnchorId)) return true;
    // An explicit unresolved link must not silently fall back to another actor.
    if (!R.EntranceAnchorId.IsNone() || !R.BuildingAnchorId.IsNone()) return false;
    const FString Address = Key(FString::Printf(TEXT("%s%d%s"), *R.StreetName, R.StreetNumber, *R.EntranceSuffix));
    const FString Id = Key(R.AddressId.ToString());
    const FString AnchorId = Key(A->GetAnchorId().ToString());
    const FString Label = Key(A->GetActorLabel());
    return AnchorId == Address || Label == Address || (!R.AddressId.IsNone() && (AnchorId == Id || Label == Id));
}
}

class STMOPAddressMap : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS(STMOPAddressMap) {}
    SLATE_END_ARGS()
    void Construct(const FArguments&) { SetClipping(EWidgetClipping::ClipToBounds); }
    struct FPoint { FVector2D Pos; FName Row; FString Label; };
    TArray<FPoint> Points;
    FBox2D Bounds = FBox2D(ForceInit);
    FName Selection;
    TFunction<void(FName)> Click;
    double Zoom = 1;
    FVector2D Pan = FVector2D::ZeroVector;
    bool bPan = false;
    void Fit() { Zoom = 1; Pan = FVector2D::ZeroVector; Invalidate(EInvalidateWidgetReason::Paint); }
    FVector2D Project(FVector2D P, FVector2D Size) const
    {
        if (!Bounds.bIsValid) return Size * .5;
        const FVector2D Span = Bounds.GetSize();
        const double Scale = FMath::Max(.001, FMath::Min((Size.X-60)/FMath::Max(Span.X,1.),(Size.Y-60)/FMath::Max(Span.Y,1.))) * Zoom;
        return (P-Bounds.GetCenter())*Scale + Size*.5 + Pan;
    }
    virtual FVector2D ComputeDesiredSize(float) const override { return FVector2D(480,400); }
    virtual FReply OnMouseWheel(const FGeometry& G, const FPointerEvent& E) override
    {
        const double Old = Zoom;
        Zoom = FMath::Clamp(Zoom*FMath::Pow(1.2,E.GetWheelDelta()),.2,40.);
        const FVector2D M = G.AbsoluteToLocal(E.GetScreenSpacePosition())-G.GetLocalSize()*.5;
        Pan = M-(M-Pan)*(Zoom/Old); Invalidate(EInvalidateWidgetReason::Paint);
        return FReply::Handled();
    }
    virtual FReply OnMouseButtonDown(const FGeometry& G, const FPointerEvent& E) override
    {
        if (E.GetEffectingButton()==EKeys::MiddleMouseButton) { bPan=true; return FReply::Handled().CaptureMouse(SharedThis(this)); }
        if (E.GetEffectingButton()!=EKeys::LeftMouseButton) return FReply::Unhandled();
        double Best=144; FName Row;
        for (const auto& P:Points)
        {
            if (P.Row.IsNone()) continue;
            double D=FVector2D::DistSquared(Project(P.Pos,G.GetLocalSize()),G.AbsoluteToLocal(E.GetScreenSpacePosition()));
            if (D<Best) { Best=D; Row=P.Row; }
        }
        if (!Row.IsNone() && Click) Click(Row);
        return FReply::Handled();
    }
    virtual FReply OnMouseMove(const FGeometry& G,const FPointerEvent& E) override
    {
        if (!bPan || !HasMouseCapture()) return FReply::Unhandled();
        Pan += G.AbsoluteToLocal(E.GetScreenSpacePosition())-G.AbsoluteToLocal(E.GetLastScreenSpacePosition()); Invalidate(EInvalidateWidgetReason::Paint); return FReply::Handled();
    }
    virtual FReply OnMouseButtonUp(const FGeometry&,const FPointerEvent& E) override
    {
        if (E.GetEffectingButton()!=EKeys::MiddleMouseButton) return FReply::Unhandled();
        bPan=false; return FReply::Handled().ReleaseMouseCapture();
    }
    virtual int32 OnPaint(const FPaintArgs&,const FGeometry& G,const FSlateRect&,FSlateWindowElementList& L,int32 Layer,const FWidgetStyle&,bool) const override
    {
        FSlateDrawElement::MakeBox(L,Layer,G.ToPaintGeometry(),FAppStyle::GetBrush("Brushes.Recessed"));
        for (const auto& P:Points)
        {
            const FVector2D At=Project(P.Pos,G.GetLocalSize());
            const bool Selected=!P.Row.IsNone() && P.Row==Selection;
            const FLinearColor Color=P.Row.IsNone()?FLinearColor(.2f,.22f,.25f):Selected?FLinearColor::Yellow:FLinearColor(.1f,.8f,.6f);
            FSlateDrawElement::MakeBox(L,Layer+1,G.ToPaintGeometry(At-FVector2D(4,4),FVector2D(8,8)),FAppStyle::GetBrush("WhiteBrush"),ESlateDrawEffect::None,Color);
            if (!P.Row.IsNone()) FSlateDrawElement::MakeText(L,Layer+2,G.ToPaintGeometry(At+FVector2D(7,-8),FVector2D(240,20)),Txt(P.Label),FAppStyle::GetFontStyle("SmallFont"),ESlateDrawEffect::None,Color);
        }
        return Layer+2;
    }
};

void STMOPAddressEditor::Construct(const FArguments&)
{
    auto& Module=FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs Args; Args.bAllowSearch=true;
    FStructureDetailsViewArgs StructArgs;
    Details=Module.CreateStructureDetailView(Args,StructArgs,nullptr);
    Details->GetOnFinishedChangingPropertiesDelegate().AddLambda([this](const FPropertyChangedEvent&) { bDirty=true; });
    ChildSlot[
      SNew(SVerticalBox)
      +SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(STextBlock).Text(Txt(TEXT("Adressregister — välj tabell eller importera JSON. Grön = entydig adress, gul = vald, grå = övrigt ankare. Karta: world X / -Y.")))]
      +SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(SObjectPropertyEntryBox)
        .AllowedClass(UDataTable::StaticClass())
        .ObjectPath_Lambda([this](){return Table.IsValid()?Table->GetPathName():FString();})
        .OnObjectChanged(this,&STMOPAddressEditor::Load)]
      +SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().FillWidth(1)[SAssignNew(JsonPath,SEditableTextBox).HintText(Txt(TEXT("Full sökväg till DT_TMOP_AddressRegistry.json")))]
        +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(Txt(TEXT("Importera JSON (ny tabell)"))).OnClicked(this,&STMOPAddressEditor::Import)]]
      +SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(SHorizontalBox)
        +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(Txt(TEXT("Tillämpa rad"))).OnClicked_Lambda([this](){Apply();return FReply::Handled();})]
        +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(Txt(TEXT("Spara tabell"))).OnClicked(this,&STMOPAddressEditor::Save)]
        +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(Txt(TEXT("Koppla säkra träffar"))).OnClicked(this,&STMOPAddressEditor::Connect)]
        +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(Txt(TEXT("Koppla valt ankare"))).OnClicked(this,&STMOPAddressEditor::BindSelected)]
        +SHorizontalBox::Slot().AutoWidth()[SNew(SButton).Text(Txt(TEXT("Uppdatera karta"))).OnClicked_Lambda([this](){Refresh();Map->Fit();return FReply::Handled();})]]
      +SVerticalBox::Slot().FillHeight(1)[SNew(SSplitter)
        +SSplitter::Slot().Value(.2f)[SNew(SVerticalBox)
          +SVerticalBox::Slot().AutoHeight()[SNew(SSearchBox).OnTextChanged_Lambda([this](const FText& T){Filter=T.ToString();Refresh();})]
          +SVerticalBox::Slot().FillHeight(1)[SAssignNew(Rows,SScrollBox)]]
        +SSplitter::Slot().Value(.38f)[SNew(SVerticalBox)
          +SVerticalBox::Slot().FillHeight(.65f)[SAssignNew(Map,STMOPAddressMap)]
          +SVerticalBox::Slot().AutoHeight()[SNew(STextBlock).Text(Txt(TEXT("Spelvy (hjul zoomar; mittenknapp panorerar kartan):")))]
          +SVerticalBox::Slot().FillHeight(.35f)[SNew(SScrollBox)+SScrollBox::Slot()[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this](){auto* R=Current();return R?Txt(TMOPAddressDisplay::Directory(*R)):FText::GetEmpty();})]]]
        +SSplitter::Slot().Value(.42f)[Details->GetWidget().ToSharedRef()]]
      +SVerticalBox::Slot().AutoHeight().Padding(4)[SNew(STextBlock).AutoWrapText(true).Text_Lambda([this](){return Txt(Status+(bDirty?TEXT(" | Ej tillämpade radändringar"):TEXT("")));})]
    ];
    Map->Click=[this](FName Name){Select(Name);};
    Status=TEXT("Importera din JSON eller välj en tabell med radtypen TMOPAddressRegistryRow.");
}

FTMOPAddressRegistryRow* STMOPAddressEditor::Current() const
{ return Draft.IsValid()?reinterpret_cast<FTMOPAddressRegistryRow*>(Draft->GetStructMemory()):nullptr; }
bool STMOPAddressEditor::Apply()
{
    if (!bDirty) return true;
    if (!Table.IsValid() || !Current() || SelectedRow.IsNone()) return false;
    if (Current()->AddressId.IsNone()) { Status=TEXT("AddressId får inte vara None."); return false; }
    for (FName N:Table->GetRowNames()) if (N!=SelectedRow && Table->FindRow<FTMOPAddressRegistryRow>(N,TEXT("Validate"))->AddressId==Current()->AddressId)
    { Status=TEXT("AddressId måste vara unikt.");return false; }
    const FScopedTransaction Transaction(Txt(TEXT("Ändra TMOP-adress")));
    Table->Modify(); Table->AddRow(SelectedRow,*Current()); Table->MarkPackageDirty(); bDirty=false;
    Status=TEXT("Raden tillämpad. Spara tabellen för att skriva till disk."); Refresh(); return true;
}
bool STMOPAddressEditor::ResolveDraft()
{
    if (!bDirty) return true;
    const auto Answer=FMessageDialog::Open(EAppMsgType::YesNoCancel,Txt(TEXT("Tillämpa ändringar på vald adress? Nej kastar radutkastet. Avbryt stannar kvar.")));
    if (Answer==EAppReturnType::Cancel) return false;
    if (Answer==EAppReturnType::Yes) return Apply();
    bDirty=false; return true;
}
bool STMOPAddressEditor::CanClose() { return ResolveDraft(); }
void STMOPAddressEditor::Load(const FAssetData& Asset)
{
    if (!ResolveDraft()) return;
    auto* Candidate=Cast<UDataTable>(Asset.GetAsset());
    if (Candidate && Candidate->GetRowStruct()!=FTMOPAddressRegistryRow::StaticStruct()) {Status=TEXT("Fel radtyp. Importera JSON med den nya radtypen.");return;}
    Table.Reset(Candidate); SelectedRow=NAME_None; Draft.Reset(); Details->SetStructureData(nullptr); Refresh();
}
void STMOPAddressEditor::Select(FName Name)
{
    if (Name==SelectedRow || !Table.IsValid() || !ResolveDraft()) return;
    const auto* R=Table->FindRow<FTMOPAddressRegistryRow>(Name,TEXT("Select"),false); if (!R) return;
    SelectedRow=Name;
    Draft=MakeShared<FStructOnScope>(FTMOPAddressRegistryRow::StaticStruct());
    FTMOPAddressRegistryRow::StaticStruct()->CopyScriptStruct(Draft->GetStructMemory(),R);
    Details->SetStructureData(Draft); bDirty=false;
    Map->Selection=Name; Map->Invalidate(EInvalidateWidgetReason::Paint);
}
void STMOPAddressEditor::Refresh()
{
    Rows->ClearChildren(); Map->Points.Reset(); Map->Bounds=FBox2D(ForceInit);
    if (!Table.IsValid()) return;
    TArray<FName> Names=Table->GetRowNames(); Names.Sort(FNameLexicalLess());
    for (FName Name:Names)
    {
        const auto* R=Table->FindRow<FTMOPAddressRegistryRow>(Name,TEXT("List"));
        const FString Label=FString::Printf(TEXT("%s %d%s"),*R->StreetName,R->StreetNumber,*R->EntranceSuffix);
        if (!Filter.IsEmpty() && !(Label+Name.ToString()+R->RegistrySearchText+TMOPAddressDisplay::Directory(*R)).Contains(Filter)) continue;
        Rows->AddSlot()[SNew(SButton).Text(Txt(Label)).OnClicked_Lambda([this,Name](){Select(Name);return FReply::Handled();})];
    }
    UWorld* W=GEditor?GEditor->GetEditorWorldContext().World():nullptr;
    if (!W) return;
    for (TActorIterator<ATMOPHistoricalAnchor> It(W);It;++It)
    {
        STMOPAddressMap::FPoint P; const FVector V=It->GetActorLocation(); P.Pos=FVector2D(V.X,-V.Y);
        int32 Count=0;
        for (FName Name:Names) if (Matches(*It,*Table->FindRow<FTMOPAddressRegistryRow>(Name,TEXT("Map")))) { P.Row=Name; ++Count; }
        if (Count!=1) P.Row=NAME_None;
        P.Label=It->GetActorLabel(); Map->Bounds+=P.Pos; Map->Points.Add(P);
    }
    Map->Invalidate(EInvalidateWidgetReason::Paint);
}
FReply STMOPAddressEditor::Import()
{
    if (!ResolveDraft()) return FReply::Handled();
    FString Json;
    if (!FFileHelper::LoadFileToString(Json,*JsonPath->GetText().ToString().TrimStartAndEnd())) { Status=TEXT("Kunde inte läsa JSON-sökvägen.");return FReply::Handled(); }
    TStrongObjectPtr<UDataTable> Temp(NewObject<UDataTable>()); Temp->RowStruct=FTMOPAddressRegistryRow::StaticStruct();
    Temp->bIgnoreMissingFields=true; // New optional family-confirmation fields default to false/empty.
    const TArray<FString> Errors=Temp->CreateTableFromJSONString(Json);
    if (!Errors.IsEmpty() || Temp->GetRowNames().IsEmpty()) {Status=TEXT("Import avbruten: ")+FString::Join(Errors,TEXT("; "));return FReply::Handled();}
    TSet<FName> IDs;
    for (FName N:Temp->GetRowNames())
    {
        const FName Id=Temp->FindRow<FTMOPAddressRegistryRow>(N,TEXT("Import"))->AddressId;
        if (Id.IsNone() || IDs.Contains(Id)) {Status=TEXT("Import avbruten: tomt eller duplicerat AddressId.");return FReply::Handled();} IDs.Add(Id);
    }
    const FString AssetName=TEXT("DT_TMOP_AddressRegistry_")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    UPackage* Package=CreatePackage(*(TEXT("/Game/TMOP/Data/")+AssetName));
    UDataTable* NewTable=NewObject<UDataTable>(Package,*AssetName,RF_Public|RF_Standalone|RF_Transactional);
    NewTable->RowStruct=FTMOPAddressRegistryRow::StaticStruct();
    for (FName N:Temp->GetRowNames()) NewTable->AddRow(N,*Temp->FindRow<FTMOPAddressRegistryRow>(N,TEXT("Copy")));
    FAssetRegistryModule::AssetCreated(NewTable); NewTable->MarkPackageDirty();
    Table.Reset(NewTable);SelectedRow=NAME_None;Draft.Reset();Details->SetStructureData(nullptr);Refresh();
    Status=FString::Printf(TEXT("Importerade %d adresser utan att skriva över befintliga tabeller. Klicka Spara tabell."),NewTable->GetRowNames().Num());
    return FReply::Handled();
}
FReply STMOPAddressEditor::Save()
{
    if (Apply() && Table.IsValid())
    {
        TArray<UPackage*> Packages{Table->GetOutermost()};
        FEditorFileUtils::PromptForCheckoutAndSave(Packages,true,false);
        Status=Table->GetOutermost()->IsDirty()?TEXT("Tabellen är fortfarande osparad."):TEXT("Tabellen sparad. Spara även banan efter ankarkoppling.");
    }
    return FReply::Handled();
}
bool STMOPAddressEditor::Bind(ATMOPHistoricalAnchor* Anchor,FName Name)
{
    if (!Anchor || Anchor->GetAnchorId().IsNone()) return false;
    UWorld* W=Anchor->GetWorld(); if (!W) return false;
    // Do not create ambiguous IDs or leave an old component linked elsewhere.
    for (TActorIterator<ATMOPHistoricalAnchor> It(W);It;++It)
    {
        if (*It==Anchor) continue;
        if (It->GetAnchorId()==Anchor->GetAnchorId()) return false;
        auto* C=It->FindComponentByClass<UTMOPAddressComponent>();
        if (C && C->Registry==Table.Get() && C->RowName==Name) return false;
    }
    for (FName N:Table->GetRowNames())
    {
        if (N==Name) continue;
        const auto* Other=Table->FindRow<FTMOPAddressRegistryRow>(N,TEXT("Check existing links"));
        if (Other->EntranceAnchorId==Anchor->GetAnchorId() || Other->BuildingAnchorId==Anchor->GetAnchorId()) return false;
    }
    auto* Existing=Anchor->FindComponentByClass<UTMOPAddressComponent>();
    if (Existing && (Existing->Registry!=Table.Get() || Existing->RowName!=Name)) return false;
    auto* R=Table->FindRow<FTMOPAddressRegistryRow>(Name,TEXT("Bind")); if (!R) return false;
    Anchor->Modify(); Table->Modify();
    if (!Existing)
    {
        Existing=NewObject<UTMOPAddressComponent>(Anchor,NAME_None,RF_Transactional);
        Anchor->AddInstanceComponent(Existing); Existing->RegisterComponent();
    }
    Existing->Modify(); Existing->Registry=Table.Get();Existing->RowName=Name;
    R->EntranceAnchorId=Anchor->GetAnchorId();
    Anchor->MarkPackageDirty();Table->MarkPackageDirty(); return true;
}
FReply STMOPAddressEditor::Connect()
{
    if (!Apply() || !Table.IsValid() || !GEditor || GEditor->PlayWorld) return FReply::Handled();
    UWorld* W=GEditor->GetEditorWorldContext().World(); if (!W) return FReply::Handled();
    const FScopedTransaction Transaction(Txt(TEXT("Koppla TMOP-adresser")));
    int32 Linked=0,Skipped=0;
    for (FName N:Table->GetRowNames())
    {
        auto* R=Table->FindRow<FTMOPAddressRegistryRow>(N,TEXT("Connect"));
        TArray<ATMOPHistoricalAnchor*> Candidates;
        for (TActorIterator<ATMOPHistoricalAnchor> It(W);It;++It) if (Matches(*It,*R)) Candidates.Add(*It);
        bool Unique=Candidates.Num()==1;
        if (Unique)
        {
            for (FName Other:Table->GetRowNames()) if (Other!=N && Matches(Candidates[0],*Table->FindRow<FTMOPAddressRegistryRow>(Other,TEXT("Conflict")))) Unique=false;
            int32 IdCount=0; for (TActorIterator<ATMOPHistoricalAnchor> It(W);It;++It) if (It->GetAnchorId()==Candidates[0]->GetAnchorId()) ++IdCount;
            Unique &= IdCount==1;
        }
        if (Unique && Bind(Candidates[0],N)) ++Linked; else ++Skipped;
    }
    const FName Old=SelectedRow; SelectedRow=NAME_None;Select(Old); Refresh();
    Status=FString::Printf(TEXT("Kopplade %d. Hoppade över %d saknade/tvetydiga/konfliktande träffar. Spara tabellen OCH banan."),Linked,Skipped);
    return FReply::Handled();
}
FReply STMOPAddressEditor::BindSelected()
{
    if (!Apply() || !Table.IsValid() || SelectedRow.IsNone() || !GEditor || GEditor->PlayWorld) return FReply::Handled();
    TArray<ATMOPHistoricalAnchor*> Actors;
    for (FSelectionIterator It(*GEditor->GetSelectedActors());It;++It) if (auto* A=Cast<ATMOPHistoricalAnchor>(*It)) Actors.Add(A);
    if (Actors.Num()!=1) {Status=TEXT("Välj exakt ett Historical Anchor i banan och en adress i listan.");return FReply::Handled();}
    if (FMessageDialog::Open(EAppMsgType::YesNo,Txt(TEXT("Koppla vald adress till ")+Actors[0]->GetActorLabel()+TEXT("?")))!=EAppReturnType::Yes) return FReply::Handled();
    const FScopedTransaction Transaction(Txt(TEXT("Koppla TMOP-adress manuellt")));
    Status=Bind(Actors[0],SelectedRow)?TEXT("Kopplad. Spara tabellen och banan."):TEXT("Ankaret saknar ID eller har redan en annan adresskomponent. Ingen överskrivning gjord.");
    const FName Old=SelectedRow;SelectedRow=NAME_None;Select(Old);Refresh();return FReply::Handled();
}

void RegisterTMOPAddressEditor()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner("TMOPAddressEditor",FOnSpawnTab::CreateLambda([](const FSpawnTabArgs&){
        auto Editor=SNew(STMOPAddressEditor);
        auto Tab=SNew(SDockTab).TabRole(ETabRole::NomadTab)[Editor];
        Tab->SetCanCloseTab(SDockTab::FCanCloseTab::CreateSP(Editor,&STMOPAddressEditor::CanClose)); return Tab;
    })).SetDisplayName(Txt(TEXT("TMOP Address Editor"))).SetMenuType(ETabSpawnerMenuType::Hidden);
}
void UnregisterTMOPAddressEditor() { FGlobalTabmanager::Get()->UnregisterNomadTabSpawner("TMOPAddressEditor"); }
void AddTMOPAddressEditorMenu()
{
    auto& S=UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Tools")->FindOrAddSection("TMOP");
    S.AddMenuEntry("OpenTMOPAddressEditor",Txt(TEXT("TMOP Address Editor")),Txt(TEXT("Redigera adressregister och ankarkopplingar.")),FSlateIcon(),FUIAction(FExecuteAction::CreateLambda([](){FGlobalTabmanager::Get()->TryInvokeTab(FName("TMOPAddressEditor"));})));
}
