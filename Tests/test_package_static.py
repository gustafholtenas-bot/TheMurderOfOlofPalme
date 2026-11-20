"""Cheap static guards; run with Python 3 (no Unreal installation required)."""

from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine"
TYPES = (SOURCE / "Public/People/TMOPAppearanceAssetTypes.h").read_text()
RESOLVER = (SOURCE / "Private/People/TMOPAppearanceResolver.cpp").read_text()
COMPONENT = (SOURCE / "Private/People/TMOPCharacterAppearanceComponent.cpp").read_text()
COMPONENT_HEADER = (SOURCE / "Public/People/TMOPCharacterAppearanceComponent.h").read_text()
EDITOR_SCRIPT = (ROOT / "UnrealScripts/tmop_install_fabric_materials.py").read_text()


class FabricStaticTests(unittest.TestCase):
    def test_all_six_surfaces_are_consistent(self):
        for name in ("Wool", "Leather", "Quilted", "Cotton", "Denim", "Synthetic"):
            self.assertIn(f"    {name},", TYPES)
            self.assertIn(f"ETMOPClothingSurfaceType::{name}", RESOLVER)
            self.assertIn(f"MI_TMOP_Fabric_{name}", COMPONENT)
            self.assertIn(f'"{name}":', EDITOR_SCRIPT)

    def test_uniform_protection_and_manual_override(self):
        self.assertIn("IsProtectedUniformSelection(Part.CatalogId)", COMPONENT)
        self.assertIn("IsProtectedUniformTagSet(SurfaceTags)", RESOLVER)
        self.assertIn("Part.bMaterialExplicitlyOverridden", COMPONENT)
        self.assertIn("ETMOPClothingSurfaceType::PreserveCatalog", RESOLVER)

    def test_other_material_slots_preserved(self):
        self.assertIn("if (Index != 0) continue;", COMPONENT)
        self.assertIn("SurfaceMaterialSlotNames.Contains(SlotNames[Index])", COMPONENT)

    def test_legacy_material_when_fabric_pack_missing(self):
        self.assertIn("Material = Part.Material.LoadSynchronous();", COMPONENT)
        self.assertIn("bEnableAutomaticCivilianFabricMaterials", COMPONENT_HEADER)

    def test_no_clothing_master_or_old_color_mi_writes(self):
        self.assertIn('MASTER_NAME = "M_TMOP_ClothingFabric1986"', EDITOR_SCRIPT)
        self.assertNotIn('MASTER_NAME = "M_TMOP_Clothing1986"', EDITOR_SCRIPT)
        self.assertIn('return existing, False', EDITOR_SCRIPT)

    def test_keeps_priority3_appearance_performance_flags(self):
        self.assertIn("bDisableUnusedSkeletalPartTicks = true", COMPONENT_HEADER)
        self.assertIn("bEnableDistanceBasedAnimationLOD = true", COMPONENT_HEADER)


if __name__ == "__main__":
    unittest.main()
