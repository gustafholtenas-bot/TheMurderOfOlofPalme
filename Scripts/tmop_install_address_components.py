"""Install the complete address registry on existing loaded address anchors.

Run in Unreal Editor with Tools > Execute Python Script while the game map and all
relevant World Partition cells/sublevels are loaded. The bundled registry is merged
into the existing DataTable without changing the asset identity. Existing explicit
anchor links win over empty values in the JSON. No anchor is moved or spawned.
"""

import collections
import datetime
import json
import os
import re
import unicodedata

REGISTRY_PATH = "/Game/TMOP/Data/DT_TMOP_AddressRegistry"
SOURCE_JSON_RELATIVE = os.path.join("DataTables", "09_13", "DT_TMOP_AddressRegistry.json")
DRY_RUN = False
AUTO_SAVE = True
LINK_FIELDS = ("EntranceAnchorId", "BuildingAnchorId", "DoorbellActorTag")
KNOWN_ANCHOR_PREFIXES = (
    "tmopaddress", "tmopadress", "addressanchor", "adressankare",
    "doorbell", "address", "adress", "entrance", "anchor", "ankare", "port",
)


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


def comparable_keys(value):
    """Return an exact key plus variants with a known technical prefix removed."""
    key = address_key(value)
    keys = {key} if key else set()
    for prefix in KNOWN_ANCHOR_PREFIXES:
        if key.startswith(prefix) and len(key) > len(prefix):
            keys.add(key[len(prefix):])
    return keys


def validate_rows(rows, label):
    if not isinstance(rows, list) or not rows:
        raise ValueError(label + " är tomt eller inte en JSON-lista.")
    row_names = set()
    address_ids = set()
    for row in rows:
        row_name = name(row.get("Name"))
        address_id = name(row.get("AddressId"))
        if not row_name or not address_id:
            raise ValueError(label + " innehåller en rad utan Name eller AddressId.")
        if row_name.casefold() in row_names:
            raise ValueError(label + " innehåller dubbelt radnamn: " + row_name)
        if address_id.casefold() in address_ids:
            raise ValueError(label + " innehåller dubbelt AddressId: " + address_id)
        row_names.add(row_name.casefold())
        address_ids.add(address_id.casefold())


def merge_registry_rows(current_rows, incoming_rows):
    """Merge full incoming data while preserving editor-created anchor links and rows."""
    validate_rows(current_rows, "Befintlig DataTable")
    validate_rows(incoming_rows, "Ny adressfil")
    current_by_id = {name(row["AddressId"]).casefold(): row for row in current_rows}
    current_names = {name(row["Name"]).casefold(): name(row["AddressId"]).casefold()
                     for row in current_rows}
    incoming_ids = set()
    merged = []
    preserved_links = 0
    for source in incoming_rows:
        row = dict(source)
        row_id = name(row["AddressId"]).casefold()
        row_name = name(row["Name"]).casefold()
        if row_name in current_names and current_names[row_name] != row_id:
            raise ValueError("Radnamnet {} pekar på olika AddressId.".format(row["Name"]))
        incoming_ids.add(row_id)
        old = current_by_id.get(row_id)
        if old:
            for field in LINK_FIELDS:
                if not name(row.get(field)) and name(old.get(field)):
                    row[field] = old[field]
                    preserved_links += 1
        merged.append(row)
    table_only = []
    for old in current_rows:
        if name(old["AddressId"]).casefold() not in incoming_ids:
            merged.append(dict(old))
            table_only.append(name(old["Name"]))
    validate_rows(merged, "Sammanslaget adressregister")
    return merged, {"incoming_rows": len(incoming_rows), "merged_rows": len(merged),
                    "preserved_link_fields": preserved_links,
                    "preserved_table_only_rows": table_only}


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
                row_keys = set()
                for value in (address, row.get("RegistrySearchText"),
                              row.get("AddressId"), row_name):
                    row_keys.update(comparable_keys(value))
                matches = []
                for anchor in anchors:
                    anchor_keys = set()
                    for field in ("id", "label", "display"):
                        anchor_keys.update(comparable_keys(anchor.get(field)))
                    for tag_value in anchor.get("tags", []):
                        anchor_keys.update(comparable_keys(tag_value))
                    if row_keys.intersection(anchor_keys):
                        matches.append(anchor)
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


def validate_anchor_state(plan, anchors, registry_path):
    """Reject duplicate IDs/components and existing links before any mutation."""
    records = {a["path"]: a for a in anchors}
    id_counts = collections.Counter(name(a["id"]).casefold() for a in anchors if name(a["id"]))
    for entry in plan:
        if entry["status"] != "candidate":
            continue
        anchor = records[entry["anchor"]]
        if id_counts[name(anchor["id"]).casefold()] > 1:
            entry.update(status="conflict", reason="Flera laddade ankare delar samma AnchorId.")
        elif anchor.get("component_count", 0) > 1:
            entry.update(status="conflict", reason="Ankaret har flera adresskomponenter.")
        elif anchor.get("row") and (anchor.get("registry") != registry_path or
                                    name(anchor.get("row")).casefold() != entry["row"].casefold()):
            entry.update(status="conflict", reason="Ankaret har redan en annan adresskoppling.")


def report_directory(unreal):
    directory = os.path.abspath(os.path.join(unreal.Paths.project_saved_dir(), "TMOP", "Reports"))
    os.makedirs(directory, exist_ok=True)
    return directory


def install_address_components(registry_path=REGISTRY_PATH, dry_run=DRY_RUN,
                               source_json_relative=SOURCE_JSON_RELATIVE, auto_save=AUTO_SAVE):
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
        raise RuntimeError("Det befintliga adressregistret kunde inte läsas.")
    current_rows = json.loads(exported)
    source_path = os.path.abspath(os.path.join(unreal.Paths.project_dir(), source_json_relative))
    if not os.path.isfile(source_path):
        raise RuntimeError("Den fullständiga adressfilen saknas: " + source_path)
    with open(source_path, encoding="utf-8") as source_file:
        incoming_rows = json.load(source_file)
    rows, merge_summary = merge_registry_rows(current_rows, incoming_rows)

    stamp = datetime.datetime.now(datetime.timezone.utc).strftime("%Y%m%dT%H%M%S%fZ")
    directory = report_directory(unreal)
    backup_path = os.path.join(directory, "address_registry_before_" + stamp + ".json")
    with open(backup_path, "w", encoding="utf-8") as backup:
        json.dump(current_rows, backup, ensure_ascii=False, indent=2)

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
    validate_anchor_state(plan, anchors, table_path)
    records = {a["path"]: a for a in anchors}

    if not dry_run:
        import_error = library.replace_address_registry_json(
            table, json.dumps(rows, ensure_ascii=False), False)
        if import_error:
            raise RuntimeError("Adressregistret uppdaterades inte: " + str(import_error))
        for entry in plan:
            if entry["status"] != "candidate":
                continue
            error = library.bind_address_anchor(
                by_path[entry["anchor"]], table, unreal.Name(entry["row"]), True)
            if error:
                entry.update(status="conflict", reason=str(error))

    for entry in plan:
        if entry["status"] != "candidate":
            continue
        actor = by_path[entry["anchor"]]
        record = records[entry["anchor"]]
        existing = (record["registry"] == table_path and
                    record["row"].casefold() == entry["row"].casefold())
        if dry_run:
            entry["status"] = "already_connected" if existing else "would_connect"
            continue
        error = library.bind_address_anchor(actor, table, unreal.Name(entry["row"]), False)
        if error:
            entry.update(status="error", reason=str(error))
        else:
            entry["status"] = "already_connected" if existing else "connected"

    summary = dict(collections.Counter(e["status"] for e in plan))
    unresolved = [e for e in plan if e["status"] in ("missing", "ambiguous", "conflict", "error")]
    report_path = os.path.join(directory, "address_components_" + stamp + ".json")
    report = {
        "registry": table_path, "source_json": source_path, "dry_run": dry_run,
        "auto_save": auto_save, "loaded_anchors": len(anchors),
        "merge": merge_summary, "summary": summary,
        "all_loaded_addresses_connected": not unresolved,
        "addresses": plan,
    }
    with open(report_path, "w", encoding="utf-8") as output:
        json.dump(report, output, ensure_ascii=False, indent=2)

    saved = False
    if auto_save and not dry_run and not any(e["status"] == "error" for e in plan):
        table_saved = unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=True)
        level_saved = unreal.EditorLevelLibrary.save_current_level()
        saved = bool(table_saved and level_saved)
        report["saved"] = saved
        with open(report_path, "w", encoding="utf-8") as output:
            json.dump(report, output, ensure_ascii=False, indent=2)

    unreal.log("TMOP adresser: " + json.dumps(summary, ensure_ascii=False))
    unreal.log("Registermerge: " + json.dumps(merge_summary, ensure_ascii=False))
    unreal.log("Rapport: " + report_path)
    unreal.log("Backup före registeruppdateringen: " + backup_path)
    if auto_save and not dry_run:
        unreal.log("Adressregistret och aktuell bana sparades." if saved else
                   "Automatisk sparning blev inte fullständig; använd Save All.")
    if unresolved:
        unreal.log_warning(
            "{} adresser saknar en säker koppling i laddad bana. Se rapporten; inga positioner gissades."
            .format(len(unresolved)))
    else:
        unreal.log("Alla adresser i den laddade banan är kopplade till varsitt ankare.")
    return report_path


if __name__ == "__main__":
    install_address_components()
