#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Audio/TMOPAudioTypes.h"
#include "TMOPAudioDirector.generated.h"

class UAudioComponent;
class UDataTable;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPAudioDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPAudioDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> SoundLibraryTable;
    /** Optional focused libraries searched before SoundLibraryTable. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TArray<TObjectPtr<UDataTable>> AdditionalSoundLibraryTables;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> ScheduledAudioTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> VenueAudioTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> VehicleAudioProfileTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") bool bAttachAudioToSpawnedAgents = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") bool bAttachAudioToSpawnedVehicles = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") bool bSpawnVenueEmittersAutomatically = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio", meta=(ClampMin="0.1")) float DiscoveryIntervalSeconds = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Traffic Lights")
    bool bAttachTrafficLightClicksAutomatically = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Traffic Lights")
    FName TrafficLightClickAudioId = TEXT("TRAFFIC_LIGHT_CLICK");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Traffic Lights")
    TArray<FString> TrafficLightNameTokens;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Traffic Lights",
        meta=(ClampMin="100.0", Units="cm"))
    float TrafficLightActivationRadiusCm = 3200.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Traffic Lights",
        meta=(ClampMin="0.25", Units="s"))
    float TrafficLightRefreshIntervalSeconds = 2.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Audio")
    UAudioComponent* Play2DById(FName AudioId, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category="TMOP|Audio")
    UAudioComponent* PlayAtLocationById(FName AudioId, FVector Location, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintCallable, Category="TMOP|Audio")
    UAudioComponent* PlayAttachedById(FName AudioId, USceneComponent* AttachTo, float VolumeMultiplier = 1.0f);

    UFUNCTION(BlueprintPure, Category="TMOP|Audio")
    bool FindSoundDefinition(FName AudioId, FTMOPSoundLibraryRow& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Audio")
    bool FindVehicleProfile(FName ProfileId, FTMOPVehicleAudioProfileRow& OutProfile) const;

private:
    USoundBase* ResolveSound(FName AudioId,
        const FTMOPSoundLibraryRow& Definition);
    void DiscoverRuntimeActors();
    void EvaluateSchedule(int32 CurrentSecond);
    void StopScheduledAudio();
    void RefreshTrafficLightAudio();
    void StopTrafficLightAudio();
    bool FindTrafficLightAttachComponent(AActor* Actor,
        USceneComponent*& OutComponent) const;

    float DiscoveryAccumulator = 0.0f;
    float TrafficLightRefreshAccumulator = 0.0f;
    int32 LastEvaluatedSecond = INDEX_NONE;
    TMap<FName, TWeakObjectPtr<UAudioComponent>> ActiveScheduledAudio;
    TMap<TWeakObjectPtr<AActor>, TWeakObjectPtr<UAudioComponent>> ActiveTrafficLightAudio;
    /** Prevents a row with multiple samples from selecting the same asset on
     * two consecutive plays. */
    TMap<FName, FString> LastResolvedSoundByAudioId;
};
