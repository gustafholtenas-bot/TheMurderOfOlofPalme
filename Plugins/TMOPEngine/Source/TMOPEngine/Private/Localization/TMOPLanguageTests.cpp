#include "Localization/TMOPLanguageSubsystem.h"
#include "Engine/GameInstance.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPLanguageSwitchTest, "TMOP.Localization.LanguageSwitch",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FTMOPLanguageSwitchTest::RunTest(const FString& Parameters)
{
    const FString OriginalLanguage = FInternationalization::Get().GetCurrentLanguage()->GetName();
    const FString OriginalLocale = FInternationalization::Get().GetCurrentLocale()->GetName();
    UGameInstance* Game = NewObject<UGameInstance>();
    auto* Language = NewObject<UTMOPLanguageSubsystem>(Game);
    Language->RegisterMenuTranslations();

    // Hold the same FText instance throughout: widgets must not need rebuilding.
    const FText Label = NSLOCTEXT("TMOP", "MainMenuSettings", "INSTÄLLNINGAR");
    const FText Untranslated = NSLOCTEXT("TMOPLanguageTest", "Fallback", "Svensk originaltext");
    TestTrue(TEXT("Switch to Swedish"), Language->SetLanguage(TEXT("sv"), false));
    TestEqual(TEXT("Swedish source"), Label.ToString(), FString(TEXT("INSTÄLLNINGAR")));
    TestTrue(TEXT("Switch to English"), Language->SetLanguage(TEXT("en"), false));
    TestEqual(TEXT("Existing text updates"), Label.ToString(), FString(TEXT("SETTINGS")));
    TestEqual(TEXT("Untranslated source remains readable"), Untranslated.ToString(), FString(TEXT("Svensk originaltext")));
    TestFalse(TEXT("Reject unavailable language"), Language->SetLanguage(TEXT("fr"), false));
    TestEqual(TEXT("Invalid selection leaves English active"), Language->GetLanguage(), FString(TEXT("en")));
    TestTrue(TEXT("Switch back"), Language->SetLanguage(TEXT("sv"), false));
    TestEqual(TEXT("Existing text returns to Swedish"), Label.ToString(), FString(TEXT("INSTÄLLNINGAR")));
    TestEqual(TEXT("Locale is not changed"), FInternationalization::Get().GetCurrentLocale()->GetName(), OriginalLocale);
    FInternationalization::Get().SetCurrentLanguage(OriginalLanguage);
    return true;
}
#endif
