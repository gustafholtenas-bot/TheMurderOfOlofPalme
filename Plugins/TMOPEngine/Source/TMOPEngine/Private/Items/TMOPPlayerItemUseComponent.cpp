#include "Items/TMOPPlayerItemUseComponent.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SpotLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "HAL/FileManager.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Misc/Paths.h"

UTMOPPlayerItemUseComponent::UTMOPPlayerItemUseComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPPlayerItemUseComponent::BeginPlay()
{
    Super::BeginPlay();
    CharacterOwner = Cast<ACharacter>(GetOwner());
    PlayerCamera = IsValid(CharacterOwner.Get())
        ? CharacterOwner->FindComponentByClass<UCameraComponent>() : nullptr;
    Inventory = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryComponent>() : nullptr;
    InventoryInput = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryInputComponent>() : nullptr;
    if (IsValid(PlayerCamera.Get())) OriginalFieldOfView = PlayerCamera->FieldOfView;
    CreateRuntimeComponents();
    if (IsValid(InventoryInput.Get()))
        InventoryInput->OnEquippedItemInput.AddDynamic(this,
            &UTMOPPlayerItemUseComponent::HandleItemInput);
    if (IsValid(Inventory.Get()))
        Inventory->OnEquippedItemChanged.AddDynamic(this,
            &UTMOPPlayerItemUseComponent::HandleEquippedItemChanged);
}

void UTMOPPlayerItemUseComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    SetCameraRaised(false);
    SetFlashlightOn(false);
    Super::EndPlay(EndPlayReason);
}

void UTMOPPlayerItemUseComponent::CreateRuntimeComponents()
{
    if (!IsValid(CharacterOwner.Get()) || !IsValid(PlayerCamera.Get())) return;
    PhotoRenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("TMOPPhotoRenderTarget"));
    if (IsValid(PhotoRenderTarget.Get()))
    {
        PhotoRenderTarget->RenderTargetFormat = ETextureRenderTargetFormat::RTF_RGBA8;
        PhotoRenderTarget->InitAutoFormat(PhotographWidth, PhotographHeight);
        PhotoRenderTarget->UpdateResourceImmediate(true);
    }
    PhotoCapture = NewObject<USceneCaptureComponent2D>(CharacterOwner.Get(), TEXT("TMOPPhotoCapture"));
    if (IsValid(PhotoCapture.Get()))
    {
        PhotoCapture->RegisterComponent();
        PhotoCapture->AttachToComponent(PlayerCamera.Get(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        PhotoCapture->TextureTarget = PhotoRenderTarget.Get();
        PhotoCapture->bCaptureEveryFrame = false;
        PhotoCapture->bCaptureOnMovement = false;
        PhotoCapture->SetVisibility(false);
    }
    Flashlight = NewObject<USpotLightComponent>(CharacterOwner.Get(), TEXT("TMOPFlashlight"));
    if (IsValid(Flashlight.Get()))
    {
        Flashlight->RegisterComponent();
        Flashlight->AttachToComponent(PlayerCamera.Get(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);
        Flashlight->SetRelativeLocation(FVector(20.0f, 18.0f, -12.0f));
        Flashlight->SetIntensity(FlashlightIntensity);
        Flashlight->SetAttenuationRadius(FlashlightAttenuationRadius);
        Flashlight->SetInnerConeAngle(FlashlightInnerConeAngle);
        Flashlight->SetOuterConeAngle(FlashlightOuterConeAngle);
        Flashlight->SetCastShadows(true);
        Flashlight->SetVisibility(false);
    }
}

bool UTMOPPlayerItemUseComponent::IsItemTypeEquipped(const ETMOPItemType ItemType) const
{
    const UTMOPItemDefinition* Item = IsValid(Inventory.Get())
        ? Inventory->EquippedItem.Get() : nullptr;
    return IsValid(Item) && Item->ItemType == ItemType;
}

void UTMOPPlayerItemUseComponent::HandleItemInput(UTMOPItemDefinition* Item,
    const ETMOPItemInput Input, const ETMOPItemInputPhase Phase)
{
    if (!IsValid(Item)) return;
    if (Item->ItemType == ETMOPItemType::Camera)
    {
        if (Input == ETMOPItemInput::Secondary)
        {
            if (Phase == ETMOPItemInputPhase::Started) SetCameraRaised(true);
            else if (Phase == ETMOPItemInputPhase::Completed ||
                Phase == ETMOPItemInputPhase::Cancelled) SetCameraRaised(false);
        }
        else if (Input == ETMOPItemInput::Primary &&
            Phase == ETMOPItemInputPhase::Started) CapturePhotograph();
    }
    else if (Item->ItemType == ETMOPItemType::Flashlight &&
        Input == ETMOPItemInput::Primary && Phase == ETMOPItemInputPhase::Started)
    {
        ToggleFlashlight();
    }
}

void UTMOPPlayerItemUseComponent::HandleEquippedItemChanged(
    UTMOPItemDefinition* PreviousItem, UTMOPItemDefinition* NewItem)
{
    if (!IsValid(NewItem) || NewItem->ItemType != ETMOPItemType::Camera)
        SetCameraRaised(false);
    if (!IsValid(NewItem) || NewItem->ItemType != ETMOPItemType::Flashlight)
        SetFlashlightOn(false);
}

void UTMOPPlayerItemUseComponent::SetCameraRaised(const bool bRaised)
{
    const bool bNewValue = bRaised && IsItemTypeEquipped(ETMOPItemType::Camera);
    if (bCameraRaised == bNewValue) return;
    bCameraRaised = bNewValue;
    if (IsValid(PlayerCamera.Get()))
        PlayerCamera->SetFieldOfView(bCameraRaised ? CameraRaisedFieldOfView : OriginalFieldOfView);
    OnCameraRaisedChanged.Broadcast(bCameraRaised);
}

bool UTMOPPlayerItemUseComponent::CapturePhotograph()
{
    if (!bCameraRaised || !IsItemTypeEquipped(ETMOPItemType::Camera) ||
        !IsValid(PhotoCapture.Get()) || !IsValid(PhotoRenderTarget.Get()) ||
        !IsValid(PlayerCamera.Get())) return false;
    PhotoCapture->SetWorldTransform(PlayerCamera->GetComponentTransform());
    PhotoCapture->FOVAngle = PlayerCamera->FieldOfView;
    PhotoCapture->SetVisibility(true);
    PhotoCapture->CaptureScene();
    PhotoCapture->SetVisibility(false);
    ++PhotographCounter;
    const FString Folder = FPaths::Combine(FPaths::ProjectSavedDir(), PhotoOutputFolder);
    IFileManager::Get().MakeDirectory(*Folder, true);
    const FString BaseName = FString::Printf(TEXT("TMOP_Photo_%04d"), PhotographCounter);
    UKismetRenderingLibrary::ExportRenderTarget(this, PhotoRenderTarget.Get(), Folder, BaseName);
    const FString AbsolutePath = FPaths::Combine(Folder, BaseName + TEXT(".png"));
    OnPhotographCaptured.Broadcast(AbsolutePath, PhotographCounter);
    return true;
}

void UTMOPPlayerItemUseComponent::SetFlashlightOn(const bool bEnabled)
{
    const bool bNewValue = bEnabled && IsItemTypeEquipped(ETMOPItemType::Flashlight);
    if (bFlashlightOn == bNewValue) return;
    bFlashlightOn = bNewValue;
    if (IsValid(Flashlight.Get())) Flashlight->SetVisibility(bFlashlightOn);
    OnFlashlightStateChanged.Broadcast(bFlashlightOn);
}

void UTMOPPlayerItemUseComponent::ToggleFlashlight()
{
    SetFlashlightOn(!bFlashlightOn);
}
