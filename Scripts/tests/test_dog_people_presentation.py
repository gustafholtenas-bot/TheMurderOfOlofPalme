from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
TYPES = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/People/TMOPPersonProfileTypes.h"
APPEARANCE_H = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/People/TMOPCharacterAppearanceComponent.h"
APPEARANCE_CPP = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/People/TMOPCharacterAppearanceComponent.cpp"
EDITOR_VM = ROOT / "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/TMOPPeopleEditorViewModels.h"
EDITOR_CPP = ROOT / "Plugins/TMOPEngine/Source/TMOPEngineEditor/Private/STMOPPeopleEditor.cpp"
AGENT_INFO = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/UI/TMOPAgentInfoChartWidget.cpp"


def read(path: Path) -> str:
    return path.read_text(encoding="utf-8")


def test_people_rows_support_dog_species_and_presentation():
    source = read(TYPES)
    assert "enum class ETMOPPersonSpecies" in source
    assert "Dog UMETA" in source
    assert "struct TMOPENGINE_API FTMOPAnimalPresentation" in source
    assert "TSoftObjectPtr<USkeletalMesh> SkeletalMesh" in source
    assert "TSoftClassPtr<UAnimInstance> AnimInstanceClass" in source
    assert "ETMOPPersonSpecies Species" in source
    assert "FTMOPAnimalPresentation AnimalPresentation" in source


def test_legacy_dog_rows_are_detected_without_moving_tables():
    source = read(TYPES)
    assert "bool IsDogProfile() const" in source
    assert 'Contains(TEXT("HUND")' in source
    assert 'Contains(TEXT("DOG")' in source


def test_dogs_bypass_human_modular_appearance_and_use_dog_animbp():
    header = read(APPEARANCE_H)
    source = read(APPEARANCE_CPP)
    assert "DefaultDogSkeletalMesh" in header
    assert "DefaultDogAnimInstanceClass" in header
    assert "ProfileComponent->Profile.IsDogProfile()" in source
    assert "ApplyDogAppearance" in source
    assert "HideHumanPresentation" in source
    assert "SetAnimInstanceClass(DogAnimClass)" in source
    assert "SetCapsuleSize" in source


def test_people_editor_round_trips_dog_settings():
    view_model = read(EDITOR_VM)
    source = read(EDITOR_CPP)
    assert "ETMOPPersonSpecies Species" in view_model
    assert "FTMOPAnimalPresentation AnimalPresentation" in view_model
    assert "General->Species" in source
    assert "General->AnimalPresentation" in source
    assert "WorkingRow.Species = General->Species" in source
    assert "WorkingRow.AnimalPresentation = General->AnimalPresentation" in source
    assert "if (Row.IsDogProfile()) return Warnings" in source


def test_agent_info_identifies_dogs_as_dogs():
    source = read(AGENT_INFO)
    assert "if (Profile.IsDogProfile())" in source
    assert 'IdentityParts.Add(TEXT("Hund"))' in source
