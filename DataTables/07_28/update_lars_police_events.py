"""Split the existing Lars/police event and connect Lars and 3230 officers."""
from __future__ import annotations

import copy
import json
from pathlib import Path


ROOT = Path("TMOP_Police_Ambulance_Import")
PEOPLE_PATH = ROOT / "DT_TMOP_People_POLICE_AMBULANCE_IMPORT.json"
EVENTS_PATH = ROOT / "DT_TMOP_HistoricalEvents_AMBULANCE_UPDATED.json"


def load(path: Path):
    return json.loads(path.read_text(encoding="utf-8-sig"))


def save(path: Path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")


def clock(minute: int, second: int):
    return {"Hour": 23, "Minute": minute, "Second": second}


people = load(PEOPLE_PATH)
events = load(EVENTS_PATH)

old_event = next(row for row in events if row["EventId"] == "LARS_MEET_POLICE")
old_event.update(
    {
        "Name": "LARS_MEETS_POLICE_FIRST_PASS",
        "EventId": "LARS_MEETS_POLICE_FIRST_PASS",
        "DisplayName": "Lars Jeppsson – poliserna passerar första gången",
        "TimingMode": "Window",
        "HistoricalLock": "Free",
        "Confidence": "Reconstructed",
        "AbsoluteTime": clock(25, 35),
        "EarliestTime": clock(25, 20),
        "PreferredTime": clock(25, 35),
        "LatestTime": clock(25, 50),
        "TriggerEventId": "None",
        "MinimumDelaySeconds": 0,
        "PreferredDelaySeconds": 0,
        "MaximumDelaySeconds": 0,
        "SourceId": "Användarens kompletterande uppgift",
        "Notes": (
            "Första kontakten uppe på åsen/David Bagares gata: "
            "poliserna passerar Lars utan att ta med honom."
        ),
    }
)

second_event = copy.deepcopy(old_event)
second_event.update(
    {
        "Name": "LARS_MEETS_POLICE_RETURN_TO_CRIME_SCENE",
        "EventId": "LARS_MEETS_POLICE_RETURN_TO_CRIME_SCENE",
        "DisplayName": "Jan Hermansson tar Lars Jeppsson till mordplatsen",
        "Confidence": "Documented",
        "AbsoluteTime": clock(26, 30),
        "EarliestTime": clock(26, 0),
        "PreferredTime": clock(26, 30),
        "LatestTime": clock(27, 0),
        "SourceId": "A14205-00; A14205-05",
        "Notes": (
            "Jan Hermansson tar med Lars Jeppsson tillbaka ned till "
            "mordplatsen för förhör."
        ),
    }
)
index = events.index(old_event)
events.insert(index + 1, second_event)

timeline_template = copy.deepcopy(
    next(entry for row in people for entry in row.get("Timeline", []))
)


def event_entry(
    entry_id: str,
    action: str,
    event_id: str,
    offset: int,
    shown_time: dict,
    anchor_id: str,
    notes: str,
):
    entry = copy.deepcopy(timeline_template)
    entry.update(
        {
            "EntryId": entry_id,
            "Action": action,
            "Time": shown_time,
            "TimingMode": "Relative",
            "SharedEventId": event_id,
            "EventOffsetSeconds": offset,
            "bTimeIsArrival": action == "MoveToAnchor",
            "TravelSpeedOverrideCmPerSecond": 0,
            "LocationType": "Anchor",
            "TargetAnchorId": anchor_id,
            "PassAnchorIds": [],
            "TargetEntityId": "None",
            "TargetSeatId": "None",
            "TargetStopId": "None",
            "OrderedLaneIds": [],
            "VehicleRouteMode": "ManualLaneRoute",
            "DrivingDestinationAnchorId": "None",
            "VehicleStartDistanceAlongFirstLaneCm": 0,
            "TargetGroupId": "None",
            "ActivityState": "Walking" if action == "MoveToAnchor" else "Standing",
            "LifeState": "Alive",
            "bTeleportDuringCatchUp": True,
            "Confidence": "Documented",
            "SourceReference": "A14205-00; A14205-05",
            "Notes": notes,
        }
    )
    return entry


def replace_entries(row, additions):
    ids = {entry["EntryId"] for entry in additions}
    row["Timeline"] = [
        entry for entry in row.get("Timeline", []) if entry.get("EntryId") not in ids
    ]
    row["Timeline"].extend(additions)
    row["Timeline"].sort(
        key=lambda entry: (
            entry.get("Time", {}).get("Hour", 0),
            entry.get("Time", {}).get("Minute", 0),
            entry.get("Time", {}).get("Second", 0),
        )
    )


lars = next(row for row in people if row["EntityId"] == "LARS_JEPPSSON")
replace_entries(
    lars,
    [
        event_entry(
            "LARS_JEPPSSON_POLICE_FIRST_PASS",
            "Wait",
            "LARS_MEETS_POLICE_FIRST_PASS",
            0,
            clock(25, 35),
            "DavidBagaresg_26",
            "Lars blir kvar när poliserna passerar första gången.",
        ),
        event_entry(
            "LARS_JEPPSSON_TAKEN_TO_CRIME_SCENE",
            "MoveToAnchor",
            "LARS_MEETS_POLICE_RETURN_TO_CRIME_SCENE",
            90,
            clock(28, 0),
            "ANCHOR_TEST_END",
            "Jan Hermansson tar med Lars ned till mordplatsen.",
        ),
    ],
)

for entity_id in ("CLAES_DJURFELDT", "JAN_HERMANSSON"):
    officer = next(row for row in people if row["EntityId"] == entity_id)
    additions = [
        event_entry(
            f"{entity_id}_PASSES_LARS_FIRST_TIME",
            "MoveToAnchor",
            "LARS_MEETS_POLICE_FIRST_PASS",
            0,
            clock(25, 35),
            "DavidBagaresg_26",
            "Deltar i den första polispassagen förbi Lars Jeppsson.",
        )
    ]
    if entity_id == "JAN_HERMANSSON":
        additions.append(
            event_entry(
                "JAN_HERMANSSON_TAKES_LARS_TO_CRIME_SCENE",
                "MoveToAnchor",
                "LARS_MEETS_POLICE_RETURN_TO_CRIME_SCENE",
                90,
                clock(28, 0),
                "ANCHOR_TEST_END",
                "Tar Lars Jeppsson tillbaka ned till mordplatsen.",
            )
        )
    replace_entries(officer, additions)

save(PEOPLE_PATH, people)
save(EVENTS_PATH, events)
