"""Export and debug every TMOP bus system object in the loaded Unreal level.

Run in Unreal Editor's Python console:
exec(open(r"C:\\Users\\User\\Documents\\Unreal Projects\\TheMurderOfOlofPalme\\Scripts\\tmop_export_and_debug_buses_unreal.py", encoding="utf-8").read())

Output:
  <Project>/Saved/TMOPExports/TMOP_BusDebug.json

Set DRAW_WORLD_DEBUG to True to draw stops and bus routes for 120 seconds.
Green = valid stop/route segment, red = invalid/missing reference.
"""

import json
import os
from collections import Counter

import unreal


DRAW_WORLD_DEBUG = True
DEBUG_DURATION_SECONDS = 120.0
EXPORT_ALL_TRAFFIC_LANES = True


def prop(obj, key, default=None):
    try:
        return obj.get_editor_property(key)
    except Exception:
        return default


def text(value):
    if value is None:
        return "None"
    result = str(value)
    return "None" if result in ("", "None") else result


def enum_text(value):
    return text(value).rsplit(".", 1)[-1]


def asset_path(value):
    if value is None:
        return "None"
    try:
        return value.get_path_name()
    except Exception:
        return text(value)


def vec(value):
    return {
        "x": round(float(value.x), 3),
        "y": round(float(value.y), 3),
        "z": round(float(value.z), 3),
    }


def rot(value):
    return {
        "pitch": round(float(value.pitch), 3),
        "yaw": round(float(value.yaw), 3),
        "roll": round(float(value.roll), 3),
    }


def transform(value):
    rotation = value.rotation.rotator()
    return {
        "location_cm": vec(value.translation),
        "rotation_deg": rot(rotation),
        "scale": vec(value.scale3d),
    }


def tmop_time(value):
    return {
        "hour": int(prop(value, "hour", 0)),
        "minute": int(prop(value, "minute", 0)),
        "second": int(prop(value, "second", 0)),
    }


def component_transform(component):
    try:
        return transform(component.get_world_transform())
    except Exception:
        return {}


def export_lane(actor, lane):
    points = []
    try:
        count = lane.get_number_of_spline_points()
        for index in range(count):
            points.append(vec(lane.get_location_at_spline_point(
                index, unreal.SplineCoordinateSpace.WORLD)))
    except Exception:
        pass
    return {
        "lane_id": text(prop(lane, "lane_id")),
        "road_id": text(prop(lane, "road_id")),
        "direction_id": text(prop(lane, "direction_id")),
        "lane_type": enum_text(prop(lane, "lane_type")),
        "speed_limit_kmh": float(prop(lane, "speed_limit_kmh", 0.0)),
        "actor_label": actor.get_actor_label(),
        "component_name": lane.get_name(),
        "spline_length_cm": round(float(lane.get_spline_length()), 3),
        "spline_points": points,
        "next_lane_ids": [
            text(prop(item, "target_lane_id"))
            for item in (prop(lane, "next_lanes", []) or [])
            if bool(prop(item, "allowed", True))
        ],
    }


def export_stop(actor, stop):
    return {
        "stop_id": text(prop(stop, "stop_id")),
        "stop_name": text(prop(stop, "stop_name")),
        "lane_id": text(prop(stop, "lane_id")),
        "distance_along_lane_cm": float(
            prop(stop, "distance_along_lane", 0.0)),
        "served_route_ids": [
            text(item) for item in (prop(stop, "served_route_ids", []) or [])],
        "minimum_dwell_seconds": float(
            prop(stop, "minimum_dwell_seconds", 0.0)),
        "maximum_dwell_seconds": float(
            prop(stop, "maximum_dwell_seconds", 0.0)),
        "stop_buffer_cm": float(prop(stop, "stop_buffer_cm", 0.0)),
        "passenger_waiting_radius_cm": float(
            prop(stop, "passenger_waiting_radius_cm", 0.0)),
        "boarding_local_offset": vec(
            prop(stop, "boarding_local_offset", unreal.Vector())),
        "alighting_local_offset": vec(
            prop(stop, "alighting_local_offset", unreal.Vector())),
        "actor_label": actor.get_actor_label(),
        "actor_name": actor.get_name(),
        "component_name": stop.get_name(),
        "world_transform": component_transform(stop),
    }


def export_route(route):
    return {
        "asset_path": asset_path(route),
        "route_id": text(prop(route, "route_id")),
        "public_line_number": text(prop(route, "public_line_number")),
        "destination_display": text(prop(route, "destination_display")),
        "ordered_lane_ids": [
            text(item) for item in (prop(route, "ordered_lane_ids", []) or [])],
        "ordered_stop_ids": [
            text(item) for item in (prop(route, "ordered_stop_ids", []) or [])],
    }


def export_manifest(manifest):
    if manifest is None:
        return None
    journeys = []
    for item in prop(manifest, "journeys", []) or []:
        journeys.append({
            "passenger_entity_id": text(prop(item, "passenger_entity_id")),
            "boarding_stop_id": text(prop(item, "boarding_stop_id")),
            "alighting_stop_id": text(prop(item, "alighting_stop_id")),
            "placement": enum_text(prop(item, "placement")),
            "assigned_seat_id": text(prop(item, "assigned_seat_id")),
            "confidence": enum_text(prop(item, "confidence")),
            "source_reference": text(prop(item, "source_reference", "")),
            "notes": text(prop(item, "notes", "")),
        })
    return {
        "asset_path": asset_path(manifest),
        "manifest_id": text(prop(manifest, "manifest_id")),
        "driver_entity_id": text(prop(manifest, "driver_entity_id")),
        "journeys": journeys,
    }


def export_run(run):
    route = prop(run, "route_data")
    manifest = prop(run, "passenger_manifest")
    return {
        "run_id": text(prop(run, "run_id")),
        "route_asset": asset_path(route),
        "route_id": text(prop(route, "route_id")) if route else "None",
        "passenger_manifest_asset": asset_path(manifest),
        "bus_class": asset_path(prop(run, "bus_class")),
        "driver_entity_id": text(prop(run, "driver_entity_id")),
        "initial_lane_id": text(prop(run, "initial_lane_id")),
        "initial_distance_along_lane_cm": float(
            prop(run, "initial_distance_along_lane", 0.0)),
        "use_exact_start_time": bool(
            prop(run, "use_exact_start_time", True)),
        "exact_start_time": tmop_time(prop(run, "exact_start_time")),
        "earliest_start_time": tmop_time(prop(run, "earliest_start_time")),
        "latest_start_time": tmop_time(prop(run, "latest_start_time")),
        "speed_limit_multiplier": float(
            prop(run, "speed_limit_multiplier", 0.0)),
        "use_forced_despawn_time": bool(
            prop(run, "use_forced_despawn_time", False)),
        "forced_despawn_time": tmop_time(prop(run, "forced_despawn_time")),
        "despawn_when_route_completes": bool(
            prop(run, "despawn_when_traffic_route_completes", True)),
        "confidence": enum_text(prop(run, "confidence")),
        "source_reference": text(prop(run, "source_reference", "")),
    }


def export_director(actor):
    runs = [export_run(item) for item in prop(actor, "scheduled_runs", []) or []]
    return {
        "actor_label": actor.get_actor_label(),
        "actor_name": actor.get_name(),
        "class": actor.get_class().get_name(),
        "schedule_seed": int(prop(actor, "schedule_seed", 0)),
        "maximum_simultaneous_buses": int(
            prop(actor, "maximum_simultaneous_buses", 0)),
        "spawn_lead_seconds": int(prop(actor, "spawn_lead_seconds", 0)),
        "spawn_clearance_radius_cm": float(
            prop(actor, "spawn_clearance_radius_cm", 0.0)),
        "maximum_stationary_seconds": float(
            prop(actor, "maximum_stationary_seconds", 0.0)),
        "progress_distance_threshold_cm": float(
            prop(actor, "progress_distance_threshold_cm", 0.0)),
        "recovery_retry_interval_seconds": float(
            prop(actor, "recovery_retry_interval_seconds", 0.0)),
        "reset_when_time_moves_backwards": bool(
            prop(actor, "reset_when_time_moves_backwards", False)),
        "use_people_timelines_for_passengers": bool(
            prop(actor, "use_people_timelines_for_passengers", False)),
        "person_profile_table": asset_path(
            prop(actor, "person_profile_table")),
        "bus_run_configuration_table": asset_path(
            prop(actor, "bus_run_configuration_table")),
        "scheduled_runs": runs,
    }


def export_bus_actor(actor):
    result = {
        "actor_label": actor.get_actor_label(),
        "actor_name": actor.get_name(),
        "class": actor.get_class().get_name(),
        "transform": transform(actor.get_actor_transform()),
        "vehicle_id": text(prop(actor, "vehicle_id")),
        "display_name": text(prop(actor, "display_name")),
        "components": [],
    }
    for component in actor.get_components_by_class(unreal.ActorComponent):
        class_name = component.get_class().get_name()
        if any(token in class_name for token in (
                "Bus", "TrafficVehicleMovement", "Seat", "Door")):
            result["components"].append({
                "name": component.get_name(),
                "class": class_name,
                "route_data": asset_path(prop(component, "route_data")),
                "run_id": text(prop(component, "run_id")),
                "service_state": enum_text(prop(component, "service_state")),
                "current_stop_index": int(
                    prop(component, "current_stop_index", -1)),
                "planned_lane_ids": [text(item) for item in
                    (prop(component, "planned_lane_ids", []) or [])],
                "initial_lane_id": text(prop(component, "initial_lane_id")),
                "traffic_state": enum_text(prop(component, "traffic_state")),
            })
    return result


def discover_route_assets():
    route_class = getattr(unreal, "TMOPBusRouteData", None)
    manifest_class = getattr(unreal, "TMOPBusPassengerManifest", None)
    routes, manifests = [], []
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    for data in registry.get_assets_by_path("/Game", recursive=True):
        try:
            class_name = text(data.asset_class_path.asset_name)
        except Exception:
            class_name = text(prop(data, "asset_class"))
        if class_name not in ("TMOPBusRouteData", "TMOPBusPassengerManifest"):
            continue
        try:
            object_path = text(data.get_soft_object_path())
        except Exception:
            object_path = "{}.{}".format(data.package_name, data.asset_name)
        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        if asset is None:
            continue
        if route_class and isinstance(asset, route_class):
            routes.append(export_route(asset))
        elif manifest_class and isinstance(asset, manifest_class):
            manifests.append(export_manifest(asset))
    return routes, manifests


def validate(lanes, stops, routes, directors):
    errors, warnings = [], []
    norm = lambda value: value.casefold()
    lane_ids = {norm(item["lane_id"]) for item in lanes if item["lane_id"] != "None"}
    stop_ids = {norm(item["stop_id"]) for item in stops if item["stop_id"] != "None"}
    lane_by_id = {norm(item["lane_id"]): item for item in lanes}
    route_by_id = {norm(item["route_id"]): item for item in routes}

    for value, count in Counter(item["lane_id"] for item in lanes).items():
        if value != "None" and count > 1:
            errors.append("Duplicate lane ID: {} ({} components)".format(value, count))
    for value, count in Counter(item["stop_id"] for item in stops).items():
        if value != "None" and count > 1:
            errors.append("Duplicate bus stop ID: {} ({} components)".format(value, count))

    for stop in stops:
        if stop["stop_id"] == "None":
            errors.append("Bus stop component {} has no StopId".format(
                stop["component_name"]))
        if norm(stop["lane_id"]) not in lane_ids:
            errors.append("Stop {} references missing lane {}".format(
                stop["stop_id"], stop["lane_id"]))
        lane = lane_by_id.get(norm(stop["lane_id"]))
        if lane and stop["distance_along_lane_cm"] > lane["spline_length_cm"]:
            errors.append("Stop {} distance {:.0f} exceeds lane {} length {:.0f} cm".format(
                stop["stop_id"], stop["distance_along_lane_cm"],
                stop["lane_id"], lane["spline_length_cm"]))
        if stop["minimum_dwell_seconds"] > stop["maximum_dwell_seconds"]:
            errors.append("Stop {} minimum dwell exceeds maximum dwell".format(
                stop["stop_id"]))

    for route in routes:
        route_id = route["route_id"]
        if route_id == "None":
            errors.append("Route asset {} has no RouteId".format(route["asset_path"]))
        if not route["ordered_lane_ids"]:
            errors.append("Route {} contains no lanes".format(route_id))
        for lane_id in route["ordered_lane_ids"]:
            if norm(lane_id) not in lane_ids:
                errors.append("Route {} references missing lane {}".format(route_id, lane_id))
        for first, second in zip(route["ordered_lane_ids"], route["ordered_lane_ids"][1:]):
            lane = lane_by_id.get(norm(first))
            if lane and norm(second) not in {
                    norm(item) for item in lane["next_lane_ids"]}:
                warnings.append("Route {} lane jump is not connected: {} -> {}".format(
                    route_id, first, second))
        for stop_id in route["ordered_stop_ids"]:
            if norm(stop_id) not in stop_ids:
                errors.append("Route {} references missing stop {}".format(route_id, stop_id))

    run_ids = []
    for director in directors:
        for run in director["scheduled_runs"]:
            run_ids.append(run["run_id"])
            route = route_by_id.get(norm(run["route_id"]))
            if run["run_id"] == "None":
                errors.append("Scheduled bus run has no RunId")
            if route is None:
                errors.append("Run {} has no valid route asset/RouteId {}".format(
                    run["run_id"], run["route_id"]))
                continue
            start_lane = run["initial_lane_id"]
            if start_lane == "None" and route["ordered_lane_ids"]:
                start_lane = route["ordered_lane_ids"][0]
            if norm(start_lane) not in lane_ids:
                errors.append("Run {} starts on missing lane {}".format(
                    run["run_id"], start_lane))
            if run["driver_entity_id"] == "None":
                warnings.append("Run {} has no driver EntityId on its schedule row".format(
                    run["run_id"]))
            if run["bus_class"] == "None":
                errors.append("Run {} has no BusClass".format(run["run_id"]))
            if not run["despawn_when_route_completes"] and not run["use_forced_despawn_time"]:
                warnings.append("Run {} has no automatic or timed despawn".format(
                    run["run_id"]))
    for value, count in Counter(run_ids).items():
        if value != "None" and count > 1:
            errors.append("Duplicate scheduled RunId: {} ({} entries)".format(value, count))
    for value, count in Counter(norm(item["route_id"]) for item in routes).items():
        if value != "none" and count > 1:
            assets = [item["asset_path"] for item in routes
                      if norm(item["route_id"]) == value]
            errors.append("Duplicate RouteId {}: {}".format(value, ", ".join(assets)))
    return sorted(set(errors)), sorted(set(warnings))


def draw_debug(world, lanes, stops, routes, errors):
    if not DRAW_WORLD_DEBUG:
        return
    bad_text = "\n".join(errors)
    bad_lane_ids = {item["lane_id"] for item in lanes if item["lane_id"] in bad_text}
    route_lane_ids = {lane_id for route in routes for lane_id in route["ordered_lane_ids"]}
    try:
        for lane in lanes:
            if lane["lane_id"] not in route_lane_ids:
                continue
            color = unreal.LinearColor.RED if lane["lane_id"] in bad_lane_ids else unreal.LinearColor.GREEN
            points = lane["spline_points"]
            for start, end in zip(points, points[1:]):
                unreal.SystemLibrary.draw_debug_line(
                    world, unreal.Vector(**start), unreal.Vector(**end),
                    color, DEBUG_DURATION_SECONDS, 8.0)
        for stop in stops:
            location = stop["world_transform"].get("location_cm")
            if not location:
                continue
            color = unreal.LinearColor.RED if stop["stop_id"] in bad_text else unreal.LinearColor.YELLOW
            unreal.SystemLibrary.draw_debug_sphere(
                world, unreal.Vector(**location), 120.0, 16, color,
                DEBUG_DURATION_SECONDS, 10.0)
    except Exception as exc:
        unreal.log_warning("TMOP Bus Debug: world drawing unavailable: {}".format(exc))


def main():
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    lane_class = getattr(unreal, "TMOPTrafficLaneComponent", None)
    stop_class = getattr(unreal, "TMOPBusStopComponent", None)
    if lane_class is None or stop_class is None:
        raise RuntimeError(
            "TMOP bus/traffic classes are unavailable. Build TMOPEngine and restart Unreal Editor.")

    lanes, stops, directors, bus_actors = [], [], [], []
    for actor in actors:
        actor_lanes = actor.get_components_by_class(lane_class)
        for lane in actor_lanes:
            lanes.append(export_lane(actor, lane))
        for stop in actor.get_components_by_class(stop_class):
            stops.append(export_stop(actor, stop))
        if actor.get_class().get_name() == "TMOPBusScheduleDirector":
            directors.append(export_director(actor))
        component_names = [c.get_class().get_name() for c in
                           actor.get_components_by_class(unreal.ActorComponent)]
        if any("BusServiceComponent" in item or "BusPassengerComponent" in item
               for item in component_names):
            bus_actors.append(export_bus_actor(actor))

    routes, manifests = discover_route_assets()
    lanes.sort(key=lambda item: item["lane_id"].lower())
    stops.sort(key=lambda item: item["stop_id"].lower())
    routes.sort(key=lambda item: item["route_id"].lower())
    errors, warnings = validate(lanes, stops, routes, directors)
    draw_debug(world, lanes, stops, routes, errors)

    used_lane_ids = {lane_id for route in routes for lane_id in route["ordered_lane_ids"]}
    exported_lanes = lanes if EXPORT_ALL_TRAFFIC_LANES else [
        lane for lane in lanes if lane["lane_id"] in used_lane_ids]
    payload = {
        "format": "TMOP_BusDebug_v1",
        "map_name": world.get_name(),
        "units": "Unreal centimeters",
        "summary": {
            "schedule_directors": len(directors),
            "scheduled_runs": sum(len(item["scheduled_runs"]) for item in directors),
            "route_assets": len(routes),
            "bus_stops": len(stops),
            "traffic_lanes": len(lanes),
            "placed_bus_actors": len(bus_actors),
            "manifest_assets": len(manifests),
            "errors": len(errors),
            "warnings": len(warnings),
        },
        "errors": errors,
        "warnings": warnings,
        "schedule_directors": directors,
        "route_assets": routes,
        "passenger_manifests": manifests,
        "bus_stops": stops,
        "traffic_lanes": exported_lanes,
        "placed_bus_actors": bus_actors,
    }
    output_directory = os.path.join(unreal.Paths.project_saved_dir(), "TMOPExports")
    os.makedirs(output_directory, exist_ok=True)
    output_path = os.path.join(output_directory, "TMOP_BusDebug.json")
    with open(output_path, "w", encoding="utf-8") as output_file:
        json.dump(payload, output_file, ensure_ascii=False, indent=2)

    unreal.log("TMOP BUS DEBUG: {} run(s), {} route(s), {} stop(s), {} error(s), {} warning(s).".format(
        payload["summary"]["scheduled_runs"], len(routes), len(stops),
        len(errors), len(warnings)))
    for item in errors:
        unreal.log_error("TMOP BUS ERROR: " + item)
    for item in warnings:
        unreal.log_warning("TMOP BUS WARNING: " + item)
    unreal.log("TMOP BUS DEBUG EXPORT: " + output_path)


main()
