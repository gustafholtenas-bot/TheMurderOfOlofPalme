#include "StockMarket/TMOPStockMarketData.h"
#include "Misc/AutomationTest.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPMarketChangeTest, "TMOP.StockMarket.Change",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPMarketChangeTest::RunTest(const FString&)
{
    FTMOPMarketEntry E;
    double Points = 0, Percent = 0;
    E.Before = 1709.06; E.After = 1696.67;
    TestTrue(TEXT("Verified Dow close can be compared"), E.Change(Points, Percent));
    TestTrue(TEXT("Dow points"), FMath::Abs(Points + 12.39) < 1.e-8);
    TestTrue(TEXT("Dow percentage uses Friday as denominator"), FMath::Abs(Percent + .7249599195) < 1.e-8);
    E.After.Reset();
    TestFalse(TEXT("Missing is not a zero return"), E.Change(Points, Percent));
    E.Before = 100.; E.After = 0.;
    TestTrue(TEXT("Actual zero endpoint remains a number"), E.Change(Points, Percent));
    TestEqual(TEXT("Zero endpoint is minus 100 percent"), Percent, -100.);
    E.Before = 0.; E.After = 100.;
    TestFalse(TEXT("Zero base cannot be divided"), E.Change(Points, Percent));
    E.Before = std::numeric_limits<double>::quiet_NaN();
    TestFalse(TEXT("Nonfinite input cannot leak into UI"), E.Change(Points, Percent));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPMarketDataTest, "TMOP.StockMarket.Data",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPMarketDataTest::RunTest(const FString&)
{
    FTMOPStockMarketData Data;
    if (!TestTrue(TEXT("Staged JSON loads"), Data.Load())) { AddError(Data.Error); return false; }
    TestEqual(TEXT("Friday date"), Data.BeforeDate, FString(TEXT("1986-02-28")));
    TestEqual(TEXT("Monday date"), Data.AfterDate, FString(TEXT("1986-03-03")));
    const auto* Montreal = Data.Entries.FindByPredicate([](const auto& E) { return E->Id == TEXT("ca-montreal-portfolio"); });
    if (TestNotNull(TEXT("Montreal is retained"), Montreal))
    {
        double Points = 0, Percent = 0;
        TestFalse(TEXT("Disputed Montreal scale is not compared"), (*Montreal)->Change(Points, Percent));
        TestEqual(TEXT("Source conflict is visible"), (*Montreal)->Status, FString(TEXT("source_conflict")));
    }
    const FString Valid = TEXT(R"json({"schema":1,"before_date":"1986-02-28","after_date":"1986-03-03","method":{"sv":"Metod"},"sources":[{"id":"ft","title":"Financial Times","published":"1986-03-05","url":"https://archive.org/"}],"entries":[{"id":"test","region":"europe","kind":"market","status":"daily","decimals":2,"market":{"sv":"Sverige"},"name":{"sv":"Test"},"notes":{"sv":""},"before":100,"after":101,"citations":[{"source":"ft","pages":"46"}]}]})json");
    TestTrue(TEXT("Minimal valid source loads"), Data.Parse(Valid));
    TestFalse(TEXT("Unknown citation rejected"), Data.Parse(Valid.Replace(TEXT("\"source\":\"ft\""), TEXT("\"source\":\"unknown\""))));
    TestTrue(TEXT("Partial data is cleared on failure"), Data.Entries.IsEmpty());
    TestFalse(TEXT("String is not accepted as numeric quote"), Data.Parse(Valid.Replace(TEXT("\"before\":100"), TEXT("\"before\":\"100\""))));
    TestFalse(TEXT("Missing quote needs explicit status"), Data.Parse(Valid.Replace(TEXT("\"after\":101"), TEXT("\"after\":null"))));
    TestFalse(TEXT("Non-HTTPS source cannot be launched"), Data.Parse(Valid.Replace(TEXT("https://archive.org/"), TEXT("file:///test"))));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPMarketLanguageTest, "TMOP.StockMarket.LanguageFallback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPMarketLanguageTest::RunTest(const FString&)
{
    const FString Previous = FInternationalization::Get().GetCurrentLanguage()->GetName();
    FTMOPMarketText Text;
    Text.Values = MakeShared<FJsonObject>();
    Text.Values->SetStringField(TEXT("sv"), TEXT("Original"));
    Text.Values->SetStringField(TEXT("en"), TEXT("Translation"));
    Text.Values->SetStringField(TEXT("en_source"), TEXT("Original"));
    FInternationalization::Get().SetCurrentLanguage(TEXT("en"));
    TestEqual(TEXT("Current inline translation"), Text.Resolve(TEXT("test-only"), TEXT("notes")).ToString(), FString(TEXT("Translation")));
    Text.Values->SetStringField(TEXT("sv"), TEXT("Uppdaterad"));
    TestEqual(TEXT("Stale English falls back to Swedish"), Text.Resolve(TEXT("test-only"), TEXT("notes")).ToString(), FString(TEXT("Uppdaterad")));
    FInternationalization::Get().SetCurrentLanguage(Previous);
    return true;
}
#endif
