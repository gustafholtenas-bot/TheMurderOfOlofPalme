#include "Player/TMOPPlayerCharacter.h"

#include "Animation/TMOPAnimationStateComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Items/TMOPPlayerItemUseComponent.h"
#include "Items/TMOPInteractable.h"
#include "Items/TMOPWorldItem.h"
#include "Player/TMOPPlayerActionComponent.h"
#include "Player/TMOPVehicleTakeoverComponent.h"
#include "Player/TMOPPlayerVehicleDrivingComponent.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Radio/TMOPPlayerRadioComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "UI/TMOPQuickInventoryWidget.h"
#include "UI/TMOPPauseMenuWidget.h"
#include "UI/TMOPInteractionPromptWidget.h"
#include "Blueprint/UserWidget.h"

ATMOPPlayerCharacter::ATMOPPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    CameraBoom->bEnableCameraLag = true;
    CameraBoom->CameraLagSpeed = 12.0f;
    CameraBoom->SocketOffset.Y = ShoulderOffsetCm;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    AnimationState = CreateDefaultSubobject<UTMOPAnimationStateComponent>(TEXT("AnimationState"));
    AnimationState->bDerivePostureAndMovementFromAgent = false;
    PlayerActions = CreateDefaultSubobject<UTMOPPlayerActionComponent>(TEXT("PlayerActions"));
    Inventory = CreateDefaultSubobject<UTMOPInventoryComponent>(TEXT("Inventory"));
    InventoryInput = CreateDefaultSubobject<UTMOPInventoryInputComponent>(TEXT("InventoryInput"));
    ItemUse = CreateDefaultSubobject<UTMOPPlayerItemUseComponent>(TEXT("ItemUse"));
    Radio = CreateDefaultSubobject<UTMOPPlayerRadioComponent>(TEXT("Radio"));
    VehicleTakeover = CreateDefaultSubobject<UTMOPVehicleTakeoverComponent>(TEXT("VehicleTakeover"));
    VehicleDriving = CreateDefaultSubobject<UTMOPPlayerVehicleDrivingComponent>(TEXT("VehicleDriving"));
    VehicleSession = CreateDefaultSubobject<UTMOPPlayerVehicleSessionComponent>(TEXT("VehicleSession"));
    WorldItemClass = ATMOPWorldItem::StaticClass();
}

void ATMOPPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
        if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                if (IsValid(DefaultMappingContext.Get()))
                    Subsystem->AddMappingContext(DefaultMappingContext.Get(), 0);

    if (bCreateQuickInventoryWidget)
    {
        if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
        {
            TSubclassOf<UTMOPQuickInventoryWidget> WidgetClass = QuickInventoryWidgetClass;
            if (!WidgetClass) WidgetClass = UTMOPQuickInventoryWidget::StaticClass();
            QuickInventoryWidget = CreateWidget<UTMOPQuickInventoryWidget>(
                PlayerController, WidgetClass);
            if (IsValid(QuickInventoryWidget.Get()))
            {
                QuickInventoryWidget->InitializeInventoryInput(InventoryInput);
                QuickInventoryWidget->AddToViewport(50);
                QuickInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
            }
        }
    }

    if (bCreatePauseMenuWidget)
    {
        if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
        {
            TSubclassOf<UTMOPPauseMenuWidget> WidgetClass = PauseMenuWidgetClass;
            if (!WidgetClass) WidgetClass = UTMOPPauseMenuWidget::StaticClass();
            PauseMenuWidget = CreateWidget<UTMOPPauseMenuWidget>(PlayerController, WidgetClass);
            if (IsValid(PauseMenuWidget.Get()))
            {
                PauseMenuWidget->InitializePauseMenu(PlayerController, this);
                PauseMenuWidget->AddToViewport(100);
                PauseMenuWidget->SetMenuVisible(false);
            }
        }
    }

    if (bCreateInteractionPromptWidget)
    {
        if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
        {
            TSubclassOf<UTMOPInteractionPromptWidget> WidgetClass = InteractionPromptWidgetClass;
            if (!WidgetClass) WidgetClass = UTMOPInteractionPromptWidget::StaticClass();
            InteractionPromptWidget = CreateWidget<UTMOPInteractionPromptWidget>(
                PlayerController, WidgetClass);
            if (IsValid(InteractionPromptWidget.Get()))
            {
                InteractionPromptWidget->AddToViewport(40);
                InteractionPromptWidget->SetPromptText(FText::GetEmpty());
            }
        }
    }
}

void ATMOPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (Input == nullptr) return;
    if (MoveAction)
    {
        Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATMOPPlayerCharacter::InputMove);
        Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &ATMOPPlayerCharacter::InputMoveCompleted);
        Input->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ATMOPPlayerCharacter::InputMoveCompleted);
    }
    if (LookAction) Input->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATMOPPlayerCharacter::InputLook);
    if (JumpAction)
    {
        Input->BindAction(JumpAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputJumpStarted);
        Input->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATMOPPlayerCharacter::InputJumpEnded);
    }
    if (SprintAction)
    {
        Input->BindAction(SprintAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputSprintStarted);
        Input->BindAction(SprintAction, ETriggerEvent::Completed, this, &ATMOPPlayerCharacter::InputSprintEnded);
        Input->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ATMOPPlayerCharacter::InputSprintEnded);
    }
    if (InteractAction && !bUseDirectInteractKeyFallback)
        Input->BindAction(InteractAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputInteract);
    if (PrimaryAction) Input->BindAction(PrimaryAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputPrimaryAction);
    if (SecondaryAction)
    {
        Input->BindAction(SecondaryAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputSecondaryActionStarted);
        Input->BindAction(SecondaryAction, ETriggerEvent::Completed, this, &ATMOPPlayerCharacter::InputSecondaryActionEnded);
    }
    if (CancelAction) Input->BindAction(CancelAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputCancel);
    if (SquatAction) Input->BindAction(SquatAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputToggleSquat);
    if (KickAction) Input->BindAction(KickAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputKick);
    if (ShoulderSwapAction)
        Input->BindAction(ShoulderSwapAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputShoulderSwap);
    // Avoid a double toggle when the same key is also handled by the direct fallback.
    if (PauseMenuAction && !bUseDirectPauseKeyFallback)
        Input->BindAction(PauseMenuAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputTogglePauseMenu);
    if (VehicleBrakeAction)
    {
        Input->BindAction(VehicleBrakeAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputVehicleBrakeStarted);
        Input->BindAction(VehicleBrakeAction, ETriggerEvent::Completed, this,
            &ATMOPPlayerCharacter::InputVehicleBrakeEnded);
        Input->BindAction(VehicleBrakeAction, ETriggerEvent::Canceled, this,
            &ATMOPPlayerCharacter::InputVehicleBrakeEnded);
    }
    if (VehicleHandbrakeAction)
    {
        Input->BindAction(VehicleHandbrakeAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputVehicleHandbrakeStarted);
        Input->BindAction(VehicleHandbrakeAction, ETriggerEvent::Completed, this,
            &ATMOPPlayerCharacter::InputVehicleHandbrakeEnded);
        Input->BindAction(VehicleHandbrakeAction, ETriggerEvent::Canceled, this,
            &ATMOPPlayerCharacter::InputVehicleHandbrakeEnded);
    }
    if (DropItemAction && !bUseDirectDropKeyFallback)
        Input->BindAction(DropItemAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputDropEquippedItem);
    if (QuickInventoryAction)
    {
        Input->BindAction(QuickInventoryAction, ETriggerEvent::Started, this,
            &ATMOPPlayerCharacter::InputQuickInventoryStarted);
        Input->BindAction(QuickInventoryAction, ETriggerEvent::Completed, this,
            &ATMOPPlayerCharacter::InputQuickInventoryCompleted);
        Input->BindAction(QuickInventoryAction, ETriggerEvent::Canceled, this,
            &ATMOPPlayerCharacter::InputQuickInventoryCompleted);
    }
    if (InventoryNavigateAction)
        Input->BindAction(InventoryNavigateAction, ETriggerEvent::Triggered, this,
            &ATMOPPlayerCharacter::InputInventoryNavigate);
    if (InventoryCycleAction)
        Input->BindAction(InventoryCycleAction, ETriggerEvent::Triggered, this,
            &ATMOPPlayerCharacter::InputInventoryCycle);
}

void ATMOPPlayerCharacter::InputMove(const FInputActionValue& Value)
{
    if (PlayerActions->bMovementBlocked || InventoryInput->bRadialMenuOpen) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsInVehicle())
    {
        VehicleSession->VehicleThrottle(Axis.Y);
        VehicleSession->VehicleSteering(Axis.X);
        return;
    }
    const FRotator Rotation(0.0f, Controller != nullptr ? Controller->GetControlRotation().Yaw : 0.0f, 0.0f);
    AddMovementInput(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X), Axis.Y);
    AddMovementInput(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void ATMOPPlayerCharacter::InputMoveCompleted()
{
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsInVehicle())
    {
        VehicleSession->VehicleThrottle(0.0f);
        VehicleSession->VehicleSteering(0.0f);
    }
}

void ATMOPPlayerCharacter::InputLook(const FInputActionValue& Value)
{
    if (InventoryInput->bRadialMenuOpen) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X * LookYawSensitivity);
    AddControllerPitchInput(Axis.Y * LookPitchSensitivity * (bInvertLookY ? -1.0f : 1.0f));
}

void ATMOPPlayerCharacter::InputJumpStarted() { Jump(); }
void ATMOPPlayerCharacter::InputJumpEnded() { StopJumping(); }

void ATMOPPlayerCharacter::InputSprintStarted()
{
    SetSprinting(true, false);
}

void ATMOPPlayerCharacter::InputSprintEnded()
{
    SetSprinting(false, false);
}

void ATMOPPlayerCharacter::SetSprinting(const bool bEnabled, const bool bExtraSprint)
{
    const bool bAllowed = bEnabled && !PlayerActions->bMovementBlocked && !bIsCrouched;
    const bool bExtraAllowed = bAllowed && bExtraSprint;
    if (bIsSprinting == bAllowed && bIsExtraSprinting == bExtraAllowed) return;
    bIsSprinting = bAllowed;
    bIsExtraSprinting = bExtraAllowed;
    GetCharacterMovement()->MaxWalkSpeed = bIsExtraSprinting ? ExtraSprintSpeed
        : (bIsSprinting ? SprintSpeed : WalkSpeed);
    AnimationState->SetLocomotionStyle(bIsSprinting
        ? ETMOPAnimLocomotionStyle::FastRun : ETMOPAnimLocomotionStyle::Normal);
}

void ATMOPPlayerCharacter::InputInteract()
{
    if (InventoryInput->bRadialMenuOpen) return;
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsInVehicle())
    {
        VehicleSession->ExitVehicle();
        return;
    }
    AActor* Target = FindInteractionTarget();
    if (IsValid(Target) && Target->GetClass()->ImplementsInterface(UTMOPInteractable::StaticClass()))
    {
        ITMOPInteractable::Execute_Interact(Target, this);
        return;
    }
    if (IsValid(VehicleSession.Get()))
    {
        const ETMOPVehicleTakeoverResult Result = VehicleSession->EnterNearestVehicle(true);
        if (Result == ETMOPVehicleTakeoverResult::SuccessEmptySeat ||
            Result == ETMOPVehicleTakeoverResult::SuccessDriverRemoved) return;
    }
    PlayerActions->StartAction(ETMOPPlayerAction::Interact, Target, 0.35f, false);
}

void ATMOPPlayerCharacter::InputDropEquippedItem()
{
    DropEquippedItem();
}

bool ATMOPPlayerCharacter::DropEquippedItem()
{
    if (bPauseMenuOpen || InventoryInput->bRadialMenuOpen || !IsValid(Inventory.Get()))
        return false;
    UTMOPItemDefinition* Item = Inventory->EquippedItem.Get();
    if (!IsValid(Item) || !Item->bCanDrop || !WorldItemClass || GetWorld() == nullptr)
        return false;

    FVector DropLocation = GetActorLocation() + GetActorForwardVector() * DropForwardDistance;
    FHitResult GroundHit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPDropGround), false, this);
    const FVector TraceStart = DropLocation + FVector(0.0f, 0.0f, 80.0f);
    const FVector TraceEnd = DropLocation - FVector(0.0f, 0.0f, 260.0f);
    if (GetWorld()->LineTraceSingleByChannel(GroundHit, TraceStart, TraceEnd,
        ECC_Visibility, Params))
        DropLocation = GroundHit.ImpactPoint + FVector(0.0f, 0.0f, 25.0f);

    const FTransform SpawnTransform(GetActorRotation(), DropLocation);
    ATMOPWorldItem* Dropped = GetWorld()->SpawnActorDeferred<ATMOPWorldItem>(
        WorldItemClass, SpawnTransform, this, this,
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
    if (!IsValid(Dropped)) return false;
    Dropped->ConfigureWorldItem(Item, 1);
    Dropped->FinishSpawning(SpawnTransform);
    if (!Inventory->RemoveItem(Item, 1))
    {
        Dropped->Destroy();
        return false;
    }
    return true;
}

void ATMOPPlayerCharacter::InputPrimaryAction()
{
    if (InventoryInput->bRadialMenuOpen) return;
    if (InventoryInput->SendEquippedItemInput(ETMOPItemInput::Primary,
        ETMOPItemInputPhase::Started)) return;
    PlayerActions->StartAction(ETMOPPlayerAction::Punch, FindInteractionTarget(), 0.7f, true);
}

void ATMOPPlayerCharacter::InputSecondaryActionStarted()
{
    if (InventoryInput->bRadialMenuOpen) return;
    if (InventoryInput->SendEquippedItemInput(ETMOPItemInput::Secondary,
        ETMOPItemInputPhase::Started)) return;
    PlayerActions->StartAction(ETMOPPlayerAction::AimGun, FindInteractionTarget(), -1.0f, false);
}

void ATMOPPlayerCharacter::InputSecondaryActionEnded()
{
    if (InventoryInput->SendEquippedItemInput(ETMOPItemInput::Secondary,
        ETMOPItemInputPhase::Completed)) return;
    if (PlayerActions->CurrentAction == ETMOPPlayerAction::AimGun) PlayerActions->CompleteCurrentAction();
}

void ATMOPPlayerCharacter::InputCancel()
{
    if (InventoryInput->bRadialMenuOpen)
    {
        FinishQuickInventory(false);
        return;
    }
    PlayerActions->CancelCurrentAction();
}

void ATMOPPlayerCharacter::InputToggleSquat()
{
    if (InventoryInput->bRadialMenuOpen) return;
    if (bIsCrouched)
    {
        UnCrouch();
        AnimationState->SetPostureOverride(ETMOPAnimPosture::Standing);
    }
    else
    {
        InputSprintEnded();
        Crouch();
        AnimationState->SetPostureOverride(ETMOPAnimPosture::Squatting);
    }
}

void ATMOPPlayerCharacter::InputKick()
{
    if (InventoryInput->bRadialMenuOpen) return;
    if (!Inventory->HasEquippedItem())
        PlayerActions->StartAction(ETMOPPlayerAction::Kick, FindInteractionTarget(), 0.8f, true);
}

void ATMOPPlayerCharacter::InputShoulderSwap()
{
    if (InventoryInput->bRadialMenuOpen) return;
    bRightShoulderCamera = !bRightShoulderCamera;
}

void ATMOPPlayerCharacter::InputTogglePauseMenu()
{
    TogglePauseMenu();
}

void ATMOPPlayerCharacter::InputVehicleBrakeStarted()
{
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleBrake(1.0f);
}

void ATMOPPlayerCharacter::InputVehicleBrakeEnded()
{
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleBrake(0.0f);
}

void ATMOPPlayerCharacter::InputVehicleHandbrakeStarted()
{
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleHandbrake(true);
}

void ATMOPPlayerCharacter::InputVehicleHandbrakeEnded()
{
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleHandbrake(false);
}

void ATMOPPlayerCharacter::TogglePauseMenu()
{
    SetPauseMenuOpen(!bPauseMenuOpen);
}

void ATMOPPlayerCharacter::SetPauseMenuOpen(const bool bOpen)
{
    if (bPauseMenuOpen == bOpen || !IsValid(PauseMenuWidget.Get())) return;

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (bOpen)
    {
        bClockWasRunningBeforePause = IsValid(Clock) && Clock->IsClockRunning();
        if (IsValid(Clock)) Clock->PauseClock();
    }

    bPauseMenuOpen = bOpen;
    if (bOpen && IsValid(InventoryInput.Get())) InventoryInput->CancelRadialMenu();
    PauseMenuWidget->SetMenuVisible(bOpen);

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC)) return;
    PC->SetPause(bOpen);
    PC->bShowMouseCursor = bOpen;
    if (bOpen)
    {
        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(PauseMenuWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
    }
    else
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        if (IsValid(Clock) && bClockWasRunningBeforePause) Clock->StartClock();
        bClockWasRunningBeforePause = false;
    }
}

void ATMOPPlayerCharacter::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (bUseDirectSprintKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bSprintHeld = IsValid(PC) && PC->IsInputKeyDown(SprintFallbackKey);
        const bool bExtraHeld = bSprintHeld && IsValid(PC)
            && PC->IsInputKeyDown(ExtraSprintModifierKey);
        SetSprinting(bSprintHeld, bExtraHeld);
    }
    if (bUseDirectQuickInventoryKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) && PC->IsInputKeyDown(QuickInventoryFallbackKey);
        if (bKeyHeld != bQuickInventoryFallbackHeld)
        {
            bQuickInventoryFallbackHeld = bKeyHeld;
            if (bKeyHeld) InputQuickInventoryStarted();
            else InputQuickInventoryCompleted();
        }
    }
    if (bUseDirectPauseKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) && PC->IsInputKeyDown(PauseMenuFallbackKey);
        if (bKeyHeld != bPauseFallbackHeld)
        {
            bPauseFallbackHeld = bKeyHeld;
            if (bKeyHeld)
            {
                if (InventoryInput->bRadialMenuOpen) FinishQuickInventory(false);
                else TogglePauseMenu();
            }
        }
    }
    if (bUseDirectDropKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) && PC->IsInputKeyDown(DropItemFallbackKey);
        if (bKeyHeld != bDropFallbackHeld)
        {
            bDropFallbackHeld = bKeyHeld;
            if (bKeyHeld) DropEquippedItem();
        }
    }
    if (bUseDirectInteractKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) && PC->IsInputKeyDown(InteractFallbackKey);
        if (bKeyHeld != bInteractFallbackHeld)
        {
            bInteractFallbackHeld = bKeyHeld;
            if (bKeyHeld && !bPauseMenuOpen && !InventoryInput->bRadialMenuOpen)
                InputInteract();
        }
    }
    if (InventoryInput->bRadialMenuOpen) UpdateQuickInventoryPointer();
    UpdateInteractionPrompt();
    if (!IsValid(CameraBoom.Get())) return;
    const float TargetY = (bRightShoulderCamera ? 1.0f : -1.0f) * ShoulderOffsetCm;
    FVector Offset = CameraBoom->SocketOffset;
    Offset.Y = FMath::FInterpTo(Offset.Y, TargetY, DeltaSeconds, ShoulderSwapSpeed);
    CameraBoom->SocketOffset = Offset;
}

void ATMOPPlayerCharacter::UpdateInteractionPrompt()
{
    if (!IsValid(InteractionPromptWidget.Get())) return;
    FText Prompt;
    if (!bPauseMenuOpen && !InventoryInput->bRadialMenuOpen)
    {
        AActor* Target = FindInteractionTarget();
        if (IsValid(Target) && Target->GetClass()->ImplementsInterface(
            UTMOPInteractable::StaticClass()))
            Prompt = ITMOPInteractable::Execute_GetInteractionText(Target);
    }
    InteractionPromptWidget->SetPromptText(Prompt);
}

void ATMOPPlayerCharacter::InputQuickInventoryStarted()
{
    if (bPauseMenuOpen || !InventoryInput->OpenRadialMenu()) return;
    SetSprinting(false, false);
    GetCharacterMovement()->StopMovementImmediately();
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        int32 SizeX = 0, SizeY = 0;
        PC->GetViewportSize(SizeX, SizeY);
        PC->SetMouseLocation(SizeX / 2, SizeY / 2);
        PC->bShowMouseCursor = true;
        FInputModeGameAndUI Mode;
        if (IsValid(QuickInventoryWidget.Get()))
            Mode.SetWidgetToFocus(QuickInventoryWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
        PC->SetInputMode(Mode);
    }
}

void ATMOPPlayerCharacter::InputQuickInventoryCompleted()
{
    FinishQuickInventory(true);
}

void ATMOPPlayerCharacter::FinishQuickInventory(const bool bConfirm)
{
    if (!InventoryInput->bRadialMenuOpen) return;
    if (bConfirm) InventoryInput->ConfirmRadialSelection();
    else InventoryInput->CancelRadialMenu();
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (!bPauseMenuOpen)
        {
            PC->bShowMouseCursor = false;
            PC->SetInputMode(FInputModeGameOnly());
        }
    }
}

void ATMOPPlayerCharacter::UpdateQuickInventoryPointer()
{
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC)) return;

    int32 SizeX = 0, SizeY = 0;
    float MouseX = 0.0f, MouseY = 0.0f;
    PC->GetViewportSize(SizeX, SizeY);
    if (PC->GetMousePosition(MouseX, MouseY) && SizeX > 0 && SizeY > 0)
    {
        const FVector2D Direction(MouseX - SizeX * 0.5f, SizeY * 0.5f - MouseY);
        const float Normalizer = FMath::Max(1.0f, FMath::Min(SizeX, SizeY) * 0.22f);
        InventoryInput->UpdateRadialSelection(Direction / Normalizer);
    }

    if (PC->WasInputKeyJustPressed(QuickInventoryPreviousKey)
        || PC->WasInputKeyJustPressed(EKeys::MouseScrollDown))
        InventoryInput->StepRadialSelection(-1);
    if (PC->WasInputKeyJustPressed(QuickInventoryNextKey)
        || PC->WasInputKeyJustPressed(EKeys::MouseScrollUp))
        InventoryInput->StepRadialSelection(1);
}

void ATMOPPlayerCharacter::InputInventoryNavigate(const FInputActionValue& Value)
{
    InventoryInput->UpdateRadialSelection(Value.Get<FVector2D>());
}

void ATMOPPlayerCharacter::InputInventoryCycle(const FInputActionValue& Value)
{
    const float Direction = Value.Get<float>();
    if (!FMath::IsNearlyZero(Direction)) InventoryInput->CycleInventory(Direction > 0.0f ? 1 : -1);
}

AActor* ATMOPPlayerCharacter::FindInteractionTarget() const
{
    if (!IsValid(FollowCamera) || GetWorld() == nullptr) return nullptr;
    const FVector Start = FollowCamera->GetComponentLocation();
    const FVector End = Start + FollowCamera->GetForwardVector() * InteractionDistance;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPPlayerInteraction), false, this);
    return GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params)
        ? Hit.GetActor() : nullptr;
}

FText ATMOPPlayerCharacter::GetInteractKeyDisplayText() const
{
    return InteractFallbackKey.IsValid()
        ? InteractFallbackKey.GetDisplayName(false)
        : FText::FromString(TEXT("E"));
}
