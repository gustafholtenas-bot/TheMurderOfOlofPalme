#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPVehicleTakeoverComponent.generated.h"

class ACharacter;
class ATMOPVehicleBase;
class UTMOPVehicleDoorComponent;
class UTMOPVehicleSeatComponent;

UENUM(BlueprintType)
enum class ETMOPVehicleTakeoverResult : uint8
{
    SuccessEmptySeat,
    SuccessDriverRemoved,
    FailedNoVehicle,
    FailedNoSeat,
    FailedOccupiedPassengerSeat,
    FailedDriverCannotBeRemoved,
    FailedDriverResisted,
    FailedInternal
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTMOPVehicleTakeoverSignature, ETMOPVehicleTakeoverResult, Result,
    ATMOPVehicleBase*, Vehicle, ACharacter*, PreviousOccupant);

/** Player-side GTA-style seat selection and driver removal. Possession comes in 0050. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleTakeoverComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPVehicleTakeoverComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Takeover")
    float InteractionRangeCm = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Takeover")
    int32 TakeoverSeed = 19860228;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Vehicle Takeover")
    TObjectPtr<ATMOPVehicleBase> CurrentVehicle;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Vehicle Takeover")
    TObjectPtr<UTMOPVehicleSeatComponent> CurrentSeat;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Vehicle Takeover|Events")
    FTMOPVehicleTakeoverSignature OnTakeoverResolved;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Takeover")
    ETMOPVehicleTakeoverResult TryEnterNearestVehicle(bool bPreferDriverSeat = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Takeover")
    ETMOPVehicleTakeoverResult TryEnterVehicle(ATMOPVehicleBase* Vehicle,
        bool bPreferDriverSeat = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Takeover")
    bool ExitCurrentVehicle();

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Takeover")
    bool IsInsideVehicle() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Takeover")
    bool IsDriver() const;

private:
    ATMOPVehicleBase* FindNearestVehicle() const;
    UTMOPVehicleSeatComponent* SelectSeat(ATMOPVehicleBase* Vehicle,
        bool bPreferDriverSeat) const;
    UTMOPVehicleDoorComponent* FindDoorForSeat(ATMOPVehicleBase* Vehicle,
        FName SeatId) const;
    bool DriverResists(ACharacter* Driver, bool& bWillAttack, bool& bWillFlee);
    void ApplyDriverRemovalReaction(ACharacter* Driver, bool bViolent);
    ETMOPVehicleTakeoverResult ResolveAndBroadcast(ETMOPVehicleTakeoverResult Result,
        ATMOPVehicleBase* Vehicle, ACharacter* PreviousOccupant);

    int32 AttemptCounter = 0;
};
