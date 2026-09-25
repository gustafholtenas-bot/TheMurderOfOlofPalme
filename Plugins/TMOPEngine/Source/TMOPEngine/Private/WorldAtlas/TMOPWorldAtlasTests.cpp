#include "WorldAtlas/TMOPGlobeMath.h"
#include "WorldAtlas/TMOPWorldAtlasData.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPGlobeProjectionTest, "TMOP.WorldAtlas.Projection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPGlobeProjectionTest::RunTest(const FString&)
{
    const FVector2D Center(300, 300);
    FVector2D P;
    for (const FVector2D LatLon : {FVector2D(0, 0), FVector2D(62, 15), FVector2D(-29, 25), FVector2D(38, -98)})
    {
        const FVector Point = TMOPGlobe::Unit(LatLon.X, LatLon.Y);
        TestTrue(TEXT("Unit radius"), FMath::IsNearlyEqual(Point.Size(), 1.0));
        TestTrue(TEXT("Focused point is front-facing"), TMOPGlobe::Project(Point, TMOPGlobe::View(LatLon.X, LatLon.Y), Center, 250, P));
        TestTrue(TEXT("Focused point is centred"), (P - Center).Size() < .0001);
        TestFalse(TEXT("Antipode is hidden"), TMOPGlobe::Project(-Point, TMOPGlobe::View(LatLon.X, LatLon.Y), Center, 250, P));
    }
    TMOPGlobe::Project(TMOPGlobe::Unit(0, 30), FQuat::Identity, Center, 250, P);
    TestTrue(TEXT("East appears to the right"), P.X > Center.X);
    TMOPGlobe::Project(TMOPGlobe::Unit(30, 0), FQuat::Identity, Center, 250, P);
    TestTrue(TEXT("North appears above centre"), P.Y < Center.Y);
    const FVector A = TMOPGlobe::Unit(0, 179), B = TMOPGlobe::Unit(0, -179);
    TestTrue(TEXT("Date-line arc takes short path"), TMOPGlobe::Arc(A, B, .5).X < -.99);
    TestTrue(TEXT("Antipodal interpolation remains finite"), FMath::IsNearlyEqual(TMOPGlobe::Arc(A, -A, .5).Size(), 1.0));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAtlasDatesTest, "TMOP.WorldAtlas.DatesAndData",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAtlasDatesTest::RunTest(const FString&)
{
    FTMOPWorldAtlasData Data;
    if (!TestTrue(TEXT("Packaged atlas data loads and validates"), Data.Load()))
    { AddError(Data.Error); return false; }
    const auto* Meeting = Data.Find(TEXT("bilderberg-1986"));
    const auto* Funds = Data.Find(TEXT("iran-money"));
    const auto* PriorShipment = Data.Find(TEXT("contra-portugal"));
    if (!TestNotNull(TEXT("Future meeting exists"), Meeting) || !TestNotNull(TEXT("Annual finance overview exists"), Funds) ||
        !TestNotNull(TEXT("Prior shipment exists"), PriorShipment)) return false;
    TestFalse(TEXT("Future meeting hidden on murder date"), Meeting->Visible(false));
    TestTrue(TEXT("Future meeting available in later history"), Meeting->Visible(true));
    TestFalse(TEXT("Annual finance overview is not murder-date fact"), Funds->Visible(false));
    TestTrue(TEXT("Annual finance overview is opt-in"), Funds->Visible(true));
    TestTrue(TEXT("Earlier shipment remains historical context"), PriorShipment->Visible(false));
    const auto* FSLN = Data.Find(TEXT("ni-fsln"));
    const auto* Contras = Data.Find(TEXT("ni-contras"));
    if (!TestNotNull(TEXT("Government actor exists"), FSLN) || !TestNotNull(TEXT("Opposition actor exists"), Contras)) return false;
    TestTrue(TEXT("Both actors visible on reference date"), FSLN->Visible(false) && Contras->Visible(false));
    TestEqual(TEXT("Actor has correct parent"), FSLN->Country, FString(TEXT("ni")));
    TestTrue(TEXT("Actor hit circles do not overlap"), (FSLN->MarkerOffset - Contras->MarkerOffset).Size() > 28);
    TestEqual(TEXT("Finance beneficiary is the opposition actor"), Funds->Route.Last(), FString(TEXT("ni-contras")));
    for (const FString Id : {TEXT("conflict-south-yemen"), TEXT("conflict-sidra"), TEXT("conflict-libya-airstrike")})
    {
        const auto* E = Data.Find(Id);
        if (!TestNotNull(TEXT("Nearby entry exists"), E)) return false;
        TestFalse(TEXT("Nearby entry absent from exact-date view"), E->Visible(false));
        TestFalse(TEXT("Later-history switch cannot enable nearby conflict"), E->Visible(true));
        TestTrue(TEXT("Nearby switch includes it"), E->Visible(false, true));
    }
    FTMOPAtlasEntry Boundary;
    Boundary.Kind = TEXT("conflict");
    Boundary.From = Boundary.To = TEXT("1985-11-30");
    TestTrue(TEXT("Minus 90 days included"), Boundary.Visible(false, true));
    Boundary.From = Boundary.To = TEXT("1985-11-29");
    TestFalse(TEXT("Minus 91 days excluded"), Boundary.Visible(true, true));
    Boundary.From = Boundary.To = TEXT("1986-05-29");
    TestTrue(TEXT("Plus 90 days included"), Boundary.Visible(false, true));
    Boundary.From = Boundary.To = TEXT("1986-05-30");
    TestFalse(TEXT("Plus 91 days excluded"), Boundary.Visible(true, true));
    const auto* Egypt = Data.Find(TEXT("conflict-egypt-mutiny"));
    if (!TestNotNull(TEXT("Murder-date mutiny exists"), Egypt)) return false;
    TestTrue(TEXT("Inclusive end date"), Egypt->Visible(false));
    return true;
}
#endif
