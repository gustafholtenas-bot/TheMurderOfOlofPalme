"""Build TMOP's cumulative Appearance Asset DataTable in Unreal Editor 5.8.

The script merges every DT_TMOP_AppearanceAssets_*.csv below Setup, preserves
custom rows already present in the DataTable, creates a timestamped backup, and
assigns the result to loaded TMOPPersonRegistryDirector actors.
"""

import csv
import io
from datetime import datetime
from pathlib import Path

import unreal


TARGET_FOLDER = "/Game/TMOP/Characters/Appearance/Data"
TARGET_NAME = "DT_TMOP_AppearanceAssets"
BACKUP_FOLDER = f"{TARGET_FOLDER}/Backups"
EXPECTED_STRUCT_NAME = "TMOPAppearanceAssetRow"
CSV_PATTERN = "DT_TMOP_AppearanceAssets_*.csv"


def log(message):
    unreal.log(f"TMOP Appearance Table: {message}")


def load_rows(csv_text, source_name):
    reader = csv.DictReader(io.StringIO(csv_text))
    if not reader.fieldnames or "CatalogId" not in reader.fieldnames:
        raise RuntimeError(f"{source_name} has an invalid Appearance Asset header")
    source_header = list(reader.fieldnames)
    key_field = "Name" if "Name" in source_header else source_header[0]
    header = list(source_header)
    header[0] = "Name"
    rows = {}
    for line_number, row in enumerate(reader, start=2):
        name = (row.get(key_field) or "").strip()
        catalog_id = (row.get("CatalogId") or "").strip()
        if not name or not catalog_id:
            raise RuntimeError(f"{source_name}:{line_number} has an empty Name or CatalogId")
        if name in rows:
            raise RuntimeError(f"Duplicate row name {name} inside {source_name}")
        canonical_row = {column: row.get(column, "") for column in source_header}
        if key_field != "Name":
            canonical_row.pop(key_field, None)
        canonical_row["Name"] = name
        rows[name] = {column: canonical_row.get(column, "") for column in header}
    return header, rows


def find_source_csv_files():
    setup_root = Path(__file__).resolve().parents[1]
    files = sorted(
        path for path in setup_root.rglob(CSV_PATTERN)
        if "05_DataTableBuilder" not in str(path)
    )
    if not files:
        raise RuntimeError(f"No {CSV_PATTERN} files found below {setup_root}")
    return files


def merge_managed_csv_files():
    header = None
    merged = {}
    catalog_ids = {}
    sources = []
    for path in find_source_csv_files():
        file_header, rows = load_rows(path.read_text(encoding="utf-8-sig"), path.name)
        if header is None:
            header = file_header
        elif file_header != header:
            raise RuntimeError(f"CSV header mismatch in {path.name}")
        for name, row in rows.items():
            if name in merged:
                raise RuntimeError(f"Duplicate managed row name {name}: {path.name}")
            catalog_id = row["CatalogId"].strip()
            if catalog_id in catalog_ids:
                raise RuntimeError(
                    f"Duplicate CatalogId {catalog_id}: {catalog_ids[catalog_id]} and {path.name}"
                )
            merged[name] = row
            catalog_ids[catalog_id] = path.name
        sources.append(path.name)
    return header, merged, sources


def get_row_struct():
    struct_type = getattr(unreal, EXPECTED_STRUCT_NAME, None)
    if struct_type and hasattr(struct_type, "static_struct"):
        return struct_type.static_struct()
    row_struct = unreal.load_object(
        None, f"/Script/TMOPEngine.{EXPECTED_STRUCT_NAME}"
    )
    if not row_struct:
        raise RuntimeError(
            "FTMOPAppearanceAssetRow is unavailable. Compile TMOPEngine and restart Unreal first."
        )
    return row_struct


def create_table_if_missing(row_struct):
    target_path = f"{TARGET_FOLDER}/{TARGET_NAME}"
    table = unreal.EditorAssetLibrary.load_asset(target_path)
    if table:
        existing_struct = table.get_row_struct()
        if existing_struct != row_struct:
            raise RuntimeError(f"{target_path} uses the wrong Row Struct")
        return table, False
    if not unreal.EditorAssetLibrary.does_directory_exist(TARGET_FOLDER):
        unreal.EditorAssetLibrary.make_directory(TARGET_FOLDER)
    factory = unreal.DataTableFactory()
    factory.set_editor_property("struct", row_struct)
    table = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        TARGET_NAME, TARGET_FOLDER, unreal.DataTable, factory
    )
    if not table:
        raise RuntimeError(f"Could not create {target_path}")
    return table, True


def backup_existing_table(table):
    if not table.get_row_names():
        return None
    if not unreal.EditorAssetLibrary.does_directory_exist(BACKUP_FOLDER):
        unreal.EditorAssetLibrary.make_directory(BACKUP_FOLDER)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    backup_path = f"{BACKUP_FOLDER}/{TARGET_NAME}_{stamp}"
    source_path = f"{TARGET_FOLDER}/{TARGET_NAME}"
    if not unreal.EditorAssetLibrary.duplicate_asset(source_path, backup_path):
        raise RuntimeError("Could not create a safety backup of the existing DataTable")
    unreal.EditorAssetLibrary.save_asset(backup_path, only_if_is_dirty=False)
    return backup_path


def read_existing_custom_rows(table, managed_names):
    exported = table.export_to_csv_string()
    if not exported:
        return {}, None
    header, rows = load_rows(exported, "existing DataTable")
    return {name: row for name, row in rows.items() if name not in managed_names}, header


def rows_to_csv(header, rows):
    output = io.StringIO(newline="")
    writer = csv.DictWriter(output, fieldnames=header, lineterminator="\n")
    writer.writeheader()
    for name in sorted(rows):
        writer.writerow(rows[name])
    return output.getvalue()


def assign_loaded_directors(table):
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    assigned = 0
    for actor in subsystem.get_all_level_actors():
        if actor.get_class().get_name() not in (
            "TMOPPersonRegistryDirector",
            "BP_TMOPPersonRegistryDirector_C",
        ) and "PersonRegistryDirector" not in actor.get_class().get_name():
            continue
        try:
            actor.set_editor_property("appearance_asset_table", table)
            actor.modify()
            assigned += 1
        except Exception as error:
            unreal.log_warning(f"TMOP: could not assign table to {actor.get_name()}: {error}")
    return assigned


def main():
    header, managed_rows, sources = merge_managed_csv_files()
    row_struct = get_row_struct()
    table, created = create_table_if_missing(row_struct)
    custom_rows, existing_header = read_existing_custom_rows(table, set(managed_rows))
    if existing_header and existing_header != header:
        raise RuntimeError("Existing DataTable export header differs from the managed CSV header")
    backup_path = None if created else backup_existing_table(table)
    final_rows = dict(custom_rows)
    final_rows.update(managed_rows)
    if not table.fill_from_csv_string(rows_to_csv(header, final_rows), row_struct):
        raise RuntimeError("Unreal rejected the merged Appearance Asset CSV")
    unreal.EditorAssetLibrary.save_loaded_asset(table, only_if_is_dirty=False)
    assigned = assign_loaded_directors(table)
    log(
        f"Built {TARGET_NAME}: {len(managed_rows)} managed + {len(custom_rows)} custom rows; "
        f"assigned to {assigned} loaded director(s)."
    )
    log("Sources: " + ", ".join(sources))
    if backup_path:
        log(f"Safety backup: {backup_path}")
    unreal.EditorDialog.show_message(
        "TMOP Appearance DataTable",
        f"Ready: {len(final_rows)} rows. Custom rows preserved: {len(custom_rows)}.\n"
        f"Loaded directors updated: {assigned}.",
        unreal.AppMsgType.OK,
    )


if __name__ == "__main__":
    main()
