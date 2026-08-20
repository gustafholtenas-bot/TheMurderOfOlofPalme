"""Create TMOP's first obscured/unknown appearance materials in Unreal Editor.

Run from Unreal Engine 5.8: Tools > Execute Python Script.
The script is idempotent and never deletes an existing asset.
"""

import unreal


ROOT = "/Game/TMOP/Characters/Appearance"
MATERIAL_DIR = f"{ROOT}/Materials/Unknown"
MESH_DIR = f"{ROOT}/Meshes/Unknown"
DATA_DIR = f"{ROOT}/Data"
MASTER_NAME = "M_TMOP_Obscured"


INSTANCE_SPECS = {
    "MI_TMOP_UnknownFace": (0.34, 0.31, 0.29, 1.0),
    "MI_TMOP_UnknownHair": (0.12, 0.12, 0.13, 1.0),
    "MI_TMOP_UnknownOuterwear": (0.10, 0.13, 0.18, 1.0),
    "MI_TMOP_UnknownUpperBody": (0.22, 0.23, 0.25, 1.0),
    "MI_TMOP_UnknownTrousers": (0.12, 0.13, 0.15, 1.0),
    "MI_TMOP_UnknownFootwear": (0.055, 0.055, 0.06, 1.0),
    "MI_TMOP_UnknownGloves": (0.07, 0.07, 0.075, 1.0),
    "MI_TMOP_UnknownHeadwear": (0.14, 0.15, 0.17, 1.0),
}


def log(message):
    unreal.log(f"TMOP Appearance: {message}")


def ensure_directories():
    for path in (ROOT, MATERIAL_DIR, MESH_DIR, DATA_DIR):
        if not unreal.EditorAssetLibrary.does_directory_exist(path):
            unreal.EditorAssetLibrary.make_directory(path)


def get_or_create_asset(asset_name, package_path, asset_class, factory):
    object_path = f"{package_path}/{asset_name}"
    existing = unreal.EditorAssetLibrary.load_asset(object_path)
    if existing:
        log(f"Reusing {object_path}")
        return existing, False
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        asset_name, package_path, asset_class, factory
    )
    if not asset:
        raise RuntimeError(f"Could not create {object_path}")
    log(f"Created {object_path}")
    return asset, True


def create_master_material():
    material, created = get_or_create_asset(
        MASTER_NAME, MATERIAL_DIR, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not created and not hasattr(
        unreal.MaterialEditingLibrary, "delete_all_material_expressions"
    ):
        log("Existing master material kept; this Unreal version cannot rebuild expressions.")
        return material
    if not created:
        unreal.MaterialEditingLibrary.delete_all_material_expressions(material)

    material.set_editor_property("two_sided", True)
    material.set_editor_property("used_with_skeletal_mesh", True)
    material.set_editor_property("used_with_morph_targets", True)
    try:
        material.set_editor_property(
            "shading_model", unreal.MaterialShadingModel.MSM_UNLIT
        )
    except Exception:
        log("Unlit shading property is managed by this engine version; emissive fallback used.")

    color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -480, -80
    )
    color.set_editor_property("parameter_name", "PrimaryColor")
    color.set_editor_property(
        "default_value", unreal.LinearColor(0.16, 0.17, 0.19, 1.0)
    )

    shadow_color = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionVectorParameter, -480, 50
    )
    shadow_color.set_editor_property("parameter_name", "ObscuredShadowColor")
    shadow_color.set_editor_property(
        "default_value", unreal.LinearColor(0.07, 0.075, 0.085, 1.0)
    )

    unknown_amount = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, -480, 190
    )
    unknown_amount.set_editor_property("parameter_name", "TMOP_IsUnknown")
    unknown_amount.set_editor_property("default_value", 1.0)

    obscurity = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionScalarParameter, -480, 300
    )
    obscurity.set_editor_property("parameter_name", "TMOP_ObscurityAmount")
    obscurity.set_editor_property("default_value", 1.0)

    fresnel = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionFresnel, -230, 90
    )

    rim_amount = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionMultiply, 0, 130
    )
    featureless_gradient = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionLinearInterpolate, 210, -20
    )

    unreal.MaterialEditingLibrary.connect_material_expressions(
        fresnel, "", rim_amount, "A"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        obscurity, "", rim_amount, "B"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        color, "", featureless_gradient, "A"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        shadow_color, "", featureless_gradient, "B"
    )
    unreal.MaterialEditingLibrary.connect_material_expressions(
        rim_amount, "", featureless_gradient, "Alpha"
    )

    # Unlit, textureless colour removes facial/clothing surface details. A soft
    # Fresnel gradient keeps the silhouette readable without implying evidence.
    unreal.MaterialEditingLibrary.connect_material_property(
        featureless_gradient, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR
    )
    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material, False)
    return material


def create_instances(master):
    created_assets = []
    for name, rgba in INSTANCE_SPECS.items():
        instance, _ = get_or_create_asset(
            name,
            MATERIAL_DIR,
            unreal.MaterialInstanceConstant,
            unreal.MaterialInstanceConstantFactoryNew(),
        )
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, master)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            instance, "PrimaryColor", unreal.LinearColor(*rgba)
        )
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(
            instance,
            "ObscuredShadowColor",
            unreal.LinearColor(rgba[0] * 0.48, rgba[1] * 0.48, rgba[2] * 0.48, 1.0),
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "TMOP_IsUnknown", 1.0
        )
        unreal.MaterialEditingLibrary.set_material_instance_scalar_parameter_value(
            instance, "TMOP_ObscurityAmount", 1.0
        )
        unreal.MaterialEditingLibrary.update_material_instance(instance)
        unreal.EditorAssetLibrary.save_loaded_asset(instance, False)
        created_assets.append(instance)
    return created_assets


def print_next_steps():
    log("Unknown material set is ready.")
    log(f"Import compatible fallback skeletal meshes into {MESH_DIR}.")
    log("Import DT_TMOP_AppearanceAssets_Unknown.csv after compiling TMOPEngine.")
    log("Assign DT_TMOP_AppearanceAssets to TMOPPersonRegistryDirector.")


def main():
    ensure_directories()
    master = create_master_material()
    create_instances(master)
    print_next_steps()
    unreal.EditorDialog.show_message(
        "TMOP Appearance",
        "Graded featureless material assets were created or updated.\n\n"
        "Next: import the fallback skeletal meshes and the supplied CSV catalog.",
        unreal.AppMsgType.OK,
    )


if __name__ == "__main__":
    main()
