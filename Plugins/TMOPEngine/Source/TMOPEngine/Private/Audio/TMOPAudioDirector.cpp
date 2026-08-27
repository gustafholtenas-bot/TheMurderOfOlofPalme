#include "Audio/TMOPAudioDirector.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Audio/TMOPAgentAudioComponent.h"
#include "Audio/TMOPVehicleAudioComponent.h"
#include "Audio/TMOPVenueAudioEmitter.h"
#include "Camera/PlayerCameraManager.h"
#include "Components/AudioComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "Engine/StaticMesh.h"
#include "Sound/SoundWave.h"
#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"
#include "Time/TMOPClockSubsystem.h"
#include "Vehicles/TMOPVehicleBase.h"

namespace
{
constexpr float TMOPLocalSoundMaxDistanceCm = 500.0f;

bool IsStrictlyLocalSound(const FTMOPSoundLibraryRow& Definition)
{
    return Definition.Category == ETMOPAudioCategory::Footstep ||
        Definition.Category == ETMOPAudioCategory::Vehicle;
}

float GetAudibleDistanceCm(const FTMOPSoundLibraryRow& Definition)
{
    return IsStrictlyLocalSound(Definition)
        ? TMOPLocalSoundMaxDistanceCm
        : FMath::Max(1.0f,
            Definition.InnerRadiusCm + Definition.FalloffDistanceCm);
}

bool HasListenerWithinDistance(
    const UObject* WorldContext, const FVector& SoundLocation,
    const float MaximumDistanceCm)
{
    if (!IsValid(WorldContext)) return false;
    const APlayerCameraManager* Camera =
        UGameplayStatics::GetPlayerCameraManager(WorldContext, 0);
    // Dedicated/server-side simulations have no listener.  Do not allocate
    // local transient sounds there.
    if (!IsValid(Camera)) return false;
    return FVector::DistSquared(
        Camera->GetCameraLocation(), SoundLocation) <=
        FMath::Square(MaximumDistanceCm);
}

void ApplyTMOPAttenuation(
    UAudioComponent* Audio, const FTMOPSoundLibraryRow& Definition)
{
    if (!IsValid(Audio)) return;
    Audio->bAllowSpatialization = Definition.bSpatial;
    Audio->bOverrideAttenuation = Definition.bSpatial;
    Audio->AttenuationOverrides.bAttenuate = Definition.bSpatial;
    Audio->AttenuationOverrides.AttenuationShape = EAttenuationShape::Sphere;

    float InnerRadius = Definition.InnerRadiusCm;
    float FalloffDistance = Definition.FalloffDistanceCm;
    if (IsStrictlyLocalSound(Definition))
    {
        // Full volume close to the source, then fade to silence at exactly 5 m.
        InnerRadius = FMath::Min(100.0f, TMOPLocalSoundMaxDistanceCm);
        FalloffDistance = TMOPLocalSoundMaxDistanceCm - InnerRadius;
    }
    Audio->AttenuationOverrides.AttenuationShapeExtents =
        FVector(InnerRadius, 0.0f, 0.0f);
    Audio->AttenuationOverrides.FalloffDistance = FalloffDistance;
}
}

ATMOPAudioDirector::ATMOPAudioDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
    TrafficLightNameTokens = {
        TEXT("trafikljus"), TEXT("trafficlight"), TEXT("traffic_light")};
}

void ATMOPAudioDirector::BeginPlay()
{
    Super::BeginPlay();
    DiscoverRuntimeActors();
    RefreshTrafficLightAudio();
    const TArray<FName> FootstepRows = {
        TEXT("FOOTSTEP_ASPHALT_WALK"), TEXT("FOOTSTEP_ASPHALT_RUN"),
        TEXT("FOOTSTEP_CARPET_WALK"), TEXT("FOOTSTEP_CARPET_RUN"),
        TEXT("FOOTSTEP_GRASS_WALK"), TEXT("FOOTSTEP_GRASS_RUN"),
        TEXT("FOOTSTEP_SNOW_WALK"), TEXT("FOOTSTEP_SNOW_RUN"),
        TEXT("FOOTSTEP_STAIRS_WALK"), TEXT("FOOTSTEP_STAIRS_RUN"),
        TEXT("FOOTSTEP_STONE_TILES_WALK"), TEXT("FOOTSTEP_STONE_TILES_RUN"),
        TEXT("FOOTSTEP_WOOD_WALK"), TEXT("FOOTSTEP_WOOD_RUN")};
    for (const FName AudioId : FootstepRows)
    {
        FTMOPSoundLibraryRow Definition;
        if (!FindSoundDefinition(AudioId, Definition))
        {
            UE_LOG(LogTemp, Error,
                TEXT("TMOP Audio: required footstep row '%s' is missing."),
                *AudioId.ToString());
            continue;
        }
        TSet<FSoftObjectPath> UniqueSamples;
        if (!Definition.Sound.IsNull())
            UniqueSamples.Add(Definition.Sound.ToSoftObjectPath());
        for (const TSoftObjectPtr<USoundBase>& Variant : Definition.Variants)
            if (!Variant.IsNull()) UniqueSamples.Add(Variant.ToSoftObjectPath());
        if (UniqueSamples.Num() < 5)
            UE_LOG(LogTemp, Warning,
                TEXT("TMOP Audio: footstep row '%s' has %d/5 assigned samples."),
                *AudioId.ToString(), UniqueSamples.Num());
    }
}

void ATMOPAudioDirector::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopScheduledAudio();
    StopTrafficLightAudio();
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
    TrafficLightRefreshAccumulator += DeltaSeconds;
    if (bAttachTrafficLightClicksAutomatically &&
        TrafficLightRefreshAccumulator >= TrafficLightRefreshIntervalSeconds)
    {
        TrafficLightRefreshAccumulator = 0.0f;
        RefreshTrafficLightAudio();
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
    if (AudioId.IsNone()) return false;
    TArray<UDataTable*> Tables;
    for (UDataTable* Additional : AdditionalSoundLibraryTables)
        if (IsValid(Additional)) Tables.Add(Additional);
    if (IsValid(SoundLibraryTable)) Tables.Add(SoundLibraryTable);
    for (const UDataTable* Table : Tables)
    {
        const FTMOPSoundLibraryRow* Direct =
            Table->FindRow<FTMOPSoundLibraryRow>(AudioId, TEXT("TMOP Audio"), false);
        if (Direct != nullptr)
        {
            OutDefinition = *Direct;
            return true;
        }
        for (const TPair<FName, uint8*>& Pair : Table->GetRowMap())
        {
            const FTMOPSoundLibraryRow* Row =
                reinterpret_cast<const FTMOPSoundLibraryRow*>(Pair.Value);
            if (Row != nullptr && Row->AudioId == AudioId)
            {
                OutDefinition = *Row;
                return true;
            }
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
    const FName AudioId, const FTMOPSoundLibraryRow& Definition)
{
    TArray<TSoftObjectPtr<USoundBase>> Choices;
    if (!Definition.Sound.IsNull()) Choices.Add(Definition.Sound);
    for (const TSoftObjectPtr<USoundBase>& Variant : Definition.Variants)
        if (!Variant.IsNull()) Choices.Add(Variant);
    if (Choices.IsEmpty()) return nullptr;
    const FString* Previous = LastResolvedSoundByAudioId.Find(AudioId);
    TArray<int32> EligibleIndices;
    for (int32 Index = 0; Index < Choices.Num(); ++Index)
        if (Choices.Num() == 1 || Previous == nullptr ||
            Choices[Index].ToSoftObjectPath().ToString() != *Previous)
            EligibleIndices.Add(Index);
    if (EligibleIndices.IsEmpty()) EligibleIndices.Add(0);
    const int32 ChoiceIndex = EligibleIndices[
        FMath::RandRange(0, EligibleIndices.Num() - 1)];
    LastResolvedSoundByAudioId.Add(
        AudioId, Choices[ChoiceIndex].ToSoftObjectPath().ToString());
    USoundBase* Resolved = Choices[ChoiceIndex].LoadSynchronous();
    if (USoundWave* Wave = Cast<USoundWave>(Resolved))
        Wave->bLooping = Definition.bLoop;
    return Resolved;
}

UAudioComponent* ATMOPAudioDirector::Play2DById(
    const FName AudioId, const float VolumeMultiplier)
{
    FTMOPSoundLibraryRow Definition;
    if (!FindSoundDefinition(AudioId, Definition)) return nullptr;
    USoundBase* Sound = ResolveSound(AudioId, Definition);
    if (!IsValid(Sound)) return nullptr;
    UAudioComponent* Audio = UGameplayStatics::SpawnSound2D(
        this, Sound, Definition.Volume * VolumeMultiplier,
        FMath::FRandRange(Definition.PitchMin, Definition.PitchMax),
        0.0f, nullptr, false, true);
    return Audio;
}

UAudioComponent* ATMOPAudioDirector::PlayAtLocationById(
    const FName AudioId, const FVector Location, const float VolumeMultiplier)
{
    FTMOPSoundLibraryRow Definition;
    if (!FindSoundDefinition(AudioId, Definition)) return nullptr;
    if (Definition.bSpatial &&
        !HasListenerWithinDistance(this, Location,
            GetAudibleDistanceCm(Definition)))
        return nullptr;
    USoundBase* Sound = ResolveSound(AudioId, Definition);
    if (!IsValid(Sound)) return nullptr;
    UAudioComponent* Audio = UGameplayStatics::SpawnSoundAtLocation(
        this, Sound, Location, FRotator::ZeroRotator,
        Definition.Volume * VolumeMultiplier,
        FMath::FRandRange(Definition.PitchMin, Definition.PitchMax),
        0.0f, nullptr, nullptr, true);
    ApplyTMOPAttenuation(Audio, Definition);
    return Audio;
}

UAudioComponent* ATMOPAudioDirector::PlayAttachedById(
    const FName AudioId, USceneComponent* AttachTo, const float VolumeMultiplier)
{
    if (!IsValid(AttachTo)) return nullptr;
    FTMOPSoundLibraryRow Definition;
    if (!FindSoundDefinition(AudioId, Definition)) return nullptr;
    if (Definition.bSpatial &&
        !HasListenerWithinDistance(this, AttachTo->GetComponentLocation(),
            GetAudibleDistanceCm(Definition)))
        return nullptr;
    USoundBase* Sound = ResolveSound(AudioId, Definition);
    if (!IsValid(Sound)) return nullptr;
    UAudioComponent* Audio = UGameplayStatics::SpawnSoundAttached(
        Sound, AttachTo, NAME_None, FVector::ZeroVector, FRotator::ZeroRotator,
        EAttachLocation::SnapToTarget, true,
        Definition.Volume * VolumeMultiplier,
        FMath::FRandRange(Definition.PitchMin, Definition.PitchMax),
        0.0f, nullptr, nullptr, true);
    ApplyTMOPAttenuation(Audio, Definition);
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

bool ATMOPAudioDirector::FindTrafficLightAttachComponent(AActor* Actor,
    USceneComponent*& OutComponent) const
{
    OutComponent = nullptr;
    if (!IsValid(Actor) || Actor == this) return false;
    auto Matches = [this](FString Identity)
    {
        Identity.ToLowerInline();
        Identity.ReplaceInline(TEXT("_"), TEXT(""));
        Identity.ReplaceInline(TEXT("-"), TEXT(""));
        Identity.ReplaceInline(TEXT(" "), TEXT(""));
        for (FString Token : TrafficLightNameTokens)
        {
            Token.ToLowerInline();
            Token.ReplaceInline(TEXT("_"), TEXT(""));
            Token.ReplaceInline(TEXT("-"), TEXT(""));
            Token.ReplaceInline(TEXT(" "), TEXT(""));
            if (!Token.IsEmpty() && Identity.Contains(Token)) return true;
        }
        return false;
    };

    TArray<UStaticMeshComponent*> Meshes;
    Actor->GetComponents<UStaticMeshComponent>(Meshes);
    if (Meshes.IsEmpty()) return false;
    const bool bActorMatches = Matches(Actor->GetName());
    for (UStaticMeshComponent* Mesh : Meshes)
    {
        if (!IsValid(Mesh)) continue;
        const FString MeshAssetName = IsValid(Mesh->GetStaticMesh())
            ? Mesh->GetStaticMesh()->GetName() : FString();
        if (bActorMatches || Matches(Mesh->GetName()) || Matches(MeshAssetName))
        {
            OutComponent = Mesh;
            return true;
        }
    }
    return false;
}

void ATMOPAudioDirector::RefreshTrafficLightAudio()
{
    if (!bAttachTrafficLightClicksAutomatically || GetWorld() == nullptr)
    {
        StopTrafficLightAudio();
        return;
    }
    const APlayerCameraManager* Camera =
        UGameplayStatics::GetPlayerCameraManager(this, 0);
    if (!IsValid(Camera)) return;
    const FVector ListenerLocation = Camera->GetCameraLocation();
    const float RadiusSquared = FMath::Square(TrafficLightActivationRadiusCm);
    TSet<TWeakObjectPtr<AActor>> SeenNearby;

    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;
        USceneComponent* AttachComponent = nullptr;
        if (!FindTrafficLightAttachComponent(Actor, AttachComponent)) continue;
        if (FVector::DistSquared(ListenerLocation,
            AttachComponent->GetComponentLocation()) > RadiusSquared) continue;
        SeenNearby.Add(Actor);
        const TWeakObjectPtr<AActor> ActorKey(Actor);
        UAudioComponent* Existing = ActiveTrafficLightAudio.FindRef(ActorKey).Get();
        if (IsValid(Existing) && Existing->IsPlaying()) continue;
        if (UAudioComponent* Started = PlayAttachedById(
            TrafficLightClickAudioId, AttachComponent, 1.0f))
            ActiveTrafficLightAudio.Add(ActorKey, Started);
    }

    for (auto It = ActiveTrafficLightAudio.CreateIterator(); It; ++It)
    {
        AActor* Actor = It.Key().Get();
        UAudioComponent* Audio = It.Value().Get();
        if (!IsValid(Actor) || !SeenNearby.Contains(TWeakObjectPtr<AActor>(Actor)))
        {
            if (IsValid(Audio)) Audio->FadeOut(0.15f, 0.0f);
            It.RemoveCurrent();
        }
    }
}

void ATMOPAudioDirector::StopTrafficLightAudio()
{
    for (auto& Pair : ActiveTrafficLightAudio)
        if (UAudioComponent* Audio = Pair.Value.Get()) Audio->Stop();
    ActiveTrafficLightAudio.Reset();
    TrafficLightRefreshAccumulator = 0.0f;
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
