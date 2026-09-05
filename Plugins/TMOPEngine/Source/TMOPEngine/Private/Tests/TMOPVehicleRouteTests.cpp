#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Vehicles/TMOPVehicleRoutePlan.h"
#include "Vehicles/TMOPVehicleTimeline.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPVehicleTimeTest, "TMOP.VehicleTimeline.StopAndDeparture",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPVehicleTimeTest::RunTest(const FString&)
{
    FTMOPHistoricalVehicleRow Row; Row.Timeline.SetNum(2);
    auto& Stop = Row.Timeline[0]; Stop.Action = ETMOPHistoricalVehicleAction::Stop;
    Stop.Time = FTMOPTime(23,21,25); Stop.bUseStopDuration = true; Stop.StopDurationSeconds = 10;
    auto& Drive = Row.Timeline[1]; Drive.Action = ETMOPHistoricalVehicleAction::BeginDriving;
    Drive.bTimeIsArrival = true; Drive.Time = FTMOPTime(23,22,0);
    auto Event = [](FName Id, int32& Time) { Time = 23*3600+21*60+30; return Id == FName(TEXT("shot")); };
    int32 Departure = 0, Arrival = 0;
    TestTrue(TEXT("Valid window"), TMOPVehicleTimeline::ResolveWindow(Row,1,Event,Departure,Arrival));
    TestEqual(TEXT("Stop duration inherited"), Departure,23*3600+21*60+35);
    TestEqual(TEXT("Arrival unchanged"),Arrival,23*3600+22*60);
    Drive.TimingMode = ETMOPEventTimingMode::Relative; Drive.SharedEventId = TEXT("shot"); Drive.EventOffsetSeconds = -5;
    Drive.bUseExplicitDepartureTime = true; Drive.DepartureTime = FTMOPTime(23,21,0);
    TestTrue(TEXT("Shared arrival and explicit departure"),TMOPVehicleTimeline::ResolveWindow(Row,1,Event,Departure,Arrival));
    TestEqual(TEXT("Five seconds before shot"),Arrival,23*3600+21*60+25);
    Drive.DepartureTime = FTMOPTime(23,21,26);
    TestFalse(TEXT("Negative driving duration rejected"),TMOPVehicleTimeline::ResolveWindow(Row,1,Event,Departure,Arrival));
    Drive.bUseExplicitDepartureTime = false; Stop.Action = ETMOPHistoricalVehicleAction::OffscreenTransfer;
    Stop.OffscreenTransferDurationSeconds = 120;
    TestTrue(TEXT("Transfer completion resolves"),TMOPVehicleTimeline::ResolveDeparture(Row,1,Event,Departure));
    TestEqual(TEXT("Transfer lasts exactly 120 seconds"),Departure,23*3600+23*60+25);
    Stop.Action = ETMOPHistoricalVehicleAction::Stop; Drive.bTimeIsArrival = false;
    Drive.TimingMode = ETMOPEventTimingMode::RelativeToPreviousEntry; Drive.EventOffsetSeconds = 0;
    TestTrue(TEXT("Departure-only row inherits stop"),TMOPVehicleTimeline::ResolveDeparture(Row,1,Event,Departure));
    TestEqual(TEXT("Stop duration counted once"),Departure,23*3600+21*60+35);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPVehicleManeuverTest, "TMOP.VehicleRoute.Maneuver",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPVehicleManeuverTest::RunTest(const FString&)
{
    for (float Side : {-1.0f,1.0f}) for (bool bReverse : {false,true})
    {
        TArray<FTransform> Anchors = {FTransform(FRotator::ZeroRotator,FVector::ZeroVector),
            FTransform(FRotator(0,180,0),FVector(0,600*Side,0))};
        FTMOPVehicleRoutePlan Plan;
        TMOPVehicleRoute::BuildManeuver(Anchors,0.5f,bReverse,ETMOPVehicleManeuverTurn::Automatic,0,Plan);
        TestTrue(TEXT("Exact start"),Plan.Sample(0).Equals(Anchors[0],0.01f));
        TestTrue(TEXT("Exact end and yaw"),Plan.Sample(Plan.LengthCm).Equals(Anchors[1],0.01f));
        for (double Fraction : {0.2,0.4,0.6,0.8})
        {
            const double Distance = Plan.LengthCm*Fraction;
            const FVector Velocity = (Plan.Sample(Distance+1).GetLocation()-Plan.Sample(Distance-1).GetLocation()).GetSafeNormal();
            const FVector Facing = Plan.Sample(Distance).GetRotation().GetForwardVector()*(bReverse?-1.0:1.0);
            TestTrue(TEXT("Car faces along travel tangent"),FVector::DotProduct(Velocity,Facing)>0.98);
        }
        TestEqual(TEXT("Preview and playback finish at full length"),
            TMOPVehicleRoute::DistanceAtTime(Plan,1.0),Plan.LengthCm);
        FTMOPHistoricalVehicleRow Row; Row.VehicleId=TEXT("TEST_CAR"); Row.Timeline.SetNum(1);
        Row.Timeline[0].EntryId=TEXT("TEST_001");
        const FString Original=TMOPVehicleRoute::Fingerprint(Row,Plan,100,110);
        TestNotEqual(TEXT("Timing edits invalidate report"),Original,TMOPVehicleRoute::Fingerprint(Row,Plan,100,111));
        Row.Timeline[0].DriverEntityId=TEXT("OTHER_DRIVER");
        TestNotEqual(TEXT("Driver edits invalidate report"),Original,TMOPVehicleRoute::Fingerprint(Row,Plan,100,110));
        TestEqual(TEXT("Unique IDs skip existing names"),TMOPVehicleRoute::UniqueEntryId(Row,TEXT("TEST")),FName(TEXT("TEST_002")));
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPVehicleTrimTest, "TMOP.VehicleRoute.TrimAndDisconnect",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPVehicleTrimTest::RunTest(const FString&)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game,false);
    if (!TestNotNull(TEXT("Test world"),World)) return false;
    World->InitializeNewWorld(UWorld::InitializationValues().AllowAudioPlayback(false)
        .CreatePhysicsScene(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false));
    auto MakeLane = [World](FName Id,float Start,float End)
    {
        AActor* OwnerActor=World->SpawnActor<AActor>();
        auto* Lane=NewObject<UTMOPTrafficLaneComponent>(OwnerActor);
        OwnerActor->SetRootComponent(Lane); OwnerActor->AddInstanceComponent(Lane);
        Lane->LaneId=Id; Lane->RegisterComponent(); Lane->ClearSplinePoints(false);
        Lane->AddSplinePoint(FVector(Start,0,0),ESplineCoordinateSpace::World,false);
        Lane->AddSplinePoint(FVector(End,0,0),ESplineCoordinateSpace::World,false);
        Lane->SetSplinePointType(0,ESplinePointType::Linear,false);
        Lane->SetSplinePointType(1,ESplinePointType::Linear,false); Lane->UpdateSpline();
        return Lane;
    };
    auto* First=MakeLane(TEXT("LANE_A"),0,1000); MakeLane(TEXT("LANE_B"),1000,2000);
    FTMOPLaneConnection Connection; Connection.TargetLaneId=TEXT("LANE_B"); First->NextLanes.Add(Connection);
    FTMOPHistoricalVehicleRow Row; Row.Timeline.SetNum(3);
    Row.Timeline[0].WorldTransform=FTransform(FVector(700,0,0));
    auto& Drive=Row.Timeline[1]; Drive.Action=ETMOPHistoricalVehicleAction::BeginDriving;
    Drive.OrderedLaneIds={FName(TEXT("LANE_A")),FName(TEXT("LANE_B"))};
    Row.Timeline[2].Action=ETMOPHistoricalVehicleAction::Stop;
    Row.Timeline[2].WorldTransform=FTransform(FVector(1300,0,0));
    FTMOPVehicleRoutePlan Plan; FString Failure;
    TestTrue(TEXT("Trimmed route builds"),TMOPVehicleRoute::Build(World,Row,1,Plan,Failure));
    TestTrue(TEXT("Only 600 cm is travelled, not 2000"),FMath::IsNearlyEqual(Plan.LengthCm,600.0,0.01));
    TestEqual(TEXT("First lane starts at 700"),Plan.StartDistanceCm,700.0f);
    TestEqual(TEXT("Last lane ends at 300"),Plan.EndDistanceCm,300.0f);
    First->NextLanes.Reset();
    TestFalse(TEXT("Disconnected manual route rejected"),TMOPVehicleRoute::Build(World,Row,1,Plan,Failure));
    Drive.VehicleRouteMode=ETMOPVehicleRouteMode::AutomaticToAnchor;
    TestFalse(TEXT("Disconnected automatic route rejected"),TMOPVehicleRoute::Build(World,Row,1,Plan,Failure));
    First->NextLanes.Add(Connection);
    TestTrue(TEXT("Automatic route uses same trimmed distance"),TMOPVehicleRoute::Build(World,Row,1,Plan,Failure));
    TestTrue(TEXT("Automatic length matches manual"),FMath::IsNearlyEqual(Plan.LengthCm,600.0,0.01));
    Drive.VehicleRouteMode=ETMOPVehicleRouteMode::ManualLaneRoute; Drive.OrderedLaneIds={FName(TEXT("LANE_A"))};
    Row.Timeline[2].WorldTransform=FTransform(FVector(100,0,0));
    TestFalse(TEXT("Behind-start target requires an actual turn"),TMOPVehicleRoute::Build(World,Row,1,Plan,Failure));
    World->DestroyWorld(false);
    return true;
}
#endif
