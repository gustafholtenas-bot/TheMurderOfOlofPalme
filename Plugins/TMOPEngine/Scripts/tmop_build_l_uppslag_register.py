#!/usr/bin/env python3
"""Build DT_TMOP_Uppslag JSON for the supplied WPU L and LA register.

Existing rows are preserved where possible. References found in exported TMOP
DataTables are marked as retrieved and partially implemented, but never marked
complete automatically.
"""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any


LA_IDS = """
LA6783 LA13963 LA14675 LA14676 LA14677 LA14678 LA14679 LA14680
LA14681 LA14682 LA14683 LA14684 LA14685 LA14686 LA14687 LA14691
LA14692 LA14693 LA14694 LA14713 LA14714 LA14728 LA14752 LA15086
LA19060 LA19359
""".split()

L_IDS = """
L14 L71 L166 L261 L490 L493 L496 L601 L746 L820 L825 L826 L827
L828 L829 L830 L831 L835 L837 L838 L839 L841 L842 L843 L844 L845
L846 L848 L849 L851 L852 L853 L854 L855 L856 L857 L858 L859 L860
L861 L862 L863 L864 L865 L866 L900 L901 L904 L938 L939 L940 L941
L942 L943 L944 L945 L946 L1052 L1061 L1103 L1104 L1106 L1107
L1108 L1109 L1110 L1111 L1113 L1114 L1115 L1116 L1117 L1118
L1119 L1163 L1171 L1172 L1188 L1571 L1572 L1576 L1622 L1627 L1629
L1630 L1632 L1634 L1635 L1636 L1637 L1638 L1639 L1640 L1642 L1643
L1644 L1645 L1647 L1702 L1801 L1802 L1804 L1805 L1819 L1821 L1832
L1833 L1834 L1835 L1836 L1842 L1919 L1920 L1921 L1922 L1972 L1979
L1996 L2034 L2486 L2487 L2488 L2489 L2490 L2491 L2492 L2493 L2614
L2807 L3699 L4331 L4598 L4599 L4600 L4604 L5172 L5182 L5184 L5821
L5951 L6018 L6028 L6048 L6052 L6054 L6056 L6113 L6114 L6115 L6233
L6624 L6674 L6782 L6812 L6814 L6825 L6828 L6830 L6874 L6912 L6961
L7019 L7267 L7278 L7815 L7848 L7849 L7850 L7851 L7852 L7853 L7855
L7857 L7881 L7897 L9259 L9597 L9793 L10015 L10098 L10302 L10793
L11033 L11674 L13965 L14396 L14486 L14505 L14592 L14593 L14594
L14595 L14596 L14597 L14598 L14726 L15456 L15546 L17866 L18210
L18241 L19047 L20620 L20870 L21991 L22411 L22489
""".split()

L261_CHILDREN = """
L261-00 L261-00-A L261-00-B L261-00-C L261-00-D L261-00-E L261-00-F
L261-00-G L261-00-H L261-01 L261-01-A L261-01-B L261-01-C L261-01-D
L261-02 L261-02-A L261-02-B L261-03 L261-03-A L261-03-B
""".split()

L261_NOT_RELEASED = {
    "L261-01-C",
    "L261-02-B",
    "L261-03-B",
}

TITLES = {
    "LA6783": "Ulf Djure – vaktmästare på Grand",
    "L14": "Lars Eric Eriksson – Grandvittne; misstänkt man utanför entrén",
    "L261": "Iakttagelser efter Bröderna Mozart i salong 1, foajén, utanför Grand och vid ABF",
    "L825": "Gun Tredite, Kjell Åke Jansson och Ingvar Selin – Bröderna Mozart",
    "L827": "Katrin Ekström – Olof Palme söder om Grand i dispyt med yngre man",
    "L859": "Hubert Falk – misstänkt man övervakar familjen Palmes ankomst till Grand",
    "L860": "Birgitta och Tomas Wennerling",
    "L865": "Leif och Jeannie Enwall – grå skåpbil på Wallingatan och Sveavägen",
    "L904": "Familjen Schaeffer – Grand och passage av mordplatsen före mordet",
    "L1061": "Max Dager – personer vid busshållplatsen och telefonkiosken utanför Grand",
    "L1114": "Björn och Agneta Rosengren",
    "L1572": "Mikael Åström",
    "L1638": "Två män vid hörnet Tegnérgatan–Sveavägen utanför Grand",
    "L6056": "Odd Greitz – uppgift om Michael Townley på Grand",
    "L10793": "Biografen Grand – fakta och skisser",
    "L15456": "Faktablad om filmerna på Grand",
    "L15546": "PM – förfrågan till personalen på Grand 1994-03-09",
}

SOURCE_KEYS = {
    "SourceReference", "GeneralSourceReference", "SourceId", "Uppslag"
}
TOKEN_RE = re.compile(r"(?<![A-Z0-9])(?:LA|L)\d+(?:-[0-9A-Z]+){0,3}(?![A-Z0-9])")


def read_json(path: Path) -> Any:
    raw = path.read_bytes()
    if raw.startswith((b"\xff\xfe", b"\xfe\xff")):
        return json.loads(raw.decode("utf-16"))
    return json.loads(raw.decode("utf-8-sig"))


def collect_source_text(value: Any, key: str = "") -> list[str]:
    found: list[str] = []
    if isinstance(value, dict):
        for child_key, child_value in value.items():
            if child_key in SOURCE_KEYS and isinstance(child_value, str):
                found.append(child_value)
            found.extend(collect_source_text(child_value, child_key))
    elif isinstance(value, list):
        for child in value:
            found.extend(collect_source_text(child, key))
    return found


def row_identity(row: dict[str, Any], filename: str) -> tuple[str, str] | None:
    candidates = {
        "People": ("EntityId", "PersonEntityIds"),
        "HistoricalVehicles": ("VehicleId", "VehicleEntityIds"),
        "Groups": ("GroupId", "GroupIds"),
        "HistoricalEvents": ("EventId", "SharedEventIds"),
        "Observations": ("ObservationId", "ObservationIds"),
    }
    for marker, (field, link_field) in candidates.items():
        if marker in filename:
            value = row.get(field) or row.get("Name")
            if value:
                return str(value), link_field
    return None


def scan_exports(paths: list[Path]) -> dict[str, dict[str, set[str]]]:
    references: dict[str, dict[str, set[str]]] = {}
    for path in paths:
        try:
            rows = read_json(path)
        except (OSError, UnicodeError, json.JSONDecodeError):
            continue
        if not isinstance(rows, list):
            continue
        for row in rows:
            if not isinstance(row, dict):
                continue
            tokens: set[str] = set()
            for text in collect_source_text(row):
                tokens.update(TOKEN_RE.findall(text.upper()))
            identity = row_identity(row, path.name)
            for token in tokens:
                bucket = references.setdefault(token, {})
                if identity:
                    linked_id, link_field = identity
                    bucket.setdefault(link_field, set()).add(linked_id)
    return references


def empty_row(uppslag_id: str) -> dict[str, Any]:
    series = "LA" if uppslag_id.startswith("LA") else "L"
    return {
        "Name": uppslag_id,
        "UppslagId": uppslag_id,
        "Title": TITLES.get(uppslag_id, ""),
        "SeriesId": series,
        "ParentUppslagId": "",
        "Availability": "Unknown",
        "bRetrieved": False,
        "bAddedToProject": False,
        "bPartiallyAdded": False,
        "bNeedsReview": False,
        "bRelevantToGame": True,
        "SourceUrl": f"https://wpu.nu/wiki/Uppslag:{uppslag_id}",
        "DocumentDate": "",
        "PersonEntityIds": [],
        "VehicleEntityIds": [],
        "GroupIds": [],
        "SharedEventIds": [],
        "ObservationIds": [],
        "AnchorIds": [],
        "ImplementedSummary": "",
        "RemainingWork": "",
        "Notes": "",
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--existing", type=Path)
    parser.add_argument("--scan", type=Path, action="append", default=[])
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    existing_rows: dict[str, dict[str, Any]] = {}
    if args.existing and args.existing.exists():
        for row in read_json(args.existing):
            existing_rows[str(row.get("Name") or row.get("UppslagId"))] = row

    export_files: list[Path] = []
    for root in args.scan:
        if root.is_dir():
            export_files.extend(root.glob("*.json"))
        elif root.is_file():
            export_files.append(root)
    references = scan_exports(export_files)

    rows: list[dict[str, Any]] = []
    for uppslag_id in LA_IDS + L_IDS + L261_CHILDREN:
        row = empty_row(uppslag_id)
        row.update(existing_rows.get(uppslag_id, {}))
        row["SeriesId"] = "LA" if uppslag_id.startswith("LA") else "L"
        row["SourceUrl"] = row.get("SourceUrl") or (
            f"https://wpu.nu/wiki/Uppslag:{uppslag_id}"
        )
        links = references.get(uppslag_id, {})
        if links:
            row["bRetrieved"] = True
            if not row.get("bAddedToProject"):
                row["bPartiallyAdded"] = True
            for field, values in links.items():
                row[field] = sorted(set(row.get(field, [])) | values)
        if uppslag_id in L261_CHILDREN:
            row["ParentUppslagId"] = "L261"
            row["Availability"] = (
                "NotReleased"
                if uppslag_id in L261_NOT_RELEASED
                else "Available"
            )
        rows.append(row)

    # Preserve detailed child references already tracked separately.
    listed = {row["Name"] for row in rows}
    for name, row in existing_rows.items():
        if name not in listed:
            if not row.get("SeriesId"):
                match = re.match(r"[A-Z]+", name.upper())
                row["SeriesId"] = match.group(0) if match else ""
            row.setdefault("ParentUppslagId", "")
            row.setdefault("Availability", "Unknown")
            rows.append(row)

    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(rows, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(f"Wrote {len(rows)} rows to {args.output}")


if __name__ == "__main__":
    main()
