import copy
import importlib.util
from pathlib import Path
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "tmop_map_skeletal_hair.py"
spec = importlib.util.spec_from_file_location("hair_mapping", SCRIPT)
mapping = importlib.util.module_from_spec(spec)
spec.loader.exec_module(mapping)


class HairMappingTests(unittest.TestCase):
    def setUp(self):
        self.rows = [dict(Name="HAIR_MALE_SHORT", CatalogId="HAIR_MALE_SHORT", PartType="Hair",
                          Mesh="None", StaticMesh="/Game/Old.hair_short", Material="/Game/Keep.Keep",
                          AttachmentSocket="HairSocket", Tags=["Short", "PendingAsset"],
                          SelectionWeight=0.8),
                     dict(Name="HAT", CatalogId="HAT", PartType="Headwear", StaticMesh="/Game/Hat.Hat",
                          AttachmentSocket="HeadwearSocket")]
        self.asset = {"name": "SK_HAIR_MALE_SHORT", "path": "/Game/Hair/SK_HAIR_MALE_SHORT"}

    def test_exact_mapping_preserves_material_and_accessories(self):
        original = copy.deepcopy(self.rows)
        rows, changes, missing = mapping.plan_mapping(self.rows, [self.asset])
        self.assertEqual(self.rows, original)
        self.assertEqual(rows[1], original[1])
        self.assertEqual(rows[0]["Material"], original[0]["Material"])
        self.assertEqual(rows[0]["SelectionWeight"], 0.8)
        self.assertEqual(rows[0]["Mesh"], "/Game/Hair/SK_HAIR_MALE_SHORT.SK_HAIR_MALE_SHORT")
        self.assertEqual(rows[0]["StaticMesh"], "None")
        self.assertEqual(rows[0]["AttachmentSocket"], "None")
        self.assertNotIn("PendingAsset", rows[0]["Tags"])
        self.assertEqual(len(changes), 1)
        self.assertFalse(missing)

    def test_missing_or_ambiguous_does_not_guess(self):
        duplicate = dict(self.asset, path="/Game/Other/SK_HAIR_MALE_SHORT")
        for assets in ([], [self.asset, duplicate]):
            rows, changes, missing = mapping.plan_mapping(self.rows, assets)
            self.assertEqual(rows, self.rows)
            self.assertFalse(changes)
            self.assertEqual(len(missing), 1)

    def test_explicit_mapping_resolves_ambiguity_and_is_idempotent(self):
        assets = [self.asset, dict(self.asset, path="/Game/Other/SK_HAIR_MALE_SHORT")]
        rows, changes, _ = mapping.plan_mapping(self.rows, assets,
                                               {"HAIR_MALE_SHORT": self.asset["path"]})
        again, changes_again, _ = mapping.plan_mapping(rows, assets,
                                                       {"HAIR_MALE_SHORT": self.asset["path"]})
        self.assertEqual(rows, again)
        self.assertTrue(changes)
        self.assertFalse(changes_again)

    def test_existing_skeletal_reference_is_retained(self):
        self.rows[0]["Mesh"] = "/Game/Legacy.Legacy"
        rows, changes, _ = mapping.plan_mapping(self.rows, [{"name": "Legacy", "path": "/Game/Legacy"}])
        self.assertEqual(rows[0]["Mesh"], "/Game/Legacy.Legacy")
        self.assertTrue(changes)


if __name__ == "__main__":
    unittest.main()
