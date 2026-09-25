#include "Player/TMOPControlSettingsSubsystem.h"

#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"
#include "Framework/Application/NavigationConfig.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"

namespace
{
constexpr TCHAR ControlSettingsSlot[] = TEXT("TMOP_ControlSettings_v1");

enum class ETMOPBindingContext : uint8 { OnFoot, Vehicle, Menu, SharedGameplay, Global };

ETMOPBindingContext ContextFor(const ETMOPControlAction Action)
{
    switch (Action)
    {
    case ETMOPControlAction::VehicleAccelerate:
    case ETMOPControlAction::VehicleReverse:
    case ETMOPControlAction::VehicleLeft:
    case ETMOPControlAction::VehicleRight:
    case ETMOPControlAction::VehicleBrake:
    case ETMOPControlAction::VehicleHandbrake:
    case ETMOPControlAction::VehicleHighSpeed:
    case ETMOPControlAction::VehicleExit: return ETMOPBindingContext::Vehicle;
    case ETMOPControlAction::MenuUp:
    case ETMOPControlAction::MenuDown:
    case ETMOPControlAction::MenuLeft:
    case ETMOPControlAction::MenuRight:
    case ETMOPControlAction::MenuConfirm:
    case ETMOPControlAction::MenuZoomIn:
    case ETMOPControlAction::MenuZoomOut:
    case ETMOPControlAction::MenuReset:
    case ETMOPControlAction::MenuPreviousPage:
    case ETMOPControlAction::MenuNextPage:
    case ETMOPControlAction::MenuBack: return ETMOPBindingContext::Menu;
    case ETMOPControlAction::LookUp:
    case ETMOPControlAction::LookDown:
    case ETMOPControlAction::LookLeft:
    case ETMOPControlAction::LookRight:
    case ETMOPControlAction::LookZoom:
    case ETMOPControlAction::TogglePerspective: return ETMOPBindingContext::SharedGameplay;
    case ETMOPControlAction::WorldMap:
    case ETMOPControlAction::TimelineCursor:
    case ETMOPControlAction::Pause: return ETMOPBindingContext::Global;
    default: return ETMOPBindingContext::OnFoot;
    }
}

void AddBinding(FTMOPPlayerControlProfile& Profile, const ETMOPControlAction Action,
    const FKey Primary, const FKey Secondary = FKey())
{
    FTMOPControlBinding& Binding = Profile.Bindings.AddDefaulted_GetRef();
    Binding.Action = Action;
    Binding.PrimaryKey = Primary;
    Binding.SecondaryKey = Secondary;
}

bool IsReservedSystemKey(const FKey Key)
{
    return Key == EKeys::AnyKey;
}

bool IsOppositeAxisPair(const ETMOPControlAction A, const ETMOPControlAction B, const FKey Key)
{
    if (!Key.IsAxis1D()) return false;
    return (A == ETMOPControlAction::MoveForward && B == ETMOPControlAction::MoveBackward) ||
        (B == ETMOPControlAction::MoveForward && A == ETMOPControlAction::MoveBackward) ||
        (A == ETMOPControlAction::MoveLeft && B == ETMOPControlAction::MoveRight) ||
        (B == ETMOPControlAction::MoveLeft && A == ETMOPControlAction::MoveRight) ||
        (A == ETMOPControlAction::LookUp && B == ETMOPControlAction::LookDown) ||
        (B == ETMOPControlAction::LookUp && A == ETMOPControlAction::LookDown) ||
        (A == ETMOPControlAction::LookLeft && B == ETMOPControlAction::LookRight) ||
        (B == ETMOPControlAction::LookLeft && A == ETMOPControlAction::LookRight) ||
        (A == ETMOPControlAction::VehicleLeft && B == ETMOPControlAction::VehicleRight) ||
        (B == ETMOPControlAction::VehicleLeft && A == ETMOPControlAction::VehicleRight) ||
        (A == ETMOPControlAction::MenuUp && B == ETMOPControlAction::MenuDown) ||
        (B == ETMOPControlAction::MenuUp && A == ETMOPControlAction::MenuDown) ||
        (A == ETMOPControlAction::MenuLeft && B == ETMOPControlAction::MenuRight) ||
        (B == ETMOPControlAction::MenuLeft && A == ETMOPControlAction::MenuRight);
}
}

class FTMOPControlInputProcessor final : public IInputProcessor
{
public:
    explicit FTMOPControlInputProcessor(UTMOPControlSettingsSubsystem* InOwner)
        : Owner(InOwner) {}

    virtual void Tick(const float DeltaTime, FSlateApplication& App,
        TSharedRef<ICursor> Cursor) override
    {
        // Key-up events may be lost when the application loses focus.
        if (!App.IsActive() && Owner.IsValid()) Owner->ReleasePhysicalInput();
    }

    virtual bool HandleKeyDownEvent(FSlateApplication& App, const FKeyEvent& Event) override
    {
        if (!Owner.IsValid()) return false;
        if (!Event.GetKey().IsGamepadKey()) Owner->SetPhysicalKeyDown(Event.GetKey(), true);
        return RouteSharedKeyboardEvent(App, Event, true);
    }
    virtual bool HandleKeyUpEvent(FSlateApplication& App, const FKeyEvent& Event) override
    {
        if (!Owner.IsValid()) return false;
        if (!Event.GetKey().IsGamepadKey()) Owner->SetPhysicalKeyDown(Event.GetKey(), false);
        return RouteSharedKeyboardEvent(App, Event, false);
    }
    virtual bool HandleMouseButtonDownEvent(FSlateApplication&, const FPointerEvent& Event) override
    { if (Owner.IsValid()) Owner->SetPhysicalKeyDown(Event.GetEffectingButton(), true); return false; }
    virtual bool HandleMouseButtonUpEvent(FSlateApplication&, const FPointerEvent& Event) override
    { if (Owner.IsValid()) Owner->SetPhysicalKeyDown(Event.GetEffectingButton(), false); return false; }
private:
    TWeakObjectPtr<UTMOPControlSettingsSubsystem> Owner;
    bool bRoutingEvent = false;

    bool RouteSharedKeyboardEvent(FSlateApplication& App, const FKeyEvent& Event,
        const bool bPressed)
    {
        if (bRoutingEvent || !Owner.IsValid()) return false;
        if (Event.GetKey().IsGamepadKey() && !Owner->IsBindingCaptureActive()) return false;
        const int32 PlayerIndex = Owner->GetKeyboardOwnerForKey(Event.GetKey());
        const uint32 UserIndex = Owner->IsBindingCaptureActive()
            ? Owner->GetBindingCaptureSlateUser()
            : (PlayerIndex > 0 ? Owner->GetSlateUserForPlayer(PlayerIndex) : MAX_uint32);
        if (UserIndex == MAX_uint32) return false;
        if (!Owner->IsBindingCaptureActive() && PlayerIndex <= 0) return false;
        const FKeyEvent Routed(Event.GetKey(), Event.GetModifierKeys(), UserIndex,
            Event.IsRepeat(), Event.GetCharacter(), Event.GetKeyCode());
        TGuardValue<bool> Guard(bRoutingEvent, true);
        if (bPressed) App.ProcessKeyDownEvent(Routed);
        else App.ProcessKeyUpEvent(Routed);
        // The routed event belongs exclusively to player 2. Do not also send it
        // through Slate user 0, which would move player 1's menu focus.
        return true;
    }
};

bool UTMOPControlSettingsSubsystem::IsKeyboardDevice(const ETMOPControlDevice Device)
{
    return Device == ETMOPControlDevice::KeyboardMouse ||
        Device == ETMOPControlDevice::KeyboardOnly;
}

void UTMOPControlSettingsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    if (!LoadSettings()) ResetAllToDefaults();
    if (FSlateApplication::IsInitialized())
    {
        InputProcessor = MakeShared<FTMOPControlInputProcessor>(this);
        FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor.ToSharedRef(), 0);
    }
}

void UTMOPControlSettingsSubsystem::Deinitialize()
{
    SaveSettings();
    for (int32 Index = 0; Index < 4; ++Index) SetMenuNavigation(Index, false);
    if (InputProcessor.IsValid() && FSlateApplication::IsInitialized())
        FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor.ToSharedRef());
    InputProcessor.Reset();
    PressedKeys.Reset();
    Super::Deinitialize();
}

FTMOPPlayerControlProfile UTMOPControlSettingsSubsystem::MakeDefaultProfile(
    const int32 PlayerIndex, const ETMOPControlDevice Device) const
{
    FTMOPPlayerControlProfile P;
    P.PlayerIndex = FMath::Clamp(PlayerIndex, 0, 3);
    P.Device = Device;
    // A distinct on-foot action. Gamepads and keyboard player 2 can bind it
    // explicitly without stealing an existing gameplay button.
    AddBinding(P, ETMOPControlAction::VehicleTakeover,
        Device == ETMOPControlDevice::KeyboardMouse && PlayerIndex == 0 ? EKeys::F8 : FKey());
    AddBinding(P, ETMOPControlAction::TimelineCursor,
        Device == ETMOPControlDevice::KeyboardMouse && PlayerIndex == 0 ? EKeys::LeftAlt : FKey());
    if (Device == ETMOPControlDevice::Gamepad)
    {
        AddBinding(P, ETMOPControlAction::MoveForward, EKeys::Gamepad_LeftY);
        AddBinding(P, ETMOPControlAction::MoveBackward, EKeys::Gamepad_LeftY);
        AddBinding(P, ETMOPControlAction::MoveLeft, EKeys::Gamepad_LeftX);
        AddBinding(P, ETMOPControlAction::MoveRight, EKeys::Gamepad_LeftX);
        AddBinding(P, ETMOPControlAction::LookUp, EKeys::Gamepad_RightY);
        AddBinding(P, ETMOPControlAction::LookDown, EKeys::Gamepad_RightY);
        AddBinding(P, ETMOPControlAction::LookLeft, EKeys::Gamepad_RightX);
        AddBinding(P, ETMOPControlAction::LookRight, EKeys::Gamepad_RightX);
        AddBinding(P, ETMOPControlAction::Jump, EKeys::Gamepad_FaceButton_Bottom);
        AddBinding(P, ETMOPControlAction::Sprint, EKeys::Gamepad_LeftThumbstick);
        AddBinding(P, ETMOPControlAction::ExtraSprint, FKey());
        AddBinding(P, ETMOPControlAction::Interact, EKeys::Gamepad_FaceButton_Left);
        AddBinding(P, ETMOPControlAction::PrimaryAction, EKeys::Gamepad_RightTrigger);
        AddBinding(P, ETMOPControlAction::SecondaryAction, EKeys::Gamepad_LeftTrigger);
        AddBinding(P, ETMOPControlAction::Cancel, EKeys::Gamepad_FaceButton_Right);
        AddBinding(P, ETMOPControlAction::Crouch, EKeys::Gamepad_DPad_Down);
        AddBinding(P, ETMOPControlAction::Kick, EKeys::Gamepad_RightShoulder);
        AddBinding(P, ETMOPControlAction::ShoulderSwap, EKeys::Gamepad_DPad_Up);
        AddBinding(P, ETMOPControlAction::Pause, EKeys::Gamepad_Special_Right);
        AddBinding(P, ETMOPControlAction::WorldMap, EKeys::Gamepad_Special_Left);
        AddBinding(P, ETMOPControlAction::QuickInventory, EKeys::Gamepad_LeftShoulder);
        AddBinding(P, ETMOPControlAction::InventoryPrevious, EKeys::Gamepad_DPad_Left);
        AddBinding(P, ETMOPControlAction::InventoryNext, EKeys::Gamepad_DPad_Right);
        AddBinding(P, ETMOPControlAction::DropItem, EKeys::Gamepad_FaceButton_Top);
        AddBinding(P, ETMOPControlAction::LookZoom, FKey());
        AddBinding(P, ETMOPControlAction::TogglePerspective, EKeys::Gamepad_RightThumbstick);
        AddBinding(P, ETMOPControlAction::VehicleAccelerate, EKeys::Gamepad_RightTriggerAxis);
        AddBinding(P, ETMOPControlAction::VehicleReverse, EKeys::Gamepad_LeftTriggerAxis);
        AddBinding(P, ETMOPControlAction::VehicleLeft, EKeys::Gamepad_LeftX);
        AddBinding(P, ETMOPControlAction::VehicleRight, EKeys::Gamepad_LeftX);
        AddBinding(P, ETMOPControlAction::VehicleBrake, EKeys::Gamepad_FaceButton_Bottom);
        AddBinding(P, ETMOPControlAction::VehicleHandbrake, EKeys::Gamepad_FaceButton_Right);
        AddBinding(P, ETMOPControlAction::VehicleExit, EKeys::Gamepad_FaceButton_Left);
        AddBinding(P, ETMOPControlAction::VehicleHighSpeed, EKeys::Gamepad_LeftThumbstick);
        AddBinding(P, ETMOPControlAction::MenuUp, EKeys::Gamepad_DPad_Up, EKeys::Gamepad_LeftY);
        AddBinding(P, ETMOPControlAction::MenuDown, EKeys::Gamepad_DPad_Down, EKeys::Gamepad_LeftY);
        AddBinding(P, ETMOPControlAction::MenuLeft, EKeys::Gamepad_DPad_Left, EKeys::Gamepad_LeftX);
        AddBinding(P, ETMOPControlAction::MenuRight, EKeys::Gamepad_DPad_Right, EKeys::Gamepad_LeftX);
        AddBinding(P, ETMOPControlAction::MenuConfirm, EKeys::Gamepad_FaceButton_Bottom);
        AddBinding(P, ETMOPControlAction::MenuZoomIn, EKeys::Gamepad_RightShoulder);
        AddBinding(P, ETMOPControlAction::MenuZoomOut, EKeys::Gamepad_LeftShoulder);
        AddBinding(P, ETMOPControlAction::MenuReset, EKeys::Gamepad_FaceButton_Left);
        AddBinding(P, ETMOPControlAction::MenuPreviousPage, EKeys::Gamepad_LeftTrigger);
        AddBinding(P, ETMOPControlAction::MenuNextPage, EKeys::Gamepad_RightTrigger);
        AddBinding(P, ETMOPControlAction::MenuBack, EKeys::Gamepad_FaceButton_Right);
        return P;
    }

    const bool bSecondKeyboard = P.PlayerIndex == 1 || Device == ETMOPControlDevice::KeyboardOnly;
    if (!bSecondKeyboard)
    {
        AddBinding(P, ETMOPControlAction::MoveForward, EKeys::W);
        AddBinding(P, ETMOPControlAction::MoveBackward, EKeys::S);
        AddBinding(P, ETMOPControlAction::MoveLeft, EKeys::A);
        AddBinding(P, ETMOPControlAction::MoveRight, EKeys::D);
        AddBinding(P, ETMOPControlAction::LookUp, EKeys::MouseY);
        AddBinding(P, ETMOPControlAction::LookDown, EKeys::MouseY);
        AddBinding(P, ETMOPControlAction::LookLeft, EKeys::MouseX);
        AddBinding(P, ETMOPControlAction::LookRight, EKeys::MouseX);
        AddBinding(P, ETMOPControlAction::Jump, EKeys::SpaceBar);
        AddBinding(P, ETMOPControlAction::Sprint, EKeys::LeftShift);
        AddBinding(P, ETMOPControlAction::ExtraSprint, EKeys::LeftControl);
        AddBinding(P, ETMOPControlAction::Interact, EKeys::E);
        AddBinding(P, ETMOPControlAction::PrimaryAction, EKeys::LeftMouseButton);
        AddBinding(P, ETMOPControlAction::SecondaryAction, EKeys::RightMouseButton);
        AddBinding(P, ETMOPControlAction::Cancel, EKeys::Escape);
        AddBinding(P, ETMOPControlAction::Crouch, EKeys::C);
        AddBinding(P, ETMOPControlAction::Kick, EKeys::F);
        AddBinding(P, ETMOPControlAction::ShoulderSwap, EKeys::X);
        AddBinding(P, ETMOPControlAction::Pause, EKeys::Enter);
        AddBinding(P, ETMOPControlAction::WorldMap, EKeys::M);
        AddBinding(P, ETMOPControlAction::QuickInventory, EKeys::Tab);
        AddBinding(P, ETMOPControlAction::InventoryPrevious, EKeys::Q, EKeys::MouseScrollDown);
        AddBinding(P, ETMOPControlAction::InventoryNext, EKeys::R, EKeys::MouseScrollUp);
        AddBinding(P, ETMOPControlAction::DropItem, EKeys::G);
        AddBinding(P, ETMOPControlAction::LookZoom, EKeys::Z);
        AddBinding(P, ETMOPControlAction::TogglePerspective, EKeys::V);
        AddBinding(P, ETMOPControlAction::VehicleAccelerate, EKeys::W);
        AddBinding(P, ETMOPControlAction::VehicleReverse, EKeys::S);
        AddBinding(P, ETMOPControlAction::VehicleLeft, EKeys::A);
        AddBinding(P, ETMOPControlAction::VehicleRight, EKeys::D);
        AddBinding(P, ETMOPControlAction::VehicleBrake, EKeys::SpaceBar);
        AddBinding(P, ETMOPControlAction::VehicleHandbrake, EKeys::LeftShift);
        AddBinding(P, ETMOPControlAction::VehicleExit, EKeys::E);
        AddBinding(P, ETMOPControlAction::VehicleHighSpeed, EKeys::LeftControl);
        AddBinding(P, ETMOPControlAction::MenuUp, EKeys::W);
        AddBinding(P, ETMOPControlAction::MenuDown, EKeys::S);
        AddBinding(P, ETMOPControlAction::MenuLeft, EKeys::A);
        AddBinding(P, ETMOPControlAction::MenuRight, EKeys::D);
        AddBinding(P, ETMOPControlAction::MenuConfirm, EKeys::SpaceBar);
        AddBinding(P, ETMOPControlAction::MenuZoomIn, EKeys::Equals);
        AddBinding(P, ETMOPControlAction::MenuZoomOut, EKeys::Hyphen);
        AddBinding(P, ETMOPControlAction::MenuReset, EKeys::R);
        AddBinding(P, ETMOPControlAction::MenuPreviousPage, EKeys::Q);
        AddBinding(P, ETMOPControlAction::MenuNextPage, EKeys::E);
        AddBinding(P, ETMOPControlAction::MenuBack, EKeys::Escape);
    }
    else
    {
        AddBinding(P, ETMOPControlAction::MoveForward, EKeys::Up);
        AddBinding(P, ETMOPControlAction::MoveBackward, EKeys::Down);
        AddBinding(P, ETMOPControlAction::MoveLeft, EKeys::Left);
        AddBinding(P, ETMOPControlAction::MoveRight, EKeys::Right);
        AddBinding(P, ETMOPControlAction::LookUp, EKeys::I);
        AddBinding(P, ETMOPControlAction::LookDown, EKeys::K);
        AddBinding(P, ETMOPControlAction::LookLeft, EKeys::J);
        AddBinding(P, ETMOPControlAction::LookRight, EKeys::L);
        AddBinding(P, ETMOPControlAction::Jump, EKeys::RightControl);
        AddBinding(P, ETMOPControlAction::Sprint, EKeys::N);
        AddBinding(P, ETMOPControlAction::ExtraSprint, EKeys::RightAlt);
        AddBinding(P, ETMOPControlAction::Interact, EKeys::RightShift);
        AddBinding(P, ETMOPControlAction::PrimaryAction, EKeys::Comma);
        AddBinding(P, ETMOPControlAction::SecondaryAction, EKeys::Period);
        AddBinding(P, ETMOPControlAction::Cancel, EKeys::BackSpace);
        AddBinding(P, ETMOPControlAction::Crouch, EKeys::H);
        AddBinding(P, ETMOPControlAction::Kick, EKeys::Semicolon);
        AddBinding(P, ETMOPControlAction::ShoulderSwap, EKeys::Apostrophe);
        AddBinding(P, ETMOPControlAction::Pause, EKeys::P);
        AddBinding(P, ETMOPControlAction::WorldMap, EKeys::O);
        AddBinding(P, ETMOPControlAction::QuickInventory, EKeys::U);
        AddBinding(P, ETMOPControlAction::InventoryPrevious, EKeys::Y);
        AddBinding(P, ETMOPControlAction::InventoryNext, EKeys::B);
        AddBinding(P, ETMOPControlAction::DropItem, EKeys::Delete);
        AddBinding(P, ETMOPControlAction::LookZoom, EKeys::Slash);
        AddBinding(P, ETMOPControlAction::TogglePerspective, EKeys::T);
        AddBinding(P, ETMOPControlAction::VehicleAccelerate, EKeys::Up);
        AddBinding(P, ETMOPControlAction::VehicleReverse, EKeys::Down);
        AddBinding(P, ETMOPControlAction::VehicleLeft, EKeys::Left);
        AddBinding(P, ETMOPControlAction::VehicleRight, EKeys::Right);
        AddBinding(P, ETMOPControlAction::VehicleBrake, EKeys::RightControl);
        AddBinding(P, ETMOPControlAction::VehicleHandbrake, EKeys::RightShift);
        AddBinding(P, ETMOPControlAction::VehicleExit, EKeys::BackSpace);
        AddBinding(P, ETMOPControlAction::VehicleHighSpeed, EKeys::N);
        AddBinding(P, ETMOPControlAction::MenuUp, EKeys::I);
        AddBinding(P, ETMOPControlAction::MenuDown, EKeys::K);
        AddBinding(P, ETMOPControlAction::MenuLeft, EKeys::J);
        AddBinding(P, ETMOPControlAction::MenuRight, EKeys::L);
        AddBinding(P, ETMOPControlAction::MenuConfirm, EKeys::RightShift);
        AddBinding(P, ETMOPControlAction::MenuZoomIn, EKeys::PageUp);
        AddBinding(P, ETMOPControlAction::MenuZoomOut, EKeys::PageDown);
        AddBinding(P, ETMOPControlAction::MenuReset, EKeys::Home);
        AddBinding(P, ETMOPControlAction::MenuPreviousPage, EKeys::Y);
        AddBinding(P, ETMOPControlAction::MenuNextPage, EKeys::B);
        AddBinding(P, ETMOPControlAction::MenuBack, EKeys::BackSpace);
    }
    return P;
}

void UTMOPControlSettingsSubsystem::ResetAllToDefaults()
{
    DeviceProfiles.Reset();
    Profiles.Reset();
    Profiles.Add(MakeDefaultProfile(0, ETMOPControlDevice::KeyboardMouse));
    Profiles.Add(MakeDefaultProfile(1, ETMOPControlDevice::KeyboardOnly));
    Profiles.Add(MakeDefaultProfile(2, ETMOPControlDevice::Gamepad));
    Profiles.Add(MakeDefaultProfile(3, ETMOPControlDevice::Gamepad));
    for (const auto& P : Profiles) RememberProfile(P);
    RememberProfile(MakeDefaultProfile(0, ETMOPControlDevice::Gamepad));
    RememberProfile(MakeDefaultProfile(1, ETMOPControlDevice::Gamepad));
    SaveSettings();
    ClearHeldInput();
}

void UTMOPControlSettingsSubsystem::ResetPlayerToDefaults(const int32 PlayerIndex)
{
    FText Error;
    TryResetPlayerToDefaults(PlayerIndex, Error);
}

bool UTMOPControlSettingsSubsystem::TryResetPlayerToDefaults(const int32 PlayerIndex, FText& OutError)
{
    if (!Profiles.IsValidIndex(PlayerIndex)) return false;
    return ApplyProfile(MakeDefaultProfile(PlayerIndex, Profiles[PlayerIndex].Device), OutError);
}

void UTMOPControlSettingsSubsystem::ConfigureForSession(const int32 PlayerCount,
    const bool bKeyboardMode, const bool bSharedKeyboardForPlayerTwo)
{
    ActivePlayerCount = FMath::Clamp(PlayerCount, 1, 4);
    while (Profiles.Num() < 4) Profiles.Add(MakeDefaultProfile(Profiles.Num(),
        Profiles.Num() < 2 ? ETMOPControlDevice::KeyboardOnly : ETMOPControlDevice::Gamepad));
    for (int32 Index = 0; Index < ActivePlayerCount; ++Index)
    {
        ETMOPControlDevice Wanted = ETMOPControlDevice::Gamepad;
        if (bKeyboardMode && Index == 0) Wanted = ETMOPControlDevice::KeyboardMouse;
        // The shared-keyboard layout is deliberately a two-player layout. With
        // 3-4 players, P2-P4 use the existing isolated controller routing.
        else if (bKeyboardMode && bSharedKeyboardForPlayerTwo &&
            ActivePlayerCount == 2 && Index == 1)
            Wanted = ETMOPControlDevice::KeyboardOnly;
        if (Profiles[Index].Device != Wanted)
        {
            RememberProfile(Profiles[Index]);
            const FTMOPPlayerControlProfile* Saved = DeviceProfiles.FindByPredicate(
                [Index, Wanted](const FTMOPPlayerControlProfile& P)
                { return P.PlayerIndex == Index && P.Device == Wanted; });
            Profiles[Index] = Saved ? *Saved : MakeDefaultProfile(Index, Wanted);
        }
    }
    SaveSettings();
    ClearHeldInput();
}

FTMOPPlayerControlProfile UTMOPControlSettingsSubsystem::GetProfile(const int32 PlayerIndex) const
{
    return Profiles.IsValidIndex(PlayerIndex) ? Profiles[PlayerIndex] : FTMOPPlayerControlProfile();
}

const FTMOPControlBinding* UTMOPControlSettingsSubsystem::FindBinding(
    const int32 PlayerIndex, const ETMOPControlAction Action) const
{
    if (!Profiles.IsValidIndex(PlayerIndex)) return nullptr;
    return Profiles[PlayerIndex].Bindings.FindByPredicate(
        [Action](const FTMOPControlBinding& B) { return B.Action == Action; });
}

FTMOPControlBinding* UTMOPControlSettingsSubsystem::FindMutableBinding(
    const int32 PlayerIndex, const ETMOPControlAction Action)
{
    if (!Profiles.IsValidIndex(PlayerIndex)) return nullptr;
    return Profiles[PlayerIndex].Bindings.FindByPredicate(
        [Action](const FTMOPControlBinding& B) { return B.Action == Action; });
}

FKey UTMOPControlSettingsSubsystem::GetKey(const int32 PlayerIndex,
    const ETMOPControlAction Action, const bool bSecondary) const
{
    const FTMOPControlBinding* Binding = FindBinding(PlayerIndex, Action);
    if (!Binding && !bSecondary && Action == ETMOPControlAction::TimelineCursor && PlayerIndex == 0 &&
        GetProfile(PlayerIndex).Device == ETMOPControlDevice::KeyboardMouse) return EKeys::LeftAlt;
    return Binding ? (bSecondary ? Binding->SecondaryKey : Binding->PrimaryKey) : FKey();
}

FText UTMOPControlSettingsSubsystem::GetKeyDisplayText(const int32 PlayerIndex,
    const ETMOPControlAction Action) const
{
    const FTMOPControlBinding* Binding = FindBinding(PlayerIndex, Action);
    if (!Binding) return NSLOCTEXT("TMOP", "ControlUnbound", "Ej bunden");
    const FText Primary = Binding->PrimaryKey.IsValid()
        ? Binding->PrimaryKey.GetDisplayName(false)
        : NSLOCTEXT("TMOP", "ControlUnbound", "Ej bunden");
    return Binding->SecondaryKey.IsValid()
        ? FText::Format(NSLOCTEXT("TMOP", "TwoControlKeys", "{0} / {1}"),
            Primary, Binding->SecondaryKey.GetDisplayName(false)) : Primary;
}

bool UTMOPControlSettingsSubsystem::HasConflict(const int32 PlayerIndex,
    const ETMOPControlAction Action, const FKey Key, FText& OutError) const
{
    if (!Key.IsValid()) return false; // Explicitly clearing a binding is supported.
    if (IsReservedSystemKey(Key) ||
        Key.IsGamepadKey() == IsKeyboardDevice(Profiles[PlayerIndex].Device) ||
        (Profiles[PlayerIndex].Device == ETMOPControlDevice::KeyboardOnly && Key.IsMouseButton()))
    {
        OutError = NSLOCTEXT("TMOP", "InvalidControlKey", "Den tangenten kan inte användas.");
        return true;
    }
    const ETMOPBindingContext WantedContext = ContextFor(Action);
    for (int32 OtherPlayer = 0; OtherPlayer < Profiles.Num(); ++OtherPlayer)
    {
        const bool bCrossKeyboardPlayer = OtherPlayer != PlayerIndex &&
            IsKeyboardDevice(Profiles[PlayerIndex].Device) &&
            IsKeyboardDevice(Profiles[OtherPlayer].Device);
        if (OtherPlayer != PlayerIndex && !bCrossKeyboardPlayer) continue;
        for (const FTMOPControlBinding& Existing : Profiles[OtherPlayer].Bindings)
        {
            if (OtherPlayer == PlayerIndex && Existing.Action == Action) continue;
            if (OtherPlayer == PlayerIndex && IsOppositeAxisPair(Action, Existing.Action, Key)) continue;
            const ETMOPBindingContext ExistingContext = ContextFor(Existing.Action);
            const bool bSameLiveContext = WantedContext == ExistingContext ||
                WantedContext == ETMOPBindingContext::Global ||
                ExistingContext == ETMOPBindingContext::Global ||
                (WantedContext == ETMOPBindingContext::SharedGameplay && ExistingContext != ETMOPBindingContext::Menu) ||
                (ExistingContext == ETMOPBindingContext::SharedGameplay && WantedContext != ETMOPBindingContext::Menu);
            if ((bCrossKeyboardPlayer || bSameLiveContext) &&
                (Existing.PrimaryKey == Key || Existing.SecondaryKey == Key))
            {
                OutError = FText::Format(NSLOCTEXT("TMOP", "ControlConflict",
                    "{0} används redan av spelare {1}: {2}."), Key.GetDisplayName(false),
                    FText::AsNumber(OtherPlayer + 1), GetActionDisplayName(Existing.Action));
                return true;
            }
        }
    }
    // Inactive keyboard profiles retain their reservation when controller layouts are used.
    for (const auto& P : DeviceProfiles)
    {
        if (P.PlayerIndex == PlayerIndex || !IsKeyboardDevice(P.Device) ||
            !IsKeyboardDevice(Profiles[PlayerIndex].Device)) continue;
        for (const auto& B : P.Bindings)
            if (B.PrimaryKey == Key || B.SecondaryKey == Key)
            {
                OutError = FText::Format(NSLOCTEXT("TMOP", "ReservedKeyboardKey",
                    "{0} är reserverad i spelare {1}:s sparade tangentbordsprofil."),
                    Key.GetDisplayName(false), FText::AsNumber(P.PlayerIndex + 1));
                return true;
            }
    }
    return false;
}

bool UTMOPControlSettingsSubsystem::Rebind(const int32 PlayerIndex,
    const ETMOPControlAction Action, const bool bSecondary, const FKey NewKey,
    FText& OutError)
{
    if (!Profiles.IsValidIndex(PlayerIndex) || HasConflict(PlayerIndex, Action, NewKey, OutError))
        return false;
    FTMOPControlBinding* Binding = FindMutableBinding(PlayerIndex, Action);
    if (!Binding)
    {
        Binding = &Profiles[PlayerIndex].Bindings.AddDefaulted_GetRef();
        Binding->Action = Action;
    }
    if (bSecondary) Binding->SecondaryKey = NewKey;
    else Binding->PrimaryKey = NewKey;
    SaveSettings();
    ClearHeldInput(PlayerIndex);
    return true;
}

void UTMOPControlSettingsSubsystem::RememberProfile(const FTMOPPlayerControlProfile& Profile)
{
    auto* Existing = DeviceProfiles.FindByPredicate([&Profile](const auto& P)
        { return P.PlayerIndex == Profile.PlayerIndex && P.Device == Profile.Device; });
    if (Existing) *Existing = Profile;
    else DeviceProfiles.Add(Profile);
}

bool UTMOPControlSettingsSubsystem::ApplyProfile(const FTMOPPlayerControlProfile& Profile, FText& OutError)
{
    const int32 Index = Profile.PlayerIndex;
    if (!Profiles.IsValidIndex(Index)) return false;
    const auto Previous = Profiles[Index];
    Profiles[Index] = Profile;
    for (const auto& B : Profile.Bindings)
        if (HasConflict(Index, B.Action, B.PrimaryKey, OutError) ||
            HasConflict(Index, B.Action, B.SecondaryKey, OutError))
        {
            Profiles[Index] = Previous;
            return false;
        }
    RememberProfile(Previous);
    SaveSettings();
    ClearHeldInput(Index);
    return true;
}

bool UTMOPControlSettingsSubsystem::SetDevice(const int32 PlayerIndex,
    const ETMOPControlDevice Device, FText& OutError)
{
    if (!Profiles.IsValidIndex(PlayerIndex))
    {
        OutError = NSLOCTEXT("TMOP", "InvalidControlPlayer", "Ogiltig spelarplats.");
        return false;
    }
    if ((PlayerIndex == 0 && Device == ETMOPControlDevice::KeyboardOnly) ||
        (PlayerIndex == 1 && Device == ETMOPControlDevice::KeyboardMouse) ||
        (PlayerIndex == 1 && Device == ETMOPControlDevice::KeyboardOnly &&
            ActivePlayerCount != 2) ||
        (PlayerIndex >= 2 && Device != ETMOPControlDevice::Gamepad))
    {
        OutError = NSLOCTEXT("TMOP", "UnsupportedControlDevice",
            "Musen tillhör spelare 1. Delat tangentbord stöds för spelare 1-2; spelare 3-4 använder handkontroller.");
        return false;
    }
    if (Profiles[PlayerIndex].Device == Device) return true;
    const auto* Saved = DeviceProfiles.FindByPredicate([PlayerIndex, Device](const auto& P)
        { return P.PlayerIndex == PlayerIndex && P.Device == Device; });
    const auto Candidate = Saved ? *Saved : MakeDefaultProfile(PlayerIndex, Device);
    if ((PlayerIndex == 0 && Device == ETMOPControlDevice::Gamepad && ActivePlayerCount > 1 &&
        Profiles[1].Device == ETMOPControlDevice::KeyboardOnly) ||
        (PlayerIndex == 1 && Device == ETMOPControlDevice::KeyboardOnly &&
        Profiles[0].Device != ETMOPControlDevice::KeyboardMouse))
    {
        OutError = NSLOCTEXT("TMOP", "MixedKeyboardOrder",
            "Delat tangentbord kräver tangentbord för spelare 1. Byt spelare 2 till handkontroll först när ni lämnar det upplägget.");
        return false;
    }
    if (!ApplyProfile(Candidate, OutError)) return false;
    if (auto* Session = GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>())
        Session->UpdateControlLayoutFromProfiles();
    return true;
}

void UTMOPControlSettingsSubsystem::SetCameraSettings(const int32 PlayerIndex,
    const float SensitivityX, const float SensitivityY, const bool bInvertY,
    const float ZoomFov)
{
    if (!Profiles.IsValidIndex(PlayerIndex)) return;
    FTMOPPlayerControlProfile& P = Profiles[PlayerIndex];
    P.LookSensitivityX = FMath::Clamp(SensitivityX, 0.1f, 3.0f);
    P.LookSensitivityY = FMath::Clamp(SensitivityY, 0.1f, 3.0f);
    P.bInvertLookY = bInvertY;
    P.CameraZoomFov = FMath::Clamp(ZoomFov, 20.0f, 80.0f);
    SaveSettings();
}

void UTMOPControlSettingsSubsystem::SetPhysicalKeyDown(const FKey Key, const bool bDown)
{
    if (!Key.IsValid()) return;
    if (bDown) PressedKeys.Add(Key);
    else
    {
        PressedKeys.Remove(Key);
        ReleaseState.ReleaseKey(Key);
    }
}

void UTMOPControlSettingsSubsystem::ClearHeldInput(const int32 PlayerIndex)
{
    for (int32 Index = 0; Index < Profiles.Num(); ++Index)
    {
        if (PlayerIndex != INDEX_NONE && Index != PlayerIndex) continue;
        const auto* GI = GetGameInstance();
        const ULocalPlayer* Local = GI && GI->GetLocalPlayers().IsValidIndex(Index)
            ? GI->GetLocalPlayers()[Index] : nullptr;
        const auto* PC = Local ? Local->GetPlayerController(GetWorld()) : nullptr;
        for (const auto& B : Profiles[Index].Bindings)
            for (const FKey Key : {B.PrimaryKey, B.SecondaryKey})
            {
                const bool bHeld = IsKeyboardDevice(Profiles[Index].Device)
                    ? PressedKeys.Contains(Key)
                    : PC && (PC->IsInputKeyDown(Key) || FMath::Abs(PC->GetInputAnalogKeyState(Key)) > 0.25f);
                if (bHeld) ReleaseState.Hold(Index, Key);
            }
    }
}

void UTMOPControlSettingsSubsystem::ReleasePhysicalInput()
{
    PressedKeys.Reset();
    ReleaseState.Reset();
}

int32 UTMOPControlSettingsSubsystem::GetKeyboardOwnerForKey(const FKey Key) const
{
    for (int32 PlayerIndex = 0;
        PlayerIndex < FMath::Min(ActivePlayerCount, Profiles.Num()); ++PlayerIndex)
    {
        if (!IsKeyboardDevice(Profiles[PlayerIndex].Device)) continue;
        for (const FTMOPControlBinding& Binding : Profiles[PlayerIndex].Bindings)
            if (Binding.PrimaryKey == Key || Binding.SecondaryKey == Key)
                return PlayerIndex;
    }
    return INDEX_NONE;
}

uint32 UTMOPControlSettingsSubsystem::GetSlateUserForPlayer(const int32 PlayerIndex) const
{
    const UGameInstance* GI = GetGameInstance();
    if (!GI || !GI->GetLocalPlayers().IsValidIndex(PlayerIndex)) return MAX_uint32;
    const ULocalPlayer* Local = GI->GetLocalPlayers()[PlayerIndex];
    return Local && Local->GetSlateUser().IsValid()
        ? Local->GetSlateUser()->GetUserIndex() : MAX_uint32;
}

float UTMOPControlSettingsSubsystem::GetActionValue(const APlayerController* PC,
    const int32 PlayerIndex, const ETMOPControlAction Action) const
{
    if (!Profiles.IsValidIndex(PlayerIndex) || !PC) return 0.0f;
    if (bBindingCaptureActive && GetSlateUserForPlayer(PlayerIndex) == BindingCaptureSlateUser) return 0.0f;
    const FTMOPControlBinding* Binding = FindBinding(PlayerIndex, Action);
    if (!Binding) return 0.0f;
    auto ReadKey = [this, PC, PlayerIndex](const FKey Key)
    {
        if (!Key.IsValid()) return 0.0f;
        if (Profiles[PlayerIndex].Device == ETMOPControlDevice::KeyboardMouse &&
            (Key == EKeys::MouseX || Key == EKeys::MouseY))
        {
            float X = 0.0f, Y = 0.0f;
            PC->GetInputMouseDelta(X, Y);
            return Key == EKeys::MouseX ? X : -Y;
        }
        if (Profiles[PlayerIndex].Device == ETMOPControlDevice::KeyboardMouse &&
            (Key == EKeys::MouseScrollUp || Key == EKeys::MouseScrollDown))
            return PC->WasInputKeyJustPressed(Key) ? 1.0f : 0.0f;
        const float Analog = IsKeyboardDevice(Profiles[PlayerIndex].Device)
            ? (PressedKeys.Contains(Key) ? 1.0f : 0.0f) : PC->GetInputAnalogKeyState(Key);
        const float Value = IsKeyboardDevice(Profiles[PlayerIndex].Device) ? Analog
            : (FMath::Abs(Analog) > 0.01f ? Analog : (PC->IsInputKeyDown(Key) ? 1.0f : 0.0f));
        if (ReleaseState.IsSuppressed(PlayerIndex, Key, FMath::Abs(Value) > 0.25f)) return 0.0f;
        return Value;
    };
    const float Primary = ReadKey(Binding->PrimaryKey);
    const float Secondary = ReadKey(Binding->SecondaryKey);
    return FMath::Abs(Secondary) > FMath::Abs(Primary) ? Secondary : Primary;
}

bool UTMOPControlSettingsSubsystem::IsActionDown(const APlayerController* PC,
    const int32 PlayerIndex, const ETMOPControlAction Action) const
{
    return FMath::Abs(GetActionValue(PC, PlayerIndex, Action)) > 0.25f;
}

bool UTMOPControlSettingsSubsystem::LoadSettings()
{
    if (!UGameplayStatics::DoesSaveGameExist(ControlSettingsSlot, 0)) return false;
    const UTMOPControlSettingsSaveGame* Save = Cast<UTMOPControlSettingsSaveGame>(
        UGameplayStatics::LoadGameFromSlot(ControlSettingsSlot, 0));
    if (!Save || Save->FormatVersion < 1 || Save->FormatVersion > 2 || Save->Profiles.Num() != 4) return false;
    Profiles = Save->Profiles;
    DeviceProfiles = Save->DeviceProfiles;
    for (int32 Index = 0; Index < Profiles.Num(); ++Index)
        if (Profiles[Index].PlayerIndex != Index || Profiles[Index].Bindings.IsEmpty()) return false;
    // New actions are appended without discarding existing per-device settings.
    auto AddMissingActions = [this](FTMOPPlayerControlProfile& P)
    {
        const auto Defaults = MakeDefaultProfile(P.PlayerIndex, P.Device);
        for (const auto& B : Defaults.Bindings)
            if (!P.Bindings.ContainsByPredicate([&B](const auto& Existing) { return Existing.Action == B.Action; }))
            {
                auto Added = B;
                if ((B.Action == ETMOPControlAction::VehicleTakeover || B.Action == ETMOPControlAction::TimelineCursor) &&
                    P.Bindings.ContainsByPredicate([&B](const auto& Existing) {
                        return Existing.PrimaryKey == B.PrimaryKey || Existing.SecondaryKey == B.PrimaryKey; }))
                    Added.PrimaryKey = FKey();
                P.Bindings.Add(Added);
            }
    };
    for (auto& P : Profiles) AddMissingActions(P);
    for (auto& P : DeviceProfiles) AddMissingActions(P);
    // Repair the two known v1 default collisions without deleting unrelated custom keys.
    if (Save->FormatVersion == 1)
        for (auto& P : Profiles)
            if (P.Device == ETMOPControlDevice::Gamepad)
                for (auto& B : P.Bindings)
                    if ((B.Action == ETMOPControlAction::ExtraSprint && B.PrimaryKey == EKeys::Gamepad_RightThumbstick) ||
                        (B.Action == ETMOPControlAction::LookZoom && B.PrimaryKey == EKeys::Gamepad_LeftTrigger))
                        B.PrimaryKey = FKey();
    for (const auto& P : Profiles) RememberProfile(P);
    for (int32 Index = 0; Index < 2; ++Index)
        for (const auto Device : {Index == 0 ? ETMOPControlDevice::KeyboardMouse : ETMOPControlDevice::KeyboardOnly,
            ETMOPControlDevice::Gamepad})
            if (!DeviceProfiles.ContainsByPredicate([Index, Device](const auto& P)
                { return P.PlayerIndex == Index && P.Device == Device; }))
                RememberProfile(MakeDefaultProfile(Index, Device));
    // Resolve legacy cross-player collisions deterministically, preserving P1.
    const auto* P1 = DeviceProfiles.FindByPredicate([](const auto& P)
        { return P.PlayerIndex == 0 && P.Device == ETMOPControlDevice::KeyboardMouse; });
    auto* P2 = DeviceProfiles.FindByPredicate([](const auto& P)
        { return P.PlayerIndex == 1 && P.Device == ETMOPControlDevice::KeyboardOnly; });
    if (P1 && P2)
        for (auto& B : P2->Bindings)
            for (FKey* Key : {&B.PrimaryKey, &B.SecondaryKey})
                if (Key->IsValid() && P1->Bindings.ContainsByPredicate([Key](const auto& Other)
                    { return Other.PrimaryKey == *Key || Other.SecondaryKey == *Key; }))
                    *Key = FKey();
    if (P2 && Profiles[1].Device == P2->Device) Profiles[1] = *P2;
    return true;
}

bool UTMOPControlSettingsSubsystem::SaveSettings()
{
    UTMOPControlSettingsSaveGame* Save = Cast<UTMOPControlSettingsSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UTMOPControlSettingsSaveGame::StaticClass()));
    if (!Save) return false;
    for (const auto& P : Profiles) RememberProfile(P);
    Save->Profiles = Profiles;
    Save->DeviceProfiles = DeviceProfiles;
    for (int32 Index = 0; Index < 4; ++Index)
        if (InstalledNavigation[Index]) SetMenuNavigation(Index, true);
    return UGameplayStatics::SaveGameToSlot(Save, ControlSettingsSlot, 0);
}

void UTMOPControlSettingsSubsystem::SetMenuNavigation(const int32 PlayerIndex, const bool bEnabled)
{
    if (PlayerIndex < 0 || PlayerIndex >= 4) return;
    if (!bEnabled)
    {
        if (auto User = NavigationUsers[PlayerIndex].Pin())
            if (User->GetUserNavigationConfig() == InstalledNavigation[PlayerIndex])
                User->SetUserNavigationConfig(PreviousNavigation[PlayerIndex]);
        NavigationUsers[PlayerIndex].Reset();
        PreviousNavigation[PlayerIndex].Reset();
        InstalledNavigation[PlayerIndex].Reset();
        return;
    }
    UGameInstance* GI = GetGameInstance();
    if (!GI || !GI->GetLocalPlayers().IsValidIndex(PlayerIndex)) return;
    ULocalPlayer* Local = GI->GetLocalPlayers()[PlayerIndex];
    TSharedPtr<FSlateUser> User = Local ? Local->GetSlateUser() : nullptr;
    if (!User) return;
    if (NavigationUsers[PlayerIndex].Pin() != User)
    {
        SetMenuNavigation(PlayerIndex, false);
        NavigationUsers[PlayerIndex] = User;
        PreviousNavigation[PlayerIndex] = User->GetUserNavigationConfig();
    }
    auto Config = MakeShared<FNavigationConfig>();
    Config->KeyEventRules.Reset();
    Config->KeyActionRules.Reset();
    Config->bTabNavigation = false;
    Config->bIgnoreModifiersForNavigationActions = true;
    Config->AnalogHorizontalKey = FKey();
    Config->AnalogVerticalKey = FKey();
    auto Direction = [this, PlayerIndex, &Config](ETMOPControlAction Action, EUINavigation Nav)
    {
        for (bool bSecondary : {false, true})
        {
            const FKey Key = GetKey(PlayerIndex, Action, bSecondary);
            if (!Key.IsValid()) continue;
            if (!Key.IsAxis1D()) Config->KeyEventRules.Add(Key, Nav);
            else if (Nav == EUINavigation::Left || Nav == EUINavigation::Right) Config->AnalogHorizontalKey = Key;
            else Config->AnalogVerticalKey = Key;
        }
    };
    Direction(ETMOPControlAction::MenuUp, EUINavigation::Up);
    Direction(ETMOPControlAction::MenuDown, EUINavigation::Down);
    Direction(ETMOPControlAction::MenuLeft, EUINavigation::Left);
    Direction(ETMOPControlAction::MenuRight, EUINavigation::Right);
    for (bool bSecondary : {false, true})
    {
        const FKey Confirm = GetKey(PlayerIndex, ETMOPControlAction::MenuConfirm, bSecondary);
        const FKey Back = GetKey(PlayerIndex, ETMOPControlAction::MenuBack, bSecondary);
        if (Confirm.IsValid()) Config->KeyActionRules.Add(Confirm, EUINavigationAction::Accept);
        if (Back.IsValid()) Config->KeyActionRules.Add(Back, EUINavigationAction::Back);
    }
    InstalledNavigation[PlayerIndex] = Config;
    User->SetUserNavigationConfig(Config);
}

FText UTMOPControlSettingsSubsystem::GetActionDisplayName(const ETMOPControlAction Action)
{
    // Runtime labels must not depend on editor-only enum display metadata.
    switch (Action)
    {
    case ETMOPControlAction::MoveForward: return NSLOCTEXT("TMOP", "ControlAction.MoveForward", "Gå framåt");
    case ETMOPControlAction::MoveBackward: return NSLOCTEXT("TMOP", "ControlAction.MoveBackward", "Gå bakåt");
    case ETMOPControlAction::MoveLeft: return NSLOCTEXT("TMOP", "ControlAction.MoveLeft", "Gå åt vänster");
    case ETMOPControlAction::MoveRight: return NSLOCTEXT("TMOP", "ControlAction.MoveRight", "Gå åt höger");
    case ETMOPControlAction::LookUp: return NSLOCTEXT("TMOP", "ControlAction.LookUp", "Titta upp");
    case ETMOPControlAction::LookDown: return NSLOCTEXT("TMOP", "ControlAction.LookDown", "Titta ned");
    case ETMOPControlAction::LookLeft: return NSLOCTEXT("TMOP", "ControlAction.LookLeft", "Titta åt vänster");
    case ETMOPControlAction::LookRight: return NSLOCTEXT("TMOP", "ControlAction.LookRight", "Titta åt höger");
    case ETMOPControlAction::Jump: return NSLOCTEXT("TMOP", "ControlAction.Jump", "Hoppa");
    case ETMOPControlAction::Sprint: return NSLOCTEXT("TMOP", "ControlAction.Sprint", "Spring");
    case ETMOPControlAction::ExtraSprint: return NSLOCTEXT("TMOP", "ControlAction.ExtraSprint", "Extra sprint");
    case ETMOPControlAction::Interact: return NSLOCTEXT("TMOP", "ControlAction.Interact", "Interagera");
    case ETMOPControlAction::PrimaryAction: return NSLOCTEXT("TMOP", "ControlAction.PrimaryAction", "Primär handling");
    case ETMOPControlAction::SecondaryAction: return NSLOCTEXT("TMOP", "ControlAction.SecondaryAction", "Sekundär handling");
    case ETMOPControlAction::Cancel: return NSLOCTEXT("TMOP", "ControlAction.Cancel", "Avbryt");
    case ETMOPControlAction::Crouch: return NSLOCTEXT("TMOP", "ControlAction.Crouch", "Huka");
    case ETMOPControlAction::Kick: return NSLOCTEXT("TMOP", "ControlAction.Kick", "Sparka");
    case ETMOPControlAction::ShoulderSwap: return NSLOCTEXT("TMOP", "ControlAction.ShoulderSwap", "Byt kameraaxel");
    case ETMOPControlAction::Pause: return NSLOCTEXT("TMOP", "ControlAction.Pause", "Pausa");
    case ETMOPControlAction::WorldMap: return NSLOCTEXT("TMOP", "ControlAction.WorldMap", "Karta");
    case ETMOPControlAction::QuickInventory: return NSLOCTEXT("TMOP", "ControlAction.QuickInventory", "Snabbinventarie");
    case ETMOPControlAction::InventoryPrevious: return NSLOCTEXT("TMOP", "ControlAction.InventoryPrevious", "Föregående föremål");
    case ETMOPControlAction::InventoryNext: return NSLOCTEXT("TMOP", "ControlAction.InventoryNext", "Nästa föremål");
    case ETMOPControlAction::DropItem: return NSLOCTEXT("TMOP", "ControlAction.DropItem", "Släpp föremål");
    case ETMOPControlAction::LookZoom: return NSLOCTEXT("TMOP", "ControlAction.LookZoom", "Tittzoom");
    case ETMOPControlAction::TogglePerspective: return NSLOCTEXT("TMOP", "ControlAction.TogglePerspective", "Byt perspektiv");
    case ETMOPControlAction::VehicleAccelerate: return NSLOCTEXT("TMOP", "ControlAction.VehicleAccelerate", "Gasa");
    case ETMOPControlAction::VehicleReverse: return NSLOCTEXT("TMOP", "ControlAction.VehicleReverse", "Backa");
    case ETMOPControlAction::VehicleLeft: return NSLOCTEXT("TMOP", "ControlAction.VehicleLeft", "Styr åt vänster");
    case ETMOPControlAction::VehicleRight: return NSLOCTEXT("TMOP", "ControlAction.VehicleRight", "Styr åt höger");
    case ETMOPControlAction::VehicleBrake: return NSLOCTEXT("TMOP", "ControlAction.VehicleBrake", "Bromsa");
    case ETMOPControlAction::VehicleHandbrake: return NSLOCTEXT("TMOP", "ControlAction.VehicleHandbrake", "Handbroms");
    case ETMOPControlAction::VehicleExit: return NSLOCTEXT("TMOP", "ControlAction.VehicleExit", "Lämna fordon");
    case ETMOPControlAction::VehicleHighSpeed: return NSLOCTEXT("TMOP", "ControlAction.VehicleHighSpeed", "Fordon: hög hastighet");
    case ETMOPControlAction::MenuUp: return NSLOCTEXT("TMOP", "ControlAction.MenuUp", "Meny: upp");
    case ETMOPControlAction::MenuDown: return NSLOCTEXT("TMOP", "ControlAction.MenuDown", "Meny: ned");
    case ETMOPControlAction::MenuLeft: return NSLOCTEXT("TMOP", "ControlAction.MenuLeft", "Meny: vänster");
    case ETMOPControlAction::MenuRight: return NSLOCTEXT("TMOP", "ControlAction.MenuRight", "Meny: höger");
    case ETMOPControlAction::MenuConfirm: return NSLOCTEXT("TMOP", "ControlAction.MenuConfirm", "Meny: bekräfta");
    case ETMOPControlAction::MenuZoomIn: return NSLOCTEXT("TMOP", "ControlAction.MenuZoomIn", "Meny: zooma in");
    case ETMOPControlAction::MenuZoomOut: return NSLOCTEXT("TMOP", "ControlAction.MenuZoomOut", "Meny: zooma ut");
    case ETMOPControlAction::MenuReset: return NSLOCTEXT("TMOP", "ControlAction.MenuReset", "Meny: återställ");
    case ETMOPControlAction::MenuPreviousPage: return NSLOCTEXT("TMOP", "ControlAction.MenuPreviousPage", "Meny: föregående sida");
    case ETMOPControlAction::MenuNextPage: return NSLOCTEXT("TMOP", "ControlAction.MenuNextPage", "Meny: nästa sida");
    case ETMOPControlAction::MenuBack: return NSLOCTEXT("TMOP", "ControlAction.MenuBack", "Meny: tillbaka");
    case ETMOPControlAction::VehicleTakeover: return NSLOCTEXT("TMOP", "ControlAction.VehicleTakeover", "Ta över fordon");
    case ETMOPControlAction::TimelineCursor: return NSLOCTEXT("TMOP", "ControlAction.TimelineCursor", "Visa markör för tidslinjen");
    default: return FText::GetEmpty();
    }
}
