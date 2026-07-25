"""Export the currently loaded TMOP traffic lanes and anchors from Unreal.

Run from Unreal Editor's Python console:
exec(open(r"C:\\Users\\User\\Documents\\Unreal Projects\\TheMurderOfOlofPalme\\Scripts\\tmop_export_lanes_and_anchors_unreal.py", encoding="utf-8").read())
"""

import json
import os

import unreal


def name(value):
    text = str(value) if value is not None else ""
    return "None" if text in ("", "None") else text


def vector(value):
    return {
        "x": round(float(value.x), 3),
        "y": round(float(value.y), 3),
        "z": round(float(value.z), 3),
    }


def enum_name(value):
    text = str(value)
    return text.rsplit(".", 1)[-1]


def prop(obj, property_name, default=None):
    try:
        return obj.get_editor_property(property_name)
    except Exception:
        return default


def export_connection(connection):
    return {
        "target_lane_id": name(prop(connection, "target_lane_id")),
        "turn_type": enum_name(prop(connection, "turn_type", "Straight")),
        "allowed": bool(prop(connection, "allowed", True)),
    }


def export_lane(actor, lane):
    spline_points = []
    point_count = lane.get_number_of_spline_points()
    for index in range(point_count):
        spline_points.append(
            {
                "index": index,
                "location_cm": vector(
                    lane.get_location_at_spline_point(
                        index, unreal.SplineCoordinateSpace.WORLD
                    )
                ),
                "tangent_cm": vector(
                    lane.get_tangent_at_spline_point(
                        index, unreal.SplineCoordinateSpace.WORLD
                    )
                ),
            }
        )

    connections = [
        export_connection(connection)
        for connection in (prop(lane, "next_lanes", []) or [])
    ]
    return {
        "lane_id": name(prop(lane, "lane_id")),
        "road_id": name(prop(lane, "road_id")),
        "direction_id": name(prop(lane, "direction_id")),
        "actor_name": actor.get_name(),
        "actor_label": actor.get_actor_label(),
        "component_name": lane.get_name(),
        "lane_index_from_right": int(prop(lane, "lane_index_from_right", 1)),
        "lane_count_same_direction": int(
            prop(lane, "lane_count_same_direction", 1)
        ),
        "right_hand_traffic": bool(prop(lane, "right_hand_traffic", True)),
        "speed_limit_kmh": float(prop(lane, "speed_limit_kmh", 50.0)),
        "lane_type": enum_name(prop(lane, "lane_type", "General")),
        "left_neighbor_lane_id": name(prop(lane, "left_neighbor_lane_id")),
        "right_neighbor_lane_id": name(prop(lane, "right_neighbor_lane_id")),
        "stop_line_id": name(prop(lane, "stop_line_id")),
        "traffic_signal_group_id": name(
            prop(lane, "traffic_signal_group_id")
        ),
        "spline_length_cm": round(float(lane.get_spline_length()), 3),
        "spline_points": spline_points,
        "next_lanes": connections,
    }


def export_anchor(actor):
    class_name = actor.get_class().get_name()
    if "TMOPHistoricalAnchor" not in class_name:
        return None
    anchor_id = None
    try:
        anchor_id = actor.get_anchor_id()
    except Exception:
        anchor_id = prop(actor, "anchor_id")
    if name(anchor_id) == "None":
        return None
    transform = actor.get_actor_transform()
    rotation = transform.rotation.rotator()
    return {
        "anchor_id": name(anchor_id),
        "actor_name": actor.get_name(),
        "actor_label": actor.get_actor_label(),
        "location_cm": vector(transform.translation),
        "rotation_deg": {
            "pitch": round(float(rotation.pitch), 3),
            "yaw": round(float(rotation.yaw), 3),
            "roll": round(float(rotation.roll), 3),
        },
    }


actors = unreal.EditorLevelLibrary.get_all_level_actors()
lanes = []
anchors = []

lane_class = getattr(unreal, "TMOPTrafficLaneComponent", None)
if lane_class is None:
    raise RuntimeError(
        "TMOPTrafficLaneComponent is unavailable. Build TMOPEngine and restart "
        "Unreal Editor before running this export."
    )

for actor in actors:
    for lane in actor.get_components_by_class(lane_class):
        lane_data = export_lane(actor, lane)
        if lane_data["lane_id"] != "None":
            lanes.append(lane_data)
    anchor_data = export_anchor(actor)
    if anchor_data:
        anchors.append(anchor_data)

lanes.sort(key=lambda item: item["lane_id"].lower())
anchors.sort(key=lambda item: item["anchor_id"].lower())

lane_ids = {item["lane_id"] for item in lanes}
duplicate_lane_ids = sorted(
    {
        lane_id
        for lane_id in lane_ids
        if sum(item["lane_id"] == lane_id for item in lanes) > 1
    }
)
missing_targets = sorted(
    {
        connection["target_lane_id"]
        for lane in lanes
        for connection in lane["next_lanes"]
        if connection["allowed"]
        and connection["target_lane_id"] not in ("None", "")
        and connection["target_lane_id"] not in lane_ids
    }
)

payload = {
    "format": "TMOP_Lanes_Anchors_v1",
    "map_name": unreal.EditorLevelLibrary.get_editor_world().get_name(),
    "units": "Unreal centimeters",
    "lane_count": len(lanes),
    "anchor_count": len(anchors),
    "duplicate_lane_ids": duplicate_lane_ids,
    "missing_connected_lane_ids": missing_targets,
    "lanes": lanes,
    "anchors": anchors,
}

output_directory = os.path.join(
    unreal.Paths.project_saved_dir(), "TMOPExports"
)
os.makedirs(output_directory, exist_ok=True)
output_path = os.path.join(
    output_directory, "TMOP_Lanes_Anchors.json"
)

with open(output_path, "w", encoding="utf-8") as output_file:
    json.dump(payload, output_file, ensure_ascii=False, indent=2)

unreal.log(
    "TMOP export complete: {} lanes, {} anchors -> {}".format(
        len(lanes), len(anchors), output_path
    )
)
if duplicate_lane_ids:
    unreal.log_warning(
        "Duplicate Lane IDs: {}".format(", ".join(duplicate_lane_ids))
    )
if missing_targets:
    unreal.log_warning(
        "Missing connected Lane IDs: {}".format(", ".join(missing_targets))
    )
