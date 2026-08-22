#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Agents/TMOPAgentTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPBusScheduleDirector.generated.h"

class ATMOPVehicleBase;
class ATMOPPersonRegistryDirector;
class UDataTable;
class UTMOPBusPassengerManifest;
class UTMOPBusRouteData;

UENUM(BlueprintType)
enum class ETMOPBusRunState : uint8
{
    Pending,
    Staged,
    Active,
    Completed,
    Failed
};

/** DataTable overlay for source-backed driver and activation data per RunId. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBusRunPeopleRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run")
    FName RunId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run")
    bool bEnabledInSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run")
    FName DriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run|Time")
    bool bOverrideExactStartTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run|Time",
        meta=(EditCondition="bOverrideExactStartTime"))
    FTMOPTime ExactStartTime = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Run|Source")
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBusScheduledRun
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    FName RunId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    TObjectPtr<UTMOPBusRouteData> RouteData;

    /** Optional explicit historical passenger/driver manifest. Never generated randomly. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Passengers")
    TObjectPtr<UTMOPBusPassengerManifest> PassengerManifest;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    TSubclassOf<ATMOPVehicleBase> BusClass;

    /** Person EntityId from DT_TMOP_People. Spawned and seated when the run starts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Driver")
    FName DriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    FName InitialLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule",
        meta=(ClampMin="0.0"))
    float InitialDistanceAlongLane = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Time")
    bool bUseExactStartTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Time")
    FTMOPTime ExactStartTime = FTMOPTime(23, 0, 0);

    /** Multiplies the speed limit stored on each lane. 0.55 means 27.5 km/h on a 50 road. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Driving",
        meta=(ClampMin="0.1", ClampMax="1.0"))
    float SpeedLimitMultiplier = 0.55f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Time",
        meta=(EditCondition="!bUseExactStartTime"))
    FTMOPTime EarliestStartTime = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Time",
        meta=(EditCondition="!bUseExactStartTime"))
    FTMOPTime LatestStartTime = FTMOPTime(23, 2, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|End")
    bool bUseForcedDespawnTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|End",
        meta=(EditCondition="bUseForcedDespawnTime"))
    FTMOPTime ForcedDespawnTime = FTMOPTime(23, 45, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|End")
    bool bDespawnWhenTrafficRouteCompletes = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Reconstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Source")
    FString SourceReference;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBusRunRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Schedule")
    FName RunId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Schedule")
    FTMOPTime ResolvedStartTime;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Schedule")
    ETMOPBusRunState State = ETMOPBusRunState::Pending;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Schedule")
    TObjectPtr<ATMOPVehicleBase> SpawnedBus;

    int32 SourceIndex = INDEX_NONE;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPBusRunEventSignature, FName, RunId, ATMOPVehicleBase*, Bus);

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPBusScheduleDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPBusScheduleDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    TArray<FTMOPBusScheduledRun> ScheduledRuns;

    /**
     * Authoritative person table for historical drivers and passengers.
     * When empty, the director reuses the table assigned to the world's
     * TMOPPersonRegistryDirector.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|People")
    TObjectPtr<UDataTable> PersonProfileTable;

    /** Optional DT_TMOP_BusRuns overlay. Row Name and RunId must match. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|People")
    TObjectPtr<UDataTable> BusRunConfigurationTable;

    /**
     * Passenger movement is read from DT_TMOP_People timeline entries whose
     * TargetEntityId equals the bus RunId. Legacy PassengerManifest assets are
     * ignored while this is enabled, preventing duplicate passengers.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|People")
    bool bUsePeopleTimelinesForPassengers = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    int32 ScheduleSeed = 19860228;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule",
        meta=(ClampMin="1"))
    int32 MaximumSimultaneousBuses = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Spawning",
        meta=(ClampMin="0", Units="s"))
    int32 SpawnLeadSeconds = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Spawning",
        meta=(ClampMin="50.0", Units="cm"))
    float SpawnClearanceRadiusCm = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    bool bResetWhenTimeMovesBackwards = true;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Bus Schedule|Events")
    FTMOPBusRunEventSignature OnBusRunSpawned;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Bus Schedule|Events")
    FTMOPBusRunEventSignature OnBusRunCompleted;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Schedule")
    bool ResolveSchedule(int32 LoopNumber);

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Schedule")
    void EvaluateSchedule(FTMOPTime CurrentTime);

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Schedule")
    void ResetBusSchedule();

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Schedule")
    bool ValidateSchedule(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Schedule")
    TArray<FTMOPBusRunRuntime> GetRuntimeRuns() const { return RuntimeRuns; }

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Schedule")
    int32 GetActiveBusCount() const;

    /** Number of enabled people with at least one runtime timeline entry for RunId. */
    UFUNCTION(BlueprintPure, Category="TMOP|Bus Schedule|People")
    int32 GetTimelinePassengerCount(FName RunId) const;

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool SpawnRun(FTMOPBusRunRuntime& Runtime);
    bool ActivateStagedRun(FTMOPBusRunRuntime& Runtime);
    void CompleteRun(FTMOPBusRunRuntime& Runtime);
    void MonitorActiveRuns(FTMOPTime CurrentTime);
    ATMOPPersonRegistryDirector* FindPeopleDirector() const;
    const FTMOPBusRunPeopleRow* FindRunPeopleRow(FName RunId) const;
    FName ResolveDriverEntityId(const FTMOPBusScheduledRun& Run) const;

    UPROPERTY(Transient)
    TArray<FTMOPBusRunRuntime> RuntimeRuns;

    int32 CurrentLoopNumber = 1;
    int32 LastEvaluatedSeconds = INDEX_NONE;
    bool bIsResetting = false;
};
