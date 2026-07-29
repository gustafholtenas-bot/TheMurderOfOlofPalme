#!/usr/bin/env python3
"""Add the three sourced Grand staring-man observations."""

from __future__ import annotations

import argparse
import copy
import json
from pathlib import Path


ANNELI_ID = "ANNELI_KORHONEN"
MARGARETA_ID = "MARGARETA_STORHOG"
MARTEN_ID = "MARTEN_PALME"
L866_MAN_ID = "STIRRANDE_MAN"
MARTEN_MAN_ID = "OBSERVED_GRANDMAN_MARTEN_SANDINS"


def read_json(path: Path) -> tuple[list[dict], str]:
    raw = path.read_bytes()
    encoding = "utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding)), encoding


def write_json(path: Path, rows: list[dict], encoding: str) -> None:
    text = json.dumps(rows, ensure_ascii=False, indent=2)
    if encoding == "utf-16":
        path.write_bytes(text.encode("utf-16"))
    else:
        path.write_text(text + "\n", encoding="utf-8")


def find_person(rows: list[dict], entity_id: str) -> dict:
    for row in rows:
        if row.get("EntityId") == entity_id:
            return row
    raise SystemExit(f"Missing person row: {entity_id}")


def entry(
    template: dict,
    entry_id: str,
    action: str,
    time: tuple[int, int, int],
    anchor: str,
    *,
    activity: str = "Walking",
    arrival: bool = False,
    notes: str = "",
    source: str = "",
) -> dict:
    value = copy.deepcopy(template)
    value.update(
        {
            "EntryId": entry_id,
            "Action": action,
            "Time": {"Hour": time[0], "Minute": time[1], "Second": time[2]},
            "TimingMode": "Absolute",
            "SharedEventId": "None",
            "EventOffsetSeconds": 0,
            "bTimeIsArrival": arrival,
            "TravelSpeedOverrideCmPerSecond": 0,
            "LocationType": "Anchor",
            "TargetAnchorId": anchor,
            "PassAnchorIds": [],
            "TargetEntityId": "None",
            "TargetSeatId": "None",
            "TargetStopId": "None",
            "TargetGroupId": "None",
            "ActivityState": activity,
            "LifeState": "Alive",
            "bTeleportDuringCatchUp": True,
            "Confidence": "Reconstructed",
            "SourceReference": source,
            "Notes": notes,
        }
    )
    return value


def blank_characteristic(source: str) -> dict:
    return {
        "OriginalText": "",
        "NormalizedValue": "None",
        "Tags": [],
        "Confidence": "Documented",
        "SourceReference": source,
    }


def set_characteristic(
    row: dict, field: str, original: str, normalized: str, source: str
) -> None:
    row[field] = {
        "OriginalText": original,
        "NormalizedValue": normalized,
        "Tags": [],
        "Confidence": "Documented",
        "SourceReference": source,
    }


def update_people(rows: list[dict]) -> None:
    anneli = find_person(rows, ANNELI_ID)
    margareta = find_person(rows, MARGARETA_ID)
    marten = find_person(rows, MARTEN_ID)
    staring_man = find_person(rows, L866_MAN_ID)
    template = staring_man["Timeline"][0]

    anneli["GeneralSourceReference"] = "https://wpu.nu/wiki/Uppslag:L866-00"
    anneli["Uppslag"] = "L866-00"
    margareta["GeneralSourceReference"] = "https://wpu.nu/wiki/Margareta_Storhök"
    margareta["Uppslag"] = "L866-01"

    # Anneli and Margareta already reach grand_outside together at 23:05:33.
    # Their existing cinema/toilet exit timelines are therefore retained.

    # Correct Mårten's post-film order: Grand -> bookshop -> Sandins -> north.
    prefix = marten["Timeline"][:4]
    marten_template = marten["Timeline"][3]
    marten["Timeline"] = prefix + [
        entry(
            marten_template, "MARTEN_AT_KULTURCIRKELN_WITH_PARENTS",
            "MoveToAnchor", (23, 12, 34), "kulturcirkeln", arrival=True,
            source="T2-00",
            notes="Sällskapet samtalar cirka 2-3 minuter vid bokhandeln Cirkeln."
        ),
        entry(
            marten_template, "MARTEN_LEAVES_PARENTS_AT_KULTURCIRKELN",
            "Wait", (23, 15, 8), "kulturcirkeln", activity="Idle",
            source="T2-00",
            notes="Mårten och Ingrid tar farväl av Olof och Lisbeth."
        ),
        entry(
            marten_template, "MARTEN_PASSES_GRANDMAN_AT_SANDINS",
            "MoveToAnchor", (23, 15, 35), "Sadins", arrival=True,
            source="T2-00",
            notes="På vägen norrut passerar Mårten mannen vid Sandins skyltfönster."
        ),
        entry(
            marten_template, "MARTEN_CONTINUES_NORTH_FROM_GRAND",
            "MoveToAnchor", (23, 17, 2), "Telefonkiosk1", arrival=True,
            source="T2-00",
            notes="Mårten och Ingrid fortsätter norrut mot Rådmansgatans tunnelbana."
        ),
        entry(
            marten_template, "MARTEN_ENTERS_RADMANSGATAN_METRO",
            "MoveToAnchor", (23, 19, 59), "MetroRadmansgatan1", arrival=True,
            source="T2-00",
            notes="Sällskapet lämnar spelområdet via tunnelbanan."
        ),
    ]

    staring_man["CategoryId"] = "OBSERVED_UNKNOWN"
    staring_man["FullName"] = (
        'NSLOCTEXT("DT_TMOP_People", "STIRRANDE_MAN_L866_FullName", '
        '"Okänd stirrande man utanför Grand")'
    )
    staring_man["Gender"] = "Male"
    staring_man["AgeAtEvent"] = 40
    staring_man["HeightCentimeters"] = 173
    staring_man["GeneralSourceReference"] = (
        "https://wpu.nu/wiki/Uppslag:L866-00; "
        "https://wpu.nu/wiki/Uppslag:L866-01"
    )
    staring_man["Uppslag"] = "L866"
    staring_man["BodyBuildCategory"] = "Average"
    staring_man["HeadwearCategory"] = "Cap"
    staring_man["OuterwearCategory"] = "Jacket"
    set_characteristic(
        staring_man, "Hair", "Troligen cendréfärgat hår", "Cendre", "L866"
    )
    set_characteristic(
        staring_man, "Headwear",
        "Tjock vaddfylld skärmmössa typ keps, eventuellt ljusbrun",
        "PaddedCap", "L866"
    )
    set_characteristic(
        staring_man, "Glasses", "Troligen glasögon", "Glasses", "L866"
    )
    set_characteristic(
        staring_man, "BodyBuild",
        "Ordinär kroppsbyggnad, något rund om magen",
        "AverageSlightBelly", "L866"
    )
    set_characteristic(
        staring_man, "JacketOrCoat",
        "Trekvartslång blå jacka", "BlueThreeQuarterJacket", "L866"
    )
    set_characteristic(
        staring_man, "Trousers", "Mörkblå byxor", "DarkBlueTrousers", "L866"
    )
    set_characteristic(
        staring_man, "OtherCharacteristics",
        "Stod utanför Grand och stirrade in i den tomma foajén",
        "IntenseStaring", "L866"
    )
    staring_man["Timeline"] = [
        entry(
            template, "STIRRANDE_MAN_L866_SPAWNS", "Spawn",
            (23, 4, 45), "grand_outside", activity="Standing",
            source="L866",
            notes="Anonym observationsfigur utanför Grands entré."
        ),
        entry(
            template, "STIRRANDE_MAN_L866_DESPAWNS", "Despawn",
            (23, 6, 15), "grand_outside", activity="Standing",
            source="L866",
            notes="Den dokumenterade L866-observationen är slut."
        ),
    ]
    staring_man["Notes"] = (
        "Anonym observationsfigur sedd samtidigt av Anneli Korhonen och "
        "Margareta Storhök. Ska inte automatiskt identifieras med Mårtens "
        "Grandman eller Lars Eric Erikssons väntande man."
    )

    if any(row.get("EntityId") == MARTEN_MAN_ID for row in rows):
        marten_man = find_person(rows, MARTEN_MAN_ID)
    else:
        marten_man = copy.deepcopy(staring_man)
        rows.append(marten_man)

    marten_man["Name"] = MARTEN_MAN_ID
    marten_man["EntityId"] = MARTEN_MAN_ID
    marten_man["CategoryId"] = "OBSERVED_UNKNOWN"
    marten_man["FullName"] = (
        'NSLOCTEXT("DT_TMOP_People", "OBSERVED_GRANDMAN_MARTEN_SANDINS_FullName", '
        '"Okänd man vid Sandins, observerad av Mårten Palme")'
    )
    marten_man["AgeAtEvent"] = 40
    marten_man["HeightCentimeters"] = 0
    marten_man["GeneralSourceReference"] = "https://wpu.nu/wiki/Mårten_Palme"
    marten_man["Uppslag"] = "T2-00"
    for field in (
        "Hair", "Headwear", "BeardOrMustache", "FaceShape", "Nose",
        "BodyBuild", "JacketOrCoat", "ShirtOrSweater", "Trousers",
        "Shoes", "Scarf", "Glasses", "OtherCharacteristics"
    ):
        marten_man[field] = blank_characteristic("T2-00")
    marten_man["BodyBuildCategory"] = "Heavy"
    marten_man["HeadwearCategory"] = "Cap"
    marten_man["OuterwearCategory"] = "Jacket"
    set_characteristic(
        marten_man, "Headwear", "Keps med knäppe upptill", "ButtonTopCap", "T2-00"
    )
    set_characteristic(
        marten_man, "Glasses",
        "Eventuellt stålbågade glasögon", "MetalFrameGlasses", "T2-00"
    )
    set_characteristic(
        marten_man, "BodyBuild",
        "Kraftig kroppsbyggnad", "Heavy", "T2-00"
    )
    set_characteristic(
        marten_man, "JacketOrCoat",
        "Lång blå täckjacka", "LongBluePaddedJacket", "T2-00"
    )
    set_characteristic(
        marten_man, "OtherCharacteristics",
        "Framåtlutad, bufflig; stod vid skyltfönstret och började därefter gå",
        "StoopedBulkyWindowWatcher", "T2-00"
    )
    marten_man["Timeline"] = [
        entry(
            template, "OBSERVED_GRANDMAN_MARTEN_SPAWNS", "Spawn",
            (23, 14, 50), "Sadins", activity="Standing",
            source="T2-00",
            notes="Anonym observationsfigur vid Sandins möbelaffär."
        ),
        entry(
            template, "OBSERVED_GRANDMAN_MARTEN_DESPAWNS", "Despawn",
            (23, 16, 30), "Sadins", activity="Walking",
            source="T2-00",
            notes="Fortsatt rörelse efter observationen modelleras först vid en särskild hypotes."
        ),
    ]
    marten_man["Notes"] = (
        "Anonym observationsfigur sedd av Mårten Palme. Likheter finns med "
        "L866-mannen, men identiteten är inte fastställd."
    )


def observation(
    observation_id: str,
    display_name: str,
    observer_id: str,
    observed_id: str,
    anchor: str,
    time: tuple[int, int, int],
    source: str,
    description: str,
    notes: str,
) -> dict:
    return {
        "Name": observation_id,
        "ObservationId": observation_id,
        "DisplayName": display_name,
        "bEnabled": True,
        "ObserverEntityIds": [observer_id],
        "ObservedEntityId": observed_id,
        "ObservedEntityType": "Person",
        "ObservationAnchorId": anchor,
        "TimingMode": "Absolute",
        "CanonicalTime": {"Hour": time[0], "Minute": time[1], "Second": time[2]},
        "ReferenceSharedEventId": "None",
        "ReferenceOffsetSeconds": 0,
        "ObservationDurationSeconds": 30,
        "ObservationRadiusCm": 1800.0,
        "bRequireObserverNearAnchor": True,
        "bRequireObservedEntityNearAnchor": True,
        "bRequiresLineOfSight": True,
        "Confidence": "Reconstructed",
        "SourceReference": source,
        "ObservedDescription": description,
        "Notes": notes,
    }


def update_observations(rows: list[dict]) -> None:
    l866_description = (
        "Man cirka 35-45 år, 170-175 cm, troligen glasögon och skärmmössa, "
        "trekvartslång blå jacka; stirrar in i Grands tomma foajé."
    )
    new_rows = [
        observation(
            "OBS_ANNELI_KORHONEN_STIRRANDE_MAN_GRAND",
            "Anneli Korhonen ser den stirrande mannen",
            ANNELI_ID, L866_MAN_ID, "grand_outside", (23, 5, 33), "L866-00",
            l866_description,
            "Samma samtidiga L866-observation som Margareta Storhöks rad."
        ),
        observation(
            "OBS_MARGARETA_STORHOG_STIRRANDE_MAN_GRAND",
            "Margareta Storhök ser den stirrande mannen",
            MARGARETA_ID, L866_MAN_ID, "grand_outside", (23, 5, 33), "L866-01",
            l866_description,
            "Samma samtidiga L866-observation som Anneli Korhonens rad."
        ),
        observation(
            "OBS_MARTEN_PALME_GRANDMAN_SANDINS",
            "Mårten Palme ser Grandmannen vid Sandins",
            MARTEN_ID, MARTEN_MAN_ID, "Sadins", (23, 15, 35), "T2-00",
            (
                "Man omkring 40 år, kraftig, möjligen stålbågade glasögon, "
                "keps och lång blå täckjacka; framåtlutad och bufflig."
            ),
            "Möjlig men inte fastställd identitet med L866-mannen."
        ),
    ]
    ids = {row["ObservationId"] for row in new_rows}
    rows[:] = [row for row in rows if row.get("ObservationId") not in ids]
    rows.extend(new_rows)


def update_links(rows: list[dict]) -> None:
    link_id = "LINK_L866_STIRRANDE_MAN_TO_MARTEN_GRANDMAN"
    rows[:] = [row for row in rows if row.get("LinkId") != link_id]
    rows.append(
        {
            "Name": link_id,
            "LinkId": link_id,
            "FromObservationId": "OBS_ANNELI_KORHONEN_STIRRANDE_MAN_GRAND",
            "ToObservationId": "OBS_MARTEN_PALME_GRANDMAN_SANDINS",
            "Relationship": "ProbableSamePerson",
            "ConfidenceScore": 0.7,
            "SupportingFactors": [
                "SameArea",
                "TenMinuteInterval",
                "SimilarAge",
                "BlueLongJacket",
                "Cap",
                "PossibleGlasses",
            ],
            "ContradictingFactors": [
                "HeightEstimateDiffers",
                "BodyBuildDescriptionDiffers",
            ],
            "RouteAnchorIds": ["grand_outside", "Sadins"],
            "CalculatedDistanceCm": 0.0,
            "RequiredAverageSpeedCmPerSecond": 0.0,
            "AlternativeRoutes": [],
            "bPlayerCreatedHypothesis": False,
            "bVisibleOnlyInInvestigationMode": True,
            "Notes": (
                "Källorna L866 och T2-00 dokumenterar två observationer. "
                "Länken uttrycker endast hypotesen att de kan avse samma man."
            ),
        }
    )


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--people", type=Path, required=True)
    parser.add_argument("--observations", type=Path, required=True)
    parser.add_argument("--links", type=Path, required=True)
    args = parser.parse_args()

    people, people_encoding = read_json(args.people)
    observations, observations_encoding = read_json(args.observations)
    links, links_encoding = read_json(args.links)
    update_people(people)
    update_observations(observations)
    update_links(links)
    write_json(args.people, people, people_encoding)
    write_json(args.observations, observations, observations_encoding)
    write_json(args.links, links, links_encoding)
    print("Updated three Grand observations and corrected Mårten's route.")


if __name__ == "__main__":
    main()
