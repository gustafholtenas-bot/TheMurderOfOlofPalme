#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "TMOPHistoricalVehicleDirector.generated.h"

class ATMOPVehicleBase;
class UDataTable;

/**
 * Loads DT_TMOP_HistoricalVehicles, reuses matching vehicles already placed in
 * the level, and spawns enabled historical vehicles at their initial transform.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPHistoricalVehicleDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPHistoricalVehicleDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles")
    TObjectPtr<UDataTable> HistoricalVehicleTable;

    /** Used when a row does not provide VehicleClass. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    TSubclassOf<ATMOPVehicleBase> DefaultVehicleClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bSpawnVehiclesAutomatically = true;

    /** Only rows enabled for simulation are considered for scheduled spawning. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bRespectRowSpawnFlags = true;

    /** Reuse level actors whose VehicleId matches a DataTable row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bReusePlacedVehicles = true;

    /** Boundary vehicles appear only this many seconds before first driving. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="0", Units="s"))
    int32 EntrySpawnLeadSeconds = 10;

    /** Do not spawn a vehicle on top of another vehicle at an entry anchor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="50.0", Units="cm"))
    float EntrySpawnClearanceRadiusCm = 300.0f;

    /** Number of deterministic queue positions searched behind an occupied entry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="1", ClampMax="8"))
    int32 EntrySpawnQueueSlots = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="100.0", Units="cm"))
    float EntrySpawnQueueSpacingCm = 450.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles")
    int32 InitializeHistoricalVehicles();

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles")
    int32 SpawnEnabledVehicles();

    /** Explicit staging/debug action which ignores bSpawnInSimulation. */
    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Historical Vehicles|Debug")
    void SpawnAllVehiclesForStaging();

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles|Validation")
    bool ValidateHistoricalVehicleTable(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Historical Vehicles")
    ATMOPVehicleBase* FindHistoricalVehicle(FName VehicleId) const;

    /**
     * Starts a parked historical vehicle on a deterministic lane route.
     * The requesting person must occupy its driver seat.
     */
    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles|Driving")
    bool BeginDrivingVehicle(
        FName VehicleId,
        FName DriverEntityId,
        const TArray<FName>& OrderedLaneIds,
        const TArray<FName>& PassAnchorIds,
        ETMOPVehicleRouteMode RouteMode =
            ETMOPVehicleRouteMode::ManualLaneRoute,
        FName DestinationAnchorId = NAME_None,
        float StartDistanceAlongFirstLaneCm = 0.0f);

private:
    struct FHistoricalVehicleRuntime
    {
        FName RowName = NAME_None;
        FTMOPHistoricalVehicleRow Profile;
        TWeakObjectPtr<ATMOPVehicleBase> Vehicle;
        bool bSpawnedByDirector = false;
        bool bDeferredPlacedVehicle = false;
        bool bTimelineDespawned = false;
        int32 InitialSpawnSecond = INDEX_NONE;
    };

    void DiscoverPlacedVehicles();
    int32 SpawnDueVehicles(int32 CurrentSecond);
    void DespawnDueVehicles(int32 CurrentSecond);
    int32 GetInitialSpawnSecond(
        const FTMOPHistoricalVehicleRow& Profile) const;
    void ApplyDeferredPlacedVehicleState(int32 CurrentSecond);
    ATMOPVehicleBase* SpawnVehicle(FHistoricalVehicleRuntime& Runtime,
        const FTransform* SpawnTransformOverride = nullptr);
    FTransform GetInitialTransform(const FTMOPHistoricalVehicleRow& Profile) const;
    bool FindClearInitialSpawnTransform(
        const FTMOPHistoricalVehicleRow& Profile,
        FTransform& OutTransform) const;
    bool ShouldSpawn(const FTMOPHistoricalVehicleRow& Profile, bool bIgnoreRowFlag) const;
    void RegisterVehicle(ATMOPVehicleBase* Vehicle) const;
    void UnregisterVehicle(ATMOPVehicleBase* Vehicle) const;
    int32 SpawnVehicles(bool bIgnoreRowFlags);
    const FTMOPHistoricalVehicleTimelineEntry* FindDrivingEntry(
        const FTMOPHistoricalVehicleRow& Profile,
        FName DriverEntityId) const;

    TMap<FName, FHistoricalVehicleRuntime> RuntimeVehicles;
    int32 LastEvaluatedSecond = INDEX_NONE;
};
