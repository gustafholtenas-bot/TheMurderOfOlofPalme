#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "TMOPCameraPerspectiveComponent.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;
class UMeshComponent;

UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPCameraPerspectiveComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPCameraPerspectiveComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera")
    bool bStartInFirstPerson = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera")
    bool bBindToggleKeysAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Input")
    FKey ToggleKey = EKeys::V;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Input")
    FKey GamepadToggleKey = EKeys::Gamepad_RightThumbstick;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom")
    bool bEnableLookZoom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Input")
    bool bReadZoomKeysAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Input")
    FKey ZoomHoldKey = EKeys::Z;

    /** Optional digital controller button. L2 is read as an analog axis by default. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Input")
    FKey ZoomHoldGamepadKey;

    /** Expected range: 0 at rest, 1 at full press. Can be changed for another input provider. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Input")
    FKey ZoomAnalogKey = EKeys::Gamepad_LeftTriggerAxis;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Input",
        meta=(ClampMin="0.0", ClampMax="0.95"))
    float ZoomAnalogDeadZone = 0.04f;

    /** Works when the platform reports two touch positions to PlayerController. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Touch")
    bool bEnableTouchPinchZoom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom|Touch",
        meta=(ClampMin="1.05", ClampMax="4.0"))
    float PinchFullZoomDistanceRatio = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom",
        meta=(ClampMin="5.0", ClampMax="120.0", Units="deg"))
    float ZoomFieldOfView = 40.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom",
        meta=(ClampMin="0.1", ClampMax="40.0"))
    float ZoomBlendSpeed = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|Zoom")
    bool bScaleLookSensitivityWithZoom = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|First Person")
    FVector FirstPersonCameraOffset = FVector(0.0f, 0.0f, 64.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|First Person",
        meta=(ClampMin="30.0", ClampMax="170.0"))
    float FirstPersonFieldOfView = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera|First Person")
    bool bHideOwnMeshInFirstPerson = true;

    /** Optional explicit existing third-person camera. Auto-detected when empty. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Camera")
    TObjectPtr<UCameraComponent> ThirdPersonCamera;

    UFUNCTION(BlueprintCallable, Category="TMOP|Camera")
    void TogglePerspective();

    UFUNCTION(BlueprintCallable, Category="TMOP|Camera")
    void SetFirstPerson(bool bEnableFirstPerson);

    UFUNCTION(BlueprintPure, Category="TMOP|Camera")
    bool IsFirstPerson() const { return bIsFirstPerson; }

    UFUNCTION(BlueprintPure, Category="TMOP|Camera")
    UCameraComponent* GetActivePerspectiveCamera() const;

    /** Hold/analog input bridge for Enhanced Input or a device-specific touchpad adapter. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Camera|Zoom")
    void SetLookZoomAmount(float Amount);

    UFUNCTION(BlueprintCallable, Category="TMOP|Camera|Zoom")
    void BeginLookZoom() { SetLookZoomAmount(1.0f); }

    /** Call on input Completed AND Canceled, or when touch contact ends. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Camera|Zoom")
    void EndLookZoom() { SetLookZoomAmount(0.0f); }

    /** Clears requests and restores the original FOV immediately for menus/other cameras. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Camera|Zoom")
    void CancelLookZoom();

    UFUNCTION(BlueprintPure, Category="TMOP|Camera|Zoom")
    float GetLookZoomAmount() const { return CurrentZoomAmount; }

    UFUNCTION(BlueprintPure, Category="TMOP|Camera|Zoom")
    float GetLookSensitivityScale() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Camera")
    bool CanUseCameraControls() const;

private:
    UPROPERTY(Transient)
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> OwnerMesh;

    bool bIsFirstPerson = false;
    bool bToggleKeyWasDown = false;
    bool bGamepadToggleKeyWasDown = false;
    bool bPreviousOwnerNoSee = false;
    bool bMeshHidingApplied = false;
    float ExternalZoomAmount = 0.0f;
    float CurrentZoomAmount = 0.0f;
    float PinchStartDistance = 0.0f;
    float UnzoomedFieldOfView = 90.0f;
    TWeakObjectPtr<UCameraComponent> ZoomedCamera;

    struct FHiddenMeshState
    {
        TWeakObjectPtr<UMeshComponent> Mesh;
        bool bOwnerNoSee = false;
        bool bCastHiddenShadow = false;
    };
    TArray<FHiddenMeshState> HiddenMeshes;

    void DiscoverOrCreateCameras();
    void UpdateZoom(float DeltaTime);
    float ReadPinchZoom();
    void RestoreZoomCamera();
    void UpdateFirstPersonMeshes();
    void RestoreFirstPersonMeshes();
};
