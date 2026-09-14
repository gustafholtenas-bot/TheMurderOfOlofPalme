"""Activate Eva Friden Kvist and her unnamed 1986 boyfriend in 09_12.

The route is based on Eva's first-person message supplied by the user. Exact
seconds and project-anchor mapping are reconstructions. The script is
idempotent and preserves the UTF-16 JSON inside DT_TMOP_People.zip.
"""

from __future__ import annotations

import copy

from tmop_add_system_test_scenario_09_12 import (
    find_row,
    load_people,
    replace_row,
    reset_person_entry,
    save_people,
)


EVA_ID = "EVA_FRIDEN_KVIST_SOCIAL_A"
BOYFRIEND_ID = "EVA_FRIDEN_KVIST_BOYFRIEND_1986"
GROUP_ID = "GROUP_EVA_FRIDEN_KVIST_PAIR"

# The west pedestrian boundary currently has an Exit-prefixed project name,
# although Eva and her boyfriend use it in the opposite direction (into map).
START_ANCHOR = "ExitTunnelgatanW_Sidewalk1"
METRO_ENTRANCE = "MetroHotorget1_entrance"
METRO_INSIDE = "MetroHotorget1_inside"

SOURCE = (
    "Direktmeddelande från Eva Fridén Kvist till Gustaf Holtenäs "
    "2026-08-31; skärmdump tillhandahållen 2026-09-13"
)
ROUTE_NOTE = (
    "Eva uppger i förstahand att hon och sin dåvarande pojkvän lämnade "
    "Bistro Bohème på Drottninggatan strax före 23.00, kom från "
    "Tunnelgatans håll, sneddade över Sveavägen och tog den närmaste öppna "
    "Hötorget-uppgången vid dåvarande Dekorima. De åkte via T-Centralen "
    "vidare mot Ropsten. Start 23.00, exakta sekunder, gånghastighet och "
    "ankarmappning är användarstyrd rekonstruktion."
)


def text(namespace_key: str, value: str) -> str:
    return (
        'NSLOCTEXT("DT_TMOP_People '
        '[5FF9126A2E58348990CC01ABAA09CC1A]", '
        f'"{namespace_key}", "{value}")'
    )


def route_entry(template: dict, person_id: str, suffix: str, action: str,
                minute: int, second: int, anchor: str, lateral_cm: int,
                *, arrival: bool = False, inside: bool = False) -> dict:
    entry = reset_person_entry(
        template, f"{person_id}_{suffix}", action, minute, second
    )
    entry.update({
        "LocationType": "Anchor",
        "TargetAnchorId": anchor,
        "AnchorOffsetCm": {"X": 0, "Y": lateral_cm, "Z": 0},
        "bTimeIsArrival": arrival,
        "TravelSpeedOverrideCmPerSecond": 135 if action == "MoveToAnchor" else 0,
        "ActivityState": "Walking" if action == "MoveToAnchor" else "Standing",
        "bTeleportDuringCatchUp": action == "InitialPlacement",
        "Confidence": "Reconstructed",
        "SourceReference": SOURCE,
        "Notes": ROUTE_NOTE + (" Personen går ned i tunnelbanan." if inside else ""),
    })
    return entry


def build_timeline(template: dict, person_id: str, lateral_cm: int) -> list[dict]:
    timeline = [
        route_entry(template, person_id, "START_TUNNELGATAN_W",
                    "InitialPlacement", 0, 0, START_ANCHOR, lateral_cm),
        route_entry(template, person_id, "CROSS_TO_HOTORGET_ENTRANCE",
                    "MoveToAnchor", 1, 35, METRO_ENTRANCE, lateral_cm,
                    arrival=True),
        route_entry(template, person_id, "DESCEND_HOTORGET_METRO",
                    "MoveToAnchor", 1, 50, METRO_INSIDE, lateral_cm,
                    arrival=True, inside=True),
        route_entry(template, person_id, "LEAVE_STREET_SIMULATION",
                    "Despawn", 2, 0, METRO_INSIDE, lateral_cm, inside=True),
    ]
    return timeline


def common_person_updates(row: dict, person_id: str, lateral_cm: int) -> None:
    template = copy.deepcopy(find_row(load_people(), "AGNETA_ROSENGREN")["Timeline"][0])
    row.update({
        "Name": person_id,
        "EntityId": person_id,
        "bSpawnInSimulation": True,
        "bPrioritizeTimeline": True,
        "SocialGroupId": GROUP_ID,
        "GroupLeaderEntityId": EVA_ID,
        "GroupFormation": "SideBySide",
        "GroupFormationSpacingCm": 110,
        "bFollowGroupLeaderSchedule": False,
        "AgentTimelineSummary": (
            "23.00 in från Tunnelgatans västra gånggräns; sneddar över "
            "Sveavägen; går ned i Hötorgets tunnelbana vid Dekorima cirka 23.02."
        ),
        "ObservationSummary": (
            "Passerar platsen tillsammans med partner cirka tjugo minuter "
            "före mordet; ingen mordobservation."
        ),
        "GeneralSourceReference": SOURCE,
        "AgentInfoSourceReference": SOURCE,
        "Timeline": build_timeline(template, person_id, lateral_cm),
        "Notes": ROUTE_NOTE,
    })


def install() -> None:
    rows = load_people()
    eva = copy.deepcopy(find_row(rows, EVA_ID))
    common_person_updates(eva, EVA_ID, -55)
    eva.update({
        "CategoryId": "WITNESS",
        "FullName": text(EVA_ID + "_FullName", "Eva Fridén Kvist"),
        "FirstName": "Eva",
        "LastName": "Fridén Kvist",
        "PostMurderEventsSummary": (
            "På tunnelbanan såg Eva människor gråta utan att först förstå "
            "varför. Hon uppger att hon fick veta vad som hade hänt genom "
            "löpsedlarna senare på morgonen."
        ),
    })

    boyfriend = copy.deepcopy(find_row(rows, "KIM_PERSSON_FATHER_MURDER_SCENE"))
    common_person_updates(boyfriend, BOYFRIEND_ID, 55)
    boyfriend.update({
        "CategoryId": "WITNESS_COMPANION",
        "FullName": text(
            BOYFRIEND_ID + "_FullName",
            "Eva Fridén Kvists dåvarande pojkvän (namn ej angivet)",
        ),
        "FirstName": "",
        "LastName": "",
        "Gender": "Male",
        "Nationality": "Unknown",
        "Occupation": "",
        "HistoricalAddress": "",
        "Uppslag": "",
        "bPoliceInterviewed": False,
        "ReferenceImage": "None",
        "EvidenceIcon": "None",
        "PostMurderEventsSummary": "",
    })

    replace_row(rows, eva)
    replace_row(rows, boyfriend)
    save_people(rows)
    print(f"Installerade {EVA_ID} och {BOYFRIEND_ID}; totalt {len(rows)} personer.")


if __name__ == "__main__":
    install()
