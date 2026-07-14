#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TMOPVehicleBase.generated.h"

class ATMOPHistoricalAgent;
class UBoxComponent;
class USceneComponent;
class UTMOPVehicleSeatComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPVehicleBase : public APawn
{
    GENERATED_BODY()

public:
    ATMOPVehicleBase();

    /** Root collision used by swept player driving. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle|Collision")
    TObjectPtr<UBoxComponent> VehicleCollision;

    /** Visual/seat origin kept at road level below the collision centre. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle")
    TObjectPtr<USceneComponent> VehicleRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle")
    bool bAllowPlayerPossession = true;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle")
    TArray<UTMOPVehicleSeatComponent*> GetVehicleSeats() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle")
    UTMOPVehicleSeatComponent* GetDriverSeat() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle")
    bool EnterVehicle(ATMOPHistoricalAgent* Agent, FName PreferredSeatId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle")
    bool EnterDriverSeat(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle")
    bool ExitVehicle(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle")
    ATMOPHistoricalAgent* GetDriverAgent() const;
};
