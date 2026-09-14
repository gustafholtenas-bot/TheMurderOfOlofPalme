"""Add/replace the two-person Grand systems test in the 09_12 tables.

The rows are deliberately marked SYSTEM_TEST and FictionalGameplay. They are
runtime diagnostics, never historical evidence. The script is idempotent.
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
DATA_DIR = ROOT / "DataTables" / "09_12"
PEOPLE_ZIP = DATA_DIR / "DT_TMOP_People.zip"
PEOPLE_ENTRY = "DT_TMOP_People.json"
VEHICLES_JSON = DATA_DIR / "DT_TMOP_HistoricalVehicles.json"
EVENTS_JSON = DATA_DIR / "DT_TMOP_HistoricalEvents.json"

DRIVER_ID = "SYSTEM_TEST_DRIVER"
PASSENGER_ID = "SYSTEM_TEST_PASSENGER"
VEHICLE_ID = "VEHICLE_SYSTEM_TEST_GRAND"
MEETING_ID = "SYSTEM_TEST_GRAND_TIMED_DIALOGUE"

START_ANCHOR = "EnterSveavagenN_Car"
GRAND_CURB = "GrandOutside_4_Curb"
GRAND_ENTRANCE = "GrandOutside_2_Entrance"
GRAND_MANEUVER = "EAE46_FORD_SMALA_GRAND_PARKED"


def tmop_time(minute: int, second: int) -> dict:
    return {"Hour": 23, "Minute": minute, "Second": second}


def load_utf16(path: Path):
    with path.open("r", encoding="utf-16") as handle:
        return json.load(handle)


def save_utf16(path: Path, value) -> None:
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-16", delete=False, dir=path.parent,
        prefix=path.stem + "_", suffix=".tmp"
    ) as handle:
        json.dump(value, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
        temporary = Path(handle.name)
    temporary.replace(path)


def load_people() -> list[dict]:
    with zipfile.ZipFile(PEOPLE_ZIP) as archive:
        with archive.open(PEOPLE_ENTRY) as raw:
            prefix = raw.read(2)
            encoding = "utf-16" if prefix in (b"\xff\xfe", b"\xfe\xff") else "utf-16-le"
            stream = io.BytesIO(prefix + raw.read())
            with io.TextIOWrapper(stream, encoding=encoding) as handle:
                return json.load(handle)


def save_people(rows: list[dict]) -> None:
    with tempfile.NamedTemporaryFile(
        "wb", delete=False, dir=PEOPLE_ZIP.parent,
        prefix="DT_TMOP_People_", suffix=".zip.tmp"
    ) as raw:
        temporary = Path(raw.name)
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-16", delete=False, dir=PEOPLE_ZIP.parent,
        prefix="DT_TMOP_People_", suffix=".json.tmp"
    ) as handle:
        json.dump(rows, handle, ensure_ascii=False, indent=2)
        handle.write("\n")
        temporary_json = Path(handle.name)
    # A stable DOS-era ZIP timestamp makes repeated installs byte-identical.
    os.utime(temporary_json, (315532800, 315532800))
    try:
        with zipfile.ZipFile(
            temporary, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9
        ) as archive:
            archive.write(temporary_json, PEOPLE_ENTRY)
        temporary.replace(PEOPLE_ZIP)
    finally:
        temporary.unlink(missing_ok=True)
        temporary_json.unlink(missing_ok=True)


def find_row(rows: list[dict], name: str) -> dict:
    return next(row for row in rows if row.get("Name") == name)


def replace_row(rows: list[dict], row: dict) -> None:
    names = {row["Name"], row.get("EntityId"), row.get("VehicleId"), row.get("EventId")}
    names.discard(None)
    rows[:] = [
        existing for existing in rows
        if not names.intersection({
            existing.get("Name"), existing.get("EntityId"),
            existing.get("VehicleId"), existing.get("EventId")
        })
    ]
    rows.append(row)


def reset_person_entry(template: dict, entry_id: str, action: str,
                       minute: int, second: int) -> dict:
    entry = copy.deepcopy(template)
    entry.update({
        "EntryId": entry_id,
        "Action": action,
        "Usage": "Simulation",
        "Time": tmop_time(minute, second),
        "TimingMode": "Absolute",
        "SharedEventId": "None",
        "EventOffsetSeconds": 0,
        "bTimeIsArrival": False,
        "TravelSpeedOverrideCmPerSecond": 0,
        "LocationType": "Unknown",
        "TargetAnchorId": "None",
        "AnchorOffsetCm": {"X": 0, "Y": 0, "Z": 0},
        "AnchorOffsetSpace": "AnchorLocal",
        "AnchorReferenceMode": "RequiredInWorld",
        "PlannedAnchorDisplayName": "",
        "PlannedAnchorNotes": "",
        "PassAnchorIds": [],
        "TargetEntityId": "None",
        "TargetSeatId": "None",
        "TargetStopId": "None",
        "OrderedLaneIds": [],
        "VehicleRouteMode": "ManualLaneRoute",
        "DrivingDestinationAnchorId": "None",
        "VehicleStartDistanceAlongFirstLaneCm": 0,
        "TargetGroupId": "None",
        "SplitGroupDefinitions": [],
        "NewGroupLeaderEntityId": "None",
        "ActivityState": "Idle",
        "LifeState": "Alive",
        "ConversationTargetMode": "Automatic",
        "MeetingDialogueId": "None",
        "AnimationAsset": "None",
        "AnimationSlotName": "DefaultSlot",
        "AnimationPlayRate": 1,
        "AnimationLoopCount": 1,
        "AnimationBlendInSeconds": 0.15,
        "AnimationBlendOutSeconds": 0.2,
        "bTeleportDuringCatchUp": False,
        "bSupersedeActiveMovementWhenDue": False,
        "Confidence": "FictionalGameplay",
        "SourceReference": "SYSTEM_TEST – inte historiskt material",
        "Notes": "Automatiskt systemtest för person-, dialog- och fordonsfunktioner."
    })
    return entry


def person_initial(template: dict, person_id: str, lateral_cm: int) -> dict:
    entry = reset_person_entry(template, person_id + "_INITIAL", "InitialPlacement", 0, 5)
    entry.update({
        "LocationType": "Anchor",
        "TargetAnchorId": START_ANCHOR,
        "AnchorOffsetCm": {"X": -250, "Y": lateral_cm, "Z": 0},
        "ActivityState": "Standing",
        "bTeleportDuringCatchUp": True,
    })
    return entry


def enter_vehicle(template: dict, person_id: str, minute: int, second: int,
                  seat: str) -> dict:
    entry = reset_person_entry(template, person_id + "_ENTER_CAR", "EnterVehicle", minute, second)
    entry.update({
        "LocationType": "VehicleSeat",
        "TargetEntityId": VEHICLE_ID,
        "TargetSeatId": seat,
        "ActivityState": "RidingVehicle",
    })
    return entry


def exit_vehicle(template: dict, person_id: str, minute: int, second: int) -> dict:
    entry = reset_person_entry(template, person_id + f"_EXIT_CAR_{minute:02d}{second:02d}",
                               "ExitVehicle", minute, second)
    entry.update({
        "LocationType": "VehicleSeat",
        "TargetEntityId": VEHICLE_ID,
        "ActivityState": "Standing",
    })
    return entry


def move_to(template: dict, person_id: str, entry_suffix: str,
            minute: int, second: int, anchor: str, lateral_cm: int,
            speed: float = 190) -> dict:
    entry = reset_person_entry(template, person_id + "_" + entry_suffix,
                               "MoveToAnchor", minute, second)
    entry.update({
        "bTimeIsArrival": True,
        "TravelSpeedOverrideCmPerSecond": speed,
        "LocationType": "Anchor",
        "TargetAnchorId": anchor,
        "AnchorOffsetCm": {"X": 0, "Y": lateral_cm, "Z": 0},
        "ActivityState": "FastWalking" if speed >= 190 else "Walking",
    })
    return entry


def look_at_person(template: dict, person_id: str, target_id: str) -> dict:
    entry = reset_person_entry(template, person_id + "_LOOK_AT_PARTNER",
                               "LookAtAnchor", 2, 25)
    entry.update({
        "TargetEntityId": target_id,
        "ConversationTargetMode": "SpecificPerson",
        "ActivityState": "Interacting",
    })
    return entry


def unique_animation(template: dict, person_id: str, target_id: str,
                     asset: str) -> list[dict]:
    play = reset_person_entry(template, person_id + "_PLAY_TEST_ANIMATION",
                              "PlayUniqueAnimation", 2, 30)
    play.update({
        "TargetEntityId": target_id,
        "ConversationTargetMode": "SpecificPerson",
        "AnimationAsset": asset,
        "AnimationSlotName": "DefaultSlot",
        "AnimationPlayRate": 1,
        "AnimationLoopCount": 2,
        "ActivityState": "Interacting",
    })
    stop = reset_person_entry(template, person_id + "_STOP_TEST_ANIMATION",
                              "StopUniqueAnimation", 2, 40)
    return [play, stop]


def meeting_marker(template: dict, person_id: str, target_id: str) -> dict:
    entry = reset_person_entry(template, person_id + "_TIMED_DIALOGUE",
                               "MeetingDialogue", 3, 0)
    entry.update({
        "TargetAnchorId": GRAND_ENTRANCE,
        "TargetEntityId": target_id,
        "ConversationTargetMode": "SpecificPerson",
        "MeetingDialogueId": MEETING_ID,
        "ActivityState": "Interacting",
    })
    return entry


def wait_entry(template: dict, person_id: str, minute: int, second: int,
               anchor: str, suffix: str) -> dict:
    entry = reset_person_entry(template, person_id + "_" + suffix,
                               "Wait", minute, second)
    entry.update({
        "LocationType": "Anchor",
        "TargetAnchorId": anchor,
        "ActivityState": "Standing",
    })
    return entry


def speech(line_id: str, minute: int, second: int, text: str) -> dict:
    return {
        "LineId": line_id,
        "Time": tmop_time(minute, second),
        "TimingMode": "Absolute",
        "SharedEventId": "None",
        "OffsetSeconds": 0,
        "Text": text,
        "VoiceOver": "None",
        "DisplayDurationOverrideSeconds": 5,
        "Confidence": "FictionalGameplay",
        "SourceReference": "SYSTEM_TEST",
        "Notes": "Visuell kontroll: repliken ska synas som pratbubbla."
    }


def make_person(base: dict, person_template: dict, person_id: str,
                display_name: str, gender: str, lateral_cm: int,
                seat: str, partner_id: str, animation_asset: str) -> dict:
    row = copy.deepcopy(base)
    row.update({
        "Name": person_id,
        "EntityId": person_id,
        "CategoryId": "SYSTEM_TEST",
        "FullName": display_name,
        "FirstName": display_name.split()[0],
        "LastName": "Testperson",
        "Gender": gender,
        "Nationality": "Svensk",
        "Occupation": "Systemtestare",
        "HistoricalAddress": "SYSTEM_TEST – ingen historisk adress",
        "ReferenceImage": "None",
        "BirthYear": 1950 if gender == "Male" else 1955,
        "AgeAtEvent": 36 if gender == "Male" else 31,
        "GeneralSourceReference": "SYSTEM_TEST – inte en historisk person",
        "EvidenceIcon": "Automatic",
        "Uppslag": [],
        "bPoliceInterviewed": False,
        "AgentTimelineSummary": "Testar bilresa från norra Sveavägen, Grand, animationer och dialog.",
        "ObservationSummary": "Fiktiv och endast avsedd för teknisk verifiering.",
        "AgentInfoSourceReference": "SYSTEM_TEST",
        "bSpawnInSimulation": True,
        "bMainCharacter": False,
        "bPrioritizeTimeline": True,
        "AssociatedVehicleIds": [VEHICLE_ID],
        "AdditionalCarriedItems": [],
        "SocialGroupId": "SYSTEM_TEST_PAIR",
        "GroupLeaderEntityId": DRIVER_ID,
        "GroupFormation": "SideBySide",
        "GroupFormationSpacingCm": 120,
        "bFollowGroupLeaderSchedule": False,
        "Dialog": {
            "BeforeShot": "[SYSTEMTEST – FÖRE SKOTTET] E-dialogen fungerar. Kontrollera text, kamera, stängning och att rätt spelare äger dialogen.",
            "AfterShot": "[SYSTEMTEST – EFTER SKOTTET] Tidsgrenen efter skottet fungerar. Kontrollera text, kamera och multiplayerfokus."
        },
        "AutomaticSpeech": [
            speech(person_id + "_READY", 0, 8,
                   f"[SYSTEMTEST] {display_name}: jag syns och väntar på testbilen."),
            speech(person_id + "_ARRIVED", 2, 18,
                   f"[SYSTEMTEST] {display_name}: ankomst och urstigning vid Grand fungerar.")
        ],
        "MeetingDialogues": [],
        "Notes": "FIKTIV SYSTEMTESTPERSON. Får aldrig användas som historiskt belägg."
    })

    timeline = [
        person_initial(person_template, person_id, lateral_cm),
        enter_vehicle(person_template, person_id, 0, 15, seat),
        exit_vehicle(person_template, person_id, 2, 5),
        move_to(person_template, person_id, "WALK_TO_GRAND", 2, 15,
                GRAND_ENTRANCE, lateral_cm),
        look_at_person(person_template, person_id, partner_id),
    ]
    timeline.extend(unique_animation(
        person_template, person_id, partner_id, animation_asset))
    timeline.extend([
        meeting_marker(person_template, person_id, partner_id),
        wait_entry(person_template, person_id, 3, 20,
                   GRAND_ENTRANCE, "END_TIMED_DIALOGUE"),
        move_to(person_template, person_id, "RETURN_TO_CAR", 3, 35,
                GRAND_CURB, lateral_cm, speed=300),
        enter_vehicle(person_template, person_id, 3, 40, seat),
        exit_vehicle(person_template, person_id, 4, 50),
        move_to(person_template, person_id, "PLAYER_TAKEOVER_CLEARANCE", 5, 0,
                GRAND_ENTRANCE, lateral_cm),
        wait_entry(person_template, person_id, 5, 5,
                   GRAND_ENTRANCE, "AVAILABLE_FOR_MANUAL_DIALOG")
    ])
    row["Timeline"] = timeline
    return row


def reset_vehicle_entry(template: dict, entry_id: str, action: str,
                        minute: int, second: int) -> dict:
    entry = copy.deepcopy(template)
    entry.update({
        "EntryId": entry_id,
        "Action": action,
        "RouteSegmentName": "",
        "Time": tmop_time(minute, second),
        "TimingMode": "Absolute",
        "SharedEventId": "None",
        "EventOffsetSeconds": 0,
        "bTimeIsArrival": False,
        "bUseExplicitDepartureTime": False,
        "DepartureTimingMode": "Absolute",
        "DepartureTime": tmop_time(minute, second),
        "DepartureSharedEventId": "None",
        "DepartureOffsetSeconds": 0,
        "CruiseSpeedOverrideKmh": 0,
        "DrivingPreset": "AutomaticFromTimeline",
        "PlacementMode": "WorldTransform",
        "PlacementAnchorId": "None",
        "OffscreenTransferDurationSeconds": 60,
        "bUseStopDuration": False,
        "StopDurationSeconds": 10,
        "OrderedLaneIds": [],
        "bAutoStartFromVehicleTimeline": True,
        "VehicleRouteMode": "ManualLaneRoute",
        "AnchorManeuverCurveStrength": 0.5,
        "bAnchorManeuverReverse": False,
        "AnchorManeuverTurn": "Automatic",
        "AnchorManeuverRadiusCm": 0,
        "bStopAtViaAnchors": False,
        "RouteStartAnchorId": "None",
        "RouteStartLaneId": "None",
        "RouteStartDistanceAlongFirstLaneCm": 0,
        "RouteDestinationAnchorId": "None",
        "RouteDestinationLaneId": "None",
        "RouteViaAnchorIds": [],
        "RouteViaLaneIds": [],
        "bIgnoreOneWayRestrictions": False,
        "bRunRedLights": False,
        "bWaitForListedOccupants": False,
        "BoardingBufferSeconds": 4,
        "DriverEntityId": DRIVER_ID,
        "PassengerEntityIds": [PASSENGER_ID],
        "Confidence": "FictionalGameplay",
        "SourceReference": "SYSTEM_TEST – inte historiskt material",
        "Notes": "Fiktiv fordonsrad för deterministisk funktionskontroll."
    })
    return entry


def arrival_drive(template: dict) -> dict:
    entry = reset_vehicle_entry(template, VEHICLE_ID + "_DRIVE_NORTH_TO_GRAND",
                                "BeginDriving", 2, 0)
    entry.update({
        "RouteSegmentName": "SYSTEMTEST: norra Sveavägen till Grand",
        "bTimeIsArrival": True,
        "bUseExplicitDepartureTime": True,
        "DepartureTime": tmop_time(0, 30),
        "CruiseSpeedOverrideKmh": 24,
        "DrivingPreset": "NormalCity",
        "OrderedLaneIds": [
            "SVEAVAGENS_001_R2",
            "X_SVEAVAGENS_001_R2_TO_SVEAVAGENS_002_R2_STRAIGHT",
            "SVEAVAGENS_002_R2",
        ],
        "VehicleRouteMode": "ManualLaneRoute",
        "RouteStartAnchorId": START_ANCHOR,
        "RouteDestinationAnchorId": GRAND_CURB,
        "bWaitForListedOccupants": True,
        "BoardingBufferSeconds": 5,
    })
    return entry


def anchor_maneuver(template: dict, entry_id: str, arrival_minute: int,
                    arrival_second: int, departure_minute: int,
                    departure_second: int, start: str, destination: str,
                    reverse: bool, turn: str) -> dict:
    entry = reset_vehicle_entry(template, entry_id, "BeginDriving",
                                arrival_minute, arrival_second)
    entry.update({
        "RouteSegmentName": "SYSTEMTEST: backande ankarmanöver" if reverse
                            else "SYSTEMTEST: kurvad ankarmanöver",
        "bTimeIsArrival": True,
        "bUseExplicitDepartureTime": True,
        "DepartureTime": tmop_time(departure_minute, departure_second),
        "CruiseSpeedOverrideKmh": 6,
        "DrivingPreset": "Parking",
        "VehicleRouteMode": "AnchorManeuver",
        "AnchorManeuverCurveStrength": 0.8,
        "bAnchorManeuverReverse": reverse,
        "AnchorManeuverTurn": turn,
        "AnchorManeuverRadiusCm": 450,
        "RouteStartAnchorId": start,
        "RouteDestinationAnchorId": destination,
        "bWaitForListedOccupants": True,
        "BoardingBufferSeconds": 2,
    })
    return entry


def placement(template: dict, entry_id: str, action: str,
              minute: int, second: int, anchor: str) -> dict:
    entry = reset_vehicle_entry(template, entry_id, action, minute, second)
    entry.update({
        "PlacementMode": "Anchor",
        "PlacementAnchorId": anchor,
    })
    return entry


def make_vehicle(base: dict, vehicle_template: dict) -> dict:
    row = copy.deepcopy(base)
    row.update({
        "Name": VEHICLE_ID,
        "VehicleId": VEHICLE_ID,
        "DisplayName": "SYSTEMTEST – turkos Volvo vid Grand",
        "CategoryId": "SYSTEM_TEST",
        "VehicleCategory": "PassengerCar",
        "bFleeingVehicle": False,
        "RegistrationStatus": "Unknown",
        "RegistrationOrigin": "Unknown",
        "RegistrationNumber": "",
        "RegistrationNotes": "Fiktiv testbil utan registreringsnummer.",
        "ModelData": "/Script/TMOPEngine.TMOPVehicleModelData'/Game/TMOP/Vehicles/Cars/Volvo240.Volvo240'",
        "bOverrideBodyColor": True,
        "BodyColor": {"R": 0.02, "G": 0.8, "B": 0.8, "A": 1},
        "AdditionalAccessories": [],
        "VehicleClass": "/Script/CoreUObject.Class'/Script/TMOPEngine.TMOPConfiguredVehicle'",
        "AssociatedPersonEntityIds": [DRIVER_ID, PASSENGER_ID],
        "PrimaryPersonEntityId": DRIVER_ID,
        "KnownDriverEntityId": DRIVER_ID,
        "bSpawnInSimulation": True,
        "bPrioritizeTimeline": True,
        "Confidence": "FictionalGameplay",
        "SourceReference": "SYSTEM_TEST – inte historiskt material",
        "EvidenceIcon": "Automatic",
        "Notes": "FIKTIV TURKOS TESTBIL. Efter 23:05 står den vid Grand utan fler fordonsrader så spelaren kan ta över och prova manuell körning."
    })

    spawn = placement(vehicle_template, VEHICLE_ID + "_SPAWN", "Spawn", 0, 0,
                      START_ANCHOR)
    stop_grand = placement(vehicle_template, VEHICLE_ID + "_STOP_AT_GRAND",
                           "Stop", 2, 0, GRAND_CURB)
    stop_grand.update({"bUseStopDuration": True, "StopDurationSeconds": 105})
    stop_maneuver = placement(vehicle_template, VEHICLE_ID + "_STOP_AFTER_CURVE",
                              "Stop", 4, 10, GRAND_MANEUVER)
    stop_maneuver.update({"bUseStopDuration": True, "StopDurationSeconds": 10})
    final_park = placement(vehicle_template, VEHICLE_ID + "_PARK_FOR_PLAYER",
                           "Park", 4, 45, GRAND_CURB)
    final_park.update({"bUseStopDuration": False, "StopDurationSeconds": 0})

    row["Timeline"] = [
        spawn,
        arrival_drive(vehicle_template),
        stop_grand,
        anchor_maneuver(vehicle_template, VEHICLE_ID + "_CURVED_MANEUVER",
                        4, 10, 3, 45, GRAND_CURB, GRAND_MANEUVER, False, "Right"),
        stop_maneuver,
        anchor_maneuver(vehicle_template, VEHICLE_ID + "_REVERSE_MANEUVER",
                        4, 45, 4, 20, GRAND_MANEUVER, GRAND_CURB, True, "Left"),
        final_park,
    ]
    return row


def make_meeting_definition() -> dict:
    texts = [
        (DRIVER_ID, "[TIDSDIALOG 1/4] Föraren talar. Kontrollera pratbubblan och att passageraren tittar hit."),
        (PASSENGER_ID, "[TIDSDIALOG 2/4] Passageraren svarar. Bubblan ska nu flytta till rätt person."),
        (DRIVER_ID, "[TIDSDIALOG 3/4] Dialogen följer simulationstiden och får inte spelas dubbelt."),
        (PASSENGER_ID, "[TIDSDIALOG 4/4] Testet är klart. Båda ska återgå till vanlig styrning.")
    ]
    lines = []
    for index, (speaker, text) in enumerate(texts):
        lines.append({
            "LineId": f"{MEETING_ID}_LINE_{index + 1}",
            "SpeakerEntityId": speaker,
            "OffsetSeconds": index * 5,
            "Text": text,
            "VoiceOver": "None",
            "DisplayDurationOverrideSeconds": 4.5,
        })
    return {
        "DialogueId": MEETING_ID,
        "DisplayName": "SYSTEMTEST – synlig tidsstyrd dialog vid Grand",
        "SharedEventId": MEETING_ID,
        "EventOffsetSeconds": 0,
        "AnchorId": GRAND_ENTRANCE,
        "ParticipantEntityIds": [DRIVER_ID, PASSENGER_ID],
        "Lines": lines,
        "Confidence": "FictionalGameplay",
        "SourceReference": "SYSTEM_TEST",
        "Notes": "Teknisk visualisering med fyra växlande pratbubblor; inte historisk dialog."
    }


def make_event(base: dict) -> dict:
    event = copy.deepcopy(base)
    event.update({
        "Name": MEETING_ID,
        "EventId": MEETING_ID,
        "DisplayName": "SYSTEMTEST – tidsdialog vid Grand",
        "TimingMode": "Absolute",
        "HistoricalLock": "Free",
        "Confidence": "FictionalGameplay",
        "AbsoluteTime": tmop_time(3, 0),
        "EarliestTime": tmop_time(3, 0),
        "PreferredTime": tmop_time(3, 0),
        "LatestTime": tmop_time(3, 0),
        "TriggerEventId": "None",
        "MinimumDelaySeconds": 0,
        "PreferredDelaySeconds": 0,
        "MaximumDelaySeconds": 0,
        "SourceId": "SYSTEM_TEST",
        "Notes": "Fiktiv teknisk händelse som driver den synliga testdialogen."
    })
    return event


def validate(people: list[dict], vehicles: list[dict], events: list[dict]) -> None:
    driver = find_row(people, DRIVER_ID)
    passenger = find_row(people, PASSENGER_ID)
    vehicle = find_row(vehicles, VEHICLE_ID)
    event = find_row(events, MEETING_ID)
    assert len({row.get("Name") for row in people}) == len(people)
    assert vehicle["AssociatedPersonEntityIds"] == [DRIVER_ID, PASSENGER_ID]
    assert vehicle["Timeline"][1]["VehicleRouteMode"] == "ManualLaneRoute"
    assert len(vehicle["Timeline"][1]["OrderedLaneIds"]) == 3
    assert vehicle["Timeline"][1]["RouteDestinationAnchorId"] == GRAND_CURB
    assert any(entry["VehicleRouteMode"] == "AnchorManeuver"
               and entry["bAnchorManeuverReverse"] for entry in vehicle["Timeline"])
    assert event["AbsoluteTime"] == tmop_time(3, 0)
    assert driver["MeetingDialogues"][0]["DialogueId"] == MEETING_ID
    assert len(driver["MeetingDialogues"][0]["Lines"]) == 4
    for row in (driver, passenger):
        assert row["CategoryId"] == "SYSTEM_TEST"
        assert row["bSpawnInSimulation"] is True
        assert isinstance(row["LeftHandItem"], dict)
        assert isinstance(row["RightHandItem"], dict)
        assert row["ReferenceImage"] == "None"
        assert any(entry["Action"] == "PlayUniqueAnimation" for entry in row["Timeline"])
        assert any(entry["Action"] == "MeetingDialogue" for entry in row["Timeline"])


def install() -> None:
    for path in (PEOPLE_ZIP, VEHICLES_JSON, EVENTS_JSON):
        if not path.exists():
            raise FileNotFoundError(path)

    people = load_people()
    vehicles = load_utf16(VEHICLES_JSON)
    events = load_utf16(EVENTS_JSON)

    male_base = find_row(people, "ANDERS_DELBOM")
    female_base = find_row(people, "ANN_CHARLOTT_HOLMGREN")
    person_template = copy.deepcopy(male_base["Timeline"][0])
    vehicle_base = find_row(vehicles, "VEHICLE_ANNETTE_KOHUTS_BIL")
    vehicle_template = copy.deepcopy(vehicle_base["Timeline"][0])

    driver = make_person(
        male_base, person_template, DRIVER_ID, "David Testperson", "Male", -90,
        "FRONT_LEFT", PASSENGER_ID,
        "/Script/Engine.AnimSequence'/Game/TMOP/Animation/Animations/standing/A_TMOP_StandingTalking.A_TMOP_StandingTalking'")
    passenger = make_person(
        female_base, person_template, PASSENGER_ID, "Eva Testperson", "Female", 90,
        "FRONT_RIGHT", DRIVER_ID,
        "/Script/Engine.AnimSequence'/Game/TMOP/Animation/Animations/standing/A_TMOP_StandingPhoneRadio.A_TMOP_StandingPhoneRadio'")
    driver["MeetingDialogues"] = [make_meeting_definition()]
    vehicle = make_vehicle(vehicle_base, vehicle_template)
    event = make_event(events[0])

    replace_row(people, driver)
    replace_row(people, passenger)
    replace_row(vehicles, vehicle)
    replace_row(events, event)
    validate(people, vehicles, events)

    save_people(people)
    save_utf16(VEHICLES_JSON, vehicles)
    save_utf16(EVENTS_JSON, events)
    print("Installed SYSTEM_TEST_DRIVER, SYSTEM_TEST_PASSENGER, VEHICLE_SYSTEM_TEST_GRAND")
    print("Timed dialogue event:", MEETING_ID, "at 23:03:00")
    print("People rows:", len(people), "Vehicle rows:", len(vehicles), "Event rows:", len(events))


if __name__ == "__main__":
    install()
