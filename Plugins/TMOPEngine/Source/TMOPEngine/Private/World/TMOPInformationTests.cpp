#include "World/TMOPInformationComponent.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPInformationContentTest,"TMOP.Information.Content",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPInformationContentTest::RunTest(const FString&)
{
    auto* Information = NewObject<UTMOPInformationComponent>();
    TestFalse(TEXT("An empty point does not offer an empty reading window"), Information->HasReadableContent());
    Information->Title = FText::FromString(TEXT("Testplats"));
    Information->Description = FText::FromString(TEXT("Första stycket.\n\nAndra stycket."));
    TestTrue(TEXT("Freestanding content needs no address table or clock"), Information->HasReadableContent());
    const UTMOPInspectableComponent* Shared = Information;
    TestEqual(TEXT("Shared reader receives title"), Shared->GetInspectionTitle().ToString(), Information->Title.ToString());
    TestEqual(TEXT("Paragraphs survive"), Shared->GetInspectionText().ToString(), Information->Description.ToString());
    TestTrue(TEXT("Missing optional source remains empty"), Shared->GetInspectionSource().IsEmpty());
    Information->SourceReference = FText::FromString(TEXT("Testkälla, sid. 2"));
    TestEqual(TEXT("Source preserved"), Shared->GetInspectionSource().ToString(), Information->SourceReference.ToString());
    Information->CategoryLabel = FText::FromString(TEXT("Händelse"));
    TestEqual(TEXT("Category preserved"), Shared->GetInspectionCategory().ToString(), FString(TEXT("Händelse")));
    Information->bInteractionEnabled = false;
    TestFalse(TEXT("Disabled point cannot be read"), Shared->HasReadableContent());
    Information->bInteractionEnabled = true;
    Information->Description = FText::FromString(TEXT(" \n\t "));
    TestFalse(TEXT("Whitespace is not content"), Shared->HasReadableContent());
    Information->Description = FText::FromString(TEXT("Brödtext"));
    Information->bUseTimeWindow = true;
    TestFalse(TEXT("A timed point without a game clock is unavailable"), Shared->HasReadableContent());
    Information->VisibleFrom = FTMOPTime(23, 10, 0);
    Information->VisibleUntil = FTMOPTime(23, 20, 0);
    TestFalse(TEXT("Before start"), Information->IsAvailableAtTime(FTMOPTime(23, 9, 59)));
    TestTrue(TEXT("Start is included"), Information->IsAvailableAtTime(FTMOPTime(23, 10, 0)));
    TestFalse(TEXT("End is excluded"), Information->IsAvailableAtTime(FTMOPTime(23, 20, 0)));
    TestTrue(TEXT("Seek back into interval"), Information->IsAvailableAtTime(FTMOPTime(23, 15, 0)));
    Information->VisibleFrom.Minute = 99;
    TestFalse(TEXT("Malformed clock input is not normalized silently"), Information->IsAvailableAtTime(FTMOPTime(23, 15, 0)));
    Information->bUseTimeWindow = false;
    TestTrue(TEXT("Untimed point ignores time configuration"), Information->HasReadableContent());
    return true;
}
#endif
