r"""Export TMOP Historical Anchors and Cinema Seats from the open Unreal level.

Run in Unreal Editor's Python console:
    exec(open(r"C:\path\tmop_export_anchors_and_seats_unreal.py", encoding="utf-8").read())
"""

import json
import os
import unreal


def _value(obj, *names, default=None):
    for name in names:
        try:
            return obj.get_editor_property(name)
        except Exception:
            pass
        try:
            candidate = getattr(obj, name)
            return candidate() if callable(candidate) else candidate
        except Exception:
            pass
    return default


def _name(value):
    if value is None:
        return ""
    text = str(value)
    return "" if text.lower() == "none" else text


def _vector(vector):
    return {
        "x": float(vector.x),
        "y": float(vector.y),
        "z": float(vector.z),
    }


def _blender_vector(vector):
    # Unreal centimetres -> Blender metres; reflect Y to change handedness.
    return {
        "x": float(vector.x) / 100.0,
        "y": -float(vector.y) / 100.0,
        "z": float(vector.z) / 100.0,
    }


def _rotation(rotator):
    return {
        "pitch": float(rotator.pitch),
        "yaw": float(rotator.yaw),
        "roll": float(rotator.roll),
    }


def _transform_payload(transform):
    location = transform.translation
    rotation = transform.rotation.rotator()
    scale = transform.scale3d
    return {
        "unreal_location_cm": _vector(location),
        "blender_location_m": _blender_vector(location),
        "unreal_rotation_degrees": _rotation(rotation),
        "unreal_scale": _vector(scale),
    }


def _class_name(obj):
    try:
        return obj.get_class().get_name()
    except Exception:
        return type(obj).__name__


def _source_list(anchor):
    result = []
    sources = _value(anchor, "sources", default=[]) or []
    for source in sources:
        result.append({
            "source_id": _name(_value(source, "source_id", default="")),
            "reference": _name(_value(source, "reference", default="")),
            "notes": _name(_value(source, "notes", default="")),
        })
    return result


def export_tmop_anchors_and_seats():
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = actor_subsystem.get_all_level_actors()

    anchors = []
    seats = []
    warnings = []

    for actor in actors:
        actor_class = _class_name(actor)
        actor_class_lower = actor_class.lower()

        if "tmophistoricalanchor" in actor_class_lower:
            try:
                anchor_id = _name(actor.get_anchor_id())
            except Exception:
                anchor_id = _name(_value(actor, "anchor_id", default=""))
            if not anchor_id:
                # TMOPHistoricalAnchor normally inherits EntityIdentity.
                identity = _value(actor, "entity_identity", "entity_identity_component")
                anchor_id = _name(_value(identity, "entity_id", default=""))
            if not anchor_id:
                anchor_id = actor.get_actor_label()
                warnings.append("Anchor without readable AnchorId: " + anchor_id)

            item = {
                "anchor_id": anchor_id,
                "actor_label": actor.get_actor_label(),
                "actor_name": actor.get_name(),
                "actor_class": actor_class,
                "display_name": _name(_value(actor, "display_name", default="")),
                "category": _name(_value(actor, "anchor_category", default="")),
                "parent_place_id": _name(_value(actor, "parent_place_id", default="")),
                "parent_anchor_id": _name(_value(actor, "parent_anchor_id", default="")),
                "floor_level": int(_value(actor, "floor_level", default=0) or 0),
                "confidence": _name(_value(actor, "confidence", default="")),
                "notes": _name(_value(actor, "notes", default="")),
                "sources": _source_list(actor),
            }
            item.update(_transform_payload(actor.get_actor_transform()))
            anchors.append(item)

        try:
            components = actor.get_components_by_class(unreal.ActorComponent)
        except Exception:
            components = []

        for component in components:
            component_class = _class_name(component)
            if "tmopcinemaseatcomponent" not in component_class.lower():
                continue
            seat_id = _name(_value(component, "seat_id", default=""))
            if not seat_id:
                seat_id = component.get_name()
                warnings.append("Cinema seat without readable SeatId: " + seat_id)
            try:
                transform = component.get_component_transform()
            except Exception:
                transform = actor.get_actor_transform()
            item = {
                "seat_id": seat_id,
                "component_name": component.get_name(),
                "owner_label": actor.get_actor_label(),
                "owner_name": actor.get_name(),
                "venue_id": _name(_value(component, "venue_id", default="")),
                "auditorium_id": _name(_value(component, "auditorium_id", default="")),
                "row_number": int(_value(component, "row_number", default=-1) or -1),
                "seat_number": int(_value(component, "seat_number", default=-1) or -1),
            }
            item.update(_transform_payload(transform))
            seats.append(item)

    anchors.sort(key=lambda item: item["anchor_id"].lower())
    seats.sort(key=lambda item: item["seat_id"].lower())

    level_name = unreal.EditorLevelLibrary.get_editor_world().get_name()
    payload = {
        "format": "TMOP_ANCHORS_AND_SEATS_V1",
        "level": level_name,
        "coordinate_conversion": "Blender=(UnrealX,-UnrealY,UnrealZ)/100",
        "anchor_count": len(anchors),
        "seat_count": len(seats),
        "anchors": anchors,
        "cinema_seats": seats,
        "warnings": warnings,
    }

    output_directory = os.path.join(unreal.Paths.project_saved_dir(), "TMOP", "Exports")
    os.makedirs(output_directory, exist_ok=True)
    output_path = os.path.join(output_directory, "TMOP_AnchorsAndSeats.json")
    with open(output_path, "w", encoding="utf-8") as output_file:
        json.dump(payload, output_file, ensure_ascii=False, indent=2)

    unreal.log("TMOP export complete: {} anchors, {} seats".format(len(anchors), len(seats)))
    unreal.log("TMOP export file: " + os.path.abspath(output_path))
    if warnings:
        unreal.log_warning("TMOP export produced {} warning(s); see JSON.".format(len(warnings)))
    return os.path.abspath(output_path)


EXPORTED_TMOP_FILE = export_tmop_anchors_and_seats()
