#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Vehicles/TMOPVehicleRoutePlan.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPVehicleDriverInheritanceTest,
    "TMOP.VehicleTimeline.DriverInheritance",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPVehicleDriverInheritanceTest::RunTest(const FString&)
{
    FTMOPHistoricalVehicleRow Row;
    Row.KnownDriverEntityId = TEXT("DEFAULT");
    Row.Timeline.SetNum(4);
    Row.Timeline[0].Action = ETMOPHistoricalVehicleAction::Spawn;
    Row.Timeline[0].DriverEntityId = TEXT("AKE_LARSSON");
    Row.Timeline[1].EntryId = TEXT("STOP");
    Row.Timeline[2].EntryId = TEXT("DRIVE");
    Row.Timeline[2].Action = ETMOPHistoricalVehicleAction::BeginDriving;
    TestEqual(TEXT("Missing driver inherits prior boarding row"),
        TMOPVehicleRoute::Driver(Row, Row.Timeline[2]), FName(TEXT("AKE_LARSSON")));
    auto Copy = Row.Timeline[2];
    TestEqual(TEXT("Editor copies use the same resolution"),
        TMOPVehicleRoute::Driver(Row, Copy), FName(TEXT("AKE_LARSSON")));
    Row.Timeline[1].DriverEntityId = TEXT("REPLACEMENT_DRIVER");
    TestEqual(TEXT("Most recent driver wins over vehicle default"),
        TMOPVehicleRoute::Driver(Row, Copy), FName(TEXT("REPLACEMENT_DRIVER")));
    Copy.DriverEntityId = TEXT("EXPLICIT");
    TestEqual(TEXT("Explicit override wins"),
        TMOPVehicleRoute::Driver(Row, Copy), FName(TEXT("EXPLICIT")));
    Copy.DriverEntityId = NAME_None;
    Row.Timeline[1].Action = ETMOPHistoricalVehicleAction::Despawn;
    TestEqual(TEXT("No inheritance across despawn"),
        TMOPVehicleRoute::Driver(Row, Copy), FName(TEXT("DEFAULT")));
    Row.Timeline[1].Action = ETMOPHistoricalVehicleAction::Spawn;
    Row.Timeline[1].DriverEntityId = NAME_None;
    TestEqual(TEXT("New life uses default"),
        TMOPVehicleRoute::Driver(Row, Copy), FName(TEXT("DEFAULT")));
    Row.KnownDriverEntityId = NAME_None;
    Row.Timeline[3].DriverEntityId = TEXT("FUTURE_DRIVER");
    TestTrue(TEXT("Never borrow a future driver"),
        TMOPVehicleRoute::Driver(Row, Copy).IsNone());
    FTMOPHistoricalVehicleTimelineEntry Unrelated;
    TestTrue(TEXT("Unrelated entry cannot borrow a timeline driver"),
        TMOPVehicleRoute::Driver(Row, Unrelated).IsNone());
    return true;
}
#endif
