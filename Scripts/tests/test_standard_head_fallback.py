import csv
import unittest
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
RESOLVER_CPP = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/People/TMOPAppearanceResolver.cpp"
RESOLVER_H = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/People/TMOPAppearanceResolver.h"
NPC_CPP = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/People/TMOPCharacterAppearanceComponent.cpp"
PLAYER_CPP = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/People/TMOPPlayerAppearanceDirector.cpp"
CATALOG_CSV = ROOT / "Setup/09_FacesAndHair/DT_TMOP_AppearanceAssets_FacesHair.csv"


class StandardHeadFallbackTests(unittest.TestCase):
    def test_blueprint_helper_and_all_catalog_ids_exist_in_runtime(self):
        header = RESOLVER_H.read_text(encoding="utf-8")
        source = RESOLVER_CPP.read_text(encoding="utf-8")
        self.assertIn("GetStandardFaceCatalogId", header)
        self.assertIn("UFUNCTION(BlueprintPure", header)
        for gender in ("MALE", "FEMALE"):
            for age in (18, 30, 45, 65):
                self.assertIn(f'FACE_STANDARD_%s_%d', source)
        self.assertIn("AgeAtEvent <= 24", source)
        self.assertIn("AgeAtEvent <= 37", source)
        self.assertIn("AgeAtEvent <= 54", source)

    def test_explicit_or_bespoke_head_precedes_standard_fallback(self):
        source = RESOLVER_CPP.read_text(encoding="utf-8")
        explicit = source.index("const bool bHasExplicitFace")
        standard = source.index("const FName StandardFaceCatalogId", explicit)
        resolved = source.index("OutAppearance.Face = ResolvePart", standard)
        self.assertLess(explicit, standard)
        self.assertLess(standard, resolved)
        self.assertIn(
            "bHasExplicitFace || bUsesBespokeHeadFlow", source[explicit:resolved]
        )

    def test_hidden_unknown_policy_does_not_discard_explicit_face(self):
        source = RESOLVER_CPP.read_text(encoding="utf-8")
        start = source.index("FTMOPResolvedAppearancePart UTMOPAppearanceResolver::ResolvePart")
        end = source.index("bool UTMOPAppearanceResolver::ResolveAppearance", start)
        resolve_part = source[start:end]
        self.assertIn("!bKnown && !bHasExplicitOverride", resolve_part)

    def test_eight_standard_rows_have_expected_ranges_and_head_mask(self):
        with CATALOG_CSV.open(encoding="utf-8-sig", newline="") as handle:
            rows = {row["CatalogId"]: row for row in csv.DictReader(handle)}
        ranges = {
            18: (15, 24),
            30: (25, 37),
            45: (38, 54),
            65: (55, 0),
        }
        for gender, csv_gender in (("MALE", "Male"), ("FEMALE", "Female")):
            for age, expected_range in ranges.items():
                catalog_id = f"FACE_STANDARD_{gender}_{age}"
                self.assertIn(catalog_id, rows)
                row = rows[catalog_id]
                self.assertEqual(row["Name"], catalog_id)
                self.assertEqual(row["PartType"], "Face")
                self.assertEqual(row["Gender"], csv_gender)
                self.assertEqual(
                    (int(row["MinimumAge"]), int(row["MaximumAge"])),
                    expected_range,
                )
                self.assertEqual(int(row["HiddenBodyRegions"]), 1)
                self.assertTrue(row["Mesh"])

    def test_visible_face_always_masks_base_body_head_for_npc_and_player(self):
        for path in (NPC_CPP, PLAYER_CPP):
            source = path.read_text(encoding="utf-8")
            self.assertIn("ETMOPBodyRegion::Head", source)
            self.assertIn("ResolvedAppearance.Face", source)
            self.assertIn("TMOPBodyRegionMask(ETMOPBodyRegion::Head)", source)


if __name__ == "__main__":
    unittest.main()
