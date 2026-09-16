from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
CPP = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/UI/"
       "TMOPSpeechBubbleWidget.cpp").read_text(encoding="utf-8")
HEADER = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Public/UI/"
          "TMOPSpeechBubbleWidget.h").read_text(encoding="utf-8")
AGENT = (ROOT / "Plugins/TMOPEngine/Source/TMOPEngine/Private/Agents/"
         "TMOPHistoricalAgent.cpp").read_text(encoding="utf-8")


def test_bubble_has_filled_box_outline_and_tapered_tail():
    assert 'Widgets/SBoxPanel.h' in CPP
    assert 'Widgets/Layout/SVerticalBox.h' not in CPP
    assert "STMOPSpeechBubbleShape" in CPP
    assert "MakeBox" in CPP
    assert "TailLeft" in CPP and "TailRight" in CPP and "TailTip" in CPP
    assert "MakeLines" in CPP


def test_speaker_name_is_supplied_and_left_aligned():
    assert "SetSpeakerName(GetInGameDisplayName())" in AGENT
    assert "SAssignNew(SpeakerNameText" in CPP
    assert ".Justification(ETextJustify::Left)" in CPP


def test_body_is_white_centered_and_typewritten():
    assert "TypewriterCharactersPerSecond = 62.0f" in HEADER
    assert "FullSpeechString.Left(RevealedCharacterCount)" in CPP
    assert ".Justification(ETextJustify::Center)" in CPP
    assert ".ColorAndOpacity(FLinearColor::White)" in CPP
