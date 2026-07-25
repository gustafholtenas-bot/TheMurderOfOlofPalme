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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles")
    TObjectPtr<UDataTable> HistoricalVehicleTable;

    /** Used when a row does not provide VehicleClass. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    TSubclassOf<ATMOPVehicleBase> DefaultVehicleClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bSpawnVehiclesAutomatically = true;

    /** Legacy compatibility flag. Valid historical rows now spawn at start. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bRespectRowSpawnFlags = true;

    /** Reuse level actors whose VehicleId matches a DataTable row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bReusePlacedVehicles = true;

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
    };

    void DiscoverPlacedVehicles();
    ATMOPVehicleBase* SpawnVehicle(FHistoricalVehicleRuntime& Runtime);
    FTransform GetInitialTransform(const FTMOPHistoricalVehicleRow& Profile) const;
    bool ShouldSpawn(const FTMOPHistoricalVehicleRow& Profile, bool bIgnoreRowFlag) const;
    void RegisterVehicle(ATMOPVehicleBase* Vehicle) const;
    void UnregisterVehicle(ATMOPVehicleBase* Vehicle) const;
    int32 SpawnVehicles(bool bIgnoreRowFlags);
    const FTMOPHistoricalVehicleTimelineEntry* FindDrivingEntry(
        const FTMOPHistoricalVehicleRow& Profile,
        FName DriverEntityId) const;

    TMap<FName, FHistoricalVehicleRuntime> RuntimeVehicles;
};
