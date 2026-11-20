#include "People/TMOPAnimationLODSubsystem.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "HAL/PlatformTime.h"
#include "People/TMOPCharacterAppearanceComponent.h"

namespace
{
constexpr float RefreshIntervalSeconds = 0.25f;
constexpr float NearDistanceCm = 2500.0f;
constexpr float MidDistanceCm = 5500.0f;
constexpr float FarDistanceCm = 11000.0f;
constexpr float FreezeIdleBeyondCm = 4000.0f;

bool IsMovementActivity(const ETMOPAgentActivityState State)
{
    switch (State)
    {
    case ETMOPAgentActivityState::Walking:
    case ETMOPAgentActivityState::FastWalking:
    case ETMOPAgentActivityState::Jogging:
    case ETMOPAgentActivityState::Running:
    case ETMOPAgentActivityState::Sprinting:
    case ETMOPAgentActivityState::Fleeing:
    case ETMOPAgentActivityState::RidingVehicle:
        return true;
    default:
        return false;
    }
}
}

bool UTMOPAnimationLODSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
    const UWorld* World = Cast<UWorld>(Outer);
    return Super::ShouldCreateSubsystem(Outer) && IsValid(World) &&
        (World->WorldType == EWorldType::Game ||
         World->WorldType == EWorldType::PIE);
}

TStatId UTMOPAnimationLODSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UTMOPAnimationLODSubsystem, STATGROUP_Tickables);
}

void UTMOPAnimationLODSubsystem::RegisterAgent(ATMOPHistoricalAgent* Agent)
{
    if (IsValid(Agent))
        Agents.AddUnique(TWeakObjectPtr<ATMOPHistoricalAgent>(Agent));
}

void UTMOPAnimationLODSubsystem::UnregisterAgent(ATMOPHistoricalAgent* Agent)
{
    Agents.Remove(TWeakObjectPtr<ATMOPHistoricalAgent>(Agent));
}

void UTMOPAnimationLODSubsystem::Deinitialize()
{
    Agents.Reset();
    CachedSeekCameras.Reset();
    Super::Deinitialize();
}

void UTMOPAnimationLODSubsystem::CollectLocalCameraLocations(
    TArray<FVector>& OutLocations) const
{
    const UWorld* World = GetWorld();
    if (!IsValid(World)) return;
    // Include all local player viewpoints, including split-screen and any
    // camera that a local controller currently possesses for inspection.
    for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator();
         It; ++It)
    {
        const APlayerController* Controller = It->Get();
        if (!IsValid(Controller) || !Controller->IsLocalController()) continue;
        FVector Location;
        FRotator Rotation;
        Controller->GetPlayerViewPoint(Location, Rotation);
        OutLocations.Add(Location);
    }
}

void UTMOPAnimationLODSubsystem::UpdateAgent(
    ATMOPHistoricalAgent* Agent, const TArray<FVector>& CameraLocations) const
{
    if (!IsValid(Agent) || Agent->IsActorBeingDestroyed() ||
        !IsValid(Agent->CharacterAppearance)) return;

    // A server with no local view, or a scene being pre-baked, must never
    // have its animations suppressed because of a missing camera.
    if (CameraLocations.IsEmpty())
    {
        Agent->CharacterAppearance->ApplyAnimationTickLOD(0.0f, true);
        return;
    }

    float NearestDistanceSquared = TNumericLimits<float>::Max();
    for (const FVector& CameraLocation : CameraLocations)
        NearestDistanceSquared = FMath::Min(NearestDistanceSquared,
            FVector::DistSquared(CameraLocation, Agent->GetActorLocation()));

    const bool bRecentlyVisible = Agent->WasRecentlyRendered(0.7f);
    const bool bMoving = IsMovementActivity(Agent->ActivityState) ||
        Agent->GetVelocity().SizeSquared2D() > FMath::Square(10.0f);
    const USkeletalMeshComponent* Body = Agent->BodyMesh;
    const UAnimInstance* AnimInstance = IsValid(Body)
        ? Body->GetAnimInstance() : nullptr;
    const bool bCriticalAction =
        Agent->ActivityState == ETMOPAgentActivityState::Interacting ||
        Agent->IsAutomaticSpeechVisible() || Agent->IsDialogueFocused() ||
        (AnimInstance != nullptr && AnimInstance->IsAnyMontagePlaying());

    // Deliberately no change to action schedules, navigation or root position.
    // The values below are visual quality presets, adjustable after profiling.
    float Interval = 0.0f;
    bool bEnableTick = true;
    if (NearestDistanceSquared <= FMath::Square(NearDistanceCm))
        Interval = 0.0f;
    else if (NearestDistanceSquared <= FMath::Square(MidDistanceCm))
        Interval = bRecentlyVisible ? (1.0f / 15.0f) : 0.2f;
    else if (NearestDistanceSquared <= FMath::Square(FarDistanceCm))
        Interval = bRecentlyVisible ? (1.0f / 8.0f) : 0.25f;
    else
        Interval = bRecentlyVisible ? 0.2f : 0.5f;

    // The expensive idle pose can freeze completely once a person is safely
    // outside the camera. Movement, dialogue and montages remain live even
    // offscreen so historical gestures are not stranded in an old pose.
    if (!bRecentlyVisible && !bMoving && !bCriticalAction &&
        NearestDistanceSquared > FMath::Square(FreezeIdleBeyondCm))
        bEnableTick = false;

    Agent->CharacterAppearance->ApplyAnimationTickLOD(Interval, bEnableTick);
}

void UTMOPAnimationLODSubsystem::Tick(float DeltaTime)
{
    TimeSinceUpdateSeconds += FMath::Max(0.0f, DeltaTime);
    if (TimeSinceUpdateSeconds < RefreshIntervalSeconds) return;
    TimeSinceUpdateSeconds = FMath::Fmod(
        TimeSinceUpdateSeconds, RefreshIntervalSeconds);

    TArray<FVector> Cameras;
    CollectLocalCameraLocations(Cameras);
    for (auto It = Agents.CreateIterator(); It; ++It)
    {
        if (!It->IsValid())
            It.RemoveCurrent();
        else
            UpdateAgent(It->Get(), Cameras);
    }
}

void UTMOPAnimationLODSubsystem::RefreshAgentAfterSeek(
    ATMOPHistoricalAgent* Agent)
{
    const double NowSeconds = FPlatformTime::Seconds();
    if (CameraCacheRealSeconds < 0.0 ||
        NowSeconds - CameraCacheRealSeconds > 0.05)
    {
        CachedSeekCameras.Reset();
        CollectLocalCameraLocations(CachedSeekCameras);
        CameraCacheRealSeconds = NowSeconds;
    }
    UpdateAgent(Agent, CachedSeekCameras);
}
