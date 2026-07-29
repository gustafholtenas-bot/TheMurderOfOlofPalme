#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Observations/TMOPObservationTypes.h"
#include "TMOPObservationDirector.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPObservationEvaluatedSignature,
    FName,
    ObservationId,
    ETMOPObservationRuntimeState,
    Result);

/**
 * Loads observation data, resolves canonical times and checks whether source
 * observations occur. It deliberately never changes actor movement or timing.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPObservationDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPObservationDirector();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Observations|Tables")
    TObjectPtr<UDataTable> ObservationTable = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Observations|Tables")
    TObjectPtr<UDataTable> ObservationLinkTable = nullptr;

    /** Inline rows override table rows with the same ID. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observations")
    TArray<FTMOPObservationDefinition> ObservationDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observations")
    TArray<FTMOPObservationLinkDefinition> ObservationLinks;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Observations")
    FTMOPObservationEvaluatedSignature OnObservationEvaluated;

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    int32 ReloadObservationData();

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    int32 ResolveCanonicalTimes();

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    void ResetObservationRuntime();

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    bool EvaluateObservationNow(FName ObservationId);

    UFUNCTION(BlueprintPure, Category="TMOP|Observations")
    bool TryGetObservationDefinition(
        FName ObservationId,
        FTMOPObservationDefinition& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Observations")
    bool TryGetObservationRuntime(
        FName ObservationId,
        FTMOPObservationRuntime& OutRuntime) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Observations")
    TArray<FTMOPObservationRuntime> GetAllObservationRuntime() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    bool ValidateObservationData(TArray<FString>& OutErrors) const;

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool ResolveCanonicalTime(FName ObservationId);
    bool EvaluateGeometry(
        const FTMOPObservationDefinition& Definition,
        FTMOPObservationRuntime& Runtime) const;
    AActor* FindEntityActor(FName EntityId) const;

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationDefinition> LoadedObservations;

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationLinkDefinition> LoadedLinks;

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationRuntime> RuntimeObservations;
};
