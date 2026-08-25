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
class UTMOPNewspaperReaderWidget;
class UTMOPNewspaperItemDefinition;
class UTMOPNewspaperReadingComponent;
class UTMOPItemDefinition;
class ATMOPHistoricalAgent;
class UTMOPVehicleTakeoverComponent;
class UTMOPPlayerVehicleDrivingComponent;
class UTMOPPlayerVehicleSessionComponent;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Inventory|Drop")
    TSubclassOf<ATMOPWorldItem> WorldItemClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Inventory|Drop")
    float DropForwardDistance = 110.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Interaction")
    AActor* FindInteractionTarget() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Player|Interaction")
    FText GetInteractKeyDisplayText() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Pause")
    void SetPauseMenuOpen(bool bOpen);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Pause")
    void TogglePauseMenu();

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

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Newspaper")
    bool OpenNewspaper(UTMOPNewspaperItemDefinition* Newspaper);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|UI|Newspaper")
    void CloseNewspaper();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Inventory")
    bool DropEquippedItem();

private:
    void InitializePlayerInterface();
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
    TWeakObjectPtr<ATMOPHistoricalAgent> ActiveDialogAgent;
    UPROPERTY(Transient)
    TObjectPtr<ACameraActor> DialogCameraActor;
    TWeakObjectPtr<AActor> PreDialogViewTarget;
};
