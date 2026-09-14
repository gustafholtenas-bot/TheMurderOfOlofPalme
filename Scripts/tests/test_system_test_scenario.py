"""Offline contracts for the two-person/vehicle 09_12 systems test."""

import io
import json
from pathlib import Path
import unittest
import zipfile


ROOT = Path(__file__).resolve().parents[2]
DATA = ROOT / "DataTables" / "09_12"
DRIVER = "SYSTEM_TEST_DRIVER"
PASSENGER = "SYSTEM_TEST_PASSENGER"
VEHICLE = "VEHICLE_SYSTEM_TEST_GRAND"
DIALOGUE = "SYSTEM_TEST_GRAND_TIMED_DIALOGUE"


class SystemTestScenarioContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with zipfile.ZipFile(DATA / "DT_TMOP_People.zip") as archive:
            with archive.open("DT_TMOP_People.json") as raw:
                with io.TextIOWrapper(raw, encoding="utf-16") as handle:
                    cls.people = json.load(handle)
        with (DATA / "DT_TMOP_HistoricalVehicles.json").open(encoding="utf-16") as handle:
            cls.vehicles = json.load(handle)
        with (DATA / "DT_TMOP_HistoricalEvents.json").open(encoding="utf-16") as handle:
            cls.events = json.load(handle)

    @staticmethod
    def row(rows, name):
        matches = [row for row in rows if row.get("Name") == name]
        if len(matches) != 1:
            raise AssertionError(f"Expected one {name}, found {len(matches)}")
        return matches[0]

    def test_rows_are_unique_and_explicitly_fictional(self):
        for person_id in (DRIVER, PASSENGER):
            person = self.row(self.people, person_id)
            self.assertEqual(person["CategoryId"], "SYSTEM_TEST")
            self.assertIn("FIKTIV", person["Notes"])
            self.assertTrue(person["bSpawnInSimulation"])
        vehicle = self.row(self.vehicles, VEHICLE)
        self.assertEqual(vehicle["CategoryId"], "SYSTEM_TEST")
        self.assertEqual(vehicle["Confidence"], "FictionalGameplay")

    def test_people_and_vehicle_are_bidirectionally_linked(self):
        vehicle = self.row(self.vehicles, VEHICLE)
        self.assertEqual(vehicle["AssociatedPersonEntityIds"], [DRIVER, PASSENGER])
        self.assertEqual(vehicle["KnownDriverEntityId"], DRIVER)
        for person_id in (DRIVER, PASSENGER):
            self.assertEqual(self.row(self.people, person_id)["AssociatedVehicleIds"], [VEHICLE])

    def test_vehicle_arrives_at_grand_then_runs_both_maneuvers(self):
        timeline = self.row(self.vehicles, VEHICLE)["Timeline"]
        drive = next(entry for entry in timeline if entry["EntryId"].endswith("DRIVE_NORTH_TO_GRAND"))
        self.assertEqual(drive["RouteStartAnchorId"], "EnterSveavagenN_Car")
        self.assertEqual(drive["RouteDestinationAnchorId"], "GrandOutside_4_Curb")
        self.assertEqual(drive["VehicleRouteMode"], "ManualLaneRoute")
        self.assertEqual(drive["OrderedLaneIds"], [
            "SVEAVAGENS_001_R2",
            "X_SVEAVAGENS_001_R2_TO_SVEAVAGENS_002_R2_STRAIGHT",
            "SVEAVAGENS_002_R2",
        ])
        self.assertTrue(drive["bTimeIsArrival"])
        maneuvers = [entry for entry in timeline if entry["VehicleRouteMode"] == "AnchorManeuver"]
        self.assertEqual(len(maneuvers), 2)
        self.assertFalse(maneuvers[0]["bAnchorManeuverReverse"])
        self.assertTrue(maneuvers[1]["bAnchorManeuverReverse"])

    def test_each_person_exercises_vehicle_movement_animation_and_dialogue(self):
        required = {
            "InitialPlacement", "EnterVehicle", "ExitVehicle", "MoveToAnchor",
            "LookAtAnchor", "PlayUniqueAnimation", "StopUniqueAnimation",
            "MeetingDialogue", "Wait"
        }
        for person_id in (DRIVER, PASSENGER):
            person = self.row(self.people, person_id)
            actions = {entry["Action"] for entry in person["Timeline"]}
            self.assertTrue(required.issubset(actions))
            self.assertIn("SYSTEMTEST – FÖRE SKOTTET", person["Dialog"]["BeforeShot"])
            self.assertIn("SYSTEMTEST – EFTER SKOTTET", person["Dialog"]["AfterShot"])
            self.assertEqual(len(person["AutomaticSpeech"]), 2)

    def test_timed_dialogue_has_event_owner_participants_and_four_visual_lines(self):
        event = self.row(self.events, DIALOGUE)
        self.assertEqual(event["TimingMode"], "Absolute")
        self.assertEqual(event["AbsoluteTime"], {"Hour": 23, "Minute": 3, "Second": 0})
        owner = self.row(self.people, DRIVER)
        dialogue = owner["MeetingDialogues"][0]
        self.assertEqual(dialogue["DialogueId"], DIALOGUE)
        self.assertEqual(dialogue["ParticipantEntityIds"], [DRIVER, PASSENGER])
        self.assertEqual([line["OffsetSeconds"] for line in dialogue["Lines"]], [0, 5, 10, 15])
        self.assertTrue(all("TIDSDIALOG" in line["Text"] for line in dialogue["Lines"]))

    def test_player_takeover_window_has_no_later_vehicle_command(self):
        timeline = self.row(self.vehicles, VEHICLE)["Timeline"]
        self.assertEqual(timeline[-1]["Action"], "Park")
        self.assertEqual(timeline[-1]["Time"], {"Hour": 23, "Minute": 4, "Second": 45})


if __name__ == "__main__":
    unittest.main()
