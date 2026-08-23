#include "Audio/TMOPAgentAudioComponent.h"

#include "Audio/TMOPAudioDirector.h"
#include "Camera/PlayerCameraManager.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

UTMOPAgentAudioComponent::UTMOPAgentAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

void UTMOPAgentAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    PreviousLocation = GetOwner() != nullptr
        ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
}

void UTMOPAgentAudioComponent::TickComponent(
    const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    AActor* Owner = GetOwner();
    if (!bEnableDistanceBasedFootstepFallback || !IsValid(Owner)) return;
    const FVector Location = Owner->GetActorLocation();
    const float Distance = FVector::Dist2D(Location, PreviousLocation);
    PreviousLocation = Location;

    // Do not accumulate, trace surfaces, or allocate audio for people whose
    // footsteps cannot be heard.  Reset the cadence so returning into range
    // never produces a burst of deferred footsteps.
    const APlayerCameraManager* Camera =
        UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!IsValid(Camera) ||
        FVector::DistSquared(Camera->GetCameraLocation(), Location) >
            FMath::Square(MaximumAudibleDistanceCm))
    {
        TravelSinceFootstepCm = 0.0f;
        return;
    }
    const float Speed = DeltaTime > KINDA_SMALL_NUMBER ? Distance / DeltaTime : 0.0f;
    if (Speed < MinimumMovingSpeedCmPerSecond) return;
    if (const ACharacter* Character = Cast<ACharacter>(Owner))
        if (const UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
            if (!Movement->IsMovingOnGround()) return;
    TravelSinceFootstepCm += Distance;
    const float RequiredDistance = Speed >= 360.0f
        ? RunStepDistanceCm : WalkStepDistanceCm;
    if (TravelSinceFootstepCm >= RequiredDistance)
    {
        TravelSinceFootstepCm = FMath::Fmod(TravelSinceFootstepCm, RequiredDistance);
        NotifyFootstep(ResolveSurfaceFootstepId(Speed >= 360.0f));
    }
}

FName UTMOPAgentAudioComponent::ResolveSurfaceFootstepId(const bool bRunning) const
{
    const AActor* Owner = GetOwner();
    if (!IsValid(Owner) || GetWorld() == nullptr)
        return bRunning ? DefaultRunFootstepId : DefaultWalkFootstepId;
    FHitResult Hit;
    FCollisionQueryParams Query(SCENE_QUERY_STAT(TMOPFootstepSurface), false, Owner);
    Query.bReturnPhysicalMaterial = true;
    const FVector Start = Owner->GetActorLocation();
    const FVector End = Start - FVector(0.0f, 0.0f, 160.0f);
    if (!GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Query))
        return bRunning ? DefaultRunFootstepId : DefaultWalkFootstepId;
    switch (UGameplayStatics::GetSurfaceType(Hit))
    {
    case SurfaceType1: return InteriorFootstepId;
    case SurfaceType2: return CarpetFootstepId;
    case SurfaceType3: return StairFootstepId;
    case SurfaceType4: return WetFootstepId;
    default: return bRunning ? DefaultRunFootstepId : DefaultWalkFootstepId;
    }
}

void UTMOPAgentAudioComponent::NotifyFootstep(const FName ExplicitAudioId)
{
    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;
    const FName AudioId = ExplicitAudioId.IsNone()
        ? DefaultWalkFootstepId : ExplicitAudioId;
    for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
    {
        It->PlayAtLocationById(AudioId, Owner->GetActorLocation(), 1.0f);
        break;
    }
}
