"""Run in Unreal's Python console after loading all relevant World Partition cells.
Read-only: exports placed signal meshes, slots, controllers, stops and crossing volumes.
"""
import json
from pathlib import Path
import unreal

def prop(obj, name, fallback=None):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return fallback

def vec(v):
    return {"x": v.x, "y": v.y, "z": v.z}

def export_signals():
    editor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    rows = []
    controllers = []
    stops = []
    crossings = []
    for actor in editor.get_all_level_actors():
        actor_label = actor.get_actor_label()
        for mesh in actor.get_components_by_class(unreal.StaticMeshComponent):
            asset = prop(mesh, "static_mesh")
            if not asset:
                continue
            slots = [str(n) for n in mesh.get_material_slot_names()]
            description = (actor_label + " " + asset.get_path_name()).lower()
            if not any(t in description for t in ("trafikljus", "trafficlight", "traffic_light", "signal")) and not any(
                s.startswith(("Vehicle_Red", "Pedestrian_Red")) for s in slots
            ):
                continue
            rows.append({
                "actor": actor.get_path_name(), "label": actor_label,
                "component": mesh.get_name(), "mesh": asset.get_path_name(),
                "location_cm": vec(mesh.get_world_location()),
                "rotation": str(mesh.get_world_rotation()),
                "slots": [{"index": i, "name": str(name),
                           "material": mesh.get_material(i).get_path_name() if mesh.get_material(i) else None}
                          for i, name in enumerate(slots)]
            })
        controller_cls = getattr(unreal, "TMOPTrafficSignalController", None)
        if controller_cls and isinstance(actor, controller_cls):
            controllers.append({
                "actor": actor.get_path_name(), "intersection_id": str(prop(actor, "intersection_id")),
                "groups": str(prop(actor, "groups")), "phases": str(prop(actor, "phases")),
                "offset": prop(actor, "cycle_offset_seconds")
            })
        for cls_name, fields, target in (
            ("TMOPTrafficStopLineComponent", ("intersection_id", "stop_line_id", "lane_id", "signal_group_id", "distance_along_lane", "stop_buffer_cm"), stops),
            ("TMOPPedestrianCrossingComponent", ("crossing_id", "intersection_id", "signal_group_id", "yielding_vehicle_groups"), crossings),
        ):
            cls = getattr(unreal, cls_name, None)
            if not cls:
                continue
            for component in actor.get_components_by_class(cls):
                row = {"actor": actor.get_path_name(), "component": component.get_name(),
                       "location_cm": vec(component.get_world_location()),
                       "rotation": str(component.get_world_rotation())}
                for field in fields:
                    value = prop(component, field)
                    row[field] = value if isinstance(value, (str, float, int, bool)) else str(value)
                if cls_name == "TMOPPedestrianCrossingComponent":
                    row["extent_cm"] = vec(component.get_scaled_box_extent())
                target.append(row)
    data = {"format": "TMOP_Signals_Inventory_v2", "loaded_actors_only": True,
            "meshes": rows, "controllers": controllers, "stop_lines": stops, "crossings": crossings}
    output = Path(unreal.Paths.project_saved_dir()) / "TMOP_Signals_Inventory.json"
    output.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    unreal.log("TMOP signal inventory: " + str(output))
    return str(output)

if __name__ == "__main__":
    export_signals()
