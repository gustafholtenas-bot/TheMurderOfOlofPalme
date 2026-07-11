#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPVehicleInteractionComponent.generated.h"

class ATMOPHistoricalAgent;
class ATMOPVehicleBase;

/**
 * Put on a Historical Agent to expose simple enter/exit calls to Blueprint.
 */
UCLASS(
    ClassGroup = (TMOP),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleInteractionComponent
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPVehicleInteractionComponent();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Vehicle")
    bool EnterVehicle(
        ATMOPVehicleBase* Vehicle,
        FName PreferredSeatId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Vehicle")
    bool EnterDriverSeat(ATMOPVehicleBase* Vehicle);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Vehicle")
    bool ExitCurrentVehicle();

    UFUNCTION(BlueprintPure, Category = "TMOP|Vehicle")
    ATMOPVehicleBase* GetCurrentVehicle() const
    {
        return CurrentVehicle;
    }

private:
    ATMOPHistoricalAgent* GetAgentOwner() const;

    UPROPERTY(Transient)
    TObjectPtr<ATMOPVehicleBase> CurrentVehicle;
};
