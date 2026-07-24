#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
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

    /**
     * Normal gameplay respects bSpawnInSimulation on every row. Disable this
     * temporarily to inspect all staging vehicles exported from Blender.
     */
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

    TMap<FName, FHistoricalVehicleRuntime> RuntimeVehicles;
};
