#include "Audio/TMOPVehicleAudioComponent.h"

#include "Audio/TMOPAudioDirector.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UTMOPVehicleAudioComponent::UTMOPVehicleAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.05f;
}

void UTMOPVehicleAudioComponent::BeginPlay()
{
    Super::BeginPlay();
    PreviousLocation = GetOwner() != nullptr
        ? GetOwner()->GetActorLocation() : FVector::ZeroVector;
    RefreshProfile();
}

void UTMOPVehicleAudioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopLoops();
    Super::EndPlay(EndPlayReason);
}

void UTMOPVehicleAudioComponent::RefreshProfile()
{
    for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
    {
        FTMOPVehicleAudioProfileRow Found;
        if (It->FindVehicleProfile(AudioProfileId, Found)) Profile = Found;
        break;
    }
}

void UTMOPVehicleAudioComponent::TickComponent(
    const float DeltaTime, const ELevelTick TickType,
    FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    AActor* Owner = GetOwner();
    if (!IsValid(Owner) || DeltaTime <= KINDA_SMALL_NUMBER) return;

    const FVector CurrentLocation = Owner->GetActorLocation();
    const float Speed = FVector::Dist2D(CurrentLocation, PreviousLocation) / DeltaTime;
    PreviousLocation = CurrentLocation;
    const float Acceleration = (Speed - PreviousSpeed) / DeltaTime;
    PreviousSpeed = Speed;
    const bool bMoving = Speed > 15.0f;

    const APlayerCameraManager* Camera =
        UGameplayStatics::GetPlayerCameraManager(this, 0);
    const bool bListenerIsNear = IsValid(Camera) &&
        FVector::DistSquared(Camera->GetCameraLocation(), CurrentLocation) <=
        FMath::Square(LocalAudioMaximumDistanceCm);

    // Engine loops are numerous and short-range.  Release them completely
    // outside five metres; Tick will recreate them when the listener returns.
    // Emergency sirens are deliberately handled separately below.
    if (!bListenerIsNear)
    {
        if (UAudioComponent* Engine = EngineLoop.Get()) Engine->Stop();
        if (UAudioComponent* Driving = DrivingLoop.Get()) Driving->Stop();
        EngineLoop.Reset();
        DrivingLoop.Reset();
    }

    if (bListenerIsNear && bEngineEnabled && !EngineLoop.IsValid())
        for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
        {
            EngineLoop = It->PlayAttachedById(
                Profile.EngineIdleAudioId, Owner->GetRootComponent(), 1.0f);
            DrivingLoop = It->PlayAttachedById(
                Profile.EngineDrivingAudioId, Owner->GetRootComponent(), 1.0f);
            break;
        }
    const float NormalizedSpeed = FMath::Clamp(
        Speed / FMath::Max(100.0f, Profile.ReferenceTopSpeedCmPerSecond), 0.0f, 1.0f);
    if (UAudioComponent* Engine = EngineLoop.Get())
    {
        Engine->SetPitchMultiplier(FMath::Lerp(0.82f, 1.12f, NormalizedSpeed));
        Engine->SetVolumeMultiplier(FMath::Lerp(0.75f, 0.08f, NormalizedSpeed));
        if (!Engine->IsPlaying()) Engine->Play();
    }
    if (UAudioComponent* Driving = DrivingLoop.Get())
    {
        Driving->SetPitchMultiplier(FMath::Lerp(0.72f, 1.45f, NormalizedSpeed));
        Driving->SetVolumeMultiplier(FMath::Lerp(0.0f, 1.0f, NormalizedSpeed));
        if (!Driving->IsPlaying()) Driving->Play();
    }

    if (!bWasMoving && bMoving)
    {
        PlayOneShot(Acceleration >= HardStartAccelerationThreshold
            ? Profile.HardStartAudioId : Profile.NormalStartAudioId);
    }
    bWasMoving = bMoving;

    // The emergency flag represents the active emergency response and is also
    // used by the vehicle to keep the blue lights flashing.  The audible
    // siren is movement-gated independently so a parked police car or
    // ambulance remains lit without producing a stationary siren loop.
    if (!bSirenAudibleForMovement && Speed >= SirenStartSpeedCmPerSecond)
        bSirenAudibleForMovement = true;
    else if (bSirenAudibleForMovement && Speed <= SirenStopSpeedCmPerSecond)
        bSirenAudibleForMovement = false;

    const bool bShouldPlaySiren =
        bEmergencySirenEnabled && bSirenAudibleForMovement;
    if (bShouldPlaySiren && !SirenLoop.IsValid())
    {
        if (!Profile.SirenAudioId.IsNone())
            for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
            {
                SirenLoop = It->PlayAttachedById(
                    Profile.SirenAudioId, Owner->GetRootComponent(), 1.0f);
                break;
            }
    }
    else if (!bShouldPlaySiren && SirenLoop.IsValid())
    {
        if (UAudioComponent* Siren = SirenLoop.Get())
            Siren->FadeOut(0.25f, 0.0f);
        SirenLoop.Reset();
    }
}

void UTMOPVehicleAudioComponent::PlayOneShot(
    const FName AudioId, const float Volume)
{
    if (AudioId.IsNone() || GetOwner() == nullptr) return;
    for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
    {
        It->PlayAttachedById(AudioId, GetOwner()->GetRootComponent(), Volume);
        break;
    }
}

void UTMOPVehicleAudioComponent::PlayHorn() { PlayOneShot(Profile.HornAudioId); }
void UTMOPVehicleAudioComponent::PlayDoorOpen() { PlayOneShot(Profile.DoorOpenAudioId); }
void UTMOPVehicleAudioComponent::PlayDoorClose() { PlayOneShot(Profile.DoorCloseAudioId); }
void UTMOPVehicleAudioComponent::PlayDoorCycle()
{
    PlayDoorOpen();
    TWeakObjectPtr<UTMOPVehicleAudioComponent> WeakThis(this);
    FTimerHandle Handle;
    GetWorld()->GetTimerManager().SetTimer(
        Handle,
        [WeakThis]()
        {
            if (UTMOPVehicleAudioComponent* Component = WeakThis.Get())
                Component->PlayDoorClose();
        },
        0.65f, false);
}
void UTMOPVehicleAudioComponent::PlaySkid() { PlayOneShot(Profile.SkidAudioId); }
void UTMOPVehicleAudioComponent::PlayTireBurst() { PlayOneShot(Profile.TireBurstAudioId); }
void UTMOPVehicleAudioComponent::PlayHardStart() { PlayOneShot(Profile.HardStartAudioId); }
void UTMOPVehicleAudioComponent::PlayCollision(const float ImpactSpeedCmPerSecond)
{
    const float ImpactSpeedKmh = FMath::Abs(ImpactSpeedCmPerSecond) *
        (3600.0f / 100000.0f);
    const bool bHighSpeed = ImpactSpeedKmh >= HighSpeedCollisionThresholdKmh;
    PlayOneShot(bHighSpeed
        ? Profile.HighSpeedCollisionAudioId
        : Profile.LowSpeedCollisionAudioId,
        FMath::GetMappedRangeValueClamped(
            FVector2D(2.0f, 60.0f), FVector2D(0.35f, 1.0f), ImpactSpeedKmh));
}

void UTMOPVehicleAudioComponent::SetEmergencySirenEnabled(const bool bEnabled)
{
    bEmergencySirenEnabled = bEnabled;
    if (!bEnabled)
    {
        if (UAudioComponent* Siren = SirenLoop.Get()) Siren->FadeOut(0.25f, 0.0f);
        SirenLoop.Reset();
    }
    // Enabling only arms the siren. TickComponent starts the sound once the
    // vehicle is actually moving. Blue lights continue to read the emergency
    // flag directly and therefore remain active while parked.
}

void UTMOPVehicleAudioComponent::StopLoops()
{
    if (UAudioComponent* Engine = EngineLoop.Get()) Engine->Stop();
    if (UAudioComponent* Driving = DrivingLoop.Get()) Driving->Stop();
    if (UAudioComponent* Siren = SirenLoop.Get()) Siren->Stop();
    EngineLoop.Reset();
    DrivingLoop.Reset();
    SirenLoop.Reset();
}
