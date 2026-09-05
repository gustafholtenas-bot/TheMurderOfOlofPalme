#include "Addresses/TMOPAddressRegistryTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAddressDisplayTest,"TMOP.Address.Display",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAddressDisplayTest::RunTest(const FString&)
{
    FTMOPAddressResident P; P.ArchivalFullName=TEXT("Johan Gustaf Andersson");
    TestEqual(TEXT("Default abbreviation"),TMOPAddressDisplay::Resident(P),FString(TEXT("J. Andersson")));
    P.InGameDisplayName=TEXT("G. Andersson");
    TestEqual(TEXT("Explicit call name"),TMOPAddressDisplay::Resident(P),P.InGameDisplayName);
    P.InGameDisplayName.Empty();
    FTMOPAddressHousehold H; H.Residents.Add(P);H.Residents.Add(P);
    TestFalse(TEXT("No inferred family"),TMOPAddressDisplay::Household(H).StartsWith(TEXT("Familjen")));
    H.bConfirmedFamily=true;H.FamilySurname=TEXT("Andersson");
    TestEqual(TEXT("Confirmed family"),TMOPAddressDisplay::Household(H),FString(TEXT("Familjen Andersson")));
    FTMOPAddressRegistryRow R;R.StreetName=TEXT("Sveavägen");R.StreetNumber=10;R.Households.Add(H);
    TestTrue(TEXT("Unknown floor is explicit"),TMOPAddressDisplay::Directory(R).Contains(TEXT("Våning okänd")));
    P.BirthDateIso=TEXT("1950-01-01");P.BirthPlace=TEXT("Private archive field");
    H.Residents={P};R.Households={H};
    TestFalse(TEXT("No birth data in directory"),TMOPAddressDisplay::Directory(R).Contains(P.BirthDateIso));
    TestFalse(TEXT("No birthplace in directory"),TMOPAddressDisplay::Directory(R).Contains(P.BirthPlace));
    return true;
}
#endif
