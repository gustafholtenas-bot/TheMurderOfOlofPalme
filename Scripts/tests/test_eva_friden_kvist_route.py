"""Offline contracts for Eva Friden Kvist's reconstructed 09_12 route."""

import io
import json
from pathlib import Path
import unittest
import zipfile


ROOT = Path(__file__).resolve().parents[2]
PEOPLE = ROOT / "DataTables" / "09_12" / "DT_TMOP_People.zip"
EVA = "EVA_FRIDEN_KVIST_SOCIAL_A"
BOYFRIEND = "EVA_FRIDEN_KVIST_BOYFRIEND_1986"


class EvaFridenKvistRouteContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with zipfile.ZipFile(PEOPLE) as archive:
            with archive.open("DT_TMOP_People.json") as raw:
                with io.TextIOWrapper(raw, encoding="utf-16") as handle:
                    cls.rows = json.load(handle)

    def row(self, name):
        matches = [row for row in self.rows if row.get("Name") == name]
        self.assertEqual(len(matches), 1)
        return matches[0]

    def test_both_people_spawn_as_one_side_by_side_pair(self):
        for entity_id in (EVA, BOYFRIEND):
            row = self.row(entity_id)
            self.assertTrue(row["bSpawnInSimulation"])
            self.assertTrue(row["bPrioritizeTimeline"])
            self.assertEqual(row["SocialGroupId"], "GROUP_EVA_FRIDEN_KVIST_PAIR")
            self.assertEqual(row["GroupLeaderEntityId"], EVA)
            self.assertEqual(row["GroupFormation"], "SideBySide")

    def test_route_starts_at_2300_and_descends_at_dekorima_entrance(self):
        for entity_id in (EVA, BOYFRIEND):
            timeline = self.row(entity_id)["Timeline"]
            self.assertEqual(timeline[0]["Time"], {"Hour": 23, "Minute": 0, "Second": 0})
            self.assertEqual(timeline[0]["TargetAnchorId"], "ExitTunnelgatanW_Sidewalk1")
            self.assertEqual(
                [entry["TargetAnchorId"] for entry in timeline[1:]],
                ["MetroHotorget1_entrance", "MetroHotorget1_inside", "MetroHotorget1_inside"],
            )
            self.assertEqual(timeline[-1]["Action"], "Despawn")

    def test_companion_is_not_given_an_invented_identity(self):
        companion = self.row(BOYFRIEND)
        self.assertEqual(companion["FirstName"], "")
        self.assertEqual(companion["LastName"], "")
        self.assertIn("namn ej angivet", companion["FullName"])

    def test_source_and_reconstruction_are_distinguished(self):
        for entity_id in (EVA, BOYFRIEND):
            row = self.row(entity_id)
            self.assertIn("Direktmeddelande", row["GeneralSourceReference"])
            self.assertIn("användarstyrd rekonstruktion", row["Notes"])
            self.assertTrue(all(e["Confidence"] == "Reconstructed" for e in row["Timeline"]))

    def test_evas_post_murder_experience_uses_the_new_information_field(self):
        eva = self.row(EVA)
        self.assertIn("människor gråta", eva["PostMurderEventsSummary"])
        self.assertIn("löpsedlarna", eva["PostMurderEventsSummary"])
        self.assertEqual(self.row(BOYFRIEND)["PostMurderEventsSummary"], "")


if __name__ == "__main__":
    unittest.main()
