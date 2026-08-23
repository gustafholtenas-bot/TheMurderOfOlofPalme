#!/usr/bin/env python3
"""Apply the 2026-08-23 early-validation data fixes to full TMOP tables."""

from __future__ import annotations

import argparse
import codecs
import copy
import json
from pathlib import Path


def load_utf16(path: Path):
    with path.open("r", encoding="utf-16") as handle:
        return json.load(handle)


def save_utf16(path: Path, value) -> None:
    text = json.dumps(value, ensure_ascii=False, indent="\t") + "\n"
    path.write_bytes(codecs.BOM_UTF16_LE + text.replace("\n", "\r\n").encode("utf-16-le"))


def row_by_id(rows, key: str, value: str):
    matches = [row for row in rows if row.get(key) == value]
    if len(matches) != 1:
        raise RuntimeError(f"Expected exactly one {key}={value}, found {len(matches)}")
    return matches[0]


def append_note(existing: str, addition: str) -> str:
    return existing.strip() if addition in existing else (existing + " " + addition).strip()


def make_entry(template, entry_id: str, action: str, time, anchor: str, notes: str):
    entry = copy.deepcopy(template)
    entry.update({
        "EntryId": entry_id,
        "Action": action,
        "Time": {"Hour": time[0], "Minute": time[1], "Second": time[2]},
        "TimingMode": "Absolute",
        "SharedEventId": "None",
        "EventOffsetSeconds": 0,
        "bTimeIsArrival": False,
        "TravelSpeedOverrideCmPerSecond": 0,
        "LocationType": "Anchor",
        "TargetAnchorId": anchor,
        "PassAnchorIds": [],
        "TargetEntityId": "None",
        "TargetSeatId": "None",
        "ActivityState": "Standing" if action == "Spawn" else "Idle",
        "bTeleportDuringCatchUp": True,
        "Confidence": "Reconstructed",
        "SourceReference": "Legacy Blender observation; observer identity unresolved",
        "Notes": notes,
    })
    return entry


def add_observation(rows, observation_id: str, entity_id: str, anchor: str, time, description: str):
    rows[:] = [row for row in rows if row.get("ObservationId") != observation_id]
    rows.append({
        "Name": observation_id,
        "ObservationId": observation_id,
        "DisplayName": description,
        "bEnabled": True,
        "ObserverEntityIds": [],
        "ObservedEntityId": entity_id,
        "ObservedEntityType": "Person",
        "ObservationAnchorId": anchor,
        "TimingMode": "Absolute",
        "CanonicalTime": {"Hour": time[0], "Minute": time[1], "Second": time[2]},
        "ReferenceSharedEventId": "None",
        "ReferenceOffsetSeconds": 0,
        "ObservationDurationSeconds": 30,
        "ObservationRadiusCm": 350.0,
        "bRequireObserverNearAnchor": False,
        "bRequireObservedEntityNearAnchor": True,
        "bRequiresLineOfSight": False,
        "bAllowUnattributedObservation": True,
        "Confidence": "Reconstructed",
        "SourceReference": "Legacy Blender observation; observer identity unresolved",
        "ObservedDescription": description,
        "Notes": "Converted from a continuously simulated draft person into a collision-free 30-second observation window.",
    })


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("database_dir", type=Path)
    args = parser.parse_args()
    people_path = args.database_dir / "DT_TMOP_People.json"
    observations_path = args.database_dir / "DT_TMOP_Observations.json"
    people = load_utf16(people_path)
    observations = load_utf16(observations_path)

    kerstin = row_by_id(people, "EntityId", "LIKNAR_KERTSIN_TEGIN_GADDY")
    template = kerstin["Timeline"][0]
    kerstin["CategoryId"] = "OBSERVED_UNKNOWN"
    kerstin["bSpawnInSimulation"] = True
    kerstin["Timeline"] = [
        make_entry(template, "OBS_KERSTIN_LIKE_SPAWN", "Spawn", (23, 0, 52),
                   "GrandFoaje",
                   "Spawn 15 seconds before the short observation in Grands foajé."),
        make_entry(template, "OBS_KERSTIN_LIKE_DESPAWN", "Despawn", (23, 1, 22),
                   "GrandFoaje",
                   "End of the 30-second observation window."),
    ]
    kerstin["Notes"] = append_note(kerstin.get("Notes", ""),
        "Converted to an OBSERVED_UNKNOWN 30-second window; no continuous Grand timeline.")
    add_observation(observations, "OBS_LEGACY_KERSTIN_LIKE_230107",
                    kerstin["EntityId"],
                    "GrandFoaje",
                    (23, 1, 7), "Kerstin-liknande kvinna observeras")

    man = row_by_id(people, "EntityId", "MAN_30_AR_ARABISKT_UTSEENDE_PRYDLIGT")
    template = man["Timeline"][0]
    man["CategoryId"] = "OBSERVED_UNKNOWN"
    man["bSpawnInSimulation"] = True
    windows = [
        ("TEGNER_SE", (23, 1, 43), "TegnerXSaltmastar_SE"),
        ("TEGNER_NE", (23, 2, 34), "TegnerXSaltmastar_NE"),
        ("TELEFON", (23, 17, 49), "Telefonkiosk1"),
        ("ABF", (23, 22, 7), "ABFhuset"),
        ("KAMMAKAR", (23, 23, 17), "KammakarXSvea_SW"),
    ]
    timeline = []
    for suffix, centre, anchor in windows:
        total = centre[0] * 3600 + centre[1] * 60 + centre[2]
        start = total - 15
        end = total + 15
        start_time = (start // 3600, start % 3600 // 60, start % 60)
        end_time = (end // 3600, end % 3600 // 60, end % 60)
        timeline.append(make_entry(template, f"OBS_ARABIC_MAN_{suffix}_SPAWN", "Spawn",
                                   start_time, anchor, "Start of a 30-second observation window."))
        timeline.append(make_entry(template, f"OBS_ARABIC_MAN_{suffix}_DESPAWN", "Despawn",
                                   end_time, anchor, "End of a 30-second observation window."))
        add_observation(observations, f"OBS_LEGACY_ARABIC_MAN_{suffix}", man["EntityId"],
                        anchor, centre, "Prydlig cirka 30-årig man med arabiskt utseende observeras")
    man["Timeline"] = timeline
    man["Notes"] = append_note(man.get("Notes", ""),
        "Converted to OBSERVED_UNKNOWN 30-second windows at the five legacy observation points; no continuous route.")

    # The 150 m Monte Carlo -> PUB leg needs about 107 seconds at 140 cm/s.
    # 23:05:45 gives 116 seconds and ~129 cm/s while preserving 23:13:42 for
    # the following historical point by shortening its relative offset.
    for entity_id in ("KICKI_J", "MAUNO_LUUKAS_JERKER"):
        row = row_by_id(people, "EntityId", entity_id)
        walk = next(e for e in row["Timeline"] if e["EntryId"].endswith("WALKS_FROM_MONTE_CARLO_TO_PUB"))
        walk["Time"] = {"Hour": 23, "Minute": 5, "Second": 45}
        walk["bTimeIsArrival"] = True
        walk["Notes"] = append_note(walk.get("Notes", ""),
            "Arrival corrected to 23:05:45: approximately 150 m in 116 seconds (~129 cm/s).")
        following = row["Timeline"][row["Timeline"].index(walk) + 1]
        following["EventOffsetSeconds"] = 477
        following["Notes"] = append_note(following.get("Notes", ""),
            "Relative offset adjusted so this point remains 23:13:42 after the PUB timing correction.")

    save_utf16(people_path, people)
    save_utf16(observations_path, observations)
    print("Updated", people_path)
    print("Updated", observations_path)


if __name__ == "__main__":
    main()
