#!/usr/bin/env python3
"""Migrate missing legacy driver BeginDriving rows into the vehicle table.

The script is deliberately additive: it never replaces an existing vehicle
timeline entry. Rows whose route/timing data changed since the supplied
baseline are protected in full. Only successfully migrated person entries are
removed from the output People table.
"""

from __future__ import annotations

import argparse
import copy
import json
import re
import sys
import zipfile
from pathlib import Path
from typing import Any


DRIVING_ACTIONS = {"BeginDriving", "EnterTrafficRoute"}
ROUTE_SENSITIVE_FIELDS = {
    "EntryId", "Action", "Time", "TimingMode", "SharedEventId",
    "EventOffsetSeconds", "bTimeIsArrival", "bUseExplicitDepartureTime",
    "DepartureTimingMode", "DepartureTime", "DepartureSharedEventId",
    "DepartureOffsetSeconds", "CruiseSpeedOverrideKmh", "DrivingPreset",
    "WorldTransform", "PlacementMode", "PlacementAnchorId",
    "AnchorLocalOffset", "OffscreenTransferDurationSeconds",
    "OrderedLaneIds", "RouteStartAnchorId", "RouteStartLaneId",
    "RouteStartDistanceAlongFirstLaneCm", "RouteDestinationAnchorId",
    "RouteDestinationLaneId", "RouteViaAnchorIds", "RouteViaLaneIds",
    "VehicleRouteMode", "bAutoStartFromVehicleTimeline",
    "bIgnoreOneWayRestrictions", "bRunRedLights",
    "bWaitForListedOccupants", "BoardingBufferSeconds", "DriverEntityId",
    "PassengerEntityIds",
}

# A historical typo exists in the 09_04 People export. Keep aliases explicit;
# fuzzy matching vehicle IDs would be unsafe in a 400-row evidence table.
KNOWN_VEHICLE_ALIASES = {
    "VEHICLE_BENGT_PALME_MERCEDES_GUL":
        "VEHICLE_BENGT_PALM_MERCEDES_GUL",
}


def read_json(path: Path) -> tuple[list[dict[str, Any]], str]:
    if path.suffix.lower() == ".zip":
        with zipfile.ZipFile(path) as archive:
            names = [name for name in archive.namelist()
                     if name.lower().endswith(".json")]
            if len(names) != 1:
                raise ValueError(f"{path}: expected exactly one JSON file")
            raw = archive.read(names[0])
    else:
        raw = path.read_bytes()
    encoding = "utf-16" if raw[:2] in (b"\xff\xfe", b"\xfe\xff") else "utf-8-sig"
    data = json.loads(raw.decode(encoding))
    if not isinstance(data, list):
        raise ValueError(f"{path}: Unreal DataTable export must be a JSON array")
    return data, encoding


def write_utf16_json(path: Path, rows: list[dict[str, Any]]) -> None:
    text = json.dumps(rows, ensure_ascii=False, indent=2) + "\n"
    path.write_text(text, encoding="utf-16", newline="\r\n")


def write_people_zip(path: Path, rows: list[dict[str, Any]]) -> None:
    payload = (json.dumps(rows, ensure_ascii=False, indent=2) + "\n").encode("utf-16")
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED,
                         compresslevel=9) as archive:
        archive.writestr("DT_TMOP_People.json", payload)


def timeline_changed(old: dict[str, Any], new: dict[str, Any]) -> bool:
    old_timeline = old.get("Timeline") or []
    new_timeline = new.get("Timeline") or []
    if len(old_timeline) != len(new_timeline):
        return True
    for old_entry, new_entry in zip(old_timeline, new_timeline):
        shared = ROUTE_SENSITIVE_FIELDS & old_entry.keys() & new_entry.keys()
        if any(old_entry[field] != new_entry[field] for field in shared):
            return True
    return False


def protected_vehicle_ids(
    baseline_rows: list[dict[str, Any]],
    current_rows: list[dict[str, Any]],
) -> set[str]:
    baseline = {row.get("VehicleId"): row for row in baseline_rows}
    protected: set[str] = set()
    for row in current_rows:
        vehicle_id = row.get("VehicleId")
        old = baseline.get(vehicle_id)
        if old is None or timeline_changed(old, row):
            protected.add(vehicle_id)
    return protected


def time_signature(entry: dict[str, Any]) -> tuple[Any, ...]:
    time = entry.get("Time") or {}
    return (
        entry.get("TimingMode", "Absolute"),
        entry.get("SharedEventId", "None"),
        entry.get("EventOffsetSeconds", 0),
        time.get("Hour", 0), time.get("Minute", 0), time.get("Second", 0),
    )


def find_existing_drive_match(
    person_entry: dict[str, Any],
    vehicle: dict[str, Any],
    driver_id: str,
) -> dict[str, Any] | None:
    candidates = [
        entry for entry in vehicle.get("Timeline") or []
        if entry.get("Action") in DRIVING_ACTIONS and
        entry.get("DriverEntityId", "None") in (None, "", "None", driver_id)
    ]
    for entry in candidates:
        if time_signature(person_entry) == time_signature(entry):
            return entry
    old_route = person_entry.get("OrderedLaneIds") or []
    if old_route:
        for entry in candidates:
            if old_route == (entry.get("OrderedLaneIds") or []):
                return entry
    return None


def normalized_vehicle_route_mode(entry: dict[str, Any]) -> str:
    existing_mode = entry.get("VehicleRouteMode")
    if existing_mode:
        return existing_mode
    if entry.get("OrderedLaneIds"):
        return "ManualLaneRoute"
    if entry.get("RouteDestinationAnchorId") not in (None, "", "None"):
        return "AutomaticToAnchor"
    return "ManualLaneRoute"


def has_usable_route(entry: dict[str, Any]) -> bool:
    mode = normalized_vehicle_route_mode(entry)
    if mode == "ManualLaneRoute":
        return bool(entry.get("OrderedLaneIds"))
    return entry.get("RouteDestinationAnchorId") not in (None, "", "None")


def adopt_existing_vehicle_entry(
    entry: dict[str, Any],
    person_entry: dict[str, Any],
    driver_id: str,
) -> None:
    """Transfer ownership without changing the existing route or timing."""
    entry["bAutoStartFromVehicleTimeline"] = True
    entry["VehicleRouteMode"] = normalized_vehicle_route_mode(entry)
    entry.setdefault(
        "RouteStartDistanceAlongFirstLaneCm",
        person_entry.get("VehicleStartDistanceAlongFirstLaneCm", 0),
    )
    if entry.get("DriverEntityId") in (None, "", "None"):
        entry["DriverEntityId"] = driver_id
    notes = entry.get("Notes") or ""
    marker = "[VEHICLE_TIMELINE_OWNS_DRIVE]"
    if marker not in notes:
        entry["Notes"] = f"{marker} {notes}".rstrip()


def safe_id(value: str) -> str:
    return re.sub(r"[^A-Z0-9_]+", "_", value.upper()).strip("_")


def identity_transform() -> dict[str, Any]:
    return {
        "Rotation": {"X": 0, "Y": 0, "Z": 0, "W": 1},
        "Translation": {"X": 0, "Y": 0, "Z": 0},
        "Scale3D": {"X": 1, "Y": 1, "Z": 1},
    }


def passenger_ids(vehicle: dict[str, Any], driver_id: str) -> list[str]:
    result: list[str] = []
    for person_id in vehicle.get("AssociatedPersonEntityIds") or []:
        if person_id not in (None, "", "None", driver_id) and person_id not in result:
            result.append(person_id)
    return result


def make_vehicle_entry(
    vehicle: dict[str, Any],
    person_entry: dict[str, Any],
    driver_id: str,
    existing_ids: set[str],
) -> dict[str, Any]:
    base_id = safe_id(
        f"{vehicle['VehicleId']}_{person_entry.get('EntryId', 'DRIVE')}_MIGRATED")
    entry_id = base_id
    suffix = 2
    while entry_id in existing_ids:
        entry_id = f"{base_id}_{suffix}"
        suffix += 1
    existing_ids.add(entry_id)

    lanes = copy.deepcopy(person_entry.get("OrderedLaneIds") or [])
    route_mode = person_entry.get("VehicleRouteMode") or "ManualLaneRoute"
    destination = person_entry.get("DrivingDestinationAnchorId", "None")
    # Older rows occasionally called an anchor-routed drive "Manual" despite
    # containing no lane list. The former person runtime effectively treated
    # the destination as authoritative, so preserve that intent explicitly.
    if (route_mode == "ManualLaneRoute" and not lanes and
            destination not in (None, "", "None")):
        route_mode = "AutomaticToAnchor"
    start_lane = lanes[0] if lanes else "None"
    # AutomaticToAnchor uses the first lane only as a start hint. Copying it to
    # OrderedLaneIds would accidentally turn the route into a one-lane manual route.
    vehicle_lanes = [] if route_mode == "AutomaticToAnchor" else lanes
    source = person_entry.get("SourceReference") or "Legacy person timeline"
    old_notes = person_entry.get("Notes") or ""
    notes = "[MIGRATED_FROM_PERSON_TIMELINE] " + old_notes

    return {
        "EntryId": entry_id,
        "Action": "BeginDriving",
        "RouteSegmentName": person_entry.get("EntryId") or "Migrated route",
        "Time": copy.deepcopy(person_entry.get("Time") or
                              {"Hour": 23, "Minute": 0, "Second": 0}),
        "TimingMode": "Absolute" if person_entry.get("TimingMode") ==
            "RelativeToPreviousEntry" else person_entry.get("TimingMode", "Absolute"),
        "SharedEventId": person_entry.get("SharedEventId", "None")
            if person_entry.get("TimingMode") != "RelativeToPreviousEntry" else "None",
        "EventOffsetSeconds": person_entry.get("EventOffsetSeconds", 0)
            if person_entry.get("TimingMode") != "RelativeToPreviousEntry" else 0,
        # On a person BeginDriving row Time is its trigger/departure. The generic
        # person bTimeIsArrival flag only has runtime meaning for MoveToAnchor.
        "bTimeIsArrival": False,
        "bUseExplicitDepartureTime": False,
        "DepartureTimingMode": "Absolute",
        "DepartureTime": {"Hour": 23, "Minute": 0, "Second": 0},
        "DepartureSharedEventId": "None",
        "DepartureOffsetSeconds": 0,
        "CruiseSpeedOverrideKmh": 0,
        "DrivingPreset": "AutomaticFromTimeline",
        "WorldTransform": identity_transform(),
        "PlacementMode": "WorldTransform",
        "PlacementAnchorId": "None",
        "AnchorLocalOffset": identity_transform(),
        "OffscreenTransferDurationSeconds": 60,
        "OrderedLaneIds": vehicle_lanes,
        "bAutoStartFromVehicleTimeline": True,
        "VehicleRouteMode": route_mode,
        "RouteStartAnchorId": "None",
        "RouteStartLaneId": start_lane,
        "RouteStartDistanceAlongFirstLaneCm":
            person_entry.get("VehicleStartDistanceAlongFirstLaneCm", 0),
        "RouteDestinationAnchorId": destination,
        "RouteDestinationLaneId": "None",
        "RouteViaAnchorIds": copy.deepcopy(person_entry.get("PassAnchorIds") or []),
        "RouteViaLaneIds": [],
        "bIgnoreOneWayRestrictions": False,
        "bRunRedLights": False,
        # Legacy BeginDriving was triggered by the driver and did not wait for
        # every historically associated person. Preserve that exact behavior;
        # the copied passenger IDs remain useful metadata in the editor.
        "bWaitForListedOccupants": False,
        "BoardingBufferSeconds": 4,
        "DriverEntityId": driver_id,
        "PassengerEntityIds": passenger_ids(vehicle, driver_id),
        "Confidence": person_entry.get("Confidence", "Reconstructed"),
        "SourceReference": source,
        "Notes": notes,
    }


def approximate_second(entry: dict[str, Any]) -> int:
    time = entry.get("Time") or {}
    return int(time.get("Hour", 0)) * 3600 + int(time.get("Minute", 0)) * 60 + int(time.get("Second", 0))


def insert_stably(timeline: list[dict[str, Any]], entry: dict[str, Any]) -> None:
    entry_second = approximate_second(entry)
    insert_at = len(timeline)
    for index, current in enumerate(timeline):
        if approximate_second(current) > entry_second:
            insert_at = index
            break
    timeline.insert(insert_at, entry)


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--people", type=Path, required=True)
    parser.add_argument("--vehicles", type=Path, required=True)
    parser.add_argument("--baseline-vehicles", type=Path, required=True,
                        help="Last export before protected manual edits")
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--protect", action="append", default=[],
                        help="Additional VehicleId to preserve; repeatable")
    args = parser.parse_args()

    people, _ = read_json(args.people)
    vehicles, _ = read_json(args.vehicles)
    baseline, _ = read_json(args.baseline_vehicles)
    vehicle_map = {row.get("VehicleId"): row for row in vehicles}
    protected = protected_vehicle_ids(baseline, vehicles) | set(args.protect)

    migrated: list[dict[str, Any]] = []
    adopted_existing: list[dict[str, Any]] = []
    skipped_protected: list[dict[str, Any]] = []
    unresolved: list[dict[str, Any]] = []

    for person in people:
        driver_id = person.get("EntityId")
        retained_timeline: list[dict[str, Any]] = []
        for person_entry in person.get("Timeline") or []:
            if person_entry.get("Action") != "BeginDriving":
                retained_timeline.append(person_entry)
                continue
            raw_vehicle_id = person_entry.get("TargetEntityId")
            vehicle_id = KNOWN_VEHICLE_ALIASES.get(raw_vehicle_id, raw_vehicle_id)
            item = {"VehicleId": vehicle_id, "DriverEntityId": driver_id,
                    "PersonEntryId": person_entry.get("EntryId")}
            vehicle = vehicle_map.get(vehicle_id)
            if vehicle is None:
                unresolved.append({**item, "OriginalVehicleId": raw_vehicle_id})
                retained_timeline.append(person_entry)
                continue
            if vehicle_id in protected:
                skipped_protected.append(item)
                retained_timeline.append(person_entry)
                continue
            existing = find_existing_drive_match(
                person_entry, vehicle, driver_id)
            if existing is not None and has_usable_route(existing):
                adopt_existing_vehicle_entry(existing, person_entry, driver_id)
                adopted_existing.append({
                    **item,
                    "VehicleEntryId": existing.get("EntryId"),
                })
                # This route now starts from the existing vehicle entry.
                continue

            timeline = vehicle.setdefault("Timeline", [])
            ids = {str(current.get("EntryId")) for current in timeline}
            new_entry = make_vehicle_entry(vehicle, person_entry, driver_id, ids)
            route_mode = new_entry["VehicleRouteMode"]
            has_manual_route = bool(new_entry["OrderedLaneIds"])
            has_destination = new_entry["RouteDestinationAnchorId"] not in (
                None, "", "None")
            if ((route_mode == "ManualLaneRoute" and not has_manual_route) or
                    (route_mode != "ManualLaneRoute" and not has_destination)):
                unresolved.append({
                    **item,
                    "OriginalVehicleId": raw_vehicle_id,
                    "Reason": "Legacy drive has neither usable lanes nor a destination anchor",
                })
                retained_timeline.append(person_entry)
                continue
            insert_stably(timeline, new_entry)
            if vehicle.get("KnownDriverEntityId") in (None, "", "None"):
                vehicle["KnownDriverEntityId"] = driver_id
            associated = vehicle.setdefault("AssociatedPersonEntityIds", [])
            if driver_id not in associated:
                associated.append(driver_id)
            migrated.append({**item, "VehicleEntryId": new_entry["EntryId"]})
            # Intentionally omit the old person entry only after a successful copy.
        person["Timeline"] = retained_timeline

    args.output_dir.mkdir(parents=True, exist_ok=True)
    vehicles_path = args.output_dir / "DT_TMOP_HistoricalVehicles_MIGRATED.json"
    people_path = args.output_dir / "DT_TMOP_People_MIGRATED.json"
    people_zip = args.output_dir / "DT_TMOP_People_MIGRATED.zip"
    report_path = args.output_dir / "TMOP_VehicleTimelineMigration_Report.json"
    write_utf16_json(vehicles_path, vehicles)
    write_utf16_json(people_path, people)
    write_people_zip(people_zip, people)
    report = {
        "SourcePeople": str(args.people),
        "SourceVehicles": str(args.vehicles),
        "ProtectionBaseline": str(args.baseline_vehicles),
        "ProtectedVehicleIds": sorted(protected),
        "Migrated": migrated,
        "AdoptedExisting": adopted_existing,
        "SkippedProtected": skipped_protected,
        "Unresolved": unresolved,
        "Counts": {
            "ProtectedVehicles": len(protected),
            "MigratedNewEntries": len(migrated),
            "AdoptedExistingEntries": len(adopted_existing),
            "ConvertedPersonEntries": len(migrated) + len(adopted_existing),
            "SkippedProtectedEntries": len(skipped_protected),
            "UnresolvedEntries": len(unresolved),
        },
    }
    report_path.write_text(json.dumps(report, ensure_ascii=False, indent=2) + "\n",
                           encoding="utf-8")
    print(json.dumps(report["Counts"], ensure_ascii=False))
    return 0 if not unresolved else 2


if __name__ == "__main__":
    sys.exit(main())
