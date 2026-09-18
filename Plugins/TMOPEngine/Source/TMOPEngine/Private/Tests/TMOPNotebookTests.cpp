#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Observations/TMOPNotebookTypes.h"
#include "UI/TMOPMenuSaveGame.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPNotebookEligibilityTest,
    "TMOP.Notebook.EligibilityAndClassification",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMOPNotebookEligibilityTest::RunTest(const FString&)
{
    TestFalse(TEXT("Ordinary witness excluded"), TMOPNotebook::IsEligible(TEXT("WITNESS_1"), TEXT("WITNESS")));
    TestFalse(TEXT("Police excluded"), TMOPNotebook::IsEligible(TEXT("POLICE_1"), TEXT("POLICE")));
    TestFalse(TEXT("Palme excluded"), TMOPNotebook::IsEligible(TEXT("OLOF_PALME"), TEXT("OBSERVED_UNKNOWN")));
    TestTrue(TEXT("Observed person eligible"), TMOPNotebook::IsEligible(TEXT("UNKNOWN_MAN"), TEXT("OBSERVED_UNKNOWN")));
    TestTrue(TEXT("Observed ID eligible"), TMOPNotebook::IsEligible(TEXT("OBSERVED_MAN"), TEXT("OTHER")));
    TestTrue(TEXT("Suspect eligible"), TMOPNotebook::IsEligible(TEXT("MAN"), TEXT("SUSPECT")));
    TestTrue(TEXT("Shooter eligible"), TMOPNotebook::IsEligible(TEXT("THE_KILLER"), TEXT("OTHER")));
    using C = ETMOPNotebookCategory;
    TestEqual(TEXT("Shooter first"), TMOPNotebook::ResolveCategory(C::Automatic, true, true, true), C::Shooter);
    TestEqual(TEXT("Fleeing before radio"), TMOPNotebook::ResolveCategory(C::Automatic, false, true, true), C::HighlySuspicious);
    TestEqual(TEXT("Radio category"), TMOPNotebook::ResolveCategory(C::Automatic, false, false, true), C::WalkieTalkie);
    TestEqual(TEXT("Other fallback"), TMOPNotebook::ResolveCategory(C::Automatic, false, false, false), C::OtherSuspicious);
    TestEqual(TEXT("Authored low category"), TMOPNotebook::ResolveCategory(C::LessSuspicious, false, true, true), C::LessSuspicious);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPNotebookSaveTest,
    "TMOP.Notebook.DeduplicationAndSaveRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMOPNotebookSaveTest::RunTest(const FString&)
{
    auto* Save = Cast<UTMOPMenuSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UTMOPMenuSaveGame::StaticClass()));
    Save->SaveFormatVersion = 3;
    Save->LocalPlayers.SetNum(2);
    for (int32 Index = 1; Index <= 5; ++Index)
    {
        FTMOPNotebookObservation Entry;
        Entry.EntityId = FName(*FString::Printf(TEXT("OBSERVED_%d"), Index));
        Entry.Category = static_cast<ETMOPNotebookCategory>(Index);
        Entry.DisplayName = FText::FromString(TEXT("Okänd man med röd täckjacka"));
        Entry.Summary = FText::FromString(TEXT("Observerad vid Grand."));
        Entry.Signalement = FText::FromString(TEXT("Röd täckjacka, mörka byxor."));
        Entry.ObserverNames.Add(FText::FromString(TEXT("Vittne A.")));
        Entry.ModelPreviewPng = {137, 80, 78, 71}; // Opaque test payload; checks serialization, not decoding.
        FTMOPNotebookEvidenceImage Sketch;
        Sketch.ImagePath = FSoftObjectPath(TEXT("/Game/Tests/Sketch.Sketch"));
        Sketch.Caption = FText::FromString(TEXT("Fantombild"));
        Sketch.Source = FText::FromString(TEXT("Källa A"));
        Entry.EvidenceImages.Add(Sketch);
        Entry.PresentationVersion = 1;
        Entry.DiscoveredSecond = 23 * 3600 + Index;
        auto& Entries = Save->LocalPlayers[0].NotebookObservations;
        TestTrue(TEXT("First inspection collected"), TMOPNotebook::AddUnique(Entries, Entry));
        TestFalse(TEXT("Repeat inspection ignored"), TMOPNotebook::AddUnique(Entries, Entry));
    }
    TestFalse(TEXT("No pending suspect ignored"),
        TMOPNotebook::AddUnique(Save->LocalPlayers[0].NotebookObservations, FTMOPNotebookObservation()));
    Save->NotebookObservations = Save->LocalPlayers[0].NotebookObservations;
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Save to memory"), UGameplayStatics::SaveGameToMemory(Save, Bytes))) return false;
    auto* Loaded = Cast<UTMOPMenuSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Loaded save"), Loaded)) return false;
    TestEqual(TEXT("All five entries restored"), Loaded->LocalPlayers[0].NotebookObservations.Num(), 5);
    TestEqual(TEXT("Second player stays empty"), Loaded->LocalPlayers[1].NotebookObservations.Num(), 0);
    TestEqual(TEXT("Legacy field restored"), Loaded->NotebookObservations.Num(), 5);
    for (int32 Index = 0; Index < 5; ++Index)
    {
        const auto& Before = Save->LocalPlayers[0].NotebookObservations[Index];
        const auto& After = Loaded->LocalPlayers[0].NotebookObservations[Index];
        TestEqual(TEXT("Stable identity"), After.EntityId, Before.EntityId);
        TestEqual(TEXT("Category"), After.Category, Before.Category);
        TestEqual(TEXT("Name"), After.DisplayName.ToString(), Before.DisplayName.ToString());
        TestEqual(TEXT("Summary"), After.Summary.ToString(), Before.Summary.ToString());
        TestEqual(TEXT("Discovery time"), After.DiscoveredSecond, Before.DiscoveredSecond);
        TestEqual(TEXT("Signalement saved"), After.Signalement.ToString(), Before.Signalement.ToString());
        TestEqual(TEXT("Witness names saved"), After.ObserverNames[0].ToString(), Before.ObserverNames[0].ToString());
        TestTrue(TEXT("Model snapshot bytes saved"), After.ModelPreviewPng == Before.ModelPreviewPng);
        TestEqual(TEXT("Sketch asset path saved"), After.EvidenceImages[0].ImagePath.ToString(), Before.EvidenceImages[0].ImagePath.ToString());
        TestEqual(TEXT("Sketch source saved"), After.EvidenceImages[0].Source.ToString(), Before.EvidenceImages[0].Source.ToString());
    }
    return true;
}
#endif
