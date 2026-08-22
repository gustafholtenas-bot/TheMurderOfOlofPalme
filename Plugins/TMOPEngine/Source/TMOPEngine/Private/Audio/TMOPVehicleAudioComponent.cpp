#include "Audio/TMOPVehicleAudioComponent.h"

#include "Audio/TMOPAudioDirector.h"
#include "Components/AudioComponent.h"
#include "EngineUtils.h"
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

    if (bEngineEnabled && !EngineLoop.IsValid())
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

    if (bEmergencySirenEnabled && !SirenLoop.IsValid())
        SetEmergencySirenEnabled(true);
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

void UTMOPVehicleAudioComponent::SetEmergencySirenEnabled(const bool bEnabled)
{
    bEmergencySirenEnabled = bEnabled;
    if (!bEnabled)
    {
        if (UAudioComponent* Siren = SirenLoop.Get()) Siren->FadeOut(0.25f, 0.0f);
        SirenLoop.Reset();
        return;
    }
    if (Profile.SirenAudioId.IsNone() || SirenLoop.IsValid() || GetOwner() == nullptr) return;
    for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
    {
        SirenLoop = It->PlayAttachedById(
            Profile.SirenAudioId, GetOwner()->GetRootComponent(), 1.0f);
        break;
    }
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
