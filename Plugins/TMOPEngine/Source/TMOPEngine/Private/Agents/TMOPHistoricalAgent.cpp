#include "Agents/TMOPHistoricalAgent.h"

#include "AI/TMOPHistoricalAIController.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TextRenderComponent.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "Routes/TMOPRouteFollowerComponent.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

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
}

void ATMOPHistoricalAgent::BeginPlay()
{
    Super::BeginPlay();
    RefreshNameLabel();
    ApplyMovementSpeedForActivity();
    ApplyInitialSeatAssignment();
}

void ATMOPHistoricalAgent::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

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

    NameLabel->SetText(LabelText);
    NameLabel->SetRelativeLocation(FVector(0.0f, 0.0f, NameLabelHeightCm));
    NameLabel->SetWorldSize(NameLabelWorldSize);
    NameLabel->SetTextRenderColor(NameLabelColor);
    NameLabel->SetVisibility(bShowNameLabel, true);
    SetActorTickEnabled(bShowNameLabel);
}

void ATMOPHistoricalAgent::SetNameLabelVisible(const bool bVisible)
{
    bShowNameLabel = bVisible;
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
        BaseSpeed * MovementProfile.PersonalSpeedMultiplier);
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
