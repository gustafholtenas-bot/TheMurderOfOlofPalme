"""Snap all existing Sveavagen R3 connectors to manually moved R3 lanes.

This script does not move, create or delete road lanes. It rebuilds the spline
geometry of exactly 27 existing connectors whose IDs contain ``_R3``. Source
and destination are read from each connector's explicit X_FROM_TO_TURN ID.

Dry-run:
exec(open(r"...tmop_SVEAVAGEN_SNAP_27_R3_CONNECTORS_TO_MANUAL_LANES_SAFE_v8.py", encoding="utf-8").read())

Apply:
TMOP_SVEA_CONNECTOR_APPLY=True; exec(open(r"...tmop_SVEAVAGEN_SNAP_27_R3_CONNECTORS_TO_MANUAL_LANES_SAFE_v8.py", encoding="utf-8").read())
"""

import math
import re

import unreal


APPLY_CHANGES = bool(globals().get("TMOP_SVEA_CONNECTOR_APPLY", False))
EXPECTED_R3_ROAD_LANES = 14
EXPECTED_R3_CONNECTORS = 27
R3_ROAD_PATTERN = re.compile(r"^SVEAVAGEN[NS]_\d{3}_R3$")


def prop(obj, name, default=None):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return default


def lane_id(lane):
    value = prop(lane, "lane_id")
    text = str(value) if value is not None else ""
    return "" if text == "None" else text


def all_lanes():
    lane_class = getattr(unreal, "TMOPTrafficLaneComponent", None)
    if lane_class is None:
        raise RuntimeError("TMOPTrafficLaneComponent is unavailable.")
    actor_subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    result = []
    for actor in actor_subsystem.get_all_level_actors():
        for lane in actor.get_components_by_class(lane_class):
            if lane_id(lane):
                result.append(lane)
    return result


def connector_endpoints(connector_id):
    if not connector_id.startswith("X_"):
        return None
    body = connector_id[2:]
    for suffix in ("_STRAIGHT", "_LEFT", "_RIGHT", "_UTURN"):
        if not body.endswith(suffix):
            continue
        body = body[:-len(suffix)]
        if "_TO_" not in body:
            return None
        source_id, destination_id = body.split("_TO_", 1)
        return source_id, destination_id, suffix[1:]
    return None


def normalized_xy(vector):
    length = math.hypot(float(vector.x), float(vector.y))
    if length < 0.001:
        return unreal.Vector(1.0, 0.0, 0.0)
    return unreal.Vector(float(vector.x) / length, float(vector.y) / length, 0.0)


def connector_curve(source, destination, turn):
    start_index = source.get_number_of_spline_points() - 1
    start = source.get_location_at_spline_point(
        start_index, unreal.SplineCoordinateSpace.WORLD
    )
    end = destination.get_location_at_spline_point(
        0, unreal.SplineCoordinateSpace.WORLD
    )
    source_direction = normalized_xy(source.get_direction_at_spline_point(
        start_index, unreal.SplineCoordinateSpace.WORLD
    ))
    destination_direction = normalized_xy(
        destination.get_direction_at_spline_point(
            0, unreal.SplineCoordinateSpace.WORLD
        )
    )
    distance = max(100.0, (end - start).length())
    if turn == "STRAIGHT":
        handle = min(700.0, max(100.0, distance * 0.34))
        samples = 7
    else:
        handle = min(900.0, max(150.0, distance * 0.42))
        samples = 9
    control_a = start + source_direction * handle
    control_b = end - destination_direction * handle
    points = []
    for step in range(samples):
        t = step / float(samples - 1)
        u = 1.0 - t
        points.append(
            start * (u ** 3)
            + control_a * (3.0 * u * u * t)
            + control_b * (3.0 * u * t * t)
            + end * (t ** 3)
        )
    return points


def set_spline_points(lane, points):
    lane.modify()
    lane.clear_spline_points(False)
    for point in points:
        lane.add_spline_point(point, unreal.SplineCoordinateSpace.WORLD, False)
    lane.set_closed_loop(False, False)
    lane.update_spline()
    lane.set_draw_debug(True)


def endpoint_gap_cm(source, connector, destination):
    source_end = source.get_location_at_spline_point(
        source.get_number_of_spline_points() - 1,
        unreal.SplineCoordinateSpace.WORLD,
    )
    connector_start = connector.get_location_at_spline_point(
        0, unreal.SplineCoordinateSpace.WORLD
    )
    connector_end = connector.get_location_at_spline_point(
        connector.get_number_of_spline_points() - 1,
        unreal.SplineCoordinateSpace.WORLD,
    )
    destination_start = destination.get_location_at_spline_point(
        0, unreal.SplineCoordinateSpace.WORLD
    )
    return (connector_start - source_end).length(), \
        (destination_start - connector_end).length()


lanes = all_lanes()
by_id = {lane_id(lane): lane for lane in lanes}
r3_roads = sorted(item for item in by_id if R3_ROAD_PATTERN.match(item))
r3_connectors = sorted(
    item for item in by_id if item.startswith("X_") and "_R3" in item
)

if len(r3_roads) != EXPECTED_R3_ROAD_LANES:
    raise RuntimeError(
        "Safety stop: expected {} R3 road lanes, found {}. No changes were made."
        .format(EXPECTED_R3_ROAD_LANES, len(r3_roads))
    )
if len(r3_connectors) != EXPECTED_R3_CONNECTORS:
    raise RuntimeError(
        "Safety stop: expected {} R3 connectors, found {}. No changes were made."
        .format(EXPECTED_R3_CONNECTORS, len(r3_connectors))
    )

plans = []
for connector_id in r3_connectors:
    endpoints = connector_endpoints(connector_id)
    if endpoints is None:
        raise RuntimeError(
            "Safety stop: cannot parse connector ID {}. No changes were made."
            .format(connector_id)
        )
    source_id, destination_id, turn = endpoints
    if source_id not in by_id or destination_id not in by_id:
        raise RuntimeError(
            "Safety stop: connector {} has a missing named endpoint. No "
            "changes were made.".format(connector_id)
        )
    connector = by_id[connector_id]
    source = by_id[source_id]
    destination = by_id[destination_id]
    start_gap, end_gap = endpoint_gap_cm(source, connector, destination)
    plans.append((connector_id, source, connector, destination, turn,
                  start_gap, end_gap))

bad_before = [plan for plan in plans if max(plan[5], plan[6]) > 25.0]
unreal.log(
    "TMOP R3 connector v8 dry-run={}: 14 R3 lanes, 27 connectors; {} "
    "connectors currently exceed a 25 cm endpoint gap."
    .format(not APPLY_CHANGES, len(bad_before))
)
for plan in plans:
    if max(plan[5], plan[6]) > 25.0:
        unreal.log(
            "TMOP R3 connector gap: {} start={:.1f} cm end={:.1f} cm"
            .format(plan[0], plan[5], plan[6])
        )

if not APPLY_CHANGES:
    unreal.log_warning(
        "TMOP R3 connector v8 DRY RUN ONLY. Re-run with "
        "TMOP_SVEA_CONNECTOR_APPLY=True after checking the log."
    )
else:
    with unreal.ScopedEditorTransaction(
        "Snap 27 Sveavagen R3 connectors to manual lanes"
    ):
        for _, source, connector, destination, turn, _, _ in plans:
            set_spline_points(
                connector, connector_curve(source, destination, turn)
            )

    remaining = []
    for connector_id, source, connector, destination, _, _, _ in plans:
        start_gap, end_gap = endpoint_gap_cm(source, connector, destination)
        if max(start_gap, end_gap) > 1.0:
            remaining.append((connector_id, start_gap, end_gap))
    if remaining:
        raise RuntimeError(
            "Connector rebuild completed, but {} connectors still exceed a "
            "1 cm gap. Undo the transaction and inspect the Output Log."
            .format(len(remaining))
        )
    try:
        level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        level_editor.editor_invalidate_viewports()
    except Exception:
        pass
    unreal.log(
        "TMOP R3 connector v8 complete: rebuilt 27 connector curves; all "
        "source and destination endpoint gaps are <= 1 cm. Inspect visually "
        "before saving the level."
    )
