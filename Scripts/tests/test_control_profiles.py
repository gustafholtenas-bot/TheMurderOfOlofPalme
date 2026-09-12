"""Source contracts for persistent 1-4 player controls. Unreal compilation remains required."""
from pathlib import Path
import unittest
import re

ROOT = Path(__file__).resolve().parents[2]
SOURCE = ROOT / "Plugins/TMOPEngine/Source/TMOPEngine"


def read(path):
    return (SOURCE / path).read_text(encoding="utf-8")


class ControlProfileContracts(unittest.TestCase):
    def test_protected_pause_tick_member_is_not_accessed(self):
        all_cpp = "\n".join(p.read_text(encoding="utf-8") for p in SOURCE.rglob("*.cpp"))
        self.assertNotIn("->bShouldPerformFullTickWhenPaused", all_cpp)
        self.assertNotIn("SetShouldPerformFullTickWhenPaused", all_cpp)
        self.assertGreaterEqual(all_cpp.count("PrimaryActorTick.bTickEvenWhenPaused = true"), 2)

    def test_profiles_are_persistent_and_have_four_slots(self):
        header = read("Public/Player/TMOPControlSettingsSubsystem.h")
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        self.assertIn("UTMOPControlSettingsSaveGame", header)
        self.assertIn("TMOP_ControlSettings_v1", cpp)
        self.assertIn("SaveGameToSlot", cpp)
        for index in range(4):
            self.assertIn(f"MakeDefaultProfile({index},", cpp)

    def test_second_keyboard_has_distinct_defaults(self):
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        for key in ("EKeys::Up", "EKeys::Down", "EKeys::Left", "EKeys::Right",
                    "EKeys::I", "EKeys::K", "EKeys::J", "EKeys::L"):
            self.assertIn(key, cpp)
        self.assertIn("ActivePlayerCount == 2 && Index == 1", cpp)
        self.assertIn("ProcessKeyDownEvent(Routed)", cpp)
        self.assertIn("GetSlateUserForPlayer", cpp)

    def test_keyboard_conflicts_are_rejected_across_players(self):
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        self.assertIn("bCrossKeyboardPlayer", cpp)
        self.assertIn("ControlConflict", cpp)
        self.assertIn("HasConflict(PlayerIndex, Action, NewKey", cpp)

    def test_vehicle_bindings_have_separate_context(self):
        header = read("Public/Player/TMOPControlSettingsSubsystem.h")
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        for action in ("VehicleAccelerate", "VehicleReverse", "VehicleLeft",
                       "VehicleRight", "VehicleBrake", "VehicleHandbrake", "VehicleExit"):
            self.assertIn(action, header)
            self.assertIn(f"ETMOPControlAction::{action}", cpp)
        self.assertIn("ETMOPBindingContext::Vehicle", cpp)

    def test_old_native_input_paths_are_disabled_under_profiles(self):
        player = read("Private/Player/TMOPPlayerCharacter.cpp")
        self.assertIn("if (bUseControlProfiles", player)
        self.assertIn("if (!bUseControlProfiles && !bInputMappingContextAdded)", player)
        self.assertIn("ProcessControlProfileInput(DeltaSeconds)", player)
        self.assertIn("would execute the same action twice", player)

    def test_rebinding_clears_held_input(self):
        controls = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        player = read("Private/Player/TMOPPlayerCharacter.cpp")
        self.assertNotIn("bSuppressActionsUntilRelease", controls)
        self.assertIn("ReleaseState.IsSuppressed(PlayerIndex, Key", controls)
        self.assertIn("Controls->ClearHeldInput(UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(this))", player)
        self.assertIn("ProfileActionStates.Reset()", player)

    def test_control_menu_is_not_static_help_list(self):
        pause = read("Private/UI/TMOPPauseMenuWidget.cpp")
        panel = read("Private/UI/TMOPControlsPanel.cpp")
        self.assertIn("SNew(STMOPControlsPanel)", pause)
        self.assertIn("BeginBindingCapture", panel)
        self.assertIn("Controls->Rebind", panel)
        self.assertIn("SetCameraSettings", panel)

    def test_ui_uses_profile_bindings(self):
        for filename in ("TMOPPauseMenuWidget.cpp", "TMOPMapWidget.cpp",
                         "TMOPNewspaperReaderWidget.cpp", "TMOPAddressDirectoryWidget.cpp",
                         "TMOPAgentInfoChartWidget.cpp", "TMOPDialogWidget.cpp"):
            self.assertIn("TMOPMatchesControl", read(f"Private/UI/{filename}"), filename)
        self.assertIn("GetKeyDisplayText", read("Private/Player/TMOPPlayerCharacter.cpp"))

    def test_input_processor_implements_required_tick(self):
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        self.assertRegex(cpp, r"virtual void Tick\(const float DeltaTime, FSlateApplication& App,\s*TSharedRef<ICursor> Cursor\) override")

    def test_default_profiles_have_no_live_context_collisions(self):
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        function = cpp.split("FTMOPPlayerControlProfile UTMOPControlSettingsSubsystem::MakeDefaultProfile", 1)[1]
        function = function.split("void UTMOPControlSettingsSubsystem::ResetAllToDefaults", 1)[0]
        blocks = function.split("return P;", 1)
        keyboard = blocks[1].split("    else\n    {", 1)
        profiles = [blocks[0], keyboard[0], keyboard[1]]
        def context(action):
            if action in ("Pause", "WorldMap"): return {"foot", "vehicle", "menu"}
            if action.startswith("Menu"): return {"menu"}
            if action.startswith("Vehicle"): return {"vehicle"}
            if action.startswith("Look") or action == "TogglePerspective": return {"foot", "vehicle"}
            return {"foot"}
        keyboard_keys = []
        expected = set(re.findall(r"ETMOPControlAction::(\w+)", function))
        for index, block in enumerate(profiles):
            entries = re.findall(r"AddBinding\(P, ETMOPControlAction::(\w+), ([^;]+)\);", block)
            self.assertEqual(len(entries), len(expected), f"profile {index}: duplicate or missing action")
            self.assertEqual({a for a, _ in entries}, expected)
            used = []
            for action, keys in entries:
                for key in re.findall(r"EKeys::(\w+)", keys):
                    for other, other_key in used:
                        if key != other_key or not context(action) & context(other): continue
                        axis = key in ("Gamepad_LeftX", "Gamepad_LeftY", "Gamepad_RightX", "Gamepad_RightY", "MouseX", "MouseY")
                        opposites = any({action, other} == set(pair) for pair in (
                            ("MoveForward", "MoveBackward"), ("MoveLeft", "MoveRight"),
                            ("LookUp", "LookDown"), ("LookLeft", "LookRight"),
                            ("VehicleLeft", "VehicleRight"), ("MenuUp", "MenuDown"),
                            ("MenuLeft", "MenuRight")))
                        self.assertTrue(axis and opposites, f"profile {index}: {key}: {other} / {action}")
                    used.append((action, key))
            if index > 0: keyboard_keys.append({key for _, key in used})
        self.assertFalse(keyboard_keys[0] & keyboard_keys[1])

    def test_reset_and_device_switch_use_transactional_validation(self):
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        self.assertIn("return ApplyProfile(MakeDefaultProfile", cpp)
        self.assertIn("if (!ApplyProfile(Candidate, OutError)) return false", cpp)
        self.assertIn("Profiles[Index] = Previous;", cpp)
        self.assertIn("Save->DeviceProfiles = DeviceProfiles", cpp)

    def test_focus_and_navigation_are_per_user(self):
        panel = read("Private/UI/TMOPControlsPanel.cpp")
        cpp = read("Private/Player/TMOPControlSettingsSubsystem.cpp")
        self.assertNotIn("SetKeyboardFocus(", panel)
        self.assertIn("SetUserFocus(Controls->GetBindingCaptureSlateUser()", panel)
        self.assertIn("User->SetUserNavigationConfig(Config)", cpp)
        self.assertIn("User->SetUserNavigationConfig(PreviousNavigation", cpp)
        navigation = cpp.split("void UTMOPControlSettingsSubsystem::SetMenuNavigation", 1)[1]
        self.assertIn("UGameInstance* GI = GetGameInstance();", navigation)
        self.assertIn("ULocalPlayer* Local = GI->GetLocalPlayers()[PlayerIndex];", navigation)
        self.assertIn("TSharedPtr<FSlateUser> User = Local ? Local->GetSlateUser()", navigation)
        self.assertNotIn("const ULocalPlayer* Local", navigation)

    def test_restart_detaches_before_world_listeners(self):
        cpp = read("Private/Time/TMOPClockSubsystem.cpp").split("void UTMOPClockSubsystem::RestartLoop()", 1)[1]
        cpp = cpp.split("void UTMOPClockSubsystem::SetCurrentTime", 1)[0]
        self.assertLess(cpp.index("CloseSessionMenus"), cpp.index("OnLoopRestarted.Broadcast"))
        self.assertLess(cpp.index("ExitVehicle"), cpp.index("OnLoopRestarted.Broadcast"))

    def test_pickup_reserves_before_inventory_callbacks(self):
        cpp = read("Private/Items/TMOPWorldItem.cpp").split("bool ATMOPWorldItem::TryPickup", 1)[1]
        self.assertLess(cpp.index("Quantity -= Transfer"), cpp.index("TargetInventory->AddItem"))

    def test_reader_pages_do_not_share_pan_actions(self):
        cpp = read("Private/UI/TMOPNewspaperReaderWidget.cpp")
        self.assertIn("ETMOPControlAction::MenuNextPage", cpp)
        self.assertNotIn("ETMOPControlAction::InventoryNext", cpp)

    def test_radio_muted_receivers_cannot_play(self):
        cpp = read("Private/Radio/TMOPPlayerRadioComponent.cpp")
        self.assertIn("!bRadioOn || bSharedOutputMuted", cpp)
        self.assertIn("Radio != Audible", cpp)


if __name__ == "__main__":
    unittest.main()
