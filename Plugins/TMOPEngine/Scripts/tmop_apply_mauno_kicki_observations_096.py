#!/usr/bin/env python3
"""Apply the sourced Mauno Luukas/Kicki J group and observation scene."""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path


MAUNO_ID = "MAUNO_LUUKAS_JERKER"
KICKI_ID = "KICKI_J"
GROUP_ID = "GROUP_MAUNO_LUUKAS_KICKI_J"
WT_MAN_ID = "THOMAS_PILTZ_MED_WALKIE_TALKIE"
PORT_MAN_ID = "OBSERVED_RUNNING_MAN_ADOLF_FREDRIKS_KYRKOGATA_5_7"
SOURCE = "EBD9631-00-D"


def read_json(path: Path) -> tuple[object, str]:
    raw = path.read_bytes()
    encoding = "utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding)), encoding


def write_json(path: Path, value: object, encoding: str) -> None:
    text = json.dumps(value, ensure_ascii=False, indent=2)
    if encoding == "utf-16":
        path.write_bytes(text.encode("utf-16"))
    else:
        path.write_text(text + "\n", encoding="utf-8")


def find_row(rows: list[dict], entity_id: str) -> dict:
    for row in rows:
        if row.get("EntityId") == entity_id:
            return row
    raise SystemExit(f"Missing person row: {entity_id}")


def timeline_entry(
    template: dict,
    entry_id: str,
    action: str,
    hour: int,
    minute: int,
    second: int,
    anchor_id: str,
    *,
    activity: str = "Walking",
    timing_mode: str = "Absolute",
    shared_event_id: str = "None",
    offset_seconds: int = 0,
    arrival: bool = False,
    confidence: str = "Reconstructed",
    notes: str = "",
) -> dict:
    entry = copy.deepcopy(template)
    entry.update(
        {
            "EntryId": entry_id,
            "Action": action,
            "Time": {"Hour": hour, "Minute": minute, "Second": second},
            "TimingMode": timing_mode,
            "SharedEventId": shared_event_id,
            "EventOffsetSeconds": offset_seconds,
            "bTimeIsArrival": arrival,
            "TravelSpeedOverrideCmPerSecond": 0,
            "LocationType": "Anchor",
            "TargetAnchorId": anchor_id,
            "PassAnchorIds": [],
            "TargetEntityId": "None",
            "TargetSeatId": "None",
            "TargetStopId": "None",
            "TargetGroupId": "None",
            "ActivityState": activity,
            "LifeState": "Alive",
            "bTeleportDuringCatchUp": True,
            "Confidence": confidence,
            "SourceReference": SOURCE,
            "Notes": notes,
        }
    )
    return entry


def update_people(rows: list[dict]) -> None:
    mauno = find_row(rows, MAUNO_ID)
    kicki = find_row(rows, KICKI_ID)
    wt_man = find_row(rows, WT_MAN_ID)
    template = mauno["Timeline"][0]

    for member in (mauno, kicki):
        member["SocialGroupId"] = GROUP_ID
        member["GroupLeaderEntityId"] = MAUNO_ID
        member["GroupFormation"] = "SideBySide"
        member["GroupFormationSpacingCm"] = 110
        member["bFollowGroupLeaderSchedule"] = True

    mauno["Timeline"] = [
        timeline_entry(
            template, "MAUNO_KICKI_INITIAL_2300", "InitialPlacement",
            23, 0, 0, "PUBVaruhus", activity="Walking",
            notes="Spelstart mitt i den rekonstruerade promenaden från Monte Carlo."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_OBSERVE_WT_MAN", "MoveToAnchor",
            23, 17, 0, "Adolffredrikskyrkog_13", arrival=True,
            notes="Rekonstruerat tidsfönster. Paret passerar WT-mannen utan att stanna."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_ENTER_PORT_5_7", "MoveToAnchor",
            23, 20, 30, "Adolffredrikskyrkog_5", arrival=True,
            notes="Paret går in i den olåsta porten 5-7 för att värma sig."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_WAIT_AFTER_RUNNING_MAN", "Wait",
            23, 22, 45, "Adolffredrikskyrkog_5", activity="Idle",
            notes="Efter cirka 20 sekunder rusar en man ut; paret stannar ytterligare ett par minuter."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_REACH_SVEAVAGEN", "MoveToAnchor",
            23, 23, 30, "AFKXSvea_SW", arrival=True,
            notes="Paret når Sveavägen och ser folksamlingen söderut vid Tunnelgatan."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_ARRIVE_CRIME_SCENE", "MoveToAnchor",
            23, 25, 0, "TunnelXSvea_NW", arrival=True,
            notes="Ambulans och möjligen polisbil finns redan på platsen."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_LEAVE_AFTER_AMBULANCE", "Wait",
            23, 29, 0, "TunnelXSvea_NW", activity="Idle",
            timing_mode="Relative", shared_event_id="AMBULANCE_951_LEAVES_CRIME_SCENE",
            notes="Paret stannar tills ambulansen med Olof Palme lämnar platsen."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_ENTER_HOTORGET_METRO", "MoveToAnchor",
            23, 34, 0, "MetroHotorget1", arrival=True,
            timing_mode="Relative", shared_event_id="AMBULANCE_951_LEAVES_CRIME_SCENE",
            offset_seconds=300,
            notes="Paret tar tunnelbanan mot Gamla stan och Kicki J:s bostad."
        ),
        timeline_entry(
            template, "MAUNO_KICKI_LEAVE_GAME_AREA", "Despawn",
            23, 34, 10, "MetroHotorget1", activity="Idle",
            timing_mode="Relative", shared_event_id="AMBULANCE_951_LEAVES_CRIME_SCENE",
            offset_seconds=310,
            notes="Tunnelbaneresan mot Gamla stan ligger utanför spelplanen."
        ),
    ]

    kicki["Timeline"] = [
        timeline_entry(
            template, "KICKI_J_INITIAL_WITH_MAUNO", "InitialPlacement",
            23, 0, 0, "PUBVaruhus", activity="Walking",
            notes="Följer gruppledaren Mauno Luukas genom hela scenen."
        )
    ]

    wt_man["CategoryId"] = "OBSERVED_UNKNOWN"
    wt_man["FullName"] = (
        'NSLOCTEXT("DT_TMOP_People", "OBSERVED_WT_MAN_AFK_13_FullName", '
        '"Okänd man med walkie-talkie, Adolf Fredriks kyrkogata 13")'
    )
    wt_man["HeightCentimeters"] = 187
    wt_man["BodyBuildCategory"] = "Athletic"
    wt_man["Glasses"]["OriginalText"] = "Sportaktiga pilotglasögon"
    wt_man["Glasses"]["NormalizedValue"] = "Pilot glasses"
    wt_man["Glasses"]["Confidence"] = "Documented"
    wt_man["Glasses"]["SourceReference"] = SOURCE
    wt_man["BodyBuild"]["OriginalText"] = "Stor, kraftig och atletiskt byggd"
    wt_man["BodyBuild"]["NormalizedValue"] = "AthleticLarge"
    wt_man["BodyBuild"]["Confidence"] = "Documented"
    wt_man["BodyBuild"]["SourceReference"] = SOURCE
    wt_man["JacketOrCoat"]["OriginalText"] = "Bylsig klädsel, uppgift om skinnjacka"
    wt_man["JacketOrCoat"]["NormalizedValue"] = "BulkyLeatherJacket"
    wt_man["JacketOrCoat"]["Confidence"] = "Documented"
    wt_man["JacketOrCoat"]["SourceReference"] = SOURCE
    wt_man["OtherCharacteristics"]["OriginalText"] = (
        "Stor rödaktig walkie-talkie i beredskapsställning; ingen hörbar radiotrafik"
    )
    wt_man["OtherCharacteristics"]["NormalizedValue"] = "RedWalkieTalkie"
    wt_man["OtherCharacteristics"]["Confidence"] = "Documented"
    wt_man["OtherCharacteristics"]["SourceReference"] = SOURCE
    wt_man["Timeline"] = [
        timeline_entry(
            template, "OBSERVED_WT_MAN_AFK_13_SPAWNS", "Spawn",
            23, 16, 30, "Adolffredrikskyrkog_13", activity="Standing",
            notes="Anonym observationsfigur; identiteten är inte fastställd."
        ),
        timeline_entry(
            template, "OBSERVED_WT_MAN_AFK_13_DESPAWNS", "Despawn",
            23, 18, 0, "Adolffredrikskyrkog_13", activity="Standing",
            notes="Observationens dokumenterade tidsfönster är slut."
        ),
    ]

    if any(row.get("EntityId") == PORT_MAN_ID for row in rows):
        port_man = find_row(rows, PORT_MAN_ID)
    else:
        port_man = copy.deepcopy(wt_man)
        rows.append(port_man)

    port_man["Name"] = PORT_MAN_ID
    port_man["EntityId"] = PORT_MAN_ID
    port_man["CategoryId"] = "OBSERVED_UNKNOWN"
    port_man["FullName"] = (
        'NSLOCTEXT("DT_TMOP_People", "OBSERVED_RUNNING_MAN_AFK_5_7_FullName", '
        '"Okänd springande man, Adolf Fredriks kyrkogata 5-7")'
    )
    port_man["HeightCentimeters"] = 0
    port_man["GeneralSourceReference"] = (
        "https://wpu.nu/wiki/Mauno_Luukas; EBD9631-00-D"
    )
    port_man["Uppslag"] = "EBD9631-00-D"
    for field in (
        "Hair", "Headwear", "BeardOrMustache", "FaceShape", "Nose",
        "BodyBuild", "JacketOrCoat", "ShirtOrSweater", "Trousers",
        "Shoes", "Scarf", "Glasses", "OtherCharacteristics"
    ):
        port_man[field] = {
            "OriginalText": "",
            "NormalizedValue": "None",
            "Tags": [],
            "Confidence": "Documented",
            "SourceReference": SOURCE,
        }
    port_man["BodyBuildCategory"] = "Unknown"
    port_man["HairColorCategory"] = "Unknown"
    port_man["HeadwearCategory"] = "Unknown"
    port_man["FacialHairCategory"] = "Unknown"
    port_man["OuterwearCategory"] = "Unknown"
    port_man["Timeline"] = [
        timeline_entry(
            template, "OBSERVED_RUNNING_MAN_AFK_5_7_SPAWNS", "Spawn",
            23, 20, 40, "Adolffredrikskyrkog_5", activity="Running",
            notes="Mannen hörs och ses rusa nedför trappan i den mörka porten."
        ),
        timeline_entry(
            template, "OBSERVED_RUNNING_MAN_AFK_5_7_DESPAWNS", "Despawn",
            23, 21, 5, "Adolffredrikskyrkog_5", activity="Running",
            notes="Riktningen efter att mannen lämnat porten är inte dokumenterad."
        ),
    ]
    port_man["Notes"] = (
        "Anonym observationsfigur. Mauno Luukas och Kicki J uppgav att mannen "
        "rusade nedför trappan, nästan sammanstötte med dem och flydde ut på "
        "Adolf Fredriks kyrkogata. Inget användbart signalement kunde lämnas."
    )


def update_groups(rows: list[dict]) -> None:
    group = {
        "Name": GROUP_ID,
        "GroupId": GROUP_ID,
        "DisplayName": "Mauno Luukas och Kicki J",
        "MemberEntityIds": [MAUNO_ID, KICKI_ID],
        "LeaderEntityId": MAUNO_ID,
        "Formation": "SideBySide",
        "FormationSpacingCm": 110,
        "bUseLeaderTimeline": True,
        "bCreateAtScenarioStart": True,
        "Confidence": "Documented",
        "SourceReference": SOURCE,
        "Notes": (
            "Mauno Luukas är teknisk gruppledare. Kicki J följer hans timeline "
            "från promenaden till mordplatsen och tunnelbanan."
        ),
    }
    rows[:] = [row for row in rows if row.get("GroupId") != GROUP_ID]
    rows.append(group)
    rows.sort(key=lambda row: row.get("Name", ""))


def update_observations(rows: list[dict]) -> None:
    new_rows = [
        {
            "Name": "OBS_MAUNO_KICKI_WT_MAN_AFK_13",
            "ObservationId": "OBS_MAUNO_KICKI_WT_MAN_AFK_13",
            "DisplayName": "Mauno och Kicki passerar mannen med walkie-talkie",
            "bEnabled": True,
            "ObserverEntityIds": [MAUNO_ID, KICKI_ID],
            "ObservedEntityId": WT_MAN_ID,
            "ObservedEntityType": "Person",
            "ObservationAnchorId": "Adolffredrikskyrkog_13",
            "TimingMode": "Absolute",
            "CanonicalTime": {"Hour": 23, "Minute": 17, "Second": 0},
            "ReferenceSharedEventId": "None",
            "ReferenceOffsetSeconds": 0,
            "ObservationDurationSeconds": 60,
            "ObservationRadiusCm": 2500.0,
            "bRequireObserverNearAnchor": True,
            "bRequireObservedEntityNearAnchor": True,
            "bRequiresLineOfSight": True,
            "Confidence": "Reconstructed",
            "SourceReference": SOURCE,
            "ObservedDescription": (
                "Man, cirka 184-190 cm, stor/kraftig och atletisk, bylsig "
                "klädsel, pilotglasögon och stor rödaktig walkie-talkie."
            ),
            "Notes": (
                "Tiden är rekonstruerad. Mauno pekade långt senare ut mannen "
                "som polisman D/Thomas Piltz; identifikationen är inte fastställd."
            ),
        },
        {
            "Name": "OBS_MAUNO_KICKI_RUNNING_MAN_AFK_5_7",
            "ObservationId": "OBS_MAUNO_KICKI_RUNNING_MAN_AFK_5_7",
            "DisplayName": "Mauno och Kicki möter mannen som rusar ur porten",
            "bEnabled": True,
            "ObserverEntityIds": [MAUNO_ID, KICKI_ID],
            "ObservedEntityId": PORT_MAN_ID,
            "ObservedEntityType": "Person",
            "ObservationAnchorId": "Adolffredrikskyrkog_5",
            "TimingMode": "RelativeToSharedEvent",
            "CanonicalTime": {"Hour": 23, "Minute": 20, "Second": 50},
            "ReferenceSharedEventId": "Palme_shot_1",
            "ReferenceOffsetSeconds": -40,
            "ObservationDurationSeconds": 25,
            "ObservationRadiusCm": 1200.0,
            "bRequireObserverNearAnchor": True,
            "bRequireObservedEntityNearAnchor": True,
            "bRequiresLineOfSight": False,
            "Confidence": "Reconstructed",
            "SourceReference": SOURCE,
            "ObservedDescription": (
                "Okänd man rusar nedför trappan, nästan kolliderar med paret "
                "och flyr ut ur porten. Inget signalement uppfattas."
            ),
            "Notes": (
                "Mörk port; fri sikt krävs därför inte. Mannen är en separat "
                "observationsperson och inte sammanlänkad med WT-mannen."
            ),
        },
    ]
    ids = {row["ObservationId"] for row in new_rows}
    rows[:] = [row for row in rows if row.get("ObservationId") not in ids]
    rows.extend(new_rows)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--people", type=Path, required=True)
    parser.add_argument("--groups", type=Path, required=True)
    parser.add_argument("--observations", type=Path, required=True)
    args = parser.parse_args()

    people, people_encoding = read_json(args.people)
    groups, groups_encoding = read_json(args.groups)
    observations, observations_encoding = read_json(args.observations)

    update_people(people)
    update_groups(groups)
    update_observations(observations)

    write_json(args.people, people, people_encoding)
    write_json(args.groups, groups, groups_encoding)
    write_json(args.observations, observations, observations_encoding)
    print("Updated Mauno, Kicki, two observed men, group and observations.")


if __name__ == "__main__":
    main()
