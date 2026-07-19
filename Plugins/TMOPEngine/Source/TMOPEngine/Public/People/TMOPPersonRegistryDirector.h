#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPersonRegistryDirector.generated.h"

class ATMOPHistoricalAgent;
class ATMOPVehicleBase;
class ATMOPGroupDirector;
class UDataTable;

/** Configures the central person DataTable when the level starts. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPersonRegistryDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPPersonRegistryDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People")
    TObjectPtr<UDataTable> PersonProfileTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    TSubclassOf<ATMOPHistoricalAgent> DefaultAgentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    bool bSpawnPeopleAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    bool bCatchUpToCurrentClockOnBeginPlay = true;

    /** Build the existing generic TMOP groups from DT_TMOP_People. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Groups")
    bool bCreateGroupsFromPeopleTable = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People|Simulation")
    FTMOPTime SimulationEpoch = FTMOPTime(23, 0, 0);

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    int32 RefreshAllActiveProfiles();

    UFUNCTION(BlueprintCallable, Category="TMOP|People|Simulation")
    int32 InitializePersonSimulation();

    UFUNCTION(BlueprintCallable, Category="TMOP|People|Simulation")
    bool ValidatePeopleTable(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category="TMOP|People|Simulation")
    ATMOPHistoricalAgent* FindSpawnedPerson(FName EntityId) const;

private:
    struct FPersonRuntime
    {
        FName RowName = NAME_None;
        FTMOPPersonProfileRow Profile;
        TWeakObjectPtr<ATMOPHistoricalAgent> Agent;
        int32 NextTimelineIndex = 0;
        bool bSpawnedByDirector = false;
        bool bCompleted = false;
        int32 CachedResolvedSecond = INDEX_NONE;
    };

    void EvaluatePeople(int32 CurrentSecond, bool bCatchUp);
    bool SpawnPerson(FPersonRuntime& Runtime, const FTMOPPersonTimelineEntry& InitialEntry);
    bool ApplyTimelineEntry(FPersonRuntime& Runtime,
        const FTMOPPersonTimelineEntry& Entry, bool bCatchUp);
    bool ApplyPlacement(ATMOPHistoricalAgent* Agent,
        const FTMOPPersonTimelineEntry& Entry, bool bCatchUp);
    bool IsAgentBusy(const ATMOPHistoricalAgent* Agent) const;
    bool ResolveEntrySecond(FPersonRuntime& Runtime,
        const FTMOPPersonTimelineEntry& Entry, int32& OutSecond) const;
    int32 EstimateTravelSeconds(const FPersonRuntime& Runtime,
        const FTMOPPersonTimelineEntry& Entry) const;
    ATMOPGroupDirector* FindGroupDirector() const;
    void RebuildGroupsFromPeople();
    ATMOPVehicleBase* FindVehicle(FName VehicleId) const;

    TMap<FName, FPersonRuntime> RuntimePeople;
    int32 LastEvaluatedSecond = INDEX_NONE;
};
