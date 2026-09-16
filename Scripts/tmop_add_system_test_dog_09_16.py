"""Add one fictional dog to David and Eva's SYSTEM_TEST car scenario.

The dog remains a normal DT_TMOP_People row so the ordinary timeline, group,
vehicle-seat and inspection systems exercise the new dog presentation path.
The installer is idempotent and targets the current 09_14 table bundle.
"""

from __future__ import annotations

import copy
import io
import json
import os
from pathlib import Path
import tempfile
import zipfile


ROOT = Path(__file__).resolve().parents[1]
DATA_DIR = ROOT / "DataTables" / "09_14"
PEOPLE_ZIP = DATA_DIR / "DT_TMOP_People.zip"
PEOPLE_ENTRY = "DT_TMOP_People.json"
VEHICLES_JSON = DATA_DIR / "DT_TMOP_HistoricalVehicles.json"

DRIVER_ID = "SYSTEM_TEST_DRIVER"
PASSENGER_ID = "SYSTEM_TEST_PASSENGER"
DOG_ID = "SYSTEM_TEST_DOG"
VEHICLE_ID = "VEHICLE_SYSTEM_TEST_GRAND"
START_ANCHOR = "EnterSveavagenN_Car"
GRAND_CURB = "GrandOutside_4_Curb"
GRAND_ENTRANCE = "GrandOutside_2_Entrance"


def load_people() -> list[dict]:
    with zipfile.ZipFile(PEOPLE_ZIP) as archive:
        with archive.open(PEOPLE_ENTRY) as raw:
            return json.load(io.TextIOWrapper(raw, encoding="utf-16"))


def save_people(rows: list[dict]) -> None:
    with tempfile.NamedTemporaryFile(
        "wb", delete=False, dir=PEOPLE_ZIP.parent,
        prefix="DT_TMOP_People_", suffix=".zip.tmp"
    ) as raw:
        temporary_zip = Path(raw.name)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-16", delete=False, dir=PEOPLE_ZIP.parent,
        prefix="DT_TMOP_People_", suffix=".json.tmp"
    ) as handle:
        json.dump(rows, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
        temporary_json = Path(handle.name)
    os.utime(temporary_json, (315532800, 315532800))
    try:
        with zipfile.ZipFile(
            temporary_zip, "w", compression=zipfile.ZIP_DEFLATED,
            compresslevel=9
        ) as archive:
            archive.write(temporary_json, PEOPLE_ENTRY)
        temporary_zip.replace(PEOPLE_ZIP)
    finally:
        temporary_zip.unlink(missing_ok=True)
        temporary_json.unlink(missing_ok=True)


def load_utf16(path: Path) -> list[dict]:
    with path.open(encoding="utf-16") as handle:
        return json.load(handle)


def save_utf16(path: Path, rows: list[dict]) -> None:
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-16", delete=False, dir=path.parent,
        prefix=path.stem + "_", suffix=".tmp"
    ) as handle:
        json.dump(rows, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    temporary.replace(path)


def find_row(rows: list[dict], entity_id: str) -> dict:
    return next(row for row in rows if
                row.get("EntityId") == entity_id or
                row.get("VehicleId") == entity_id)


def replace_row(rows: list[dict], row: dict) -> None:
    identity = row.get("EntityId") or row.get("VehicleId")
    rows[:] = [existing for existing in rows if
               (existing.get("EntityId") or existing.get("VehicleId")) != identity]
    rows.append(row)


def timeline_entry(template: dict, entry_id: str, action: str,
                   minute: int, second: int) -> dict:
    entry = copy.deepcopy(template)
    entry.update({
        "EntryId": entry_id,
        "Action": action,
        "Usage": "Simulation",
        "Time": {"Hour": 23, "Minute": minute, "Second": second},
        "TimingMode": "Absolute",
        "SharedEventId": "None",
        "EventOffsetSeconds": 0,
        "bTimeIsArrival": False,
        "TravelSpeedOverrideCmPerSecond": 0,
        "LocationType": "Unknown",
        "TargetAnchorId": "None",
        "AnchorOffsetCm": {"X": 0, "Y": 0, "Z": 0},
        "TargetEntityId": "None",
        "TargetSeatId": "None",
        "ActivityState": "Idle",
        "LifeState": "Alive",
        "bTeleportDuringCatchUp": False,
        "bSupersedeActiveMovementWhenDue": False,
        "Confidence": "FictionalGameplay",
        "SourceReference": "SYSTEM_TEST – inte historiskt material",
        "Notes": "Fiktiv hund för bil-, sätes- och djurpresentationstest."
    })
    return entry


def make_dog(dog_base: dict, entry_template: dict) -> dict:
    dog = copy.deepcopy(dog_base)
    dog.update({
        "Name": DOG_ID,
        "EntityId": DOG_ID,
        "CategoryId": "SYSTEM_TEST",
        "FullName": "Bosse Testhund",
        "FirstName": "Bosse",
        "LastName": "Testhund",
        "Gender": "Male",
        "Species": "Dog",
        "Nationality": "",
        "Occupation": "Testhund",
        "HistoricalAddress": "SYSTEM_TEST – ingen historisk adress",
        "BirthYear": 1981,
        "AgeAtEvent": 5,
        "HeightCentimeters": 60,
        "GeneralSourceReference": "SYSTEM_TEST – inte ett historiskt djur",
        "Uppslag": [],
        "bPoliceInterviewed": False,
        "AgentTimelineSummary": "Åker med David och Eva i den turkosa testbilen.",
        "ObservationSummary": "Fiktiv hund för teknisk verifiering.",
        "AgentInfoSourceReference": "SYSTEM_TEST",
        "bSpawnInSimulation": True,
        "bMainCharacter": False,
        "bPrioritizeTimeline": True,
        "AssociatedVehicleIds": [VEHICLE_ID],
        "LeftHandItem": copy.deepcopy(dog_base["LeftHandItem"]),
        "RightHandItem": copy.deepcopy(dog_base["RightHandItem"]),
        "AdditionalCarriedItems": [],
        "SocialGroupId": "SYSTEM_TEST_PAIR",
        "GroupLeaderEntityId": DRIVER_ID,
        "GroupFormation": "SideBySide",
        "GroupFormationSpacingCm": 95,
        "bFollowGroupLeaderSchedule": False,
        "Dialog": {"BeforeShot": "", "AfterShot": ""},
        "AutomaticSpeech": [],
        "MeetingDialogues": [],
        "Notes": "FIKTIV SYSTEMTESTHUND. Får aldrig användas som historiskt belägg.",
        "AnimalPresentation": {
            "SkeletalMesh": "None",
            "AnimInstanceClass": "None",
            "bOverrideMeshRelativeTransform": False,
            "MeshRelativeTransform": {
                "Rotation": {"X": 0, "Y": 0, "Z": 0, "W": 1},
                "Translation": {"X": 0, "Y": 0, "Z": 0},
                "Scale3D": {"X": 1, "Y": 1, "Z": 1}
            },
            "CapsuleRadiusCm": 34,
            "CapsuleHalfHeightCm": 48,
            "NameLabelHeightCm": 75,
            "SpeechBubbleHeightCm": 135
        }
    })

    initial = timeline_entry(entry_template, DOG_ID + "_INITIAL",
                             "InitialPlacement", 0, 5)
    initial.update({
        "LocationType": "Anchor", "TargetAnchorId": START_ANCHOR,
        "AnchorOffsetCm": {"X": -330, "Y": 0, "Z": 0},
        "ActivityState": "Standing", "bTeleportDuringCatchUp": True
    })

    def enter(minute: int, second: int) -> dict:
        entry = timeline_entry(entry_template, DOG_ID + f"_ENTER_{minute:02d}{second:02d}",
                               "EnterVehicle", minute, second)
        entry.update({"LocationType": "VehicleSeat", "TargetEntityId": VEHICLE_ID,
                      "TargetSeatId": "REAR_LEFT", "ActivityState": "RidingVehicle"})
        return entry

    def exit_car(minute: int, second: int) -> dict:
        entry = timeline_entry(entry_template, DOG_ID + f"_EXIT_{minute:02d}{second:02d}",
                               "ExitVehicle", minute, second)
        entry.update({"LocationType": "VehicleSeat", "TargetEntityId": VEHICLE_ID,
                      "TargetSeatId": "REAR_LEFT", "ActivityState": "Standing"})
        return entry

    def move(suffix: str, minute: int, second: int, anchor: str,
             speed: int = 190) -> dict:
        entry = timeline_entry(entry_template, DOG_ID + "_" + suffix,
                               "MoveToAnchor", minute, second)
        entry.update({"bTimeIsArrival": True,
                      "TravelSpeedOverrideCmPerSecond": speed,
                      "LocationType": "Anchor", "TargetAnchorId": anchor,
                      "ActivityState": "FastWalking"})
        return entry

    def wait(suffix: str, minute: int, second: int, anchor: str) -> dict:
        entry = timeline_entry(entry_template, DOG_ID + "_" + suffix,
                               "Wait", minute, second)
        entry.update({"LocationType": "Anchor", "TargetAnchorId": anchor,
                      "ActivityState": "Standing"})
        return entry

    dog["Timeline"] = [
        initial,
        enter(0, 15),
        exit_car(2, 5),
        move("WALK_TO_GRAND", 2, 15, GRAND_ENTRANCE),
        wait("WAIT_DURING_DIALOGUE", 3, 20, GRAND_ENTRANCE),
        move("RETURN_TO_CAR", 3, 35, GRAND_CURB, 300),
        enter(3, 40),
        exit_car(4, 50),
        move("PLAYER_TAKEOVER_CLEARANCE", 5, 0, GRAND_ENTRANCE),
        wait("AVAILABLE_FOR_INSPECTION", 5, 5, GRAND_ENTRANCE),
    ]
    return dog


def update_vehicle(vehicle: dict) -> dict:
    result = copy.deepcopy(vehicle)
    associated = [DRIVER_ID, PASSENGER_ID, DOG_ID]
    result["AssociatedPersonEntityIds"] = associated
    for entry in result["Timeline"]:
        entry["DriverEntityId"] = DRIVER_ID
        entry["PassengerEntityIds"] = [PASSENGER_ID, DOG_ID]
    result["Notes"] = (
        "FIKTIV TURKOS TESTBIL. David, Eva och Bosse Testhund åker tillsammans. "
        "Efter 23:05 står bilen vid Grand för manuell körning."
    )
    return result


def validate(people: list[dict], vehicles: list[dict]) -> None:
    dog = find_row(people, DOG_ID)
    vehicle = find_row(vehicles, VEHICLE_ID)
    assert dog["Species"] == "Dog"
    assert dog["AssociatedVehicleIds"] == [VEHICLE_ID]
    assert dog["SocialGroupId"] == "SYSTEM_TEST_PAIR"
    assert [entry["Action"] for entry in dog["Timeline"]].count("EnterVehicle") == 2
    assert all(entry["TargetSeatId"] == "REAR_LEFT" for entry in dog["Timeline"]
               if entry["Action"] in ("EnterVehicle", "ExitVehicle"))
    assert vehicle["AssociatedPersonEntityIds"] == [DRIVER_ID, PASSENGER_ID, DOG_ID]
    assert all(entry["PassengerEntityIds"] == [PASSENGER_ID, DOG_ID]
               for entry in vehicle["Timeline"])


def install() -> None:
    people = load_people()
    vehicles = load_utf16(VEHICLES_JSON)
    driver = find_row(people, DRIVER_ID)
    dog_base = find_row(people, "DOG_EBE2354_MAN_COMPANION")
    vehicle = find_row(vehicles, VEHICLE_ID)
    entry_template = copy.deepcopy(driver["Timeline"][0])

    replace_row(people, make_dog(dog_base, entry_template))
    replace_row(vehicles, update_vehicle(vehicle))
    validate(people, vehicles)
    save_people(people)
    save_utf16(VEHICLES_JSON, vehicles)
    print("Installed SYSTEM_TEST_DOG with David and Eva in VEHICLE_SYSTEM_TEST_GRAND")


if __name__ == "__main__":
    install()
