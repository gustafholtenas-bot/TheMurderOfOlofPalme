#include "Agents/TMOPHistoricalAgent.h"

#include "AIController.h"
#include "AI/TMOPHistoricalAIController.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/GameInstance.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "NavigationSystem.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Routes/TMOPRouteFollowerComponent.h"
#include "People/TMOPCharacterAppearanceComponent.h"
#include "People/TMOPPersonProfileComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Items/TMOPItemMeshSubsystem.h"
#include "Sound/SoundBase.h"
#include "Time/TMOPClockSubsystem.h"
#include "UI/TMOPSpeechBubbleWidget.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

namespace
{
FString CompactSourceNumber(FString Source)
{
    Source.TrimStartAndEndInline();
    int32 Cut = INDEX_NONE;
    const TCHAR Separators[] = { TCHAR(','), TCHAR(';'), TCHAR('\n'), TCHAR('\r') };
    for (const TCHAR Separator : Separators)
    {
        int32 Found = INDEX_NONE;
        if (Source.FindChar(Separator, Found) && (Cut == INDEX_NONE || Found < Cut))
            Cut = Found;
    }
    if (Cut != INDEX_NONE) Source.LeftInline(Cut);
    Source.TrimStartAndEndInline();
    return Source.Left(32);
}
}

ATMOPHistoricalAgent::ATMOPHistoricalAgent()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.05f;

    AIControllerClass = ATMOPHistoricalAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    EntityIdentity =
        CreateDefaultSubobject<UTMOPWorldEntityComponent>(
            TEXT("EntityIdentity"));

    if (EntityIdentity != nullptr)
    {
        EntityIdentity->EntityType = TEXT("Agent");
    }

    ActionExecutor =
        CreateDefaultSubobject<UTMOPActionExecutorComponent>(
            TEXT("ActionExecutor"));

    RouteFollower =
        CreateDefaultSubobject<UTMOPRouteFollowerComponent>(
            TEXT("RouteFollower"));

    PersonProfile = CreateDefaultSubobject<UTMOPPersonProfileComponent>(
        TEXT("PersonProfile"));
    CharacterAppearance =
        CreateDefaultSubobject<UTMOPCharacterAppearanceComponent>(
            TEXT("CharacterAppearance"));

    BodyMesh = GetMesh();
    auto CreateModularPart = [this](const FName Name)
    {
        USkeletalMeshComponent* Part =
            CreateDefaultSubobject<USkeletalMeshComponent>(Name);
        Part->SetupAttachment(GetMesh());
        Part->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Part->SetGenerateOverlapEvents(false);
        Part->SetVisibility(false, true);
        return Part;
    };
    FaceMesh = CreateModularPart(TEXT("FaceMesh"));
    HairMesh = CreateModularPart(TEXT("HairMesh"));
    FacialHairMesh = CreateModularPart(TEXT("FacialHairMesh"));
    OuterwearMesh = CreateModularPart(TEXT("OuterwearMesh"));
    UpperBodyMesh = CreateModularPart(TEXT("UpperBodyMesh"));
    TrousersMesh = CreateModularPart(TEXT("TrousersMesh"));
    FootwearMesh = CreateModularPart(TEXT("FootwearMesh"));
    GlovesMesh = CreateModularPart(TEXT("GlovesMesh"));
    HeadwearMesh = CreateModularPart(TEXT("HeadwearMesh"));
    ScarfMesh = CreateModularPart(TEXT("ScarfMesh"));
    GlassesMesh = CreateModularPart(TEXT("GlassesMesh"));

    LeftHandHeldItem = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("LeftHandHeldItem"));
    LeftHandHeldItem->SetupAttachment(BodyMesh);
    LeftHandHeldItem->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    LeftHandHeldItem->SetGenerateOverlapEvents(false);
    LeftHandHeldItem->SetCastShadow(true);
    LeftHandHeldItem->SetVisibility(false, true);

    RightHandHeldItem = CreateDefaultSubobject<UStaticMeshComponent>(
        TEXT("RightHandHeldItem"));
    RightHandHeldItem->SetupAttachment(BodyMesh);
    RightHandHeldItem->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    RightHandHeldItem->SetGenerateOverlapEvents(false);
    RightHandHeldItem->SetCastShadow(true);
    RightHandHeldItem->SetVisibility(false, true);

    NameLabel =
        CreateDefaultSubobject<UTextRenderComponent>(
            TEXT("NameLabel"));
    NameLabel->SetupAttachment(GetCapsuleComponent());
    NameLabel->SetHorizontalAlignment(EHorizTextAligment::EHTA_Center);
    NameLabel->SetVerticalAlignment(EVerticalTextAligment::EVRTA_TextCenter);
    NameLabel->SetWorldSize(NameLabelWorldSize);
    NameLabel->SetTextRenderColor(NameLabelColor);
    NameLabel->SetCastShadow(false);
    NameLabel->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NameLabel->SetGenerateOverlapEvents(false);
    NameLabel->SetHiddenInGame(false);
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> UnlitTextMaterial(
        TEXT("/Engine/EngineMaterials/DefaultTextMaterialOpaque.DefaultTextMaterialOpaque"));
    if (UnlitTextMaterial.Succeeded())
    {
        NameLabelUnlitMaterial = UnlitTextMaterial.Object;
        NameLabel->SetTextMaterial(NameLabelUnlitMaterial);
    }

    SpeechBubble = CreateDefaultSubobject<UWidgetComponent>(
        TEXT("SpeechBubble"));
    SpeechBubble->SetupAttachment(GetCapsuleComponent());
    SpeechBubble->SetRelativeLocation(FVector(0.0f, 0.0f, SpeechBubbleHeightCm));
    SpeechBubble->SetWidgetSpace(EWidgetSpace::Screen);
    SpeechBubble->SetDrawAtDesiredSize(true);
    SpeechBubble->SetPivot(FVector2D(0.5f, 1.0f));
    SpeechBubble->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SpeechBubble->SetGenerateOverlapEvents(false);
    SpeechBubble->SetWidgetClass(UTMOPSpeechBubbleWidget::StaticClass());
    SpeechBubble->SetVisibility(false);

    GetCapsuleComponent()->InitCapsuleSize(PedestrianCapsuleRadiusCm, 90.0f);
    GetCharacterMovement()->MaxStepHeight = MaximumThresholdHeightCm;
    GetCharacterMovement()->JumpZVelocity = 260.0f;
}

void ATMOPHistoricalAgent::ApplyHeldItems(
    const FTMOPHeldItemDefinition& LeftItem,
    const FTMOPHeldItemDefinition& RightItem,
    const TArray<FTMOPHeldItemDefinition>& AdditionalItems)
{
    ActiveLeftHandItem = LeftItem;
    ActiveRightHandItem = RightItem;
    auto ApplyOne = [this](UStaticMeshComponent* Component,
        const FTMOPHeldItemDefinition& Item, const FName DefaultSocket,
        ETMOPHeldItemPose& OutPose)
    {
        if (!IsValid(Component) || !IsValid(BodyMesh)) return;
        UStaticMesh* LoadedMesh = Item.Mesh.LoadSynchronous();
        FTransform CatalogTransform = FTransform::Identity;
        ETMOPHeldItemPose ResolvedPose = Item.GripPose;
        if (!IsValid(LoadedMesh) && !Item.ItemId.IsNone())
            if (UTMOPItemMeshSubsystem* Catalog = GetGameInstance() != nullptr
                ? GetGameInstance()->GetSubsystem<UTMOPItemMeshSubsystem>() : nullptr)
            {
                FTMOPItemMeshRow Definition;
                if (Catalog->FindItemMeshDefinition(Item.ItemId, Definition))
                {
                    LoadedMesh = Definition.Mesh.LoadSynchronous();
                    CatalogTransform = Definition.DefaultAttachmentTransform;
                    if (ResolvedPose == ETMOPHeldItemPose::None)
                        ResolvedPose = Definition.DefaultGripPose;
                }
            }
        OutPose = IsValid(LoadedMesh) && IsHeldItemVisibleNow(Item)
            ? ResolvedPose : ETMOPHeldItemPose::None;
        Component->SetStaticMesh(LoadedMesh);
        const FName Socket = Item.SocketName.IsNone() ? DefaultSocket : Item.SocketName;
        Component->AttachToComponent(BodyMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, Socket);
        Component->SetRelativeTransform(CatalogTransform * Item.RelativeTransform);
        Component->SetVisibility(IsValid(LoadedMesh) && IsHeldItemVisibleNow(Item), true);
    };

    ApplyOne(LeftHandHeldItem, LeftItem, FName(TEXT("hand_lSocket")), LeftHandGripPose);
    ApplyOne(RightHandHeldItem, RightItem, FName(TEXT("hand_rSocket")), RightHandGripPose);
    ResolvedLeftHandGripPose = LeftHandGripPose;
    ResolvedRightHandGripPose = RightHandGripPose;

    for (UStaticMeshComponent* Component : AdditionalCarriedItemComponents)
        if (IsValid(Component)) Component->DestroyComponent();
    AdditionalCarriedItemComponents.Reset();
    for (int32 Index = 0; Index < AdditionalItems.Num(); ++Index)
    {
        const FTMOPHeldItemDefinition& Item = AdditionalItems[Index];
        UStaticMesh* LoadedMesh = Item.Mesh.LoadSynchronous();
        FTransform CatalogTransform = FTransform::Identity;
        if (!IsValid(LoadedMesh) && !Item.ItemId.IsNone())
            if (UTMOPItemMeshSubsystem* Catalog = GetGameInstance() != nullptr
                ? GetGameInstance()->GetSubsystem<UTMOPItemMeshSubsystem>() : nullptr)
            {
                FTMOPItemMeshRow Definition;
                if (Catalog->FindItemMeshDefinition(Item.ItemId, Definition))
                {
                    LoadedMesh = Definition.Mesh.LoadSynchronous();
                    CatalogTransform = Definition.DefaultAttachmentTransform;
                }
            }
        if (!IsValid(LoadedMesh) || !Item.bVisible) continue;
        const FName ComponentName(*FString::Printf(TEXT("CarriedItem_%d_%s"),
            Index, *Item.ItemId.ToString()));
        UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this,
            UStaticMeshComponent::StaticClass(), ComponentName);
        if (!IsValid(Component)) continue;
        AddInstanceComponent(Component);
        Component->SetupAttachment(BodyMesh);
        Component->RegisterComponent();
        Component->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        Component->SetGenerateOverlapEvents(false);
        Component->SetStaticMesh(LoadedMesh);
        const FName Socket = Item.SocketName.IsNone()
            ? GetDefaultItemSocket(Item.AttachmentPoint) : Item.SocketName;
        FName ResolvedSocket = Socket;
        if (!BodyMesh->DoesSocketExist(ResolvedSocket))
        {
            switch (Item.AttachmentPoint)
            {
            case ETMOPItemAttachmentPoint::Chest:
            case ETMOPItemAttachmentPoint::Back: ResolvedSocket = TEXT("spine_03"); break;
            case ETMOPItemAttachmentPoint::LeftShoulder: ResolvedSocket = TEXT("clavicle_l"); break;
            case ETMOPItemAttachmentPoint::RightShoulder: ResolvedSocket = TEXT("clavicle_r"); break;
            case ETMOPItemAttachmentPoint::LeftHip: ResolvedSocket = TEXT("thigh_l"); break;
            case ETMOPItemAttachmentPoint::RightHip: ResolvedSocket = TEXT("thigh_r"); break;
            case ETMOPItemAttachmentPoint::LeftHand: ResolvedSocket = TEXT("hand_l"); break;
            case ETMOPItemAttachmentPoint::RightHand: ResolvedSocket = TEXT("hand_r"); break;
            default: ResolvedSocket = NAME_None; break;
            }
        }
        Component->AttachToComponent(BodyMesh,
            FAttachmentTransformRules::SnapToTargetNotIncludingScale, ResolvedSocket);
        Component->SetRelativeTransform(CatalogTransform * Item.RelativeTransform);
        AdditionalCarriedItemComponents.Add(Component);
    }
}

bool ATMOPHistoricalAgent::IsHeldItemVisibleNow(
    const FTMOPHeldItemDefinition& Item) const
{
    if (!Item.bVisible) return false;
    if (!Item.bUseVisibilityWindow) return true;
    UGameInstance* GameInstance = GetGameInstance();
    UTMOPClockSubsystem* Clock = IsValid(GameInstance)
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (!IsValid(Clock)) return false;
    const int32 Now = Clock->GetCurrentTime().ToSecondsFromMidnight();
    return Now >= Item.VisibleFrom.ToSecondsFromMidnight() &&
        Now <= Item.VisibleUntil.ToSecondsFromMidnight();
}

void ATMOPHistoricalAgent::UpdateHeldItemVisibility()
{
    const bool bLeftVisible = IsValid(LeftHandHeldItem) &&
        IsValid(LeftHandHeldItem->GetStaticMesh()) && IsHeldItemVisibleNow(ActiveLeftHandItem);
    const bool bRightVisible = IsValid(RightHandHeldItem) &&
        IsValid(RightHandHeldItem->GetStaticMesh()) && IsHeldItemVisibleNow(ActiveRightHandItem);
    if (IsValid(LeftHandHeldItem)) LeftHandHeldItem->SetVisibility(bLeftVisible, true);
    if (IsValid(RightHandHeldItem)) RightHandHeldItem->SetVisibility(bRightVisible, true);
    LeftHandGripPose = bLeftVisible ? ResolvedLeftHandGripPose : ETMOPHeldItemPose::None;
    RightHandGripPose = bRightVisible ? ResolvedRightHandGripPose : ETMOPHeldItemPose::None;
}

FName ATMOPHistoricalAgent::GetDefaultItemSocket(
    const ETMOPItemAttachmentPoint AttachmentPoint) const
{
    switch (AttachmentPoint)
    {
    case ETMOPItemAttachmentPoint::LeftHand: return TEXT("hand_lSocket");
    case ETMOPItemAttachmentPoint::RightHand: return TEXT("hand_rSocket");
    case ETMOPItemAttachmentPoint::Chest: return TEXT("item_chest_socket");
    case ETMOPItemAttachmentPoint::Back: return TEXT("item_back_socket");
    case ETMOPItemAttachmentPoint::LeftShoulder: return TEXT("item_shoulder_l_socket");
    case ETMOPItemAttachmentPoint::RightShoulder: return TEXT("item_shoulder_r_socket");
    case ETMOPItemAttachmentPoint::LeftHip: return TEXT("item_hip_l_socket");
    case ETMOPItemAttachmentPoint::RightHip: return TEXT("item_hip_r_socket");
    default: return NAME_None;
    }
}

void ATMOPHistoricalAgent::ClearHeldItems()
{
    FTMOPHeldItemDefinition Empty;
    const TArray<FTMOPHeldItemDefinition> NoAdditionalItems;
    ApplyHeldItems(Empty, Empty, NoAdditionalItems);
}

void ATMOPHistoricalAgent::BeginPlay()
{
    Super::BeginPlay();
    const TArray<USkeletalMeshComponent*> ModularParts = {
        FaceMesh.Get(), HairMesh.Get(), OuterwearMesh.Get(), UpperBodyMesh.Get(),
        TrousersMesh.Get(), FootwearMesh.Get(), GlovesMesh.Get(), HeadwearMesh.Get() };
    for (USkeletalMeshComponent* Part : ModularParts)
        if (IsValid(Part) && IsValid(BodyMesh)) Part->SetLeaderPoseComponent(BodyMesh);
    RefreshNameLabel();
    ApplyMovementSpeedForActivity();
    ApplyInitialSeatAssignment();
    OriginalCapsuleRadiusCm = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
    BaseMeshRelativeRotation = GetMesh()->GetRelativeRotation().Quaternion();
    LastUnstuckLocation = GetActorLocation();
    bUnstuckInitialized = true;
}

void ATMOPHistoricalAgent::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateHeldItemVisibility();
    UpdateAutomaticUnstuck(DeltaSeconds);
    UpdateSocialFocus(DeltaSeconds);
    UpdateAutomaticSpeech(DeltaSeconds);

    if (!bShowNameLabel || !IsValid(NameLabel) || GetWorld() == nullptr)
    {
        return;
    }

    const APlayerCameraManager* CameraManager =
        GetWorld()->GetFirstPlayerController() != nullptr
            ? GetWorld()->GetFirstPlayerController()->PlayerCameraManager
            : nullptr;

    if (IsValid(CameraManager))
    {
        const FVector ToCamera =
            CameraManager->GetCameraLocation() -
            NameLabel->GetComponentLocation();

        if (!ToCamera.IsNearlyZero())
        {
            NameLabel->SetWorldRotation(ToCamera.Rotation());
        }
    }
}

float ATMOPHistoricalAgent::ShowAutomaticSpeech(
    const FText& Text, USoundBase* VoiceOver,
    const float DisplayDurationOverrideSeconds)
{
    if (Text.IsEmpty() || !IsValid(SpeechBubble)) return 0.0f;
    SpeechBubble->SetRelativeLocation(
        FVector(0.0f, 0.0f, SpeechBubbleHeightCm));
    SpeechBubble->InitWidget();
    if (UTMOPSpeechBubbleWidget* Bubble =
        Cast<UTMOPSpeechBubbleWidget>(SpeechBubble->GetUserWidgetObject()))
        Bubble->SetSpeechText(Text);
    SpeechBubble->SetVisibility(true);

    const float ReadDuration = FMath::Clamp(
        1.2f + static_cast<float>(Text.ToString().Len()) / 14.0f,
        2.5f, 12.0f);
    float Duration = DisplayDurationOverrideSeconds > 0.0f
        ? DisplayDurationOverrideSeconds : ReadDuration;

    if (IsValid(ActiveSpeechAudio.Get()))
        ActiveSpeechAudio->Stop();
    ActiveSpeechAudio = nullptr;
    if (IsValid(VoiceOver))
    {
        const float VoiceDuration = VoiceOver->GetDuration();
        if (VoiceDuration > 0.0f)
            Duration = FMath::Max(Duration, VoiceDuration + 0.25f);
        ActiveSpeechAudio = UGameplayStatics::SpawnSoundAttached(
            VoiceOver, GetRootComponent(), NAME_None, FVector::ZeroVector,
            EAttachLocation::KeepRelativeOffset, true);
    }
    AutomaticSpeechSecondsRemaining = FMath::Clamp(Duration, 1.5f, 300.0f);

    if (UTMOPAnimationStateComponent* Animation =
        FindComponentByClass<UTMOPAnimationStateComponent>())
        if (Animation->Overlay == ETMOPAnimOverlay::None ||
            Animation->Overlay == ETMOPAnimOverlay::Talking)
        {
            Animation->SetOverlay(ETMOPAnimOverlay::Talking);
            bAutomaticSpeechUsesTalkingOverlay = true;
        }
    return AutomaticSpeechSecondsRemaining;
}

void ATMOPHistoricalAgent::HideAutomaticSpeech()
{
    AutomaticSpeechSecondsRemaining = 0.0f;
    if (IsValid(SpeechBubble)) SpeechBubble->SetVisibility(false);
    ActiveSpeechAudio = nullptr;
    if (bAutomaticSpeechUsesTalkingOverlay && !bDialogueFocusLocked)
        if (UTMOPAnimationStateComponent* Animation =
            FindComponentByClass<UTMOPAnimationStateComponent>())
            if (Animation->Overlay == ETMOPAnimOverlay::Talking &&
                AutomaticSpeechSecondsRemaining <= 0.0f)
                Animation->SetOverlay(ETMOPAnimOverlay::None);
    bAutomaticSpeechUsesTalkingOverlay = false;
}

void ATMOPHistoricalAgent::UpdateAutomaticSpeech(const float DeltaSeconds)
{
    if (AutomaticSpeechSecondsRemaining <= 0.0f) return;
    AutomaticSpeechSecondsRemaining -= DeltaSeconds;
    if (AutomaticSpeechSecondsRemaining <= 0.0f)
        HideAutomaticSpeech();
}

void ATMOPHistoricalAgent::SetSocialFocus(AActor* Target,
    const float DurationSeconds, const bool bUseTalkingOverlay)
{
    // Group presentation must not steal focus while a player dialogue is open.
    if (bDialogueFocusLocked) return;
    if (!IsValid(Target) || Target == this)
    {
        ClearSocialFocus();
        return;
    }
    SocialFocusTarget = Target;
    bSocialFocusHasNoAutomaticEnd = DurationSeconds < 0.0f;
    SocialFocusSecondsRemaining =
        bSocialFocusHasNoAutomaticEnd ? -1.0f : FMath::Max(0.05f, DurationSeconds);
    bSocialFocusUsesTalkingOverlay = bUseTalkingOverlay;
    if (bUseTalkingOverlay)
        if (UTMOPAnimationStateComponent* Animation =
            FindComponentByClass<UTMOPAnimationStateComponent>())
            if (Animation->Overlay == ETMOPAnimOverlay::None ||
                Animation->Overlay == ETMOPAnimOverlay::Talking)
                Animation->SetOverlay(ETMOPAnimOverlay::Talking);
}

void ATMOPHistoricalAgent::ClearSocialFocus()
{
    if (bDialogueFocusLocked) return;
    SocialFocusTarget.Reset();
    SocialFocusSecondsRemaining = 0.0f;
    bSocialFocusHasNoAutomaticEnd = false;
    if (bSocialFocusUsesTalkingOverlay)
        if (UTMOPAnimationStateComponent* Animation =
            FindComponentByClass<UTMOPAnimationStateComponent>())
            if (Animation->Overlay == ETMOPAnimOverlay::Talking &&
                AutomaticSpeechSecondsRemaining <= 0.0f)
                Animation->SetOverlay(ETMOPAnimOverlay::None);
    bSocialFocusUsesTalkingOverlay = false;
}

void ATMOPHistoricalAgent::BeginDialogueFocus(AActor* Target)
{
    if (!IsValid(Target) || Target == this) return;

    const bool bSeated = IsSeatedForDialogue();
    if (!bDialogueFocusLocked && !bSeated)
    {
        DialogueReturnRotation = GetActorRotation();
        bDialogueReturnRotationSaved = true;
    }

    bDialogueFocusLocked = false;
    SetSocialFocus(Target, -1.0f, true);
    bDialogueFocusLocked = true;

    // Seated people retain the chair/vehicle transform. Their neck and upper
    // spine use SocialLookYaw/Pitch instead of rotating the whole actor.
    if (!bSeated && GetVelocity().Size2D() <= 20.0f)
    {
        const FVector Direction = Target->GetActorLocation() - GetActorLocation();
        if (!Direction.IsNearlyZero())
        {
            FRotator Facing = Direction.Rotation();
            Facing.Pitch = 0.0f;
            Facing.Roll = 0.0f;
            SetActorRotation(Facing);
        }
    }
}

void ATMOPHistoricalAgent::BeginLookAtFocus(AActor* Target)
{
    if (!IsValid(Target) || Target == this) return;

    const bool bSeated = IsSeatedForDialogue();
    if (!bDialogueFocusLocked && !bSeated)
    {
        DialogueReturnRotation = GetActorRotation();
        bDialogueReturnRotationSaved = true;
    }

    // Reuse the persistent focus lock so Move/Wait can end this authored look,
    // but explicitly omit the talking overlay.
    bDialogueFocusLocked = false;
    SetSocialFocus(Target, -1.0f, false);
    bDialogueFocusLocked = true;

    if (!bSeated && GetVelocity().Size2D() <= 20.0f)
    {
        const FVector Direction =
            Target->GetActorLocation() - GetActorLocation();
        if (!Direction.IsNearlyZero())
        {
            FRotator Facing = Direction.Rotation();
            Facing.Pitch = 0.0f;
            Facing.Roll = 0.0f;
            SetActorRotation(Facing);
        }
    }
}

bool ATMOPHistoricalAgent::IsSeatedForDialogue() const
{
    if (ActivityState == ETMOPAgentActivityState::Seated ||
        ActivityState == ETMOPAgentActivityState::RidingVehicle)
        return true;
    const UTMOPAnimationStateComponent* Animation =
        FindComponentByClass<UTMOPAnimationStateComponent>();
    return IsValid(Animation) &&
        (Animation->Posture == ETMOPAnimPosture::Sitting ||
         Animation->Posture == ETMOPAnimPosture::SittingInCar);
}

void ATMOPHistoricalAgent::EndDialogueFocus()
{
    if (!bDialogueFocusLocked) return;
    bDialogueFocusLocked = false;
    ClearSocialFocus();

    if (bDialogueReturnRotationSaved && GetVelocity().Size2D() <= 20.0f)
        SetActorRotation(DialogueReturnRotation);
    bDialogueReturnRotationSaved = false;
}

void ATMOPHistoricalAgent::UpdateSocialFocus(const float DeltaSeconds)
{
    if (!bSocialFocusHasNoAutomaticEnd && SocialFocusTarget.IsValid())
    {
        SocialFocusSecondsRemaining -= DeltaSeconds;
        if (SocialFocusSecondsRemaining <= 0.0f) ClearSocialFocus();
    }

    float TargetYaw = 0.0f;
    float TargetPitch = 0.0f;
    float TargetAlpha = 0.0f;
    if (const AActor* Target = SocialFocusTarget.Get())
    {
        const FVector EyeLocation =
            GetActorLocation() + FVector(0.0f, 0.0f, BaseEyeHeight);
        const FVector TargetLocation =
            Target->GetActorLocation() + FVector(0.0f, 0.0f, 60.0f);
        const FRotator LocalLook =
            (TargetLocation - EyeLocation).Rotation() - GetActorRotation();
        const float MaximumYaw = IsSeatedForDialogue()
            ? MaximumSeatedSocialLookYaw : MaximumSocialLookYaw;
        TargetYaw = FMath::Clamp(
            FMath::FindDeltaAngleDegrees(0.0f, LocalLook.Yaw),
            -MaximumYaw, MaximumYaw);
        TargetPitch = FMath::Clamp(
            FMath::FindDeltaAngleDegrees(0.0f, LocalLook.Pitch),
            -MaximumSocialLookPitch, MaximumSocialLookPitch);
        TargetAlpha = 1.0f;
    }

    SocialLookYaw = FMath::FInterpTo(
        SocialLookYaw, TargetYaw, DeltaSeconds, SocialLookInterpolationSpeed);
    SocialLookPitch = FMath::FInterpTo(
        SocialLookPitch, TargetPitch, DeltaSeconds, SocialLookInterpolationSpeed);
    SocialLookAlpha = FMath::FInterpTo(
        SocialLookAlpha, TargetAlpha, DeltaSeconds, SocialLookInterpolationSpeed);

    if (bUseSubtleMeshTurnForSocialLook && !IsSeatedForDialogue() &&
        IsValid(GetMesh()))
    {
        const float MeshYaw = FMath::Clamp(
            SocialLookYaw, -MaximumSocialMeshTurnYaw, MaximumSocialMeshTurnYaw)
            * SocialLookAlpha;
        const FQuat SocialTurn = FRotator(0.0f, MeshYaw, 0.0f).Quaternion();
        GetMesh()->SetRelativeRotation(SocialTurn * BaseMeshRelativeRotation);
    }
}

void ATMOPHistoricalAgent::UpdateAutomaticUnstuck(const float DeltaSeconds)
{
    UpdateCrowdPassThrough(DeltaSeconds);

    if (!bEnableAutomaticUnstuck || DeltaSeconds <= 0.0f ||
        !CanMove() || GetCharacterMovement() == nullptr ||
        GetCharacterMovement()->MovementMode != MOVE_Walking)
    {
        EndCrowdPassThrough();
        ResetAutomaticUnstuck(true);
        return;
    }

    AAIController* AIController = Cast<AAIController>(GetController());
    if (!IsValid(AIController))
    {
        EndCrowdPassThrough();
        ResetAutomaticUnstuck(true);
        return;
    }

    if (!bUnstuckInitialized)
    {
        OriginalCapsuleRadiusCm = GetCapsuleComponent()->GetUnscaledCapsuleRadius();
        LastUnstuckLocation = GetActorLocation();
        bUnstuckInitialized = true;
    }

    if (bReturningFromSideStep)
    {
        SideStepReturnSeconds -= DeltaSeconds;
        if (SideStepReturnSeconds <= 0.0f)
        {
            bReturningFromSideStep = false;
            ReissueMove(AIController, SavedMoveDestination);
        }
    }

    const float MovedDistance2D =
        FVector::Dist2D(GetActorLocation(), LastUnstuckLocation);
    LastUnstuckLocation = GetActorLocation();
    const bool bMeaningfullyMoving =
        MovedDistance2D >= FMath::Max(1.5f, GetVelocity().Size2D() * DeltaSeconds * 0.25f);
    if (bMeaningfullyMoving)
    {
        ResetAutomaticUnstuck(true);
        LastUnstuckLocation = GetActorLocation();
        bUnstuckInitialized = true;
        return;
    }

    const FVector ImmediateDestination = AIController->GetImmediateMoveDestination();
    if (!ImmediateDestination.IsNearlyZero())
        SavedMoveDestination = ImmediateDestination;
    else if (IsValid(ActionExecutor))
    {
        // MoveTo can be rejected immediately in a dense crowd, leaving the AI
        // controller without a goal. Preserve the authoritative action target
        // so repath and pass-through can recover the move.
        FVector ActiveActionTarget = FVector::ZeroVector;
        if (ActionExecutor->TryGetActiveMoveTarget(ActiveActionTarget) &&
            !ActiveActionTarget.IsNearlyZero())
            SavedMoveDestination = ActiveActionTarget;
    }
    if (SavedMoveDestination.IsNearlyZero()) return;

    StationarySeconds += DeltaSeconds;
    if (!bThresholdStepAttempted &&
        StationarySeconds >= ThresholdStepAfterSeconds)
    {
        bThresholdStepAttempted = true;
        TryThresholdStep(SavedMoveDestination);
    }
    if (!bRepathAttempted && StationarySeconds >= RepathAfterSeconds)
    {
        bRepathAttempted = true;
        ReissueMove(AIController, SavedMoveDestination);
    }
    if (!bSideStepAttempted && StationarySeconds >= SideStepAfterSeconds)
    {
        bSideStepAttempted = true;
        TrySideStep(AIController, SavedMoveDestination);
    }
    if (!bSqueezeActive && StationarySeconds >= SqueezeAfterSeconds)
    {
        bSqueezeActive = true;
        GetCapsuleComponent()->SetCapsuleRadius(
            FMath::Max(22.0f, OriginalCapsuleRadiusCm * 0.82f), true);
    }
    if (!bFailsafeAttempted && StationarySeconds >= FailsafeAfterSeconds)
    {
        bFailsafeAttempted = true;
        if (TryFailsafeAdvance(SavedMoveDestination))
            ReissueMove(AIController, SavedMoveDestination);
    }
    if (!bCrowdPassThroughActive && !bCrowdPassThroughAttempted &&
        StationarySeconds >= CrowdPassThroughAfterSeconds)
    {
        bCrowdPassThroughAttempted = true;
        if (BeginCrowdPassThrough(SavedMoveDestination))
            ReissueMove(AIController, SavedMoveDestination);
    }
}

void ATMOPHistoricalAgent::ResetAutomaticUnstuck(const bool bRestoreCapsule)
{
    StationarySeconds = 0.0f;
    SideStepReturnSeconds = 0.0f;
    bRepathAttempted = false;
    bSideStepAttempted = false;
    bFailsafeAttempted = false;
    bThresholdStepAttempted = false;
    bReturningFromSideStep = false;
    if (!bCrowdPassThroughActive)
        bCrowdPassThroughAttempted = false;
    if (bRestoreCapsule && bSqueezeActive && OriginalCapsuleRadiusCm > 0.0f)
    {
        GetCapsuleComponent()->SetCapsuleRadius(OriginalCapsuleRadiusCm, true);
        bSqueezeActive = false;
    }
}

bool ATMOPHistoricalAgent::BeginCrowdPassThrough(
    const FVector& Destination)
{
    if (GetWorld() == nullptr || !IsValid(GetCapsuleComponent())) return false;

    FVector Forward = Destination - GetActorLocation();
    Forward.Z = 0.0f;
    if (!Forward.Normalize()) Forward = GetActorForwardVector().GetSafeNormal2D();

    const FVector SearchCenter =
        GetActorLocation() + Forward * (CrowdPassThroughRadiusCm * 0.3f);
    FCollisionObjectQueryParams PawnObjects;
    PawnObjects.AddObjectTypesToQuery(ECC_Pawn);
    PawnObjects.AddObjectTypesToQuery(ECC_Vehicle);
    FCollisionQueryParams Query(
        SCENE_QUERY_STAT(TMOPCrowdPassThrough), false, this);
    TArray<FOverlapResult> Overlaps;
    GetWorld()->OverlapMultiByObjectType(
        Overlaps, SearchCenter, FQuat::Identity, PawnObjects,
        FCollisionShape::MakeSphere(CrowdPassThroughRadiusCm), Query);

    CrowdPassThroughIgnoredAgents.Reset();
    for (const FOverlapResult& Overlap : Overlaps)
    {
        AActor* Other = Overlap.GetActor();
        if (!IsValid(Other) || Other == this) continue;
        // Only ignore historical pedestrians and effectively stationary vehicles;
        // world geometry and moving traffic keep normal collision.
        const bool bHistoricalPedestrian = Cast<ATMOPHistoricalAgent>(Other) != nullptr;
        const bool bStationaryVehicle = Other->GetVelocity().Size2D() < 10.0f;
        if (!bHistoricalPedestrian && !bStationaryVehicle) continue;

        FVector ToOther = Other->GetActorLocation() - GetActorLocation();
        const float HeightDifference = FMath::Abs(ToOther.Z);
        ToOther.Z = 0.0f;
        if (HeightDifference > 130.0f ||
            (!ToOther.IsNearlyZero() &&
             FVector::DotProduct(Forward, ToOther.GetSafeNormal()) < -0.15f))
            continue;

        GetCapsuleComponent()->IgnoreActorWhenMoving(Other, true);
        CrowdPassThroughIgnoredAgents.Add(Other);
    }

    if (CrowdPassThroughIgnoredAgents.IsEmpty()) return false;

    bCrowdPassThroughActive = true;
    CrowdPassThroughSecondsRemaining = CrowdPassThroughDurationSeconds;
    CrowdPassThroughStartLocation = GetActorLocation();
    UE_LOG(LogTemp, Display,
        TEXT("TMOP crowd pass-through: '%s' temporarily ignores %d nearby pedestrians/vehicles."),
        IsValid(EntityIdentity) ? *EntityIdentity->EntityId.ToString() : *GetName(),
        CrowdPassThroughIgnoredAgents.Num());
    return true;
}

void ATMOPHistoricalAgent::UpdateCrowdPassThrough(const float DeltaSeconds)
{
    if (!bCrowdPassThroughActive) return;

    CrowdPassThroughSecondsRemaining -= FMath::Max(0.0f, DeltaSeconds);
    const float AdvancedDistance = FVector::Dist2D(
        GetActorLocation(), CrowdPassThroughStartLocation);
    if (CrowdPassThroughSecondsRemaining <= 0.0f ||
        AdvancedDistance >= CrowdPassThroughAdvanceCm)
        EndCrowdPassThrough();
}

void ATMOPHistoricalAgent::EndCrowdPassThrough()
{
    if (!bCrowdPassThroughActive && CrowdPassThroughIgnoredAgents.IsEmpty())
        return;

    if (IsValid(GetCapsuleComponent()))
        for (const TWeakObjectPtr<AActor>& Ignored : CrowdPassThroughIgnoredAgents)
            if (AActor* Actor = Ignored.Get())
                GetCapsuleComponent()->IgnoreActorWhenMoving(Actor, false);

    CrowdPassThroughIgnoredAgents.Reset();
    CrowdPassThroughSecondsRemaining = 0.0f;
    bCrowdPassThroughActive = false;
}

void ATMOPHistoricalAgent::ReissueMove(
    AAIController* AIController, const FVector& Destination)
{
    if (!IsValid(AIController) || Destination.IsNearlyZero()) return;
    AIController->MoveToLocation(Destination, 40.0f, true, true, false, true);
}

bool ATMOPHistoricalAgent::TrySideStep(
    AAIController* AIController, const FVector& Destination)
{
    UNavigationSystemV1* Navigation =
        UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(AIController) || Navigation == nullptr) return false;

    FVector Forward = Destination - GetActorLocation();
    Forward.Z = 0.0f;
    if (!Forward.Normalize()) Forward = GetActorForwardVector();
    const FVector Right = FVector::CrossProduct(FVector::UpVector, Forward);
    const float SideSign =
        (GetTypeHash(EntityIdentity != nullptr ? EntityIdentity->EntityId : GetFName()) & 1)
        ? 1.0f : -1.0f;
    const FVector Candidate =
        GetActorLocation() + Forward * 55.0f + Right * SideStepDistanceCm * SideSign;
    FNavLocation Projected;
    if (!Navigation->ProjectPointToNavigation(
        Candidate, Projected, FVector(70.0f, 70.0f, 130.0f))) return false;

    AIController->MoveToLocation(Projected.Location, 30.0f, true, true, false, true);
    bReturningFromSideStep = true;
    SideStepReturnSeconds = 1.15f;
    return true;
}

bool ATMOPHistoricalAgent::TryFailsafeAdvance(const FVector& Destination)
{
    UNavigationSystemV1* Navigation =
        UNavigationSystemV1::GetCurrent(GetWorld());
    if (Navigation == nullptr || Destination.IsNearlyZero()) return false;

    FVector Direction = Destination - GetActorLocation();
    Direction.Z = 0.0f;
    if (!Direction.Normalize()) return false;
    FNavLocation Projected;
    const FVector Candidate =
        GetActorLocation() + Direction * FailsafeAdvanceCm;
    if (!Navigation->ProjectPointToNavigation(
        Candidate, Projected, FVector(60.0f, 60.0f, 120.0f))) return false;

    FVector CapsuleLocation = Projected.Location;
    if (const UCapsuleComponent* Capsule = GetCapsuleComponent())
        CapsuleLocation.Z += Capsule->GetScaledCapsuleHalfHeight() + 2.0f;

    FHitResult Hit;
    return SetActorLocation(CapsuleLocation, true, &Hit,
        ETeleportType::TeleportPhysics);
}

bool ATMOPHistoricalAgent::SnapCapsuleToGround(
    const float HorizontalSearchCm, const float VerticalSearchCm,
    const float GroundClearanceCm)
{
    const UCapsuleComponent* Capsule = GetCapsuleComponent();
    UWorld* World = GetWorld();
    if (!IsValid(Capsule) || World == nullptr) return false;

    const float HorizontalExtent = FMath::Max(1.0f, HorizontalSearchCm);
    const float VerticalExtent = FMath::Max(1.0f, VerticalSearchCm);
    FVector SurfaceLocation = GetActorLocation();
    bool bFoundSurface = false;

    if (UNavigationSystemV1* Navigation =
        UNavigationSystemV1::GetCurrent(World))
    {
        FNavLocation Projected;
        if (Navigation->ProjectPointToNavigation(
            GetActorLocation(), Projected,
            FVector(HorizontalExtent, HorizontalExtent, VerticalExtent)))
        {
            SurfaceLocation = Projected.Location;
            bFoundSurface = true;
        }
    }

    // Some interiors intentionally have no NavMesh. A WorldStatic trace keeps
    // their initial placement correct without treating people or vehicles as
    // floor geometry.
    if (!bFoundSurface)
    {
        FCollisionObjectQueryParams StaticOnly;
        StaticOnly.AddObjectTypesToQuery(ECC_WorldStatic);
        FCollisionQueryParams Query(
            SCENE_QUERY_STAT(TMOPInitialGroundSnap), false, this);
        FHitResult FloorHit;
        const FVector Start =
            GetActorLocation() + FVector(0.0f, 0.0f, VerticalExtent);
        const FVector End =
            GetActorLocation() - FVector(0.0f, 0.0f, VerticalExtent);
        if (World->LineTraceSingleByObjectType(
            FloorHit, Start, End, StaticOnly, Query))
        {
            SurfaceLocation = FloorHit.ImpactPoint;
            bFoundSurface = true;
        }
    }

    if (!bFoundSurface) return false;

    FVector CorrectedLocation = SurfaceLocation;
    CorrectedLocation.Z += Capsule->GetScaledCapsuleHalfHeight() +
        FMath::Max(0.0f, GroundClearanceCm);
    return SetActorLocation(CorrectedLocation, false, nullptr,
        ETeleportType::TeleportPhysics);
}

bool ATMOPHistoricalAgent::TryThresholdStep(const FVector& Destination)
{
    UCharacterMovementComponent* Movement = GetCharacterMovement();
    if (!IsValid(Movement) || !Movement->IsMovingOnGround() ||
        Destination.IsNearlyZero())
        return false;

    FVector Direction = Destination - GetActorLocation();
    Direction.Z = 0.0f;
    if (!Direction.Normalize()) return false;

    const float HalfHeight =
        GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
    const FVector Feet = GetActorLocation() - FVector(0.0f, 0.0f, HalfHeight);
    const FVector LowStart = Feet + FVector(0.0f, 0.0f, 12.0f);
    const FVector LowEnd = LowStart + Direction * 75.0f;
    const FVector HighStart =
        Feet + FVector(0.0f, 0.0f, MaximumThresholdHeightCm + 8.0f);
    const FVector HighEnd = HighStart + Direction * 75.0f;

    FCollisionObjectQueryParams StaticOnly;
    StaticOnly.AddObjectTypesToQuery(ECC_WorldStatic);
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TMOPThresholdStep), false, this);
    FHitResult LowHit;
    FHitResult HighHit;
    const bool bLowBlocked = GetWorld()->LineTraceSingleByObjectType(
        LowHit, LowStart, LowEnd, StaticOnly, Query);
    const bool bHighBlocked = GetWorld()->LineTraceSingleByObjectType(
        HighHit, HighStart, HighEnd, StaticOnly, Query);
    if (!bLowBlocked || bHighBlocked) return false;

    LaunchCharacter(
        Direction * 90.0f + FVector(0.0f, 0.0f, Movement->JumpZVelocity),
        false, true);
    UE_LOG(LogTemp, Display,
        TEXT("TMOP threshold step: '%s' jumped a low obstacle at %s."),
        IsValid(EntityIdentity) ? *EntityIdentity->EntityId.ToString() : *GetName(),
        *LowHit.ImpactPoint.ToString());
    return true;
}

void ATMOPHistoricalAgent::RefreshNameLabel()
{
    if (!IsValid(NameLabel))
    {
        return;
    }

    FText LabelText = DisplayName;
    if (LabelText.IsEmpty() && IsValid(EntityIdentity) &&
        !EntityIdentity->EntityId.IsNone())
    {
        LabelText = FText::FromName(EntityIdentity->EntityId);
    }

    const FString Category = PersonCategoryId.ToString().ToUpper();
    FString EntityId;
    if (IsValid(EntityIdentity) && !EntityIdentity->EntityId.IsNone())
        EntityId = EntityIdentity->EntityId.ToString().ToUpper();
    ETMOPEntityEvidenceIcon ResolvedIcon = EvidenceIcon;
    if (ResolvedIcon == ETMOPEntityEvidenceIcon::Automatic)
    {
        if (Category.StartsWith(TEXT("OBSERVED_")) ||
            EntityId.StartsWith(TEXT("OBSERVED_")))
            ResolvedIcon = ETMOPEntityEvidenceIcon::Observed;
        else
        {
            const FString SourceUpper = SourceReference.ToUpper();
            const bool bOtherSource =
                SourceUpper.Contains(TEXT("MEDIA")) ||
                SourceUpper.Contains(TEXT("SOCIAL")) ||
                SourceUpper.Contains(TEXT("TIDNING")) ||
                SourceUpper.Contains(TEXT("TV ")) ||
                SourceUpper.Contains(TEXT("PLACERAD AV"));
            ResolvedIcon = bOtherSource || SourceReference.IsEmpty()
                ? ETMOPEntityEvidenceIcon::OtherDocumentation
                : ETMOPEntityEvidenceIcon::PoliceInterview;
        }
    }
    const FString* Symbol = &PoliceInterviewSymbol;
    if (ResolvedIcon == ETMOPEntityEvidenceIcon::Observed)
        Symbol = &ObservedSymbol;
    else if (ResolvedIcon == ETMOPEntityEvidenceIcon::OtherDocumentation)
        Symbol = &OtherDocumentationSymbol;
    FString FullLabel = *Symbol;
    const FString CompactSource = CompactSourceNumber(SourceDocumentNumber);
    FullLabel += TEXT("\n") + (CompactSource.IsEmpty()
        ? MissingSourceText : CompactSource);
    FullLabel += TEXT("\n") + LabelText.ToString();
    NameLabel->SetText(FText::FromString(FullLabel));
    NameLabel->SetRelativeLocation(FVector(0.0f, 0.0f, NameLabelHeightCm));
    NameLabel->SetWorldSize(NameLabelWorldSize);
    NameLabel->SetTextRenderColor(ResolveNameLabelColor());
    if (IsValid(NameLabelUnlitMaterial))
        NameLabel->SetTextMaterial(NameLabelUnlitMaterial);
    const bool bDisplayLabel = ShouldDisplayNameLabel();
    NameLabel->SetVisibility(bDisplayLabel, true);
    SetActorTickEnabled(bDisplayLabel);
}

bool ATMOPHistoricalAgent::ShouldDisplayNameLabel() const
{
    return bShowNameLabel;
}

FColor ATMOPHistoricalAgent::ResolveNameLabelColor() const
{
    FString EntityId;
    if (IsValid(EntityIdentity) && !EntityIdentity->EntityId.IsNone())
    {
        EntityId = EntityIdentity->EntityId.ToString().ToUpper();
    }

    // PALME_COMPANY also contains non-family companions. EntityId keeps those
    // people white while automatically covering additional Palme relatives.
    if (EntityId.EndsWith(TEXT("_PALME")))
    {
        return PalmeFamilyNameLabelColor;
    }

    const FString Category = PersonCategoryId.ToString().ToUpper();

    if (Category.StartsWith(TEXT("OBSERVED_")) ||
        EntityId.StartsWith(TEXT("OBSERVED_")))
    {
        return SuspectNameLabelColor;
    }
    if (Category == TEXT("POLICE") || Category == TEXT("POLIS"))
    {
        return PoliceNameLabelColor;
    }
    if (Category == TEXT("SUSPECT"))
    {
        return SuspectNameLabelColor;
    }
    return NameLabelColor;
}

void ATMOPHistoricalAgent::SetNameLabelVisible(const bool bVisible)
{
    bShowNameLabel = bVisible;
    RefreshNameLabel();
}

void ATMOPHistoricalAgent::SetPersonCategoryId(const FName InCategoryId)
{
    PersonCategoryId = InCategoryId;
    RefreshNameLabel();
}

bool ATMOPHistoricalAgent::ApplyInitialSeatAssignment()
{
    if (!InitialSeatAssignment.bStartsSeated ||
        InitialSeatAssignment.SeatId.IsNone())
    {
        return false;
    }

    UGameInstance* GameInstance =
        GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;

    if (GameInstance == nullptr)
    {
        return false;
    }

    UTMOPCinemaSeatSubsystem* Seats =
        GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>();

    if (Seats == nullptr)
    {
        return false;
    }

    Seats->DiscoverSeatsInWorld();

    UTMOPCinemaSeatComponent* Seat =
        Seats->FindSeat(InitialSeatAssignment.SeatId);

    if (!IsValid(Seat))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("TMOP agent '%s' could not find initial seat '%s'."),
            *GetName(),
            *InitialSeatAssignment.SeatId.ToString());
        return false;
    }

    if (!Seat->SeatAgent(this))
    {
        UE_LOG(
            LogTemp,
            Warning,
            TEXT("TMOP agent '%s' could not occupy seat '%s'."),
            *GetName(),
            *InitialSeatAssignment.SeatId.ToString());
        return false;
    }

    return true;
}

bool ATMOPHistoricalAgent::SetLifeState(
    const ETMOPAgentLifeState NewState)
{
    if (LifeState == NewState)
    {
        return false;
    }

    const ETMOPAgentLifeState OldState = LifeState;
    LifeState = NewState;

    HandleLifeStateChanged(OldState, NewState);
    OnLifeStateChanged.Broadcast(OldState, NewState);
    return true;
}

bool ATMOPHistoricalAgent::SetActivityState(
    const ETMOPAgentActivityState NewActivity)
{
    if (ActivityState == NewActivity)
    {
        return false;
    }

    const ETMOPAgentActivityState OldActivity = ActivityState;
    ActivityState = NewActivity;

    ApplyMovementSpeedForActivity();
    HandleActivityStateChanged(OldActivity, NewActivity);
    OnActivityStateChanged.Broadcast(OldActivity, NewActivity);
    return true;
}

void ATMOPHistoricalAgent::ApplyMovementSpeedForActivity()
{
    if (UCharacterMovementComponent* Movement =
        GetCharacterMovement())
    {
        Movement->MaxWalkSpeed = GetDesiredMovementSpeed();
        Movement->SetMovementMode(
            CanMove() ? MOVE_Walking : MOVE_None);
    }
}

float ATMOPHistoricalAgent::GetDesiredMovementSpeed() const
{
    float BaseSpeed = 0.0f;

    switch (ActivityState)
    {
    case ETMOPAgentActivityState::Walking:
        BaseSpeed = MovementProfile.NormalWalkSpeed;
        break;
    case ETMOPAgentActivityState::FastWalking:
        BaseSpeed = MovementProfile.FastWalkSpeed;
        break;
    case ETMOPAgentActivityState::Jogging:
        BaseSpeed = MovementProfile.JogSpeed;
        break;
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Fleeing:
        BaseSpeed = MovementProfile.RunSpeed;
        break;
    case ETMOPAgentActivityState::Sprinting:
        BaseSpeed = MovementProfile.SprintSpeed;
        break;
    default:
        BaseSpeed = 0.0f;
        break;
    }

    return FMath::Max(
        0.0f,
        BaseSpeed * MovementProfile.PersonalSpeedMultiplier *
            AppearanceMovementSpeedMultiplier);
}

bool ATMOPHistoricalAgent::CanMove() const
{
    if (LifeState != ETMOPAgentLifeState::Alive)
    {
        return false;
    }

    switch (ActivityState)
    {
    case ETMOPAgentActivityState::Walking:
    case ETMOPAgentActivityState::FastWalking:
    case ETMOPAgentActivityState::Jogging:
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Sprinting:
    case ETMOPAgentActivityState::Fleeing:
        return true;
    default:
        return false;
    }
}

bool ATMOPHistoricalAgent::HasInitialSeatAssignment() const
{
    return !InitialSeatAssignment.VenueId.IsNone() &&
        !InitialSeatAssignment.SeatId.IsNone();
}

void ATMOPHistoricalAgent::HandleLifeStateChanged(
    const ETMOPAgentLifeState OldState,
    const ETMOPAgentLifeState NewState)
{
    ApplyMovementSpeedForActivity();
}

void ATMOPHistoricalAgent::HandleActivityStateChanged(
    const ETMOPAgentActivityState OldActivity,
    const ETMOPAgentActivityState NewActivity)
{
}
