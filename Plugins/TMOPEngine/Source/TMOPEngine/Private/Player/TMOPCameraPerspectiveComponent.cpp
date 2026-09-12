#include "Player/TMOPCameraPerspectiveComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/MeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "TMOPLookZoomMath.h"

UTMOPCameraPerspectiveComponent::UTMOPCameraPerspectiveComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
}

void UTMOPCameraPerspectiveComponent::BeginPlay()
{
    Super::BeginPlay();
    DiscoverOrCreateCameras();
    SetFirstPerson(bStartInFirstPerson);
}

void UTMOPCameraPerspectiveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CancelLookZoom();
    RestoreFirstPersonMeshes();
    Super::EndPlay(EndPlayReason);
}

void UTMOPCameraPerspectiveComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    const APawn* Pawn = Cast<APawn>(GetOwner());
    const APlayerController* PC = IsValid(Pawn)
        ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    const bool bLocalInput = IsValid(PC) && PC->IsLocalController();
    const bool bKeyDown = bLocalInput && ToggleKey.IsValid() && PC->IsInputKeyDown(ToggleKey);
    const bool bGamepadDown = bLocalInput && GamepadToggleKey.IsValid() &&
        PC->IsInputKeyDown(GamepadToggleKey);
    if (bBindToggleKeysAutomatically &&
        ((bKeyDown && !bToggleKeyWasDown) || (bGamepadDown && !bGamepadToggleKeyWasDown)))
        TogglePerspective();
    // Track edges in menus too, so a held toggle does not fire when returning to play.
    bToggleKeyWasDown = bKeyDown;
    bGamepadToggleKeyWasDown = bGamepadDown;
    if (CanUseCameraControls()) UpdateFirstPersonMeshes();
    UpdateZoom(DeltaTime);
}

bool UTMOPCameraPerspectiveComponent::CanUseCameraControls() const
{
    const APawn* Pawn = Cast<APawn>(GetOwner());
    const APlayerController* PC = IsValid(Pawn)
        ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!IsValid(PC) || !PC->IsLocalController() || PC->GetViewTarget() != Pawn ||
        PC->bShowMouseCursor || PC->IsLookInputIgnored() || UGameplayStatics::IsGamePaused(this))
        return false;
    if (const ATMOPPlayerCharacter* Player = Cast<ATMOPPlayerCharacter>(Pawn))
    {
        if (Player->bPauseMenuOpen || Player->bWorldMapOpen || Player->bNewspaperOpen ||
            Player->bDialogOpen || Player->bAgentInfoChartOpen || Player->bAddressDirectoryOpen ||
            !Player->IsGameplayHUDVisible() ||
            (IsValid(Player->InventoryInput.Get()) && Player->InventoryInput->bRadialMenuOpen) ||
            (IsValid(Player->VehicleSession.Get()) && Player->VehicleSession->IsInVehicle()))
            return false;
    }
    return true;
}

void UTMOPCameraPerspectiveComponent::DiscoverOrCreateCameras()
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;

    TArray<UCameraComponent*> Cameras;
    Owner->GetComponents<UCameraComponent>(Cameras);
    if (!IsValid(ThirdPersonCamera))
        for (UCameraComponent* Camera : Cameras)
            if (IsValid(Camera) && Camera != FirstPersonCamera && Camera->IsActive())
            {
                ThirdPersonCamera = Camera;
                break;
            }
    if (!IsValid(ThirdPersonCamera))
        for (UCameraComponent* Camera : Cameras)
            if (IsValid(Camera) && Camera != FirstPersonCamera)
            {
                ThirdPersonCamera = Camera;
                break;
            }

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
    FirstPersonCamera->SetAutoActivate(false);
    Owner->AddInstanceComponent(FirstPersonCamera);
    FirstPersonCamera->RegisterComponent();
    FirstPersonCamera->SetActive(false);
}

UCameraComponent* UTMOPCameraPerspectiveComponent::GetActivePerspectiveCamera() const
{
    UCameraComponent* Camera = bIsFirstPerson ? FirstPersonCamera.Get() : ThirdPersonCamera.Get();
    return IsValid(Camera) && Camera->IsActive() ? Camera : nullptr;
}

void UTMOPCameraPerspectiveComponent::TogglePerspective()
{
    if (CanUseCameraControls()) SetFirstPerson(!bIsFirstPerson);
}

void UTMOPCameraPerspectiveComponent::SetFirstPerson(const bool bEnableFirstPerson)
{
    DiscoverOrCreateCameras();
    if (!IsValid(FirstPersonCamera) || (!bEnableFirstPerson && !IsValid(ThirdPersonCamera))) return;
    RestoreZoomCamera();
    bIsFirstPerson = bEnableFirstPerson;
    if (IsValid(ThirdPersonCamera))
        ThirdPersonCamera->SetActive(!bIsFirstPerson);
    FirstPersonCamera->SetRelativeLocation(FirstPersonCameraOffset);
    FirstPersonCamera->SetFieldOfView(FirstPersonFieldOfView);
    FirstPersonCamera->SetActive(bIsFirstPerson);

    UpdateFirstPersonMeshes();
}

void UTMOPCameraPerspectiveComponent::SetLookZoomAmount(const float Amount)
{
    ExternalZoomAmount = bEnableLookZoom && CanUseCameraControls()
        ? TMOPLookZoomMath::Amount(Amount) : 0.0f;
}

void UTMOPCameraPerspectiveComponent::RestoreZoomCamera()
{
    if (UCameraComponent* Camera = ZoomedCamera.Get()) Camera->SetFieldOfView(UnzoomedFieldOfView);
    ZoomedCamera.Reset();
    CurrentZoomAmount = 0.0f;
}

void UTMOPCameraPerspectiveComponent::CancelLookZoom()
{
    ExternalZoomAmount = 0.0f;
    PinchStartDistance = 0.0f;
    RestoreZoomCamera();
}

float UTMOPCameraPerspectiveComponent::ReadPinchZoom()
{
    const APawn* Pawn = Cast<APawn>(GetOwner());
    const APlayerController* PC = IsValid(Pawn)
        ? Cast<APlayerController>(Pawn->GetController()) : nullptr;
    if (!bEnableTouchPinchZoom || !IsValid(PC))
    {
        PinchStartDistance = 0.0f;
        return 0.0f;
    }
    float X1 = 0.0f, Y1 = 0.0f, X2 = 0.0f, Y2 = 0.0f;
    bool bTouch1 = false, bTouch2 = false;
    PC->GetInputTouchState(ETouchIndex::Touch1, X1, Y1, bTouch1);
    PC->GetInputTouchState(ETouchIndex::Touch2, X2, Y2, bTouch2);
    if (!bTouch1 || !bTouch2)
    {
        PinchStartDistance = 0.0f;
        return 0.0f;
    }
    const float Distance = FVector2D::Distance(FVector2D(X1, Y1), FVector2D(X2, Y2));
    if (PinchStartDistance < 1.0f) PinchStartDistance = Distance;
    return TMOPLookZoomMath::PinchAmount(Distance, PinchStartDistance, PinchFullZoomDistanceRatio);
}

void UTMOPCameraPerspectiveComponent::UpdateZoom(const float DeltaTime)
{
    if (!bEnableLookZoom || !CanUseCameraControls())
    {
        CancelLookZoom();
        return;
    }
    UCameraComponent* Camera = GetActivePerspectiveCamera();
    if (!IsValid(Camera))
    {
        CancelLookZoom();
        return;
    }
    if (ZoomedCamera.Get() != Camera) RestoreZoomCamera();
    float RequestedAmount = FMath::Max(ExternalZoomAmount, ReadPinchZoom());
    const APawn* Pawn = Cast<APawn>(GetOwner());
    const APlayerController* PC = Cast<APlayerController>(Pawn->GetController());
    if (bReadZoomKeysAutomatically)
    {
        if ((ZoomHoldKey.IsValid() && PC->IsInputKeyDown(ZoomHoldKey)) ||
            (ZoomHoldGamepadKey.IsValid() && PC->IsInputKeyDown(ZoomHoldGamepadKey)))
            RequestedAmount = 1.0f;
        if (ZoomAnalogKey.IsValid())
            RequestedAmount = FMath::Max(RequestedAmount,
                TMOPLookZoomMath::AnalogAmount(PC->GetInputAnalogKeyState(ZoomAnalogKey), ZoomAnalogDeadZone));
    }
    if (!ZoomedCamera.IsValid())
    {
        if (RequestedAmount <= 0.0f) return;
        ZoomedCamera = Camera;
        UnzoomedFieldOfView = Camera->FieldOfView;
    }
    const float Speed = FMath::IsFinite(ZoomBlendSpeed) ? FMath::Max(0.1f, ZoomBlendSpeed) : 10.0f;
    CurrentZoomAmount = FMath::FInterpTo(CurrentZoomAmount, RequestedAmount, DeltaTime, Speed);
    if (RequestedAmount <= 0.0f && CurrentZoomAmount < 0.001f)
    {
        RestoreZoomCamera();
        return;
    }
    Camera->SetFieldOfView(TMOPLookZoomMath::FieldOfView(
        UnzoomedFieldOfView, ZoomFieldOfView, CurrentZoomAmount));
}

float UTMOPCameraPerspectiveComponent::GetLookSensitivityScale() const
{
    const UCameraComponent* Camera = ZoomedCamera.Get();
    return bScaleLookSensitivityWithZoom && IsValid(Camera) && CanUseCameraControls()
        ? TMOPLookZoomMath::LookSensitivity(Camera->FieldOfView, UnzoomedFieldOfView) : 1.0f;
}

void UTMOPCameraPerspectiveComponent::UpdateFirstPersonMeshes()
{
    if (!bIsFirstPerson || !bHideOwnMeshInFirstPerson)
    {
        RestoreFirstPersonMeshes();
        return;
    }
    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;
    const bool bAlreadyHiding = bMeshHidingApplied;
    if (!bAlreadyHiding && IsValid(OwnerMesh)) bPreviousOwnerNoSee = OwnerMesh->bOwnerNoSee;
    TArray<UMeshComponent*> Meshes;
    Owner->GetComponents<UMeshComponent>(Meshes);
    for (UMeshComponent* Mesh : Meshes)
    {
        if (!IsValid(Mesh) || (Mesh != OwnerMesh && !Mesh->GetName().StartsWith(TEXT("TMOP_Player_"))))
            continue;
        if (!HiddenMeshes.ContainsByPredicate([Mesh](const FHiddenMeshState& State)
            { return State.Mesh.Get() == Mesh; }))
        {
            FHiddenMeshState State;
            State.Mesh = Mesh;
            State.bOwnerNoSee = Mesh->bOwnerNoSee;
            State.bCastHiddenShadow = Mesh->bCastHiddenShadow;
            // AppearanceDirector copies Body's OwnerNoSee to parts created during clothing changes.
            if (bAlreadyHiding && Mesh != OwnerMesh) State.bOwnerNoSee = bPreviousOwnerNoSee;
            HiddenMeshes.Add(State);
        }
        Mesh->SetOwnerNoSee(true);
        Mesh->SetCastHiddenShadow(true);
    }
    bMeshHidingApplied = true;
}

void UTMOPCameraPerspectiveComponent::RestoreFirstPersonMeshes()
{
    for (const FHiddenMeshState& State : HiddenMeshes)
        if (UMeshComponent* Mesh = State.Mesh.Get())
        {
            Mesh->SetOwnerNoSee(State.bOwnerNoSee);
            Mesh->SetCastHiddenShadow(State.bCastHiddenShadow);
        }
    HiddenMeshes.Reset();
    bMeshHidingApplied = false;
}
