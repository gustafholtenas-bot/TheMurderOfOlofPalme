"""Bootstrap the first modular 1986 TMOP clothing set in Unreal Editor 5.8.

Fill SOURCE_MESHES with compatible modular skeletal meshes when they exist.
The script always creates the material set and duplicates every configured mesh.
It is idempotent and never deletes assets.
"""

import unreal


ROOT = "/Game/TMOP/Characters/Appearance"
MESH_DIR = f"{ROOT}/Clothing/1986/Meshes"
MATERIAL_DIR = f"{ROOT}/Materials/Clothing1986"
MASTER_NAME = "M_TMOP_Clothing1986"

# Supply real modular skeletal meshes later. All must use the same skeleton as
# the selected TMOP body. Empty paths are reported and safely skipped.
SOURCE_MESHES = {
    "COAT_LONG": "",
    "COAT_SHORT": "",
    "JACKET_STANDARD": "",
    "JACKET_LEATHER": "",
    "PARKA": "",
    "SAILING_JACKET": "",
    "SHIRT": "",
    "SWEATER": "",
    "TROUSERS_STRAIGHT": "",
    "JEANS": "",
    "DRESS_TROUSERS": "",
    "SHOES": "",
    "BOOTS": "",
    "WINTER_BOOTS": "",
    "GLOVES": "",
    "KNIT_CAP": "",
    "FUR_HAT": "",
    "BRIMMED_HAT": "",
    "CAP": "",
}

COLORS = {
    "Black": (0.025, 0.025, 0.03, 1.0),
    "Charcoal": (0.075, 0.08, 0.09, 1.0),
    "DarkBlue": (0.025, 0.055, 0.12, 1.0),
    "Navy": (0.018, 0.035, 0.085, 1.0),
    "DarkGrey": (0.12, 0.13, 0.145, 1.0),
    "Grey": (0.27, 0.28, 0.30, 1.0),
    "Brown": (0.18, 0.095, 0.045, 1.0),
    "DarkBrown": (0.075, 0.038, 0.022, 1.0),
    "Beige": (0.48, 0.39, 0.25, 1.0),
    "Olive": (0.16, 0.17, 0.075, 1.0),
    "Red": (0.32, 0.025, 0.025, 1.0),
    "White": (0.72, 0.72, 0.68, 1.0),
}

# Output name, source slot. Several colored catalog rows can share one mesh.
MESH_OUTPUTS = {
    "SK_TMOP_Coat_Long": "COAT_LONG",
    "SK_TMOP_Coat_Short": "COAT_SHORT",
    "SK_TMOP_Jacket_Standard": "JACKET_STANDARD",
    "SK_TMOP_Jacket_Leather": "JACKET_LEATHER",
    "SK_TMOP_Parka": "PARKA",
    "SK_TMOP_SailingJacket": "SAILING_JACKET",
    "SK_TMOP_Shirt": "SHIRT",
    "SK_TMOP_Sweater": "SWEATER",
    "SK_TMOP_Trousers_Straight": "TROUSERS_STRAIGHT",
    "SK_TMOP_Jeans": "JEANS",
    "SK_TMOP_DressTrousers": "DRESS_TROUSERS",
    "SK_TMOP_Shoes": "SHOES",
    "SK_TMOP_Boots": "BOOTS",
    "SK_TMOP_WinterBoots": "WINTER_BOOTS",
    "SK_TMOP_Gloves": "GLOVES",
    "SK_TMOP_KnitCap": "KNIT_CAP",
    "SK_TMOP_FurHat": "FUR_HAT",
    "SK_TMOP_BrimmedHat": "BRIMMED_HAT",
    "SK_TMOP_Cap": "CAP",
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
        name, folder, asset_class, factory
    )
    if not asset:
        raise RuntimeError(f"Could not create {path}")
    return asset, True


def create_master_material():
    material, created = get_or_create(
        MASTER_NAME, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not created:
        return material
    material.set_editor_property("two_sided", True)
    color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -420, -80
    )
    color.set_editor_property("parameter_name", "PrimaryColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.1, 0.1, 0.1, 1.0))
    roughness = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, -420, 80
    )
    roughness.set_editor_property("parameter_name", "Roughness")
    roughness.set_editor_property("default_value", 0.72)
    unreal.MaterialEditingLibrary.connect_material_property(
        color, "", unreal.MaterialProperty.MP_BASE_COLOR
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness, "", unreal.MaterialProperty.MP_ROUGHNESS
    )
    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, False)
    return material


def create_color_materials(master):
    for color_name, rgba in COLORS.items():
        name = f"MI_TMOP_Clothing_{color_name}"
        instance, _ = get_or_create(
            name,
            MATERIAL_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, master)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            instance, "PrimaryColor", unreal.LinearColor(*rgba)
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "Roughness", 0.72
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "TMOP_IsUnknown", 0.0
        )
        unreal.MaterialEditingLibrary.update_material_instance(instance)
        unreal.EditorAssetLibrary.save_loaded_asset(instance, False)


def duplicate_meshes():
    created = 0
    missing = []
    for output_name, source_key in MESH_OUTPUTS.items():
        destination = f"{MESH_DIR}/{output_name}"
        if unreal.EditorAssetLibrary.does_asset_exist(destination):
            continue
        source = SOURCE_MESHES.get(source_key, "")
        if not source or not unreal.EditorAssetLibrary.does_asset_exist(source):
            missing.append(source_key)
            continue
        if unreal.EditorAssetLibrary.duplicate_asset(source, destination):
            unreal.EditorAssetLibrary.save_asset(destination, only_if_is_dirty=False)
            created += 1
        else:
            unreal.log_error(f"TMOP: could not duplicate {source} to {destination}")
    return created, sorted(set(missing))


def main():
    ensure_directory(MESH_DIR)
    ensure_directory(MATERIAL_DIR)
    master = create_master_material()
    create_color_materials(master)
    created, missing = duplicate_meshes()
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    unreal.log(f"TMOP: 12 clothing color materials ready; {created} meshes created.")
    if missing:
        unreal.log_warning(
            "TMOP: source meshes still needed in SOURCE_MESHES: " + ", ".join(missing)
        )
    unreal.EditorDialog.show_message(
        "TMOP 1986 Clothing",
        f"Material set ready. Meshes created: {created}. Missing source slots: {len(missing)}.",
        unreal.AppMsgType.OK,
    )


if __name__ == "__main__":
    main()
