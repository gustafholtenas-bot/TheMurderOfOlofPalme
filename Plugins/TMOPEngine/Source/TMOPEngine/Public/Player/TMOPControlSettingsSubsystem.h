#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "InputCoreTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Player/TMOPInputReleasePolicy.h"
#include "TMOPControlSettingsSubsystem.generated.h"

class APlayerController;
class FTMOPControlInputProcessor;
class FNavigationConfig;
class FSlateUser;

UENUM(BlueprintType)
enum class ETMOPControlDevice : uint8
{
    KeyboardMouse UMETA(DisplayName="Keyboard and mouse"),
    KeyboardOnly UMETA(DisplayName="Keyboard only"),
    Gamepad UMETA(DisplayName="Gamepad")
};

UENUM(BlueprintType)
enum class ETMOPControlAction : uint8
{
    MoveForward, MoveBackward, MoveLeft, MoveRight,
    LookUp, LookDown, LookLeft, LookRight,
    Jump, Sprint, ExtraSprint, Interact, PrimaryAction, SecondaryAction,
    Cancel, Crouch, Kick, ShoulderSwap, Pause, WorldMap,
    QuickInventory, InventoryPrevious, InventoryNext, DropItem,
    LookZoom, TogglePerspective,
    VehicleAccelerate, VehicleReverse, VehicleLeft, VehicleRight,
    VehicleBrake, VehicleHandbrake, VehicleExit, VehicleHighSpeed,
    MenuUp, MenuDown, MenuLeft, MenuRight, MenuConfirm,
    MenuZoomIn, MenuZoomOut, MenuReset, MenuPreviousPage, MenuNextPage, MenuBack
};

USTRUCT(BlueprintType)
struct FTMOPControlBinding
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    ETMOPControlAction Action = ETMOPControlAction::MoveForward;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    FKey PrimaryKey;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    FKey SecondaryKey;
};

USTRUCT(BlueprintType)
struct FTMOPPlayerControlProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    int32 PlayerIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    ETMOPControlDevice Device = ETMOPControlDevice::Gamepad;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    TArray<FTMOPControlBinding> Bindings;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    float LookSensitivityX = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    float LookSensitivityY = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    bool bInvertLookY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame)
    float CameraZoomFov = 40.0f;
};

UCLASS()
class TMOPENGINE_API UTMOPControlSettingsSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame)
    int32 FormatVersion = 2;

    UPROPERTY(SaveGame)
    TArray<FTMOPPlayerControlProfile> Profiles;

    UPROPERTY(SaveGame)
    TArray<FTMOPPlayerControlProfile> DeviceProfiles;
};

/** Persistent input profiles plus a shared-keyboard input router for local players 1-4. */
UCLASS()
class TMOPENGINE_API UTMOPControlSettingsSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()
public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintPure, Category="TMOP|Controls")
    FTMOPPlayerControlProfile GetProfile(int32 PlayerIndex) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    bool SetDevice(int32 PlayerIndex, ETMOPControlDevice Device, FText& OutError);

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    bool Rebind(int32 PlayerIndex, ETMOPControlAction Action, bool bSecondary,
        FKey NewKey, FText& OutError);

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    void ResetPlayerToDefaults(int32 PlayerIndex);

    bool TryResetPlayerToDefaults(int32 PlayerIndex, FText& OutError);

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    void ResetAllToDefaults();

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    void ConfigureForSession(int32 PlayerCount, bool bKeyboardMode,
        bool bSharedKeyboardForPlayerTwo);

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    void SetCameraSettings(int32 PlayerIndex, float SensitivityX,
        float SensitivityY, bool bInvertY, float ZoomFov);

    UFUNCTION(BlueprintPure, Category="TMOP|Controls")
    FKey GetKey(int32 PlayerIndex, ETMOPControlAction Action,
        bool bSecondary = false) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Controls")
    FText GetKeyDisplayText(int32 PlayerIndex, ETMOPControlAction Action) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Controls")
    bool IsActionDown(const APlayerController* PlayerController, int32 PlayerIndex,
        ETMOPControlAction Action) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Controls")
    float GetActionValue(const APlayerController* PlayerController, int32 PlayerIndex,
        ETMOPControlAction Action) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Controls")
    bool SaveSettings();

    void SetPhysicalKeyDown(FKey Key, bool bDown);
    void ClearHeldInput(int32 PlayerIndex = INDEX_NONE);
    void BeginBindingCapture(uint32 SlateUserIndex)
    { bBindingCaptureActive = true; BindingCaptureSlateUser = SlateUserIndex; }
    void EndBindingCapture()
    {
        for (int32 Index = 0; Index < 4; ++Index)
            if (GetSlateUserForPlayer(Index) == BindingCaptureSlateUser) ClearHeldInput(Index);
        bBindingCaptureActive = false;
        BindingCaptureSlateUser = MAX_uint32;
    }
    bool IsBindingCaptureActive() const { return bBindingCaptureActive; }
    uint32 GetBindingCaptureSlateUser() const { return BindingCaptureSlateUser; }
    int32 GetKeyboardOwnerForKey(FKey Key) const;
    uint32 GetSlateUserForPlayer(int32 PlayerIndex) const;
    int32 GetActivePlayerCount() const { return ActivePlayerCount; }
    static FText GetActionDisplayName(ETMOPControlAction Action);
    static bool IsKeyboardDevice(ETMOPControlDevice Device);
    void ReleasePhysicalInput();
    void SetMenuNavigation(int32 PlayerIndex, bool bEnabled);

private:
    UPROPERTY(Transient)
    TArray<FTMOPPlayerControlProfile> Profiles;

    UPROPERTY(Transient)
    TArray<FTMOPPlayerControlProfile> DeviceProfiles;

    TSet<FKey> PressedKeys;
    TSharedPtr<FTMOPControlInputProcessor> InputProcessor;
    int32 ActivePlayerCount = 1;
    mutable TMOPInputReleasePolicy::TState<FKey> ReleaseState;
    TWeakPtr<FSlateUser> NavigationUsers[4];
    TSharedPtr<FNavigationConfig> PreviousNavigation[4];
    TSharedPtr<FNavigationConfig> InstalledNavigation[4];
    bool bBindingCaptureActive = false;
    uint32 BindingCaptureSlateUser = MAX_uint32;

    FTMOPPlayerControlProfile MakeDefaultProfile(int32 PlayerIndex,
        ETMOPControlDevice Device) const;
    const FTMOPControlBinding* FindBinding(int32 PlayerIndex,
        ETMOPControlAction Action) const;
    FTMOPControlBinding* FindMutableBinding(int32 PlayerIndex,
        ETMOPControlAction Action);
    bool HasConflict(int32 PlayerIndex, ETMOPControlAction Action,
        FKey Key, FText& OutError) const;
    bool LoadSettings();
    void RememberProfile(const FTMOPPlayerControlProfile& Profile);
    bool ApplyProfile(const FTMOPPlayerControlProfile& Profile, FText& OutError);
};
