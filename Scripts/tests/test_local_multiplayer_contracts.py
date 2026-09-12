"""Source integration checks; these do NOT substitute for Unreal/UHT or controller tests."""
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine"

def source(path):
    return (SOURCE / path).read_text(encoding="utf-8")

class LocalMultiplayerContracts(unittest.TestCase):
    def test_scoped_pause_owns_both_clock_and_world(self):
        clock = source("Private/Time/TMOPClockSubsystem.cpp")
        self.assertIn("Request.Owner.Get() == Owner && Request.Reason == Reason", clock)
        self.assertIn("Request.Owner.IsValid()", clock)
        self.assertIn("CanRunClock", clock)
        self.assertIn("World->IsPaused()", clock)
        self.assertIn("SetGamePaused(World, true)", clock)
        self.assertIn("!bWantPause && bOwnsWorldPause", clock)

    def test_player_menus_never_unpause_other_players(self):
        player = source("Private/Player/TMOPPlayerCharacter.cpp")
        self.assertNotRegex(player, r"->(?:SetPause|StartClock|PauseClock)\(")
        for reason in ("PauseMenu", "WorldMap", "Newspaper"):
            self.assertIn(f'RequestPause(this, TEXT("{reason}"))', player)
            self.assertIn(f'ReleasePause(this, TEXT("{reason}"))', player)
        self.assertIn("ReleaseAllPauses(this)", player)

    def test_player_viewports_and_focus(self):
        player = source("Private/Player/TMOPPlayerCharacter.cpp")
        self.assertNotIn("AddToViewport", player)
        self.assertEqual(player.count("->AddToPlayerScreen("), 11)
        self.assertIn("GetSlateUser()->GetUserIndex()", player)
        self.assertIn("SetUserFocus(UserIndex", player)
        self.assertIn("GetPlayerViewRect", player)
        self.assertNotIn("SetKeyboardFocus", source("Private/UI/TMOPLoopEndWidget.cpp"))
        self.assertIn("NativePaint", source("Private/UI/TMOPLocalPlayerOverlay.cpp"))

    def test_session_guards_and_grounded_spawn(self):
        session = source("Private/Player/TMOPLocalMultiplayerSubsystem.cpp")
        for token in ("NM_Standalone", "IsValidPlayerCount", "CreatePlayer", "RemovePlayer",
                      "LineTraceSingleByChannel", "OverlapBlockingTestByProfile", "bOverlapsParty",
                      "AutoPossessPlayer = EAutoReceiveInput::Disabled", "bChangingSession",
                      "!bRestartPartyWithClock", "ConfigureForLocalPlayer"):
            self.assertIn(token, session)
        self.assertIn("IsClockRunRequested", source("Private/Time/TMOPSimulationDebugDirector.cpp"))

    def test_four_menu_choices(self):
        menu = source("Private/UI/TMOPMainMenuWidget.cpp")
        for count in range(1, 5):
            self.assertIn(f"PlayerCountClicked, {count}", menu)
        self.assertIn("KeyboardModeClicked", menu)
        intro = source("Private/UI/TMOPMainMenuIntroDirector.cpp")
        self.assertIn("StartParty", intro)
        self.assertIn("bNewGameRequested", intro)
        self.assertIn("bMenuInputApplied != bMenuInput", intro)

    def test_gamepad_reading_and_map_controls(self):
        map_source = source("Private/UI/TMOPMapWidget.cpp")
        for key in ("Gamepad_LeftX", "Gamepad_LeftY", "Gamepad_RightShoulder", "Gamepad_FaceButton_Right"):
            self.assertIn(key, map_source)
        self.assertIn("TMOPFitLocalPanel(this", map_source)
        for name in ("TMOPAddressDirectoryWidget", "TMOPNewspaperReaderWidget", "TMOPAgentInfoChartWidget"):
            widget = source(f"Private/UI/{name}.cpp")
            self.assertIn("Gamepad_DPad_Down", widget)
            self.assertIn("Gamepad_FaceButton_Right", widget)

    def test_save_validation_precedes_party_mutation(self):
        save = source("Private/UI/TMOPSaveGameService.cpp")
        self.assertLess(save.index("ContainsNaN"), save.index("EnsurePlayerCount"))
        self.assertLess(save.index("SavedSecond >="), save.index("EnsurePlayerCount"))

    def test_shared_vehicle_and_item_ownership(self):
        takeover = source("Private/Player/TMOPVehicleTakeoverComponent.cpp")
        self.assertIn("PreviousOccupant->IsPlayerControlled()", takeover)
        camera = source("Private/Player/TMOPPlayerVehicleSessionComponent.cpp")
        self.assertIn("LocalVehicleCamera", camera)
        self.assertNotIn("GetFirstPlayerController", camera)
        self.assertIn("Quantity <= 0", source("Private/Items/TMOPWorldItem.cpp"))

    def test_save_party_and_read_legacy(self):
        save = source("Private/UI/TMOPSaveGameService.cpp")
        for token in ("SaveFormatVersion = 3", "Save->LocalPlayers", "Save->SaveFormatVersion >= 3",
                      "EnsurePlayerCount", "SaveExitVehicles", "LoadPartyLeader", "AdoptLoadedSession"):
            self.assertIn(token, save)
        self.assertIn("Player->SetActorTransform(Save->PlayerTransform", save)

    def test_shared_media_and_all_listeners(self):
        film = source("Private/Venues/TMOPGrandFilmDirector.cpp")
        self.assertIn("bTickEvenWhenPaused = true", film)
        self.assertIn("!Clock->IsClockRunning()", film)
        for name in ("TMOPAudioDirector", "TMOPAgentAudioComponent", "TMOPVehicleAudioComponent"):
            text = source(f"Private/Audio/{name}.cpp")
            self.assertIn("FindNearestCamera", text)
            self.assertNotIn("GetPlayerCameraManager(this, 0)", text)
        self.assertIn("SetOwnerNoSee(true)", source("Private/Newspapers/TMOPNewspaperReadingComponent.cpp"))

    def test_prior_killer_fix_preserved(self):
        self.assertIn("TUniquePtr<FTMOPKillerBranchRuntime, FTMOPKillerBranchRuntimeDeleter>",
                      source("Public/Killer/TMOPKillerBranchDirector.h"))

    def test_generated_headers_last(self):
        for path in SOURCE.glob("Public/**/*.h"):
            includes = re.findall(r'^#include\s+[<"]([^>"\n]+)', path.read_text(encoding="utf-8"), re.M)
            generated = [inc for inc in includes if inc.endswith(".generated.h")]
            if generated:
                self.assertEqual(includes[-1], generated[0], str(path))

if __name__ == "__main__":
    unittest.main()
