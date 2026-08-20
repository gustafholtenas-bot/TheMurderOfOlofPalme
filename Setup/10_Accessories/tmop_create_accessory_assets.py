"""Duplicate configured TMOP facial-hair, glasses and scarf meshes."""
import unreal

ROOT = "/Game/TMOP/Characters/Appearance/Accessories/1986"
SOURCE_MESHES = {
    "MUSTACHE": "", "BEARD": "", "STUBBLE": "",
    "GLASSES_PLASTIC": "", "GLASSES_METAL": "",
    "SCARF_WOOL": "",
}
OUTPUTS = {
    "SK_TMOP_Mustache": "MUSTACHE",
    "SK_TMOP_Beard": "BEARD",
    "SK_TMOP_Stubble": "STUBBLE",
    "SK_TMOP_Glasses_Plastic": "GLASSES_PLASTIC",
    "SK_TMOP_Glasses_Metal": "GLASSES_METAL",
    "SK_TMOP_Scarf_Wool": "SCARF_WOOL",
}

def main():
    if not unreal.EditorAssetLibrary.does_directory_exist(ROOT):
        unreal.EditorAssetLibrary.make_directory(ROOT)
    created, missing = 0, []
    for output, key in OUTPUTS.items():
        destination = f"{ROOT}/{output}"
        if unreal.EditorAssetLibrary.does_asset_exist(destination): continue
        source = SOURCE_MESHES.get(key, "")
        if not source or not unreal.EditorAssetLibrary.does_asset_exist(source):
            missing.append(key); continue
        if unreal.EditorAssetLibrary.duplicate_asset(source, destination):
            unreal.EditorAssetLibrary.save_asset(destination, only_if_is_dirty=False)
            created += 1
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    unreal.log(f"TMOP: {created} accessory mesh aliases created.")
    if missing: unreal.log_warning("TMOP accessory sources needed: " + ", ".join(sorted(set(missing))))
    unreal.EditorDialog.show_message(
        "TMOP Accessories", f"Created: {created}. Missing sources: {len(set(missing))}.",
        unreal.AppMsgType.OK)

if __name__ == "__main__": main()
