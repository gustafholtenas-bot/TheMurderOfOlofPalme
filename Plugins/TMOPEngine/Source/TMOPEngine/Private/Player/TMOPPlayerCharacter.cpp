#include "Player/TMOPPlayerCharacter.h"

#include "Animation/TMOPAnimationStateComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Player/TMOPPlayerActionComponent.h"

ATMOPPlayerCharacter::ATMOPPlayerCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = false;
    bUseControllerRotationRoll = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 400.0f;
    CameraBoom->bUsePawnControlRotation = true;
    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;
    AnimationState = CreateDefaultSubobject<UTMOPAnimationStateComponent>(TEXT("AnimationState"));
    AnimationState->bDerivePostureAndMovementFromAgent = false;
    PlayerActions = CreateDefaultSubobject<UTMOPPlayerActionComponent>(TEXT("PlayerActions"));
}

void ATMOPPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
        if (ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
                LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
                if (IsValid(DefaultMappingContext)) Subsystem->AddMappingContext(DefaultMappingContext, 0);
}

void ATMOPPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
    UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(PlayerInputComponent);
    if (Input == nullptr) return;
    if (MoveAction) Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATMOPPlayerCharacter::InputMove);
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
    }
    if (InteractAction) Input->BindAction(InteractAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputInteract);
    if (PrimaryAction) Input->BindAction(PrimaryAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputPrimaryAction);
    if (SecondaryAction)
    {
        Input->BindAction(SecondaryAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputSecondaryActionStarted);
        Input->BindAction(SecondaryAction, ETriggerEvent::Completed, this, &ATMOPPlayerCharacter::InputSecondaryActionEnded);
    }
    if (CancelAction) Input->BindAction(CancelAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputCancel);
    if (SquatAction) Input->BindAction(SquatAction, ETriggerEvent::Started, this, &ATMOPPlayerCharacter::InputToggleSquat);
}

void ATMOPPlayerCharacter::InputMove(const FInputActionValue& Value)
{
    if (PlayerActions->bMovementBlocked) return;
    const FVector2D Axis = Value.Get<FVector2D>();
    const FRotator Rotation(0.0f, Controller != nullptr ? Controller->GetControlRotation().Yaw : 0.0f, 0.0f);
    AddMovementInput(FRotationMatrix(Rotation).GetUnitAxis(EAxis::X), Axis.Y);
    AddMovementInput(FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y), Axis.X);
}

void ATMOPPlayerCharacter::InputLook(const FInputActionValue& Value)
{
    const FVector2D Axis = Value.Get<FVector2D>();
    AddControllerYawInput(Axis.X);
    AddControllerPitchInput(Axis.Y);
}

void ATMOPPlayerCharacter::InputJumpStarted() { Jump(); }
void ATMOPPlayerCharacter::InputJumpEnded() { StopJumping(); }

void ATMOPPlayerCharacter::InputSprintStarted()
{
    if (PlayerActions->bMovementBlocked) return;
    GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
    AnimationState->SetLocomotionStyle(ETMOPAnimLocomotionStyle::FastRun);
}

void ATMOPPlayerCharacter::InputSprintEnded()
{
    GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
    AnimationState->SetLocomotionStyle(ETMOPAnimLocomotionStyle::Normal);
}

void ATMOPPlayerCharacter::InputInteract()
{
    PlayerActions->StartAction(ETMOPPlayerAction::Interact, FindInteractionTarget(), 0.35f, false);
}

void ATMOPPlayerCharacter::InputPrimaryAction()
{
    PlayerActions->StartAction(ETMOPPlayerAction::Punch, FindInteractionTarget(), 0.7f, true);
}

void ATMOPPlayerCharacter::InputSecondaryActionStarted()
{
    PlayerActions->StartAction(ETMOPPlayerAction::AimGun, FindInteractionTarget(), -1.0f, false);
}

void ATMOPPlayerCharacter::InputSecondaryActionEnded()
{
    if (PlayerActions->CurrentAction == ETMOPPlayerAction::AimGun) PlayerActions->CompleteCurrentAction();
}

void ATMOPPlayerCharacter::InputCancel() { PlayerActions->CancelCurrentAction(); }

void ATMOPPlayerCharacter::InputToggleSquat()
{
    if (AnimationState->Posture == ETMOPAnimPosture::Squatting)
        AnimationState->SetPostureOverride(ETMOPAnimPosture::Standing);
    else AnimationState->SetPostureOverride(ETMOPAnimPosture::Squatting);
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
