"""Audit TMOP appearance assets inside Unreal Editor 5.8.

Run after building DT_TMOP_AppearanceAssets. The script never changes assets;
it writes a CSV report below Saved/TMOP/AppearanceAudit.
"""

import csv
import io
from datetime import datetime
from pathlib import Path

import unreal


TABLE_PATH = "/Game/TMOP/Characters/Appearance/Data/DT_TMOP_AppearanceAssets"
MINIMUM_LOD_COUNT = 3
EXPECTED_BODY_MORPHS = (
    "TMOP_BodyWeight",
    "TMOP_Muscularity",
    "TMOP_HeadScale",
    "TMOP_ShoulderScale",
    "TMOP_TorsoLength",
    "TMOP_ArmLength",
    "TMOP_LegLength",
)
REPORT_COLUMNS = (
    "Severity", "CatalogId", "PartType", "Check", "AssetPath", "Details"
)


def log(message):
    unreal.log(f"TMOP Appearance Audit: {message}")


def warning(message):
    unreal.log_warning(f"TMOP Appearance Audit: {message}")


def normalized_asset_path(value):
    value = (value or "").strip()
    if not value or value.lower() in {"none", "null"}:
        return ""
    if value.startswith(("SkeletalMesh'", "Material'", "MaterialInstanceConstant'")):
        value = value.split("'", 1)[1].rsplit("'", 1)[0]
    return value


def load_catalog_rows(table):
    text = table.export_to_csv_string()
    if not text:
        raise RuntimeError("Appearance DataTable could not be exported")
    rows = list(csv.DictReader(io.StringIO(text)))
    if not rows or "CatalogId" not in rows[0]:
        raise RuntimeError("Appearance DataTable uses an unexpected row format")
    return rows


def add_issue(issues, severity, row, check, asset_path, details):
    issues.append({
        "Severity": severity,
        "CatalogId": (row.get("CatalogId") or row.get("Name") or "").strip(),
        "PartType": (row.get("PartType") or "").strip(),
        "Check": check,
        "AssetPath": asset_path,
        "Details": details,
    })


def object_path(asset):
    if not asset:
        return ""
    try:
        return asset.get_path_name()
    except Exception:
        return str(asset)


def get_skeleton_path(mesh):
    try:
        return object_path(mesh.get_editor_property("skeleton"))
    except Exception:
        return ""


def get_lod_count(mesh):
    for function_name in ("get_lod_num", "get_num_lods"):
        function = getattr(mesh, function_name, None)
        if callable(function):
            try:
                return int(function())
            except Exception:
                pass
    library = getattr(unreal, "EditorSkeletalMeshLibrary", None)
    function = getattr(library, "get_lod_count", None) if library else None
    if callable(function):
        try:
            return int(function(mesh))
        except Exception:
            pass
    return None


def get_morph_names(mesh):
    function = getattr(mesh, "get_morph_targets", None)
    if not callable(function):
        return None
    try:
        return {str(morph.get_name()) for morph in function()}
    except Exception:
        return None


def is_known_empty_part(row):
    catalog_id = (row.get("CatalogId") or "").upper()
    return catalog_id.startswith("NONE_")


def audit_rows(rows):
    issues = []
    loaded_meshes = []
    body_skeletons = set()

    for row in rows:
        mesh_path = normalized_asset_path(row.get("Mesh"))
        material_path = normalized_asset_path(row.get("Material"))
        part_type = (row.get("PartType") or "").strip()
        obscured = (row.get("bObscuredFallback") or "").strip().lower() == "true"

        if not mesh_path and not is_known_empty_part(row):
            add_issue(issues, "ERROR", row, "Mesh", "", "No mesh is assigned.")
        elif mesh_path:
            mesh = unreal.load_asset(mesh_path)
            if not mesh:
                add_issue(issues, "ERROR", row, "Mesh", mesh_path,
                          "The referenced mesh asset does not exist.")
            elif not isinstance(mesh, unreal.SkeletalMesh):
                add_issue(issues, "ERROR", row, "MeshType", mesh_path,
                          f"Expected SkeletalMesh, found {mesh.get_class().get_name()}.")
            else:
                loaded_meshes.append((row, mesh, mesh_path))
                skeleton_path = get_skeleton_path(mesh)
                if not skeleton_path:
                    add_issue(issues, "ERROR", row, "Skeleton", mesh_path,
                              "The skeletal mesh has no skeleton.")
                if part_type == "Body" and skeleton_path:
                    body_skeletons.add(skeleton_path)

                lod_count = get_lod_count(mesh)
                if lod_count is None:
                    add_issue(issues, "INFO", row, "LOD", mesh_path,
                              "LOD count could not be read by this Unreal build.")
                elif lod_count < MINIMUM_LOD_COUNT:
                    add_issue(issues, "WARNING", row, "LOD", mesh_path,
                              f"Only {lod_count} LOD(s); at least {MINIMUM_LOD_COUNT} are recommended.")

                if part_type == "Body":
                    morph_names = get_morph_names(mesh)
                    if morph_names is None:
                        add_issue(issues, "INFO", row, "MorphTargets", mesh_path,
                                  "Morph targets could not be enumerated by this Unreal build.")
                    else:
                        missing = [name for name in EXPECTED_BODY_MORPHS
                                   if name not in morph_names]
                        if missing:
                            add_issue(issues, "WARNING", row, "MorphTargets", mesh_path,
                                      "Missing: " + ", ".join(missing))

        if material_path:
            material = unreal.load_asset(material_path)
            if not material:
                add_issue(issues, "ERROR", row, "Material", material_path,
                          "The referenced material asset does not exist.")
            elif not isinstance(material, unreal.MaterialInterface):
                add_issue(issues, "ERROR", row, "MaterialType", material_path,
                          f"Expected MaterialInterface, found {material.get_class().get_name()}.")
        elif obscured:
            add_issue(issues, "ERROR", row, "ObscuredMaterial", "",
                      "An obscured fallback row must have a material.")

    if len(body_skeletons) > 1:
        synthetic = {"CatalogId": "ALL_BODY_ROWS", "PartType": "Body"}
        add_issue(issues, "ERROR", synthetic, "SharedSkeleton", "",
                  "Body rows use multiple skeletons: " + "; ".join(sorted(body_skeletons)))

    if body_skeletons:
        for row, mesh, mesh_path in loaded_meshes:
            if (row.get("PartType") or "").strip() == "Body":
                continue
            skeleton_path = get_skeleton_path(mesh)
            if skeleton_path and skeleton_path not in body_skeletons:
                add_issue(issues, "ERROR", row, "SkeletonCompatibility", mesh_path,
                          f"Uses {skeleton_path}, not the body skeleton.")

    return issues


def write_report(rows, issues):
    project_saved = Path(unreal.Paths.project_saved_dir())
    report_folder = project_saved / "TMOP" / "AppearanceAudit"
    report_folder.mkdir(parents=True, exist_ok=True)
    stamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    report_path = report_folder / f"TMOP_Appearance_Audit_{stamp}.csv"
    with report_path.open("w", encoding="utf-8-sig", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=REPORT_COLUMNS)
        writer.writeheader()
        writer.writerows(issues)
    return report_path


def main():
    table = unreal.load_asset(TABLE_PATH)
    if not table or not isinstance(table, unreal.DataTable):
        raise RuntimeError(
            f"{TABLE_PATH} is missing. Run Setup/05_DataTableBuilder first."
        )
    rows = load_catalog_rows(table)
    issues = audit_rows(rows)
    report_path = write_report(rows, issues)
    counts = {level: sum(1 for issue in issues if issue["Severity"] == level)
              for level in ("ERROR", "WARNING", "INFO")}
    summary = (
        f"{len(rows)} catalog rows checked. "
        f"Errors: {counts['ERROR']}, warnings: {counts['WARNING']}, "
        f"info: {counts['INFO']}.\n\nReport: {report_path}"
    )
    log(summary.replace("\n", " "))
    unreal.EditorDialog.show_message(
        "TMOP Appearance Asset Audit", summary, unreal.AppMsgType.OK
    )


if __name__ == "__main__":
    main()
