"""Extend the current police foundation DataTables with the two ambulances."""
from __future__ import annotations

import copy
import json
import sys
from pathlib import Path


WORK = Path(sys.argv[1])
UPLOAD = Path(sys.argv[2])
OUTPUT = Path(sys.argv[3])
OUTPUT.mkdir(parents=True, exist_ok=True)


def load(path: Path):
    raw = path.read_bytes()
    encoding = "utf-16" if raw[:2] in (b"\xff\xfe", b"\xfe\xff") else "utf-8-sig"
    return json.loads(raw.decode(encoding))


def save(name: str, data):
    (OUTPUT / name).write_text(
        json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8"
    )


people = load(WORK / "DT_TMOP_People_POLICE_FOUNDATION_IMPORT.json")
vehicles = load(WORK / "DT_TMOP_HistoricalVehicles_POLICE_FOUNDATION_IMPORT.json")
groups = load(WORK / "DT_TMOP_Groups_POLICE_FOUNDATION_IMPORT.json")
events = load(UPLOAD / "DT_TMOP_HistoricalEvents(1).json")

person_template = copy.deepcopy(people[0])
person_timeline_template = copy.deepcopy(
    next(entry for row in people for entry in row["Timeline"])
)
vehicle_template = copy.deepcopy(vehicles[0])
vehicle_spawn_template = copy.deepcopy(
    next(
        entry
        for row in vehicles
        for entry in row.get("Timeline", [])
        if entry.get("Action") == "Spawn"
    )
)
group_template = copy.deepcopy(groups[0])


def clock(hour: int, minute: int, second: int):
    return {"Hour": hour, "Minute": minute, "Second": second}


def person_entry(
    entry_id: str,
    action: str,
    event_id: str,
    offset: int,
    shown_time: dict,
    vehicle_id: str,
    seat_id: str,
    activity: str,
    life_state: str = "Alive",
    destination_anchor: str = "None",
    notes: str = "",
):
    entry = copy.deepcopy(person_timeline_template)
    entry.update(
        {
            "EntryId": entry_id,
            "Action": action,
            "Time": shown_time,
            "TimingMode": "Relative",
            "SharedEventId": event_id,
            "EventOffsetSeconds": offset,
            "bTimeIsArrival": False,
            "TravelSpeedOverrideCmPerSecond": 0,
            "LocationType": "VehicleSeat",
            "TargetAnchorId": "None",
            "PassAnchorIds": [],
            "TargetEntityId": vehicle_id,
            "TargetSeatId": seat_id,
            "TargetStopId": "None",
            "OrderedLaneIds": [],
            "VehicleRouteMode": (
                "AutomaticToAnchor" if action == "BeginDriving" else "ManualLaneRoute"
            ),
            "DrivingDestinationAnchorId": destination_anchor,
            "VehicleStartDistanceAlongFirstLaneCm": 0,
            "TargetGroupId": "None",
            "ActivityState": activity,
            "LifeState": life_state,
            "bTeleportDuringCatchUp": True,
            "Confidence": "Documented",
            "SourceReference": "Ambulanspersonalens förhör",
            "Notes": notes,
        }
    )
    return entry


def ambulance_person(
    entity_id: str,
    name: str,
    category: str,
    occupation: str,
    vehicle_ids: list[str],
    group_id: str,
    timeline: list[dict],
    notes: str,
):
    row = copy.deepcopy(person_template)
    parts = name.split()
    row.update(
        {
            "Name": entity_id,
            "EntityId": entity_id,
            "CategoryId": category,
            "FullName": name,
            "FirstName": parts[0],
            "LastName": " ".join(parts[1:]),
            "Occupation": occupation,
            "GeneralSourceReference": "Ambulanspersonalens förhör",
            "AgentClass": "None",
            "bSpawnInSimulation": False,
            "AssociatedVehicleIds": vehicle_ids,
            "SocialGroupId": group_id,
            "GroupLeaderEntityId": entity_id,
            "bFollowGroupLeaderSchedule": False,
            "AutomaticSpeech": [],
            "Timeline": timeline,
            "Notes": notes,
        }
    )
    return row


a951_arrive = "AMBULANCE_951_ARRIVES_CRIME_SCENE"
a912_arrive = "AMBULANCE_912_ARRIVES_CRIME_SCENE"
a951_leave = "AMBULANCE_951_LEAVES_CRIME_SCENE"
a912_leave = "AMBULANCE_912_LEAVES_CRIME_SCENE"

a951 = "AMBULANCE_A951"
a912 = "AMBULANCE_912"

ambulance_people = [
    ambulance_person(
        "PETER_ANDERSSON_AMB",
        "Peter Andersson",
        "AMBULANCE",
        "Ambulansförare",
        [a951],
        "GROUP_AMBULANCE_A951_CREW",
        [
            person_entry(
                "PETER_ANDERSSON_A951_INITIAL",
                "InitialPlacement",
                a951_arrive,
                -90,
                clock(23, 24, 45),
                a951,
                "FRONT_LEFT",
                "RidingVehicle",
                notes="Startar som förare i A951 på väg mot mordplatsen.",
            ),
            person_entry(
                "PETER_ANDERSSON_A951_APPROACH",
                "BeginDriving",
                a951_arrive,
                -89,
                clock(23, 24, 46),
                a951,
                "FRONT_LEFT",
                "RidingVehicle",
                destination_anchor="ANCHOR_TEST_END",
                notes="Kör från Kungsgatan och svänger söderut på Sveavägen.",
            ),
            person_entry(
                "PETER_ANDERSSON_A951_DEPART",
                "BeginDriving",
                a951_leave,
                0,
                clock(23, 29, 30),
                a951,
                "FRONT_LEFT",
                "RidingVehicle",
                destination_anchor="ExitSveavagenN_Car",
                notes="Kör Olof och Lisbeth Palme till Sabbatsbergs sjukhus.",
            ),
        ],
        "Förare i Sollentunaambulansen A951.",
    ),
    ambulance_person(
        "CHRISTER_ERIKSSON_AMB",
        "Christer Eriksson",
        "AMBULANCE",
        "Ambulanssjukvårdare",
        [a951],
        "GROUP_AMBULANCE_A951_CREW",
        [
            person_entry(
                "CHRISTER_ERIKSSON_A951_INITIAL",
                "InitialPlacement",
                a951_arrive,
                -90,
                clock(23, 24, 45),
                a951,
                "PATIENT_BENCH_1",
                "RidingVehicle",
            ),
            person_entry(
                "CHRISTER_ERIKSSON_EXITS_A951",
                "ExitVehicle",
                a951_arrive,
                0,
                clock(23, 26, 15),
                a951,
                "PATIENT_BENCH_1",
                "Standing",
                notes="Tar över de livräddande insatserna vid Olof Palme.",
            ),
            person_entry(
                "CHRISTER_ERIKSSON_ENTERS_A951",
                "EnterVehicle",
                a951_leave,
                -8,
                clock(23, 29, 22),
                a951,
                "PATIENT_BENCH_1",
                "RidingVehicle",
                notes="Fortsätter återupplivningsförsöken i patientutrymmet.",
            ),
        ],
        "Ambulanssjukvårdare i A951; använde svalgtub, syrgas och sug.",
    ),
    ambulance_person(
        "MARIA_DEGERMAN",
        "Maria Degerman",
        "POLICE",
        "Polis och extra ambulansförare",
        [a912],
        "GROUP_AMBULANCE_912_CREW",
        [
            person_entry(
                "MARIA_DEGERMAN_912_INITIAL",
                "InitialPlacement",
                a912_arrive,
                -90,
                clock(23, 26, 30),
                a912,
                "FRONT_LEFT",
                "RidingVehicle",
            ),
            person_entry(
                "MARIA_DEGERMAN_912_APPROACH",
                "BeginDriving",
                a912_arrive,
                -89,
                clock(23, 26, 31),
                a912,
                "FRONT_LEFT",
                "RidingVehicle",
                destination_anchor="ANCHOR_TEST_END",
                notes="Kör 912 från Sabbatsberg via Norra Bantorget och Tunnelgatan.",
            ),
            person_entry(
                "MARIA_DEGERMAN_912_DEPART",
                "BeginDriving",
                a912_leave,
                0,
                clock(23, 30, 15),
                a912,
                "FRONT_LEFT",
                "RidingVehicle",
                destination_anchor="ExitTunnelgatanW_Car",
                notes="Återvänder utan patient tillsammans med Eva Lantz.",
            ),
        ],
        "Polis vid Norrmalmspolisen som arbetade extra som ambulansförare.",
    ),
    ambulance_person(
        "KENNETH_LAVRELL",
        "Kenneth Lavrell",
        "AMBULANCE",
        "Ambulanssjukvårdare",
        [a912, a951],
        "GROUP_AMBULANCE_912_CREW",
        [
            person_entry(
                "KENNETH_LAVRELL_912_INITIAL",
                "InitialPlacement",
                a912_arrive,
                -90,
                clock(23, 26, 30),
                a912,
                "FRONT_RIGHT",
                "RidingVehicle",
            ),
            person_entry(
                "KENNETH_LAVRELL_EXITS_912",
                "ExitVehicle",
                a912_arrive,
                0,
                clock(23, 28, 0),
                a912,
                "FRONT_RIGHT",
                "Standing",
                notes="Lämnar 912 och hjälper till att lyfta in Palmes bår.",
            ),
            person_entry(
                "KENNETH_LAVRELL_ENTERS_A951",
                "EnterVehicle",
                a951_leave,
                -8,
                clock(23, 29, 22),
                a951,
                "PATIENT_BENCH_2",
                "RidingVehicle",
                notes="Följer med A951 och hjälper Christer Eriksson.",
            ),
        ],
        "Kom med 912 men följde med A951 till Sabbatsberg.",
    ),
    ambulance_person(
        "EVA_LANTZ",
        "Eva Lantz",
        "POLICE",
        "Polisassistent och ambulancelev",
        [a912],
        "GROUP_AMBULANCE_912_CREW",
        [
            person_entry(
                "EVA_LANTZ_912_INITIAL",
                "InitialPlacement",
                a912_arrive,
                -90,
                clock(23, 26, 30),
                a912,
                "PATIENT_BENCH_1",
                "RidingVehicle",
            ),
            person_entry(
                "EVA_LANTZ_EXITS_REAR_912",
                "ExitVehicle",
                a912_arrive,
                5,
                clock(23, 28, 5),
                a912,
                "PATIENT_BENCH_1",
                "Standing",
            ),
            person_entry(
                "EVA_LANTZ_ENTERS_FRONT_912",
                "EnterVehicle",
                a912_leave,
                -8,
                clock(23, 30, 7),
                a912,
                "FRONT_RIGHT",
                "RidingVehicle",
                notes="Byter från patientutrymmet till främre passagerarsätet.",
            ),
        ],
        "Polisassistent från VD 1 som följde med 912 som elev.",
    ),
]

people_by_id = {row["EntityId"]: row for row in people}
for row in ambulance_people:
    people_by_id[row["EntityId"]] = row

for entity_id, seat_id, life_state, note in (
    (
        "OLOF_PALME",
        "PATIENT_STRETCHER",
        "Dead",
        "Olof Palme transporteras på bår i A951 till Sabbatsberg.",
    ),
    (
        "LISBET_PALME",
        "FRONT_RIGHT",
        "Alive",
        "Lisbeth Palme följer med A951 i främre passagerarsätet.",
    ),
):
    row = people_by_id[entity_id]
    if a951 not in row["AssociatedVehicleIds"]:
        row["AssociatedVehicleIds"].append(a951)
    entry_id = f"{entity_id}_ENTERS_A951"
    row["Timeline"] = [
        entry for entry in row["Timeline"] if entry.get("EntryId") != entry_id
    ]
    row["Timeline"].append(
        person_entry(
            entry_id,
            "EnterVehicle",
            a951_leave,
            -8,
            clock(23, 29, 22),
            a951,
            seat_id,
            "RidingVehicle",
            life_state=life_state,
            notes=note,
        )
    )

people = list(people_by_id.values())


def spawn_entry(entry_id: str, time: dict, anchor: str, driver: str, passengers: list[str]):
    entry = copy.deepcopy(vehicle_spawn_template)
    entry.update(
        {
            "EntryId": entry_id,
            "Action": "Spawn",
            "Time": time,
            "PlacementMode": "Anchor",
            "PlacementAnchorId": anchor,
            "OrderedLaneIds": [],
            "DriverEntityId": driver,
            "PassengerEntityIds": passengers,
            "Confidence": "Reconstructed",
            "SourceReference": "Ambulanspersonalens förhör",
            "Notes": "Spawnas vid kartinfarten före sitt befintliga ankomst-event.",
        }
    )
    return entry


def ambulance_vehicle(
    vehicle_id: str,
    name: str,
    occupants: list[str],
    driver: str,
    spawn: dict,
    notes: str,
):
    row = copy.deepcopy(vehicle_template)
    row.update(
        {
            "Name": vehicle_id,
            "VehicleId": vehicle_id,
            "DisplayName": name,
            "CategoryId": "AMBULANCE",
            "VehicleCategory": "Ambulance",
            "ModelData": "None",
            "VehicleClass": "None",
            "AssociatedPersonEntityIds": occupants,
            "PrimaryPersonEntityId": driver,
            "KnownDriverEntityId": driver,
            "bSpawnInSimulation": False,
            "Timeline": [spawn],
            "Confidence": "Documented",
            "SourceReference": "Ambulanspersonalens förhör",
            "Notes": notes,
        }
    )
    return row


ambulance_vehicles = [
    ambulance_vehicle(
        a951,
        "Ambulans A951 – Sollentuna",
        [
            "PETER_ANDERSSON_AMB",
            "CHRISTER_ERIKSSON_AMB",
            "KENNETH_LAVRELL",
            "OLOF_PALME",
            "LISBET_PALME",
        ],
        "PETER_ANDERSSON_AMB",
        spawn_entry(
            "AMBULANCE_A951_SPAWN",
            clock(23, 24, 30),
            "EnterKungsgatanE_Car",
            "PETER_ANDERSSON_AMB",
            ["CHRISTER_ERIKSSON_AMB"],
        ),
        "Tar Olof och Lisbeth Palme till Sabbatsbergs sjukhus.",
    ),
    ambulance_vehicle(
        a912,
        "Ambulans 912 – Stockholm",
        ["MARIA_DEGERMAN", "KENNETH_LAVRELL", "EVA_LANTZ"],
        "MARIA_DEGERMAN",
        spawn_entry(
            "AMBULANCE_912_SPAWN",
            clock(23, 25, 30),
            "EnterTunnelgatanW_Car",
            "MARIA_DEGERMAN",
            ["KENNETH_LAVRELL", "EVA_LANTZ"],
        ),
        "Kommer från Sabbatsberg men transporterar ingen patient tillbaka.",
    ),
]

vehicles_by_id = {row["VehicleId"]: row for row in vehicles}
for row in ambulance_vehicles:
    vehicles_by_id[row["VehicleId"]] = row
vehicles = list(vehicles_by_id.values())


def ambulance_group(group_id: str, name: str, members: list[str], leader: str):
    row = copy.deepcopy(group_template)
    row.update(
        {
            "Name": group_id,
            "GroupId": group_id,
            "DisplayName": name,
            "MemberEntityIds": members,
            "LeaderEntityId": leader,
            "Formation": "SideBySide",
            "FormationSpacingCm": 110,
            "bUseLeaderTimeline": False,
            "bCreateAtScenarioStart": False,
            "Confidence": "Documented",
            "SourceReference": "Ambulanspersonalens förhör",
            "Notes": "Fordonsbesättning; personerna använder egna sätes- och eventtidslinjer.",
        }
    )
    return row


ambulance_groups = [
    ambulance_group(
        "GROUP_AMBULANCE_A951_CREW",
        "A951 – ursprunglig besättning",
        ["PETER_ANDERSSON_AMB", "CHRISTER_ERIKSSON_AMB"],
        "PETER_ANDERSSON_AMB",
    ),
    ambulance_group(
        "GROUP_AMBULANCE_912_CREW",
        "912 – besättning och elev",
        ["MARIA_DEGERMAN", "KENNETH_LAVRELL", "EVA_LANTZ"],
        "MARIA_DEGERMAN",
    ),
]
groups_by_id = {row["GroupId"]: row for row in groups}
for row in ambulance_groups:
    groups_by_id[row["GroupId"]] = row
groups = list(groups_by_id.values())


event_updates = {
    a951_arrive: {
        "TimingMode": "Window",
        "AbsoluteTime": clock(23, 26, 15),
        "EarliestTime": clock(23, 26, 0),
        "PreferredTime": clock(23, 26, 15),
        "LatestTime": clock(23, 26, 30),
        "TriggerEventId": "None",
        "MinimumDelaySeconds": 0,
        "PreferredDelaySeconds": 0,
        "MaximumDelaySeconds": 0,
    },
    a912_arrive: {
        "TimingMode": "Window",
        "AbsoluteTime": clock(23, 28, 0),
        "EarliestTime": clock(23, 27, 0),
        "PreferredTime": clock(23, 28, 0),
        "LatestTime": clock(23, 29, 0),
        "TriggerEventId": "None",
        "MinimumDelaySeconds": 0,
        "PreferredDelaySeconds": 0,
        "MaximumDelaySeconds": 0,
    },
    a951_leave: {
        "TimingMode": "Relative",
        "AbsoluteTime": clock(23, 29, 30),
        "EarliestTime": clock(23, 28, 30),
        "PreferredTime": clock(23, 29, 30),
        "LatestTime": clock(23, 30, 30),
        "TriggerEventId": a912_arrive,
        "MinimumDelaySeconds": 90,
        "PreferredDelaySeconds": 90,
        "MaximumDelaySeconds": 90,
    },
    a912_leave: {
        "TimingMode": "Relative",
        "AbsoluteTime": clock(23, 30, 15),
        "EarliestTime": clock(23, 29, 0),
        "PreferredTime": clock(23, 30, 15),
        "LatestTime": clock(23, 31, 30),
        "TriggerEventId": a951_leave,
        "MinimumDelaySeconds": 30,
        "PreferredDelaySeconds": 45,
        "MaximumDelaySeconds": 60,
    },
}
for row in events:
    update = event_updates.get(row["EventId"])
    if update:
        row.update(update)

save("DT_TMOP_People_POLICE_AMBULANCE_IMPORT.json", people)
save("DT_TMOP_HistoricalVehicles_POLICE_AMBULANCE_IMPORT.json", vehicles)
save("DT_TMOP_Groups_POLICE_AMBULANCE_IMPORT.json", groups)
save("DT_TMOP_HistoricalEvents_AMBULANCE_UPDATED.json", events)

