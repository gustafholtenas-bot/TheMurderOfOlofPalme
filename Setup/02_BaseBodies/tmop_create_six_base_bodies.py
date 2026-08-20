"""Create six TMOP body asset aliases from two compatible skeletal meshes.

Run inside Unreal Editor 5.8 after changing MALE_SOURCE_MESH and
FEMALE_SOURCE_MESH below. The source meshes must use the same skeleton as the
animations and modular garments used by ATMOPHistoricalAgent.
"""

import unreal


MALE_SOURCE_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Manny"
FEMALE_SOURCE_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Quinn"
OUTPUT_FOLDER = "/Game/TMOP/Characters/Appearance/Bodies"

BODY_ASSETS = (
    ("SK_TMOP_Body_Male_Slim", MALE_SOURCE_MESH),
    ("SK_TMOP_Body_Male_Average", MALE_SOURCE_MESH),
    ("SK_TMOP_Body_Male_Heavy", MALE_SOURCE_MESH),
    ("SK_TMOP_Body_Female_Slim", FEMALE_SOURCE_MESH),
    ("SK_TMOP_Body_Female_Average", FEMALE_SOURCE_MESH),
    ("SK_TMOP_Body_Female_Heavy", FEMALE_SOURCE_MESH),
)


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def duplicate_if_missing(name, source_path):
    destination = f"{OUTPUT_FOLDER}/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(destination):
        unreal.log(f"TMOP: already exists: {destination}")
        return destination
    if not unreal.EditorAssetLibrary.does_asset_exist(source_path):
        raise RuntimeError(
            f"TMOP source mesh is missing: {source_path}. "
            "Change MALE_SOURCE_MESH/FEMALE_SOURCE_MESH at the top of the script."
        )
    if not unreal.EditorAssetLibrary.duplicate_asset(source_path, destination):
        raise RuntimeError(f"TMOP could not duplicate {source_path} to {destination}")
    unreal.EditorAssetLibrary.save_asset(destination, only_if_is_dirty=False)
    unreal.log(f"TMOP: created {destination}")
    return destination


def main():
    ensure_folder(OUTPUT_FOLDER)
    created = [duplicate_if_missing(name, source) for name, source in BODY_ASSETS]
    unreal.EditorAssetLibrary.save_directory(OUTPUT_FOLDER, only_if_is_dirty=False, recursive=True)
    unreal.log(f"TMOP: six body aliases are ready ({len(created)} assets).")
    unreal.log("TMOP: now import DT_TMOP_AppearanceAssets_Bodies.csv into the appearance DataTable.")


if __name__ == "__main__":
    main()
