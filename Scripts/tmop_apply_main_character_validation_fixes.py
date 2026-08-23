#!/usr/bin/env python3
"""Apply the 2026-08-23 main-character validation fixes to full TMOP JSON exports.

The script is intentionally idempotent and preserves every unrelated row.  It
accepts a directory containing the complete UTF-16 Unreal DataTable exports and
updates only DT_TMOP_People.json and DT_TMOP_HistoricalVehicles.json.
"""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path


SOURCE = (
    "TMOP_AnchorsAndSeats(5).json; TMOP_Lanes_Anchors(4).json; "
    "TimelineValidation_20260823_183509; TimelineValidation_20260823_201605"
)

PALME_ROUTE_ARRIVALS = {
    "kulturcirkeln": (23, 16, 9),
    "ABFhuset": (23, 16, 21),
    "KammakarXSvea_NW": (23, 16, 27),
    "ANCHOR_SHOT1_GUNNILA_DAHLEN_KANNER_SUSANNE_KARLSSON": (23, 16, 57),
    "KORVKIOSK_AF_KYRKOGATA_outside": (23, 18, 13),
    "AFKXSvea_NW": (23, 18, 38),
    "AFKXSvea": (23, 18, 53),
    "SariBoutique": (23, 19, 21),
    "SkandiaIngang": (23, 20, 21),
    "Dekorimaingang": (23, 21, 4),
    "Mordplatsen": (23, 21, 19),
}


def read_table(path: Path):
    with path.open("r", encoding="utf-16") as handle:
        return json.load(handle)


def write_table(path: Path, rows) -> None:
    text = json.dumps(rows, ensure_ascii=False, indent="\t")
    path.write_text(text.replace("\n", "\r\n") + "\r\n", encoding="utf-16")


def row_by_id(rows, field: str, entity_id: str):
    for row in rows:
        if row.get(field) == entity_id:
            return row
    raise KeyError(f"Missing {field}={entity_id}")


def time_value(hour: int, minute: int, second: int):
    return {"Hour": hour, "Minute": minute, "Second": second}


def seconds(entry) -> int:
    value = entry["Time"]
    return value["Hour"] * 3600 + value["Minute"] * 60 + value["Second"]


def replace_or_add(timeline, entry) -> None:
    timeline[:] = [item for item in timeline if item.get("EntryId") != entry["EntryId"]]
    timeline.append(entry)
    timeline.sort(key=seconds)


def identity_transform():
    return {
        "Rotation": {"X": 0, "Y": 0, "Z": 0, "W": 1},
        "Translation": {"X": 0, "Y": 0, "Z": 0},
        "Scale3D": {"X": 1, "Y": 1, "Z": 1},
    }


def make_vehicle_stop(
    row,
    entry_id: str,
    minute: int,
    second: int,
    anchor_id: str,
    driver_id: str,
    passengers: list[str],
    local_x_cm: float = 0.0,
):
    entry = copy.deepcopy(row["Timeline"][0])
    entry.update(
        {
            "EntryId": entry_id,
            "Action": "Stop",
            "Time": time_value(23, minute, second),
            "WorldTransform": identity_transform(),
            "PlacementMode": "Anchor",
            "PlacementAnchorId": anchor_id,
            "AnchorLocalOffset": identity_transform(),
            "OrderedLaneIds": [],
            "DriverEntityId": driver_id,
            "PassengerEntityIds": passengers,
            "Confidence": "Reconstructed",
            "SourceReference": SOURCE,
            "Notes": (
                "Deterministic timed stop added from validation. The vehicle is "
                "stopped and placed before its occupants leave, so a delayed or "
                "blocked lane run cannot turn into a drive-by or a long pedestrian walk."
            ),
        }
    )
    entry["AnchorLocalOffset"]["Translation"]["X"] = local_x_cm
    replace_or_add(row["Timeline"], entry)


def make_person_drive(template, entry_id: str, minute: int, second: int, lanes: list[str]):
    entry = copy.deepcopy(template)
    entry.update(
        {
            "EntryId": entry_id,
            "Action": "BeginDriving",
            "Time": time_value(23, minute, second),
            "TimingMode": "Absolute",
            "SharedEventId": "None",
            "EventOffsetSeconds": 0,
            "bTimeIsArrival": False,
            "LocationType": "VehicleSeat",
            "PassAnchorIds": [],
            "OrderedLaneIds": lanes,
            "VehicleRouteMode": "ManualLaneRoute",
            "DrivingDestinationAnchorId": "None",
            "bTeleportDuringCatchUp": False,
            "Confidence": "Reconstructed",
            "SourceReference": SOURCE,
            "Notes": (
                "Connected manual lane route verified against the exported lane graph. "
                "This replaces an automatic route that failed in validation."
            ),
        }
    )
    return entry


INGE_TO_ATM = [
    "SVEAVAGENN_001_R1",
    "X_SVEAVAGENN_001_R1_TO_SVEAVAGENN_002_R1_STRAIGHT",
    "SVEAVAGENN_002_R1",
    "X_SVEAVAGENN_002_R1_TO_SVEAVAGENN_003_R1_STRAIGHT",
    "SVEAVAGENN_003_R1",
    "X_SVEAVAGENN_003_R1_TO_SVEAVAGENN_004_R1_STRAIGHT",
    "SVEAVAGENN_004_R1",
    "X_SVEAVAGENN_004_R1_TO_SVEAVAGENN_005_R1_STRAIGHT",
    "SVEAVAGENN_005_R1",
    "X_SVEAVAGENN_005_R1_TO_SVEAVAGENN_006_R1_STRAIGHT",
    "SVEAVAGENN_006_R1",
    "X_SVEAVAGENN_006_R1_TO_TEGNERGATANE_004_R1_RIGHT",
    "TEGNERGATANE_004_R1",
    "X_TEGNERGATANE_004_R1_TO_LUNTMAKARGATANS_002_R1_RIGHT",
    "LUNTMAKARGATANS_002_R1",
    "X_LUNTMAKARGATANS_002_R1_TO_LUNTMAKARGATANS_003_R1_STRAIGHT",
    "LUNTMAKARGATANS_003_R1",
    "X_LUNTMAKARGATANS_003_R1_TO_KAMMAKAREGATANW_002_R1_RIGHT",
    "KAMMAKAREGATANW_002_R1",
    "X_KAMMAKAREGATANW_002_R1_TO_KAMMAKAREGATANW_003_R1_STRAIGHT",
    "KAMMAKAREGATANW_003_R1",
    "X_KAMMAKAREGATANW_003_R1_TO_KAMMAKAREGATANW_004_R1_STRAIGHT",
    "KAMMAKAREGATANW_004_R1",
    "X_KAMMAKAREGATANW_004_R1_TO_HOLLANDAREGATANN_004_R1_RIGHT",
    "HOLLANDAREGATANN_004_R1",
    "X_HOLLANDAREGATANN_004_R1_TO_TEGNERGATANE_002_R1_RIGHT",
    "TEGNERGATANE_002_R1",
    "X_TEGNERGATANE_002_R1_TO_TEGNERGATANE_003_R1_STRAIGHT",
    "TEGNERGATANE_003_R1",
    "X_TEGNERGATANE_003_R1_TO_SVEAVAGENS_002_R1_RIGHT",
    "SVEAVAGENS_002_R1",
    "X_SVEAVAGENS_002_R1_TO_SVEAVAGENS_003_R1_STRAIGHT",
    "SVEAVAGENS_003_R1",
    "X_SVEAVAGENS_003_R1_TO_SVEAVAGENS_004_R1_STRAIGHT",
    "SVEAVAGENS_004_R1",
    "X_SVEAVAGENS_004_R1_TO_TUNNELGATANW_001_R1_RIGHT",
    "TUNNELGATANW_001_R1",
]

INGE_ATM_TO_SCENE = [
    "TUNNELGATANW_001_R1",
    "X_TUNNELGATANW_001_R1_TO_TUNNELGATANW_002_R1_STRAIGHT",
    "TUNNELGATANW_002_R1",
    "X_TUNNELGATANW_002_R1_TO_HOLLANDAREGATANN_001_R1_RIGHT",
    "HOLLANDAREGATANN_001_R1",
    "X_HOLLANDAREGATANN_001_R1_TO_ADOLFFREDRIKSKYRKOGE_002_R1_RIGHT",
    "ADOLFFREDRIKSKYRKOGE_002_R1",
    "X_ADOLFFREDRIKSKYRKOGE_002_R1_TO_OLOFGATANS_001_R1_RIGHT",
    "OLOFGATANS_001_R1",
    "X_OLOFGATANS_001_R1_TO_TUNNELGATANE_003_R1_LEFT",
    "TUNNELGATANE_003_R1",
]

INGE_SCENE_TO_EXIT = [
    "TUNNELGATANE_003_R1",
    "X_TUNNELGATANE_003_R1_TO_SVEAVAGENN_004_R2_LEFT",
    "SVEAVAGENN_004_R2",
    "X_SVEAVAGENN_004_R2_TO_SVEAVAGENN_005_R2_STRAIGHT",
    "SVEAVAGENN_005_R2",
    "X_SVEAVAGENN_005_R2_TO_KAMMAKAREGATANW_003_R1_LEFT",
    "KAMMAKAREGATANW_003_R1",
    "X_KAMMAKAREGATANW_003_R1_TO_KAMMAKAREGATANW_004_R1_STRAIGHT",
    "KAMMAKAREGATANW_004_R1",
    "X_KAMMAKAREGATANW_004_R1_TO_HOLLANDAREGATANN_004_R1_RIGHT",
    "HOLLANDAREGATANN_004_R1",
    "X_HOLLANDAREGATANN_004_R1_TO_TEGNERGATANE_002_R1_RIGHT",
    "TEGNERGATANE_002_R1",
    "X_TEGNERGATANE_002_R1_TO_TEGNERGATANE_003_R1_STRAIGHT",
    "TEGNERGATANE_003_R1",
    "X_TEGNERGATANE_003_R1_TO_TEGNERGATANE_004_R1_STRAIGHT",
    "TEGNERGATANE_004_R1",
    "X_TEGNERGATANE_004_R1_TO_LUNTMAKARGATANS_002_R1_RIGHT",
    "LUNTMAKARGATANS_002_R1",
    "X_LUNTMAKARGATANS_002_R1_TO_LUNTMAKARGATANS_003_R1_STRAIGHT",
    "LUNTMAKARGATANS_003_R1",
    "X_LUNTMAKARGATANS_003_R1_TO_KAMMAKAREGATANW_002_R1_RIGHT",
    "KAMMAKAREGATANW_002_R1",
    "X_KAMMAKAREGATANW_002_R1_TO_SVEAVAGENS_003_R2_LEFT",
    "SVEAVAGENS_003_R2",
    "X_SVEAVAGENS_003_R2_TO_SVEAVAGENS_004_R2_STRAIGHT",
    "SVEAVAGENS_004_R2",
    "X_SVEAVAGENS_004_R2_TO_SVEAVAGENS_005_R2_STRAIGHT",
    "SVEAVAGENS_005_R2",
    "X_SVEAVAGENS_005_R2_TO_SVEAVAGENS_006_R2_STRAIGHT",
    "SVEAVAGENS_006_R2",
    "X_SVEAVAGENS_006_R2_TO_SVEAVAGENS_007_R2_STRAIGHT",
    "SVEAVAGENS_007_R2",
]

DELBOM_POST_SHOT = [
    "SVEAVAGENS_004_R2",
    "X_SVEAVAGENS_004_R2_TO_SVEAVAGENS_005_R2_STRAIGHT",
    "SVEAVAGENS_005_R2",
    "X_SVEAVAGENS_005_R2_TO_APELBERGSGATANE_003_R1_LEFT",
    "APELBERGSGATANE_003_R1",
    "X_APELBERGSGATANE_003_R1_TO_APELBERGSGATANE_004_R1_STRAIGHT",
    "APELBERGSGATANE_004_R1",
    "X_APELBERGSGATANE_004_R1_TO_APELBERGSGATANE_005_R1_STRAIGHT",
    "APELBERGSGATANE_005_R1",
    "X_APELBERGSGATANE_005_R1_TO_APELBERGSGATANE_006_R1_STRAIGHT",
    "APELBERGSGATANE_006_R1",
    "X_APELBERGSGATANE_006_R1_TO_NORRLANDSGATANS_001_R1_RIGHT",
    "NORRLANDSGATANS_001_R1",
    "X_NORRLANDSGATANS_001_R1_TO_KUNGSGATANW_003_R1_RIGHT",
    "KUNGSGATANW_003_R1",
    "X_KUNGSGATANW_003_R1_TO_SVEAVAGENN_002_R1_RIGHT",
    "SVEAVAGENN_002_R1",
    "X_SVEAVAGENN_002_R1_TO_SVEAVAGENN_003_R1_STRAIGHT",
    "SVEAVAGENN_003_R1",
]


def patch_people(rows) -> list[str]:
    changes = []
    for entity_id in ("OLOF_PALME", "LISBET_PALME"):
        palme = row_by_id(rows, "EntityId", entity_id)
        on_sveavagen_route = False
        for entry in palme["Timeline"]:
            if entry.get("EntryId", "").endswith("SANDINS_CONVERSATION_END"):
                entry["Time"] = time_value(23, 15, 58)
                entry["TimingMode"] = "Absolute"
                entry["SharedEventId"] = "None"
                entry["EventOffsetSeconds"] = 0
                entry["SourceReference"] = SOURCE
                entry["Notes"] = (
                    "Validation fix: the Sandins conversation ends at 23:15:58, "
                    "leaving enough real walking time for a calm route to Mordplatsen."
                )
            if entry.get("EntryId", "").endswith("BLENDER_06"):
                on_sveavagen_route = True
            if on_sveavagen_route and entry.get("Action") == "MoveToAnchor":
                arrival = PALME_ROUTE_ARRIVALS.get(entry.get("TargetAnchorId"))
                if arrival is not None:
                    entry["Time"] = time_value(*arrival)
                    entry["TimingMode"] = "Absolute"
                    entry["SharedEventId"] = "None"
                    entry["EventOffsetSeconds"] = 0
                    entry["bTimeIsArrival"] = True
                entry["TravelSpeedOverrideCmPerSecond"] = 100
                entry["bTeleportDuringCatchUp"] = False
                entry["SourceReference"] = SOURCE
                note = (
                    "Validation fix: the Palme pair walks this route side by side "
                    "at a calm 100 cm/s instead of rushing between overdue entries."
                )
                if note not in entry.get("Notes", ""):
                    entry["Notes"] = (entry.get("Notes", "") + " " + note).strip()
            if entry.get("TargetAnchorId") == "Mordplatsen" and on_sveavagen_route:
                break
        for entry in palme["Timeline"]:
            if entry.get("EntryId", "").endswith("LOOKS_INTO_SARI"):
                entry["Time"] = time_value(23, 19, 21)
                entry["TimingMode"] = "Absolute"
                entry["SharedEventId"] = "None"
                entry["EventOffsetSeconds"] = 0
                entry["SourceReference"] = SOURCE
    changes.append(
        "OLOF_PALME/LISBET_PALME: Sandins departure 23:15:58, "
        "calm 100 cm/s side-by-side route, Sari look and Mordplatsen 23:21:19"
    )

    # The configured runtime vehicles expose standard seat IDs. The validation
    # proved that the descriptive ambulance aliases were not present, leaving
    # Olof and ambulance staff outside even though the vehicle existed.
    ambulance_seats = {
        "OLOF_PALME": {"PATIENT_STRETCHER": "REAR_CENTER"},
        "CHRISTER_ERIKSSON_AMB": {"PATIENT_BENCH_1": "REAR_LEFT"},
        "EVA_LANTZ": {"PATIENT_BENCH_1": "REAR_LEFT"},
        "KENNETH_LAVRELL": {"PATIENT_BENCH_2": "REAR_RIGHT"},
    }
    for entity_id, aliases in ambulance_seats.items():
        person = row_by_id(rows, "EntityId", entity_id)
        for entry in person["Timeline"]:
            if entry.get("TargetEntityId") in ("AMBULANCE_A951", "AMBULANCE_912"):
                target_seat = entry.get("TargetSeatId")
                if target_seat in aliases:
                    entry["TargetSeatId"] = aliases[target_seat]
                    entry["SourceReference"] = SOURCE
                    entry["Notes"] = (
                        entry.get("Notes", "") +
                        " Validation fix: mapped the unavailable ambulance seat "
                        f"alias {target_seat} to runtime seat {aliases[target_seat]}."
                    ).strip()
    changes.append("A951/A912: mapped unavailable patient seat aliases to runtime seats")

    inge = row_by_id(rows, "EntityId", "INGE_MORELIUS_G")
    inge_routes = {
        "INGE_MORELIUS_DRIVES_FROM_SOUTH_SVEAVAGEN_TO_ATM_DROPOFF": INGE_TO_ATM,
        "INGE_MORELIUS_TURNS_BEFORE_OLOFSGATAN_AND_PARKS": INGE_ATM_TO_SCENE,
        "INGE_MORELIUS_DRIVES_GROUP_TOWARD_ALBATROSS": INGE_SCENE_TO_EXIT,
    }
    for entry in inge["Timeline"]:
        if entry.get("EntryId") in inge_routes:
            entry["OrderedLaneIds"] = inge_routes[entry["EntryId"]]
            entry["PassAnchorIds"] = []
            entry["VehicleRouteMode"] = "ManualLaneRoute"
            entry["DrivingDestinationAnchorId"] = "None"
            entry["SourceReference"] = SOURCE
            entry["Notes"] = (
                "Connected manual route verified against the lane export; automatic "
                "routing for this entry failed continuously in validation 18:35:09."
            )
    changes.append("INGE_MORELIUS_G: three connected manual vehicle routes")

    delbom = row_by_id(rows, "EntityId", "ANDERS_DELBOM")
    template = next(
        entry for entry in delbom["Timeline"]
        if entry.get("EntryId") == "ANDERS_DELBOM_VEHICLE_01"
    )
    post_shot = make_person_drive(
        template,
        "ANDERS_DELBOM_DRIVES_TO_POST_SHOT_STOP",
        21,
        36,
        DELBOM_POST_SHOT,
    )
    post_shot["DrivingDestinationAnchorId"] = "DELSBORN_TAXI_STOP_AFTER_SHOTS"
    replace_or_add(delbom["Timeline"], post_shot)
    changes.append("ANDERS_DELBOM: post-shot drive before exit")
    return changes


def patch_vehicles(rows) -> list[str]:
    changes = []

    specs = [
        (
            "VEHICLE_AKE_LARSSONS_BIL_FORD_ESCORT_GRAMETALLIC",
            "AKE_LARSSON_RED_LIGHT_GUARANTEED_STOP", 21, 14,
            "Ake_Larsson_RED_LIGHT", "AKE_LARSSON", ["AKE_LARSSON", "ANNE_HAGE"], 0,
        ),
        (
            "VEHICLE_ANDERS_DELBOMS_TAXI_BIL_JARFALLA_TAXI_MITSUBISHI_GALANT",
            "ANDERS_DELBOM_POST_SHOT_GUARANTEED_STOP", 22, 42,
            "DELSBORN_TAXI_STOP_AFTER_SHOTS", "ANDERS_DELBOM", ["ANDERS_DELBOM"], 0,
        ),
        (
            "VEHICLE_GLANTZ_BASEN_TAXI_E9979",
            "GLANTZ_BASEN_TAXI_GUARANTEED_SCENE_STOP", 22, 55,
            "HANS_JOHANSSON_CRIME_SCENE_STOP", "UNKNOWN_DRIVER_GLANTZ_TAXI_E9979",
            ["UNKNOWN_DRIVER_GLANTZ_TAXI_E9979", "GORAN_ISRAELSSON", "LENA_BASEN", "STEFAN_GLANTZ"], -500,
        ),
        (
            "VEHICLE_POLICE_RB2520", "RB2520_GUARANTEED_CRIME_SCENE_STOP", 24, 5,
            "Police2520_CrimeScene_Stop", "INGVAR_WIDEN", ["INGVAR_WIDEN", "GOSTA_SODERSTROM"], 0,
        ),
        (
            "VEHICLE_POLICE_PIKET1230", "PIKET1230_GUARANTEED_CRIME_SCENE_STOP", 26, 0,
            "Police1230_CrimeScene_Stop", "PER_BORG",
            ["PER_BORG", "CHRISTER_PERSSON", "LENA_LOHR", "KENT_BACKLUND", "ULF_HELLMAN"], 0,
        ),
        (
            "AMBULANCE_A951", "A951_GUARANTEED_CRIME_SCENE_STOP", 26, 10,
            "VEHICLE_AMBULANCE_951", "PETER_ANDERSSON_AMB",
            ["PETER_ANDERSSON_AMB", "CHRISTER_ERIKSSON_AMB"], 0,
        ),
        (
            "AMBULANCE_912", "AMBULANCE_912_GUARANTEED_CRIME_SCENE_STOP", 27, 55,
            "VEHICLE_AMBULANCE_912", "MARIA_DEGERMAN",
            ["MARIA_DEGERMAN", "KENNETH_LAVRELL", "EVA_LANTZ"], 0,
        ),
        (
            "VEHICLE_POLICE_RB1520", "RB1520_GUARANTEED_DAVID_BAGARES_STOP", 23, 20,
            "PoliceRB1520_DavidBagaresStop", "THOMAS_EKESATER",
            ["THOMAS_EKESATER", "CHRISTIAN_DALSGAARD"], 0,
        ),
    ]
    for vehicle_id, entry_id, minute, second, anchor, driver, passengers, offset in specs:
        row = row_by_id(rows, "VehicleId", vehicle_id)
        make_vehicle_stop(row, entry_id, minute, second, anchor, driver, passengers, offset)
        changes.append(f"{vehicle_id}: {anchor} at 23:{minute:02d}:{second:02d}")

    rb1210 = row_by_id(rows, "VehicleId", "VEHICLE_POLICE_RB1210")
    for entry_id, minute, second, anchor in [
        ("RB1210_MALMSKILLNAD_GUARANTEED_STOP", 25, 27, "PoliceRB1210_MalmskillnadBrunnStop"),
        ("RB1210_JOHANNES_GUARANTEED_STOP", 26, 47, "Johanneskyrkan"),
        ("RB1210_DAVID_BAGARES_GUARANTEED_STOP", 27, 41, "DavidBXJohannes"),
        ("RB1210_CRIME_SCENE_GUARANTEED_STOP", 29, 55, "PoliceRB1210_CrimeSceneStop"),
    ]:
        make_vehicle_stop(
            rb1210, entry_id, minute, second, anchor, "HANS_REHNSTAM",
            ["HANS_REHNSTAM", "LARS_CHRISTIANSSON"], 0,
        )
        changes.append(f"VEHICLE_POLICE_RB1210: {anchor} at 23:{minute:02d}:{second:02d}")
    return changes


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("database_directory", type=Path)
    args = parser.parse_args()
    people_path = args.database_directory / "DT_TMOP_People.json"
    vehicles_path = args.database_directory / "DT_TMOP_HistoricalVehicles.json"
    people = read_table(people_path)
    vehicles = read_table(vehicles_path)
    changes = patch_people(people) + patch_vehicles(vehicles)
    write_table(people_path, people)
    write_table(vehicles_path, vehicles)
    print(json.dumps({"changes": changes}, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
