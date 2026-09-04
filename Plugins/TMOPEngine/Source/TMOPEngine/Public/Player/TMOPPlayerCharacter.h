#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Items/TMOPPlayerItemUseComponent.h"
#include "Radio/TMOPPlayerRadioComponent.h"
#include "Time/TMOPTime.h"
#include "TMOPPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UTMOPAnimationStateComponent;
class UTMOPPlayerActionComponent;
class UTMOPQuickInventoryWidget;
class UTMOPPauseMenuWidget;
class ATMOPWorldItem;
class UTMOPInteractionPromptWidget;
class UTMOPDialogWidget;
class UTMOPAgentInfoChartWidget;
class UTMOPNewspaperReaderWidget;
class UTMOPNewspaperItemDefinition;
class UTMOPNewspaperReadingComponent;
class UTMOPItemDefinition;
class ATMOPHistoricalAgent;
class UTMOPVehicleTakeoverComponent;
class UTMOPPlayerVehicleDrivingComponent;
class UTMOPPlayerVehicleSessionComponent;
class UTMOPCameraPerspectiveComponent;
class UTMOPAgentAudioComponent;
class UTMOPPlayerMovementAudioComponent;
class UTMOPMapComponent;
class ACameraActor;
class UTMOPMapWidget;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATMOPPlayerCharacter();
    virtual void BeginPlay() override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UCameraComponent> FollowCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UTMOPAnimationStateComponent> AnimationState;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UTMOPPlayerActionComponent> PlayerActions;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UTMOPInventoryComponent> Inventory;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UTMOPInventoryInputComponent> InventoryInput;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UTMOPPlayerItemUseComponent> ItemUse;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player")
    TObjectPtr<UTMOPPlayerRadioComponent> Radio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Vehicle")
    TObjectPtr<UTMOPVehicleTakeoverComponent> VehicleTakeover;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Vehicle")
    TObjectPtr<UTMOPPlayerVehicleDrivingComponent> VehicleDriving;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Vehicle")
    TObjectPtr<UTMOPPlayerVehicleSessionComponent> VehicleSession;

    /** V toggles the local player between third- and first-person cameras. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Camera")
    TObjectPtr<UTMOPCameraPerspectiveComponent> CameraPerspective;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Audio")
    TObjectPtr<UTMOPAgentAudioComponent> FootstepAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Audio")
    TObjectPtr<UTMOPPlayerMovementAudioComponent> MovementAudio;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|Map")
    TObjectPtr<UTMOPMapComponent> MapComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Map")
    TSubclassOf<UTMOPMapWidget> WorldMapWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Map")
    TSubclassOf<UTMOPMapWidget> MinimapWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Map")
    bool bCreateMapWidgets = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Map")
    TObjectPtr<UTMOPMapWidget> WorldMapWidget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Map")
    TObjectPtr<UTMOPMapWidget> MinimapWidget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Map")
    bool bWorldMapOpen = false;

    /** Optional visual class. Empty uses the built-in C++ placeholder menu. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI")
    TSubclassOf<UTMOPQuickInventoryWidget> QuickInventoryWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI")
    bool bCreateQuickInventoryWidget = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI")
    TObjectPtr<UTMOPQuickInventoryWidget> QuickInventoryWidget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Pause")
    TSubclassOf<UTMOPPauseMenuWidget> PauseMenuWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Pause")
    bool bCreatePauseMenuWidget = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Pause")
    TObjectPtr<UTMOPPauseMenuWidget> PauseMenuWidget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Pause")
    bool bPauseMenuOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Interaction")
    TSubclassOf<UTMOPInteractionPromptWidget> InteractionPromptWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Interaction")
    bool bCreateInteractionPromptWidget = true;

    /** Ignores stale Blueprint widget settings and guarantees the native target HUD. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Interaction")
    bool bForceNativeTargetInformationWidget = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Interaction")
    TObjectPtr<UTMOPInteractionPromptWidget> InteractionPromptWidget;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Dialog")
    TSubclassOf<UTMOPDialogWidget> DialogWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Dialog")
    bool bCreateDialogWidget = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Dialog")
    TObjectPtr<UTMOPDialogWidget> DialogWidget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Dialog")
    bool bDialogOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Agent Info")
    TSubclassOf<UTMOPAgentInfoChartWidget> AgentInfoChartWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Agent Info")
    bool bCreateAgentInfoChartWidget = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Agent Info")
    TObjectPtr<UTMOPAgentInfoChartWidget> AgentInfoChartWidget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Agent Info")
    bool bAgentInfoChartOpen = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Newspaper")
    TSubclassOf<UTMOPNewspaperReaderWidget> NewspaperReaderWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Newspaper")
    bool bCreateNewspaperReaderWidget = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Newspaper")
    TObjectPtr<UTMOPNewspaperReaderWidget> NewspaperReaderWidget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|UI|Newspaper")
    bool bNewspaperOpen = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|Newspaper")
    TObjectPtr<UTMOPNewspaperReadingComponent> NewspaperReading;

    /** Stable IDs shown in Notebook/Evidence and persisted by the pause-menu save. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|Evidence")
    TArray<FName> DiscoveredEvidenceIds;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Evidence")
    bool DiscoverEvidence(FName EvidenceId);

    /** BeforeShot is used before this time; AfterShot is used at and after it. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Dialog")
    FTMOPTime DialogShotThreshold = FTMOPTime(23, 21, 30);

    /** Dialogue closes automatically if either participant moves too far away. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|UI|Dialog",
        meta=(ClampMin="100.0", Units="cm"))
    float DialogMaximumDistanceCm = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputMappingContext> DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> MoveAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> LookAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> JumpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> SprintAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> InteractAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input")
    bool bUseDirectInteractKeyFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input")
    FKey InteractFallbackKey = EKeys::E;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> PrimaryAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> SecondaryAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> CancelAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> SquatAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> KickAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> ShoulderSwapAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input")
    TObjectPtr<UInputAction> PauseMenuAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Vehicle")
    TObjectPtr<UInputAction> VehicleBrakeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Vehicle")
    TObjectPtr<UInputAction> VehicleHandbrakeAction;

    /** Current IMC_TMOPPlayer sends W/S on X and A/D on Y. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Vehicle")
    bool bSwapVehicleMoveAxes = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Inventory")
    TObjectPtr<UInputAction> DropItemAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    bool bUseDirectDropKeyFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    FKey DropItemFallbackKey = EKeys::G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input")
    bool bUseDirectPauseKeyFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input")
    FKey PauseMenuFallbackKey = EKeys::Enter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Pause")
    FKey PauseMenuGamepadFallbackKey = EKeys::Gamepad_Special_Right;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Map")
    FKey WorldMapFallbackKey = EKeys::M;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Inventory")
    TObjectPtr<UInputAction> QuickInventoryAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Inventory")
    TObjectPtr<UInputAction> InventoryNavigateAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Inventory")
    TObjectPtr<UInputAction> InventoryCycleAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    float WalkSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    float SprintSpeed = 600.0f;

    /** Speed used while both sprint and extra-sprint modifier keys are held. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    float ExtraSprintSpeed = 850.0f;

    /** UE 5.8 fallback if an Enhanced Input Started event is consumed elsewhere. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    bool bUseDirectSprintKeyFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    FKey SprintFallbackKey = EKeys::LeftShift;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    FKey ExtraSprintModifierKey = EKeys::LeftControl;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|Movement")
    bool bIsSprinting = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player|Movement")
    bool bIsExtraSprinting = false;

    /** Direct UE 5.8 fallback for IA_QuickInventory, equivalent to sprint fallback. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    bool bUseDirectQuickInventoryKeyFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    FKey QuickInventoryFallbackKey = EKeys::Tab;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    FKey QuickInventoryGamepadFallbackKey = EKeys::Gamepad_LeftShoulder;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    FKey QuickInventoryPreviousKey = EKeys::Q;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    FKey QuickInventoryNextKey = EKeys::E;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Movement")
    float CrouchSpeed = 170.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera")
    float LookYawSensitivity = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera")
    float LookPitchSensitivity = 0.65f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera")
    bool bInvertLookY = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera")
    float ShoulderOffsetCm = 65.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera")
    float ShoulderSwapSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog")
    bool bEnableDialogCloseUp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog",
        meta=(ClampMin="0.0", Units="cm"))
    float DialogCameraDistanceCm = 185.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog",
        meta=(Units="cm"))
    float DialogCameraSideOffsetCm = 38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog",
        meta=(ClampMin="80.0", ClampMax="220.0", Units="cm"))
    float DialogFaceHeightCm = 160.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog",
        meta=(ClampMin="20.0", ClampMax="90.0"))
    float DialogCameraFieldOfView = 42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog",
        meta=(ClampMin="0.0", ClampMax="3.0", Units="s"))
    float DialogCameraBlendSeconds = 0.45f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Camera|Dialog",
        meta=(ClampMin="1.0"))
    float DialogCameraTrackingSpeed = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Interaction")
    float InteractionDistance = 300.0f;

    /** Maximum distance for identifying a nearby person, vehicle or item. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Target",
        meta=(ClampMin="100.0", Units="cm"))
    float TargetInformationDistance = 900.0f;

    /** A target inside this camera cone counts as something the player looks at. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Target",
        meta=(ClampMin="1.0", ClampMax="45.0", Units="deg"))
    float DirectTargetConeDegrees = 10.0f;

    /** If no direct target exists, use the nearest target in the character's front 180 degrees. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Target")
    bool bUseFrontHemisphereTargetFallback = true;

    /** Vertical point inside a person's bounds used for the on-screen marker. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Target",
        meta=(ClampMin="-0.5", ClampMax="0.5"))
    float PersonTargetMarkerHeightFraction = 0.20f;

    /** Keeps the target marker and its text away from the viewport edges. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Target",
        meta=(ClampMin="0.0"))
    float TargetMarkerScreenEdgePadding = 110.0f;

    /** Maximum off-centre angle for selecting a person or small item. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Interaction",
        meta=(ClampMin="1.0", ClampMax="20.0", Units="deg"))
    float InteractionTargetConeDegrees = 6.0f;

    /** Vehicles use a tighter centre target so visible occupants can be selected separately. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Interaction",
        meta=(ClampMin="1.0", ClampMax="20.0", Units="deg"))
    float VehicleTargetConeDegrees = 4.0f;

    UPROPERTY(Transient, BlueprintReadOnly, Category="TMOP|Player|Interaction")
    TObjectPtr<AActor> CurrentInteractionTarget;

    UPROPERTY(Transient, BlueprintReadOnly, Category="TMOP|Player|Target")
    TObjectPtr<AActor> CurrentInformationTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Inventory|Drop")
    TSubclassOf<ATMOPWorldItem> WorldItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Inventory|Drop")
    float DropForwardDistance = 110.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Interaction")
    AActor* FindInteractionTarget() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Target")
    AActor* FindInformationTarget() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Player|Interaction")
    FText GetInteractKeyDisplayText() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Pause")
    void SetPauseMenuOpen(bool bOpen);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Pause")
    void TogglePauseMenu();

    /**
     * Hides the normal clock/HUD and minimap for a named reason. Multiple
     * systems may hide it simultaneously without one system revealing it too
     * early. Use the same Reason with bShouldHide=false when that system finishes.
     */
    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|HUD")
    void SetGameplayHUDHidden(FName Reason, bool bShouldHide);

    /** Convenience function for Sequencer Event Tracks. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|HUD")
    void SetCinematicHUDHidden(bool bShouldHide);

    UFUNCTION(BlueprintPure, Category="TMOP|Player|UI|HUD")
    bool IsGameplayHUDVisible() const { return bGameplayHUDVisible; }

    /** Use this in a custom Blueprint clock widget if it was added directly to the viewport. */
    UFUNCTION(BlueprintImplementableEvent, Category="TMOP|Player|UI|HUD")
    void OnGameplayHUDVisibilityChanged(bool bVisible);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Player|UI|HUD")
    bool bGameplayHUDVisible = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Map")
    bool OpenWorldMap();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Map")
    void CloseWorldMap();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Map")
    void ToggleWorldMap();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Dialog")
    bool OpenPersonDialog(ATMOPHistoricalAgent* HistoricalAgent);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Dialog")
    void ClosePersonDialog();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Agent Info")
    bool OpenAgentInfoChart(ATMOPHistoricalAgent* HistoricalAgent);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Agent Info")
    void CloseAgentInfoChart();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Newspaper")
    bool OpenNewspaper(UTMOPNewspaperItemDefinition* Newspaper);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Newspaper")
    void CloseNewspaper();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Inventory")
    bool DropEquippedItem();

private:
    void InitializePlayerInterface();
    void UpdateGameplayHUDVisibility();
    void InputMove(const FInputActionValue& Value);
    void InputMoveCompleted();
    void InputLook(const FInputActionValue& Value);
    void InputJumpStarted();
    void InputJumpEnded();
    void InputSprintStarted();
    void InputSprintEnded();
    void InputInteract();
    void InputPrimaryAction();
    void InputSecondaryActionStarted();
    void InputSecondaryActionEnded();
    void InputCancel();
    void InputToggleSquat();
    void InputKick();
    void InputShoulderSwap();
    void InputTogglePauseMenu();
    void InputVehicleBrakeStarted();
    void InputVehicleBrakeEnded();
    void InputVehicleHandbrakeStarted();
    void InputVehicleHandbrakeEnded();
    void InputDropEquippedItem();
    void InputQuickInventoryStarted();
    void InputQuickInventoryCompleted();
    void InputInventoryNavigate(const FInputActionValue& Value);
    void InputInventoryCycle(const FInputActionValue& Value);
    void FinishQuickInventory(bool bConfirm);
    void UpdateQuickInventoryPointer();
    void UpdateInteractionPrompt();
    void SetSprinting(bool bEnabled, bool bExtraSprint = false);
    void BeginDialogCloseUp(ATMOPHistoricalAgent* HistoricalAgent);
    void UpdateDialogCloseUp(float DeltaSeconds);
    void EndDialogCloseUp();

    UFUNCTION()
    void HandleItemMenuRequested(UTMOPItemDefinition* Item);

    bool bRightShoulderCamera = true;
    bool bInputMappingContextAdded = false;
    bool bPlayerInterfaceInitialized = false;
    bool bQuickInventoryFallbackHeld = false;
    bool bPauseFallbackHeld = false;
    bool bMapFallbackHeld = false;
    bool bClockWasRunningBeforeMap = false;
    bool bClockWasRunningBeforePause = false;
    bool bClockWasRunningBeforeNewspaper = false;
    bool bNewspaperPausedSimulation = false;
    bool bDropFallbackHeld = false;
    bool bInteractFallbackHeld = false;
    TSet<FName> GameplayHUDHiddenReasons;
    TWeakObjectPtr<ATMOPHistoricalAgent> ActiveDialogAgent;
    UPROPERTY(Transient)
    TObjectPtr<ACameraActor> DialogCameraActor;
    TWeakObjectPtr<AActor> PreDialogViewTarget;
};
