import unreal


# Nominal first-stop times reconstructed from the 1986 weekday timetable.
# The actor is spawned 20 seconds earlier on the first lane of its existing route.
# first_stop_hms is the planned timetable time. A positive delay means late.
# (run_id, route_asset, planned_first_stop_hms, driver_entity_id)
RUNS = [
    ("BUSS41_W_2253", "BUSS41_W", (23, 7, 30), "DRIVER_41_A"),
    ("BUSS41_E_2312", "BUSS41_E", (23, 27, 30), "DRIVER_41_B"),
    ("BUSS41_W_2315", "BUSS41_W", (23, 29, 30), "DRIVER_41_C"),

    ("BUSS42_W_2300", "BUSS42_W", (23, 10, 0), "DRIVER_42_B"),
    ("BUSS42_E_2300", "BUSS42_E", (23, 16, 0), "DRIVER_42_A"),
    ("BUSS42_W_2330", "BUSS42_W", (23, 40, 0), "DRIVER_42_A"),

    ("BUSS43_S_2305", "BUSS43_S", (23, 10, 30), "DRIVER_43_B"),
    ("BUSS43_N_2305", "BUSS43_N", (23, 28, 30), "DRIVER_43_A"),
    ("BUSS43_S_2335", "BUSS43_S", (23, 40, 30), "DRIVER_43_A"),

    ("BUSS46_N_2246", "BUSS46_N", (23, 4, 0), "DRIVER_46_A"),
    ("BUSS46_S_2302", "BUSS46_S", (23, 11, 30), "DRIVER_46_B"),
    ("BUSS46_N_2302", "BUSS46_N", (23, 20, 0), "DRIVER_46_C"),
    ("BUSS46_S_2318", "BUSS46_S", (23, 27, 30), "DRIVER_46_A"),
    ("BUSS46_N_2325", "BUSS46_N", (23, 43, 0), "DRIVER_46_B"),

    ("BUSS52_S_2250", "BUSS52_S", (23, 8, 0), "DRIVER_52_A"),
    # Pia Engstrom's bus: planned at Buss52_S1 23:28, reconstructed actual
    # passage 23:30 (+120 seconds) from L261.
    ("BUSS52_S_2310", "BUSS52_S", (23, 28, 0), "DRIVER_52_B"),
    ("BUSS52_N_2320", "BUSS52_N", (23, 34, 30), "DRIVER_52_A"),
]

RUN_DELAYS_SECONDS = {
    "BUSS52_S_2310": 120,
}

ROUTE_ROOT = "/Game/TMOP/Vehicles/Bus"
BUS_BLUEPRINT_NAME = "BP_TMOPBus_ScaniaCN112"
SPEED_LIMIT_MULTIPLIER = 0.55  # 27.5 km/h on a lane marked 50 km/h.
SPAWN_LEAD_SECONDS = 20


def hms_minus_seconds(hms, seconds):
    total = hms[0] * 3600 + hms[1] * 60 + hms[2] - seconds
    total %= 24 * 3600
    return total // 3600, (total % 3600) // 60, total % 60


def hms_plus_seconds(hms, seconds):
    total = hms[0] * 3600 + hms[1] * 60 + hms[2] + seconds
    total %= 24 * 3600
    return total // 3600, (total % 3600) // 60, total % 60


def format_hms(hms):
    return "{:02d}:{:02d}:{:02d}".format(*hms)


def tmop_time(hms):
    value = unreal.TMOPTime()
    value.set_editor_property("hour", int(hms[0]))
    value.set_editor_property("minute", int(hms[1]))
    value.set_editor_property("second", int(hms[2]))
    return value


def find_asset_by_name(asset_name):
    matches = []
    for object_path in unreal.EditorAssetLibrary.list_assets(
            ROUTE_ROOT, recursive=True, include_folder=False):
        package_part = object_path.split(".", 1)[0]
        if package_part.rsplit("/", 1)[-1].lower() == asset_name.lower():
            matches.append(object_path)

    # Projects often keep BP_TMOPBus_ScaniaCN112 outside the route-data folder.
    # Fall back to Asset Registry instead of assuming a fixed Content path.
    if not matches:
        registry = unreal.AssetRegistryHelpers.get_asset_registry()
        for asset_data in registry.get_assets_by_path("/Game", recursive=True):
            if str(asset_data.asset_name).lower() == asset_name.lower():
                matches.append(
                    "{}.{}".format(asset_data.package_name, asset_data.asset_name))

    # Redirectors or duplicated registry records can report the same object twice.
    matches = sorted(set(matches))
    if len(matches) != 1:
        raise RuntimeError(
            "{}: expected exactly one asset in /Game, found {}. "
            "Rename duplicates or set an explicit asset path in the script.".format(
                asset_name, matches))
    asset = unreal.EditorAssetLibrary.load_asset(matches[0])
    if not asset:
        raise RuntimeError("Could not load asset {}".format(matches[0]))
    return asset


def find_bus_class():
    blueprint = find_asset_by_name(BUS_BLUEPRINT_NAME)
    generated = blueprint.generated_class()
    if not generated:
        raise RuntimeError("{} has no generated class".format(BUS_BLUEPRINT_NAME))
    return generated


def find_schedule_director():
    subsystem = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    matches = [
        actor for actor in subsystem.get_all_level_actors()
        if actor.get_class().get_name() == "TMOPBusScheduleDirector"
    ]
    if len(matches) != 1:
        raise RuntimeError(
            "Expected exactly one TMOPBusScheduleDirector in the loaded level; found {}."
            .format(len(matches)))
    return matches[0]


def make_run(run_id, route_name, planned_first_stop_hms, driver_id, bus_class):
    route = find_asset_by_name(route_name)
    delay_seconds = RUN_DELAYS_SECONDS.get(run_id, 0)
    actual_first_stop_hms = hms_plus_seconds(
        planned_first_stop_hms, delay_seconds)
    run = unreal.TMOPBusScheduledRun()
    run.set_editor_property("run_id", run_id)
    run.set_editor_property("route_data", route)
    run.set_editor_property("passenger_manifest", None)
    run.set_editor_property("bus_class", bus_class)
    run.set_editor_property("driver_entity_id", driver_id)

    # Empty InitialLaneId intentionally uses OrderedLaneIds[0] from the route asset.
    run.set_editor_property("initial_lane_id", "None")
    run.set_editor_property("initial_distance_along_lane", 0.0)
    run.set_editor_property("use_exact_start_time", True)
    run.set_editor_property(
        "exact_start_time",
        tmop_time(hms_minus_seconds(
            actual_first_stop_hms, SPAWN_LEAD_SECONDS)))
    run.set_editor_property("speed_limit_multiplier", SPEED_LIMIT_MULTIPLIER)
    run.set_editor_property("use_forced_despawn_time", True)
    run.set_editor_property("forced_despawn_time", tmop_time((23, 46, 0)))
    run.set_editor_property("despawn_when_traffic_route_completes", True)
    if run_id == "BUSS52_S_2310":
        source_reference = (
            "L261; SL weekday timetable 1986. Buss 52 southbound at "
            "Buss52_S1: planned {}, selected delay {:+d}s, actual {}."
            .format(
                format_hms(planned_first_stop_hms),
                delay_seconds,
                format_hms(actual_first_stop_hms)))
    else:
        source_reference = (
            "SL weekday timetable 1986; route histories; "
            "reconstructed stop passage time")
    run.set_editor_property("source_reference", source_reference)
    return run


def main():
    director = find_schedule_director()
    bus_class = find_bus_class()
    scheduled_runs = [
        make_run(run_id, route, first_stop, driver, bus_class)
        for run_id, route, first_stop, driver in RUNS
    ]

    director.modify()
    director.set_editor_property("scheduled_runs", scheduled_runs)
    director.set_editor_property("schedule_seed", 19860228)
    director.set_editor_property("maximum_simultaneous_buses", 8)
    director.set_editor_property("reset_when_time_moves_backwards", True)

    unreal.EditorLevelLibrary.save_current_level()
    unreal.log(
        "TMOP: applied {} scheduled bus runs with {} historical driver IDs."
        .format(len(scheduled_runs), len(set(item[3] for item in RUNS))))
    unreal.log(
        "TMOP: bus speed multiplier {} = {:.1f} km/h on 50 km/h lanes."
        .format(SPEED_LIMIT_MULTIPLIER, 50.0 * SPEED_LIMIT_MULTIPLIER))
    unreal.log(
        "TMOP: BUSS52_S_2310 planned 23:28:00 at Buss52_S1, "
        "delay +120s, actual 23:30:00; actor spawn 23:29:40.")


main()
