#!/usr/bin/env python3
"""Fix the missing Palme event and distribute Grand 1 exits over four anchors."""

from __future__ import annotations

import argparse
import json
import re
from collections import Counter
from collections import defaultdict
from pathlib import Path


GRAND_OUTSIDE_ANCHORS = (
    "GrandOutside_1_North",
    "GrandOutside_2_Entrance",
    "GrandOutside_3_South",
    "GrandOutside_4_Curb",
)
OLD_GRAND_OUTSIDE = {"OutsideGrand", "grand_outside"}


def load_json(path: Path):
    raw = path.read_bytes()
    encoding = "utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding))


def save_unreal_json(path: Path, value) -> None:
    text = json.dumps(value, ensure_ascii=False, indent="\t") + "\n"
    path.write_text(text, encoding="utf-16")


def is_grand_one(profile: dict) -> bool:
    if profile.get("CategoryId") == "GRAND_1":
        return True
    for entry in profile.get("Timeline", []):
        if str(entry.get("TargetSeatId", "")).startswith("SEAT_GRAND_1_") or \
                str(entry.get("TargetAnchorId", "")).startswith("SEAT_GRAND_1_"):
            return True
    return False


def fix_people(path: Path) -> dict:
    people = load_json(path)
    grand_people = [p for p in people if is_grand_one(p)]
    grouped = defaultdict(list)
    for profile in grand_people:
        key = profile.get("SocialGroupId")
        if not key or key == "None":
            key = profile["EntityId"]
        grouped[key].append(profile)

    assignments = {}
    load = [0, 0, 0, 0]
    # The two Palme groups must meet at the same exit location.
    for key in ("GROUP_OLOF_LISBET_PALME", "GROUP_MARTEN_PALME_INGRID_KLERING"):
        if key in grouped:
            assignments[key] = GRAND_OUTSIDE_ANCHORS[1]
            load[1] += len(grouped[key])
    for key, members in sorted(grouped.items(), key=lambda item: (-len(item[1]), item[0])):
        if key in assignments:
            continue
        target_index = min(range(4), key=lambda index: (load[index], index))
        assignments[key] = GRAND_OUTSIDE_ANCHORS[target_index]
        load[target_index] += len(members)

    replacements = 0
    changed_people = set()
    for key, profiles in grouped.items():
        destination = assignments[key]
        for profile in profiles:
            for entry in profile.get("Timeline", []):
                if entry.get("TargetAnchorId") in OLD_GRAND_OUTSIDE:
                    entry["TargetAnchorId"] = destination
                    note = entry.get("Notes", "")
                    marker = "Validation 23:25: Grand 1 exit distributed over four anchors."
                    if marker not in note:
                        entry["Notes"] = (note + " " + marker).strip()
                    replacements += 1
                    changed_people.add(profile["EntityId"])

    # The briefly observed mistaken-greeting man must share Olof's exit point.
    for profile in people:
        if profile.get("EntityId") == "OBSERVED_UNKNOWN_L1113_MISTAKEN_GREETING_MAN":
            for entry in profile.get("Timeline", []):
                if entry.get("TargetAnchorId") in OLD_GRAND_OUTSIDE:
                    entry["TargetAnchorId"] = GRAND_OUTSIDE_ANCHORS[1]
                    replacements += 1
                    changed_people.add(profile["EntityId"])

    # Explicit seats prevent nondeterministic row-order placement from putting
    # Anders Ersson in the driver's seat and blocking the real taxi driver.
    taxi_seats = {
        "UNKNOWN_DRIVER_GLANTZ_TAXI_E9979": "FRONT_LEFT",
        "LENA_BASEN": "FRONT_RIGHT",
        "GORAN_ISRAELSSON": "REAR_LEFT",
        "STEFAN_GLANTZ": "REAR_CENTER",
        "ANDERS_ERSSON": "REAR_RIGHT",
    }
    taxi_vehicle = "VEHICLE_GLANTZ_BASEN_TAXI_E9979"
    taxi_entries = 0
    for profile in people:
        seat = taxi_seats.get(profile.get("EntityId"))
        if not seat:
            continue
        for entry in profile.get("Timeline", []):
            if (entry.get("TargetEntityId") == taxi_vehicle and
                    entry.get("LocationType") == "VehicleSeat"):
                entry["TargetSeatId"] = seat
                taxi_entries += 1

    # Enwall's source states that the car continues south on Sveavägen. The
    # old manual route was empty and could never start.
    enwall_entries = 0
    for profile in people:
        if profile.get("EntityId") != "LEIF_ENWALL":
            continue
        for entry in profile.get("Timeline", []):
            if entry.get("EntryId") == "LEIF_ENWALL_L865_BEGINS_DRIVING":
                entry["VehicleRouteMode"] = "AutomaticToDestination"
                entry["DrivingDestinationAnchorId"] = "ExitSveavagenS_Car"
                entry["OrderedLaneIds"] = []
                enwall_entries += 1

    marco_speed_entries = 0
    for profile in people:
        if profile.get("EntityId") != "MARCO_NEESER_BOFORS_ANSTALLD":
            continue
        for entry in profile.get("Timeline", []):
            if entry.get("EntryId") == "MARCO_NEESER_BOFORS_ANSTALLD_VEHICLE_01":
                entry["TravelSpeedOverrideCmPerSecond"] = 150
                marco_speed_entries += 1

    save_unreal_json(path, people)
    return {
        "grand1_people": len(grand_people),
        "changed_people": len(changed_people),
        "changed_entries": replacements,
        "anchor_load_by_people": dict(zip(GRAND_OUTSIDE_ANCHORS, load)),
        "explicit_glantz_taxi_seat_entries": taxi_entries,
        "enwall_routes_fixed": enwall_entries,
        "marco_departure_speed_entries": marco_speed_entries,
    }


def seconds_to_time(seconds: int) -> dict:
    seconds %= 24 * 3600
    return {"Hour": seconds // 3600,
            "Minute": (seconds % 3600) // 60,
            "Second": seconds % 60}


def fix_events(source_path: Path, output_path: Path, people_path: Path) -> dict:
    events = load_json(source_path)
    existing = {row.get("EventId") for row in events}
    added = False
    if "PALME_MARTEN_MEET_UP" not in existing:
        template = next(row for row in events if row.get("EventId") == "PALME_MEETING_SANDINS")
        event = dict(template)
        event.update({
            "Name": "PALME_MARTEN_MEET_UP",
            "EventId": "PALME_MARTEN_MEET_UP",
            "DisplayName": "Palmefamiljen möter Mårten och Ingrid utanför Grand",
            "TriggerEventId": "PALME_ANNAE_AT_GRAND",
            "MinimumDelaySeconds": 35,
            "PreferredDelaySeconds": 45,
            "MaximumDelaySeconds": 0,
            "SourceId": "PALME_GRAND_TIMELINE",
            "Notes": "Tillagt efter validering 23:25: eventet refererades av flera personer men saknades i eventtabellen.",
        })
        insert_at = next(i for i, row in enumerate(events) if row.get("EventId") == "PALME_MEETING_SANDINS")
        events.insert(insert_at, event)
        added = True

    sandins = next(row for row in events if row.get("EventId") == "PALME_MEETING_SANDINS")
    sandins["TriggerEventId"] = "PALME_MARTEN_MEET_UP"
    sandins["MinimumDelaySeconds"] = 25
    sandins["PreferredDelaySeconds"] = 25
    note = sandins.get("Notes", "")
    marker = "Rechained after PALME_MARTEN_MEET_UP; total preferred offset from Anna remains 70 seconds."
    if marker not in note:
        sandins["Notes"] = (note + " " + marker).strip()

    # Recover every other dangling SharedEvent reference. A missing definition
    # otherwise blocks the person's entire remaining timeline without an error.
    people = load_json(people_path)
    existing = {row.get("EventId") for row in events}
    references = defaultdict(list)
    for profile in people:
        for entry in profile.get("Timeline", []):
            event_id = entry.get("SharedEventId")
            if (entry.get("TimingMode") == "Relative" and
                    event_id not in existing and
                    event_id not in (None, "", "None")):
                references[event_id].append(entry)

    explicit_base_seconds = {
        "BINO_BUMPS_OLOF_AT_GRAND": 23 * 3600 + 8 * 60 + 10,
        "L825_GROUP_CROSSES_SVEAVAGEN_2312": 23 * 3600 + 12 * 60,
    }
    recovered = 0
    normalized_offsets = 0
    template = next(row for row in events if row.get("EventId") == "PALME_MEETING_SANDINS")
    for event_id, entries in sorted(references.items()):
        raw_seconds = []
        for entry in entries:
            time = entry.get("Time", {})
            raw_seconds.append(int(time.get("Hour", 0)) * 3600 +
                               int(time.get("Minute", 0)) * 60 +
                               int(time.get("Second", 0)))
        match = re.search(r"(?:^|_)([01]\d|2[0-3])([0-5]\d)(?:_|$)", event_id)
        if event_id in explicit_base_seconds:
            base_second = explicit_base_seconds[event_id]
        elif match:
            base_second = int(match.group(1)) * 3600 + int(match.group(2)) * 60
        else:
            counts = Counter(raw_seconds)
            maximum_count = max(counts.values())
            base_second = min(second for second, count in counts.items()
                              if count == maximum_count)

        event = dict(template)
        event.update({
            "Name": event_id,
            "EventId": event_id,
            "DisplayName": event_id.replace("_", " ").title(),
            "TimingMode": "Absolute",
            "HistoricalLock": "Free",
            "Confidence": "Reconstructed",
            "AbsoluteTime": seconds_to_time(base_second),
            "EarliestTime": seconds_to_time(base_second),
            "PreferredTime": seconds_to_time(base_second),
            "LatestTime": seconds_to_time(base_second),
            "TriggerEventId": "None",
            "MinimumDelaySeconds": 0,
            "PreferredDelaySeconds": 0,
            "MaximumDelaySeconds": 0,
            "SourceId": "AUTO_VALIDATION_EVENT_RECOVERY",
            "Notes": ("Recovered from dangling DT_TMOP_People SharedEvent references after "
                      "validation 23:25. Review historical precision when source research permits."),
        })
        events.append(event)
        recovered += 1

        # If the same event was reused for distinct raw times with offset zero,
        # preserve those action times as explicit offsets from the recovered base.
        for entry, raw_second in zip(entries, raw_seconds):
            if int(entry.get("EventOffsetSeconds", 0)) == 0 and raw_second != base_second:
                entry["EventOffsetSeconds"] = raw_second - base_second
                normalized_offsets += 1

    if normalized_offsets:
        save_unreal_json(people_path, people)
    save_unreal_json(output_path, events)
    return {"palme_event_added": added, "recovered_missing_events": recovered,
            "normalized_zero_offsets": normalized_offsets,
            "event_count": len(events)}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("package")
    parser.add_argument("--event-source", default="repo/Scripts/export/DT_TMOP_HistoricalEvents.json")
    args = parser.parse_args()
    package = Path(args.package)
    people_result = fix_people(package / "Database/DT_TMOP_People.json")
    events_result = fix_events(Path(args.event_source),
                               package / "Database/DT_TMOP_HistoricalEvents.json",
                               package / "Database/DT_TMOP_People.json")
    report = {"people": people_result, "events": events_result}
    (package / "Database/TMOP_Validation_2325_CHANGE_REPORT.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n", encoding="utf-8")
    print(json.dumps(report, ensure_ascii=False, indent=2))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
