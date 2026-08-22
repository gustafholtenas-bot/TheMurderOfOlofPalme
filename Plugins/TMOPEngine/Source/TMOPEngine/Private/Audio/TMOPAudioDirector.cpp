#include "Audio/TMOPAudioDirector.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Audio/TMOPAgentAudioComponent.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "Audio/TMOPVenueAudioEmitter.h"
#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Time/TMOPClockSubsystem.h"
#include "Vehicles/TMOPVehicleBase.h"

ATMOPAudioDirector::ATMOPAudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void ATMOPAudioDirector::BeginPlay()
{
    Super::BeginPlay();
    DiscoverRuntimeActors();
}

void ATMOPAudioDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopScheduledAudio();
    Super::EndPlay(EndPlayReason);
}

void ATMOPAudioDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    DiscoveryAccumulator += DeltaSeconds;
    if (DiscoveryAccumulator >= DiscoveryIntervalSeconds)
    {
        DiscoveryAccumulator = 0.0f;
        DiscoverRuntimeActors();
    }

    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock == nullptr) return;
    const int32 CurrentSecond = Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE && CurrentSecond < LastEvaluatedSecond)
        StopScheduledAudio();
    if (CurrentSecond != LastEvaluatedSecond)
    {
        EvaluateSchedule(CurrentSecond);
        LastEvaluatedSecond = CurrentSecond;
    }
}

bool ATMOPAudioDirector::FindSoundDefinition(
    const FName AudioId, FTMOPSoundLibraryRow& OutDefinition) const
{
    if (SoundLibraryTable == nullptr || AudioId.IsNone()) return false;
    const FTMOPSoundLibraryRow* Direct =
        SoundLibraryTable->FindRow<FTMOPSoundLibraryRow>(AudioId, TEXT("TMOP Audio"), false);
    if (Direct != nullptr)
    {
        OutDefinition = *Direct;
        return true;
    }
    for (const TPair<FName, uint8*>& Pair : SoundLibraryTable->GetRowMap())
    {
        const FTMOPSoundLibraryRow* Row =
            reinterpret_cast<const FTMOPSoundLibraryRow*>(Pair.Value);
        if (Row != nullptr && Row->AudioId == AudioId)
        {
            OutDefinition = *Row;
            return true;
        }
    }
    return false;
}

bool ATMOPAudioDirector::FindVehicleProfile(
    const FName ProfileId, FTMOPVehicleAudioProfileRow& OutProfile) const
{
    if (VehicleAudioProfileTable == nullptr || ProfileId.IsNone()) return false;
    const FTMOPVehicleAudioProfileRow* Row =
        VehicleAudioProfileTable->FindRow<FTMOPVehicleAudioProfileRow>(
            ProfileId, TEXT("TMOP Vehicle Audio"), false);
    if (Row == nullptr) return false;
    OutProfile = *Row;
    return true;
}

USoundBase* ATMOPAudioDirector::ResolveSound(
    const FTMOPSoundLibraryRow& Definition) const
{
    TArray<TSoftObjectPtr<USoundBase>> Choices;
    if (!Definition.Sound.IsNull()) Choices.Add(Definition.Sound);
    for (const TSoftObjectPtr<USoundBase>& Variant : Definition.Variants)
        if (!Variant.IsNull()) Choices.Add(Variant);
    if (Choices.IsEmpty()) return nullptr;
    return Choices[FMath::RandRange(0, Choices.Num() - 1)].LoadSynchronous();
}

UAudioComponent* ATMOPAudioDirector::Play2DById(
    const FName AudioId, const float VolumeMultiplier)
{
    FTMOPSoundLibraryRow Definition;
    if (!FindSoundDefinition(AudioId, Definition)) return nullptr;
    USoundBase* Sound = ResolveSound(Definition);
    if (!IsValid(Sound)) return nullptr;
    UAudioComponent* Audio = NewObject<UAudioComponent>(this);
    Audio->bAutoDestroy = true;
    Audio->bAllowSpatialization = false;
    Audio->SetSound(Sound);
    Audio->SetVolumeMultiplier(Definition.Volume * VolumeMultiplier);
    Audio->SetPitchMultiplier(FMath::FRandRange(Definition.PitchMin, Definition.PitchMax));
    Audio->RegisterComponentWithWorld(GetWorld());
    Audio->Play();
    return Audio;
}

UAudioComponent* ATMOPAudioDirector::PlayAtLocationById(
    const FName AudioId, const FVector Location, const float VolumeMultiplier)
{
    FTMOPSoundLibraryRow Definition;
    if (!FindSoundDefinition(AudioId, Definition)) return nullptr;
    USoundBase* Sound = ResolveSound(Definition);
    if (!IsValid(Sound)) return nullptr;
    UAudioComponent* Audio = NewObject<UAudioComponent>(this);
    Audio->bAutoDestroy = true;
    Audio->bAllowSpatialization = Definition.bSpatial;
    Audio->bOverrideAttenuation = Definition.bSpatial;
    Audio->AttenuationOverrides.bAttenuate = Definition.bSpatial;
    Audio->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
    Audio->AttenuationOverrides.AttenuationShapeExtents = FVector(Definition.InnerRadiusCm, 0.0f, 0.0f);
    Audio->AttenuationOverrides.FalloffDistance = Definition.FalloffDistanceCm;
    Audio->SetSound(Sound);
    Audio->SetVolumeMultiplier(Definition.Volume * VolumeMultiplier);
    Audio->SetPitchMultiplier(FMath::FRandRange(Definition.PitchMin, Definition.PitchMax));
    Audio->SetWorldLocation(Location);
    Audio->RegisterComponentWithWorld(GetWorld());
    Audio->Play();
    return Audio;
}

UAudioComponent* ATMOPAudioDirector::PlayAttachedById(
    const FName AudioId, USceneComponent* AttachTo, const float VolumeMultiplier)
{
    if (!IsValid(AttachTo)) return nullptr;
    FTMOPSoundLibraryRow Definition;
    if (!FindSoundDefinition(AudioId, Definition)) return nullptr;
    USoundBase* Sound = ResolveSound(Definition);
    if (!IsValid(Sound)) return nullptr;
    UAudioComponent* Audio = NewObject<UAudioComponent>(this);
    Audio->bAutoDestroy = true;
    Audio->bAllowSpatialization = Definition.bSpatial;
    Audio->bOverrideAttenuation = Definition.bSpatial;
    Audio->AttenuationOverrides.bAttenuate = Definition.bSpatial;
    Audio->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;
    Audio->AttenuationOverrides.AttenuationShapeExtents = FVector(Definition.InnerRadiusCm, 0.0f, 0.0f);
    Audio->AttenuationOverrides.FalloffDistance = Definition.FalloffDistanceCm;
    Audio->SetSound(Sound);
    Audio->SetVolumeMultiplier(Definition.Volume * VolumeMultiplier);
    Audio->SetPitchMultiplier(FMath::FRandRange(Definition.PitchMin, Definition.PitchMax));
    Audio->RegisterComponentWithWorld(GetWorld());
    Audio->AttachToComponent(AttachTo, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
    Audio->Play();
    return Audio;
}

void ATMOPAudioDirector::DiscoverRuntimeActors()
{
    if (bAttachAudioToSpawnedAgents)
        for (TActorIterator<ATMOPHistoricalAgent> It(GetWorld()); It; ++It)
            if (It->FindComponentByClass<UTMOPAgentAudioComponent>() == nullptr)
            {
                UTMOPAgentAudioComponent* Component =
                    NewObject<UTMOPAgentAudioComponent>(*It, TEXT("TMOPAgentAudio"));
                It->AddInstanceComponent(Component);
                Component->RegisterComponent();
            }

    if (bAttachAudioToSpawnedVehicles)
        for (TActorIterator<ATMOPVehicleBase> It(GetWorld()); It; ++It)
            if (It->FindComponentByClass<UTMOPVehicleAudioComponent>() == nullptr)
            {
                UTMOPVehicleAudioComponent* Component =
                    NewObject<UTMOPVehicleAudioComponent>(*It, TEXT("TMOPVehicleAudio"));
                const FString Identity =
                    (It->VehicleId.ToString() + TEXT(" ") +
                     It->VehicleCategoryId.ToString() + TEXT(" ") +
                     It->GetClass()->GetName()).ToUpper();
                if (Identity.Contains(TEXT("AMBULANCE")) ||
                    Identity.Contains(TEXT("AMBULANS")) ||
                    Identity.Contains(TEXT("A951")) || Identity.Contains(TEXT("A912")))
                    Component->AudioProfileId = TEXT("AMBULANCE_VOLVO");
                else if (Identity.Contains(TEXT("POLICE")) ||
                    Identity.Contains(TEXT("POLIS")) ||
                    Identity.Contains(TEXT("RADIOBIL")) || Identity.Contains(TEXT("PIKET")))
                    Component->AudioProfileId = TEXT("POLICE_VOLVO");
                else if (Identity.Contains(TEXT("BUS")) || Identity.Contains(TEXT("SCANIA")))
                    Component->AudioProfileId = TEXT("BUS_SCANIA_CN112");
                It->AddInstanceComponent(Component);
                Component->RegisterComponent();
            }

    if (bSpawnVenueEmittersAutomatically && VenueAudioTable != nullptr)
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
        for (const TPair<FName, uint8*>& Pair : VenueAudioTable->GetRowMap())
        {
            const FTMOPVenueAudioRow* Profile =
                reinterpret_cast<const FTMOPVenueAudioRow*>(Pair.Value);
            if (Profile == nullptr || Profile->VenueId.IsNone()) continue;
            bool bExists = false;
            for (TActorIterator<ATMOPVenueAudioEmitter> It(GetWorld()); It; ++It)
                if (It->VenueId == Profile->VenueId) { bExists = true; break; }
            if (bExists) continue;
            ATMOPHistoricalAnchor* Anchor = Anchors != nullptr
                ? Anchors->FindAnchor(Profile->PlacementAnchorId) : nullptr;
            if (!IsValid(Anchor))
            {
                UE_LOG(LogTemp, Warning,
                    TEXT("TMOP Audio: venue '%s' has no placement anchor '%s'."),
                    *Profile->VenueId.ToString(), *Profile->PlacementAnchorId.ToString());
                continue;
            }
            const FVector Location = Anchor->GetActorLocation();
            ATMOPVenueAudioEmitter* Emitter =
                GetWorld()->SpawnActor<ATMOPVenueAudioEmitter>(Location, FRotator::ZeroRotator);
            if (IsValid(Emitter))
            {
                Emitter->VenueId = Profile->VenueId;
                Emitter->VenueAudioTable = VenueAudioTable;
                Emitter->bSnapToPlacementAnchorOnBeginPlay = false;
                Emitter->RefreshFromVenueTable();
            }
        }
    }
}

void ATMOPAudioDirector::EvaluateSchedule(const int32 CurrentSecond)
{
    if (ScheduledAudioTable == nullptr) return;
    UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;

    for (const TPair<FName, uint8*>& Pair : ScheduledAudioTable->GetRowMap())
    {
        const FTMOPScheduledAudioRow* Row =
            reinterpret_cast<const FTMOPScheduledAudioRow*>(Pair.Value);
        if (Row == nullptr || !Row->bEnabled) continue;
        const bool bActiveNow = CurrentSecond >= Row->StartSecond &&
            (Row->EndSecond < 0 || CurrentSecond < Row->EndSecond);
        UAudioComponent* Existing = ActiveScheduledAudio.FindRef(Pair.Key).Get();
        if (!bActiveNow)
        {
            if (IsValid(Existing)) Existing->FadeOut(Row->FadeOutSeconds, 0.0f);
            ActiveScheduledAudio.Remove(Pair.Key);
            continue;
        }
        if (IsValid(Existing) && Existing->IsPlaying()) continue;

        UAudioComponent* Started = nullptr;
        if (Row->bBackground2D)
            Started = Play2DById(Row->AudioId, Row->VolumeMultiplier);
        else if (Anchors != nullptr)
            if (ATMOPHistoricalAnchor* Anchor = Anchors->FindAnchor(Row->AnchorId))
                Started = PlayAtLocationById(
                    Row->AudioId, Anchor->GetActorLocation(), Row->VolumeMultiplier);
        if (IsValid(Started))
        {
            Started->FadeIn(Row->FadeInSeconds,
                Started->VolumeMultiplier, 0.0f);
            ActiveScheduledAudio.Add(Pair.Key, Started);
        }
    }
}

void ATMOPAudioDirector::StopScheduledAudio()
{
    for (TPair<FName, TWeakObjectPtr<UAudioComponent>>& Pair : ActiveScheduledAudio)
        if (UAudioComponent* Audio = Pair.Value.Get()) Audio->Stop();
    ActiveScheduledAudio.Reset();
    LastEvaluatedSecond = INDEX_NONE;
}
