#include "WorldAtlas/TMOPGlobeMath.h"
#include "WorldAtlas/TMOPWorldAtlasData.h"
#include "WorldAtlas/TMOPAtlasAppearance.h"
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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPGlobeLandClipTest, "TMOP.WorldAtlas.LandClipping",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPGlobeLandClipTest::RunTest(const FString&)
{
    FVector Polygon[4];
    TestEqual(TEXT("Far-side land is invisible"), TMOPGlobe::ClipFrontTriangle(FVector(-1,0,0), FVector(-.5,.4,0), FVector(-.5,0,.4), Polygon), 0);
    TestEqual(TEXT("Visible land triangle is retained"), TMOPGlobe::ClipFrontTriangle(FVector(1,0,0), FVector(.5,.4,0), FVector(.5,0,.4), Polygon), 3);
    TestEqual(TEXT("One vertex behind the horizon yields a quadrilateral"), TMOPGlobe::ClipFrontTriangle(FVector(-.5,0,0), FVector(.5,.4,0), FVector(.5,0,.4), Polygon), 4);
    for (const FVector& P : Polygon) TestTrue(TEXT("All clipped vertices are finite and in front"), !P.ContainsNaN() && P.X >= 0);
    TestEqual(TEXT("Two vertices behind the horizon yield one triangle"), TMOPGlobe::ClipFrontTriangle(FVector(.5,0,0), FVector(-.5,.4,0), FVector(-.5,0,.4), Polygon), 3);
    FVector A(-.5, .4, 0), B(.5, 0, .4);
    TestTrue(TEXT("Crossing border is kept"), TMOPGlobe::ClipFrontSegment(A, B));
    TestTrue(TEXT("Border terminates on the limb"), FMath::IsNearlyZero(A.X));
    A = FVector(-.2,0,0); B = FVector(-.8,.2,0);
    TestFalse(TEXT("Hidden border is removed"), TMOPGlobe::ClipFrontSegment(A,B));
    FTMOPAtlasAppearance Appearance;
    if (!TestTrue(TEXT("Staged land, alignments and flags load"), Appearance.Load()))
    { AddError(Appearance.Error); return false; }
    TestTrue(TEXT("Global land coverage loaded"), Appearance.Lands.Num() > 180);
    TestNotNull(TEXT("Soviet flag exists"), Appearance.Flag(TEXT("su")));
    TestNotNull(TEXT("Historical Afghan flag exists"), Appearance.Flag(TEXT("af")));
    TestNotNull(TEXT("Chinese participant flag exists"), Appearance.Flag(TEXT("cn")));
    TestNotNull(TEXT("Vietnamese participant flag exists"), Appearance.Flag(TEXT("vn")));
    TestNull(TEXT("A region is not given a national flag"), Appearance.Flag(TEXT("kurdistan")));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAtlasDatesTest, "TMOP.WorldAtlas.DatesAndData",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPAtlasDatesTest::RunTest(const FString&)
{
    FTMOPWorldAtlasData Data;
    if (!TestTrue(TEXT("Packaged atlas data loads and validates"), Data.Load()))
    { AddError(Data.Error); return false; }
    for (const auto& Country : Data.Entries) if (Country.Kind == TEXT("country"))
        TestFalse(TEXT("Country hierarchy is loaded: ") + Country.Id, Country.Hierarchy.IsEmpty());
    const auto* USA = Data.Find(TEXT("us"));
    if (!TestNotNull(TEXT("USA profile exists"), USA)) return false;
    const auto* FBI = USA->Hierarchy.FindByPredicate([](const auto& Office) { return Office.Id == TEXT("fbi"); });
    if (!TestNotNull(TEXT("FBI office is parsed"), FBI)) return false;
    TestEqual(TEXT("FBI reports to Attorney General"), FBI->Parent, FString(TEXT("minister-3")));
    TestFalse(TEXT("Localized office label is resolved"), USA->Text(FBI->Label).IsEmpty());
    const auto* Border = Data.Find(TEXT("conflict-china-vietnam"));
    if (!TestNotNull(TEXT("China/Vietnam conflict exists"), Border) || Border->Participants.Num() != 2) return false;
    TestEqual(TEXT("Chinese flag reference is parsed"), Border->Participants[0].Flag, FString(TEXT("cn")));
    TestEqual(TEXT("Vietnamese flag reference is parsed"), Border->Participants[1].Flag, FString(TEXT("vn")));
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
