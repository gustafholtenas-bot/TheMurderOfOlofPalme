"""Unreal Engine 5.8 Editor Python script: install TMOP civilian fabric assets.

Creates a NEW master and six NEW fabric MIs. Does NOT touch existing
M_TMOP_Clothing1986 or MI_TMOP_Clothing_* color instances, or bespoke police
and guard materials. Re-running the script does not overwrite existing assets.

Run INSIDE the Unreal Editor through Tools > Execute Python Script.
"""

import unreal


FOLDER = "/Game/TMOP/Characters/Appearance/Materials/Clothing1986"
MASTER_NAME = "M_TMOP_ClothingFabric1986"

# Generic engine textures let a user create the material before importing any
# custom cloth maps. Replace the sample parameters in each new MI afterward.
WHITE_TEXTURE_PATHS = (
    "/Engine/EngineResources/WhiteSquareTexture",
    "/Engine/EngineResources/DefaultTexture",
    "/Engine/EngineMaterials/DefaultWhiteGrid",
)
NORMAL_TEXTURE_PATHS = (
    "/Engine/EngineMaterials/DefaultNormal",
    "/Engine/EngineResources/DefaultNormal",
)

# Optional: enter imported texture asset paths here before running.
# Each type accepts Color, Normal, and Roughness. Unfilled entries use neutral
# engine textures; colour tint still comes from PrimaryColor in the NPC MID.
CUSTOM_TEXTURES = {
    "Wool": {"Color": "", "Normal": "", "Roughness": ""},
    "Leather": {"Color": "", "Normal": "", "Roughness": ""},
    "Quilted": {"Color": "", "Normal": "", "Roughness": ""},
    "Cotton": {"Color": "", "Normal": "", "Roughness": ""},
    "Denim": {"Color": "", "Normal": "", "Roughness": ""},
    "Synthetic": {"Color": "", "Normal": "", "Roughness": ""},
}

# Surface defaults are independent of the NPC's evidence-based clothing colour.
FABRICS = {
    "Wool": {"Roughness": 0.90, "TextureStrength": 0.55, "TextureTiling": 1.0},
    "Leather": {"Roughness": 0.43, "TextureStrength": 0.55, "TextureTiling": 1.0},
    "Quilted": {"Roughness": 0.76, "TextureStrength": 0.55, "TextureTiling": 1.0},
    "Cotton": {"Roughness": 0.86, "TextureStrength": 0.50, "TextureTiling": 1.0},
    "Denim": {"Roughness": 0.82, "TextureStrength": 0.60, "TextureTiling": 1.0},
    "Synthetic": {"Roughness": 0.64, "TextureStrength": 0.50, "TextureTiling": 1.0},
}

mel = unreal.MaterialEditingLibrary
eal = unreal.EditorAssetLibrary


def report(message):
    unreal.log("TMOP Fabrics: " + message)


def find_first_asset(candidates, required_type):
    for path in candidates:
        result = unreal.load_asset(path)
        if result and isinstance(result, required_type):
            return result
    return None


def create_if_missing(name, cls, factory):
    path = FOLDER + "/" + name
    existing = eal.load_asset(path)
    if existing:
        if not isinstance(existing, cls):
            raise TypeError("Asset name already occupied by another class: " + path)
        report("Already exists, keeping unchanged: " + name)
        return existing, False
    asset = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name, FOLDER, cls, factory
    )
    if not asset:
        raise RuntimeError("Could not create " + path)
    return asset, True


def node(material, kind, x, y):
    result = mel.create_material_expression(material, kind, x, y)
    if not result:
        raise RuntimeError("Could not create material node " + str(kind))
    return result


def scalar(material, name, default, x, y):
    result = node(material, unreal.MaterialExpressionScalarParameter, x, y)
    result.set_editor_property("parameter_name", name)
    result.set_editor_property("default_value", float(default))
    return result


def vector(material, name, color, x, y):
    result = node(material, unreal.MaterialExpressionVectorParameter, x, y)
    result.set_editor_property("parameter_name", name)
    result.set_editor_property("default_value", color)
    return result


def texture(material, name, initial_texture, x, y, normal_map=False):
    result = node(material, unreal.MaterialExpressionTextureSampleParameter2D, x, y)
    result.set_editor_property("parameter_name", name)
    result.set_editor_property("texture", initial_texture)
    if normal_map:
        result.set_editor_property(
            "sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL
        )
    return result


def connect(source, target, input_name, output_name=""):
    if not mel.connect_material_expressions(source, output_name, target, input_name):
        raise RuntimeError("Failed to connect material graph input " + input_name)


def connect_property(source, target_property, output_name=""):
    if not mel.connect_material_property(source, output_name, target_property):
        raise RuntimeError("Failed to connect material property " + str(target_property))


def make_master(white, neutral_normal):
    master, created = create_if_missing(
        MASTER_NAME, unreal.Material, unreal.MaterialFactoryNew()
    )
    if not created:
        return master

    # Color: lerp(white, base-color-texture, TextureStrength) * PrimaryColor.
    # A white/neutral texture keeps the original NPC colour exactly unchanged.
    primary = vector(master, "PrimaryColor", unreal.LinearColor(1, 1, 1, 1), -620, -200)
    strength = scalar(master, "TextureStrength", 0.50, -780, -40)
    tiling = scalar(master, "TextureTiling", 1.0, -1120, 260)

    texcoord = node(master, unreal.MaterialExpressionTextureCoordinate, -1300, 220)
    tiled_uv = node(master, unreal.MaterialExpressionMultiply, -920, 230)
    connect(texcoord, tiled_uv, "A")
    connect(tiling, tiled_uv, "B")

    color_tex = texture(master, "FabricColor", white, -650, 80)
    connect(tiled_uv, color_tex, "UVs")
    white_vector = node(master, unreal.MaterialExpressionConstant3Vector, -640, -30)
    white_vector.set_editor_property("constant", unreal.LinearColor(1, 1, 1, 1))
    lerp = node(master, unreal.MaterialExpressionLinearInterpolate, -290, -20)
    connect(white_vector, lerp, "A")
    connect(color_tex, lerp, "B", "RGB")
    connect(strength, lerp, "Alpha")
    tinted = node(master, unreal.MaterialExpressionMultiply, -60, -130)
    connect(primary, tinted, "A")
    connect(lerp, tinted, "B")
    connect_property(tinted, unreal.MaterialProperty.MP_BASE_COLOR)

    # Roughness: a white map leaves Roughness unaffected. A real grayscale map
    # can be chosen per MI without modifying this master.
    roughness = scalar(master, "Roughness", 0.80, -560, 550)
    rough_tex = texture(master, "FabricRoughness", white, -660, 680)
    connect(tiled_uv, rough_tex, "UVs")
    rough_mul = node(master, unreal.MaterialExpressionMultiply, -160, 550)
    connect(roughness, rough_mul, "A")
    connect(rough_tex, rough_mul, "B", "R")
    connect_property(rough_mul, unreal.MaterialProperty.MP_ROUGHNESS)

    # Normal: use a flat tangent-space normal until the user imports real maps.
    normal_tex = texture(master, "FabricNormal", neutral_normal, -600, 930,
                         normal_map=True)
    connect(tiled_uv, normal_tex, "UVs")
    connect_property(normal_tex, unreal.MaterialProperty.MP_NORMAL, "RGB")

    mel.layout_material_expressions(master)
    compile_errors = mel.recompile_material(master)
    if compile_errors:
        raise RuntimeError("Fabric master shader compilation failed: " +
                           "; ".join(str(item) for item in compile_errors))
    eal.save_loaded_asset(master)
    report("Created new master: " + MASTER_NAME)
    return master


def make_instance(fabric, settings, master, default_white, default_normal):
    name = "MI_TMOP_Fabric_" + fabric
    mi, created = create_if_missing(
        name, unreal.MaterialInstanceConstant,
        unreal.MaterialInstanceConstantFactoryNew(),
    )
    if not created:
        return
    mel.set_material_instance_parent(mi, master)
    mel.set_material_instance_vector_parameter_value(
        mi, "PrimaryColor", unreal.LinearColor(1, 1, 1, 1)
    )
    for param, value in settings.items():
        mel.set_material_instance_scalar_parameter_value(mi, param, value)

    texture_roles = {
        "Color": ("FabricColor", default_white),
        "Normal": ("FabricNormal", default_normal),
        "Roughness": ("FabricRoughness", default_white),
    }
    for role, (parameter, fallback) in texture_roles.items():
        custom_path = CUSTOM_TEXTURES[fabric].get(role, "")
        custom_texture = unreal.load_asset(custom_path) if custom_path else None
        if custom_path and not isinstance(custom_texture, unreal.Texture2D):
            raise RuntimeError("Invalid texture for %s/%s: %s" % (
                fabric, role, custom_path
            ))
        mel.set_material_instance_texture_parameter_value(
            mi, parameter, custom_texture or fallback
        )
    eal.save_loaded_asset(mi)
    report("Created: " + name)


def main():
    if not eal.does_directory_exist(FOLDER):
        eal.make_directory(FOLDER)
    white = find_first_asset(WHITE_TEXTURE_PATHS, unreal.Texture2D)
    normal = find_first_asset(NORMAL_TEXTURE_PATHS, unreal.Texture2D)
    if not white or not normal:
        raise RuntimeError(
            "Could not load neutral Engine textures. Set WHITE_TEXTURE_PATHS "
            "and NORMAL_TEXTURE_PATHS to existing texture assets first; "
            "no materials were created."
        )
    master = make_master(white, normal)
    for kind, options in FABRICS.items():
        make_instance(kind, options, master, white, normal)
    report("Complete. Add real PBR FabricColor, FabricNormal and FabricRoughness "
           "maps to each of the six MIs, then test a civilian NPC.")


if __name__ == "__main__":
    main()
