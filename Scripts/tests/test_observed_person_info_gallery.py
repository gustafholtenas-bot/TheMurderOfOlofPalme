from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TYPES = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/People/"
         "TMOPPersonProfileTypes.h").read_text(encoding="utf-8")
OBS_HEADER = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/Observations/"
              "TMOPObservationDirector.h").read_text(encoding="utf-8")
OBS_CPP = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/Observations/"
           "TMOPObservationDirector.cpp").read_text(encoding="utf-8")
WIDGET_H = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/UI/"
            "TMOPAgentInfoChartWidget.h").read_text(encoding="utf-8")
WIDGET_CPP = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/UI/"
              "TMOPAgentInfoChartWidget.cpp").read_text(encoding="utf-8")
PLAYER = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/Player/"
          "TMOPPlayerCharacter.cpp").read_text(encoding="utf-8")
EDITOR_VM = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/"
             "TMOPPeopleEditorViewModels.h").read_text(encoding="utf-8")
EDITOR_CPP = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/"
              "STMOPPeopleEditor.cpp").read_text(encoding="utf-8")


def test_person_rows_support_multiple_evidence_images():
    assert "struct TMOPENGINE_API FTMOPEvidenceImage" in TYPES
    assert "TArray<FTMOPEvidenceImage> EvidenceImages" in TYPES
    assert "PhantomImage" in TYPES and "Sketch" in TYPES


def test_people_editor_round_trips_gallery_data():
    assert "TArray<FTMOPEvidenceImage> EvidenceImages" in EDITOR_VM
    assert "General->EvidenceImages = WorkingRow.EvidenceImages" in EDITOR_CPP
    assert "WorkingRow.EvidenceImages = General->EvidenceImages" in EDITOR_CPP


def test_observation_director_resolves_direct_and_linked_observers():
    assert "GetObserverEntityIdsForTarget" in OBS_HEADER
    assert "Pair.Value.ObservedEntityId == ObservedEntityId" in OBS_CPP
    assert "Link.LinkedEntityId != ObservedEntityId" in OBS_CPP
    assert "Observation.ObserverEntityIds" in OBS_CPP


def test_agent_info_receives_entity_and_builds_observer_list():
    assert "FName InspectedEntityId" in WIDGET_H
    assert "BuildObserverSummary" in WIDGET_CPP
    assert "OBSERVERAD AV" in WIDGET_CPP
    assert "ProfileComponent->ResolvedEntityId" in PLAYER


def test_agent_info_builds_scrollable_multi_image_gallery():
    assert "RefreshEvidenceGallery" in WIDGET_CPP
    assert "EvidenceGallery->AddSlot()" in WIDGET_CPP
    assert ".Orientation(Orient_Horizontal)" in WIDGET_CPP
    assert "EvidenceImageBrushes" in WIDGET_H
