#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputCoreTypes.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Items/TMOPPlayerItemUseComponent.h"
#include "Radio/TMOPPlayerRadioComponent.h"
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
class UTMOPVehicleTakeoverComponent;
class UTMOPPlayerVehicleDrivingComponent;
class UTMOPPlayerVehicleSessionComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPlayerCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ATMOPPlayerCharacter();
    virtual void BeginPlay() override;
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

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Player|Input|Inventory")
    TObjectPtr<UInputAction> DropItemAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    bool bUseDirectDropKeyFallback = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input|Inventory")
    FKey DropItemFallbackKey = EKeys::G;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input")
    bool bUseDirectPauseKeyFallback = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Player|Input")
    FKey PauseMenuFallbackKey = EKeys::Enter;

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

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Inventory")
    bool DropEquippedItem();

private:
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

    bool bRightShoulderCamera = true;
    bool bQuickInventoryFallbackHeld = false;
    bool bPauseFallbackHeld = false;
    bool bClockWasRunningBeforePause = false;
    bool bDropFallbackHeld = false;
    bool bInteractFallbackHeld = false;
};
