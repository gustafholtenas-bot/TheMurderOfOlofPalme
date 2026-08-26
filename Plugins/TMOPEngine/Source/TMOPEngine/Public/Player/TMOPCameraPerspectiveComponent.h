#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "TMOPCameraPerspectiveComponent.generated.h"

class UCameraComponent;
class USkeletalMeshComponent;

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

private:
    UPROPERTY(Transient)
    TObjectPtr<UCameraComponent> FirstPersonCamera;

    UPROPERTY(Transient)
    TObjectPtr<USkeletalMeshComponent> OwnerMesh;

    bool bIsFirstPerson = false;
    bool bInputBound = false;
    bool bPreviousOwnerNoSee = false;
    bool bPreviousCastHiddenShadow = false;

    void DiscoverOrCreateCameras();
    void TryBindInput();
};
