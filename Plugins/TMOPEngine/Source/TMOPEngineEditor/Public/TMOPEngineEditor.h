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
    void OpenLaneRepairEditor();
    void OpenObservationEditor();
    void CreateOrUpdateNewspapersFromSelectedFolders();
    void GenerateExitChildrenFromSelection();
    void GenerateIntersectionCornersFromSelection();
    void SnapSelectedVehicleAnchorsToGround();
    TSharedRef<SDockTab> SpawnPeopleEditorTab(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnVehicleEditorTab(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnLaneRepairEditorTab(const FSpawnTabArgs& Args);
    TSharedRef<SDockTab> SpawnObservationEditorTab(const FSpawnTabArgs& Args);

    static const FName PeopleEditorTabName;
    static const FName VehicleEditorTabName;
    static const FName LaneRepairEditorTabName;
    static const FName ObservationEditorTabName;
};
