#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Time/TMOPTimeTravelPolicy.h"
#include "Time/TMOPWorldPlaybackComponent.h"
#include "Engine/Engine.h"
#include "Engine/World.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPTimeGridTest, "TMOP.TimeTravel.FiveSecondGrid",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPTimeGridTest::RunTest(const FString&)
{
    for (int32 T=82800; T<=85500; T+=5)
        if (!TestEqual(TEXT("Every destination is stable"), TMOPTimeTravel::Snap(T,82800,85500), T)) return false;
    TestEqual(TEXT("No future pose before spawn"), TMOPTimeTravel::FloorKeyIndex(1, [](int) { return 10.0; },9.99), -1);
    TestEqual(TEXT("Birth is right-continuous"), TMOPTimeTravel::FloorKeyIndex(1, [](int) { return 10.0; },10.0), 0);
    TestFalse(TEXT("Seek never fires skipped audio"), TMOPTimeTravel::Crossed(10,20,15,true));
    return true;
}

// This integration test deliberately requires the user's real, baked map in PIE.
// An empty synthetic map cannot validate their routes, attachments or meshes.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPWorldRoundTripTest, "TMOP.TimeTravel.LoadedWorldRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPWorldRoundTripTest::RunTest(const FString&)
{
    if (GEngine) for (const FWorldContext& Context : GEngine->GetWorldContexts())
        if (Context.WorldType == EWorldType::PIE && Context.World())
            if (auto* Replay = UTMOPWorldPlaybackComponent::Find(Context.World()))
            {
                if (!Replay->IsReady()) { AddError(Replay->GetStatus()); return false; }
                const bool Passed = Replay->VerifyRepeatability();
                if (!Passed) AddError(Replay->GetStatus());
                return Passed;
            }
    AddError(TEXT("Start Play in the historical map with a complete .tmopreplay bake, then run this test. No historical world was tested."));
    return false;
}
#endif
