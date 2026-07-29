#!/usr/bin/env python3
"""Merge listed WPU sections and all their document subcases into the register."""

from __future__ import annotations

import argparse
import json
import re
import time
from pathlib import Path
from typing import Any, Iterable
from urllib.parse import urlencode
from urllib.error import HTTPError, URLError
from urllib.request import Request, urlopen


API_URL = "https://wpu.nu/api.php"
USER_AGENT = "TMOP-Uppslag-Register/0.0.99"
MAIN_ID_RE = re.compile(
    r"^([A-ZÅÄÖ]{1,3}\d+)\b(?:\s*-\s*(.*))?$"
)
CASE_ID_RE = re.compile(r"^([A-ZÅÄÖ]{1,3}\d+)-.+$")
SERIES_RE = re.compile(r"^[A-Z]+")


def api(params: dict[str, str]) -> dict[str, Any]:
    query = {"format": "json", "formatversion": "2", **params}
    request = Request(
        API_URL + "?" + urlencode(query),
        headers={"User-Agent": USER_AGENT},
    )
    for attempt in range(5):
        try:
            with urlopen(request, timeout=60) as response:
                return json.load(response)
        except (HTTPError, URLError, TimeoutError):
            if attempt == 4:
                raise
            time.sleep(1.0 * (attempt + 1))
    raise RuntimeError("Unreachable WPU retry state")


def chunks(values: list[str], size: int) -> Iterable[list[str]]:
    for index in range(0, len(values), size):
        yield values[index:index + size]


def parse_main_cases(path: Path) -> dict[str, str]:
    cases: dict[str, str] = {}
    for original_line in path.read_text(encoding="utf-8-sig").splitlines():
        line = original_line.replace("Curved-arrow-centered.png", "").strip()
        match = MAIN_ID_RE.match(line)
        if match:
            cases[match.group(1)] = (match.group(2) or "").strip()
    return cases


def fetch_uppslag_titles(prefixes: set[str]) -> list[str]:
    titles: list[str] = []
    for prefix in sorted(prefixes):
        continuation: str | None = None
        while True:
            params = {
                "action": "query",
                "list": "allpages",
                "apnamespace": "3100",
                "apprefix": prefix,
                "aplimit": "max",
            }
            if continuation:
                params["apcontinue"] = continuation
            result = api(params)
            titles.extend(
                page["title"] for page in result["query"]["allpages"]
            )
            continuation = result.get("continue", {}).get("apcontinue")
            if not continuation:
                break
    return titles


def fetch_availability(titles: list[str]) -> dict[str, str]:
    availability: dict[str, str] = {}
    for batch in chunks(titles, 40):
        result = api({
            "action": "query",
            "prop": "categories|revisions",
            "cllimit": "max",
            "redirects": "1",
            "rvprop": "content",
            "rvslots": "main",
            "titles": "|".join(batch),
        })
        target_statuses: dict[str, str] = {}
        for page in result["query"]["pages"]:
            case_id = page["title"].removeprefix("Uppslag:")
            categories = {
                category["title"]
                for category in page.get("categories", [])
            }
            if "Kategori:Uppslag med dokument" in categories:
                target_statuses[case_id] = "Available"
            elif (
                "Kategori:Uppslag utan dokument" in categories
                or "Kategori:Uppslag med felaktig kod" in categories
                or "Kategori:Uppslag med felaktigt namn" in categories
            ):
                target_statuses[case_id] = "NotReleased"
            else:
                revisions = page.get("revisions", [])
                content = ""
                if revisions:
                    content = (
                        revisions[0]
                        .get("slots", {})
                        .get("main", {})
                        .get("content", "")
                    )
                target_statuses[case_id] = (
                    "Available" if content.strip() else "Unknown"
                )
        availability.update(target_statuses)
        for redirect in result["query"].get("redirects", []):
            source_id = redirect["from"].removeprefix("Uppslag:")
            target_id = redirect["to"].removeprefix("Uppslag:")
            availability[source_id] = target_statuses.get(
                target_id, "Unknown"
            )
        time.sleep(0.05)
    return availability


def series_for(case_id: str) -> str:
    match = SERIES_RE.match(case_id)
    return match.group(0) if match else ""


def base_row(
    case_id: str,
    title: str,
    parent_id: str = "",
    availability: str = "Unknown",
) -> dict[str, Any]:
    return {
        "Name": case_id,
        "UppslagId": case_id,
        "Title": title,
        "SeriesId": series_for(case_id),
        "ParentUppslagId": parent_id,
        "Availability": availability,
        "bRetrieved": False,
        "bAddedToProject": False,
        "bPartiallyAdded": False,
        "bNeedsReview": False,
        "bRelevantToGame": True,
        "SourceUrl": (
            f"https://wpu.nu/wiki/Uppslag:{case_id}"
            if parent_id
            else f"https://wpu.nu/wiki/Avsnitt:{case_id}"
        ),
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


def natural_key(value: str) -> list[object]:
    return [
        int(part) if part.isdigit() else part
        for part in re.split(r"(\d+)", value)
    ]


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--section-list", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", type=Path, required=True)
    args = parser.parse_args()

    rows = json.loads(args.input.read_text(encoding="utf-8-sig"))
    by_name = {
        str(row.get("Name") or row.get("UppslagId")): row
        for row in rows
    }
    main_cases = parse_main_cases(args.section_list)

    for case_id, title in main_cases.items():
        if case_id not in by_name:
            by_name[case_id] = base_row(case_id, title)
        else:
            row = by_name[case_id]
            row["SeriesId"] = series_for(case_id)
            if title and not row.get("Title"):
                row["Title"] = title

    child_to_parent: dict[str, str] = {}
    search_prefixes = {case_id[0] for case_id in main_cases}
    for full_title in fetch_uppslag_titles(search_prefixes):
        case_id = full_title.removeprefix("Uppslag:")
        match = CASE_ID_RE.fullmatch(case_id)
        if match and match.group(1) in main_cases:
            child_to_parent[case_id] = match.group(1)

    statuses = fetch_availability(
        [f"Uppslag:{case_id}" for case_id in child_to_parent]
    )
    for case_id, parent_id in child_to_parent.items():
        status = statuses.get(case_id, "Unknown")
        if case_id not in by_name:
            by_name[case_id] = base_row(
                case_id, "", parent_id, status
            )
        else:
            row = by_name[case_id]
            row["SeriesId"] = series_for(parent_id)
            row["ParentUppslagId"] = parent_id
            row["Availability"] = status
            row["SourceUrl"] = (
                row.get("SourceUrl")
                or f"https://wpu.nu/wiki/Uppslag:{case_id}"
            )

    def row_sort_key(row: dict[str, Any]) -> tuple[list[object], int, list[object]]:
        name = str(row.get("Name", ""))
        parent = str(row.get("ParentUppslagId") or name)
        return (
            natural_key(series_for(parent)),
            natural_key(parent),
            0 if not row.get("ParentUppslagId") else 1,
            natural_key(name),
        )

    output_rows = sorted(by_name.values(), key=row_sort_key)
    args.output.write_text(
        json.dumps(output_rows, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    report_parents: list[dict[str, Any]] = []
    for parent_id in sorted(main_cases, key=natural_key):
        children = [
            row for row in output_rows
            if row.get("ParentUppslagId") == parent_id
        ]
        available = [
            row["Name"] for row in children
            if row.get("Availability") == "Available"
        ]
        missing = [
            row["Name"] for row in children
            if row.get("Availability") == "NotReleased"
        ]
        unknown = [
            row["Name"] for row in children
            if row.get("Availability") == "Unknown"
        ]
        report_parents.append({
            "ParentUppslagId": parent_id,
            "SeriesId": series_for(parent_id),
            "Title": main_cases[parent_id],
            "SubcaseCount": len(children),
            "AvailableCount": len(available),
            "NotReleasedCount": len(missing),
            "UnknownCount": len(unknown),
            "AvailableIds": available,
            "NotReleasedIds": missing,
            "UnknownIds": unknown,
        })

    all_children = [
        child
        for parent in report_parents
        for child in (
            parent["AvailableIds"]
            + parent["NotReleasedIds"]
            + parent["UnknownIds"]
        )
    ]
    report = {
        "Sources": [
            f"https://wpu.nu/wiki/Avsnitt:{series}"
            for series in sorted({
                series_for(case_id) for case_id in main_cases
            })
        ],
        "MainCaseCount": len(main_cases),
        "SubcaseCount": len(all_children),
        "AvailableCount": sum(
            parent["AvailableCount"] for parent in report_parents
        ),
        "NotReleasedCount": sum(
            parent["NotReleasedCount"] for parent in report_parents
        ),
        "UnknownCount": sum(
            parent["UnknownCount"] for parent in report_parents
        ),
        "Parents": report_parents,
    }
    args.report.write_text(
        json.dumps(report, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )
    print(f"Main cases: {report['MainCaseCount']}")
    print(f"Subcases: {report['SubcaseCount']}")
    print(f"Available on WPU: {report['AvailableCount']}")
    print(f"Not released/missing: {report['NotReleasedCount']}")
    print(f"Unknown: {report['UnknownCount']}")


if __name__ == "__main__":
    main()
