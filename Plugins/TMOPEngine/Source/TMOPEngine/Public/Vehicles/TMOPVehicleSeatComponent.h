#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPVehicleSeatComponent.generated.h"

class ACharacter;
class ATMOPHistoricalAgent;

UENUM(BlueprintType)
enum class ETMOPVehicleSeatRole : uint8
{
    Driver,
    FrontPassenger,
    RearLeft,
    RearRight,
    OtherPassenger
};

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleSeatComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPVehicleSeatComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Seat")
    FName SeatId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Seat")
    ETMOPVehicleSeatRole SeatRole = ETMOPVehicleSeatRole::OtherPassenger;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Seat")
    FVector ExitLocalOffset = FVector(0.0f, 110.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Seat")
    FRotator ExitRotationOffset = FRotator::ZeroRotator;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Seat")
    bool IsOccupied() const;

    /** Backward-compatible NPC accessor. Returns null when the player occupies the seat. */
    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Seat")
    ATMOPHistoricalAgent* GetOccupant() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Seat")
    ACharacter* GetOccupantCharacter() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Seat")
    bool EnterSeat(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Seat")
    bool ExitSeat(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Seat")
    bool EnterCharacterSeat(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Seat")
    bool ExitCharacterSeat(ACharacter* Character);

private:
    UPROPERTY(Transient)
    TObjectPtr<ACharacter> CharacterOccupant;
};
