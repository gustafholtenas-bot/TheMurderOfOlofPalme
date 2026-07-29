#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "TMOPVehicleBase.generated.h"

class ATMOPHistoricalAgent;
class UBoxComponent;
class USceneComponent;
class UTextRenderComponent;
class UTMOPVehicleSeatComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPVehicleBase : public APawn
{
    GENERATED_BODY()

public:
    ATMOPVehicleBase();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** Root collision used by swept player driving. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle|Collision")
    TObjectPtr<UBoxComponent> VehicleCollision;

    /** Visual/seat origin kept at road level below the collision centre. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle")
    TObjectPtr<USceneComponent> VehicleRoot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle|Identity")
    FText DisplayName;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle|Identity")
    FName VehicleCategoryId = NAME_None;

    /** World-space name shown above the vehicle. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle|Debug")
    TObjectPtr<UTextRenderComponent> NameLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle|Debug")
    bool bShowNameLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle|Debug",
        meta=(ClampMin="0.0", Units="cm"))
    float NameLabelHeightCm = 130.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle|Debug",
        meta=(ClampMin="1.0"))
    float NameLabelWorldSize = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle|Debug")
    FColor NameLabelColor = FColor::White;

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

    /** Refresh after DisplayName or VehicleId changes. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle|Debug")
    void RefreshNameLabel();

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle|Debug")
    void SetNameLabelVisible(bool bVisible);

private:
    bool ShouldDisplayNameLabel() const;
};
