#include "Newspapers/TMOPNewspaperReadingComponent.h"

#include "Animation/AnimSequence.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Texture2D.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Newspapers/TMOPNewspaperItemDefinition.h"

UTMOPNewspaperReadingComponent::UTMOPNewspaperReadingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bTickEvenWhenPaused = true;
}

void UTMOPNewspaperReadingComponent::BeginPlay()
{
    Super::BeginPlay();
    CreateReadingComponents();
}

void UTMOPNewspaperReadingComponent::CreateReadingComponents()
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || IsValid(ReadingArms)) return;

    ReadingArms = NewObject<USkeletalMeshComponent>(Owner, TEXT("TMOPNewspaperReadingArms"));
    ReadingNewspaper = NewObject<UStaticMeshComponent>(Owner, TEXT("TMOPNewspaper3D"));
    ReadingCamera = NewObject<UCameraComponent>(Owner, TEXT("TMOPNewspaperFirstPersonCamera"));
    if (!IsValid(ReadingArms) || !IsValid(ReadingNewspaper) || !IsValid(ReadingCamera)) return;

    Owner->AddInstanceComponent(ReadingArms);
    Owner->AddInstanceComponent(ReadingNewspaper);
    Owner->AddInstanceComponent(ReadingCamera);
    ReadingArms->RegisterComponent();
    ReadingNewspaper->RegisterComponent();
    ReadingCamera->SetupAttachment(Owner->GetRootComponent());
    ReadingCamera->RegisterComponent();
    ReadingCamera->bUsePawnControlRotation = true;
    ReadingCamera->SetActive(false);
    ReadingArms->PrimaryComponentTick.bTickEvenWhenPaused = true;
    ReadingArms->SetOnlyOwnerSee(true);
    ReadingNewspaper->SetOnlyOwnerSee(true);
    ReadingArms->SetCastShadow(false);
    ReadingNewspaper->SetCastShadow(false);
    ReadingArms->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ReadingNewspaper->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ReadingArms->SetVisibility(false, true);
    ReadingNewspaper->SetVisibility(false, true);
}

UCameraComponent* UTMOPNewspaperReadingComponent::FindActiveCamera() const
{
    TArray<UCameraComponent*> Cameras;
    if (IsValid(GetOwner())) GetOwner()->GetComponents(Cameras);
    for (UCameraComponent* Camera : Cameras)
        if (IsValid(Camera) && Camera->IsActive()) return Camera;
    return Cameras.IsEmpty() ? nullptr : Cameras[0];
}

bool UTMOPNewspaperReadingComponent::BeginReading(
    UTMOPNewspaperItemDefinition* Newspaper, const int32 PageIndex)
{
    if (!IsValid(Newspaper)) return false;
    CreateReadingComponents();
    PreviousCamera = FindActiveCamera();
    if (!IsValid(PreviousCamera) || !IsValid(ReadingCamera) ||
        !IsValid(ReadingArms) || !IsValid(ReadingNewspaper) ||
        !IsValid(NewspaperMesh) || !IsValid(NewspaperMaterial)) return false;

    ActiveNewspaper = Newspaper;
    PreviousCamera->SetActive(false);
    ReadingCamera->SetRelativeLocation(FirstPersonCameraOffset);
    ReadingCamera->SetFieldOfView(FirstPersonFieldOfView);
    ReadingCamera->SetActive(true);
    ReadingArms->AttachToComponent(ReadingCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    ReadingNewspaper->AttachToComponent(ReadingCamera, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

    USkeletalMesh* ResolvedArms = FirstPersonArmsMesh;
    if (!IsValid(ResolvedArms))
        if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
            if (IsValid(Character->GetMesh())) ResolvedArms = Character->GetMesh()->GetSkeletalMeshAsset();
    ReadingArms->SetSkeletalMeshAsset(ResolvedArms);
    if (const ACharacter* Character = Cast<ACharacter>(GetOwner()))
        if (IsValid(Character->GetMesh()))
            for (int32 Slot = 0; Slot < Character->GetMesh()->GetNumMaterials(); ++Slot)
                ReadingArms->SetMaterial(Slot, Character->GetMesh()->GetMaterial(Slot));

    ReadingArms->SetRelativeTransform(ArmsRelativeTransform);
    CurrentNewspaperTransform = NewspaperRelativeTransform;
    ReadingNewspaper->SetRelativeTransform(CurrentNewspaperTransform);
    ReadingNewspaper->SetStaticMesh(NewspaperMesh);
    NewspaperMID = UMaterialInstanceDynamic::Create(NewspaperMaterial, this);
    ReadingNewspaper->SetMaterial(0, NewspaperMID);
    if (IsValid(HoldNewspaperAnimation)) ReadingArms->PlayAnimation(HoldNewspaperAnimation, true);
    ReadingArms->SetVisibility(true, true);
    ReadingNewspaper->SetVisibility(true, true);
    SetComponentTickEnabled(true);
    return ShowPage(PageIndex, false);
}

void UTMOPNewspaperReadingComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    if (PageTurnSecondsRemaining <= 0.0f) return;
    PageTurnSecondsRemaining -= DeltaTime;
    if (PageTurnSecondsRemaining <= 0.0f && IsValid(ReadingArms) &&
        IsValid(HoldNewspaperAnimation))
        ReadingArms->PlayAnimation(HoldNewspaperAnimation, true);
}

bool UTMOPNewspaperReadingComponent::ShowPage(
    const int32 PageIndex, const bool bPlayTurnAnimation)
{
    if (!IsValid(ActiveNewspaper) || !IsValid(NewspaperMID) ||
        !ActiveNewspaper->Pages.IsValidIndex(PageIndex)) return false;
    UTexture2D* Left = ActiveNewspaper->Pages[PageIndex].PageImage.LoadSynchronous();
    UTexture2D* Right = ActiveNewspaper->Pages.IsValidIndex(PageIndex + 1)
        ? ActiveNewspaper->Pages[PageIndex + 1].PageImage.LoadSynchronous() : Left;
    NewspaperMID->SetTextureParameterValue(LeftPageTextureParameter, Left);
    NewspaperMID->SetTextureParameterValue(RightPageTextureParameter, Right);
    if (bPlayTurnAnimation && IsValid(TurnPageAnimation) && IsValid(ReadingArms))
    {
        ReadingArms->PlayAnimation(TurnPageAnimation, false);
        PageTurnSecondsRemaining = TurnPageAnimation->GetPlayLength();
    }
    return true;
}

void UTMOPNewspaperReadingComponent::Pan(
    const float HorizontalDirection, const float VerticalDirection)
{
    const FVector Delta(0.0f, HorizontalDirection * PanStepCm,
        VerticalDirection * PanStepCm);
    FVector Location = CurrentNewspaperTransform.GetLocation();
    Location += Delta;
    CurrentNewspaperTransform.SetLocation(Location);
    if (IsValid(ReadingNewspaper)) ReadingNewspaper->SetRelativeTransform(CurrentNewspaperTransform);
    if (IsValid(ReadingArms)) ReadingArms->AddRelativeLocation(Delta);
}

void UTMOPNewspaperReadingComponent::Zoom(const float Direction)
{
    FVector Location = CurrentNewspaperTransform.GetLocation();
    const float OldX = Location.X;
    Location.X = FMath::Clamp(OldX - Direction * ZoomStepCm, 30.0f, 110.0f);
    CurrentNewspaperTransform.SetLocation(Location);
    if (IsValid(ReadingNewspaper)) ReadingNewspaper->SetRelativeTransform(CurrentNewspaperTransform);
    if (IsValid(ReadingArms)) ReadingArms->AddRelativeLocation(
        FVector(Location.X - OldX, 0.0f, 0.0f));
}

void UTMOPNewspaperReadingComponent::EndReading()
{
    if (IsValid(ReadingArms)) ReadingArms->SetVisibility(false, true);
    if (IsValid(ReadingNewspaper)) ReadingNewspaper->SetVisibility(false, true);
    if (IsValid(ReadingCamera)) ReadingCamera->SetActive(false);
    if (IsValid(PreviousCamera)) PreviousCamera->SetActive(true);
    ActiveNewspaper = nullptr;
    NewspaperMID = nullptr;
    PreviousCamera = nullptr;
    PageTurnSecondsRemaining = 0.0f;
    SetComponentTickEnabled(false);
}
