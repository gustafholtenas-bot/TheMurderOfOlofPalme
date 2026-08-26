#include "Newspapers/TMOPNewspaperReadingComponent.h"

#include "Animation/AnimInstance.h"
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
    ReadingCamera->bUsePawnControlRotation = false;
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

UMaterialInstanceDynamic* UTMOPNewspaperReadingComponent::CreatePageMaterial(
    const FName SlotName, const int32 FallbackIndex)
{
    if (!IsValid(ReadingNewspaper) || !IsValid(NewspaperMaterial)) return nullptr;
    int32 SlotIndex = ReadingNewspaper->GetMaterialIndex(SlotName);
    if (SlotIndex == INDEX_NONE) SlotIndex = FallbackIndex;
    if (SlotIndex < 0 || SlotIndex >= ReadingNewspaper->GetNumMaterials()) return nullptr;

    UMaterialInstanceDynamic* MID = UMaterialInstanceDynamic::Create(NewspaperMaterial, this);
    ReadingNewspaper->SetMaterial(SlotIndex, MID);
    return MID;
}

bool UTMOPNewspaperReadingComponent::BeginReading(
    UTMOPNewspaperItemDefinition* Newspaper, const int32 PageIndex)
{
    if (!IsValid(Newspaper) || Newspaper->Pages.IsEmpty()) return false;
    CreateReadingComponents();
    PreviousCamera = FindActiveCamera();
    ACharacter* Character = Cast<ACharacter>(GetOwner());
    USkeletalMeshComponent* PlayerMesh = IsValid(Character) ? Character->GetMesh() : nullptr;
    bUsingExistingPlayerMesh = !bForceFloatingWaistLayout &&
        bUseExistingPlayerMesh && IsValid(PlayerMesh);
    if (!IsValid(PreviousCamera) || !IsValid(ReadingCamera) ||
        !IsValid(ReadingNewspaper) ||
        !IsValid(NewspaperMesh) || !IsValid(NewspaperMaterial)) return false;

    ActiveNewspaper = Newspaper;
    // The dedicated reading camera sits inside the character. Hide the owner's
    // regular body only from that owner's view while reading so the head and
    // limbs cannot clip through the floating newspaper. Preserve the previous
    // value because another first-person system may already use Owner No See.
    if (!bUsingExistingPlayerMesh && IsValid(PlayerMesh))
    {
        HiddenPlayerMesh = PlayerMesh;
        bPreviousPlayerMeshOwnerNoSee = PlayerMesh->bOwnerNoSee;
        bPlayerMeshVisibilityOverridden = true;
        PlayerMesh->SetOwnerNoSee(true);
    }
    PreviousCamera->SetActive(false);
    ReadingCamera->SetRelativeLocation(FirstPersonCameraOffset);
    ReadingCamera->SetRelativeRotation(FirstPersonCameraRotation);
    ReadingCamera->SetFieldOfView(FirstPersonFieldOfView);
    ReadingCamera->SetActive(true);
    if (bUsingExistingPlayerMesh)
    {
        ReadingArms->SetVisibility(false, true);
        const FName ResolvedSocket = PlayerMesh->DoesSocketExist(NewspaperHandSocket)
            ? NewspaperHandSocket : FName(TEXT("hand_r"));
        ReadingNewspaper->AttachToComponent(PlayerMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, ResolvedSocket);
        CurrentNewspaperTransform = NewspaperHandRelativeTransform;
    }
    else
    {
        ReadingNewspaper->AttachToComponent(ReadingCamera,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale);

        if (!bForceFloatingWaistLayout && bShowReadingArms && IsValid(ReadingArms))
        {
            ReadingArms->AttachToComponent(ReadingCamera,
                FAttachmentTransformRules::SnapToTargetNotIncludingScale);
            USkeletalMesh* ResolvedArms = FirstPersonArmsMesh;
            if (!IsValid(ResolvedArms) && IsValid(PlayerMesh))
                ResolvedArms = PlayerMesh->GetSkeletalMeshAsset();
            ReadingArms->SetSkeletalMeshAsset(ResolvedArms);
            if (IsValid(PlayerMesh))
                for (int32 Slot = 0; Slot < PlayerMesh->GetNumMaterials(); ++Slot)
                    ReadingArms->SetMaterial(Slot, PlayerMesh->GetMaterial(Slot));
            ReadingArms->SetRelativeTransform(ArmsRelativeTransform);
        }
        CurrentNewspaperTransform = NewspaperRelativeTransform;
    }
    ReadingNewspaper->SetRelativeTransform(CurrentNewspaperTransform);
    ReadingNewspaper->SetStaticMesh(nullptr);
    ReadingArms->SetVisibility(!bForceFloatingWaistLayout &&
        !bUsingExistingPlayerMesh && bShowReadingArms, true);
    ReadingNewspaper->SetVisibility(true, true);
    SetComponentTickEnabled(false);
    const bool bPageShown = ShowPage(PageIndex, false);
    if (!bPageShown)
    {
        EndReading();
        return false;
    }
    if (bPageShown && bUsingExistingPlayerMesh && IsValid(NewspaperReadingMontage))
    {
        if (UAnimInstance* AnimInstance = PlayerMesh->GetAnimInstance())
        {
            AnimInstance->Montage_Play(NewspaperReadingMontage,
                NewspaperReadingMontagePlayRate);
        }
    }
    return true;
}

void UTMOPNewspaperReadingComponent::TickComponent(const float DeltaTime,
    const ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

bool UTMOPNewspaperReadingComponent::ShowPage(
    const int32 PageIndex, const bool bPlayTurnAnimation)
{
    (void)bPlayTurnAnimation;
    if (!IsValid(ActiveNewspaper) ||
        !ActiveNewspaper->Pages.IsValidIndex(PageIndex)) return false;

    const int32 LastPageIndex = ActiveNewspaper->Pages.Num() - 1;
    const bool bShowFront = PageIndex == 0;
    const bool bShowBack = PageIndex == LastPageIndex && LastPageIndex > 0;
    const bool bUseFoldedMesh = (bShowFront || bShowBack) && IsValid(FoldedNewspaperMesh);
    UStaticMesh* DesiredMesh = bUseFoldedMesh ? FoldedNewspaperMesh : NewspaperMesh;
    if (!IsValid(DesiredMesh)) return false;

    if (ReadingNewspaper->GetStaticMesh() != DesiredMesh)
    {
        ReadingNewspaper->SetStaticMesh(DesiredMesh);
        FrontPageMID = nullptr;
        LeftPageMID = nullptr;
        RightPageMID = nullptr;
        EndPageMID = nullptr;

        if (bUseFoldedMesh)
        {
            FrontPageMID = CreatePageMaterial(FrontMaterialSlot, 0);
            EndPageMID = CreatePageMaterial(EndPageMaterialSlot, 1);
            if (!IsValid(FrontPageMID) || !IsValid(EndPageMID)) return false;
        }
        else
        {
            FrontPageMID = CreatePageMaterial(FrontMaterialSlot, 0);
            LeftPageMID = CreatePageMaterial(LeftPageMaterialSlot, 1);
            RightPageMID = CreatePageMaterial(RightPageMaterialSlot, 2);
            EndPageMID = CreatePageMaterial(EndPageMaterialSlot, 3);
            if (!IsValid(FrontPageMID) || !IsValid(LeftPageMID) ||
                !IsValid(RightPageMID) || !IsValid(EndPageMID)) return false;
        }
    }

    UTexture2D* Front = ActiveNewspaper->Pages[0].PageImage.LoadSynchronous();
    UTexture2D* End = ActiveNewspaper->Pages.Last().PageImage.LoadSynchronous();
    FrontPageMID->SetTextureParameterValue(PageTextureParameter, Front);
    EndPageMID->SetTextureParameterValue(PageTextureParameter, End);

    bShowingFoldedMesh = bUseFoldedMesh;
    CurrentNewspaperTransform = bUsingExistingPlayerMesh
        ? NewspaperHandRelativeTransform : NewspaperRelativeTransform;
    if (bShowBack && bUseFoldedMesh)
    {
        FRotator Rotation = CurrentNewspaperTransform.Rotator();
        Rotation += FoldedBackRotationOffset;
        CurrentNewspaperTransform.SetRotation(Rotation.Quaternion());
    }
    ApplyNewspaperTransform(DesiredMesh);

    if (bUseFoldedMesh) return true;

    UTexture2D* Left = ActiveNewspaper->Pages[PageIndex].PageImage.LoadSynchronous();
    const bool bHasRightInnerPage = ActiveNewspaper->Pages.IsValidIndex(PageIndex + 1) &&
        PageIndex + 1 < ActiveNewspaper->Pages.Num() - 1;
    UTexture2D* Right = bHasRightInnerPage
        ? ActiveNewspaper->Pages[PageIndex + 1].PageImage.LoadSynchronous() : Left;
    LeftPageMID->SetTextureParameterValue(PageTextureParameter, Left);
    RightPageMID->SetTextureParameterValue(PageTextureParameter, Right);
    return true;
}

void UTMOPNewspaperReadingComponent::ApplyNewspaperTransform(UStaticMesh* Mesh)
{
    if (!IsValid(ReadingNewspaper)) return;
    FTransform Applied = CurrentNewspaperTransform;
    if (!bUsingExistingPlayerMesh && bCenterMeshBoundsOnFloatingTransform &&
        IsValid(Mesh))
    {
        // Treat NewspaperRelativeTransform as the desired visual centre, not
        // as the imported mesh pivot. This keeps folded/open meshes centred
        // even when Blender exported their pivot along one page edge.
        const FVector ScaledBoundsOrigin =
            Mesh->GetBounds().Origin * Applied.GetScale3D();
        Applied.AddToTranslation(
            -Applied.GetRotation().RotateVector(ScaledBoundsOrigin));
    }
    ReadingNewspaper->SetRelativeTransform(Applied);
}

void UTMOPNewspaperReadingComponent::Pan(
    const float HorizontalDirection, const float VerticalDirection)
{
    const FVector Delta(0.0f, HorizontalDirection * PanStepCm,
        VerticalDirection * PanStepCm);
    FVector Location = CurrentNewspaperTransform.GetLocation();
    Location += Delta;
    CurrentNewspaperTransform.SetLocation(Location);
    if (IsValid(ReadingNewspaper))
        ApplyNewspaperTransform(ReadingNewspaper->GetStaticMesh());
    if (!bForceFloatingWaistLayout && !bUsingExistingPlayerMesh &&
        bShowReadingArms && IsValid(ReadingArms))
        ReadingArms->AddRelativeLocation(Delta);
}

void UTMOPNewspaperReadingComponent::Zoom(const float Direction)
{
    FVector Location = CurrentNewspaperTransform.GetLocation();
    const float OldX = Location.X;
    Location.X = FMath::Clamp(OldX - Direction * ZoomStepCm, 30.0f, 110.0f);
    CurrentNewspaperTransform.SetLocation(Location);
    if (IsValid(ReadingNewspaper))
        ApplyNewspaperTransform(ReadingNewspaper->GetStaticMesh());
    if (!bForceFloatingWaistLayout && !bUsingExistingPlayerMesh &&
        bShowReadingArms && IsValid(ReadingArms))
        ReadingArms->AddRelativeLocation(FVector(Location.X - OldX, 0.0f, 0.0f));
}

void UTMOPNewspaperReadingComponent::EndReading()
{
    if (bUsingExistingPlayerMesh && IsValid(NewspaperReadingMontage))
    {
        ACharacter* Character = Cast<ACharacter>(GetOwner());
        USkeletalMeshComponent* PlayerMesh = IsValid(Character) ? Character->GetMesh() : nullptr;
        if (IsValid(PlayerMesh))
        {
            if (UAnimInstance* AnimInstance = PlayerMesh->GetAnimInstance())
            {
                AnimInstance->Montage_Stop(NewspaperReadingMontageBlendOutTime,
                    NewspaperReadingMontage);
            }
        }
    }
    if (IsValid(ReadingArms)) ReadingArms->SetVisibility(false, true);
    if (bPlayerMeshVisibilityOverridden && IsValid(HiddenPlayerMesh))
        HiddenPlayerMesh->SetOwnerNoSee(bPreviousPlayerMeshOwnerNoSee);
    HiddenPlayerMesh = nullptr;
    bPlayerMeshVisibilityOverridden = false;
    if (IsValid(ReadingNewspaper))
    {
        ReadingNewspaper->SetVisibility(false, true);
        ReadingNewspaper->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    }
    if (IsValid(ReadingCamera)) ReadingCamera->SetActive(false);
    if (IsValid(PreviousCamera)) PreviousCamera->SetActive(true);
    ActiveNewspaper = nullptr;
    FrontPageMID = nullptr;
    LeftPageMID = nullptr;
    RightPageMID = nullptr;
    EndPageMID = nullptr;
    PreviousCamera = nullptr;
    bUsingExistingPlayerMesh = false;
    SetComponentTickEnabled(false);
}
