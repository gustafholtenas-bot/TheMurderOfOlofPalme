#include "Audio/TMOPPlayerMovementAudioComponent.h"

#include "Audio/TMOPAudioDirector.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Vehicles/TMOPVehicleBase.h"

UTMOPPlayerMovementAudioComponent::UTMOPPlayerMovementAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

void UTMOPPlayerMovementAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    AActor* Owner = GetOwner();
    if (!IsValid(Owner)) return;
    Owner->OnActorHit.AddUniqueDynamic(this,
        &UTMOPPlayerMovementAudioComponent::HandleOwnerHit);
    if (UTMOPPlayerVehicleSessionComponent* Session =
        Owner->FindComponentByClass<UTMOPPlayerVehicleSessionComponent>())
        Session->OnVehicleSessionStarted.AddUniqueDynamic(this,
            &UTMOPPlayerMovementAudioComponent::HandleVehicleSessionStarted);
    if (const ACharacter* Character = Cast<ACharacter>(Owner))
        if (const UCharacterMovementComponent* Movement =
            Character->GetCharacterMovement())
            bWasFalling = Movement->IsFalling();
}

void UTMOPPlayerMovementAudioComponent::TickComponent(
    const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    const ACharacter* Character = Cast<ACharacter>(GetOwner());
    const UCharacterMovementComponent* Movement = IsValid(Character)
        ? Character->GetCharacterMovement() : nullptr;
    if (!IsValid(Movement)) return;

    ImpactCooldownRemainingSeconds = FMath::Max(
        0.0f, ImpactCooldownRemainingSeconds - DeltaTime);
    const bool bFalling = Movement->IsFalling();
    if (!bWasFalling && bFalling && Movement->Velocity.Z > 0.0f)
        PlayLocal(JumpAudioId);
    else if (bWasFalling && !bFalling)
        PlayLocal(LandAudioId,
            FMath::GetMappedRangeValueClamped(
                FVector2D(150.0f, 900.0f), FVector2D(0.55f, 1.25f),
                FMath::Abs(PreviousVerticalVelocityCmPerSecond)));
    bWasFalling = bFalling;
    PreviousVerticalVelocityCmPerSecond = Movement->Velocity.Z;

    const float HorizontalSpeed = Movement->Velocity.Size2D();
    if (!bFalling && HorizontalSpeed >= RunningBreathMinimumSpeedCmPerSecond)
    {
        BreathElapsedSeconds += DeltaTime;
        if (BreathElapsedSeconds >= RunningBreathIntervalSeconds)
        {
            BreathElapsedSeconds = FMath::Fmod(
                BreathElapsedSeconds, RunningBreathIntervalSeconds);
            PlayLocal(RunBreathAudioId);
        }
    }
    else BreathElapsedSeconds = 0.0f;
}

void UTMOPPlayerMovementAudioComponent::HandleOwnerHit(
    AActor* SelfActor, AActor* OtherActor, const FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (ImpactCooldownRemainingSeconds > 0.0f || !IsValid(SelfActor)) return;
    const float IntoSurfaceSpeed = FMath::Abs(FVector::DotProduct(
        SelfActor->GetVelocity(), Hit.ImpactNormal));
    const float EffectiveSpeed = FMath::Max(
        IntoSurfaceSpeed, NormalImpulse.Size() * 0.01f);
    if (EffectiveSpeed < MinimumImpactSpeedCmPerSecond) return;
    ImpactCooldownRemainingSeconds = ImpactCooldownSeconds;
    PlayLocal(BodyImpactAudioId,
        FMath::GetMappedRangeValueClamped(
            FVector2D(MinimumImpactSpeedCmPerSecond, 900.0f),
            FVector2D(0.45f, 1.2f), EffectiveSpeed));
}

void UTMOPPlayerMovementAudioComponent::HandleVehicleSessionStarted(
    ATMOPVehicleBase* Vehicle, const bool bDriver)
{
    PlayLocal(EnterVehicleAudioId);
}

ATMOPAudioDirector* UTMOPPlayerMovementAudioComponent::FindAudioDirector() const
{
    if (GetWorld() == nullptr) return nullptr;
    for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
        return *It;
    return nullptr;
}

void UTMOPPlayerMovementAudioComponent::PlayLocal(
    const FName AudioId, const float VolumeMultiplier) const
{
    if (ATMOPAudioDirector* Director = FindAudioDirector())
        Director->Play2DById(AudioId, VolumeMultiplier);
}
