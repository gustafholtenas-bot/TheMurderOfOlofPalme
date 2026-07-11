#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Traffic/TMOPTrafficSignalTypes.h"
#include "TMOPTrafficSignalController.generated.h"

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTrafficSignalController : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTrafficSignalController();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    FName IntersectionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    TArray<FTMOPTrafficSignalPhase> Phases;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bCycleAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    int32 InitialPhaseIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic Signal")
    int32 CurrentPhaseIndex = INDEX_NONE;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool SetPhase(int32 NewPhaseIndex);

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    void ForceGroupState(FName SignalGroupId, ETMOPTrafficSignalState NewState);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic Signal")
    ETMOPTrafficSignalState GetGroupState(FName SignalGroupId, bool& bFound) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool ValidateController(TArray<FString>& OutErrors) const;

private:
    void ApplyCurrentPhase();
    float RemainingPhaseSeconds = 0.0f;
    TMap<FName, ETMOPTrafficSignalState> RuntimeStates;
};
