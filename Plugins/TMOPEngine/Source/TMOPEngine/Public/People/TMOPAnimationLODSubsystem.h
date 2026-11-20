#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "TMOPAnimationLODSubsystem.generated.h"

class ATMOPHistoricalAgent;

/**
 * One world-level LOD scheduler for all TMOP agents. Only the visual skeletal
 * update rate is changed; clocks, movement, schedules and recorded poses are
 * owned by their existing systems. Never create a timer for each NPC.
 */
UCLASS()
class TMOPENGINE_API UTMOPAnimationLODSubsystem : public UTickableWorldSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
    virtual void Tick(float DeltaTime) override;
    virtual TStatId GetStatId() const override;
    virtual void Deinitialize() override;

    void RegisterAgent(ATMOPHistoricalAgent* Agent);
    void UnregisterAgent(ATMOPHistoricalAgent* Agent);

    /** Immediate visibility reclassification after an authoritative time seek. */
    void RefreshAgentAfterSeek(ATMOPHistoricalAgent* Agent);

private:
    TArray<TWeakObjectPtr<ATMOPHistoricalAgent>> Agents;
    float TimeSinceUpdateSeconds = 0.0f;
    // A seek can update hundreds of actors in the same rendered frame.
    // Cache the viewpoints briefly so every actor does not re-query them.
    mutable double CameraCacheRealSeconds = -1.0;
    mutable TArray<FVector> CachedSeekCameras;

    void CollectLocalCameraLocations(TArray<FVector>& OutLocations) const;
    void UpdateAgent(ATMOPHistoricalAgent* Agent,
        const TArray<FVector>& CameraLocations) const;
};
