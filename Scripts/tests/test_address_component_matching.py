"""Run outside Unreal: python -m unittest discover -s Scripts/tests -p test_address_component_matching.py"""
import importlib.util
from pathlib import Path
import unittest

SPEC = importlib.util.spec_from_file_location("address_install", Path(__file__).parents[1] / "tmop_install_address_components.py")
INSTALL = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(INSTALL)
REGISTRY = "/Game/Test.Registry"


def row(number=12, suffix="", **fields):
    return dict(Name="SVEAVAGEN_" + str(number) + suffix, AddressId="SVEAVAGEN_" + str(number) + suffix,
                StreetName="Sveavägen", StreetNumber=number, EntranceSuffix=suffix, **fields)


def anchor(identity="Sveavagen12", path="level.actor", **fields):
    return dict(path=path, id=identity, label=identity, display=identity, **fields)


class AddressMatchingTests(unittest.TestCase):
    def plan(self, rows, anchors):
        return INSTALL.plan_bindings(rows, anchors, REGISTRY)

    def test_swedish_names_and_spaces_match(self):
        result = self.plan([row()], [anchor("Sveavägen 12")])[0]
        self.assertEqual(result["status"], "candidate")

    def test_house_number_ranges_do_not_collapse(self):
        self.assertEqual(INSTALL.address_key("Gatan_5_7"), INSTALL.address_key("Gatan 5–7"))
        self.assertNotEqual(INSTALL.address_key("Gatan 5-7"), INSTALL.address_key("Gatan 57"))
        self.assertNotEqual(INSTALL.address_key("Gatan 5 7"), INSTALL.address_key("Gatan 57"))

    def test_entrances_are_distinct(self):
        self.assertEqual(self.plan([row(suffix="A")], [anchor()])[0]["status"], "missing")
        self.assertEqual(self.plan([row(suffix="A")], [anchor("Sveavagen12B")])[0]["status"], "missing")

    def test_no_substring_or_inside_anchor_guessing(self):
        for identity in ("Sveavagen120", "Sveavagen12_inside", "Sveavagen12_floor2"):
            self.assertEqual(self.plan([row()], [anchor(identity)])[0]["status"], "missing")

    def test_explicit_link_has_priority(self):
        result = self.plan([row(EntranceAnchorId="DOOR_12")], [anchor(), anchor("DOOR_12", "other")])[0]
        self.assertEqual(result["anchor"], "other")

    def test_missing_explicit_link_never_falls_back(self):
        result = self.plan([row(EntranceAnchorId="UNLOADED")], [anchor()])[0]
        self.assertEqual(result["status"], "missing")
        self.assertIn("UNLOADED", result["reason"])

    def test_duplicate_anchors_are_ambiguous(self):
        result = self.plan([row()], [anchor(), anchor(path="second")])[0]
        self.assertEqual(result["status"], "ambiguous")
        self.assertEqual(len(result["candidates"]), 2)

    def test_existing_component_survives_actor_rename(self):
        result = self.plan([row()], [anchor("Renamed", registry=REGISTRY, row="SVEAVAGEN_12")])[0]
        self.assertEqual(result["status"], "candidate")
        self.assertEqual(result["match_method"], "existing_component")

    def test_explicit_tag_connects_differently_named_anchor(self):
        result = self.plan([row(DoorbellActorTag="DOORBELL_12")],
                           [anchor("DoorAnchor", tags=["DOORBELL_12"])])[0]
        self.assertEqual(result["status"], "candidate")
        self.assertEqual(result["match_method"], "doorbell_tag")

    def test_two_rows_cannot_claim_one_anchor(self):
        result = self.plan([row(), row(13, DoorbellActorTag="SECOND_ADDRESS")],
                           [anchor(tags=["SECOND_ADDRESS"])])
        self.assertEqual([r["status"] for r in result], ["conflict", "conflict"])

    def test_plan_is_repeatable_and_does_not_mutate_input(self):
        rows, anchors = [row()], [anchor()]
        first = self.plan(rows, anchors)
        self.assertEqual(first, self.plan(rows, anchors))
        self.assertNotIn("EntranceAnchorId", rows[0])
        self.assertNotIn("registry", anchors[0])


if __name__ == "__main__":
    unittest.main()
