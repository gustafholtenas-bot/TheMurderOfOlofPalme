#include "Player/TMOPPlayerCharacter.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Application/SlateUser.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "Audio/TMOPAgentAudioComponent.h"
#include "Audio/TMOPPlayerMovementAudioComponent.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraActor.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedActionKeyMapping.h"
#include "Engine/OverlapResult.h"
#include "EngineUtils.h"
#include "InputMappingContext.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Items/TMOPPlayerItemUseComponent.h"
#include "Items/TMOPInteractable.h"
#include "Items/TMOPWorldItem.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"
#include "Newspapers/TMOPNewspaperReadingComponent.h"
#include "Player/TMOPPlayerActionComponent.h"
#include "Player/TMOPVehicleTakeoverComponent.h"
#include "Player/TMOPPlayerVehicleDrivingComponent.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Player/TMOPCameraPerspectiveComponent.h"
#include "People/TMOPPersonRegistryDirector.h"
#include "People/TMOPPersonProfileComponent.h"
#include "Radio/TMOPPlayerRadioComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "UI/TMOPQuickInventoryWidget.h"
#include "UI/TMOPPauseMenuWidget.h"
#include "UI/TMOPLoopEndWidget.h"
#include "UI/TMOPLocalPlayerOverlay.h"
#include "UI/TMOPInteractionPromptWidget.h"
#include "UI/TMOPDialogWidget.h"
#include "UI/TMOPAddressDirectoryWidget.h"
#include "Addresses/TMOPAddressComponent.h"
#include "World/TMOPInspectableComponent.h"
#include "EngineGlobals.h"
#include "UI/TMOPAgentInfoChartWidget.h"
#include "UI/TMOPNewspaperReaderWidget.h"
#include "UI/TMOPMapComponent.h"
#include "UI/TMOPMapWidget.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
bool IsMappedActionHeld(const APlayerController* PC, const UInputMappingContext* Context,
    const UInputAction* Action, FKey KeyboardKey, FKey GamepadFallback)
{
    if (!PC) return false;
    if (PC->IsInputKeyDown(KeyboardKey)) return true;
    bool bHasGamepadMapping = false;
    if (Context && Action)
        for (const FEnhancedActionKeyMapping& Mapping : Context->GetMappings())
            if (Mapping.Action == Action && Mapping.Key.IsGamepadKey())
            {
                bHasGamepadMapping = true;
                if (PC->IsInputKeyDown(Mapping.Key)) return true;
            }
    return !bHasGamepadMapping && PC->IsInputKeyDown(GamepadFallback);
}
}

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
    CameraPerspective = CreateDefaultSubobject<UTMOPCameraPerspectiveComponent>(
        TEXT("CameraPerspective"));
    FootstepAudio = CreateDefaultSubobject<UTMOPAgentAudioComponent>(TEXT("FootstepAudio"));
    MovementAudio = CreateDefaultSubobject<UTMOPPlayerMovementAudioComponent>(TEXT("MovementAudio"));
    MapComponent = CreateDefaultSubobject<UTMOPMapComponent>(TEXT("MapComponent"));
    NewspaperReading = CreateDefaultSubobject<UTMOPNewspaperReadingComponent>(
        TEXT("NewspaperReading"));
    WorldItemClass = ATMOPWorldItem::StaticClass();
}

void ATMOPPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
    InitializePlayerInterface();
}

void ATMOPPlayerCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializePlayerInterface();
}

void ATMOPPlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
    {
        Clock->OnLoopEnded.RemoveDynamic(this, &ATMOPPlayerCharacter::HandleLoopEnded);
        Clock->ReleaseAllPauses(this);
    }
    if (IsValid(VehicleSession)) VehicleSession->ExitVehicle();
    // All widgets belong to this local player and must not survive a removed pawn.
    const TArray<UUserWidget*> Widgets = {QuickInventoryWidget, PauseMenuWidget, WorldMapWidget,
        MinimapWidget, InteractionPromptWidget, DialogWidget, AgentInfoChartWidget, NewspaperReaderWidget};
    for (UUserWidget* Widget : Widgets) if (IsValid(Widget)) Widget->RemoveFromParent();
    if (IsValid(LoopEndWidget.Get())) LoopEndWidget->RemoveFromParent();
    if (LocalPlayerOverlay) LocalPlayerOverlay->RemoveFromParent();
    LoopEndWidget = nullptr;
    bLoopEndMenuOpen = false;
    CloseAddressDirectory();
    if (IsValid(AddressDirectoryWidget.Get())) AddressDirectoryWidget->RemoveFromParent();
    AddressDirectoryWidget = nullptr;
    Super::EndPlay(EndPlayReason);
}

void ATMOPPlayerCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    InitializePlayerInterface();
}

void ATMOPPlayerCharacter::InitializePlayerInterface()
{
    APlayerController* PlayerController = Cast<APlayerController>(Controller);
    if (!IsValid(PlayerController) || !PlayerController->IsLocalController()) return;
    PlayerController->PrimaryActorTick.bTickEvenWhenPaused = true;
    PlayerController->bShouldPerformFullTickWhenPaused = true;
    if (!LocalPlayerOverlay)
    {
        LocalPlayerOverlay = CreateWidget<UTMOPLocalPlayerOverlay>(PlayerController, UTMOPLocalPlayerOverlay::StaticClass());
        if (LocalPlayerOverlay)
        {
            LocalPlayerOverlay->AddToPlayerScreen(1500);
            LocalPlayerOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
        }
    }

    if (IsValid(Inventory.Get()))
        Inventory->OnItemMenuRequested.AddUniqueDynamic(
            this, &ATMOPPlayerCharacter::HandleItemMenuRequested);

    UTMOPClockSubsystem* ScenarioClock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (IsValid(ScenarioClock))
        ScenarioClock->OnLoopEnded.AddUniqueDynamic(
            this, &ATMOPPlayerCharacter::HandleLoopEnded);

    if (!bInputMappingContextAdded)
    {
        if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                if (IsValid(DefaultMappingContext.Get()))
                {
                    Subsystem->AddMappingContext(DefaultMappingContext.Get(), 0);
                    bInputMappingContextAdded = true;
                }
    }

    if (bCreateQuickInventoryWidget && !IsValid(QuickInventoryWidget.Get()))
    {
        TSubclassOf<UTMOPQuickInventoryWidget> WidgetClass = QuickInventoryWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPQuickInventoryWidget::StaticClass();
        QuickInventoryWidget = CreateWidget<UTMOPQuickInventoryWidget>(
            PlayerController, WidgetClass);
        if (IsValid(QuickInventoryWidget.Get()))
        {
            QuickInventoryWidget->InitializeInventoryInput(InventoryInput);
            QuickInventoryWidget->AddToPlayerScreen(50);
            QuickInventoryWidget->SetVisibility(ESlateVisibility::Collapsed);
        }
    }

    if (bCreatePauseMenuWidget && !IsValid(PauseMenuWidget.Get()))
    {
        TSubclassOf<UTMOPPauseMenuWidget> WidgetClass = PauseMenuWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPPauseMenuWidget::StaticClass();
        PauseMenuWidget = CreateWidget<UTMOPPauseMenuWidget>(
            PlayerController, WidgetClass);
        if (IsValid(PauseMenuWidget.Get()))
        {
            PauseMenuWidget->InitializePauseMenu(PlayerController, this);
            PauseMenuWidget->AddToPlayerScreen(100);
            PauseMenuWidget->SetMenuVisible(false);
        }
    }

    if (bCreateLoopEndWidget && !IsValid(LoopEndWidget.Get()))
    {
        TSubclassOf<UTMOPLoopEndWidget> WidgetClass = LoopEndWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPLoopEndWidget::StaticClass();
        LoopEndWidget = CreateWidget<UTMOPLoopEndWidget>(PlayerController, WidgetClass);
        if (IsValid(LoopEndWidget.Get()))
        {
            LoopEndWidget->InitializeLoopEnd(this);
            LoopEndWidget->AddToPlayerScreen(2000);
            LoopEndWidget->SetMenuVisible(false);
        }
    }

    if (bCreateMapWidgets && !IsValid(WorldMapWidget.Get()))
    {
        TSubclassOf<UTMOPMapWidget> WidgetClass = WorldMapWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPMapWidget::StaticClass();
        WorldMapWidget = CreateWidget<UTMOPMapWidget>(PlayerController, WidgetClass);
        if (IsValid(WorldMapWidget.Get()))
        {
            WorldMapWidget->InitializeMap(MapComponent, this, false);
            WorldMapWidget->AddToPlayerScreen(95);
            WorldMapWidget->SetMapVisible(false);
        }
    }
    if (bCreateMapWidgets && !IsValid(MinimapWidget.Get()))
    {
        TSubclassOf<UTMOPMapWidget> WidgetClass = MinimapWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPMapWidget::StaticClass();
        MinimapWidget = CreateWidget<UTMOPMapWidget>(PlayerController, WidgetClass);
        if (IsValid(MinimapWidget.Get()))
        {
            MinimapWidget->InitializeMap(MapComponent, this, true);
            MinimapWidget->AddToPlayerScreen(20);
            MinimapWidget->SetMapVisible(IsValid(MapComponent) && MapComponent->bShowMinimap);
        }
    }

    if ((bCreateInteractionPromptWidget || bForceNativeTargetInformationWidget) &&
        !IsValid(InteractionPromptWidget.Get()))
    {
        TSubclassOf<UTMOPInteractionPromptWidget> WidgetClass =
            InteractionPromptWidgetClass;
        if (bForceNativeTargetInformationWidget)
            WidgetClass = UTMOPInteractionPromptWidget::StaticClass();
        if (!WidgetClass) WidgetClass = UTMOPInteractionPromptWidget::StaticClass();
        InteractionPromptWidget = CreateWidget<UTMOPInteractionPromptWidget>(
            PlayerController, WidgetClass);
        if (IsValid(InteractionPromptWidget.Get()))
        {
            InteractionPromptWidget->AddToPlayerScreen(40);
            InteractionPromptWidget->SetPromptText(FText::GetEmpty());
        }
    }

    if (bCreateDialogWidget && !IsValid(DialogWidget.Get()))
    {
        TSubclassOf<UTMOPDialogWidget> WidgetClass = DialogWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPDialogWidget::StaticClass();
        DialogWidget = CreateWidget<UTMOPDialogWidget>(
            PlayerController, WidgetClass);
        if (IsValid(DialogWidget.Get()))
        {
            DialogWidget->InitializeDialog(this);
            DialogWidget->AddToPlayerScreen(80);
            DialogWidget->HideDialog();
        }
    }

    if (bCreateAgentInfoChartWidget && !IsValid(AgentInfoChartWidget.Get()))
    {
        TSubclassOf<UTMOPAgentInfoChartWidget> WidgetClass =
            AgentInfoChartWidgetClass;
        if (!WidgetClass) WidgetClass = UTMOPAgentInfoChartWidget::StaticClass();
        AgentInfoChartWidget = CreateWidget<UTMOPAgentInfoChartWidget>(
            PlayerController, WidgetClass);
        if (IsValid(AgentInfoChartWidget.Get()))
        {
            AgentInfoChartWidget->InitializeAgentInfo(this);
            AgentInfoChartWidget->AddToPlayerScreen(85);
            AgentInfoChartWidget->HideAgentInfo();
        }
    }

    if (bCreateNewspaperReaderWidget &&
        !IsValid(NewspaperReaderWidget.Get()))
    {
        TSubclassOf<UTMOPNewspaperReaderWidget> WidgetClass =
            NewspaperReaderWidgetClass;
        if (!WidgetClass)
            WidgetClass = UTMOPNewspaperReaderWidget::StaticClass();
        NewspaperReaderWidget =
            CreateWidget<UTMOPNewspaperReaderWidget>(
                PlayerController, WidgetClass);
        if (IsValid(NewspaperReaderWidget.Get()))
        {
            NewspaperReaderWidget->InitializeReader(this);
            NewspaperReaderWidget->AddToPlayerScreen(90);
            NewspaperReaderWidget->SetVisibility(
                ESlateVisibility::Collapsed);
        }
    }

    bPlayerInterfaceInitialized =
        bInputMappingContextAdded &&
        (!bCreateQuickInventoryWidget || IsValid(QuickInventoryWidget.Get())) &&
        (!bCreatePauseMenuWidget || IsValid(PauseMenuWidget.Get())) &&
        (!bCreateLoopEndWidget || IsValid(LoopEndWidget.Get())) &&
        (!bCreateAgentInfoChartWidget || IsValid(AgentInfoChartWidget.Get())) &&
        (!bCreateNewspaperReaderWidget ||
            IsValid(NewspaperReaderWidget.Get()));

    // A menu/cinematic may have requested hidden HUD before the widgets were
    // constructed. Apply the current effective state to newly created widgets.
    UpdateGameplayHUDVisibility();

    // Also covers loading/possessing a player when the clock is already at 23:45.
    if (IsValid(ScenarioClock) && ScenarioClock->IsAwaitingLoopDecision())
        OpenLoopEndMenu();
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
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (PlayerActions->bMovementBlocked || InventoryInput->bRadialMenuOpen) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsInVehicle())
    {
        const float VehicleThrottle = bSwapVehicleMoveAxes ? Axis.X : Axis.Y;
        const float VehicleSteering = bSwapVehicleMoveAxes ? Axis.Y : Axis.X;
        VehicleSession->VehicleThrottle(VehicleThrottle);
        VehicleSession->VehicleSteering(VehicleSteering);
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
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (InventoryInput->bRadialMenuOpen) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    const float ZoomSensitivity = IsValid(CameraPerspective.Get())
        ? CameraPerspective->GetLookSensitivityScale() : 1.0f;
    AddControllerYawInput(Axis.X * LookYawSensitivity * ZoomSensitivity);
    AddControllerPitchInput(Axis.Y * LookPitchSensitivity * ZoomSensitivity * (bInvertLookY ? -1.0f : 1.0f));
}

void ATMOPPlayerCharacter::InputJumpStarted() { if (!IsSessionGameplayBlocked() && !bAddressDirectoryOpen) Jump(); }
void ATMOPPlayerCharacter::InputJumpEnded() { StopJumping(); }

void ATMOPPlayerCharacter::InputSprintStarted()
{
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsDrivingVehicle())
    {
        VehicleSession->VehicleHighSpeedMode(true);
        return;
    }
    SetSprinting(true, false);
}

void ATMOPPlayerCharacter::InputSprintEnded()
{
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsDrivingVehicle())
    {
        VehicleSession->VehicleHighSpeedMode(false);
        return;
    }
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
    // A UI key can also reach the direct-key fallback during this frame.
    if (AddressDirectoryClosedFrame == GFrameCounter || bPauseMenuOpen ||
        bWorldMapOpen || !bGameplayHUDVisible) return;
    if (bAddressDirectoryOpen)
    {
        CloseAddressDirectory();
        return;
    }
    if (bNewspaperOpen)
    {
        CloseNewspaper();
        return;
    }
    if (bDialogOpen)
    {
        ClosePersonDialog();
        return;
    }
    if (bAgentInfoChartOpen)
    {
        CloseAgentInfoChart();
        return;
    }
    if (IsSessionGameplayBlocked()) return;
    if (InventoryInput->bRadialMenuOpen) return;
    if (IsValid(VehicleSession.Get()) && VehicleSession->IsInVehicle())
    {
        VehicleSession->ExitVehicle();
        return;
    }
    AActor* InformationTarget = FindInformationTarget();
    AActor* Target = FindInteractionTargetForInformation(InformationTarget);
    // Use the same readable point as the hover marker, even when another target is nearby.
    if (UTMOPInspectableComponent* Inspection = IsValid(InformationTarget)
        ? InformationTarget->FindComponentByClass<UTMOPInspectableComponent>() : nullptr)
    {
        OpenInformation(Inspection); // Revalidates range and visibility on the press.
        return;
    }
    if (UTMOPInspectableComponent* Inspection = IsValid(Target)
        ? Target->FindComponentByClass<UTMOPInspectableComponent>() : nullptr)
    {
        OpenInformation(Inspection);
        return;
    }
    if (ATMOPHistoricalAgent* LockedAgent =
        Cast<ATMOPHistoricalAgent>(InformationTarget))
        if (FVector::DistSquared(GetActorLocation(), LockedAgent->GetActorLocation()) <=
            FMath::Square(InteractionDistance))
            Target = LockedAgent;
    if (ATMOPHistoricalAgent* HistoricalAgent =
        Cast<ATMOPHistoricalAgent>(Target))
    {
        OpenAgentInfoChart(HistoricalAgent);
        return;
    }
    if (IsValid(Target) && Target->GetClass()->ImplementsInterface(UTMOPInteractable::StaticClass()))
    {
        ITMOPInteractable::Execute_Interact(Target, this);
        return;
    }
    if (ATMOPVehicleBase* Vehicle = Cast<ATMOPVehicleBase>(Target))
    {
        if (IsValid(VehicleSession.Get()))
        {
            const ETMOPVehicleTakeoverResult Result =
                VehicleSession->EnterVehicle(Vehicle, true);
            if (Result == ETMOPVehicleTakeoverResult::SuccessEmptySeat ||
                Result == ETMOPVehicleTakeoverResult::SuccessDriverRemoved) return;
        }
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
    if (IsSessionGameplayBlocked()) return false;
    if (bAddressDirectoryOpen) return false;
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
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (bNewspaperOpen) return;
    if (InventoryInput->bRadialMenuOpen) return;
    if (InventoryInput->SendEquippedItemInput(ETMOPItemInput::Primary,
        ETMOPItemInputPhase::Started)) return;
    PlayerActions->StartAction(ETMOPPlayerAction::Punch, FindInteractionTarget(), 0.7f, true);
}

void ATMOPPlayerCharacter::InputSecondaryActionStarted()
{
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (bNewspaperOpen) return;
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
    if (bAddressDirectoryOpen)
    {
        CloseAddressDirectory();
        return;
    }
    if (bPauseMenuOpen)
    {
        SetPauseMenuOpen(false);
        return;
    }
    if (bWorldMapOpen)
    {
        CloseWorldMap();
        return;
    }
    if (bNewspaperOpen)
    {
        CloseNewspaper();
        return;
    }
    if (bAgentInfoChartOpen)
    {
        CloseAgentInfoChart();
        return;
    }
    if (bDialogOpen)
    {
        ClosePersonDialog();
        return;
    }
    if (InventoryInput->bRadialMenuOpen)
    {
        FinishQuickInventory(false);
        return;
    }
    PlayerActions->CancelCurrentAction();
}

void ATMOPPlayerCharacter::InputToggleSquat()
{
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
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
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (InventoryInput->bRadialMenuOpen) return;
    if (!Inventory->HasEquippedItem())
        PlayerActions->StartAction(ETMOPPlayerAction::Kick, FindInteractionTarget(), 0.8f, true);
}

void ATMOPPlayerCharacter::InputShoulderSwap()
{
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (InventoryInput->bRadialMenuOpen) return;
    bRightShoulderCamera = !bRightShoulderCamera;
}

void ATMOPPlayerCharacter::InputTogglePauseMenu()
{
    if (AddressDirectoryClosedFrame == GFrameCounter) return;
    if (bAddressDirectoryOpen) { CloseAddressDirectory(); return; }
    TogglePauseMenu();
}

void ATMOPPlayerCharacter::InputVehicleBrakeStarted()
{
    if (IsSessionGameplayBlocked()) return;
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleBrake(1.0f);
}

void ATMOPPlayerCharacter::InputVehicleBrakeEnded()
{
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleBrake(0.0f);
}

void ATMOPPlayerCharacter::InputVehicleHandbrakeStarted()
{
    if (IsSessionGameplayBlocked()) return;
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleHandbrake(true);
}

void ATMOPPlayerCharacter::InputVehicleHandbrakeEnded()
{
    if (IsValid(VehicleSession.Get())) VehicleSession->VehicleHandbrake(false);
}

void ATMOPPlayerCharacter::TogglePauseMenu()
{
    if (bLoopEndMenuOpen) return;
    SetPauseMenuOpen(!bPauseMenuOpen);
}

bool ATMOPPlayerCharacter::IsSessionGameplayBlocked() const
{
    return !IsLocallyControlled() || !bGameplayHUDVisible || bLoopEndMenuOpen ||
        bPauseMenuOpen || bWorldMapOpen || bNewspaperOpen || bDialogOpen ||
        bAgentInfoChartOpen || bAddressDirectoryOpen ||
        UGameplayStatics::IsGamePaused(this);
}

void ATMOPPlayerCharacter::ApplyLocalInputMode(UUserWidget* FocusWidget)
{
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!PC || !PC->IsLocalController()) return;
    TFunction<TSharedPtr<SWidget>(TSharedRef<SWidget>)> FirstFocusable;
    FirstFocusable = [&FirstFocusable](TSharedRef<SWidget> Widget) -> TSharedPtr<SWidget>
    {
        if (!Widget->GetVisibility().IsVisible() || !Widget->IsEnabled()) return nullptr;
        FChildren* Children = Widget->GetChildren();
        for (int32 Index = 0; Children && Index < Children->Num(); ++Index)
            if (TSharedPtr<SWidget> Child = FirstFocusable(Children->GetChildAt(Index))) return Child;
        return Widget->SupportsKeyboardFocus() ? TSharedPtr<SWidget>(Widget) : nullptr;
    };
    TSharedPtr<SWidget> FocusTarget = FocusWidget ? FirstFocusable(FocusWidget->TakeWidget()) : nullptr;
    if (FocusWidget && !FocusTarget) FocusTarget = FocusWidget->TakeWidget();
    if (!UTMOPLocalMultiplayerSubsystem::IsMultiplayer(this))
    {
        if (FocusWidget)
        {
            FInputModeGameAndUI Mode;
            Mode.SetWidgetToFocus(FocusTarget);
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            PC->SetInputMode(Mode);
        }
        else PC->SetInputMode(FInputModeGameOnly());
        PC->bShowMouseCursor = FocusWidget != nullptr;
        return;
    }
    // Input mode/capture is viewport-global. Do not let a second player reset
    // it while the first player is reading. Slate focus itself is per user.
    PC->bShowMouseCursor = FocusWidget && UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(this) == 0;
    ULocalPlayer* Local = PC->GetLocalPlayer();
    if (!Local || !Local->GetSlateUser().IsValid()) return;
    const uint32 UserIndex = Local->GetSlateUser()->GetUserIndex();
    if (FocusWidget)
        FSlateApplication::Get().SetUserFocus(UserIndex, FocusTarget, EFocusCause::SetDirectly);
    else FSlateApplication::Get().SetUserFocusToGameViewport(UserIndex, EFocusCause::SetDirectly);
}

void ATMOPPlayerCharacter::CloseSessionMenus()
{
    CloseLoopEndMenu();
    SetPauseMenuOpen(false);
    CloseWorldMap();
    CloseNewspaper();
    ClosePersonDialog();
    CloseAgentInfoChart();
    CloseAddressDirectory();
    FinishQuickInventory(false);
    if (GetGameInstance())
        GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->ReleaseAllPauses(this);
    SetGameplayHUDHidden(TEXT("MainMenu"), false);
    SetGameplayHUDHidden(TEXT("Cinematic"), false);
    ApplyLocalInputMode();
}

void ATMOPPlayerCharacter::RefreshLoopEndMenu()
{
    if (GetGameInstance())
        if (GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->IsAwaitingLoopDecision())
        {
            OpenLoopEndMenu();
            if (bLoopEndMenuOpen && LoopEndWidget) ApplyLocalInputMode(LoopEndWidget);
        }
}

void ATMOPPlayerCharacter::SetPauseMenuOpen(const bool bOpen)
{
    if (bOpen && bLoopEndMenuOpen) return;
    if (bPauseMenuOpen == bOpen || !IsValid(PauseMenuWidget.Get())) return;
    if (bOpen && bWorldMapOpen) CloseWorldMap();
    if (bOpen && bNewspaperOpen) CloseNewspaper();
    if (bOpen && bDialogOpen) ClosePersonDialog();
    if (bOpen && bAgentInfoChartOpen) CloseAgentInfoChart();
    if (bOpen && bAddressDirectoryOpen) CloseAddressDirectory();

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (bOpen)
    {
        if (IsValid(Clock)) Clock->RequestPause(this, TEXT("PauseMenu"));
    }
    else if (IsValid(Clock)) Clock->ReleasePause(this, TEXT("PauseMenu"));

    bPauseMenuOpen = bOpen;
    SetGameplayHUDHidden(TEXT("PauseMenu"), bOpen);
    if (bOpen && IsValid(InventoryInput.Get())) InventoryInput->CancelRadialMenu();
    PauseMenuWidget->SetMenuVisible(bOpen);

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC)) return;
    PC->bShowMouseCursor = bOpen;
    if (bOpen)
    {
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        ApplyLocalInputMode(PauseMenuWidget);
    }
    else
    {
        ApplyLocalInputMode();
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        bClockWasRunningBeforePause = false;
    }
}

void ATMOPPlayerCharacter::HandleLoopEnded(
    const int32 FinishedLoopNumber, const FTMOPTime EndTime)
{
    UE_LOG(LogTemp, Log, TEXT("TMOP loop %d ended at %s; opening choice menu."),
        FinishedLoopNumber, *EndTime.ToDisplayString());
    OpenLoopEndMenu();
}

void ATMOPPlayerCharacter::OpenLoopEndMenu()
{
    if (bLoopEndMenuOpen || !IsValid(LoopEndWidget.Get())) return;
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC) || !PC->IsLocalController()) return;

    if (bPauseMenuOpen) SetPauseMenuOpen(false);
    if (bWorldMapOpen) CloseWorldMap();
    if (bNewspaperOpen) CloseNewspaper();
    if (bDialogOpen) ClosePersonDialog();
    if (bAgentInfoChartOpen) CloseAgentInfoChart();
    if (bAddressDirectoryOpen) CloseAddressDirectory();
    if (IsValid(InventoryInput.Get())) InventoryInput->CancelRadialMenu();
    if (IsValid(CameraPerspective.Get())) CameraPerspective->CancelLookZoom();
    GetCharacterMovement()->StopMovementImmediately();
    SetSprinting(false, false);

    bLoopEndMenuOpen = true;
    SetGameplayHUDHidden(TEXT("LoopEnd"), true);
    LoopEndWidget->SetMenuVisible(true);
    PC->bShowMouseCursor = true;
    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    ApplyLocalInputMode(LoopEndWidget);
}

void ATMOPPlayerCharacter::CloseLoopEndMenu()
{
    if (!bLoopEndMenuOpen) return;
    bLoopEndMenuOpen = false;
    if (IsValid(LoopEndWidget.Get())) LoopEndWidget->SetMenuVisible(false);
    SetGameplayHUDHidden(TEXT("LoopEnd"), false);
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        ApplyLocalInputMode();
    }
}

void ATMOPPlayerCharacter::ReplayLoopFromBeginning()
{
    if (!bLoopEndMenuOpen) return;
    if (GetGameInstance()) GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>()->RestartFromBeginning();
}

void ATMOPPlayerCharacter::ReturnToMainMenuFromLoopEnd()
{
    if (!bLoopEndMenuOpen || GetWorld() == nullptr) return;
    if (GetGameInstance()) GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>()->ReturnToMainMenu();
}

void ATMOPPlayerCharacter::QuitFromLoopEnd()
{
    if (!bLoopEndMenuOpen) return;
    UKismetSystemLibrary::QuitGame(this, Cast<APlayerController>(Controller),
        EQuitPreference::Quit, false);
}

void ATMOPPlayerCharacter::SetGameplayHUDHidden(
    const FName Reason, const bool bShouldHide)
{
    const FName ResolvedReason = Reason.IsNone() ? FName(TEXT("Unspecified")) : Reason;
    if (bShouldHide) GameplayHUDHiddenReasons.Add(ResolvedReason);
    else GameplayHUDHiddenReasons.Remove(ResolvedReason);
    UpdateGameplayHUDVisibility();
}

void ATMOPPlayerCharacter::SetCinematicHUDHidden(const bool bShouldHide)
{
    SetGameplayHUDHidden(TEXT("Cinematic"), bShouldHide);
}

void ATMOPPlayerCharacter::UpdateGameplayHUDVisibility()
{
    const bool bShouldBeVisible = GameplayHUDHiddenReasons.IsEmpty();
    const bool bVisibilityChanged = bGameplayHUDVisible != bShouldBeVisible;
    bGameplayHUDVisible = bShouldBeVisible;
    if (!bGameplayHUDVisible && IsValid(CameraPerspective.Get())) CameraPerspective->CancelLookZoom();
    if (!bGameplayHUDVisible && bAddressDirectoryOpen) CloseAddressDirectory();

    if (IsValid(MinimapWidget.Get()))
    {
        const bool bShowMinimap = bGameplayHUDVisible && !bWorldMapOpen &&
            IsValid(MapComponent.Get()) && MapComponent->bShowMinimap;
        MinimapWidget->SetMapVisible(bShowMinimap);
    }

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (AHUD* HUD = PC->GetHUD())
        {
            if (HUD->bShowHUD != bGameplayHUDVisible)
                HUD->ShowHUD();
        }
    }

    if (bVisibilityChanged)
        OnGameplayHUDVisibilityChanged(bGameplayHUDVisible);
}

void ATMOPPlayerCharacter::HandleItemMenuRequested(
    UTMOPItemDefinition* Item)
{
    if (IsValid(Item) && Item->ItemType == ETMOPItemType::Map)
    {
        OpenWorldMap();
        return;
    }
    if (UTMOPNewspaperItemDefinition* Newspaper =
        Cast<UTMOPNewspaperItemDefinition>(Item))
    {
        OpenNewspaper(Newspaper);
    }
}

bool ATMOPPlayerCharacter::OpenWorldMap()
{
    if (bLoopEndMenuOpen || !bGameplayHUDVisible || bWorldMapOpen || bPauseMenuOpen || bNewspaperOpen ||
        !IsValid(WorldMapWidget.Get())) return false;
    if (bDialogOpen) ClosePersonDialog();
    if (bAddressDirectoryOpen) CloseAddressDirectory();
    if (bAgentInfoChartOpen) CloseAgentInfoChart();
    if (IsValid(InventoryInput.Get())) InventoryInput->CancelRadialMenu();
    if (IsValid(MinimapWidget.Get())) MinimapWidget->SetMapVisible(false);
    WorldMapWidget->ResetViewToPlayer();
    WorldMapWidget->SetMapVisible(true);
    bWorldMapOpen = true;

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (IsValid(Clock)) Clock->RequestPause(this, TEXT("WorldMap"));
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = true;
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        ApplyLocalInputMode(WorldMapWidget);
    }
    return true;
}

void ATMOPPlayerCharacter::CloseWorldMap()
{
    if (!bWorldMapOpen) return;
    bWorldMapOpen = false;
    if (IsValid(WorldMapWidget.Get())) WorldMapWidget->SetMapVisible(false);
    UpdateGameplayHUDVisibility();
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        ApplyLocalInputMode();
    }
    if (IsValid(Clock)) Clock->ReleasePause(this, TEXT("WorldMap"));
    bClockWasRunningBeforeMap = false;
}

void ATMOPPlayerCharacter::ToggleWorldMap()
{
    if (bWorldMapOpen) CloseWorldMap();
    else OpenWorldMap();
}

bool ATMOPPlayerCharacter::DiscoverEvidence(const FName EvidenceId)
{
    if (EvidenceId.IsNone() || DiscoveredEvidenceIds.Contains(EvidenceId))
        return false;
    DiscoveredEvidenceIds.Add(EvidenceId);
    return true;
}

bool ATMOPPlayerCharacter::OpenNewspaper(
    UTMOPNewspaperItemDefinition* Newspaper)
{
    if (bAddressDirectoryOpen) return false;
    if (!IsValid(Newspaper) || Newspaper->Pages.IsEmpty() ||
        bPauseMenuOpen || bWorldMapOpen || bNewspaperOpen || bLoopEndMenuOpen ||
        !bGameplayHUDVisible || !IsValid(NewspaperReaderWidget.Get()))
        return false;
    if (bDialogOpen) ClosePersonDialog();
    if (IsValid(InventoryInput.Get())) InventoryInput->CancelRadialMenu();
    if (bAgentInfoChartOpen) CloseAgentInfoChart();
    if (!NewspaperReaderWidget->OpenNewspaper(Newspaper)) return false;
    if (!IsValid(NewspaperReading) || !NewspaperReading->BeginReading(Newspaper, 0))
    {
        NewspaperReaderWidget->DismissReader();
        return false;
    }

    bNewspaperOpen = true;
    bNewspaperPausedSimulation = Newspaper->bPauseSimulationWhileReading;
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (bNewspaperPausedSimulation)
    {
        if (IsValid(Clock)) Clock->RequestPause(this, TEXT("Newspaper"));
    }

    APlayerController* PC = Cast<APlayerController>(Controller);
    if (IsValid(PC))
    {
        PC->bShowMouseCursor = true;
        ApplyLocalInputMode(NewspaperReaderWidget);
        // SetInputMode supplies the initial Slate focus target, while this
        // explicit user focus also covers readers opened during a paused game.
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
    }
    return true;
}

void ATMOPPlayerCharacter::CloseNewspaper()
{
    if (!bNewspaperOpen) return;
    if (IsValid(NewspaperReading)) NewspaperReading->EndReading();
    if (IsValid(NewspaperReaderWidget.Get()))
    {
        NewspaperReaderWidget->DismissReader();
    }

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (IsValid(PC))
    {
        PC->bShowMouseCursor = false;
        ApplyLocalInputMode();
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
    }
    if (IsValid(Clock)) Clock->ReleasePause(this, TEXT("Newspaper"));
    bClockWasRunningBeforeNewspaper = false;
    bNewspaperPausedSimulation = false;
    bNewspaperOpen = false;
}

void ATMOPPlayerCharacter::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsLocallyControlled()) return;
    if (UGameplayStatics::IsGamePaused(this))
    {
        GetCharacterMovement()->StopMovementImmediately();
        InputMoveCompleted();
        SetSprinting(false, false);
    }
    if (bAddressDirectoryOpen && (!ActiveInspection.IsValid() ||
        !ActiveInspection->HasReadableContent() ||
        FVector::DistSquared(GetActorLocation(), ActiveInspection->GetInteractionLocation()) >
            FMath::Square(InteractionDistance + 100.0f)))
        CloseAddressDirectory();
    if (bDialogOpen) UpdateDialogCloseUp(DeltaSeconds);
    if (!bPlayerInterfaceInitialized) InitializePlayerInterface();
    if (bDialogOpen)
    {
        const ATMOPHistoricalAgent* Agent = ActiveDialogAgent.Get();
        if (!IsValid(Agent) ||
            FVector::DistSquared(GetActorLocation(), Agent->GetActorLocation()) >
                FMath::Square(DialogMaximumDistanceCm))
            ClosePersonDialog();
    }
    if (!IsSessionGameplayBlocked() && !bAddressDirectoryOpen && bUseDirectSprintKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bSprintHeld = IsMappedActionHeld(PC, DefaultMappingContext, SprintAction,
            SprintFallbackKey, EKeys::Gamepad_LeftThumbstick);
        const bool bExtraHeld = bSprintHeld && IsValid(PC)
            && PC->IsInputKeyDown(ExtraSprintModifierKey);
        if (IsValid(VehicleSession.Get()) && VehicleSession->IsDrivingVehicle())
        {
            VehicleSession->VehicleHighSpeedMode(bSprintHeld);
            if (bIsSprinting) SetSprinting(false, false);
        }
        else
        {
            if (IsValid(VehicleSession.Get()))
                VehicleSession->VehicleHighSpeedMode(false);
            SetSprinting(bSprintHeld, bExtraHeld);
        }
    }
    if (!IsSessionGameplayBlocked() && !bAddressDirectoryOpen && bUseDirectQuickInventoryKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) &&
            (PC->IsInputKeyDown(QuickInventoryFallbackKey) ||
             PC->IsInputKeyDown(QuickInventoryGamepadFallbackKey));
        if (bKeyHeld != bQuickInventoryFallbackHeld)
        {
            bQuickInventoryFallbackHeld = bKeyHeld;
            if (bKeyHeld) InputQuickInventoryStarted();
            else InputQuickInventoryCompleted();
        }
    }
    if (!bNewspaperOpen && bUseDirectPauseKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) &&
            (PC->IsInputKeyDown(PauseMenuFallbackKey) ||
             PC->IsInputKeyDown(PauseMenuGamepadFallbackKey));
        if (bKeyHeld != bPauseFallbackHeld)
        {
            bPauseFallbackHeld = bKeyHeld;
            if (bKeyHeld)
            {
                if (InventoryInput->bRadialMenuOpen) FinishQuickInventory(false);
                else if (AddressDirectoryClosedFrame != GFrameCounter)
                {
                    if (bAddressDirectoryOpen) CloseAddressDirectory();
                    else TogglePauseMenu();
                }
            }
        }
    }
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) && (PC->IsInputKeyDown(WorldMapFallbackKey) ||
            PC->IsInputKeyDown(WorldMapGamepadFallbackKey));
        if (bKeyHeld != bMapFallbackHeld)
        {
            bMapFallbackHeld = bKeyHeld;
            if (bKeyHeld && !bPauseMenuOpen && !bNewspaperOpen)
                ToggleWorldMap();
        }
    }
    if (!bAddressDirectoryOpen && !bNewspaperOpen && !bWorldMapOpen && bUseDirectDropKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsValid(PC) && PC->IsInputKeyDown(DropItemFallbackKey);
        if (bKeyHeld != bDropFallbackHeld)
        {
            bDropFallbackHeld = bKeyHeld;
            if (bKeyHeld) DropEquippedItem();
        }
    }
    if (!bNewspaperOpen && !bWorldMapOpen && bUseDirectInteractKeyFallback)
    {
        const APlayerController* PC = Cast<APlayerController>(Controller);
        const bool bKeyHeld = IsMappedActionHeld(PC, DefaultMappingContext, InteractAction,
            InteractFallbackKey, EKeys::Gamepad_FaceButton_Left);
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
    FText TargetTitle;
    FText TargetDetails;
    CurrentInteractionTarget = nullptr;
    CurrentInformationTarget = nullptr;
    if (bGameplayHUDVisible && !bAddressDirectoryOpen && !bAgentInfoChartOpen &&
        !bPauseMenuOpen && !bWorldMapOpen && !bNewspaperOpen && !bDialogOpen &&
        !InventoryInput->bRadialMenuOpen &&
        (!IsValid(VehicleSession.Get()) || !VehicleSession->IsInVehicle()))
    {
        AActor* InformationTarget = FindInformationTarget();
        CurrentInformationTarget = InformationTarget;
        const float DistanceMetres = IsValid(InformationTarget)
            ? FVector::Dist(GetActorLocation(), InformationTarget->GetActorLocation()) / 100.0f
            : 0.0f;
        if (const ATMOPHistoricalAgent* Agent =
            Cast<ATMOPHistoricalAgent>(InformationTarget))
        {
            TargetTitle = Agent->GetInGameDisplayName();
            TargetDetails = FText::FromString(FString::Printf(
                TEXT("Person  ·  %.1f m"), DistanceMetres));
        }
        else if (const ATMOPVehicleBase* Vehicle =
            Cast<ATMOPVehicleBase>(InformationTarget))
        {
            TargetTitle = !Vehicle->DisplayName.IsEmpty()
                ? Vehicle->DisplayName
                : (!Vehicle->VehicleId.IsNone()
                    ? FText::FromName(Vehicle->VehicleId)
                    : NSLOCTEXT("TMOP", "UnknownTargetVehicle", "Okänt fordon"));
            TargetDetails = FText::FromString(FString::Printf(
                TEXT("Fordon  ·  %.1f m"), DistanceMetres));
        }
        else if (const ATMOPWorldItem* WorldItem =
            Cast<ATMOPWorldItem>(InformationTarget))
        {
            TargetTitle = IsValid(WorldItem->ItemDefinition.Get()) &&
                !WorldItem->ItemDefinition->DisplayName.IsEmpty()
                ? WorldItem->ItemDefinition->DisplayName
                : NSLOCTEXT("TMOP", "UnknownTargetItem", "Föremål");
            TargetDetails = FText::FromString(FString::Printf(
                TEXT("Föremål  ·  %.1f m"), DistanceMetres));
        }
        else if (const UTMOPInspectableComponent* Inspection = IsValid(InformationTarget)
            ? InformationTarget->FindComponentByClass<UTMOPInspectableComponent>() : nullptr)
        {
            TargetTitle = Inspection->GetInspectionTitle();
            TargetDetails = Inspection->GetInspectionCategory();
        }

        if (IsValid(InformationTarget))
        {
            FVector BoundsOrigin;
            FVector BoundsExtent;
            InformationTarget->GetActorBounds(
                false, BoundsOrigin, BoundsExtent, true);
            FVector MarkerWorldLocation = BoundsOrigin;
            if (const auto* Inspection = InformationTarget->FindComponentByClass<UTMOPInspectableComponent>())
                MarkerWorldLocation = Inspection->GetInteractionLocation();
            if (InformationTarget->IsA<ATMOPHistoricalAgent>())
                MarkerWorldLocation.Z += BoundsExtent.Z *
                    PersonTargetMarkerHeightFraction;

            APlayerController* PlayerController =
                Cast<APlayerController>(Controller);
            FVector2D ScreenPosition;
            if (IsValid(PlayerController) &&
                PlayerController->ProjectWorldLocationToScreen(
                    MarkerWorldLocation, ScreenPosition, true))
            {
                FVector2D ViewOrigin, ViewSize;
                UTMOPLocalMultiplayerSubsystem::GetPlayerViewRect(PlayerController, ViewOrigin, ViewSize);
                const float ViewportScale = FMath::Max(0.01f,
                    UWidgetLayoutLibrary::GetViewportScale(this));
                const FVector2D ViewportSize = ViewSize / ViewportScale;
                FVector2D SlatePosition = ScreenPosition / ViewportScale;
                const float Padding = TargetMarkerScreenEdgePadding /
                    ViewportScale;
                SlatePosition.X = FMath::Clamp(SlatePosition.X, Padding,
                    FMath::Max(Padding, ViewportSize.X - Padding));
                SlatePosition.Y = FMath::Clamp(SlatePosition.Y, Padding,
                    FMath::Max(Padding, ViewportSize.Y - Padding));
                InteractionPromptWidget->SetRenderTranslation(
                    SlatePosition - ViewportSize * 0.5f);
            }
            else
            {
                InteractionPromptWidget->SetRenderTranslation(
                    FVector2D::ZeroVector);
            }
        }
        else
        {
            InteractionPromptWidget->SetRenderTranslation(
                FVector2D::ZeroVector);
        }

        AActor* Target = FindInteractionTargetForInformation(InformationTarget);
        if (const auto* Inspection = IsValid(InformationTarget)
            ? InformationTarget->FindComponentByClass<UTMOPInspectableComponent>() : nullptr)
        {
            Target = Inspection->HasReadableContent() && FVector::DistSquared(
                GetActorLocation(), Inspection->GetInteractionLocation()) <= FMath::Square(InteractionDistance)
                ? InformationTarget : nullptr;
            if (!Target) TargetDetails = NSLOCTEXT("TMOP", "AddressTooFar", "Gå närmare för att läsa");
        }
        CurrentInteractionTarget = Target;
        if (const ATMOPHistoricalAgent* Agent =
            Cast<ATMOPHistoricalAgent>(Target))
        {
            const FText Name = Agent->GetInGameDisplayName();
            Prompt = FText::Format(
                NSLOCTEXT("TMOP", "InspectPerson", "Visa personakt: {0}"), Name);
        }
        else if (Cast<ATMOPVehicleBase>(Target))
            Prompt = NSLOCTEXT("TMOP", "EnterTargetVehicle", "Hoppa in");
        else if (const auto* Inspection = IsValid(Target)
            ? Target->FindComponentByClass<UTMOPInspectableComponent>() : nullptr)
            Prompt = Inspection->GetInspectionAction();
        else if (IsValid(Target) && Target->GetClass()->ImplementsInterface(
            UTMOPInteractable::StaticClass()))
            Prompt = ITMOPInteractable::Execute_GetInteractionText(Target);
    }
    if (!Prompt.IsEmpty())
        Prompt = FText::Format(NSLOCTEXT("TMOP", "InteractionWithKey", "[{0}] {1}"),
            GetInteractKeyDisplayText(), Prompt);
    if (!IsValid(CurrentInformationTarget.Get()))
        InteractionPromptWidget->SetRenderTranslation(FVector2D::ZeroVector);
    InteractionPromptWidget->SetTargetInformation(TargetTitle, TargetDetails);
    InteractionPromptWidget->SetPromptText(Prompt);
}

bool ATMOPPlayerCharacter::OpenAddressDirectory(UTMOPAddressComponent* Address)
{
    return OpenInformation(Address);
}

bool ATMOPPlayerCharacter::OpenInformation(UTMOPInspectableComponent* Inspection)
{
    APlayerController* PC = Cast<APlayerController>(Controller);
    UCameraComponent* Camera = GetGameplayCamera();
    if (!IsValid(PC) || !PC->IsLocalController() || !IsValid(Inspection) ||
        !Inspection->HasReadableContent() || !IsValid(Camera) ||
        bAddressDirectoryOpen || bPauseMenuOpen || bWorldMapOpen || bNewspaperOpen ||
        bDialogOpen || bAgentInfoChartOpen || !bGameplayHUDVisible ||
        (IsValid(VehicleSession.Get()) && VehicleSession->IsInVehicle()) ||
        FVector::DistSquared(GetActorLocation(), Inspection->GetInteractionLocation()) >
            FMath::Square(InteractionDistance) ||
        !Inspection->IsVisibleFrom(Camera->GetComponentLocation(), this)) return false;

    if (!IsValid(AddressDirectoryWidget.Get()))
    {
        AddressDirectoryWidget = CreateWidget<UTMOPAddressDirectoryWidget>(
            PC, UTMOPAddressDirectoryWidget::StaticClass());
        if (!IsValid(AddressDirectoryWidget.Get())) return false;
        AddressDirectoryWidget->InitializeDirectory(this);
        AddressDirectoryWidget->AddToPlayerScreen(88);
    }
    if (IsValid(InventoryInput.Get())) InventoryInput->CancelRadialMenu();
    ActiveInspection = Inspection;
    bAddressDirectoryOpen = true;
    if (IsValid(CameraPerspective.Get())) CameraPerspective->CancelLookZoom();
    AddressDirectoryWidget->ShowInformation(Inspection->GetInspectionTitle(), Inspection->GetInspectionText(),
        Inspection->GetInspectionCategory(), Inspection->GetInspectionSource());
    GetCharacterMovement()->StopMovementImmediately();
    SetSprinting(false, false);
    PC->bShowMouseCursor = true;
    PC->SetIgnoreMoveInput(true);
    PC->SetIgnoreLookInput(true);
    ApplyLocalInputMode(AddressDirectoryWidget);
    UpdateInteractionPrompt();
    return true;
}

void ATMOPPlayerCharacter::CloseAddressDirectory()
{
    if (!bAddressDirectoryOpen) return;
    bAddressDirectoryOpen = false;
    ActiveInspection.Reset();
    AddressDirectoryClosedFrame = GFrameCounter;
    if (IsValid(AddressDirectoryWidget.Get())) AddressDirectoryWidget->HideDirectory();
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        ApplyLocalInputMode();
        // Consume keys used by the UI so direct fallback polling cannot reopen it.
        bInteractFallbackHeld = PC->IsInputKeyDown(InteractFallbackKey);
        bPauseFallbackHeld = PC->IsInputKeyDown(PauseMenuFallbackKey) ||
            PC->IsInputKeyDown(PauseMenuGamepadFallbackKey);
    }
}

bool ATMOPPlayerCharacter::OpenPersonDialog(
    ATMOPHistoricalAgent* HistoricalAgent)
{
    if (!IsValid(HistoricalAgent) || !IsValid(DialogWidget.Get()) ||
        GetWorld() == nullptr || IsSessionGameplayBlocked() || HistoricalAgent->IsDialogueFocused())
        return false;

    const FName EntityId = IsValid(HistoricalAgent->EntityIdentity.Get())
        ? HistoricalAgent->EntityIdentity->GetEntityId()
        : NAME_None;
    ATMOPPersonRegistryDirector* Registry = nullptr;
    for (TActorIterator<ATMOPPersonRegistryDirector> It(GetWorld()); It; ++It)
    {
        Registry = *It;
        break;
    }

    bool bAfterShot = false;
    if (UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
        bAfterShot = Clock->GetCurrentTime().ToSecondsFromMidnight() >=
            DialogShotThreshold.ToSecondsFromMidnight();

    FText Dialog = IsValid(Registry)
        ? Registry->GetPersonDialog(EntityId, bAfterShot)
        : FText::GetEmpty();
    if (Dialog.IsEmpty())
        Dialog = NSLOCTEXT(
            "TMOP", "EmptyPersonDialog", "Jag har inget att säga just nu.");

    const FText Speaker = HistoricalAgent->GetInGameDisplayName();

    ActiveDialogAgent = HistoricalAgent;
    bDialogOpen = true;
    HistoricalAgent->BeginDialogueFocus(this);
    BeginDialogCloseUp(HistoricalAgent);
    DialogWidget->ShowDialog(Speaker, Dialog);
    if (IsValid(InteractionPromptWidget.Get()))
        InteractionPromptWidget->SetPromptText(FText::GetEmpty());

    GetCharacterMovement()->StopMovementImmediately();
    SetSprinting(false, false);
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = true;
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        ApplyLocalInputMode(DialogWidget);
    }
    return true;
}

void ATMOPPlayerCharacter::ClosePersonDialog()
{
    if (!bDialogOpen) return;
    if (ATMOPHistoricalAgent* Agent = ActiveDialogAgent.Get())
        Agent->EndDialogueFocus();
    ActiveDialogAgent.Reset();
    bDialogOpen = false;
    EndDialogCloseUp();
    if (IsValid(DialogWidget.Get())) DialogWidget->HideDialog();

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        ApplyLocalInputMode();
    }
}

bool ATMOPPlayerCharacter::OpenAgentInfoChart(
    ATMOPHistoricalAgent* HistoricalAgent)
{
    if (!IsValid(HistoricalAgent) || !IsValid(AgentInfoChartWidget.Get()) ||
        IsSessionGameplayBlocked())
        return false;

    UTMOPPersonProfileComponent* ProfileComponent =
        HistoricalAgent->PersonProfile.Get();
    if (!IsValid(ProfileComponent))
        ProfileComponent = HistoricalAgent->FindComponentByClass<
            UTMOPPersonProfileComponent>();
    if (!IsValid(ProfileComponent)) return false;
    if (!ProfileComponent->bHasLoadedProfile && !ProfileComponent->LoadProfile())
        return false;

    const FTMOPPersonProfileRow& Profile = ProfileComponent->Profile;
    FText TimelineSummary = Profile.AgentTimelineSummary;
    if (TimelineSummary.IsEmpty())
    {
        TArray<FString> Lines;
        for (const FTMOPPersonTimelineEntry& Entry : Profile.Timeline)
        {
            FString Place = !Entry.PlannedAnchorDisplayName.IsEmpty()
                ? Entry.PlannedAnchorDisplayName.ToString()
                : Entry.TargetAnchorId.ToString().Replace(TEXT("_"), TEXT(" "));
            if ((Place.IsEmpty() || Place == TEXT("None")) &&
                !Entry.TargetEntityId.IsNone())
                Place = Entry.TargetEntityId.ToString().Replace(TEXT("_"), TEXT(" "));
            if (Place == TEXT("None")) Place.Reset();
            FString Description;
            switch (Entry.Action)
            {
            case ETMOPPersonTimelineAction::InitialPlacement:
            case ETMOPPersonTimelineAction::Spawn:
                Description = Place.IsEmpty() ? TEXT("Jag kommer in i händelseförloppet.")
                    : FString::Printf(TEXT("Jag befinner mig vid %s."), *Place);
                break;
            case ETMOPPersonTimelineAction::MoveToAnchor:
                Description = Place.IsEmpty() ? TEXT("Jag går vidare.")
                    : FString::Printf(TEXT("Jag går mot %s."), *Place);
                break;
            case ETMOPPersonTimelineAction::Wait:
                Description = Place.IsEmpty() ? TEXT("Jag väntar en stund.")
                    : FString::Printf(TEXT("Jag väntar vid %s."), *Place);
                break;
            case ETMOPPersonTimelineAction::SitDown:
                Description = TEXT("Jag sätter mig ned."); break;
            case ETMOPPersonTimelineAction::StandUp:
                Description = TEXT("Jag reser mig upp."); break;
            case ETMOPPersonTimelineAction::EnterVehicle:
                Description = Place.IsEmpty() ? TEXT("Jag stiger in i ett fordon.")
                    : FString::Printf(TEXT("Jag stiger in i %s."), *Place);
                break;
            case ETMOPPersonTimelineAction::ExitVehicle:
                Description = TEXT("Jag stiger ur fordonet."); break;
            case ETMOPPersonTimelineAction::BeginDriving:
                Description = Place.IsEmpty() ? TEXT("Jag börjar köra.")
                    : FString::Printf(TEXT("Jag kör mot %s."), *Place);
                break;
            case ETMOPPersonTimelineAction::Despawn:
                Description = TEXT("Jag lämnar det simulerade området."); break;
            default:
                Description = !Entry.Notes.IsEmpty() ? Entry.Notes
                    : TEXT("Nästa dokumenterade händelse inträffar.");
                break;
            }
            Lines.Add(FString::Printf(TEXT("%s - %s"),
                *Entry.Time.ToDisplayString(), *Description));
        }
        TimelineSummary = Lines.IsEmpty() ? FText::GetEmpty()
            : FText::FromString(FString::Join(Lines, TEXT("\n\n")));
    }

    const bool bPoliceInterviewed = Profile.bPoliceInterviewed ||
        Profile.EvidenceIcon == ETMOPEntityEvidenceIcon::PoliceInterview;
    AgentInfoChartWidget->ShowAgentInfo(
        Profile, TimelineSummary, bPoliceInterviewed);
    bAgentInfoChartOpen = true;
    GetCharacterMovement()->StopMovementImmediately();
    SetSprinting(false, false);
    if (IsValid(InteractionPromptWidget.Get()))
        InteractionPromptWidget->SetPromptText(FText::GetEmpty());

    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = true;
        PC->SetIgnoreMoveInput(true);
        PC->SetIgnoreLookInput(true);
        ApplyLocalInputMode(AgentInfoChartWidget);
    }
    return true;
}

void ATMOPPlayerCharacter::CloseAgentInfoChart()
{
    if (!bAgentInfoChartOpen) return;
    bAgentInfoChartOpen = false;
    if (IsValid(AgentInfoChartWidget.Get()))
        AgentInfoChartWidget->HideAgentInfo();
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        PC->bShowMouseCursor = false;
        PC->SetIgnoreMoveInput(false);
        PC->SetIgnoreLookInput(false);
        ApplyLocalInputMode();
    }
}

void ATMOPPlayerCharacter::BeginDialogCloseUp(
    ATMOPHistoricalAgent* HistoricalAgent)
{
    if (!bEnableDialogCloseUp || !IsValid(HistoricalAgent) ||
        GetWorld() == nullptr)
        return;
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC)) return;

    PreDialogViewTarget = PC->GetViewTarget();
    DialogCameraActor = GetWorld()->SpawnActor<ACameraActor>(
        ACameraActor::StaticClass(), FTransform::Identity);
    if (!IsValid(DialogCameraActor)) return;
    DialogCameraActor->GetCameraComponent()->SetFieldOfView(
        DialogCameraFieldOfView);
    UpdateDialogCloseUp(0.0f);
    PC->SetViewTargetWithBlend(DialogCameraActor,
        DialogCameraBlendSeconds, EViewTargetBlendFunction::VTBlend_Cubic);
}

void ATMOPPlayerCharacter::UpdateDialogCloseUp(const float DeltaSeconds)
{
    ATMOPHistoricalAgent* Agent = ActiveDialogAgent.Get();
    if (!IsValid(DialogCameraActor) || !IsValid(Agent)) return;

    const FVector Focus = Agent->GetActorLocation() +
        FVector(0.0f, 0.0f, DialogFaceHeightCm);
    FVector TowardPlayer = GetActorLocation() - Agent->GetActorLocation();
    TowardPlayer.Z = 0.0f;
    if (!TowardPlayer.Normalize())
        TowardPlayer = -Agent->GetActorForwardVector().GetSafeNormal2D();
    const FVector Side = FVector::CrossProduct(FVector::UpVector, TowardPlayer);
    FVector DesiredLocation = Focus + TowardPlayer * DialogCameraDistanceCm +
        Side * DialogCameraSideOffsetCm + FVector(0.0f, 0.0f, 8.0f);

    FCollisionObjectQueryParams StaticOnly;
    StaticOnly.AddObjectTypesToQuery(ECC_WorldStatic);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TMOPDialogCamera), false);
    Query.AddIgnoredActor(this);
    Query.AddIgnoredActor(Agent);
    FHitResult Hit;
    if (GetWorld()->LineTraceSingleByObjectType(Hit, Focus, DesiredLocation,
        StaticOnly, Query))
        DesiredLocation = Hit.Location + Hit.Normal * 12.0f;

    const FRotator DesiredRotation = (Focus - DesiredLocation).Rotation();
    if (DeltaSeconds <= 0.0f)
    {
        DialogCameraActor->SetActorLocationAndRotation(
            DesiredLocation, DesiredRotation);
        return;
    }
    DialogCameraActor->SetActorLocationAndRotation(
        FMath::VInterpTo(DialogCameraActor->GetActorLocation(), DesiredLocation,
            DeltaSeconds, DialogCameraTrackingSpeed),
        FMath::RInterpTo(DialogCameraActor->GetActorRotation(), DesiredRotation,
            DeltaSeconds, DialogCameraTrackingSpeed));
}

void ATMOPPlayerCharacter::EndDialogCloseUp()
{
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (IsValid(PC) && IsValid(DialogCameraActor))
    {
        AActor* ReturnTarget = PreDialogViewTarget.IsValid()
            ? PreDialogViewTarget.Get() : this;
        PC->SetViewTargetWithBlend(ReturnTarget, DialogCameraBlendSeconds,
            EViewTargetBlendFunction::VTBlend_Cubic);
        DialogCameraActor->SetLifeSpan(DialogCameraBlendSeconds + 0.15f);
    }
    DialogCameraActor = nullptr;
    PreDialogViewTarget.Reset();
}

void ATMOPPlayerCharacter::InputQuickInventoryStarted()
{
    if (IsSessionGameplayBlocked()) return;
    if (bAddressDirectoryOpen) return;
    if (bPauseMenuOpen || !InventoryInput->OpenRadialMenu()) return;
    SetSprinting(false, false);
    GetCharacterMovement()->StopMovementImmediately();
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        FVector2D Origin, Size;
        UTMOPLocalMultiplayerSubsystem::GetPlayerViewRect(PC, Origin, Size);
        if (UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(this) == 0)
            PC->SetMouseLocation(FMath::RoundToInt(Origin.X + Size.X * 0.5f),
                FMath::RoundToInt(Origin.Y + Size.Y * 0.5f));
        PC->bShowMouseCursor = true;
        ApplyLocalInputMode(QuickInventoryWidget);
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
            ApplyLocalInputMode();
        }
    }
}

void ATMOPPlayerCharacter::UpdateQuickInventoryPointer()
{
    APlayerController* PC = Cast<APlayerController>(Controller);
    if (!IsValid(PC)) return;

    float MouseX = 0.0f, MouseY = 0.0f;
    FVector2D Origin, Size;
    UTMOPLocalMultiplayerSubsystem::GetPlayerViewRect(PC, Origin, Size);
    const FVector2D Stick(PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftX),
        PC->GetInputAnalogKeyState(EKeys::Gamepad_LeftY));
    if (Stick.SizeSquared() > 0.04f) InventoryInput->UpdateRadialSelection(Stick);
    else if (UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(this) == 0 &&
        PC->GetMousePosition(MouseX, MouseY) && Size.X > 0 && Size.Y > 0)
    {
        const FVector2D Direction(MouseX - Origin.X - Size.X * 0.5f,
            Origin.Y + Size.Y * 0.5f - MouseY);
        const float Normalizer = FMath::Max(1.0f, FMath::Min(Size.X, Size.Y) * 0.22f);
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
    if (bAddressDirectoryOpen) return;
    InventoryInput->UpdateRadialSelection(Value.Get<FVector2D>());
}

void ATMOPPlayerCharacter::InputInventoryCycle(const FInputActionValue& Value)
{
    if (bAddressDirectoryOpen) return;
    const float Direction = Value.Get<float>();
    if (!FMath::IsNearlyZero(Direction)) InventoryInput->CycleInventory(Direction > 0.0f ? 1 : -1);
}

UCameraComponent* ATMOPPlayerCharacter::GetGameplayCamera() const
{
    if (IsValid(CameraPerspective.Get())) return CameraPerspective->GetActivePerspectiveCamera();
    return IsValid(FollowCamera.Get()) && FollowCamera->IsActive() ? FollowCamera.Get() : nullptr;
}

AActor* ATMOPPlayerCharacter::FindInteractionTarget() const
{
    return FindInteractionTargetForInformation(FindInformationTarget());
}

AActor* ATMOPPlayerCharacter::FindInteractionTargetForInformation(AActor* InformationTarget) const
{
    UCameraComponent* Camera = GetGameplayCamera();
    if (!IsValid(Camera) || GetWorld() == nullptr) return nullptr;
    // Readable-point interaction range is measured from the player, not the trailing camera.
    if (AActor* Hovered = InformationTarget)
        if (const auto* Inspection = Hovered->FindComponentByClass<UTMOPInspectableComponent>())
            return Inspection->HasReadableContent() && FVector::DistSquared(GetActorLocation(),
                Inspection->GetInteractionLocation()) <= FMath::Square(InteractionDistance)
                ? Hovered : nullptr;
    const FVector Start = Camera->GetComponentLocation();
    const FVector Forward = Camera->GetForwardVector();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPPlayerInteraction), false, this);
    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(Overlaps, Start, FQuat::Identity,
        ObjectTypes, FCollisionShape::MakeSphere(InteractionDistance), Params);

    TArray<AActor*> Candidates;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* OverlapActor = Overlap.GetActor();
        if (!IsValid(OverlapActor)) continue;
        Candidates.Add(OverlapActor);
        if (const ATMOPVehicleBase* Vehicle = Cast<ATMOPVehicleBase>(OverlapActor))
            for (const UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
                if (IsValid(Seat) && IsValid(Seat->GetOccupantCharacter()))
                    Candidates.Add(Seat->GetOccupantCharacter());
    }

    AActor* BestTarget = nullptr;
    float BestScore = TNumericLimits<float>::Max();
    TSet<AActor*> TestedActors;
    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate) || Candidate == this || TestedActors.Contains(Candidate))
            continue;
        TestedActors.Add(Candidate);

        const bool bAgent = Candidate->IsA<ATMOPHistoricalAgent>();
        const bool bVehicle = Candidate->IsA<ATMOPVehicleBase>();
        const bool bInteractable = Candidate->GetClass()->ImplementsInterface(
            UTMOPInteractable::StaticClass());
        if (!bAgent && !bVehicle && !bInteractable) continue;

        FVector BoundsOrigin;
        FVector BoundsExtent;
        Candidate->GetActorBounds(false, BoundsOrigin, BoundsExtent, true);
        FVector AimPoint = BoundsOrigin;
        if (bAgent) AimPoint.Z += BoundsExtent.Z * 0.35f;

        const FVector ToTarget = AimPoint - Start;
        const float ForwardDistance = FVector::DotProduct(ToTarget, Forward);
        if (ForwardDistance <= 1.0f || ForwardDistance > InteractionDistance) continue;
        const float PerpendicularDistance =
            (ToTarget - Forward * ForwardDistance).Size();
        const float AngleDegrees = FMath::RadiansToDegrees(
            FMath::Atan2(PerpendicularDistance, ForwardDistance));
        const float AllowedAngle = bVehicle
            ? VehicleTargetConeDegrees : InteractionTargetConeDegrees;
        if (AngleDegrees > AllowedAngle) continue;

        FHitResult VisibilityHit;
        FCollisionQueryParams VisibilityParams(
            SCENE_QUERY_STAT(TMOPPlayerInteractionVisibility), false, this);
        const bool bBlocked = GetWorld()->LineTraceSingleByChannel(
            VisibilityHit, Start, AimPoint, ECC_Visibility, VisibilityParams);
        bool bVisible = !bBlocked || VisibilityHit.GetActor() == Candidate;
        if (!bVisible && bAgent && IsValid(VisibilityHit.GetActor()) &&
            VisibilityHit.GetActor()->IsA<ATMOPVehicleBase>())
        {
            FVector VehicleOrigin;
            FVector VehicleExtent;
            VisibilityHit.GetActor()->GetActorBounds(
                false, VehicleOrigin, VehicleExtent, true);
            const FBox ExpandedVehicleBounds(
                VehicleOrigin - VehicleExtent * 1.25f,
                VehicleOrigin + VehicleExtent * 1.25f);
            bVisible = Candidate->GetAttachParentActor() == VisibilityHit.GetActor() ||
                ExpandedVehicleBounds.IsInsideOrOn(AimPoint);
        }
        if (!bVisible) continue;

        const float Score = AngleDegrees / FMath::Max(AllowedAngle, 0.1f) +
            ForwardDistance / FMath::Max(InteractionDistance, 1.0f) * 0.04f;
        if (Score < BestScore)
        {
            BestScore = Score;
            BestTarget = Candidate;
        }
    }
    return BestTarget;
}

AActor* ATMOPPlayerCharacter::FindInformationTarget() const
{
    UCameraComponent* Camera = GetGameplayCamera();
    if (!IsValid(Camera) || GetWorld() == nullptr) return nullptr;

    const FVector CameraLocation = Camera->GetComponentLocation();
    const FVector CameraForward = Camera->GetForwardVector();
    const FVector CharacterLocation = GetActorLocation();
    const FVector CharacterForward = GetActorForwardVector();
    FCollisionQueryParams Params(SCENE_QUERY_STAT(TMOPPlayerInformationTarget),
        false, this);
    FCollisionObjectQueryParams ObjectTypes;
    ObjectTypes.AddObjectTypesToQuery(ECC_Pawn);
    ObjectTypes.AddObjectTypesToQuery(ECC_WorldDynamic);
    ObjectTypes.AddObjectTypesToQuery(ECC_PhysicsBody);

    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(Overlaps, CharacterLocation,
        FQuat::Identity, ObjectTypes,
        FCollisionShape::MakeSphere(TargetInformationDistance), Params);

    TArray<AActor*> Candidates;
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Actor = Overlap.GetActor();
        if (!IsValid(Actor)) continue;
        Candidates.AddUnique(Actor);
        if (const ATMOPVehicleBase* Vehicle = Cast<ATMOPVehicleBase>(Actor))
            for (const UTMOPVehicleSeatComponent* Seat : Vehicle->GetVehicleSeats())
                if (IsValid(Seat) && IsValid(Seat->GetOccupantCharacter()))
                    Candidates.AddUnique(Seat->GetOccupantCharacter());
    }

    AActor* BestDirectTarget = nullptr;
    AActor* BestFallbackTarget = nullptr;
    float BestDirectScore = TNumericLimits<float>::Max();
    float BestFallbackDistanceSquared = TNumericLimits<float>::Max();
    for (AActor* Candidate : Candidates)
    {
        if (!IsValid(Candidate) || Candidate == this) continue;
        const bool bSupportedTarget = Candidate->IsA<ATMOPHistoricalAgent>() ||
            Candidate->IsA<ATMOPVehicleBase>() || Candidate->IsA<ATMOPWorldItem>() ||
            Candidate->FindComponentByClass<UTMOPInspectableComponent>();
        if (!bSupportedTarget) continue;
        const auto* Inspection = Candidate->FindComponentByClass<UTMOPInspectableComponent>();
        if (Inspection && !Inspection->HasReadableContent()) continue;

        FVector BoundsOrigin;
        FVector BoundsExtent;
        Candidate->GetActorBounds(false, BoundsOrigin, BoundsExtent, true);
        FVector AimPoint = BoundsOrigin;
        if (Candidate->IsA<ATMOPHistoricalAgent>())
            AimPoint.Z += BoundsExtent.Z * 0.35f;
        if (Inspection) AimPoint = Inspection->GetInteractionLocation();

        const FVector CameraToTarget = AimPoint - CameraLocation;
        const float CameraDistance = CameraToTarget.Size();
        if (CameraDistance <= 1.0f || CameraDistance > TargetInformationDistance)
            continue;

        FHitResult VisibilityHit;
        FCollisionQueryParams VisibilityParams(
            SCENE_QUERY_STAT(TMOPPlayerInformationTargetVisibility), false, this);
        const bool bBlocked = !Inspection && GetWorld()->LineTraceSingleByChannel(
            VisibilityHit, CameraLocation, AimPoint, ECC_Visibility, VisibilityParams);
        bool bVisible = Inspection ? Inspection->IsVisibleFrom(CameraLocation, this)
            : (!bBlocked || VisibilityHit.GetActor() == Candidate);
        if (!bVisible && Candidate->IsA<ATMOPHistoricalAgent>() &&
            IsValid(VisibilityHit.GetActor()) &&
            VisibilityHit.GetActor()->IsA<ATMOPVehicleBase>())
            bVisible = Candidate->GetAttachParentActor() == VisibilityHit.GetActor();
        if (!bVisible) continue;

        const FVector CameraDirection = CameraToTarget / CameraDistance;
        const float CameraDot = FVector::DotProduct(CameraForward, CameraDirection);
        const float CameraAngle = FMath::RadiansToDegrees(
            FMath::Acos(FMath::Clamp(CameraDot, -1.0f, 1.0f)));
        if (CameraAngle <= DirectTargetConeDegrees)
        {
            const float Score = CameraAngle /
                FMath::Max(DirectTargetConeDegrees, 0.1f) +
                CameraDistance / FMath::Max(TargetInformationDistance, 1.0f) * 0.05f;
            if (Score < BestDirectScore)
            {
                BestDirectScore = Score;
                BestDirectTarget = Candidate;
            }
        }

        if (!Inspection && bUseFrontHemisphereTargetFallback)
        {
            const FVector CharacterToTarget = AimPoint - CharacterLocation;
            const float DistanceSquared = CharacterToTarget.SizeSquared();
            if (DistanceSquared > 1.0f &&
                FVector::DotProduct(CharacterForward,
                    CharacterToTarget.GetSafeNormal()) >= 0.0f &&
                DistanceSquared < BestFallbackDistanceSquared)
            {
                BestFallbackDistanceSquared = DistanceSquared;
                BestFallbackTarget = Candidate;
            }
        }
    }

    return IsValid(BestDirectTarget) ? BestDirectTarget : BestFallbackTarget;
}

FText ATMOPPlayerCharacter::GetInteractKeyDisplayText() const
{
    return InteractFallbackKey.IsValid()
        ? InteractFallbackKey.GetDisplayName(false)
        : FText::FromString(TEXT("E"));
}
