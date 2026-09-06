"""Add a third Sveavagen traffic lane in both directions.

The script is intentionally dry-run by default. Run it from Unreal Editor's
Python console while the traffic level is open:

exec(open(r"C:\\Users\\User\\Documents\\Unreal Projects\\TheMurderOfOlofPalme\\Scripts\\tmop_add_sveavagen_third_lanes.py", encoding="utf-8").read())

Read the Output Log, then set ``TMOP_SVEA_APPLY=True`` in the same Python
console command that executes the script. Save the level only after visually
checking the generated splines.
"""

import math
import re

import unreal


# Keep dry-run as the safe default, but allow Unreal's Python console to opt in
# without editing this file:
# TMOP_SVEA_APPLY=True; exec(open(r"...script.py", encoding="utf-8").read())
APPLY_CHANGES = bool(globals().get("TMOP_SVEA_APPLY", False))
RIGHT_OFFSET_FROM_R1_CM = 200.0
EXPECTED_ADJACENT_CONNECTOR_COUNT = 15
SVEA_ROAD_PATTERN = re.compile(r"^SVEAVAGEN([NS])_(\d{3})_R([123])$")


def prop(obj, name, default=None):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return default


def set_prop(obj, name, value):
    try:
        obj.set_editor_property(name, value)
        return True
    except Exception as exc:
        unreal.log_warning("TMOP R3: could not set {}.{}: {}".format(
            obj.get_name(), name, exc))
        return False


def lane_id(lane):
    value = prop(lane, "lane_id")
    text = str(value) if value is not None else ""
    return "" if text == "None" else text


def lane_components():
    lane_class = getattr(unreal, "TMOPTrafficLaneComponent", None)
    if lane_class is None:
        raise RuntimeError(
            "TMOPTrafficLaneComponent is unavailable. Build TMOPEngine and "
            "restart Unreal Editor before running this script."
        )
    result = []
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    for actor in actor_subsystem.get_all_level_actors():
        for lane in actor.get_components_by_class(lane_class):
            if lane_id(lane):
                result.append(lane)
    return result


def world_points(lane):
    return [
        lane.get_location_at_spline_point(
            index, unreal.SplineCoordinateSpace.WORLD
        )
        for index in range(lane.get_number_of_spline_points())
    ]


def normalized_xy(vector):
    length = math.hypot(float(vector.x), float(vector.y))
    if length < 0.001:
        return unreal.Vector(1.0, 0.0, 0.0)
    return unreal.Vector(float(vector.x) / length, float(vector.y) / length, 0.0)


def offset_right(points, distance_cm):
    """Offset a directed polyline to its right while retaining every Z value."""
    shifted = []
    for index, point in enumerate(points):
        if index == 0:
            tangent = points[1] - points[0]
        elif index == len(points) - 1:
            tangent = points[-1] - points[-2]
        else:
            tangent = points[index + 1] - points[index - 1]
        forward = normalized_xy(tangent)
        right = unreal.Vector(float(forward.y), -float(forward.x), 0.0)
        shifted.append(point + right * distance_cm)
    return shifted


def enum_value(enum_class_name, token, fallback=None):
    enum_class = getattr(unreal, enum_class_name, None)
    if enum_class is None:
        return fallback
    return getattr(enum_class, token.upper(), fallback)


def connection_target(connection):
    value = prop(connection, "target_lane_id")
    text = str(value) if value is not None else ""
    return "" if text == "None" else text


def make_connection(target_id, turn_token="STRAIGHT", allowed=True):
    connection_class = getattr(unreal, "TMOPLaneConnection", None)
    if connection_class is None:
        raise RuntimeError("TMOPLaneConnection is unavailable.")
    connection = connection_class()
    set_prop(connection, "target_lane_id", unreal.Name(target_id))
    turn = enum_value("TMOPTrafficTurnType", turn_token)
    if turn is not None:
        set_prop(connection, "turn_type", turn)
    set_prop(connection, "allowed", bool(allowed))
    return connection


def add_connection(source, target_id, turn_token="STRAIGHT", allowed=True):
    connections = list(prop(source, "next_lanes", []) or [])
    if any(connection_target(item) == target_id for item in connections):
        return False
    connections.append(make_connection(target_id, turn_token, allowed))
    source.modify()
    set_prop(source, "next_lanes", connections)
    return True


def set_spline_points(lane, points):
    lane.modify()
    lane.clear_spline_points(False)
    for point in points:
        lane.add_spline_point(point, unreal.SplineCoordinateSpace.WORLD, False)
    lane.set_closed_loop(False, False)
    lane.update_spline()
    lane.set_draw_debug(True)


def spawn_lane_actor(new_id, points, template=None, crossing=False):
    actor_class = getattr(unreal, "TMOPLaneSplineActor", None)
    if actor_class is None:
        raise RuntimeError(
            "TMOPLaneSplineActor is unavailable. Build the game module and "
            "restart Unreal Editor."
        )
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actor = actor_subsystem.spawn_actor_from_class(
        actor_class, unreal.Vector(0.0, 0.0, 0.0), unreal.Rotator(0.0, 0.0, 0.0)
    )
    if actor is None:
        raise RuntimeError("Could not spawn lane actor for {}".format(new_id))
    actor.modify()
    actor.set_actor_label(new_id)
    try:
        actor.set_folder_path(
            "TMOP Traffic Network/Sveavagen R3/{}".format(
                "Connectors" if crossing else "Road Lanes"
            )
        )
    except Exception:
        pass

    lane = prop(actor, "lane_spline")
    if lane is None:
        actor.destroy_actor()
        raise RuntimeError("Spawned actor has no LaneSpline component.")

    set_prop(actor, "lane_id", unreal.Name(new_id))
    set_prop(actor, "is_crossing", bool(crossing))
    set_prop(lane, "lane_id", unreal.Name(new_id))
    set_spline_points(lane, points)

    if template is not None:
        road_id = prop(template, "road_id", unreal.Name("SVEAVAGEN"))
        direction_id = prop(template, "direction_id", unreal.Name(""))
        set_prop(lane, "road_id", road_id)
        set_prop(lane, "direction_id", direction_id)
        set_prop(lane, "right_hand_traffic",
                 bool(prop(template, "right_hand_traffic", True)))
        set_prop(lane, "speed_limit_kmh",
                 float(prop(template, "speed_limit_kmh", 50.0)))
        lane_type = prop(template, "lane_type")
        if lane_type is not None:
            set_prop(lane, "lane_type", lane_type)
        set_prop(lane, "stop_line_id", prop(template, "stop_line_id", unreal.Name("")))
        set_prop(lane, "traffic_signal_group_id",
                 prop(template, "traffic_signal_group_id", unreal.Name("")))
    else:
        set_prop(lane, "road_id", unreal.Name("CROSSING"))
        set_prop(lane, "direction_id", unreal.Name("CROSSING"))

    set_prop(lane, "lane_index_from_right", 1)
    set_prop(lane, "lane_count_same_direction", 1 if crossing else 3)
    return actor, lane


def bezier_connector(source, target):
    start = source.get_location_at_spline_point(
        source.get_number_of_spline_points() - 1,
        unreal.SplineCoordinateSpace.WORLD,
    )
    end = target.get_location_at_spline_point(
        0, unreal.SplineCoordinateSpace.WORLD
    )
    source_direction = normalized_xy(source.get_direction_at_spline_point(
        source.get_number_of_spline_points() - 1,
        unreal.SplineCoordinateSpace.WORLD,
    ))
    target_direction = normalized_xy(target.get_direction_at_spline_point(
        0, unreal.SplineCoordinateSpace.WORLD
    ))
    gap = max(100.0, (end - start).length())
    handle = min(900.0, max(150.0, gap * 0.42))
    control_a = start + source_direction * handle
    control_b = end - target_direction * handle
    points = []
    for step in range(9):
        t = step / 8.0
        u = 1.0 - t
        points.append(
            start * (u ** 3)
            + control_a * (3.0 * u * u * t)
            + control_b * (3.0 * u * t * t)
            + end * (t ** 3)
        )
    return points


def connector_turn_token(connector_id):
    upper = connector_id.upper()
    if upper.endswith("_LEFT"):
        return "LEFT"
    if upper.endswith("_RIGHT"):
        return "RIGHT"
    if upper.endswith("_UTURN"):
        return "UTURN"
    return "STRAIGHT"


def connector_named_endpoints(connector_id):
    """Return the explicit FROM/TO lane IDs encoded in a connector ID.

    Do not infer these from every graph edge pointing at the connector: older
    repair passes may have left an unintended extra incoming edge, which must
    not be cloned into the new R3 network.
    """
    if not connector_id.startswith("X_"):
        return None
    body = connector_id[2:]
    for suffix in ("_STRAIGHT", "_LEFT", "_RIGHT", "_UTURN"):
        if not body.endswith(suffix):
            continue
        endpoints = body[:-len(suffix)]
        if "_TO_" not in endpoints:
            return None
        source_id, target_id = endpoints.split("_TO_", 1)
        return source_id, target_id
    return None


def r3_id(r1_id):
    if SVEA_ROAD_PATTERN.match(r1_id) and r1_id.endswith("_R1"):
        return r1_id[:-2] + "R3"
    return r1_id


def connector_id(source_id, target_id, turn_token):
    return "X_{}_TO_{}_{}".format(source_id, target_id, turn_token)


lanes = lane_components()
by_id = {lane_id(lane): lane for lane in lanes}
r1_lanes = {
    lane_id(lane): lane
    for lane in lanes
    if SVEA_ROAD_PATTERN.match(lane_id(lane))
    and lane_id(lane).endswith("_R1")
}

expected_r1 = [
    "SVEAVAGEN{}_{}{}_R1".format(direction, "0" * (3 - len(str(section))), section)
    for direction in ("N", "S")
    for section in range(1, 8)
]
missing_r1 = [item for item in expected_r1 if item not in r1_lanes]
if missing_r1:
    raise RuntimeError(
        "The open level is missing required Sveavagen R1 lanes: {}".format(
            ", ".join(missing_r1)
        )
    )

planned_road_ids = [r3_id(item) for item in expected_r1]
existing_r3 = [item for item in planned_road_ids if item in by_id]

adjacent_plans = []
for existing_connector_id, existing_connector in by_id.items():
    if not existing_connector_id.startswith("X_"):
        continue
    turn = connector_turn_token(existing_connector_id)
    if turn != "RIGHT":
        continue
    endpoints = connector_named_endpoints(existing_connector_id)
    if endpoints is None:
        continue
    source_id, target_id = endpoints
    if source_id not in by_id or target_id not in by_id:
        unreal.log_warning(
            "TMOP R3 skipping connector with missing named endpoint: {}".format(
                existing_connector_id
            )
        )
        continue
    source_is_svea_r1 = source_id in r1_lanes
    target_is_svea_r1 = target_id in r1_lanes
    if source_is_svea_r1 == target_is_svea_r1:
        continue
    adjacent_plans.append((r3_id(source_id), r3_id(target_id), turn))

adjacent_plans = sorted(set(adjacent_plans))
if len(adjacent_plans) != EXPECTED_ADJACENT_CONNECTOR_COUNT:
    raise RuntimeError(
        "Safety stop: expected exactly {} adjacent-street connectors, found {}. "
        "No changes were made.".format(
            EXPECTED_ADJACENT_CONNECTOR_COUNT, len(adjacent_plans)
        )
    )
straight_plans = []
for direction in ("N", "S"):
    for section in range(1, 7):
        source_id = "SVEAVAGEN{}_{:03d}_R3".format(direction, section)
        target_id = "SVEAVAGEN{}_{:03d}_R3".format(direction, section + 1)
        straight_plans.append((source_id, target_id, "STRAIGHT"))

unreal.log("TMOP R3 dry-run={} right_offset_from_R1_cm={:.1f}".format(
    not APPLY_CHANGES, RIGHT_OFFSET_FROM_R1_CM))
unreal.log("TMOP R3 will create {} road lanes ({} already exist).".format(
    len(planned_road_ids) - len(existing_r3), len(existing_r3)))
unreal.log("TMOP R3 will ensure {} straight and {} adjacent-street connectors.".format(
    len(straight_plans), len(adjacent_plans)))
for source_id, target_id, turn in adjacent_plans:
    unreal.log("TMOP R3 adjacent: {} -> {} ({})".format(
        source_id, target_id, turn))

if not APPLY_CHANGES:
    unreal.log_warning(
        "TMOP R3 DRY RUN ONLY. Re-run with TMOP_SVEA_APPLY=True after checking the log."
    )
else:
    if existing_r3:
        raise RuntimeError(
            "Safety stop: R3 lanes already exist ({}). Undo the previous R3 "
            "transaction or remove that generated attempt before applying v3."
            .format(", ".join(sorted(existing_r3)))
        )
    created_lanes = 0
    created_connectors = 0
    added_connections = 0
    with unreal.ScopedEditorTransaction("Add Sveavagen R3 lanes and connectors"):
        # Create all R3 road lanes first so every connector endpoint exists.
        for source_id in expected_r1:
            new_id = r3_id(source_id)
            if new_id in by_id:
                continue
            template = r1_lanes[source_id]
            points = offset_right(world_points(template), RIGHT_OFFSET_FROM_R1_CM)
            _, new_lane = spawn_lane_actor(new_id, points, template, False)
            by_id[new_id] = new_lane
            created_lanes += 1

        # Physical order from the right edge in the travel direction is now
        # R3 (new outer lane), R1 (old outer lane), R2 (old inner lane).
        for direction in ("N", "S"):
            for section in range(1, 8):
                stem = "SVEAVAGEN{}_{:03d}".format(direction, section)
                r1 = by_id.get(stem + "_R1")
                r2 = by_id.get(stem + "_R2")
                r3 = by_id.get(stem + "_R3")
                if not all((r1, r2, r3)):
                    unreal.log_warning("TMOP R3 neighbour group incomplete: " + stem)
                    continue
                for lane in (r1, r2, r3):
                    lane.modify()
                    set_prop(lane, "lane_count_same_direction", 3)
                set_prop(r1, "lane_index_from_right", 2)
                set_prop(r1, "right_neighbor_lane_id", unreal.Name(stem + "_R3"))
                set_prop(r1, "left_neighbor_lane_id", unreal.Name(stem + "_R2"))
                set_prop(r2, "lane_index_from_right", 3)
                set_prop(r2, "right_neighbor_lane_id", unreal.Name(stem + "_R1"))
                set_prop(r2, "left_neighbor_lane_id", unreal.Name(""))
                set_prop(r3, "lane_index_from_right", 1)
                set_prop(r3, "right_neighbor_lane_id", unreal.Name(""))
                set_prop(r3, "left_neighbor_lane_id", unreal.Name(stem + "_R1"))

        # Create both the through-road connectors and outer-lane right turns.
        for source_id, target_id, turn in straight_plans + adjacent_plans:
            source = by_id.get(source_id)
            target = by_id.get(target_id)
            if source is None or target is None:
                unreal.log_error("TMOP R3 missing connector endpoint: {} -> {}".format(
                    source_id, target_id))
                continue
            new_connector_id = connector_id(source_id, target_id, turn)
            connector = by_id.get(new_connector_id)
            if connector is None:
                points = bezier_connector(source, target)
                _, connector = spawn_lane_actor(
                    new_connector_id, points, None, True
                )
                by_id[new_connector_id] = connector
                created_connectors += 1
            if add_connection(source, new_connector_id, turn, True):
                added_connections += 1
            if add_connection(connector, target_id, "STRAIGHT", True):
                added_connections += 1

    try:
        level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        level_editor.editor_invalidate_viewports()
    except Exception:
        # The generated actors are still visible after selection or any normal
        # viewport redraw on engine builds without this Python method.
        pass
    unreal.log(
        "TMOP R3 complete: created {} road lanes, {} connectors and {} graph edges. "
        "Inspect every intersection, run lane validation, then save the level.".format(
            created_lanes, created_connectors, added_connections
        )
    )
