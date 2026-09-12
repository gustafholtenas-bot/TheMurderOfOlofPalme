"""Run in Unreal's Python console after rebuilding TMOPEngine; then Save All.

Creates a ghost material, a director and starter corners at existing killer-route
anchors. Authored alternatives are added through each corner's Next Corners.
Existing actors, materials, routes and data-table rows are never overwritten.
"""
import unreal

NETWORK = "KillerEscape"
MATERIAL_PATH = "/Game/TMOP/Killer/M_TMOP_KillerGhost"
RUN_PATH = "/Game/TMOP/Animation/Animations/walkRun/A_TMOP_RunFast"
MAIN_ANCHORS = (
    "ANCHOR_PAGE31_THE_KILLER_TP02_TURNS_TO_TUNNELGATAN",
    "Tunnelgatan_Stairs_down",
    "D21659_TUNNELGATAN_STAIRS_TOP",
    "Killer_sees_yvonne",
    "Anki_runinto_by_killer",
)
TAG_PREFIX = "TMOP_KILLER_CORNER_"
ARRIVAL_HINTS = {
    "Tunnelgatan_Stairs_down": ("Palme_shot_1", 25.0),
    "D21659_TUNNELGATAN_STAIRS_TOP": ("Palme_shot_1", 39.0),
    "Killer_sees_yvonne": ("Yvonne_meets_killer", 0.0),
    "Anki_runinto_by_killer": ("Samson_robrandt_meets_killer", 0.0),
}


def get_ghost_material():
    material = unreal.load_asset(MATERIAL_PATH)
    if material:
        if not isinstance(material, unreal.Material):
            raise RuntimeError(f"{MATERIAL_PATH} finns men är inte ett Material.")
        parameters = {str(x) for x in unreal.MaterialEditingLibrary.get_scalar_parameter_names(material)}
        vectors = {str(x) for x in unreal.MaterialEditingLibrary.get_vector_parameter_names(material)}
        if "GhostOpacity" not in parameters or "GhostColor" not in vectors:
            raise RuntimeError("Befintligt spökmaterial saknar GhostOpacity eller GhostColor. Kontrollera materialet manuellt.")
        return material
    unreal.EditorAssetLibrary.make_directory("/Game/TMOP/Killer")
    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        "M_TMOP_KillerGhost", "/Game/TMOP/Killer", unreal.Material, unreal.MaterialFactoryNew())
    if not material:
        raise RuntimeError("Kunde inte skapa spökmaterialet.")
    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    material.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_UNLIT)
    library = unreal.MaterialEditingLibrary
    library.set_material_usage(material, unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
    color = library.create_material_expression(material, unreal.MaterialExpressionVectorParameter, -320, -120)
    color.set_editor_property("parameter_name", "GhostColor")
    color.set_editor_property("default_value", unreal.LinearColor(0.2, 0.65, 1.0, 1.0))
    opacity = library.create_material_expression(material, unreal.MaterialExpressionScalarParameter, -320, 120)
    opacity.set_editor_property("parameter_name", "GhostOpacity")
    opacity.set_editor_property("default_value", 0.22)
    if not library.connect_material_property(color, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR):
        raise RuntimeError("Kunde inte koppla spökfärgen.")
    if not library.connect_material_property(opacity, "", unreal.MaterialProperty.MP_OPACITY):
        raise RuntimeError("Kunde inte koppla genomskinligheten.")
    library.recompile_material(material)
    if not unreal.EditorAssetLibrary.save_loaded_asset(material):
        raise RuntimeError("Kunde inte spara spökmaterialet.")
    return material


def install():
    corner_class = getattr(unreal, "TMOPKillerBranchPoint", None)
    director_class = getattr(unreal, "TMOPKillerBranchDirector", None)
    if not corner_class or not director_class:
        raise RuntimeError("Bygg TMOPEngine som Development Editor och starta om Unreal först.")
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = subsystem.get_all_level_actors()
    directors = [a for a in actors if isinstance(a, director_class)
                 and str(a.get_editor_property("network_id")) == NETWORK]
    if len(directors) > 1:
        raise RuntimeError("Flera Killer Branch Directors finns redan för KillerEscape; välj en innan scriptet körs.")
    anchors = {}
    corners = {}
    for actor in actors:
        if hasattr(actor, "get_anchor_id"):
            key = str(actor.get_anchor_id()).casefold()
            if key in {x.casefold() for x in MAIN_ANCHORS}:
                if key in anchors:
                    raise RuntimeError(f"Flera ankare med samma id: {key}")
                anchors[key] = actor
        if isinstance(actor, corner_class):
            for tag in actor.get_editor_property("tags"):
                text = str(tag)
                if text.startswith(TAG_PREFIX):
                    if str(actor.get_editor_property("network_id")) != NETWORK:
                        raise RuntimeError(f"Startpunktens tagg används i ett annat nät: {text}")
                    key = text[len(TAG_PREFIX):].casefold()
                    if key in corners:
                        raise RuntimeError(f"Flera startpunkter med samma tagg: {text}")
                    corners[key] = actor
    animation = unreal.load_asset(RUN_PATH)
    if not isinstance(animation, unreal.AnimSequence):
        raise RuntimeError(f"Löpanimation saknas: {RUN_PATH}. Justera RUN_PATH i scriptet.")
    material = get_ghost_material()
    created = []
    with unreal.ScopedEditorTransaction("TMOP Killer branch setup"):
        director = directors[0] if directors else subsystem.spawn_actor_from_class(director_class, unreal.Vector())
        if not director:
            raise RuntimeError("Kunde inte skapa Killer Branch Director.")
        if not directors:
            director.set_actor_label("TMOP_KillerBranchDirector")
            director.set_editor_property("network_id", NETWORK)
        if not director.get_editor_property("run_animation"):
            director.set_editor_property("run_animation", animation)
        if not director.get_editor_property("ghost_material"):
            director.set_editor_property("ghost_material", material)
        for anchor_id in MAIN_ANCHORS:
            key = anchor_id.casefold()
            if key in corners:
                continue
            anchor = anchors.get(key)
            if not anchor:
                unreal.log_warning(f"Killer-hörn saknar befintligt ankare: {anchor_id}. Ingen position gissas.")
                continue
            corner = subsystem.spawn_actor_from_class(corner_class, anchor.get_actor_location())
            if not corner:
                raise RuntimeError(f"Kunde inte skapa hörn vid {anchor_id}.")
            corner.set_actor_label(f"KillerCorner_{anchor_id}")
            corner.set_editor_property("network_id", NETWORK)
            corner.set_editor_property("branch_when_original_passes", True)
            corner.set_editor_property("tags", [unreal.Name(TAG_PREFIX + anchor_id)])
            if anchor_id in ARRIVAL_HINTS:
                event_id, offset = ARRIVAL_HINTS[anchor_id]
                corner.set_editor_property("arrival_event_id", event_id)
                corner.set_editor_property("arrival_event_offset_seconds", offset)
                # Hints from the current source timeline; actual passage remains the default.
            corners[key] = corner
            created.append(key)
        # Only initialize new corners. A rerun preserves all user-edited connections.
        for index, anchor_id in enumerate(MAIN_ANCHORS):
            key = anchor_id.casefold()
            if key not in created:
                continue
            corner = corners[key]
            previous = corners.get(MAIN_ANCHORS[index - 1].casefold()) if index > 0 else None
            following = corners.get(MAIN_ANCHORS[index + 1].casefold()) if index + 1 < len(MAIN_ANCHORS) else None
            if previous:
                corner.set_editor_property("original_previous_corner", previous)
            if following:
                corner.set_editor_property("original_next_corner", following)
                corner.set_editor_property("next_corners", [following])
    subsystem.set_selected_level_actors([director])
    unreal.log(f"Killer-system förberett. {len(created)} nya hörn. Lägg till alternativa hörn i Next Corners, kontrollera markhöjd och välj Save All.")
    unreal.log("Startnätet innehåller originalets riktning. Spöken skapas först när du har kopplat in alternativa vägar.")


if __name__ == "__main__":
    install()
