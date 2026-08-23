#include "Player/TMOPCameraPerspectiveComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

UTMOPCameraPerspectiveComponent::UTMOPCameraPerspectiveComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    ToggleKey = EKeys::V;
    GamepadToggleKey = EKeys::Gamepad_RightThumbstick;
}

void UTMOPCameraPerspectiveComponent::BeginPlay()
{
    Super::BeginPlay();
    DiscoverOrCreateCameras();
    TryBindInput();
    SetFirstPerson(bStartInFirstPerson);
}

void UTMOPCameraPerspectiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(OwnerMesh))
    {
        OwnerMesh->SetOwnerNoSee(bPreviousOwnerNoSee);
        OwnerMesh->SetCastHiddenShadow(bPreviousCastHiddenShadow);
    }
    Super::EndPlay(EndPlayReason);
}

void UTMOPCameraPerspectiveComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (!bInputBound) TryBindInput();
}

void UTMOPCameraPerspectiveComponent::DiscoverOrCreateCameras()
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;

    TArray<UCameraComponent*> Cameras;
    Owner->GetComponents<UCameraComponent>(Cameras);
    if (!IsValid(ThirdPersonCamera))
        for (UCameraComponent* Camera : Cameras)
            if (IsValid(Camera) && Camera->IsActive())
            {
                ThirdPersonCamera = Camera;
                break;
            }
    if (!IsValid(ThirdPersonCamera) && !Cameras.IsEmpty())
        ThirdPersonCamera = Cameras[0];

    if (ACharacter* Character = Cast<ACharacter>(Owner))
        OwnerMesh = Character->GetMesh();
    if (!IsValid(OwnerMesh))
        OwnerMesh = Owner->FindComponentByClass<USkeletalMeshComponent>();

    if (IsValid(FirstPersonCamera)) return;

    FirstPersonCamera = NewObject<UCameraComponent>(Owner,
        UCameraComponent::StaticClass(), TEXT("TMOPFirstPersonCamera"));
    if (!IsValid(FirstPersonCamera)) return;
    FirstPersonCamera->CreationMethod = EComponentCreationMethod::Instance;
    FirstPersonCamera->SetupAttachment(Owner->GetRootComponent());
    FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
    FirstPersonCamera->SetFieldOfView(FirstPersonFieldOfView);
    FirstPersonCamera->bUsePawnControlRotation = true;
    Owner->AddInstanceComponent(FirstPersonCamera);
    FirstPersonCamera->RegisterComponent();
    FirstPersonCamera->SetActive(false);

    if (IsValid(OwnerMesh))
    {
        bPreviousOwnerNoSee = OwnerMesh->bOwnerNoSee;
        bPreviousCastHiddenShadow = OwnerMesh->bCastHiddenShadow;
    }
}

void UTMOPCameraPerspectiveComponent::TryBindInput()
{
    if (bInputBound || !bBindToggleKeysAutomatically || !IsValid(GetOwner())) return;
    APawn* Pawn = Cast<APawn>(GetOwner());
    APlayerController* Controller = IsValid(Pawn)
        ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!IsValid(Controller) || !Controller->IsLocalController()) return;

    GetOwner()->EnableInput(Controller);
    UInputComponent* Input = GetOwner()->GetInputComponent();
    if (!IsValid(Input)) return;
    if (ToggleKey.IsValid())
        Input->BindKey(ToggleKey, IE_Pressed, this,
            &UTMOPCameraPerspectiveComponent::TogglePerspective);
    if (GamepadToggleKey.IsValid() && GamepadToggleKey != ToggleKey)
        Input->BindKey(GamepadToggleKey, IE_Pressed, this,
            &UTMOPCameraPerspectiveComponent::TogglePerspective);
    bInputBound = true;
}

void UTMOPCameraPerspectiveComponent::TogglePerspective()
{
    SetFirstPerson(!bIsFirstPerson);
}

void UTMOPCameraPerspectiveComponent::SetFirstPerson(const bool bEnableFirstPerson)
{
    DiscoverOrCreateCameras();
    if (!IsValid(FirstPersonCamera)) return;

    bIsFirstPerson = bEnableFirstPerson;
    if (IsValid(ThirdPersonCamera))
        ThirdPersonCamera->SetActive(!bIsFirstPerson);
    FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
    FirstPersonCamera->SetFieldOfView(FirstPersonFieldOfView);
    FirstPersonCamera->SetActive(bIsFirstPerson);

    if (IsValid(OwnerMesh) && bHideOwnMeshInFirstPerson)
    {
        OwnerMesh->SetOwnerNoSee(bIsFirstPerson ? true : bPreviousOwnerNoSee);
        // The local mesh is hidden from the owner, but its shadow may remain.
        OwnerMesh->SetCastHiddenShadow(bIsFirstPerson ? true : bPreviousCastHiddenShadow);
    }
}
