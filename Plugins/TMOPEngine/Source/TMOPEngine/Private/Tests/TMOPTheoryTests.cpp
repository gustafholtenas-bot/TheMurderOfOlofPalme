#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Research/TMOPTheoryTypes.h"
#include "UI/TMOPMenuSaveGame.h"
#include "Kismet/GameplayStatics.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPTheoryTemplatesTest, "TMOP.TheoryBuilder.TemplatesAndDiscovery",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPTheoryTemplatesTest::RunTest(const FString&)
{
    const int32 People[] = {1, 5, 14, 19};
    const int32 Vehicles[] = {0, 2, 5, 7};
    FTMOPNotebookObservation Shooter;
    Shooter.EntityId = TEXT("THE_KILLER"); Shooter.Category = ETMOPNotebookCategory::Shooter;
    Shooter.DisplayName = FText::FromString(TEXT("Mannen med revolvern"));
    for (int32 I = 0; I < 4; ++I)
    {
        auto T = TMOPTheory::CreateTemplate(I);
        TestEqual(TEXT("Template identity"), T.TemplateIndex, I);
        TestEqual(TEXT("Person slots match supplied design"), T.Nodes.FilterByPredicate([](const auto& N) {
            return N.Kind == ETMOPTheoryNodeKind::Person; }).Num(), People[I]);
        TestEqual(TEXT("Vehicle slots match supplied design"), T.Nodes.FilterByPredicate([](const auto& N) {
            return N.Kind == ETMOPTheoryNodeKind::Vehicle; }).Num(), Vehicles[I]);
        TestEqual(TEXT("Exactly one shooter"), T.Nodes.FilterByPredicate([](const auto& N) { return N.bShooter; }).Num(), 1);
        TestTrue(TEXT("Player draws their own relationships"), T.Links.IsEmpty());
        TMOPTheory::SynchronizeShooter(T, {});
        auto* Root = T.Nodes.FindByPredicate([](const auto& N) { return N.bShooter; });
        TestTrue(TEXT("Undiscovered shooter is empty"), Root->EntityId.IsNone());
        TMOPTheory::SynchronizeShooter(T, {Shooter});
        TestEqual(TEXT("Same discovered shooter in every scenario"), Root->EntityId, Shooter.EntityId);
        TestFalse(TEXT("Shooter cannot be removed"), TMOPTheory::RemoveNode(T, Root->Id));
        TestFalse(TEXT("Shooter cannot be replaced"), TMOPTheory::AssignObservation(T, Root->Id, TEXT("OTHER"), {Shooter}));
        TMOPTheory::SynchronizeShooter(T, {});
        TestTrue(TEXT("Loading a save before discovery clears shooter"), Root->EntityId.IsNone());
    }
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPTheoryEditingTest, "TMOP.TheoryBuilder.CollectionAndGraphEdits",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPTheoryEditingTest::RunTest(const FString&)
{
    FTMOPNotebookObservation Person, Car;
    Person.EntityId = TEXT("OBSERVED_SHARED_ID"); Person.DisplayName = FText::FromString(TEXT("Okänd man"));
    Car.EntityId = Person.EntityId; Car.Kind = ETMOPNotebookEntityKind::Vehicle;
    Car.DisplayName = FText::FromString(TEXT("Rosa bil"));
    TArray<FTMOPNotebookObservation> Collected;
    TestTrue(TEXT("Collect person"), TMOPNotebook::AddUnique(Collected, Person));
    TestTrue(TEXT("Same ID in different type can be collected"), TMOPNotebook::AddUnique(Collected, Car));
    TestFalse(TEXT("Same vehicle never duplicated"), TMOPNotebook::AddUnique(Collected, Car));
    TestFalse(TEXT("Normal cars not collected"), TMOPNotebook::IsVehicleEligible(TEXT("CAR_1"), TEXT("CIVILIAN")));
    TestTrue(TEXT("Green observed car qualifies"), TMOPNotebook::IsVehicleEligible(TEXT("CAR_1"), TEXT("OBSERVED_CAR")));
    auto T = TMOPTheory::CreateTemplate(0);
    auto A = TMOPTheory::AddNode(T, ETMOPTheoryNodeKind::Person, FVector2D(20, 20), TEXT("Medhjälpare"));
    auto B = TMOPTheory::AddNode(T, ETMOPTheoryNodeKind::Vehicle, FVector2D(20, 200), TEXT("Flyktbil"));
    auto C = TMOPTheory::AddNode(T, ETMOPTheoryNodeKind::Person, FVector2D(200, 20), TEXT("Ytterligare person"));
    TestFalse(TEXT("Cannot use undiscovered observation"), TMOPTheory::AssignObservation(T, A, Person.EntityId, {}));
    TestFalse(TEXT("Cannot assign car to person slot"), TMOPTheory::AssignObservation(T, A, Car.EntityId, {Car}));
    TestTrue(TEXT("Assign collected person"), TMOPTheory::AssignObservation(T, A, Person.EntityId, Collected));
    TestTrue(TEXT("Assign collected car"), TMOPTheory::AssignObservation(T, B, Car.EntityId, Collected));
    TestFalse(TEXT("Person cannot occupy two slots in one tree"), TMOPTheory::AssignObservation(T, C, Person.EntityId, Collected));
    TestTrue(TEXT("Draw link"), TMOPTheory::AddLink(T, A, B));
    TestFalse(TEXT("No reverse duplicate"), TMOPTheory::AddLink(T, B, A));
    TestFalse(TEXT("No self link"), TMOPTheory::AddLink(T, A, A));
    TestFalse(TEXT("No dangling link"), TMOPTheory::AddLink(T, A, FGuid::NewGuid()));
    const auto BeforeRemoval = T; // The editor stores these value snapshots for undo.
    TestTrue(TEXT("Remove person"), TMOPTheory::RemoveNode(T, A));
    TestTrue(TEXT("Incident links removed"), T.Links.IsEmpty());
    T = BeforeRemoval;
    TestEqual(TEXT("Snapshot restores links"), T.Links.Num(), 1);
    TestEqual(TEXT("Snapshot restores assignment"), T.Nodes.FindByPredicate([A](const auto& N) { return N.Id == A; })->EntityId, Person.EntityId);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPTheorySaveTest, "TMOP.TheoryBuilder.SaveRoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPTheorySaveTest::RunTest(const FString&)
{
    auto* Save = Cast<UTMOPMenuSaveGame>(UGameplayStatics::CreateSaveGameObject(UTMOPMenuSaveGame::StaticClass()));
    Save->SaveFormatVersion = 3; Save->LocalPlayers.SetNum(2);
    auto T = TMOPTheory::CreateTemplate(3);
    T.Title = TEXT("Min teori åäö"); T.Zoom = 0.73f; T.Pan = FVector2D(-80, 250);
    T.Nodes[0].Notes = TEXT("En egen anteckning\nmed radbrytning."); T.Nodes[0].Position = FVector2D(-700, 22);
    TMOPTheory::AddLink(T, T.Nodes[0].Id, T.Nodes[1].Id); T.Links[0].Label = TEXT("Möjligt samband");
    FTMOPNotebookObservation Car; Car.EntityId = TEXT("OBSERVED_CAR"); Car.Kind = ETMOPNotebookEntityKind::Vehicle;
    Save->LocalPlayers[0].NotebookObservations.Add(Car);
    Save->LocalPlayers[0].TheoryTrees.Add(T); Save->LocalPlayers[0].ActiveTheoryTreeId = T.Id;
    Save->TheoryTrees.Add(T); Save->ActiveTheoryTreeId = T.Id;
    TArray<uint8> Bytes;
    if (!TestTrue(TEXT("Serialize"), UGameplayStatics::SaveGameToMemory(Save, Bytes))) return false;
    auto* Loaded = Cast<UTMOPMenuSaveGame>(UGameplayStatics::LoadGameFromMemory(Bytes));
    if (!TestNotNull(TEXT("Deserialize"), Loaded)) return false;
    TestEqual(TEXT("Other player isolated"), Loaded->LocalPlayers[1].TheoryTrees.Num(), 0);
    if (!TestEqual(TEXT("Player tree restored"), Loaded->LocalPlayers[0].TheoryTrees.Num(), 1)) return false;
    const auto& After = Loaded->LocalPlayers[0].TheoryTrees[0];
    TestEqual(TEXT("Tree ID"), After.Id, T.Id);
    TestEqual(TEXT("Active tree"), Loaded->LocalPlayers[0].ActiveTheoryTreeId, T.Id);
    TestEqual(TEXT("Title"), After.Title, T.Title);
    TestEqual(TEXT("Positions"), After.Nodes[0].Position, T.Nodes[0].Position);
    TestEqual(TEXT("Notes"), After.Nodes[0].Notes, T.Nodes[0].Notes);
    TestEqual(TEXT("Roles"), After.Nodes[1].Role, T.Nodes[1].Role);
    TestEqual(TEXT("Node dimensions"), After.Nodes[1].Size, T.Nodes[1].Size);
    TestEqual(TEXT("Link endpoint"), After.Links[0].From, T.Links[0].From);
    TestEqual(TEXT("Link label"), After.Links[0].Label, T.Links[0].Label);
    TestEqual(TEXT("Zoom"), After.Zoom, T.Zoom); TestEqual(TEXT("Pan"), After.Pan, T.Pan);
    TestEqual(TEXT("Vehicle type preserved"), Loaded->LocalPlayers[0].NotebookObservations[0].Kind, ETMOPNotebookEntityKind::Vehicle);
    TestEqual(TEXT("Legacy single-player fields"), Loaded->TheoryTrees[0].Id, T.Id);
    return true;
}
#endif
