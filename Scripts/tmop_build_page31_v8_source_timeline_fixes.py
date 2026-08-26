#!/usr/bin/env python3
"""Build cumulative Page31 v8 tables with source-checked legacy-person fixes.

The v8 package starts from the complete v7 package. It only changes the six
legacy/observation persons found at or near the murder scene in validation
TimelineValidation_20260826_194615 and adds the corresponding observation
table corrections.
"""

from __future__ import annotations

import copy
import json
import shutil
from pathlib import Path


REPO = Path(__file__).resolve().parents[1]
WORKSPACE = REPO.parent
V7 = WORKSPACE / "work" / "TMOP_Page31_Logic_0826_v7"
OUT = WORKSPACE / "work" / "TMOP_Page31_Logic_0826_v8"
OBSERVATIONS_IN = REPO / "DataTables/08_23/DT_TMOP_Observations.json"
SHOT_EVENT = "Palme_shot_1"
SHOT_SECOND = 23 * 3600 + 21 * 60 + 30


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


def source_entry(
    template: dict,
    entry_id: str,
    offset: int,
    action: str,
    target: str = "None",
    activity: str = "Standing",
    notes: str = "",
    *,
    teleport_on_catchup: bool = False,
) -> dict:
    entry = copy.deepcopy(template)
    entry.update({
        "EntryId": entry_id,
        "Action": action,
        "Time": preview_time(offset),
        "TimingMode": "Relative",
        "SharedEventId": SHOT_EVENT,
        "EventOffsetSeconds": offset,
        "bTimeIsArrival": action == "MoveToAnchor",
        "LocationType": "Anchor" if target != "None" else "Unknown",
        "TargetAnchorId": target,
        "PassAnchorIds": [],
        "TargetEntityId": "None",
        "TargetSeatId": "None",
        "ActivityState": activity,
        "bTeleportDuringCatchUp": teleport_on_catchup,
        "bSupersedeActiveMovementWhenDue": True,
        "Confidence": "WitnessUncertain",
        "SourceReference": notes,
        "Notes": notes,
    })
    return entry


def append_note(person: dict, note: str) -> None:
    existing = str(person.get("Notes", "")).strip()
    person["Notes"] = f"{existing} | {note}" if existing else note


def make_people(people: list[dict]) -> list[str]:
    by_id = {person["EntityId"]: person for person in people}
    changed: list[str] = []

    witness = by_id["VITTNE_EAD9976_01_MASKERAD_KVINNLIG_BESOKARE_1"]
    witness_templates = witness["Timeline"]
    witness["SocialGroupId"] = "None"
    witness["GroupLeaderEntityId"] = "None"
    witness["bFollowGroupLeaderSchedule"] = False
    witness["Timeline"] = [
        source_entry(
            witness_templates[0], "EAD9976_05_WITNESS_START_TEGNER", -570,
            "InitialPlacement", "TegnerXSvea", "Walking",
            "EAD9976-05; reconstructed start for the walk south from Tegnergatan. Exact start second is unknown.",
            teleport_on_catchup=True,
        ),
        source_entry(
            witness_templates[1], "EAD9976_05_WITNESS_REACHES_TUNNEL", -300,
            "MoveToAnchor", "TunnelXSvea_NE", "Walking",
            "EAD9976-01-A and EAD9976-05 place the separation/conversation near the escalator about five minutes before the murder.",
        ),
        source_entry(
            witness_templates[2], "EAD9976_05_WITNESS_TALKS_AT_TUNNEL", -300,
            "Wait", "None", "Standing",
            "EAD9976-05 says the women talked for a while. Wait holds the live arrival position and has no competing destination.",
        ),
        source_entry(
            witness_templates[3], "EAD9976_05_WITNESS_ASCENDS_STAIRS", -150,
            "MoveToAnchor", "TunnelgatanStairsTop", "Walking",
            "EAD9976-05: the witness probably took the escalator, looked back from above and saw the intersection empty.",
        ),
        source_entry(
            witness_templates[3], "EAD9976_05_WITNESS_LEAVES_CRITICAL_MAP", -120,
            "Despawn", "TunnelgatanStairsTop", "Walking",
            "Observation proxy ends after the witness has left the critical street area toward home; she did not hear the shots.",
        ),
    ]
    append_note(witness, "v8: independent source-timed route; absent from the murder corner at T0.")
    changed.append(witness["EntityId"])

    companion = by_id["VITTNE_EAD9976_01_MASKERAD_BESOKARE_2"]
    companion_templates = companion["Timeline"]
    companion["SocialGroupId"] = "None"
    companion["GroupLeaderEntityId"] = "None"
    companion["bFollowGroupLeaderSchedule"] = False
    companion["Timeline"] = [
        source_entry(
            companion_templates[0], "EAD9976_05_COMPANION_START_TEGNER", -570,
            "InitialPlacement", "TegnerXSvea", "Walking",
            "EAD9976-05; walks south with the interviewed witness. Exact start second is reconstructed.",
            teleport_on_catchup=True,
        ),
        source_entry(
            companion_templates[1], "EAD9976_05_COMPANION_REACHES_TUNNEL", -300,
            "MoveToAnchor", "TunnelXSvea_NE", "Walking",
            "EAD9976-01-A and EAD9976-05: reaches the escalator/intersection about five minutes before the murder.",
        ),
        source_entry(
            companion_templates[2], "EAD9976_05_COMPANION_RETURNS_TO_LA_CARTERIE", -180,
            "MoveToAnchor", "LaCarterie_entrance", "Walking",
            "EAD9976-01-A says the women separated and the companion probably returned to the shop. Decorimahornet is no longer used as a proxy.",
        ),
        source_entry(
            companion_templates[2], "EAD9976_05_COMPANION_ENTERS_LA_CARTERIE", -170,
            "Despawn", "LaCarterie_entrance", "Walking",
            "Exterior observation proxy ends at the La Carterie entrance; the source supports a probable return indoors, not presence at the murder corner.",
        ),
    ]
    append_note(companion, "v8: no Decorima proxy; probable return toward La Carterie before T0.")
    changed.append(companion["EntityId"])

    masked = by_id["MASKED_WOMAN_DECORIMA_EAD9976"]
    masked_templates = masked["Timeline"]
    masked["GeneralSourceReference"] = (
        "EAD9976-04; https://wpu.nu/wiki/Uppslag:EAD9976-04; "
        "Pol-1993-09-09 EAD9976-00 Förhör-personal-la-Carterie.pdf"
    )
    masked["Uppslag"] = "EAD9976-04"
    masked["bFollowGroupLeaderSchedule"] = False
    masked["Timeline"] = [
        source_entry(
            masked_templates[0], "EAD9976_ARRIVES_DECORIMA", -1890,
            "InitialPlacement", "Dekorimahornet", "Standing",
            "EAD9976-04 documents an approximately ten-minute stop at the former Beckers/Decorima. The 22:50 start remains a low-confidence reconstruction.",
            teleport_on_catchup=True,
        ),
        source_entry(
            masked_templates[-1], "EAD9976_LEAVES_DECORIMA_OFFSCREEN", -1290,
            "Despawn", "Dekorimahornet", "Walking",
            "EAD9976-04 says she left directly for home after about ten minutes. Her masked route is not invented; the exterior proxy ends at 23:00.",
        ),
    ]
    append_note(masked, "v8: source corrected to EAD9976-04; physical proxy lasts only for the documented ten-minute stop.")
    changed.append(masked["EntityId"])

    # E11626 describes a man with an object at the corner immediately before
    # the shot. This is retained as an observation of THE_KILLER and must not
    # create a second physical man beside the scenario perpetrator.
    man4 = by_id["OBSERVED_D21659_P15_E11626_MAN_4"]
    man4["bSpawnInSimulation"] = False
    man4["Timeline"] = []
    man4["bFollowGroupLeaderSchedule"] = False
    append_note(man4, "v8: non-spawning source record; OBS_D21659_P15_E11626_DEKORIMA_OBJECT_MAN targets THE_KILLER for T-10..T+5.")
    changed.append(man4["EntityId"])

    # EBE2354 is second-hand information from an unidentified girl and gives
    # neither a usable exact time nor a precise location.
    talker = by_id["UNKNOWN_PERSON_TALKING_TO_OLOF_EBE2354"]
    talker["bSpawnInSimulation"] = False
    talker["Timeline"] = []
    talker["bFollowGroupLeaderSchedule"] = False
    append_note(talker, "v8: evidence-only/hypothesis record; no canonical physical spawn without an exact time or place.")
    changed.append(talker["EntityId"])

    mustached = by_id["UNKNOWN_MUSTACHED_MAN_EAD622"]
    mustached_templates = mustached["Timeline"]
    mustached["bFollowGroupLeaderSchedule"] = False
    mustached["Timeline"] = [
        source_entry(
            mustached_templates[0], "UNKNOWN_MUSTACHED_MAN_EAD622_EXITS_CAR", -690,
            "InitialPlacement", "EAD622_TUNNELGATAN_OLOFSGATAN_DROPOFF", "Walking",
            "EAD622-00: reconstructed first sighting when the man exits the rear of a stopped car on Tunnelgatan.",
            teleport_on_catchup=True,
        ),
        source_entry(
            mustached_templates[1], "UNKNOWN_MUSTACHED_MAN_EAD622_WATCHES_SVEAVAGEN", -270,
            "MoveToAnchor", "TunnelXSvea_NW", "Standing",
            "EAD622-00: Wiklund sees the same man standing at the north-west corner at about 23:17.",
        ),
        source_entry(
            mustached_templates[1], "UNKNOWN_MUSTACHED_MAN_EAD622_OBSERVATION_ENDS", -240,
            "Despawn", "TunnelXSvea_NW", "Standing",
            "The observation proxy ends thirty seconds after the last supported sighting. This does not assert that the real man left.",
        ),
    ]
    append_note(mustached, "v8: visual presence ends after the last source-supported observation, before T0.")
    changed.append(mustached["EntityId"])

    return changed


def make_observations(observations: list[dict]) -> list[str]:
    by_id = {row["ObservationId"]: row for row in observations}
    changed: list[str] = []

    e11626 = by_id["OBS_D21659_P15_E11626_DEKORIMA_OBJECT_MAN"]
    e11626.update({
        "ObservedEntityId": "THE_KILLER",
        "TimingMode": "Relative",
        "CanonicalTime": preview_time(-10),
        "ReferenceSharedEventId": SHOT_EVENT,
        "ReferenceOffsetSeconds": -10,
        "ObservationDurationSeconds": 15,
        "Confidence": "Low",
        "Notes": (
            "The man with an object is represented as a low-confidence observation "
            "of THE_KILLER from T-10 to T+5, not as an additional physical man. "
            "The object is not asserted to be a radio or weapon."
        ),
    })
    changed.append(e11626["ObservationId"])

    ead622_exit = by_id["OBS_EAD622_WIKLUND_SEES_MAN_EXIT_CAR"]
    ead622_exit.update({
        "TimingMode": "Relative",
        "CanonicalTime": preview_time(-690),
        "ReferenceSharedEventId": SHOT_EVENT,
        "ReferenceOffsetSeconds": -690,
        "ObservationDurationSeconds": 15,
    })
    changed.append(ead622_exit["ObservationId"])

    ead622_corner = by_id["OBS_EAD622_WIKLUND_SEES_MAN_WATCH_NORTH"]
    ead622_corner.update({
        "TimingMode": "Relative",
        "CanonicalTime": preview_time(-270),
        "ReferenceSharedEventId": SHOT_EVENT,
        "ReferenceOffsetSeconds": -270,
        "ObservationDurationSeconds": 30,
        "Notes": (
            "Wiklund sees the man at the north-west corner at about 23:17. "
            "The 30-second window ends the visualization without claiming that "
            "the real person left or remained until the murder."
        ),
    })
    changed.append(ead622_corner["ObservationId"])
    return changed


def make_groups(groups: list[dict]) -> list[str]:
    target = next(row for row in groups if row.get("GroupId") == "GROUP_EAD9976_05_TWO_WOMEN")
    target["bUseLeaderTimeline"] = False
    target["bCreateAtScenarioStart"] = False
    target["Notes"] = (
        str(target.get("Notes", "")).strip()
        + " v8: runtime grouping disabled; source-timed individual routes prevent "
          "the leader from retaining both women at the murder corner."
    ).strip()
    return [target["GroupId"]]


def validate(people: list[dict], observations: list[dict]) -> dict:
    by_id = {person["EntityId"]: person for person in people}
    obs_by_id = {row["ObservationId"]: row for row in observations}
    errors: list[str] = []

    for entity_id in (
        "OBSERVED_D21659_P15_E11626_MAN_4",
        "UNKNOWN_PERSON_TALKING_TO_OLOF_EBE2354",
    ):
        person = by_id[entity_id]
        if person.get("bSpawnInSimulation") or person.get("Timeline"):
            errors.append(f"{entity_id} still has a physical spawn")

    for entity_id in (
        "VITTNE_EAD9976_01_MASKERAD_KVINNLIG_BESOKARE_1",
        "VITTNE_EAD9976_01_MASKERAD_BESOKARE_2",
        "MASKED_WOMAN_DECORIMA_EAD9976",
        "UNKNOWN_MUSTACHED_MAN_EAD622",
    ):
        person = by_id[entity_id]
        last_offset = max(int(entry.get("EventOffsetSeconds", -999999)) for entry in person["Timeline"])
        if last_offset >= 0 or person["Timeline"][-1].get("Action") != "Despawn":
            errors.append(f"{entity_id} does not end before T0")
        if person.get("bFollowGroupLeaderSchedule"):
            errors.append(f"{entity_id} still follows a runtime group")

    e11626 = obs_by_id["OBS_D21659_P15_E11626_DEKORIMA_OBJECT_MAN"]
    if e11626.get("ObservedEntityId") != "THE_KILLER":
        errors.append("E11626 observation still creates a separate man")
    if int(e11626.get("ReferenceOffsetSeconds", 0)) + int(e11626.get("ObservationDurationSeconds", 0)) > 5:
        errors.append("E11626 observation extends beyond T+5")

    if errors:
        raise SystemExit("; ".join(errors))
    return {
        "changed_person_count": 6,
        "physical_duplicate_count": 0,
        "legacy_persons_present_at_T0": 0,
        "e11626_observation_target": "THE_KILLER",
        "validation_status": "PASS",
    }


def main() -> None:
    if OUT.exists():
        raise SystemExit(f"Output already exists: {OUT}")
    if not V7.exists():
        raise SystemExit(f"Missing v7 base package: {V7}")

    shutil.copytree(V7, OUT)
    people_path = OUT / "DataTables/08_26/DT_TMOP_People.json"
    groups_path = OUT / "DataTables/08_26/DT_TMOP_Groups.json"
    observations_path = OUT / "DataTables/08_26/DT_TMOP_Observations.json"

    people = read_json(people_path)
    groups = read_json(groups_path)
    observations = read_json(OBSERVATIONS_IN)

    changed_people = make_people(people)
    changed_observations = make_observations(observations)
    changed_groups = make_groups(groups)
    validation = validate(people, observations)

    write_json(people_path, people, utf16=True)
    write_json(groups_path, groups, utf16=True)
    write_json(observations_path, observations, utf16=True)
    shutil.copy2(Path(__file__), OUT / "Scripts" / Path(__file__).name)

    report = {
        "package": "TMOP Page31 v8 source-checked legacy-person timeline fixes",
        "engine_version": "0.0.104",
        "base": "TMOP_Page31_Logic_0826_v7",
        "shot_event": SHOT_EVENT,
        "changed_people": changed_people,
        "changed_observations": changed_observations,
        "changed_groups": changed_groups,
        "observation_table_base": "DataTables/08_23/DT_TMOP_Observations.json (unchanged latest table plus v8 corrections)",
        **validation,
    }
    write_json(OUT / "VALIDATION_V8.json", report)

    readme = """# Page31 v8 – källkontrollerade personfixar

Ersätt tabellerna i projektets `DataTables/08_26` med samtliga fyra tabeller i
denna mapp. `DT_TMOP_Observations.json` är nu också med eftersom E11626 och
EAD622 behövde korrigeras tillsammans med personernas timelines.

- La Carterie-kvinnorna styrs individuellt och lämnar mordområdet före T0.
- EAD9976-04-kvinnans proxy visas bara under det cirka tio minuter långa stoppet.
- E11626-mannen spawnas inte dubbelt utan länkas som observation till THE_KILLER.
- EBE2354 är en käll-/hypotespost utan fysisk kanonisk spawn.
- EAD622-proxyn avslutas efter sista belagda observationen före mordet.

Alla kritiska tider är relativa till `Palme_shot_1`. Pluginversionen är fortsatt
0.0.104; inga nya C++-ändringar krävs för v8.
"""
    (OUT / "SOURCE_TIMELINE_FIXES_V8_SV.md").write_text(readme, encoding="utf-8")

    install_path = OUT / "INSTALLERA_OCH_VALIDERA_SV.md"
    install_text = install_path.read_text(encoding="utf-8")
    install_text = install_text.replace(
        "# TMOP Page31 v7 – logik, grupper och fordonsparkering",
        "# TMOP Page31 v8 – logik, grupper, observationer och fordonsparkering",
    ).replace(
        "Det innehåller hela pluginversion `0.0.104`, tre kompletta ersättningstabeller,",
        "Det innehåller hela pluginversion `0.0.104`, fyra kompletta ersättningstabeller,",
    ).replace(
        "   - `DataTables/08_26/DT_TMOP_Groups.json` till `DT_TMOP_Groups`",
        "   - `DataTables/08_26/DT_TMOP_Groups.json` till `DT_TMOP_Groups`\n"
        "   - `DataTables/08_26/DT_TMOP_Observations.json` till\n"
        "     `DT_TMOP_Observations`",
    ).replace(
        "`VALIDATION_V7.json` innehåller de statiska kontroller som redan har passerat.",
        "`VALIDATION_V7.json` innehåller baskontrollerna från föregående paket.\n"
        "`VALIDATION_V8.json` innehåller de nya källkontrollerade person- och\n"
        "observationskontrollerna som redan har passerat.\n\n"
        "Kopiera även `DT_TMOP_Observations.json` från `DataTables/08_26`. Den länkar\n"
        "E11626-iakttagelsen till `THE_KILLER` utan att skapa en extra fysisk person och\n"
        "begränsar EAD622 till det källbelagda observationsfönstret före skottet.",
    )
    install_path.write_text(install_text, encoding="utf-8")


if __name__ == "__main__":
    main()
