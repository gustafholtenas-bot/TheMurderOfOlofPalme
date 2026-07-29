#!/usr/bin/env python3
"""Mark the first observation-test agents without changing any other rows."""

from __future__ import annotations

import argparse
import json
from pathlib import Path


OBSERVED_UNKNOWN_IDS = {
    "SUSPECT_PASSAT_DRIVER_AFK_LUNTMAKAR",
    "SUSPECT_PASSAT_PASSENGER_AFK_LUNTMAKAR",
    "SUSPECT_PASSAT_THIRD_MAN_AFK_LUNTMAKAR",
    "SUSPECT_RED_JACKET_KARELIA",
    "SUSPECT_DAVID_BAGARES_MAN_CONTROLLED",
    "THOMAS_PILTZ_MED_WALKIE_TALKIE",
}

OBSERVED_UNKNOWN_VEHICLE_IDS = {
    "VEHICLE_SUSPECT_PASSAT_AFK_LUNTMAKAR",
}

THOMAS_PILTZ_OBSERVATION_NOTES = (
    "Anonym observationsfigur. Mauno Luukas, även kallad \"vittnet Jerker\", "
    "uppgav senare att han identifierade mannen med walkie-talkie som "
    "\"polisman D\", vilket i Granskningskommissionens redovisning anges som "
    "Thomas Piltz. Identifikationen är ett senare utpekande och ska inte "
    "behandlas som fastställd identitet i den orörda observationen."
)

THOMAS_PILTZ_SOURCE = (
    "https://wpu.nu/wiki/Mauno_Luukas; "
    "SOU 1999:88, Granskningskommissionen, s. 338"
)


def read_rows(path: Path) -> tuple[list[dict], str]:
    raw = path.read_bytes()
    encoding = "utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding)), encoding


def update_rows(
    rows: list[dict], expected_ids: set[str]
) -> list[str]:
    changed: list[str] = []
    for row in rows:
        entity_id = row.get(
            "EntityId", row.get("VehicleId", row.get("Name", "")))
        if entity_id in expected_ids:
            row["CategoryId"] = "OBSERVED_UNKNOWN"
            if entity_id == "THOMAS_PILTZ_MED_WALKIE_TALKIE":
                row["GeneralSourceReference"] = THOMAS_PILTZ_SOURCE
                row["Uppslag"] = "EBD9631"
                row["Notes"] = THOMAS_PILTZ_OBSERVATION_NOTES
            changed.append(entity_id)
    missing = sorted(expected_ids.difference(changed))
    if missing:
        raise SystemExit(f"Missing expected rows: {', '.join(missing)}")
    return changed


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("input", type=Path)
    parser.add_argument("output", type=Path)
    parser.add_argument(
        "--vehicles",
        action="store_true",
        help="Update the observed test vehicle instead of person rows.")
    args = parser.parse_args()

    rows, encoding = read_rows(args.input)
    expected_ids = (
        OBSERVED_UNKNOWN_VEHICLE_IDS if args.vehicles
        else OBSERVED_UNKNOWN_IDS)
    changed = update_rows(rows, expected_ids)

    text = json.dumps(rows, ensure_ascii=False, indent=2)
    if encoding == "utf-16":
        args.output.write_bytes(text.encode("utf-16"))
    else:
        args.output.write_text(text + "\n", encoding="utf-8")

    print(f"Updated {len(changed)} rows: {args.output}")


if __name__ == "__main__":
    main()
