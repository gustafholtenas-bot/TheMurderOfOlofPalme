#!/usr/bin/env python3
"""Build cumulative Page31 v7 replacement tables from the verified v6 package.

This keeps the 08_25 research data intact and writes import-ready 08_26
replacements with the critical-minute, group, vehicle and parking corrections.
"""

from __future__ import annotations

import copy
import json
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
WORKSPACE = REPO.parent
V6 = WORKSPACE / "work" / "Page31Package"
OUT = WORKSPACE / "work" / "TMOP_Page31_Logic_0826_v7"
SHOT_EVENT = "Palme_shot_1"
SHOT_SECOND = 23 * 3600 + 21 * 60 + 30

PEOPLE_IN = V6 / "DataTables/08_25/DT_TMOP_People.json"
VEHICLES_IN = V6 / "DataTables/08_25/DT_TMOP_HistoricalVehicles.json"
GROUPS_IN = REPO / "DataTables/08_23/DT_TMOP_Groups.json"
ANCHORS_IN = V6 / "Content/TMOP/Data/TMOP_PAGE31_WITNESS_TIMELINE_ANCHORS.json"

CRITICAL_GROUPS = {
    "GROUP_OLOF_LISBET_PALME",
    "GROUP_ANNE_KARIN",
    "GROUP_ANNIKA_EGON",
    "GROUP_CARINA_SIRPA",
    "GROUP_VALLIN",
    "GROUP_SUSANNE_ULRIKA",
    "GROUP_INGE_MORELIUS_WITNESSES",
}

CRITICAL_INDIVIDUALS = {
    "OLOF_PALME", "LISBET_PALME",
    "ANNE_HAGE", "KARIN_JOHANSSON",
    "ANNIKA_BLOMQVIST", "EGON_ENOCKSSON",
    "CARINA_PETTERSSON", "SIRPA_LINDGREN",
    "PER_VALLIN", "CHRISTINA_VALLIN",
    "SUSANNE_KARLSSON", "ULRIKA_RYTTERSTAL",
    "HELENA_LAHDE", "SUSANNE_LARSSON",
    "SVEN_ERIK_ROLFART_FODD_59",
}

RUN_TO_SCENE = {
    "ANNE_HAGE", "KARIN_JOHANSSON", "STEFAN_GLANTZ",
    "LENA_BASEN", "GORAN_ISRAELSSON", "ANDERS_ERSSON",
}


def read_json(path: Path):
    raw = path.read_bytes()
    encoding = "utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding))


def write_json(path: Path, value, *, utf16: bool = False) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(
        json.dumps(value, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-16" if utf16 else "utf-8",
    )


def preview_time(offset: int) -> dict[str, int]:
    second = (SHOT_SECOND + offset) % (24 * 3600)
    return {
        "Hour": second // 3600,
        "Minute": (second % 3600) // 60,
        "Second": second % 60,
    }


def absolute_preview_second(entry: dict) -> int:
    time = entry.get("Time") or {}
    return (
        int(time.get("Hour", 0)) * 3600
        + int(time.get("Minute", 0)) * 60
        + int(time.get("Second", 0))
    )


def page_entry(template: dict, entry_id: str, offset: int, action: str,
               target_anchor: str = "None", activity: str = "Standing") -> dict:
    entry = copy.deepcopy(template)
    entry.update({
        "EntryId": entry_id,
        "Action": action,
        "Time": preview_time(offset),
        "TimingMode": "Relative",
        "SharedEventId": SHOT_EVENT,
        "EventOffsetSeconds": offset,
        "bTimeIsArrival": action == "MoveToAnchor",
        "LocationType": "Anchor" if target_anchor != "None" else "Unknown",
        "TargetAnchorId": target_anchor,
        "PassAnchorIds": [],
        "TargetEntityId": "None",
        "TargetSeatId": "None",
        "ActivityState": activity,
        "bTeleportDuringCatchUp": False,
        "bSupersedeActiveMovementWhenDue": True,
        "Confidence": "Reconstructed",
        "SourceReference": "itdemokrati.nu/page31.html; witness interviews; TimelineValidation_20260826_194615",
    })
    return entry


def insert_before_first_page(person: dict, entry: dict) -> None:
    timeline = person.setdefault("Timeline", [])
    index = next(
        (i for i, item in enumerate(timeline)
         if str(item.get("EntryId", "")).startswith("PAGE31_")),
        len(timeline),
    )
    timeline.insert(index, entry)


def make_people(people: list[dict], anchors: dict) -> dict:
    by_name = {person["Name"]: person for person in people}
    changed: set[str] = set()

    # Runtime groups are deliberately absent during the critical minute. The
    # semantic group rows remain in DT_TMOP_Groups for research and later use.
    for name in CRITICAL_INDIVIDUALS:
        person = by_name[name]
        person["SocialGroupId"] = "None"
        person["GroupLeaderEntityId"] = "None"
        person["bFollowGroupLeaderSchedule"] = False
        changed.add(name)

    # Every Page31 deadline may replace an older delayed movement, but still
    # starts from the live position and never uses catch-up teleporting.
    for person in people:
        has_page = False
        last_page_index = -1
        for index, entry in enumerate(person.get("Timeline", [])):
            if str(entry.get("EntryId", "")).startswith("PAGE31_"):
                has_page = True
                last_page_index = index
                entry["bSupersedeActiveMovementWhenDue"] = True
                entry["bTeleportDuringCatchUp"] = False

                if (entry.get("Action") == "Wait"
                    and entry.get("TargetAnchorId") not in {None, "", "None"}):
                    if person["Name"] == "OLOF_PALME":
                        entry["TargetAnchorId"] = "None"
                        entry["LocationType"] = "Unknown"
                        entry["Notes"] = (
                            "Holds Olof's fallen/body state at the position reached by the "
                            "preceding movement. Wait deliberately has no destination anchor."
                        )
                    else:
                        entry["Action"] = "MoveToAnchor"
                        entry["bTimeIsArrival"] = True
                        entry["LocationType"] = "Anchor"
                        if person["Name"] in RUN_TO_SCENE:
                            entry["ActivityState"] = "Running"
                        elif entry.get("ActivityState") in {"Standing", "Idle"}:
                            entry["ActivityState"] = "Walking"
                        entry["Notes"] = (
                            "Physical arrival at the Page31 final milestone. The person "
                            "remains available for later timeline actions."
                        )
        if has_page:
            changed.add(person["Name"])
            # A later RelativeToPrevious entry must not inherit the calculated
            # departure time of an arrival-timed Page31 move.
            for entry in person.get("Timeline", [])[last_page_index + 1:]:
                if entry.get("TimingMode") == "RelativeToPreviousEntry":
                    entry["TimingMode"] = "Absolute"
                    entry["SharedEventId"] = "None"
                    entry["EventOffsetSeconds"] = 0

    # Sirpa previously had only MoveToAnchor entries and therefore could never
    # spawn. She starts with Carina at Tre Backar.
    sirpa = by_name["SIRPA_LINDGREN"]
    if not any(e.get("Action") in {"InitialPlacement", "Spawn"}
               for e in sirpa.get("Timeline", [])):
        template = copy.deepcopy(by_name["CARINA_PETTERSSON"]["Timeline"][0])
        template.update({
            "EntryId": "SIRPA_LINDGREN_TRE_BACKAR_INITIAL",
            "Action": "InitialPlacement",
            "Time": {"Hour": 23, "Minute": 0, "Second": 0},
            "TimingMode": "Absolute",
            "SharedEventId": "None",
            "EventOffsetSeconds": 0,
            "bTimeIsArrival": False,
            "LocationType": "Anchor",
            "TargetAnchorId": "TreBackar_inside",
            "ActivityState": "Standing",
            "bTeleportDuringCatchUp": True,
            "bSupersedeActiveMovementWhenDue": False,
            "Confidence": "Documented",
            "Notes": "Spawns with Carina Pettersson at Tre Backar before their independent critical-minute routes.",
        })
        sirpa["Timeline"].insert(0, template)

    # Annika and Egon are at Monte Carlo when the shots are heard. They then
    # turn north on the west pavement and stop opposite the scene; they do not
    # cross to the eastern crime-scene pavement.
    anchor_list = anchors["anchors"]
    for name in ("ANNIKA_BLOMQVIST", "EGON_ENOCKSSON"):
        person = by_name[name]
        template = next(e for e in person["Timeline"]
                        if str(e.get("EntryId", "")).startswith("PAGE31_"))
        if not any(e.get("EntryId") == f"PAGE31_{name}_TM20_MONTE_CARLO"
                   for e in person["Timeline"]):
            monte = page_entry(
                template, f"PAGE31_{name}_TM20_MONTE_CARLO", -20,
                "MoveToAnchor", "MonteCarlo_outside", "Walking")
            monte["Notes"] = (
                "Arrives at Monte Carlo before the shots, reconstructed from the "
                "Annika Blomqvist/Egon Enocksson witness sequence."
            )
            insert_before_first_page(person, monte)

        old_anchor = f"ANCHOR_PAGE31_{name}_CRIME_SCENE_FINAL"
        new_anchor = f"ANCHOR_PAGE31_{name}_OPPOSITE_SCENE_WEST_SIDE"
        for anchor in anchor_list:
            if anchor.get("anchor_id") == old_anchor:
                anchor["anchor_id"] = new_anchor
                anchor["display_name"] = f"{name} opposite scene on west side"
                anchor["category"] = "WITNESS_POSITION"
                anchor["notes"] = (
                    "Stops on the west side opposite the murder scene. This is not "
                    "a crime-scene crossing or an eastern-pavement final position."
                )
        for entry in person["Timeline"]:
            if entry.get("TargetAnchorId") == old_anchor:
                entry["TargetAnchorId"] = new_anchor
                entry["EntryId"] = f"PAGE31_{name}_TP43_OPPOSITE_SCENE_WEST_SIDE"
                entry["Action"] = "MoveToAnchor"
                entry["bTimeIsArrival"] = True
                entry["LocationType"] = "Anchor"
                entry["ActivityState"] = "Walking"
                entry["Notes"] = (
                    "Stops on the west pavement opposite the scene without crossing "
                    "Sveavägen; later departure actions remain active."
                )

    # Åke's boarding chain must be able to recover from the earlier McDonald's
    # walk before the shot-window car route begins.
    ake = by_name["AKE_LARSSON"]
    for entry in ake["Timeline"]:
        if entry.get("EntryId") in {
            "AKE_LARSSON_VEHICLE_01", "AKE_LARSSON_VEHICLE_02",
            "AKE_LARSSON_VEHICLE_03", "AKE_LARSSON_VEHICLE_04",
            "PAGE31_AKE_LARSSON_VEHICLE_ROUTE",
        }:
            entry["bSupersedeActiveMovementWhenDue"] = True
            entry["bTeleportDuringCatchUp"] = False

    # Anders must leave his earlier group even if the preceding ATM movement
    # completed late, otherwise his independent Page31 movements are skipped.
    anders = by_name["ANDERS_BJORKMAN"]
    for entry in anders["Timeline"]:
        if entry.get("EntryId") == "ANDERS_BJORKMAN_LEAVE_SALLSKAP":
            entry["bSupersedeActiveMovementWhenDue"] = True

    # Vehicle and person spawning at the same second was order-dependent.
    bengt = by_name["BENGT_PALM"]
    initial = next(e for e in bengt["Timeline"]
                   if e.get("Action") == "InitialPlacement")
    initial["Time"] = {"Hour": 23, "Minute": 0, "Second": 1}
    initial["bSupersedeActiveMovementWhenDue"] = False
    bengt["bFollowGroupLeaderSchedule"] = False

    # Morelius's two manual approaches now use exact destinations. The first
    # reaches the ATM drop-off; the second parks at the Page31 waiting point.
    inge = by_name["INGE_MORELIUS_G"]
    morelius_wait = (
        "ANCHOR_PAGE31_VEHICLE_"
        "VEHICLE_INGE_MORELIUS_MORK_GUL_OPEL_REKORD_73_A_TM20_WAITING"
    )
    for entry in inge["Timeline"]:
        if entry.get("EntryId") == "INGE_MORELIUS_DRIVES_FROM_SOUTH_SVEAVAGEN_TO_ATM_DROPOFF":
            entry["DrivingDestinationAnchorId"] = "ATM2_parkering"
        elif entry.get("EntryId") == "INGE_MORELIUS_TURNS_BEFORE_OLOFSGATAN_AND_PARKS":
            entry["DrivingDestinationAnchorId"] = morelius_wait
        elif entry.get("EntryId") == "INGE_MORELIUS_WAITS_20_SECONDS_AT_ATM2_PARKING":
            entry["TargetAnchorId"] = "None"
            entry["Notes"] = (
                "Waits seated while the vehicle remains at the ATM2 parking "
                "destination reached by the preceding manual route."
            )

    # The first Delsborn route and Hans's U-turn previously ended at a lane or
    # an older anchor and relied on the later Stop row to correct the position.
    # Point the actual driving action at the same Page31 stop anchor.
    delsborn = by_name["ANDERS_DELBOM"]
    delsborn_destination = (
        "ANCHOR_PAGE31_VEHICLE_VEHICLE_ANDERS_DELBOMS_TAXI_"
        "BIL_JARFALLA_TAXI_MITSUBISHI_GALANT_TM20_WAITING"
    )
    next(e for e in delsborn["Timeline"]
         if e.get("EntryId") == "ANDERS_DELBOM_VEHICLE_01")[
             "DrivingDestinationAnchorId"] = delsborn_destination
    changed.add("ANDERS_DELBOM")

    hans = by_name["HANS_JOHANSSON"]
    hans_destination = (
        "ANCHOR_PAGE31_VEHICLE_"
        "VEHICLE_HANS_JOHANSSON_TAXI_TP20_STOPS"
    )
    next(e for e in hans["Timeline"]
         if e.get("EntryId") == "HANS_JOHANSSON_UTURN_TO_CRIME_SCENE")[
             "DrivingDestinationAnchorId"] = hans_destination
    changed.add("HANS_JOHANSSON")

    # Existing ambulance manual lanes now honor these exact destinations in
    # engine 0.0.104. Keep the fields explicit for validation/import review.
    for name, entry_id, destination in (
        ("PETER_ANDERSSON_AMB", "PETER_ANDERSSON_A951_APPROACH", "VEHICLE_AMBULANCE_951"),
        ("MARIA_DEGERMAN", "MARIA_DEGERMAN_912_APPROACH", "VEHICLE_AMBULANCE_912"),
    ):
        person = by_name[name]
        entry = next(e for e in person["Timeline"] if e.get("EntryId") == entry_id)
        entry["DrivingDestinationAnchorId"] = destination
        changed.add(name)

    # Data validation.
    errors: list[str] = []
    page_people = [p for p in people if any(
        str(e.get("EntryId", "")).startswith("PAGE31_")
        for e in p.get("Timeline", []))]
    for person in page_people:
        ids = [e.get("EntryId") for e in person.get("Timeline", [])]
        if len(ids) != len(set(ids)):
            errors.append(f"duplicate timeline entry in {person['Name']}")
        if person.get("bSpawnInSimulation") and not any(
            e.get("Action") in {"InitialPlacement", "Spawn"}
            for e in person.get("Timeline", [])):
            errors.append(f"missing spawn/initial placement for {person['Name']}")
        for entry in person.get("Timeline", []):
            if not str(entry.get("EntryId", "")).startswith("PAGE31_"):
                continue
            if not entry.get("bSupersedeActiveMovementWhenDue", False):
                errors.append(f"non-critical Page31 entry {entry['EntryId']}")
            if (entry.get("Action") == "Wait"
                and entry.get("TargetAnchorId") not in {None, "", "None"}):
                errors.append(f"Wait incorrectly claims anchor {entry['EntryId']}")
    if errors:
        raise SystemExit("; ".join(errors))

    return {
        "changed_people": sorted(changed),
        "page31_people": len(page_people),
        "page31_entries": sum(
            str(e.get("EntryId", "")).startswith("PAGE31_")
            for p in people for e in p.get("Timeline", [])),
    }


def make_groups(groups: list[dict]) -> list[str]:
    changed = []
    for group in groups:
        if group.get("GroupId") not in CRITICAL_GROUPS:
            continue
        group["bUseLeaderTimeline"] = False
        group["bCreateAtScenarioStart"] = False
        group["Notes"] = (
            group.get("Notes", "")
            + " v7: runtime group movement is disabled during the critical "
              "Page31 minute; members use independent person timelines."
        ).strip()
        changed.append(group["GroupId"])
    missing = sorted(CRITICAL_GROUPS - set(changed))
    if missing:
        raise SystemExit(f"Missing critical group rows: {missing}")
    return sorted(changed)


def make_vehicles(vehicles: list[dict]) -> list[str]:
    by_name = {vehicle["Name"]: vehicle for vehicle in vehicles}
    changed = []
    for vehicle_id in ("AMBULANCE_A951", "AMBULANCE_912"):
        vehicle = by_name[vehicle_id]
        for entry in vehicle.get("Timeline", []):
            if entry.get("Action") in {"Stop", "Park"}:
                entry["Notes"] = (
                    entry.get("Notes", "")
                    + " Engine 0.0.104 suppresses distant timed teleports; the "
                      "driver route must physically reach this anchor."
                ).strip()
        changed.append(vehicle_id)
    return sorted(changed)


def main() -> None:
    if OUT.exists():
        raise SystemExit(f"Output already exists: {OUT}")

    people = read_json(PEOPLE_IN)
    vehicles = read_json(VEHICLES_IN)
    groups = read_json(GROUPS_IN)
    anchors = read_json(ANCHORS_IN)

    people_report = make_people(people, anchors)
    changed_groups = make_groups(groups)
    changed_vehicles = make_vehicles(vehicles)
    anchors["format"] = "TMOP_PAGE31_RELATIVE_TIMELINE_ANCHORS_V2"
    anchors["anchor_count"] = len(anchors["anchors"])
    anchors["v7_logic_note"] = (
        "Annika Blomqvist and Egon Enocksson stop on the west side opposite "
        "the scene; CRIME_SCENE_FINAL does not imply crossing Sveavägen."
    )

    write_json(OUT / "DataTables/08_26/DT_TMOP_People.json", people, utf16=True)
    write_json(OUT / "DataTables/08_26/DT_TMOP_HistoricalVehicles.json", vehicles, utf16=True)
    write_json(OUT / "DataTables/08_26/DT_TMOP_Groups.json", groups, utf16=True)
    write_json(
        OUT / "Content/TMOP/Data/TMOP_PAGE31_WITNESS_TIMELINE_ANCHORS.json",
        anchors,
    )
    write_json(OUT / "PAGE31_MARKER_LEGEND.json", read_json(V6 / "PAGE31_MARKER_LEGEND.json"))

    report = {
        "package": "TMOP Page31 v7 logic and parking fixes",
        "engine_version": "0.0.104",
        "base": "TMOP_Page31_Timeline_0826_v6_ALL_VEHICLE_CONFLICTS_FIXED",
        "people": people_report,
        "groups_changed": changed_groups,
        "vehicle_rows_annotated": changed_vehicles,
        "parking_behavior": "Manual destinations use smooth final approach; distant Stop/Park teleports suppressed.",
        "validation_status": "PASS",
    }
    write_json(OUT / "VALIDATION_V7.json", report)


if __name__ == "__main__":
    main()
