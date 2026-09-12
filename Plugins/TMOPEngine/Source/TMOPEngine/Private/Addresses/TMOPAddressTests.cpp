#include "Addresses/TMOPAddressRegistryTypes.h"
#include "Addresses/TMOPAddressComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAddressDisplayTest,"TMOP.Address.Display",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAddressDisplayTest::RunTest(const FString&)
{
    FTMOPAddressResident P; P.ArchivalFullName=TEXT("Johan Gustaf Andersson");
    TestEqual(TEXT("Default abbreviation"),TMOPAddressDisplay::Resident(P),FString(TEXT("J. A.")));
    P.InGameDisplayName=TEXT("G. Andersson");
    TestEqual(TEXT("Explicit call name"),TMOPAddressDisplay::Resident(P),FString(TEXT("G. A.")));
    P.InGameDisplayName.Empty();
    FTMOPAddressHousehold H; H.Residents.Add(P);H.Residents.Add(P);
    TestFalse(TEXT("No inferred family"),TMOPAddressDisplay::Household(H).StartsWith(TEXT("Familjen")));
    H.bConfirmedFamily=true;H.FamilySurname=TEXT("Andersson");
    TestEqual(TEXT("Confirmed family"),TMOPAddressDisplay::Household(H),FString(TEXT("Familjen A.")));
    FTMOPAddressRegistryRow R;R.StreetName=TEXT("Sveavägen");R.StreetNumber=10;R.Households.Add(H);
    TestTrue(TEXT("Unknown floor is explicit"),TMOPAddressDisplay::Directory(R).Contains(TEXT("Våning okänd")));
    P.BirthDateIso=TEXT("1950-01-01");P.BirthPlace=TEXT("Private archive field");
    H.Residents={P};R.Households={H};
    TestFalse(TEXT("No birth data in directory"),TMOPAddressDisplay::Directory(R).Contains(P.BirthDateIso));
    TestFalse(TEXT("No birthplace in directory"),TMOPAddressDisplay::Directory(R).Contains(P.BirthPlace));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAddressComponentTest,"TMOP.Address.Component",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAddressComponentTest::RunTest(const FString&)
{
    auto* Component = NewObject<UTMOPAddressComponent>();
    TestFalse(TEXT("Missing registry is not targetable"), Component->HasValidAddress());
    auto* Table = NewObject<UDataTable>();
    Table->RowStruct = FTMOPAddressRegistryRow::StaticStruct();
    Component->Registry = Table;
    Component->RowName = TEXT("TEST_ADDRESS");
    TestFalse(TEXT("Missing row is not targetable"), Component->HasValidAddress());
    FTMOPAddressRegistryRow Row;
    Row.StreetName = TEXT("Testgatan"); Row.StreetNumber = 12; Row.EntranceSuffix = TEXT("A");
    FTMOPAddressHousehold Upper, Lower;
    Upper.FloorNumber = 4; Lower.FloorNumber = 1;
    FTMOPAddressResident Resident;
    Resident.ArchivalFullName = TEXT("Anna Andersson");
    Upper.Residents.Add(Resident); Lower.Residents.Add(Resident);
    Row.Households = {Upper, Lower};
    Table->AddRow(Component->RowName, Row);
    TestTrue(TEXT("Linked row is targetable"), Component->HasValidAddress());
    TestEqual(TEXT("Title preserves entrance suffix"), Component->GetAddressTitle().ToString(),
        FString(TEXT("Testgatan 12A")));
    const FString Directory = Component->GetResidentDirectory().ToString();
    TestTrue(TEXT("Floors appear in ascending order"),
        Directory.Find(TEXT("Våning 1")) < Directory.Find(TEXT("Våning 4")));
    TestTrue(TEXT("Directory abbreviates given names and surnames"), Directory.Contains(TEXT("A. A.")));
    const UTMOPInspectableComponent* Shared = Component;
    TestTrue(TEXT("Address works through shared inspection API"), Shared->HasReadableContent());
    TestEqual(TEXT("Shared title equals address title"), Shared->GetInspectionTitle().ToString(),
        Component->GetAddressTitle().ToString());
    TestFalse(TEXT("Shared body does not repeat the title"), Shared->GetInspectionText().ToString().Contains(TEXT("Testgatan")));
    TestTrue(TEXT("Address source is not automatically exposed"), Shared->GetInspectionSource().IsEmpty());
    TestEqual(TEXT("Existing address height retained"), Component->InteractionOffset.Z, 140.0);
    Component->bInteractionEnabled = false;
    TestFalse(TEXT("Disabled address is not targetable"), Component->HasValidAddress());
    Component->bInteractionEnabled = true;
    Table->RemoveRow(Component->RowName);
    TestFalse(TEXT("Deleted row stops being targetable"), Component->HasValidAddress());
    TestTrue(TEXT("Deleted row produces no directory"), Component->GetResidentDirectory().IsEmpty());
    return true;
}
#endif
