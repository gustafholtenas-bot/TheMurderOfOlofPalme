#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPTrafficSignalDirector.generated.h"

class ATMOPTrafficSignalController;
class UTMOPTrafficStopLineComponent;

/** Applies signal-controlled stop constraints to all active traffic vehicles. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTrafficSignalDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTrafficSignalDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bStopOnYellow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal",
        meta=(ClampMin="0.02"))
    float UpdateIntervalSeconds = 0.10f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    int32 DiscoverSignalSystem();

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool ValidateSignalSystem(TArray<FString>& OutErrors) const;

private:
    void UpdateVehicleConstraints();
    ATMOPTrafficSignalController* FindControllerForGroup(FName SignalGroupId) const;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTMOPTrafficStopLineComponent>> StopLines;

    UPROPERTY(Transient)
    TArray<TObjectPtr<ATMOPTrafficSignalController>> Controllers;

    float UpdateAccumulator = 0.0f;
};
