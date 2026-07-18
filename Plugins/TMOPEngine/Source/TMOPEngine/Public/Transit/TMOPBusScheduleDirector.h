#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Agents/TMOPAgentTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPBusScheduleDirector.generated.h"

class ATMOPVehicleBase;
class UTMOPBusPassengerManifest;
class UTMOPBusRouteData;

UENUM(BlueprintType)
enum class ETMOPBusRunState : uint8
{
    Pending,
    Active,
    Completed,
    Failed
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    FName InitialLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule",
        meta=(ClampMin="0.0"))
    float InitialDistanceAlongLane = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Time")
    bool bUseExactStartTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule|Time")
    FTMOPTime ExactStartTime = FTMOPTime(23, 0, 0);

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule")
    int32 ScheduleSeed = 19860228;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Schedule",
        meta=(ClampMin="1"))
    int32 MaximumSimultaneousBuses = 4;

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

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool SpawnRun(FTMOPBusRunRuntime& Runtime);
    void CompleteRun(FTMOPBusRunRuntime& Runtime);
    void MonitorActiveRuns(FTMOPTime CurrentTime);

    UPROPERTY(Transient)
    TArray<FTMOPBusRunRuntime> RuntimeRuns;

    int32 CurrentLoopNumber = 1;
    int32 LastEvaluatedSeconds = INDEX_NONE;
    bool bIsResetting = false;
};
