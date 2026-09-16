"""Offline contracts for Bosse Testhund in the current system-test tables."""

import io
import json
from pathlib import Path
import unittest
import zipfile


ROOT = Path(__file__).resolve().parents[2]
DATA = ROOT / "DataTables" / "09_14"
DOG = "SYSTEM_TEST_DOG"
DRIVER = "SYSTEM_TEST_DRIVER"
PASSENGER = "SYSTEM_TEST_PASSENGER"
VEHICLE = "VEHICLE_SYSTEM_TEST_GRAND"


class SystemTestDogContracts(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        with zipfile.ZipFile(DATA / "DT_TMOP_People.zip") as archive:
            with archive.open("DT_TMOP_People.json") as raw:
                cls.people = json.load(io.TextIOWrapper(raw, encoding="utf-16"))
        with (DATA / "DT_TMOP_HistoricalVehicles.json").open(
            encoding="utf-16"
        ) as handle:
            cls.vehicles = json.load(handle)

    @staticmethod
    def row(rows, identity):
        matches = [row for row in rows if
                   row.get("EntityId") == identity or
                   row.get("VehicleId") == identity]
        if len(matches) != 1:
            raise AssertionError(f"Expected one {identity}, found {len(matches)}")
        return matches[0]

    def test_dog_remains_a_people_row_and_uses_dog_presentation(self):
        dog = self.row(self.people, DOG)
        self.assertEqual(dog["CategoryId"], "SYSTEM_TEST")
        self.assertEqual(dog["Species"], "Dog")
        self.assertEqual(dog["FullName"], "Bosse Testhund")
        self.assertIn("FIKTIV", dog["Notes"])

    def test_dog_rides_both_car_segments_in_rear_left_seat(self):
        dog = self.row(self.people, DOG)
        entries = [entry for entry in dog["Timeline"]
                   if entry["Action"] in ("EnterVehicle", "ExitVehicle")]
        self.assertEqual([entry["Action"] for entry in entries],
                         ["EnterVehicle", "ExitVehicle", "EnterVehicle", "ExitVehicle"])
        self.assertTrue(all(entry["TargetEntityId"] == VEHICLE for entry in entries))
        self.assertTrue(all(entry["TargetSeatId"] == "REAR_LEFT" for entry in entries))

    def test_vehicle_waits_for_eva_and_the_dog(self):
        vehicle = self.row(self.vehicles, VEHICLE)
        self.assertEqual(vehicle["AssociatedPersonEntityIds"],
                         [DRIVER, PASSENGER, DOG])
        for entry in vehicle["Timeline"]:
            self.assertEqual(entry["PassengerEntityIds"], [PASSENGER, DOG])

    def test_dog_stays_with_the_test_group_at_grand(self):
        dog = self.row(self.people, DOG)
        self.assertEqual(dog["GroupLeaderEntityId"], DRIVER)
        self.assertEqual(dog["SocialGroupId"], "SYSTEM_TEST_PAIR")
        anchors = {entry["TargetAnchorId"] for entry in dog["Timeline"]}
        self.assertIn("GrandOutside_2_Entrance", anchors)
        self.assertIn("GrandOutside_4_Curb", anchors)


if __name__ == "__main__":
    unittest.main()
