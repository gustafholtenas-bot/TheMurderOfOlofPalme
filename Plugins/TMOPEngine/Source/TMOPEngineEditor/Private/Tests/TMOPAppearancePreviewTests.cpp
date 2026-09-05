#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "STMOPAppearancePreview.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "JsonObjectConverter.h"
#include "Vehicles/TMOPConfiguredVehicle.h"
#include "Vehicles/TMOPVehicleModelData.h"
#include "Vehicles/TMOPVehiclePresentation.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAppearancePreviewIsolationTest, "TMOP.Appearance.PreviewIsolationAndEquipment",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAppearancePreviewIsolationTest::RunTest(const FString&)
{
    UStaticMesh* Cube=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube"));
    if(!TestNotNull(TEXT("Engine reference mesh"),Cube))return false;
    UTMOPVehicleModelData* Model=NewObject<UTMOPVehicleModelData>();
    Model->BodyMesh=Cube;
    FTMOPHistoricalVehicleRow Row;
    Row.VehicleId=TEXT("PREVIEW_TEST");
    Row.ModelData=Model;Row.bOverrideBodyColor=false;Row.BodyColor=FLinearColor::Red;
    FTMOPVehicleAccessoryVisual Equipment;
    Equipment.AccessoryId=TEXT("TestTaxi");Equipment.Type=ETMOPRoofAccessoryType::TaxiSign;
    Equipment.Mesh=Cube;Equipment.LocalTransform=FTransform(FRotator(0,90,0),FVector(10,20,5),FVector(0.2));
    Row.AdditionalAccessories.Add(Equipment);
    const FTMOPHistoricalVehicleRow Original=Row;
    auto Panel=SNew(STMOPAppearancePreview);
    Panel->ShowVehicle(Row);
    ATMOPConfiguredVehicle* Actor=Cast<ATMOPConfiguredVehicle>(Panel->GetPreviewActor());
    if(!TestNotNull(TEXT("Preview uses game vehicle class"),Actor))return false;
    TestTrue(TEXT("Uses selected model mesh"),
        Actor->BodyMesh->GetStaticMesh() == Cube);
    TestTrue(TEXT("Paint settings copied"),Actor->bOverrideBodyColor==Row.bOverrideBodyColor && Actor->BodyColor==Row.BodyColor);
    TestTrue(TEXT("Isolated editor world"),Actor->GetWorld()->WorldType==EWorldType::EditorPreview);
    TestFalse(TEXT("No gameplay started"),Actor->HasActorBegunPlay());
    TestTrue(TEXT("Preview is transient"),Actor->HasAnyFlags(RF_Transient));
    TestTrue(TEXT("Source row unchanged"),FTMOPHistoricalVehicleRow::StaticStruct()->CompareScriptStruct(&Row,&Original,0));
    auto EquipmentComponents=[](AActor* Owner)
    {
        TArray<UStaticMeshComponent*> Result;
        TInlineComponentArray<UStaticMeshComponent*> Parts(Owner);
        for(auto* Part:Parts)if(Part->ComponentHasTag(TEXT("TMOP.GeneratedVehicleAccessory")))Result.Add(Part);
        return Result;
    };
    auto Parts=EquipmentComponents(Actor);
    TestEqual(TEXT("Exactly one piece of equipment"),Parts.Num(),1);
    if(Parts.Num()==1)
    {
        TestTrue(TEXT("Attached to automatic roof mount"),Parts[0]->GetAttachParent()==Actor->RoofAccessorySocket);
        TestTrue(TEXT("Socket offset and rotation preserved"),Parts[0]->GetRelativeTransform().Equals(Equipment.LocalTransform));
    }
    TMOPVehiclePresentation::ApplyProfile(Actor,Row);
    TestEqual(TEXT("Reapplication does not duplicate equipment"),EquipmentComponents(Actor).Num(),1);
    Row.AdditionalAccessories[0].SocketName=TEXT("MissingSocket");
    TMOPVehiclePresentation::ApplyProfile(Actor,Row);
    TestEqual(TEXT("Invalid socket never puts equipment at origin"),EquipmentComponents(Actor).Num(),0);
    TestTrue(TEXT("Invalid socket explained"),!Actor->AccessoryWarnings.IsEmpty());
    Row.AdditionalAccessories.Reset();
    TMOPVehiclePresentation::ApplyProfile(Actor,Row);
    TestEqual(TEXT("Removal reaches runtime assembly"),EquipmentComponents(Actor).Num(),0);
    Panel->Clear(TEXT("cleared"));
    TestNull(TEXT("Selection clear removes actor"),Panel->GetPreviewActor());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAccessoryImportTest, "TMOP.Appearance.AccessoryDataCompatibility",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAccessoryImportTest::RunTest(const FString&)
{
    FTMOPHistoricalVehicleRow Row;
    const FString Legacy=TEXT("{\"VehicleId\":\"LEGACY\",\"RoofAccessory\":{\"Type\":\"TaxiSign\"}}");
    TestTrue(TEXT("Old table JSON imports"),FJsonObjectConverter::JsonObjectStringToUStruct(Legacy,&Row,0,0));
    TestEqual(TEXT("Legacy roof mount retained"),Row.RoofAccessory.SocketName,FName(TEXT("RoofAccessorySocket")));
    TestEqual(TEXT("Legacy import adds no duplicate accessories"),Row.AdditionalAccessories.Num(),0);
    FTMOPVehicleAccessoryVisual Part;
    Part.AccessoryId=TEXT("Beacon");Part.Type=ETMOPRoofAccessoryType::PoliceBeacon;
    Part.SocketName=TEXT("CustomRoof");Part.LocalTransform.SetTranslation(FVector(12,34,56));
    Row.AdditionalAccessories.Add(Part);
    FString Serialized;
    TestTrue(TEXT("Export"),FJsonObjectConverter::UStructToJsonObjectString(Row,Serialized));
    FTMOPHistoricalVehicleRow Restored;
    TestTrue(TEXT("Reimport"),FJsonObjectConverter::JsonObjectStringToUStruct(Serialized,&Restored,0,0));
    TestTrue(TEXT("Full equipment settings survive export/import"),
        FTMOPHistoricalVehicleRow::StaticStruct()->CompareScriptStruct(&Row,&Restored,0));
    return true;
}
#endif
