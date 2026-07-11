#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPTrafficSpawner.generated.h"

class ATMOPVehicleBase;
class UTMOPVehicleAppearanceData;
class UTMOPVehicleModelData;

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTrafficSpawnEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn")
    TSubclassOf<ATMOPVehicleBase> VehicleClass;

    /** Optional catalog model. VehicleClass must inherit TMOPConfiguredVehicle. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn|Catalog")
    TObjectPtr<UTMOPVehicleModelData> VehicleModel;

    /** Optional civilian, taxi, police or ambulance appearance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn|Catalog")
    TObjectPtr<UTMOPVehicleAppearanceData> AppearancePreset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn")
    FName InitialLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn",
        meta=(ClampMin="0.0"))
    float StartDistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn")
    TArray<FName> PlannedLaneIds;
};

/** Reproducible traffic population for a simulation session. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTrafficSpawner : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTrafficSpawner();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn")
    TArray<FTMOPTrafficSpawnEntry> SpawnEntries;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn")
    bool bSpawnAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Spawn",
        meta=(ClampMin="100.0"))
    float MinimumInitialCenterSpacingCm = 700.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Spawn")
    int32 SpawnTraffic();

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Spawn")
    int32 ClearSpawnedTraffic();

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Spawn")
    bool ValidateSpawnEntries(TArray<FString>& OutErrors) const;

private:
    UPROPERTY(Transient)
    TArray<TObjectPtr<ATMOPVehicleBase>> SpawnedVehicles;
};
