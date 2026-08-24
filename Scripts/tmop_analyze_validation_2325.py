#!/usr/bin/env python3
"""Auditable person-by-person analysis of the schema-v3 TMOP validation export."""

from __future__ import annotations

import csv
import json
import sys
from collections import Counter, defaultdict
from pathlib import Path


def number(row, key, default=-1.0):
    try:
        return float(row.get(key, default))
    except (TypeError, ValueError):
        return default


def main() -> int:
    folder = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("validation_latest_2325")
    package = Path(sys.argv[2]) if len(sys.argv) > 2 else Path("TMOP_ValidationDrivenFix_v5_20260823")
    report_path = Path(sys.argv[3]) if len(sys.argv) > 3 else Path("validation_latest_2325/person_review.md")
    main_csv = next(folder.glob("TimelineValidation_*.csv"))
    # Exclude the similarly prefixed snapshot/summary files.
    main_csv = next(p for p in folder.glob("TimelineValidation_*.csv") if not any(x in p.name for x in ("Snapshots", "Summary")))
    agent_csv = next(folder.glob("*_AgentSnapshots.csv"))
    summary_csv = next(folder.glob("*_EntitySummary.csv"))

    with main_csv.open(encoding="utf-8-sig", newline="") as handle:
        records = list(csv.DictReader(handle))
    with agent_csv.open(encoding="utf-8-sig", newline="") as handle:
        snapshots = list(csv.DictReader(handle))
    with summary_csv.open(encoding="utf-8-sig", newline="") as handle:
        summaries = {r["EntityId"]: r for r in csv.DictReader(handle)}
    with (package / "Database/DT_TMOP_People.json").open(encoding="utf-16") as handle:
        people = {r["EntityId"]: r for r in json.load(handle)}

    by_entity = defaultdict(list)
    snaps_by_entity = defaultdict(list)
    for row in records:
        by_entity[row["EntityId"]].append(row)
    for row in snapshots:
        snaps_by_entity[row["EntityId"]].append(row)

    spawned = sorted({r["EntityId"] for r in records if r["Event"] == "Spawned"})
    spawn_counts = Counter(r["EntityId"] for r in records if r["Event"] == "Spawned")
    rows = []
    issue_counts = Counter()

    for entity in spawned:
        recs = by_entity[entity]
        snaps = sorted(snaps_by_entity[entity], key=lambda r: int(r["SampleSecond"]))
        profile = people.get(entity, {})
        first = snaps[0] if snaps else {}
        last = snaps[-1] if snaps else {}
        issues = []
        positives = []

        raw_stuck = [r for r in recs if r["Event"] == "Stuck"]
        false_group_stuck = [r for r in raw_stuck if
            r.get("GroupState") == "Arrived" or
            (r.get("GroupState") == "Moving" and r.get("GroupLeaderId") not in ("", "None", entity))]
        stuck = len(raw_stuck) - len(false_group_stuck)
        failures = [r for r in recs if r["Event"] == "Failed" and r["Severity"] == "Error"]
        impossible = [r for r in recs if r["Event"] == "TravelPreflight" and r["PhysicallyPossible"].lower() == "false"]
        late = [r for r in recs if r["Event"] == "Completed" and number(r, "TimeDeviationSeconds", 0) > 30]
        delayed = [r for r in recs if r["Event"] in ("Started", "Applied") and number(r, "TimeDeviationSeconds", 0) > 30]
        missed = [r for r in recs if r["Event"] == "Completed" and number(r, "DistanceToTargetCm") > 100]
        # Schema v3 measured from capsule centre, normally ~90 cm above the
        # floor. Values above 150 cm remain suspicious even after that bias.
        off_nav = [s for s in snaps if s["OnNavMesh"].lower() == "false" and number(s, "DistanceToNavMeshCm") > 150 and s["VehicleId"] in ("", "None")]
        sunk_start = bool(first) and first["VehicleId"] in ("", "None") and number(first, "DistanceToNavMeshCm") > 150
        collision_disabled = sum(s["CollisionEnabled"].lower() == "false" for s in snaps)

        if failures:
            codes = Counter(r["FailureCode"] or "UnclassifiedActionFailure" for r in failures)
            issues.append("failure:" + ",".join(f"{k}×{v}" for k, v in codes.items()))
            issue_counts["runtime failures"] += 1
        if stuck:
            issues.append(f"stuck×{stuck}")
            issue_counts["stuck"] += 1
        if false_group_stuck:
            positives.append(f"validator-group-false-positive×{len(false_group_stuck)}")
        if impossible:
            issues.append(f"impossible-arrival×{len(impossible)}")
            issue_counts["impossible arrival"] += 1
        if late:
            issues.append(f"late>30s×{len(late)}")
            issue_counts["late arrival"] += 1
        if delayed:
            issues.append(f"delayed-action>30s×{len(delayed)}")
            issue_counts["delayed timeline action"] += 1
        if missed:
            issues.append(f"outside-anchor×{len(missed)}")
            issue_counts["anchor miss"] += 1
        if sunk_start:
            issues.append(f"spawn-off-nav:{number(first, 'DistanceToNavMeshCm'):.0f}cm")
            issue_counts["bad initial placement"] += 1
        elif off_nav:
            issues.append(f"off-nav-samples×{len(off_nav)}")
            issue_counts["off navmesh"] += 1
        if spawn_counts[entity] > 1:
            positives.append(f"respawned×{spawn_counts[entity]}")
        if collision_disabled:
            positives.append(f"pass-through-samples×{collision_disabled}")
        if any(r["Event"] == "Completed" and r["Severity"] == "Passed" for r in recs):
            positives.append("completed-on-time")
        if not issues:
            positives.append("no-recorded-error")

        last_active = last.get("ActiveEntryId", "")
        last_target = last.get("TargetAnchorId", "")
        group = last.get("GroupId", profile.get("SocialGroupId", ""))
        rows.append((entity, profile.get("CategoryId", ""), group, "; ".join(issues) or "—", "; ".join(positives) or "—", last_active, last_target))

    failures_by_code = Counter(r["FailureCode"] or "UnclassifiedActionFailure" for r in records if r["Event"] == "Failed" and r["Severity"] == "Error")
    lines = [
        "# TMOP validation – person review through 23:26:41",
        "",
        f"- Unique spawned people: {len(spawned)}",
        f"- Timeline records: {len(records)}",
        f"- Agent snapshots: {len(snapshots)}",
        f"- People with no detected issue: {sum(r[3] == '—' for r in rows)}",
        f"- People with one or more detected issues: {sum(r[3] != '—' for r in rows)}",
        "- Issue classes: " + ", ".join(f"{k}={v}" for k, v in issue_counts.most_common()),
        "- Runtime failure codes: " + ", ".join(f"{k}={v}" for k, v in failures_by_code.most_common()),
        "",
        "## Every spawned person",
        "",
        "| Entity | Category | Group | Problems | Working evidence | Last active entry | Last target |",
        "|---|---|---|---|---|---|---|",
    ]
    for row in rows:
        lines.append("| " + " | ".join(str(v).replace("|", "\\|") for v in row) + " |")
    report_path.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"Wrote {report_path} with {len(rows)} spawned people")
    print("Issue classes:", issue_counts)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
