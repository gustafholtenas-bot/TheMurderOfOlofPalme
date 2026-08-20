"""Create TMOP face/hair materials and duplicate configured modular meshes."""
import unreal

ROOT = "/Game/TMOP/Characters/Appearance"
FACE_DIR = f"{ROOT}/Faces/Standard"
HAIR_DIR = f"{ROOT}/Hair/1986"
MATERIAL_DIR = f"{ROOT}/Materials/Hair1986"

# Fill these after compatible modular assets have been imported.
SOURCE_MESHES = {
    "FACE_MALE": "", "FACE_FEMALE": "", "FACE_UNISEX": "",
    "HAIR_SHORT": "", "HAIR_MEDIUM": "", "HAIR_LONG": "",
    "HAIR_CURLY": "", "HAIR_RECEDING": "",
}
MESH_OUTPUTS = {
    "SK_TMOP_Face_Male_Generic": (FACE_DIR, "FACE_MALE"),
    "SK_TMOP_Face_Female_Generic": (FACE_DIR, "FACE_FEMALE"),
    "SK_TMOP_Face_Unisex_Generic": (FACE_DIR, "FACE_UNISEX"),
    "SK_TMOP_Hair_Short": (HAIR_DIR, "HAIR_SHORT"),
    "SK_TMOP_Hair_Medium": (HAIR_DIR, "HAIR_MEDIUM"),
    "SK_TMOP_Hair_Long": (HAIR_DIR, "HAIR_LONG"),
    "SK_TMOP_Hair_Curly": (HAIR_DIR, "HAIR_CURLY"),
    "SK_TMOP_Hair_Receding": (HAIR_DIR, "HAIR_RECEDING"),
}
HAIR_COLORS = {
    "Black": (0.012, 0.009, 0.008, 1.0),
    "Dark": (0.035, 0.022, 0.015, 1.0),
    "Brown": (0.115, 0.055, 0.025, 1.0),
    "Blond": (0.43, 0.31, 0.16, 1.0),
    "Grey": (0.30, 0.30, 0.29, 1.0),
    "White": (0.62, 0.61, 0.57, 1.0),
    "Red": (0.31, 0.075, 0.022, 1.0),
}

def ensure_directory(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)

def get_or_create(name, folder, asset_class, factory):
    path = f"{folder}/{name}"
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if asset:
        return asset, False
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, folder, asset_class, factory)
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    return asset, True

def create_hair_master():
    material, created = get_or_create(
        "M_TMOP_Hair1986", MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew())
    if not created:
        return material
    material.set_editor_property("two_sided", True)
    material.set_editor_property("used_with_skeletal_mesh", True)
    material.set_editor_property("used_with_morph_targets", True)
    color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -350, -50)
    color.set_editor_property("parameter_name", "PrimaryColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.04, 0.025, 0.018, 1.0))
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, -350, 80)
    roughness.set_editor_property("parameter_name", "Roughness")
    roughness.set_editor_property("default_value", 0.62)
    unreal.MaterialEditingLibrary.connect_material_property(
        color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, False)
    return material

def create_hair_materials(master):
    for color_name, rgba in HAIR_COLORS.items():
        instance, _ = get_or_create(
            f"MI_TMOP_Hair_{color_name}", MATERIAL_DIR,
            unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, master)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            instance, "PrimaryColor", unreal.LinearColor(*rgba))
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "Roughness", 0.62)
        unreal.MaterialEditingLibrary.update_material_instance(instance)
        unreal.EditorAssetLibrary.save_loaded_asset(instance, False)

def duplicate_meshes():
    created, missing = 0, []
    for output_name, (folder, source_key) in MESH_OUTPUTS.items():
        destination = f"{folder}/{output_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(destination):
            continue
        source = SOURCE_MESHES.get(source_key, "")
        if not source or not unreal.EditorAssetLibrary.does_asset_exist(source):
            missing.append(source_key); continue
        if unreal.EditorAssetLibrary.duplicate_asset(source, destination):
            unreal.EditorAssetLibrary.save_asset(destination, only_if_is_dirty=False)
            created += 1
    return created, sorted(set(missing))

def main():
    for path in (FACE_DIR, HAIR_DIR, MATERIAL_DIR): ensure_directory(path)
    create_hair_materials(create_hair_master())
    created, missing = duplicate_meshes()
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    unreal.log(f"TMOP: face/hair setup ready; {created} mesh aliases created.")
    if missing: unreal.log_warning("TMOP face/hair sources needed: " + ", ".join(missing))
    unreal.EditorDialog.show_message(
        "TMOP Faces and Hair",
        f"Seven hair materials ready. Meshes: {created}. Missing sources: {len(missing)}.",
        unreal.AppMsgType.OK)

if __name__ == "__main__": main()
