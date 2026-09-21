#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Materials/Material.h"
#include "People/TMOPAppearanceResolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPHairMaterialKeysTest,
    "TMOP.Appearance.Hair.MaterialKeys",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPHairMaterialKeysTest::RunTest(const FString& Parameters)
{
    FTMOPAppearanceSlot Evidence;
    TestEqual(TEXT("Unknown does not invent a colour"),
        UTMOPAppearanceResolver::GetHairMaterialKey(Evidence, ETMOPHairColor::Unknown), FName(TEXT("Unknown")));
    Evidence.OriginalText = TEXT("mörkblont, halvlångt");
    TestEqual(TEXT("Dark blond is not generic dark"),
        UTMOPAppearanceResolver::GetHairMaterialKey(Evidence, ETMOPHairColor::Blond), FName(TEXT("DarkBlond")));
    Evidence.OriginalText = TEXT("cendréfärgat");
    TestEqual(TEXT("Ash blond refines category"),
        UTMOPAppearanceResolver::GetHairMaterialKey(Evidence, ETMOPHairColor::Blond), FName(TEXT("Cendre")));
    Evidence.OriginalText = TEXT("svart med grått inslag");
    TestEqual(TEXT("Mixed grey stays distinct"),
        UTMOPAppearanceResolver::GetHairMaterialKey(Evidence, ETMOPHairColor::Black), FName(TEXT("SaltAndPepper")));
    Evidence.OriginalText = TEXT("blåsvart hår");
    TestEqual(TEXT("Blue black stays distinct"),
        UTMOPAppearanceResolver::GetHairMaterialKey(Evidence, ETMOPHairColor::Black), FName(TEXT("BlueBlack")));
    const FName UnknownA = UTMOPAppearanceResolver::GetDeterministicUnknownHairMaterialKey(19860228, 30);
    const FName UnknownB = UTMOPAppearanceResolver::GetDeterministicUnknownHairMaterialKey(19860228, 30);
    TestEqual(TEXT("Unknown colour is deterministic"), UnknownA, UnknownB);
    TestTrue(TEXT("Unknown colour resolves to a real material key"),
        UnknownA == TEXT("Blond") || UnknownA == TEXT("Brown") ||
        UnknownA == TEXT("Dark") || UnknownA == TEXT("Black") ||
        UnknownA == TEXT("Red") || UnknownA == TEXT("Grey") ||
        UnknownA == TEXT("White"));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPHairCatalogSelectionTest,
    "TMOP.Appearance.Hair.CatalogSelection",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPHairCatalogSelectionTest::RunTest(const FString& Parameters)
{
    UDataTable* Table = NewObject<UDataTable>();
    Table->RowStruct = FTMOPAppearanceAssetRow::StaticStruct();
    FTMOPAppearanceAssetRow Valid;
    Valid.CatalogId = TEXT("CUT");
    Valid.PartType = ETMOPAppearancePartType::Hair;
    Valid.Gender = ETMOPPersonGender::Male;
    Valid.StaticMesh = NewObject<UStaticMesh>();
    Valid.Tags = { FName(TEXT("Short")) };
    Valid.SelectionWeight = 1.0f;
    Table->AddRow(Valid.CatalogId, Valid);
    FTMOPAppearanceAssetRow Disabled = Valid;
    Disabled.CatalogId = TEXT("DISABLED");
    Disabled.SelectionWeight = 0.0f;
    Disabled.Tags.Add(TEXT("Curly"));
    Table->AddRow(Disabled.CatalogId, Disabled);
    FTMOPAppearanceAssetRow Pending = Disabled;
    Pending.CatalogId = TEXT("PENDING");
    Pending.SelectionWeight = 100.0f;
    Pending.Tags.Add(TEXT("PendingAsset"));
    Table->AddRow(Pending.CatalogId, Pending);
    FTMOPAppearanceAssetRow Empty = Disabled;
    Empty.CatalogId = TEXT("EMPTY");
    Empty.SelectionWeight = 100.0f;
    Empty.StaticMesh.Reset();
    Table->AddRow(Empty.CatalogId, Empty);
    FTMOPPersonProfileRow Profile;
    Profile.Gender = ETMOPPersonGender::Male;
    Profile.Hair.Tags = { FName(TEXT("Short")), FName(TEXT("Curly")) };
    Profile.HairColorCategory = ETMOPHairColor::Blond;
    FTMOPResolvedAppearance Result;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Result);
    TestEqual(TEXT("Empty, pending and zero-weight cuts cannot beat valid geometry"), Result.Hair.CatalogId, Valid.CatalogId);
    Profile.HairColorCategory = ETMOPHairColor::Black;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Result);
    TestEqual(TEXT("Colour change does not change cut"), Result.Hair.CatalogId, Valid.CatalogId);
    UMaterial* ExplicitMaterial = NewObject<UMaterial>();
    Profile.AppearanceProfile.Hair.CatalogId = Valid.CatalogId;
    Profile.AppearanceProfile.Hair.MaterialOverride = ExplicitMaterial;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Result);
    TestTrue(TEXT("Catalog lookup preserves explicit material"), Result.Hair.Material.Get() == ExplicitMaterial);
    Profile.HairColorCategory = ETMOPHairColor::Bald;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Result);
    TestTrue(TEXT("Bald remains intentionally empty"), Result.Hair.bIntentionallyEmpty);
    return true;
}
#endif
