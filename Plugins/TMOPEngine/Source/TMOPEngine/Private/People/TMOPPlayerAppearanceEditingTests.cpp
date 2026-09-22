#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Engine/DataTable.h"
#include "Engine/SkeletalMesh.h"
#include "People/TMOPAppearanceResolver.h"
#include "People/TMOPPlayerAppearanceDirector.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPPlayerAppearanceHiddenTest,
    "TMOP.Appearance.Player.ExplicitHiddenAndEvidence",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FTMOPPlayerAppearanceHiddenTest::RunTest(const FString& Parameters)
{
    UDataTable* Table = NewObject<UDataTable>();
    Table->RowStruct = FTMOPAppearanceAssetRow::StaticStruct();
    FTMOPAppearanceAssetRow Row;
    Row.CatalogId = TEXT("TEST_JACKET");
    Row.PartType = ETMOPAppearancePartType::Outerwear;
    Row.Mesh = NewObject<USkeletalMesh>();
    Row.HiddenBodyRegions = TMOPBodyRegionMask(ETMOPBodyRegion::Torso);
    Table->AddRow(Row.CatalogId, Row);
    FTMOPPersonProfileRow Profile;
    Profile.Gender = ETMOPPersonGender::Male;
    Profile.AppearanceProfile.Outerwear.CatalogId = Row.CatalogId;
    Profile.OuterwearCategory = ETMOPOuterwearType::None;
    FTMOPResolvedAppearance Result;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Result);
    TestFalse(TEXT("Explicit player jacket overrides old absent evidence"), Result.Outerwear.bIntentionallyEmpty);
    TestEqual(TEXT("Exact jacket retained"), Result.Outerwear.CatalogId, Row.CatalogId);
    Profile.AppearanceProfile.Outerwear.bHidden = true;
    UTMOPAppearanceResolver::ResolveAppearance(Profile, Table, Result);
    TestTrue(TEXT("Inget wins over catalog/evidence"), Result.Outerwear.bIntentionallyEmpty);
    TestTrue(TEXT("No hidden garment geometry"), Result.Outerwear.Mesh.IsNull());
    TestEqual(TEXT("Removed garment carries no skin mask"), Result.Outerwear.HiddenBodyRegions, 0);
    ATMOPPlayerAppearanceDirector::Choice(Profile, ETMOPAppearancePartType::Hair).bHidden = true;
    TestTrue(TEXT("Slot editor writes correct slot"), Profile.AppearanceProfile.Hair.bHidden);
    TestFalse(TEXT("Slot editor leaves other slots alone"), Profile.AppearanceProfile.Trousers.bHidden);
    return true;
}
#endif
