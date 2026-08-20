#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "People/TMOPAppearanceResolver.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTMOPAppearanceDefaultsTest,
    "TMOP.Appearance.Defaults",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FTMOPAppearanceDefaultsTest::RunTest(const FString& Parameters)
{
    FTMOPPersonProfileRow Male;
    Male.EntityId = TEXT("TEST_MALE");
    Male.Gender = ETMOPPersonGender::Male;
    TestEqual(TEXT("Unknown male height uses 1986 default"),
        Male.GetResolvedHeightCentimeters(), 178.0f);
    TestTrue(TEXT("Unknown build resolves to average"),
        Male.GetResolvedBodyBuild() == ETMOPBodyBuild::Average);

    FTMOPPersonProfileRow Female;
    Female.EntityId = TEXT("TEST_FEMALE");
    Female.Gender = ETMOPPersonGender::Female;
    TestEqual(TEXT("Unknown female height uses 1986 default"),
        Female.GetResolvedHeightCentimeters(), 165.0f);

    FTMOPResolvedAppearance First;
    FTMOPResolvedAppearance Second;
    UTMOPAppearanceResolver::ResolveAppearance(Male, nullptr, First);
    UTMOPAppearanceResolver::ResolveAppearance(Male, nullptr, Second);
    TestEqual(TEXT("Appearance seed is stable"), First.ResolvedSeed, Second.ResolvedSeed);
    TestTrue(TEXT("Unknown face uses obscured fallback"),
        First.Face.bUsesObscuredFallback);
    TestEqual(TEXT("Unknown face is fully obscured"),
        First.Face.ObscurityAmount, 1.0f);
    TestFalse(TEXT("Unknown body type is not obscured"),
        First.Body.bUsesObscuredFallback);
    TestEqual(TEXT("Unknown trousers have the reserved ID"),
        First.Trousers.CatalogId, FName(TEXT("UNKNOWN_TROUSERS_OBSCURED")));

    FTMOPAppearanceSlot SwedishCoat;
    SwedishCoat.OriginalText = TEXT("lång mörkblå yllemek rock");
    const TArray<FName> CoatTags =
        UTMOPAppearanceResolver::GetNormalizedEvidenceTags(SwedishCoat);
    TestTrue(TEXT("Swedish dark blue is normalized"),
        CoatTags.Contains(TEXT("DarkBlue")));
    TestTrue(TEXT("Swedish coat is normalized"),
        CoatTags.Contains(TEXT("Coat")));
    TestTrue(TEXT("Swedish long is normalized"),
        CoatTags.Contains(TEXT("Long")));
    TestTrue(TEXT("Swedish wool is normalized"),
        CoatTags.Contains(TEXT("Wool")));

    FTMOPAppearanceSlot SwedishHat;
    SwedishHat.OriginalText = TEXT("svart stickad mössa");
    const TArray<FName> HatTags =
        UTMOPAppearanceResolver::GetNormalizedEvidenceTags(SwedishHat);
    TestTrue(TEXT("Knit cap is normalized"),
        HatTags.Contains(TEXT("KnitCap")));
    TestTrue(TEXT("Black is normalized"),
        HatTags.Contains(TEXT("Black")));

    FTMOPPersonProfileRow HiddenUnknown;
    HiddenUnknown.EntityId = TEXT("TEST_HIDDEN_UNKNOWN");
    HiddenUnknown.AppearanceProfile.UnknownPartStyle =
        ETMOPUnknownAppearanceStyle::Hidden;
    FTMOPResolvedAppearance HiddenResult;
    UTMOPAppearanceResolver::ResolveAppearance(HiddenUnknown, nullptr, HiddenResult);
    TestTrue(TEXT("Hidden unknown face is intentionally empty"),
        HiddenResult.Face.bIntentionallyEmpty);

    FTMOPPersonProfileRow DescribedFace;
    DescribedFace.EntityId = TEXT("TEST_DESCRIBED_FACE");
    DescribedFace.AgeAtEvent = 42;
    DescribedFace.FaceShape.OriginalText =
        TEXT("rektangulärt ansikte med markerade kindknotor och framskjutet hakparti");
    DescribedFace.Nose.OriginalText = TEXT("smal lång näsa och tunna läppar");
    FTMOPResolvedAppearance DescribedResult;
    UTMOPAppearanceResolver::ResolveAppearance(
        DescribedFace, nullptr, DescribedResult);
    TestEqual(TEXT("Rectangular face maps to expected width"),
        DescribedResult.FaceMorphs.FaceWidth, 0.18f);
    TestEqual(TEXT("Projected jaw maps to expected morph"),
        DescribedResult.FaceMorphs.JawProjection, 0.48f);
    TestEqual(TEXT("Marked cheekbones map to expected morph"),
        DescribedResult.FaceMorphs.CheekboneProminence, 0.50f);
    TestEqual(TEXT("Thin lips map to expected morph"),
        DescribedResult.FaceMorphs.LipThickness, -0.48f);
    TestTrue(TEXT("Unknown glasses are not invented"),
        First.Glasses.bIntentionallyEmpty);
    TestTrue(TEXT("Unknown scarf is not invented"),
        First.Scarf.bIntentionallyEmpty);
    TestTrue(TEXT("Unknown facial hair is not invented"),
        First.FacialHair.bIntentionallyEmpty);

    FTMOPAppearanceSlot Accessories;
    Accessories.OriginalText = TEXT("svart mustasch, metallbågade glasögon och halsduk");
    const TArray<FName> AccessoryTags =
        UTMOPAppearanceResolver::GetNormalizedEvidenceTags(Accessories);
    TestTrue(TEXT("Mustache is normalized"), AccessoryTags.Contains(TEXT("Mustache")));
    TestTrue(TEXT("Glasses are normalized"), AccessoryTags.Contains(TEXT("Glasses")));
    TestTrue(TEXT("Scarf is normalized"), AccessoryTags.Contains(TEXT("Scarf")));

    FTMOPPersonProfileRow MetaHuman;
    MetaHuman.EntityId = TEXT("TEST_METAHUMAN");
    MetaHuman.AppearanceProfile.GenerationMode =
        ETMOPAppearanceGenerationMode::MetaHuman;
    FTMOPResolvedAppearance MetaHumanResult;
    UTMOPAppearanceResolver::ResolveAppearance(
        MetaHuman, nullptr, MetaHumanResult);
    TestTrue(TEXT("MetaHuman mode marks the resolved appearance as bespoke"),
        MetaHumanResult.bUsesBespokeMetaHuman);
    TestEqual(TEXT("MetaHuman body is preserved"),
        MetaHumanResult.Body.CatalogId, FName(TEXT("METAHUMAN_BESPOKE")));
    TestEqual(TEXT("MetaHuman face is preserved"),
        MetaHumanResult.Face.CatalogId, FName(TEXT("METAHUMAN_BESPOKE")));
    TestEqual(TEXT("MetaHuman hair is preserved"),
        MetaHumanResult.Hair.CatalogId, FName(TEXT("METAHUMAN_BESPOKE")));
    TestEqual(TEXT("MetaHuman facial hair is preserved"),
        MetaHumanResult.FacialHair.CatalogId,
        FName(TEXT("METAHUMAN_BESPOKE")));
    TestFalse(TEXT("MetaHuman mode still resolves modular outerwear"),
        MetaHumanResult.Outerwear.CatalogId.IsNone());
    return true;
}

#endif
