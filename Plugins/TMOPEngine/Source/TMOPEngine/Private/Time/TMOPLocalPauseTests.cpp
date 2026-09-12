#include "Time/TMOPClockSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Engine/GameInstance.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPSharedPauseTest, "TMOP.LocalMultiplayer.PauseOwnership",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMOPSharedPauseTest::RunTest(const FString& Parameters)
{
    UGameInstance* GI = NewObject<UGameInstance>();
    auto* Clock = NewObject<UTMOPClockSubsystem>(GI);
    UObject* P1 = NewObject<UObject>(GI);
    UObject* P2 = NewObject<UObject>(GI);
    Clock->StartClock();
    TestTrue(TEXT("Running before menus"), Clock->IsClockRunning());
    Clock->RequestPause(P1, TEXT("PauseMenu"));
    Clock->RequestPause(P1, TEXT("PauseMenu"));
    Clock->RequestPause(P2, TEXT("WorldMap"));
    TestFalse(TEXT("Shared pause"), Clock->IsClockRunning());
    Clock->ReleasePause(P1, TEXT("PauseMenu"));
    Clock->StartClock();
    TestFalse(TEXT("P1 cannot release P2's lock, nor can StartClock bypass it"), Clock->IsClockRunning());
    Clock->ReleasePause(P2, TEXT("WorldMap"));
    TestTrue(TEXT("Idempotent P1 request needs only one release"), Clock->IsClockRunning());
    Clock->RequestPause(P1, TEXT("PauseMenu"));
    Clock->RequestPause(P1, TEXT("Newspaper"));
    Clock->ReleasePause(P1, TEXT("PauseMenu"));
    TestFalse(TEXT("A second reason on the same player still pauses"), Clock->IsClockRunning());
    Clock->ReleaseAllPauses(P1);
    TestTrue(TEXT("Removing player releases their reasons"), Clock->IsClockRunning());
    Clock->PauseClock();
    Clock->RequestPause(P1, TEXT("PauseMenu"));
    Clock->ReleaseAllPauses(P1);
    TestFalse(TEXT("Releasing a menu does not override manual clock stop"), Clock->IsClockRunning());
    Clock->StartClock();
    Clock->SetCurrentTime(Clock->GetLoopEndTime());
    Clock->StartClock();
    TestFalse(TEXT("End cannot be resumed accidentally"), Clock->IsClockRunning());
    Clock->RequestPause(P2, TEXT("PauseMenu"));
    Clock->RestartLoop();
    Clock->StartClock();
    TestFalse(TEXT("Clock restart retains other owners' locks"), Clock->IsClockRunning());
    Clock->ReleaseAllPauses(P2);
    TestTrue(TEXT("Restart clears only end lock"), Clock->IsClockRunning());
    return true;
}
#endif
