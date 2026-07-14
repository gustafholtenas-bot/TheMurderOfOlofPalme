#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "TMOPPlayerCharacter.generated.h"

class UCameraComponent;
class UInputAction;
class UInputMappingContext;
class USpringArmComponent;
class UTMOPAnimationStateComponent;
class UTMOPPlayerActionComponent;

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

    UFUNCTION(BlueprintCallable, Category="TMOP|Player|Interaction")
    AActor* FindInteractionTarget() const;

private:
    void InputMove(const FInputActionValue& Value);
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
    void InputQuickInventoryStarted();
    void InputQuickInventoryCompleted();
    void InputInventoryNavigate(const FInputActionValue& Value);
    void InputInventoryCycle(const FInputActionValue& Value);

    bool bRightShoulderCamera = true;
};
