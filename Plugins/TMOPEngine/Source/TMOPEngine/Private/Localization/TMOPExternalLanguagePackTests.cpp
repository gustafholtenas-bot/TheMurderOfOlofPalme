#include "Localization/TMOPLocalization.h"
#include "Internationalization/Culture.h"
#include "Internationalization/Internationalization.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "People/TMOPPersonProfileTypes.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPExternalLanguagePackTest, "TMOP.Localization.ExternalPack",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ClientContext | EAutomationTestFlags::EngineFilter)

bool FTMOPExternalLanguagePackTest::RunTest(const FString& Parameters)
{
    const FString PreviousLanguage = FInternationalization::Get().GetCurrentLanguage()->GetName();
    IFileManager::Get().MakeDirectory(*FPaths::ProjectSavedDir(), true);
    const FString File = FPaths::CreateTempFilename(*FPaths::ProjectSavedDir(), TEXT("TMOP_Loc_"), TEXT(".json"));
    const FString Source = TEXT("Testkälla {0}");
    const FString Json = FString::Printf(
        TEXT("{\"schema_version\":1,\"language\":\"en\",\"entries\":[{\"namespace\":\"TMOP.Source\",\"key\":\"%s\",\"source\":\"Testkälla {0}\",\"translation\":\"Test source {0}\"}]}"),
        *FTMOPLocalization::SourceKey(Source));
    TestTrue(TEXT("Write temporary pack"), FFileHelper::SaveStringToFile(Json, *File, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM));
    TestTrue(TEXT("Load valid pack"), FTMOPLocalization::LoadPack(File, TEXT("en")));
    const FText Existing = FTMOPLocalization::Text(Source);
    FInternationalization::Get().SetCurrentLanguage(TEXT("en"));
    TestEqual(TEXT("External text resolves"), Existing.ToString(), FString(TEXT("Test source {0}")));
    FInternationalization::Get().SetCurrentLanguage(TEXT("sv"));
    TestEqual(TEXT("Same FText returns to Swedish"), Existing.ToString(), Source);

    const FString Invalid = FString::Printf(
        TEXT("{\"schema_version\":1,\"language\":\"en\",\"entries\":[{\"namespace\":\"TMOP.Source\",\"key\":\"%s\",\"source\":\"Atomic pack test\",\"translation\":\"Must not apply\"},{\"namespace\":\"TMOP.Source\",\"key\":\"bad\",\"source\":\"Bad entry\",\"translation\":\"Rejected\"}]}"),
        *FTMOPLocalization::SourceKey(TEXT("Atomic pack test")));
    FFileHelper::SaveStringToFile(Invalid, *File, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    AddExpectedError(TEXT("Invalid entry in TMOP language pack"), EAutomationExpectedErrorFlags::Contains, 1);
    TestFalse(TEXT("Reject malformed pack"), FTMOPLocalization::LoadPack(File, TEXT("en")));
    FInternationalization::Get().SetCurrentLanguage(TEXT("en"));
    TestEqual(TEXT("No partial application"), FTMOPLocalization::Text(TEXT("Atomic pack test")).ToString(), FString(TEXT("Atomic pack test")));
    // Same Swedish sentence, different row contexts. A stale or unreviewed row
    // must not borrow an approved neighbour's English translation.
    auto MakeEntry = [](const FString& Row, const FString& Translation, const FString& Status)
    {
        auto Entry = MakeShared<FJsonObject>();
        Entry->SetStringField(TEXT("namespace"), TEXT("TMOP.Table"));
        Entry->SetStringField(TEXT("table"), TEXT("DT_TMOP_LocTest"));
        Entry->SetStringField(TEXT("row"), Row);
        Entry->SetStringField(TEXT("field"), TEXT("ObservationSummary"));
        Entry->SetStringField(TEXT("key"), FTMOPLocalization::TableKey(TEXT("DT_TMOP_LocTest"), Row, TEXT("ObservationSummary")));
        Entry->SetStringField(TEXT("source"), TEXT("Samma testtext"));
        Entry->SetStringField(TEXT("source_hash"), FTMOPLocalization::SourceKey(TEXT("Samma testtext")));
        Entry->SetStringField(TEXT("translation"), Translation);
        Entry->SetStringField(TEXT("status"), Status);
        return MakeShared<FJsonValueObject>(Entry);
    };
    auto StablePack = MakeShared<FJsonObject>();
    StablePack->SetNumberField(TEXT("schema_version"), 2);
    StablePack->SetStringField(TEXT("language"), TEXT("en"));
    TArray<TSharedPtr<FJsonValue>> StableEntries;
    StableEntries.Add(MakeEntry(TEXT("A"), TEXT("First context"), TEXT("reviewed")));
    StableEntries.Add(MakeEntry(TEXT("B"), TEXT("Second context"), TEXT("reviewed")));
    StableEntries.Add(MakeEntry(TEXT("C"), TEXT("Unreviewed draft"), TEXT("draft")));
    StablePack->SetArrayField(TEXT("entries"), StableEntries);
    FString StableJson;
    FJsonSerializer::Serialize(StablePack, TJsonWriterFactory<>::Create(&StableJson));
    FFileHelper::SaveStringToFile(StableJson, *File, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
    TestTrue(TEXT("Load stable pack"), FTMOPLocalization::LoadPack(File, TEXT("en")));
    const auto Lookup = [](const TCHAR* Row, const TCHAR* Source)
    {
        return FTMOPLocalization::TableText(TEXT("DT_TMOP_LocTest"), Row, TEXT("ObservationSummary"), FString(Source));
    };
    const FText ContextA = Lookup(TEXT("A"), TEXT("Samma testtext"));
    TestEqual(TEXT("First context"), ContextA.ToString(), FString(TEXT("First context")));
    TestEqual(TEXT("Second context"), Lookup(TEXT("B"), TEXT("Samma testtext")).ToString(), FString(TEXT("Second context")));
    TestEqual(TEXT("Draft stays Swedish"), Lookup(TEXT("C"), TEXT("Samma testtext")).ToString(), FString(TEXT("Samma testtext")));
    TestEqual(TEXT("Stale pack stays Swedish"), Lookup(TEXT("A"), TEXT("Ändrad text")).ToString(), FString(TEXT("Ändrad text")));
    TestEqual(TEXT("Ambiguous legacy lookup stays Swedish"), FTMOPLocalization::Text(FString(TEXT("Samma testtext"))).ToString(), FString(TEXT("Samma testtext")));
    FTMOPPersonProfileRow Original;
    Original.ObservationSummary = FText::FromString(TEXT("Samma testtext"));
    Original.FullName = FText::FromString(TEXT("Samma testtext"));
    const auto View = FTMOPLocalization::RowView(TEXT("DT_TMOP_LocTest"), TEXT("A"), Original);
    TestEqual(TEXT("View is translated"), View.ObservationSummary.ToString(), FString(TEXT("First context")));
    TestEqual(TEXT("Original row unchanged"), Original.ObservationSummary.ToString(), FString(TEXT("Samma testtext")));
    TestEqual(TEXT("Proper name unchanged"), View.FullName.ToString(), Original.FullName.ToString());
    FInternationalization::Get().SetCurrentLanguage(TEXT("sv"));
    TestEqual(TEXT("Stable FText switches back live"), ContextA.ToString(), FString(TEXT("Samma testtext")));
    FInternationalization::Get().SetCurrentLanguage(PreviousLanguage);
    IFileManager::Get().Delete(*File);
    return true;
}
#endif
