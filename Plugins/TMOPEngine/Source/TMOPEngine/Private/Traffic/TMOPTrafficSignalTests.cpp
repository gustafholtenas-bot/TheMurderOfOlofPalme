#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/World.h"
#include "Traffic/TMOPTrafficSignalController.h"
#include "Traffic/TMOPPedestrianCrossingComponent.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPSignalProgramTest, "TMOP.Traffic.SignalProgramAndReplay",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPSignalProgramTest::RunTest(const FString& Parameters)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    if (!World) return false;
    auto* C = World->SpawnActor<ATMOPTrafficSignalController>();
    if (!C) { World->DestroyWorld(false); return false; }
    C->IntersectionId = TEXT("TEST");
    FTMOPTrafficSignalGroup A; A.GroupId = TEXT("A"); C->Groups.Add(A);
    FTMOPTrafficSignalGroup B; B.GroupId = TEXT("B"); B.bPedestrian = true; C->Groups.Add(B);
    FTMOPTrafficSignalConflict Conflict; Conflict.GroupA = A.GroupId; Conflict.GroupB = B.GroupId; C->Conflicts.Add(Conflict);
    FTMOPTrafficSignalStage First; First.GreenGroups.Add(A.GroupId); C->Stages.Add(First);
    FTMOPTrafficSignalStage Second; Second.GreenGroups.Add(B.GroupId); C->Stages.Add(Second);
    C->BuildProtectedProgram();
    TArray<FString> Errors;
    TestTrue(TEXT("Protected two-stage program is valid"), C->ValidateController(Errors));
    TestEqual(TEXT("Builder creates transitions and clearance"), C->Phases.Num(), 8);
    C->EvaluateAtTime(82822.0);
    bool Found = false;
    TestTrue(TEXT("1986 uses simultaneous green/amber"),
        C->GetGroupState(A.GroupId, Found) == ETMOPTrafficSignalState::GreenYellow);
    const auto Saved = C->CapturePlaybackSignals();
    const int32 SavedPhase = C->CurrentPhaseIndex;
    const double SavedEnd = C->PhaseEndSecond;
    C->EvaluateAtTime(82900);
    C->RestorePlaybackPhase(SavedPhase, SavedEnd);
    C->RestorePlaybackSignals(Saved);
    TestTrue(TEXT("Restored color"), C->GetGroupState(A.GroupId, Found) == ETMOPTrafficSignalState::GreenYellow);
    TestEqual(TEXT("Restored deadline"), C->PhaseEndSecond, SavedEnd);
    FTMOPSignalGroupState Bad; Bad.SignalGroupId = B.GroupId; Bad.State = ETMOPTrafficSignalState::Green;
    C->Phases[1].GroupStates.RemoveAll([&](const auto& S){ return S.SignalGroupId == B.GroupId; });
    C->Phases[1].GroupStates.Add(Bad);
    TestFalse(TEXT("Conflicting greens rejected"), C->ValidateController(Errors));
    auto* Crossing = NewObject<UTMOPPedestrianCrossingComponent>(C);
    Crossing->SetBoxExtent(FVector(500,150,200));
    TestTrue(TEXT("Approach crosses curb"), Crossing->CrossesEntrance(FVector(-600,0,0), FVector(-400,0,0)));
    TestFalse(TEXT("Parallel sidewalk walk"), Crossing->CrossesEntrance(FVector(-600,-200,0), FVector(-600,200,0)));
    TestFalse(TEXT("Already inside may clear"), Crossing->CrossesEntrance(FVector(0,0,0), FVector(600,0,0)));
    TestFalse(TEXT("Other floor does not block"), Crossing->CrossesEntrance(FVector(-600,0,1000), FVector(-400,0,1000)));
    World->DestroyWorld(false);
    return true;
}
#endif
