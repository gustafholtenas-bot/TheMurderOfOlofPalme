"""Fix confirmed bus route/stop/lane errors from TMOP_BusDebug 2026-08-23.

Run once in Unreal Editor's Python console, with the main level loaded:
exec(open(r"C:\\Users\\User\\Documents\\Unreal Projects\\TheMurderOfOlofPalme\\Scripts\\tmop_fix_bus_level_from_debug_086.py", encoding="utf-8").read())
"""

import unreal


ROUTE_46_N_PATH = "/Game/TMOP/Vehicles/Bus/BUSS46_N.BUSS46_N"
LANE_CONNECTIONS = {
    "BIRGERJARLSGATAN_007_R1":
        "X_BIRGERJARLSGATAN_007_R1_TO_BIRGERJARLSGATAN_008_R1_STRAIGHT",
    "BIRGERJARLSGATAS_001_R1":
        "X_BIRGERJARLSGATAS_001_R1_TO_BIRGERJARLSGATAS_002_R1_STRAIGHT",
}
STOP_TO_PROJECT = "buss4142_W3"


def prop(obj, key, default=None):
    try:
        return obj.get_editor_property(key)
    except Exception:
        return default


def n(value):
    return str(value).casefold()


def fix_route_46_n():
    route = unreal.EditorAssetLibrary.load_asset(ROUTE_46_N_PATH)
    if route is None:
        raise RuntimeError("Missing route asset " + ROUTE_46_N_PATH)
    route.modify()
    route.set_editor_property("route_id", "BUSS46_N")
    route.set_editor_property("public_line_number", "46")
    unreal.EditorAssetLibrary.save_loaded_asset(route, only_if_is_dirty=False)
    unreal.log("TMOP BUS FIX: BUSS46_N now has RouteId BUSS46_N and line number 46.")


def collect_level_components():
    lane_class = getattr(unreal, "TMOPTrafficLaneComponent", None)
    stop_class = getattr(unreal, "TMOPBusStopComponent", None)
    if lane_class is None or stop_class is None:
        raise RuntimeError("Build TMOPEngine and restart Unreal Editor first.")
    lanes, stops = {}, {}
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        for lane in actor.get_components_by_class(lane_class):
            lanes[n(prop(lane, "lane_id"))] = lane
        for stop in actor.get_components_by_class(stop_class):
            stops[n(prop(stop, "stop_id"))] = stop
    return lanes, stops


def fix_lane_connections(lanes):
    connection_class = getattr(unreal, "TMOPLaneConnection", None)
    if connection_class is None:
        raise RuntimeError("TMOPLaneConnection is unavailable after build.")
    for source_id, target_id in LANE_CONNECTIONS.items():
        lane = lanes.get(n(source_id))
        if lane is None or n(target_id) not in lanes:
            raise RuntimeError("Missing lane {} or target {}".format(source_id, target_id))
        connections = list(prop(lane, "next_lanes", []) or [])
        if not any(n(prop(item, "target_lane_id")) == n(target_id)
                   for item in connections):
            item = connection_class()
            item.set_editor_property("target_lane_id", target_id)
            item.set_editor_property("turn_type", unreal.TMOPTrafficTurnType.STRAIGHT)
            item.set_editor_property("allowed", True)
            connections.append(item)
            lane.modify()
            lane.set_editor_property("next_lanes", connections)
        unreal.log("TMOP BUS FIX: connected {} -> {}".format(source_id, target_id))


def fix_stop_distance(lanes, stops):
    stop = stops.get(n(STOP_TO_PROJECT))
    if stop is None:
        raise RuntimeError("Missing stop " + STOP_TO_PROJECT)
    lane_id = str(prop(stop, "lane_id"))
    lane = lanes.get(n(lane_id))
    if lane is None:
        raise RuntimeError("Stop {} references missing lane {}".format(
            STOP_TO_PROJECT, lane_id))
    stop_location = stop.get_world_location()
    input_key = lane.find_input_key_closest_to_world_location(stop_location)
    try:
        distance = lane.get_distance_along_spline_at_spline_input_key(input_key)
    except Exception:
        # UE builds without the input-key distance wrapper: the confirmed
        # closest distance from the exported level is 8038.8 cm.
        distance = 8038.8
    distance = max(0.0, min(float(distance), float(lane.get_spline_length())))
    stop.modify()
    stop.set_editor_property("distance_along_lane", distance)
    unreal.log("TMOP BUS FIX: {} projected onto {} at {:.1f} cm.".format(
        STOP_TO_PROJECT, lane_id, distance))


def main():
    fix_route_46_n()
    lanes, stops = collect_level_components()
    fix_lane_connections(lanes)
    fix_stop_distance(lanes, stops)
    unreal.EditorLevelLibrary.save_current_level()
    unreal.log("TMOP BUS FIX COMPLETE: route asset and loaded level saved.")


main()
