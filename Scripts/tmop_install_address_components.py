"""Connect the loaded level's existing address anchors to the resident registry.

Run with Unreal Editor: Tools > Execute Python Script.
Rebuild TMOPEngine first. The script does not move/spawn actors or save the level.
Save All after inspecting the report. Run again freely; existing components and
hand-adjusted interaction offsets are preserved. Set DRY_RUN to True to preview.
"""

import collections
import datetime
import json
import os
import re
import unicodedata

REGISTRY_PATH = "/Game/TMOP/Data/DT_TMOP_AddressRegistry"
DRY_RUN = False


def name(value):
    text = str(value).strip() if value is not None else ""
    return "" if text.casefold() == "none" else text


def address_key(value):
    """Exact normalized address, preserving house ranges and entrance letters."""
    text = name(value).casefold().replace("–", "-").replace("—", "-")
    text = "".join(c for c in unicodedata.normalize("NFKD", text)
                   if not unicodedata.combining(c))
    text = re.sub(r"(?<=\d)[_\s]+(?=\d)", "-", text)
    return "".join(c for c in text if c.isalnum() or c == "-")


def plan_bindings(rows, anchors, registry_path):
    """Pure planning step, completed before any level/table mutation."""
    plan = []
    claims = collections.defaultdict(list)
    for row in rows:
        row_name = name(row.get("Name"))
        address = "{} {}{}".format(name(row.get("StreetName")),
                                    row.get("StreetNumber", 0), name(row.get("EntranceSuffix")))
        entry = {"row": row_name, "address": address, "status": "missing", "candidates": []}
        explicit = name(row.get("EntranceAnchorId")) or name(row.get("BuildingAnchorId"))
        if explicit:
            matches = [a for a in anchors if name(a["id"]).casefold() == explicit.casefold()]
            entry["match_method"] = "explicit_anchor_id"
        else:
            matches = [a for a in anchors if a.get("registry") == registry_path
                       and name(a.get("row")).casefold() == row_name.casefold()]
            entry["match_method"] = "existing_component"
            if not matches:
                tag = name(row.get("DoorbellActorTag")).casefold()
                matches = [a for a in anchors if tag and tag in
                           {name(t).casefold() for t in a.get("tags", [])}]
                entry["match_method"] = "doorbell_tag"
            if not matches:
                keys = {address_key(v) for v in (address, row.get("AddressId"), row_name)} - {""}
                matches = [a for a in anchors if keys.intersection(
                    address_key(a.get(field)) for field in ("id", "label", "display"))]
                entry["match_method"] = "exact_address"
        entry["candidates"] = [a["path"] for a in matches]
        if len(matches) > 1:
            entry["status"] = "ambiguous"
        elif len(matches) == 1:
            entry["status"] = "candidate"
            entry["anchor"] = matches[0]["path"]
            claims[matches[0]["path"]].append(entry)
        elif explicit:
            entry["reason"] = "Explicit AnchorId saknas bland laddade ankare: " + explicit
        plan.append(entry)
    for entries in claims.values():
        if len(entries) > 1:
            for entry in entries:
                entry["status"] = "conflict"
                entry["reason"] = "Flera adressrader matchar samma ankare."
    return plan


def install_address_components(registry_path=REGISTRY_PATH, dry_run=DRY_RUN):
    import unreal

    library = getattr(unreal, "TMOPAddressEditorLibrary", None)
    anchor_class = getattr(unreal, "TMOPHistoricalAnchor", None)
    component_class = getattr(unreal, "TMOPAddressComponent", None)
    if library is None or anchor_class is None or component_class is None:
        raise RuntimeError("Bygg om projektets Development Editor med den nya adresskoden först.")
    table = unreal.load_asset(registry_path)
    if not isinstance(table, unreal.DataTable):
        raise RuntimeError("Adressregistret hittades inte: " + registry_path)
    row_struct = unreal.DataTableFunctionLibrary.get_data_table_row_struct(table)
    if row_struct is None or row_struct.get_name() != "TMOPAddressRegistryRow":
        raise RuntimeError("Tabellen måste använda radtypen TMOPAddressRegistryRow.")
    exported = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
    if not exported:
        raise RuntimeError("Adressregistret kunde inte läsas.")
    rows = json.loads(exported)
    if not isinstance(rows, list) or not rows or any(not name(r.get("Name")) for r in rows):
        raise RuntimeError("Tomt eller ogiltigt adressregister.")

    actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors()
    by_path = {}
    anchors = []
    for actor in actors:
        if not isinstance(actor, anchor_class):
            continue
        path = actor.get_path_name()
        by_path[path] = actor
        components = actor.get_components_by_class(component_class)
        component = components[0] if components else None
        existing_table = component.get_editor_property("registry") if component else None
        anchors.append({
            "path": path, "id": name(actor.get_anchor_id()),
            "label": actor.get_actor_label(), "display": name(actor.get_editor_property("display_name")),
            "tags": [name(t) for t in actor.get_editor_property("tags")],
            "registry": existing_table.get_path_name() if existing_table else "",
            "row": name(component.get_editor_property("row_name")) if component else "",
            "component_count": len(components),
        })
    table_path = table.get_path_name()
    plan = plan_bindings(rows, anchors, table_path)
    records = {a["path"]: a for a in anchors}

    # Check the full plan before attaching anything, including existing table links,
    # duplicate IDs/components and actors already using this row elsewhere.
    for entry in plan:
        if entry["status"] != "candidate":
            continue
        actor = by_path[entry["anchor"]]
        error = library.bind_address_anchor(actor, table, unreal.Name(entry["row"]), True)
        if error:
            entry.update(status="conflict", reason=str(error))

    with unreal.ScopedEditorTransaction("Koppla TMOP-adresskomponenter"):
        for entry in plan:
            if entry["status"] != "candidate":
                continue
            actor = by_path[entry["anchor"]]
            record = records[entry["anchor"]]
            existing = record["registry"] == table_path and record["row"].casefold() == entry["row"].casefold()
            if dry_run:
                entry["status"] = "already_connected" if existing else "would_connect"
                continue
            error = library.bind_address_anchor(actor, table, unreal.Name(entry["row"]), False)
            if error:
                entry.update(status="error", reason=str(error))
            else:
                entry["status"] = "already_connected" if existing else "connected"

    summary = dict(collections.Counter(e["status"] for e in plan))
    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    directory = os.path.abspath(os.path.join(unreal.Paths.project_saved_dir(), "TMOP", "Reports"))
    os.makedirs(directory, exist_ok=True)
    report_path = os.path.join(directory, "address_components_" + stamp + ".json")
    with open(report_path, "w", encoding="utf-8") as output:
        json.dump({"registry": table_path, "dry_run": dry_run, "loaded_anchors": len(anchors),
                   "summary": summary, "addresses": plan}, output, ensure_ascii=False, indent=2)
    unreal.log("TMOP adresser: " + json.dumps(summary, ensure_ascii=False))
    unreal.log("Rapport: " + report_path)
    unreal.log("Kopplingen omfattar bara laddade delar av banan. Ladda fler delar och kör igen vid behov.")
    if not dry_run:
        unreal.log("Spara tabellen och banan med Save All. Ctrl+Z ångrar scriptets kopplingar.")
    if any(e["status"] in ("missing", "ambiguous", "conflict", "error") for e in plan):
        unreal.log_warning("Alla adresser kunde inte kopplas. Se rapportens orsaker och kandidater.")
    return report_path


if __name__ == "__main__":
    install_address_components()
