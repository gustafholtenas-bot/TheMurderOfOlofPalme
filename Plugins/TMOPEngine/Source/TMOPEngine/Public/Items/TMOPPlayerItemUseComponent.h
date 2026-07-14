#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "TMOPPlayerItemUseComponent.generated.h"

class ACharacter;
class UCameraComponent;
class UInventoryComponent;
class USceneCaptureComponent2D;
class USpotLightComponent;
class UTextureRenderTarget2D;
class UTMOPInventoryComponent;
class UTMOPInventoryInputComponent;
class UTMOPItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTMOPPhotographCapturedSignature,
    FString, AbsoluteFilePath, int32, PhotographNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTMOPFlashlightStateSignature,
    bool, bFlashlightOn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTMOPCameraRaisedSignature,
    bool, bCameraRaised);

/** Runtime implementation for camera and flashlight inventory items. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerItemUseComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerItemUseComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Camera",
        meta=(ClampMin="64", ClampMax="4096"))
    int32 PhotographWidth = 1920;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Camera",
        meta=(ClampMin="64", ClampMax="4096"))
    int32 PhotographHeight = 1080;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Camera",
        meta=(ClampMin="10.0", ClampMax="120.0"))
    float CameraRaisedFieldOfView = 55.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Camera")
    FString PhotoOutputFolder = TEXT("TMOPPhotos");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Flashlight",
        meta=(ClampMin="0.0"))
    float FlashlightIntensity = 8000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Flashlight",
        meta=(ClampMin="1.0"))
    float FlashlightAttenuationRadius = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Flashlight",
        meta=(ClampMin="1.0", ClampMax="80.0"))
    float FlashlightInnerConeAngle = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Items|Flashlight",
        meta=(ClampMin="1.0", ClampMax="80.0"))
    float FlashlightOuterConeAngle = 32.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Items|Camera")
    bool bCameraRaised = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Items|Flashlight")
    bool bFlashlightOn = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Items|Camera")
    int32 PhotographCounter = 0;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Items|Events")
    FTMOPPhotographCapturedSignature OnPhotographCaptured;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Items|Events")
    FTMOPFlashlightStateSignature OnFlashlightStateChanged;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Items|Events")
    FTMOPCameraRaisedSignature OnCameraRaisedChanged;

    UFUNCTION(BlueprintCallable, Category="TMOP|Items|Camera")
    bool CapturePhotograph();

    UFUNCTION(BlueprintCallable, Category="TMOP|Items|Camera")
    void SetCameraRaised(bool bRaised);

    UFUNCTION(BlueprintCallable, Category="TMOP|Items|Flashlight")
    void SetFlashlightOn(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="TMOP|Items|Flashlight")
    void ToggleFlashlight();

private:
    UFUNCTION()
    void HandleItemInput(UTMOPItemDefinition* Item, ETMOPItemInput Input,
        ETMOPItemInputPhase Phase);

    UFUNCTION()
    void HandleEquippedItemChanged(UTMOPItemDefinition* PreviousItem,
        UTMOPItemDefinition* NewItem);

    void CreateRuntimeComponents();
    bool IsItemTypeEquipped(ETMOPItemType ItemType) const;

    UPROPERTY(Transient)
    TObjectPtr<ACharacter> CharacterOwner;

    UPROPERTY(Transient)
    TObjectPtr<UCameraComponent> PlayerCamera;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryComponent> Inventory;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryInputComponent> InventoryInput;

    UPROPERTY(Transient)
    TObjectPtr<USceneCaptureComponent2D> PhotoCapture;

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> PhotoRenderTarget;

    UPROPERTY(Transient)
    TObjectPtr<USpotLightComponent> Flashlight;

    float OriginalFieldOfView = 90.0f;
};
