#!/usr/bin/env python3
"""Fetch every WPU subcase beneath the registered L and LA main cases.

Availability is based on WPU's semantic categories:
  Kategori:Uppslag med dokument  -> Available
  Kategori:Uppslag utan dokument -> NotReleased

This does not change bRetrieved; that flag means that our project has actually
obtained/reviewed the document.
"""

from __future__ import annotations

import argparse
import json
import re
import time
from pathlib import Path
from typing import Any, Iterable
from urllib.parse import urlencode
from urllib.request import Request, urlopen


API_URL = "https://wpu.nu/api.php"
USER_AGENT = "TMOP-Uppslag-Register/0.0.99"
ID_RE = re.compile(r"^(LA?\d+)(?:-.+)?$")


def api(params: dict[str, str]) -> dict[str, Any]:
    query = {"format": "json", "formatversion": "2", **params}
    request = Request(
        API_URL + "?" + urlencode(query),
        headers={"User-Agent": USER_AGENT},
    )
    with urlopen(request, timeout=60) as response:
        return json.load(response)


def chunks(values: list[str], size: int) -> Iterable[list[str]]:
    for index in range(0, len(values), size):
        yield values[index:index + size]


def fetch_l_uppslag_titles() -> list[str]:
    titles: list[str] = []
    continuation: str | None = None
    while True:
        params = {
            "action": "query",
            "list": "allpages",
            "apnamespace": "3100",
            "apprefix": "L",
            "aplimit": "max",
        }
        if continuation:
            params["apcontinue"] = continuation
        result = api(params)
        titles.extend(page["title"] for page in result["query"]["allpages"])
        continuation = result.get("continue", {}).get("apcontinue")
        if not continuation:
            return titles


def fetch_availability(titles: list[str]) -> dict[str, str]:
    availability: dict[str, str] = {}
    for batch in chunks(titles, 40):
        result = api({
            "action": "query",
            "prop": "categories",
            "cllimit": "max",
            "titles": "|".join(batch),
        })
        for page in result["query"]["pages"]:
            title = page["title"].removeprefix("Uppslag:")
            categories = {
                category["title"]
                for category in page.get("categories", [])
            }
            if "Kategori:Uppslag med dokument" in categories:
                availability[title] = "Available"
            elif "Kategori:Uppslag utan dokument" in categories:
                availability[title] = "NotReleased"
            else:
                availability[title] = "Unknown"
        time.sleep(0.05)
    return availability


def natural_key(value: str) -> list[object]:
    return [
        int(part) if part.isdigit() else part
        for part in re.split(r"(\d+)", value)
    ]


def new_row(case_id: str, parent_id: str, status: str) -> dict[str, Any]:
    series = "LA" if case_id.startswith("LA") else "L"
    return {
        "Name": case_id,
        "UppslagId": case_id,
        "Title": "",
        "SeriesId": series,
        "ParentUppslagId": parent_id,
        "Availability": status,
        "bRetrieved": False,
        "bAddedToProject": False,
        "bPartiallyAdded": False,
        "bNeedsReview": False,
        "bRelevantToGame": True,
        "SourceUrl": f"https://wpu.nu/wiki/Uppslag:{case_id}",
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
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--report", type=Path)
    args = parser.parse_args()

    rows = json.loads(args.input.read_text(encoding="utf-8-sig"))
    by_name = {
        str(row.get("Name") or row.get("UppslagId")): row
        for row in rows
    }
    parent_ids = {
        name
        for name, row in by_name.items()
        if row.get("SeriesId") in {"L", "LA"}
        and not row.get("ParentUppslagId")
        and ID_RE.fullmatch(name)
    }

    all_titles = fetch_l_uppslag_titles()
    child_to_parent: dict[str, str] = {}
    for full_title in all_titles:
        case_id = full_title.removeprefix("Uppslag:")
        match = ID_RE.fullmatch(case_id)
        if not match or "-" not in case_id:
            continue
        parent_id = match.group(1)
        if parent_id in parent_ids:
            child_to_parent[case_id] = parent_id

    statuses = fetch_availability(
        [f"Uppslag:{case_id}" for case_id in child_to_parent]
    )
    for case_id, parent_id in child_to_parent.items():
        status = statuses.get(case_id, "Unknown")
        if case_id in by_name:
            row = by_name[case_id]
            row["ParentUppslagId"] = parent_id
            row["SeriesId"] = "LA" if case_id.startswith("LA") else "L"
            row["Availability"] = status
            row["SourceUrl"] = (
                row.get("SourceUrl")
                or f"https://wpu.nu/wiki/Uppslag:{case_id}"
            )
        else:
            by_name[case_id] = new_row(case_id, parent_id, status)

    def row_sort_key(row: dict[str, Any]) -> tuple[list[object], int, list[object]]:
        name = str(row.get("Name", ""))
        parent = str(row.get("ParentUppslagId") or name)
        return (
            natural_key(parent),
            0 if not row.get("ParentUppslagId") else 1,
            natural_key(name),
        )

    output_rows = sorted(by_name.values(), key=row_sort_key)
    args.output.write_text(
        json.dumps(output_rows, ensure_ascii=False, indent=2) + "\n",
        encoding="utf-8",
    )

    children = [
        row for row in output_rows
        if row.get("ParentUppslagId")
        and row.get("SeriesId") in {"L", "LA"}
    ]
    counts = {
        status: sum(row.get("Availability") == status for row in children)
        for status in ("Available", "NotReleased", "Unknown")
    }
    if args.report:
        report_rows: list[dict[str, Any]] = []
        for parent_id in sorted(parent_ids, key=natural_key):
            parent_children = [
                row for row in children
                if row.get("ParentUppslagId") == parent_id
            ]
            available_ids = [
                row["Name"] for row in parent_children
                if row.get("Availability") == "Available"
            ]
            missing_ids = [
                row["Name"] for row in parent_children
                if row.get("Availability") == "NotReleased"
            ]
            unknown_ids = [
                row["Name"] for row in parent_children
                if row.get("Availability") == "Unknown"
            ]
            report_rows.append({
                "ParentUppslagId": parent_id,
                "SubcaseCount": len(parent_children),
                "AvailableCount": len(available_ids),
                "NotReleasedCount": len(missing_ids),
                "UnknownCount": len(unknown_ids),
                "AvailableIds": available_ids,
                "NotReleasedIds": missing_ids,
                "UnknownIds": unknown_ids,
            })
        args.report.write_text(
            json.dumps({
                "Source": "https://wpu.nu/wiki/Avsnitt:L",
                "MainCaseCount": len(parent_ids),
                "SubcaseCount": len(children),
                "AvailableCount": counts["Available"],
                "NotReleasedCount": counts["NotReleased"],
                "UnknownCount": counts["Unknown"],
                "Parents": report_rows,
            }, ensure_ascii=False, indent=2) + "\n",
            encoding="utf-8",
        )
    print(f"Main cases: {len(parent_ids)}")
    print(f"Subcases: {len(children)}")
    print(f"Available on WPU: {counts['Available']}")
    print(f"Not released/missing: {counts['NotReleased']}")
    print(f"Unknown: {counts['Unknown']}")


if __name__ == "__main__":
    main()
