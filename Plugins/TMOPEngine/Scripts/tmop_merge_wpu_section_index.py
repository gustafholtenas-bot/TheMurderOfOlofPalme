#!/usr/bin/env python3
"""Add WPU letter-section metadata rows without changing uppslag statistics."""

from __future__ import annotations

import argparse
import json
import re
from pathlib import Path
from typing import Any


def read_json(path: Path) -> list[dict[str, Any]]:
    raw = path.read_bytes()
    encoding = "utf-16" if raw.startswith((b"\xff\xfe", b"\xfe\xff")) else "utf-8-sig"
    return json.loads(raw.decode(encoding))


def natural_key(value: str) -> list[object]:
    return [int(part) if part.isdigit() else part for part in re.split(r"(\d+)", value)]


def section_row(section: dict[str, str]) -> dict[str, Any]:
    section_id = section["SectionId"]
    description = section.get("Description", "")
    return {
        "Name": f"SECTION_{section_id}",
        "SourceCategory": "PoliceUppslag",
        "UppslagId": f"SECTION_{section_id}",
        "Title": description,
        "SeriesId": section_id,
        "bIsSectionDefinition": True,
        "SectionDescription": description,
        "ParentUppslagId": "",
        "Availability": "Unknown",
        "bRetrieved": False,
        "bAddedToProject": False,
        "bPartiallyAdded": False,
        "bNeedsReview": False,
        "bRelevantToGame": False,
        "SourceUrl": section.get("SourceUrl", ""),
        "DocumentDate": "",
        "PersonEntityIds": [],
        "VehicleEntityIds": [],
        "GroupIds": [],
        "SharedEventIds": [],
        "ObservationIds": [],
        "AnchorIds": [],
        "ImplementedSummary": "",
        "RemainingWork": "",
        "Notes": "WPU avsnittsmetadata; räknas inte som ett uppslag."
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--section-index", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    rows = read_json(args.input)
    sections = read_json(args.section_index)
    rows = [row for row in rows if not row.get("bIsSectionDefinition")]
    for row in rows:
        row.setdefault("SourceCategory", "PoliceUppslag")
    rows.extend(section_row(section) for section in sections)

    rows.sort(key=lambda row: (
        natural_key(str(row.get("SeriesId", ""))),
        0 if row.get("bIsSectionDefinition") else 1,
        natural_key(str(row.get("UppslagId", row.get("Name", ""))))
    ))
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(
        json.dumps(rows, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-16"
    )
    print(f"Wrote {len(rows)} rows, including {len(sections)} WPU section definitions")


if __name__ == "__main__":
    main()
