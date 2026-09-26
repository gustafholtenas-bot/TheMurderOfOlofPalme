#include "WorldAtlas/Flights/TMOPFlightData.h"
#include "Misc/AutomationTest.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include <limits>

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPFlightClockTest, "TMOP.Flights.ClockAndBoundaries",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPFlightClockTest::RunTest(const FString&)
{
    FTMOPFlightLeg Leg; Leg.Departure = -3600; Leg.Arrival = 3600;
    TestTrue(TEXT("Already airborne at start"), Leg.Airborne(0));
    TestTrue(TEXT("Departure inclusive"), Leg.Airborne(-3600));
    TestFalse(TEXT("Arrival exclusive"), Leg.Airborne(3600));
    TestEqual(TEXT("Halfway progress"), Leg.Progress(0), .5);
    TestEqual(TEXT("Before departure is clamped"), Leg.Progress(-7200), 0.);
    TestEqual(TEXT("After arrival is clamped"), Leg.Progress(7200), 1.);
    auto Data = MakeShared<FTMOPFlightData>();
    Data->StartUTC = FDateTime(1986,2,27,22,21,30);
    FTMOPFlightState A, B; A.Data = Data; B.Data = Data;
    A.bPlaying = true;
    A.Advance(2);
    TestEqual(TEXT("Disabled layer cannot advance"), A.Seconds, 0.);
    A.bEnabled = true; A.Speed = 1800;
    A.Advance(2);
    TestEqual(TEXT("Speed is simulated seconds per real second"), A.Seconds, 3600.);
    TestEqual(TEXT("Other player retains independent clock"), B.Seconds, 0.);
    A.SetTime(86400);
    TestTrue(TEXT("Anchor displays Swedish local time"), A.Clock().ToString().Contains(TEXT("1986-02-28 23:21:30 CET")));
    A.Advance(-1);
    A.SetTime(std::numeric_limits<double>::quiet_NaN());
    TestEqual(TEXT("Invalid time inputs ignored"), A.Seconds, 86400.);
    A.SetTime(Data->Duration - 1); A.Advance(2);
    TestEqual(TEXT("Playback ends at right boundary"), A.Seconds, 172800.);
    TestFalse(TEXT("Playback stops at end"), A.bPlaying);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPFlightDatasetTest, "TMOP.Flights.DataAndFilters",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPFlightDatasetTest::RunTest(const FString&)
{
    auto Data = MakeShared<FTMOPFlightData>();
    if (!TestTrue(TEXT("Staged flight JSON loads"), Data->Load())) { AddError(Data->Error); return false; }
    TestTrue(TEXT("Curated movements exist"), !Data->Legs.IsEmpty());
    TestEqual(TEXT("Window begins 24 hours before anchor"), Data->StartUTC, FDateTime(1986,2,27,22,21,30));
    FTMOPFlightState State; State.Data = Data; State.Refilter();
    TestEqual(TEXT("No filter retains every movement"), State.Filtered.Num(), Data->Legs.Num());
    TestTrue(TEXT("Overlap flights visible at start"), !State.Active.IsEmpty());
    State.Airline = TEXT("golden-air"); State.Refilter();
    TestEqual(TEXT("Friday-only Golden Air timetable has twelve movements"), State.Filtered.Num(), 12);
    State.Country = TEXT("no"); State.Refilter();
    TestEqual(TEXT("Country also matches arrival/departure, not just airline base"), State.Filtered.Num(), 6);
    if (!State.Filtered.IsEmpty())
    {
        const int32 Index = State.Filtered[0]; const auto& Leg = Data->Legs[Index];
        State.bOnlyAirborne = true; State.SetTime(Leg.Departure);
        TestTrue(TEXT("Airborne-only list includes departure boundary"), State.Listed().Contains(Index));
        State.Select(Index); State.SetTime(Leg.Arrival);
        TestFalse(TEXT("Arrival removes flight from airborne list"), State.Listed().Contains(Index));
        State.Search = TEXT("unknown-flight-test-fixture"); State.Refilter();
        TestEqual(TEXT("Hidden selection cleared on filter change"), State.Selected, INDEX_NONE);
    }
    TestFalse(TEXT("Malformed JSON rejected"), Data->Parse(TEXT("{}")));
    TestTrue(TEXT("Partial data cleared after load error"), Data->Legs.IsEmpty() && Data->Airports.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPFlightLanguageTest, "TMOP.Flights.LanguageFallback",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPFlightLanguageTest::RunTest(const FString&)
{
    const FString Previous = FInternationalization::Get().GetCurrentLanguage()->GetName();
    FTMOPFlightText Text; Text.Values = MakeShared<FJsonObject>();
    Text.Values->SetStringField(TEXT("sv"), TEXT("Original"));
    Text.Values->SetStringField(TEXT("en"), TEXT("Translation"));
    Text.Values->SetStringField(TEXT("en_source"), TEXT("Original"));
    FInternationalization::Get().SetCurrentLanguage(TEXT("en"));
    TestEqual(TEXT("Current English is used"), Text.Resolve(TEXT("flight-test-only"),TEXT("notes")).ToString(), FString(TEXT("Translation")));
    Text.Values->SetStringField(TEXT("sv"), TEXT("Ändrad källa"));
    TestEqual(TEXT("Stale translation falls back to Swedish"), Text.Resolve(TEXT("flight-test-only"),TEXT("notes")).ToString(), FString(TEXT("Ändrad källa")));
    FInternationalization::Get().SetCurrentLanguage(Previous);
    return true;
}
#endif
