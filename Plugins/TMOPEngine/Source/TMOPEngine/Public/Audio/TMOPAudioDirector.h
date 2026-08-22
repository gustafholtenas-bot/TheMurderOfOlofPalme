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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> ScheduledAudioTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> VenueAudioTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") TObjectPtr<UDataTable> VehicleAudioProfileTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") bool bAttachAudioToSpawnedAgents = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") bool bAttachAudioToSpawnedVehicles = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio") bool bSpawnVenueEmittersAutomatically = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio", meta=(ClampMin="0.1")) float DiscoveryIntervalSeconds = 0.5f;

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
    USoundBase* ResolveSound(const FTMOPSoundLibraryRow& Definition) const;
    void DiscoverRuntimeActors();
    void EvaluateSchedule(int32 CurrentSecond);
    void StopScheduledAudio();

    float DiscoveryAccumulator = 0.0f;
    int32 LastEvaluatedSecond = INDEX_NONE;
    TMap<FName, TWeakObjectPtr<UAudioComponent>> ActiveScheduledAudio;
};
