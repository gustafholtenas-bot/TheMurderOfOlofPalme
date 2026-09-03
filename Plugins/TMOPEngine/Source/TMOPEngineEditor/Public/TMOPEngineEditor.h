#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class SDockTab;
class FSpawnTabArgs;

class FTMOPEngineEditorModule final : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

private:
    void RegisterMenus();
    void OpenPeopleEditor();
    void OpenVehicleEditor();
    void GenerateExitChildrenFromSelection();
    void GenerateIntersectionCornersFromSelection();
    void SnapSelectedVehicleAnchorsToGround();
    TSharedRef<SDockTab> SpawnPeopleEditorTab(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnVehicleEditorTab(const FSpawnTabArgs& Args);

    static const FName PeopleEditorTabName;
    static const FName VehicleEditorTabName;
};
