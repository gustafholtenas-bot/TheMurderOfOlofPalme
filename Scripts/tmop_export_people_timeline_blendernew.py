"""Export compact TMOP person/timeline data from Blender without baking frames.

Run from Blender's Scripting workspace. The JSON is written beside the .blend.
No mesh geometry, images, materials, or per-frame FBX data are exported.
"""

import bpy
import json
import math
import os
from mathutils import Matrix


OUTPUT_PATH = bpy.path.abspath("//TMOP_PeopleTimelineExport.json")
MAX_EVALUATED_FRAMES_PER_OBJECT = 5000


def json_value(value):
    if value is None or isinstance(value, (bool, int, float, str)):
        return value
    if hasattr(value, "to_list"):
        return value.to_list()
    if isinstance(value, (list, tuple)):
        return [json_value(item) for item in value]
    try:
        return value.to_dict()
    except Exception:
        return str(value)


def custom_properties(id_block):
    result = {}
    for key in id_block.keys():
        if key == "_RNA_UI":
            continue
        try:
            result[key] = json_value(id_block[key])
        except Exception as exc:
            result[key] = "<unreadable: %s>" % exc
    return result


def collection_paths_for_object(obj):
    return sorted(collection.name for collection in obj.users_collection)


def is_relevant_collection_name(name):
    lower = name.casefold()
    tokens = (
        "person", "people", "agent", "vittne", "witness",
        "tmop_anchor", "tmop_cinemaseat", "cinemaseat",
    )
    return any(token in lower for token in tokens)


def is_relevant_object(obj):
    name = obj.name.casefold()
    if name.startswith("anchor__") or name.startswith("seat__"):
        return True
    if obj.animation_data is not None:
        action = obj.animation_data.action
        if action is not None or len(obj.animation_data.nla_tracks) > 0:
            return True
    return any(is_relevant_collection_name(c.name) for c in obj.users_collection)


def keyframe_point_data(point):
    return {
        "co": [float(point.co[0]), float(point.co[1])],
        "handle_left": [float(point.handle_left[0]), float(point.handle_left[1])],
        "handle_right": [float(point.handle_right[0]), float(point.handle_right[1])],
        "interpolation": point.interpolation,
        "easing": point.easing,
    }


def iter_action_fcurves(action, slot=None):
    """Yield F-curves from both legacy and Blender 4.4+ layered actions."""
    if action is None:
        return

    legacy_curves = getattr(action, "fcurves", None)
    if legacy_curves is not None:
        for curve in legacy_curves:
            yield curve
        return

    seen = set()
    for layer in getattr(action, "layers", []):
        for strip in getattr(layer, "strips", []):
            channelbags = []

            # Prefer the channel bag belonging to this animated object's slot.
            if slot is not None and hasattr(strip, "channelbag"):
                try:
                    bag = strip.channelbag(slot)
                    if bag is not None:
                        channelbags.append(bag)
                except (TypeError, RuntimeError):
                    pass

            if not channelbags:
                channelbags.extend(list(getattr(strip, "channelbags", [])))

            for bag in channelbags:
                for curve in getattr(bag, "fcurves", []):
                    pointer = curve.as_pointer()
                    if pointer not in seen:
                        seen.add(pointer)
                        yield curve


def action_data(action, slot=None):
    if action is None:
        return None
    curves = []
    for curve in iter_action_fcurves(action, slot):
        curves.append({
            "data_path": curve.data_path,
            "array_index": int(curve.array_index),
            "extrapolation": curve.extrapolation,
            "keyframes": [keyframe_point_data(p) for p in curve.keyframe_points],
        })
    return {
        "name": action.name,
        "frame_range": [float(action.frame_range[0]), float(action.frame_range[1])],
        "custom_properties": custom_properties(action),
        "fcurves": curves,
    }


def animation_data(obj):
    anim = obj.animation_data
    if anim is None:
        return None
    nla_tracks = []
    for track in anim.nla_tracks:
        strips = []
        for strip in track.strips:
            strips.append({
                "name": strip.name,
                "type": strip.type,
                "frame_start": float(strip.frame_start),
                "frame_end": float(strip.frame_end),
                "action": strip.action.name if strip.action else None,
                "action_frame_start": float(strip.action_frame_start),
                "action_frame_end": float(strip.action_frame_end),
                "scale": float(strip.scale),
                "repeat": float(strip.repeat),
                "blend_type": strip.blend_type,
                "influence": float(strip.influence),
            })
        nla_tracks.append({
            "name": track.name,
            "mute": bool(track.mute),
            "is_solo": bool(track.is_solo),
            "strips": strips,
        })
    return {
        "action": action_data(
            anim.action,
            getattr(anim, "action_slot", None)),
        "nla_tracks": nla_tracks,
        "use_nla": bool(anim.use_nla),
    }


def collect_action_frames(action, output, slot=None):
    if action is None:
        return
    for curve in iter_action_fcurves(action, slot):
        for point in curve.keyframe_points:
            output.add(float(point.co[0]))


def collect_keyed_frames(obj):
    frames = set()
    chain = []
    current = obj
    while current is not None:
        chain.append(current)
        current = current.parent
    for item in chain:
        anim = item.animation_data
        if anim is None:
            continue
        collect_action_frames(
            anim.action,
            frames,
            getattr(anim, "action_slot", None))
        for track in anim.nla_tracks:
            for strip in track.strips:
                frames.add(float(strip.frame_start))
                frames.add(float(strip.frame_end))
                if strip.action is not None:
                    action_start = float(strip.action_frame_start)
                    action_end = float(strip.action_frame_end)
                    source_span = action_end - action_start
                    target_span = float(strip.frame_end - strip.frame_start)
                    if source_span > 0.0 and target_span > 0.0:
                        for curve in iter_action_fcurves(
                            strip.action,
                            getattr(strip, "action_slot", None)):
                            for point in curve.keyframe_points:
                                source_frame = float(point.co[0])
                                if action_start <= source_frame <= action_end:
                                    normalized = (source_frame - action_start) / source_span
                                    frames.add(float(strip.frame_start) + normalized * target_span)
    frames.add(float(bpy.context.scene.frame_start))
    frames.add(float(bpy.context.scene.frame_end))
    return sorted(frames)


def transform_from_matrix(matrix: Matrix):
    location, rotation, scale = matrix.decompose()
    euler = rotation.to_euler("XYZ")
    return {
        "location_m": [float(v) for v in location],
        "rotation_euler_degrees": [float(math.degrees(v)) for v in euler],
        "rotation_quaternion": [float(v) for v in rotation],
        "scale": [float(v) for v in scale],
        "matrix_world": [[float(v) for v in row] for row in matrix],
    }


def evaluated_transforms(obj, frames, scene):
    if len(frames) > MAX_EVALUATED_FRAMES_PER_OBJECT:
        step = max(1, math.ceil(len(frames) / MAX_EVALUATED_FRAMES_PER_OBJECT))
        selected = frames[::step]
        if frames[-1] not in selected:
            selected.append(frames[-1])
    else:
        selected = frames
    samples = []
    for frame in selected:
        base = math.floor(frame)
        subframe = frame - base
        scene.frame_set(int(base), subframe=float(subframe))
        bpy.context.view_layer.update()
        evaluated = obj.evaluated_get(bpy.context.evaluated_depsgraph_get())
        samples.append({
            "frame": float(frame),
            "time_seconds_from_scene_start":
                (float(frame) - float(scene.frame_start)) / float(scene.render.fps / scene.render.fps_base),
            "world_transform": transform_from_matrix(evaluated.matrix_world.copy()),
        })
    return samples


def object_record(obj, scene):
    keyed_frames = collect_keyed_frames(obj)
    return {
        "name": obj.name,
        "type": obj.type,
        "data_name": obj.data.name if obj.data else None,
        "parent": obj.parent.name if obj.parent else None,
        "parent_type": obj.parent_type,
        "collections": collection_paths_for_object(obj),
        "custom_properties": custom_properties(obj),
        "hide_viewport": bool(obj.hide_viewport),
        "hide_render": bool(obj.hide_render),
        "base_world_transform": transform_from_matrix(obj.matrix_world.copy()),
        "animation": animation_data(obj),
        "keyed_frames_including_parents": keyed_frames,
        "evaluated_world_transforms_at_keyframes":
            evaluated_transforms(obj, keyed_frames, scene),
    }


def main():
    scene = bpy.context.scene
    original_frame = scene.frame_current
    original_subframe = scene.frame_subframe
    fps = float(scene.render.fps / scene.render.fps_base)
    relevant = sorted(
        (obj for obj in scene.objects if is_relevant_object(obj)),
        key=lambda obj: obj.name.casefold(),
    )
    try:
        records = []
        total = len(relevant)
        for index, obj in enumerate(relevant, 1):
            print("TMOP export %d/%d: %s" % (index, total, obj.name))
            records.append(object_record(obj, scene))
        payload = {
            "format": "TMOP Blender People Timeline Export",
            "version": 1,
            "source_blend": bpy.data.filepath,
            "scene": scene.name,
            "frame_start": int(scene.frame_start),
            "frame_end": int(scene.frame_end),
            "fps": fps,
            "duration_seconds":
                (float(scene.frame_end) - float(scene.frame_start)) / fps,
            "timeline_markers": [
                {"name": marker.name, "frame": int(marker.frame),
                 "time_seconds_from_scene_start":
                    (float(marker.frame) - float(scene.frame_start)) / fps}
                for marker in sorted(scene.timeline_markers, key=lambda item: item.frame)
            ],
            "collections": [
                {"name": collection.name,
                 "custom_properties": custom_properties(collection)}
                for collection in sorted(bpy.data.collections, key=lambda item: item.name.casefold())
            ],
            "objects": records,
        }
        os.makedirs(os.path.dirname(OUTPUT_PATH), exist_ok=True)
        with open(OUTPUT_PATH, "w", encoding="utf-8") as handle:
            json.dump(payload, handle, ensure_ascii=False, indent=2)
        print("TMOP compact timeline export complete: %d objects" % len(records))
        print("TMOP export file: %s" % OUTPUT_PATH)
    finally:
        scene.frame_set(int(original_frame), subframe=float(original_subframe))
        bpy.context.view_layer.update()


if __name__ == "__main__":
    main()
