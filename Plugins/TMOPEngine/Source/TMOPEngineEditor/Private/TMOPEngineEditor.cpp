#include "TMOPEngineEditor.h"

#include "Framework/Docking/TabManager.h"
#include "STMOPPeopleEditor.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "TMOPEngineEditor"

const FName FTMOPEngineEditorModule::PeopleEditorTabName(
    TEXT("TMOPPeopleEditor"));

void FTMOPEngineEditorModule::StartupModule()
{
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        PeopleEditorTabName,
        FOnSpawnTab::CreateRaw(
            this, &FTMOPEngineEditorModule::SpawnPeopleEditorTab))
        .SetDisplayName(LOCTEXT("PeopleEditorTitle", "TMOP People Editor"))
        .SetTooltipText(LOCTEXT(
            "PeopleEditorTooltip",
            "Edit DT_TMOP_People in a focused visual timeline editor."))
        .SetMenuType(ETabSpawnerMenuType::Hidden);

    UToolMenus::RegisterStartupCallback(
        FSimpleMulticastDelegate::FDelegate::CreateRaw(
            this, &FTMOPEngineEditorModule::RegisterMenus));
}

void FTMOPEngineEditorModule::ShutdownModule()
{
    UToolMenus::UnRegisterStartupCallback(this);
    UToolMenus::UnregisterOwner(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PeopleEditorTabName);
}

void FTMOPEngineEditorModule::RegisterMenus()
{
    FToolMenuOwnerScoped OwnerScoped(this);
    UToolMenu* ToolsMenu =
        UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
    FToolMenuSection& Section =
        ToolsMenu->FindOrAddSection(TEXT("TMOP"));
    Section.AddMenuEntry(
        TEXT("OpenTMOPPeopleEditor"),
        LOCTEXT("OpenPeopleEditor", "TMOP People Editor"),
        LOCTEXT("OpenPeopleEditorTooltip",
            "Open the visual person and timeline editor."),
        FSlateIcon(),
        FUIAction(FExecuteAction::CreateRaw(
            this, &FTMOPEngineEditorModule::OpenPeopleEditor)));
}

void FTMOPEngineEditorModule::OpenPeopleEditor()
{
    FGlobalTabmanager::Get()->TryInvokeTab(PeopleEditorTabName);
}

TSharedRef<SDockTab> FTMOPEngineEditorModule::SpawnPeopleEditorTab(
    const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(STMOPPeopleEditor)
        ];
}

IMPLEMENT_MODULE(FTMOPEngineEditorModule, TMOPEngineEditor)

#undef LOCTEXT_NAMESPACE
