"""Select the live Appearance DataTable in Unreal, then execute this file.

Maps imported skeletal hair by exact CatalogId (also accepts SK_/SKM_ prefixes).
Keeps materials and all non-hair rows. Saves a JSON backup before modifying the
table in memory; review Output Log, then save the DataTable manually.
"""
import copy
import json
from datetime import datetime, timezone
from pathlib import Path
from uuid import uuid4

DRY_RUN = False
SEARCH_ROOT = "/Game"
# Optional explicit CatalogId -> skeletal asset path for differently named files.
EXPLICIT_MAPPING = {}
BODY_MESHES = {
    "Male": "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple",
    "Female": "/Game/Characters/Mannequins/Meshes/SKM_Quinn_Simple",
}
IDENTITY = {
    "Rotation": {"X": 0, "Y": 0, "Z": 0, "W": 1},
    "Translation": {"X": 0, "Y": 0, "Z": 0},
    "Scale3D": {"X": 1, "Y": 1, "Z": 1},
}


def object_path(value):
    value = str(value or "").strip()
    if "'" in value:
        value = value.split("'", 2)[1]
    if value in ("", "None"):
        return ""
    return value if "." in value else value + "." + value.rsplit("/", 1)[-1]


def plan_mapping(rows, assets, explicit=None):
    """Pure planner. assets contains only real SkeletalMesh paths and asset names."""
    explicit = explicit or {}
    by_path = {object_path(a["path"]): a for a in assets}
    result = copy.deepcopy(rows)
    changes, missing = [], []
    for row in result:
        if row.get("PartType", "").split("::")[-1] != "Hair":
            continue
        cid = row.get("CatalogId") or row["Name"]
        if cid in explicit:
            candidates = [object_path(explicit[cid])]
        else:
            names = {cid, row["Name"], "SK_" + cid, "SKM_" + cid}
            candidates = [p for p, a in by_path.items() if a["name"] in names]
            if not candidates:
                # Existing references are only accepted if the registry confirms
                # that they now point to actual skeletal assets.
                candidates = [object_path(row.get(field)) for field in ("Mesh", "StaticMesh")
                              if object_path(row.get(field)) in by_path]
        candidates = sorted(set(candidates))
        if len(candidates) != 1 or candidates[0] not in by_path:
            missing.append((cid, "ambiguous matches" if len(candidates) > 1 else "skeletal mesh missing"))
            continue
        path = candidates[0]
        new_row = copy.deepcopy(row)
        new_row.update(Mesh=path, StaticMesh="None", AttachmentSocket="None",
                       AttachmentTransform=copy.deepcopy(IDENTITY))
        if "PendingAsset" in new_row.get("Tags", []):
            new_row["Tags"].remove("PendingAsset")
        if new_row != row:
            changes.append((cid, path, row.get("Gender", "Unknown").split("::")[-1]))
            row.clear()
            row.update(new_row)
    return result, changes, missing


def main():
    import unreal
    tables = []
    for table in unreal.EditorUtilityLibrary.get_selected_assets():
        if not isinstance(table, unreal.DataTable):
            continue
        original = unreal.DataTableFunctionLibrary.export_data_table_to_json_string(table)
        rows = json.loads(original)
        if rows and all("PartType" in r and "CatalogId" in r for r in rows):
            tables.append((table, original, rows))
    if len(tables) != 1:
        raise RuntimeError("Select exactly one live DT_TMOP_AppearanceAssets in Content Browser")
    table, original, rows = tables[0]
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.scan_paths_synchronous([SEARCH_ROOT], force_rescan=False)
    assets = []
    for data in registry.get_assets_by_path(SEARCH_ROOT, recursive=True):
        if str(data.asset_class_path.asset_name) == "SkeletalMesh":
            assets.append({"name": str(data.asset_name),
                           "path": str(data.package_name) + "." + str(data.asset_name)})
    updated, changes, missing = plan_mapping(rows, assets, EXPLICIT_MAPPING)
    for cid, reason in missing:
        unreal.log_warning(f"TMOP hair: {cid}: {reason}; row left unchanged")
    # Check the actual skeleton before touching the DataTable.
    bodies = {gender: unreal.load_asset(path) for gender, path in BODY_MESHES.items()}
    for cid, path, gender in changes:
        mesh = unreal.load_asset(path)
        if not isinstance(mesh, unreal.SkeletalMesh):
            raise RuntimeError(f"{cid}: asset failed to load as SkeletalMesh: {path}")
        genders = [gender] if gender in bodies else list(bodies)
        for required in genders:
            body = bodies[required]
            if not isinstance(body, unreal.SkeletalMesh):
                raise RuntimeError(f"Set BODY_MESHES for {required}: body asset not found")
            if mesh.get_editor_property("skeleton") != body.get_editor_property("skeleton"):
                raise RuntimeError(f"{cid}: import on the SAME Skeleton asset as {required} body: {path}")
        unreal.log(f"TMOP hair: {cid} -> {path}")
    if not changes or DRY_RUN:
        unreal.log(f"TMOP hair: {len(changes)} planned, {len(missing)} unresolved. No table changes.")
        return
    backup_dir = Path(unreal.Paths.project_saved_dir()) / "TMOP_Hair_Backups"
    backup_dir.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now(timezone.utc).strftime("%Y%m%d_%H%M%S")
    backup = backup_dir / f"{table.get_name()}_{stamp}_{uuid4().hex[:8]}.json"
    backup.write_text(original, encoding="utf-8")
    try:
        if not unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(
                table, json.dumps(updated, ensure_ascii=False)):
            raise RuntimeError("DataTable import failed")
    except Exception:
        restored = unreal.DataTableFunctionLibrary.fill_data_table_from_json_string(table, original)
        unreal.log_warning(f"TMOP hair rollback: {restored}. Backup: {backup}")
        raise
    unreal.log(f"TMOP hair: updated {len(changes)} rows; {len(missing)} unresolved. "
               f"Backup: {backup}. Review and Save the DataTable.")


if __name__ == "__main__":
    main()
