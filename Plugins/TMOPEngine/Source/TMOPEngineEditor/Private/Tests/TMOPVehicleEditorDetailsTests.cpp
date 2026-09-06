#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "STMOPVehicleEditor.h"
#include "STMOPAppearancePreview.h"
#include "TMOPVehicleEditorObjects.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "UObject/GarbageCollection.h"
#include "UObject/StrongObjectPtr.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPVehicleEditorDetailsLifetimeTest,
    "TMOP.VehicleEditor.DetailsLifetimeAndSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMOPVehicleEditorDetailsLifetimeTest::RunTest(const FString&)
{
    TStrongObjectPtr<UDataTable> Table(NewObject<UDataTable>(GetTransientPackage()));
    Table->RowStruct = FTMOPHistoricalVehicleRow::StaticStruct();
    UStaticMesh* Cube = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (!TestNotNull(TEXT("Preview fixture mesh"), Cube)) return false;
    TStrongObjectPtr<UTMOPVehicleModelData> Model(NewObject<UTMOPVehicleModelData>(GetTransientPackage()));
    Model->BodyMesh = Cube;
    FTMOPHistoricalVehicleRow Large;
    Large.VehicleId = TEXT("TEST_LARGE");
    Large.Notes = TEXT("Large vehicle source");
    Large.ModelData = Model.Get();
    for (int32 Index = 0; Index < 256; ++Index)
    {
        FTMOPHistoricalVehicleTimelineEntry Entry;
        Entry.EntryId = FName(*FString::Printf(TEXT("ENTRY_%03d"), Index));
        Entry.RouteViaAnchorIds.Add(TEXT("TEST_VIA"));
        Large.Timeline.Add(Entry);
    }
    FTMOPHistoricalVehicleRow Small;
    Small.VehicleId = TEXT("TEST_SMALL");
    Small.Notes = TEXT("Small vehicle source");
    Small.ModelData = Model.Get();
    Small.Timeline.AddDefaulted();
    Small.Timeline[0].EntryId = TEXT("SMALL_ENTRY");
    Table->AddRow(Large.VehicleId, Large);
    Table->AddRow(Small.VehicleId, Small);

    auto Panel = SNew(STMOPVehicleEditor).VehicleTableOverride(Table.Get());
    const auto CheckPreview = [&](FName Expected)
    {
        const auto* Actor = Cast<ATMOPVehicleBase>(Panel->AppearancePreview->GetPreviewActor());
        if (!TestNotNull(TEXT("Selected vehicle has a preview actor"), Actor)) return false;
        return TestEqual(TEXT("Preview actor belongs to selected vehicle"), Actor->VehicleId, Expected);
    };
    Panel->AddEntry();
    TestEqual(TEXT("Cannot add entries before choosing a vehicle"), Panel->WorkingRow.Timeline.Num(), 0);
    Panel->SelectVehicle(Large.VehicleId);
    TestTrue(TEXT("Selection queues the visible update"), Panel->bPendingDetailsRefresh);
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    if (!TestNotNull(TEXT("Entry view object"), Panel->EntryDetailsObject.Get())) return false;
    TestEqual(TEXT("All authored entries retained"), Panel->WorkingRow.Timeline.Num(), 256);
    TestEqual(TEXT("General panel excludes timeline data"),
        Panel->VehicleDetailsObject->Data.Timeline.Num(), 0);
    if (!CheckPreview(Large.VehicleId)) return false;
    Panel->SelectVehicle(Small.VehicleId);
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    if (!CheckPreview(Small.VehicleId)) return false;
    Panel->SelectVehicle(Large.VehicleId);
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    if (!CheckPreview(Large.VehicleId)) return false;

    TStrongObjectPtr<UTMOPVehicleEntryDetailsObject> OldEntry(Panel->EntryDetailsObject.Get());
    Panel->ClearViaPoints();
    TestEqual(TEXT("Route command modifies working data"),
        Panel->WorkingRow.Timeline[0].RouteViaAnchorIds.Num(), 0);
    TestEqual(TEXT("Visible array remains intact until rebinding"),
        OldEntry->Data.RouteViaAnchorIds.Num(), 1);
    const UObject* EditedObjects[] = { OldEntry.Get() };
    FPropertyChangedEvent Changed(nullptr, EPropertyChangeType::ValueSet, MakeArrayView(EditedObjects));
    OldEntry->Data.Notes = TEXT("Late callback from previous view");
    Panel->OnDetailsChanged(Changed, false);
    TestTrue(TEXT("Late notification cannot overwrite newer command"),
        Panel->WorkingRow.Timeline[0].Notes.IsEmpty());
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    TestTrue(TEXT("Rebind creates a different object"),
        Panel->EntryDetailsObject.Get() != OldEntry.Get());

    // A notification that arrives after the pending flag clears still belongs
    // to the old object and must not read/commit the replacement object's data.
    const auto BeforeLateEvent = Panel->WorkingRow.Timeline[0];
    Panel->EntryDetailsObject->Data.Notes = TEXT("Must not be committed by old callback");
    Panel->OnDetailsChanged(Changed, false);
    TestEqual(TEXT("Old object's callback rejected after rebind"),
        Panel->WorkingRow.Timeline[0].Notes, BeforeLateEvent.Notes);

    EditedObjects[0] = Panel->EntryDetailsObject.Get();
    Panel->EntryDetailsObject->Data.Notes = TEXT("Committed edit");
    Panel->OnDetailsChanged(Changed, false);
    Panel->EntryDetailsObject->Data.Notes = TEXT("Final committed edit");
    Panel->OnDetailsChanged(Changed, false);
    Panel->CommitEntry();
    Panel->CommitVehicle();
    TestEqual(TEXT("Immediate commit before timer keeps latest edit"),
        Panel->WorkingRow.Timeline[0].Notes, FString(TEXT("Final committed edit")));

    // Prevent a save prompt: this fixture deliberately discards its changes.
    Panel->SavedRow = Panel->WorkingRow;
    Panel->SelectVehicle(Small.VehicleId);
    // Stop here on failure: otherwise the next selection could open a modal
    // save prompt and hide the failed assertion by hanging the test runner.
    if (!TestEqual(TEXT("Small vehicle keeps its own metadata"),
        Panel->WorkingRow.VehicleId, Small.VehicleId)) return false;
    TestEqual(TEXT("Small vehicle source retained"), Panel->WorkingRow.Notes, Small.Notes);
    TestFalse(TEXT("Selection alone does not create edits"), Panel->HasUnsavedChanges());
    Panel->SelectVehicle(Large.VehicleId);
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    TestEqual(TEXT("Rapid switching displays the final selected vehicle"),
        Panel->VehicleDetailsObject->Data.VehicleId, Large.VehicleId);
    if (!CheckPreview(Large.VehicleId)) return false;
    TestEqual(TEXT("Switches preserve full timeline"), Panel->WorkingRow.Timeline.Num(), 256);

    Panel->AddAccessory(ETMOPRoofAccessoryType::TaxiSign);
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    if (!TestNotNull(TEXT("Additional accessory has its own object"),
        Panel->AccessoryDetailsObject.Get())) return false;
    Panel->AccessoryDetailsObject->Data.SocketName = TEXT("EditedSocket");
    EditedObjects[0] = Panel->AccessoryDetailsObject.Get();
    Panel->OnAccessoryDetailsChanged(Changed);
    Panel->AccessoryDetailsObject->Data.SocketName = TEXT("FinalSocket");
    Panel->OnAccessoryDetailsChanged(Changed);
    Panel->CommitVehicle();
    TestEqual(TEXT("Accessory edit survives immediate vehicle commit"),
        Panel->WorkingRow.AdditionalAccessories[0].SocketName, FName(TEXT("FinalSocket")));
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    CollectGarbage(RF_NoFlags);
    TestTrue(TEXT("Visible vehicle survives garbage collection"), IsValid(Panel->VehicleDetailsObject.Get()));
    TestTrue(TEXT("Visible entry survives garbage collection"), IsValid(Panel->EntryDetailsObject.Get()));
    TestTrue(TEXT("Visible accessory survives garbage collection"), IsValid(Panel->AccessoryDetailsObject.Get()));
    Panel->RemoveAccessory();
    Panel->ApplyPendingDetailsRefresh(0.0, 0.0f);
    TestNotNull(TEXT("Switch back to roof accessory"), Panel->RoofDetailsObject.Get());
    TestNull(TEXT("Previous accessory type is unbound"), Panel->AccessoryDetailsObject.Get());
    TestTrue(TEXT("Opening/editing does not mutate source table"),
        FTMOPHistoricalVehicleRow::StaticStruct()->CompareScriptStruct(
            &Large, Table->FindRow<FTMOPHistoricalVehicleRow>(Large.VehicleId, TEXT("Test")), 0));
    Panel->SavedRow = Panel->WorkingRow;
    Panel->SelectVehicle(Small.VehicleId);
    Table->RemoveRow(Small.VehicleId);
    Panel->SaveVehicle();
    TestFalse(TEXT("Saving an externally removed row does not recreate it"),
        Table->GetRowMap().Contains(Small.VehicleId));
    return true;
}
#endif
