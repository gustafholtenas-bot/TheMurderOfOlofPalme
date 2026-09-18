from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TYPES = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/People/"
         "TMOPPersonProfileTypes.h").read_text(encoding="utf-8")
PEOPLE_RUNTIME = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/People/"
                  "TMOPPersonRegistryDirector.cpp").read_text(encoding="utf-8")
VEHICLE_HEADER = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/Vehicles/"
                  "TMOPHistoricalVehicleDirector.h").read_text(encoding="utf-8")
VEHICLE_RUNTIME = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/Vehicles/"
                   "TMOPHistoricalVehicleDirector.cpp").read_text(encoding="utf-8")
EDITOR = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/"
          "STMOPPeopleEditor.cpp").read_text(encoding="utf-8")


def test_person_entry_stores_stable_vehicle_reference():
    assert "enum class ETMOPVehicleTimelineReferencePoint" in TYPES
    assert "bUseVehicleTimelineReference" in TYPES
    assert "VehicleReferenceEntityId" in TYPES
    assert "VehicleReferenceEntryId" in TYPES
    assert "VehicleReferenceOffsetSeconds" in TYPES


def test_runtime_resolves_reference_from_vehicle_director():
    assert "ResolvePersonTimelineReference" in VEHICLE_HEADER
    assert "ResolvePersonTimelineReference" in VEHICLE_RUNTIME
    assert "ResolveDrivingDepartureSecond" in VEHICLE_RUNTIME
    assert "ResolveDrivingWindow" in VEHICLE_RUNTIME
    assert "ResolveTimelineEntryCompletionSecond" in VEHICLE_RUNTIME
    assert "Entry.VehicleReferenceOffsetSeconds" in PEOPLE_RUNTIME


def test_people_editor_has_route_picker_and_one_click_timing_buttons():
    assert '"BILRUTTENS TID"' in EDITOR
    assert "BuildVehicleRouteReferenceMenu" in EDITOR
    assert '"2 s före avgång"' in EDITOR
    assert '"Vid avgång"' in EDITOR
    assert '"Vid ankomst"' in EDITOR
    assert '"2 s efter ankomst"' in EDITOR
    assert '"Efter stopp"' in EDITOR


def test_editor_resolves_and_displays_live_vehicle_time():
    assert "TMOPVehicleTimeline::ResolveDeparture" in EDITOR
    assert "TMOPVehicleTimeline::ResolveWindow" in EDITOR
    assert "TMOPVehicleRoute::CompletionDelay" in EDITOR
    assert "It will follow future vehicle timeline changes" in EDITOR
