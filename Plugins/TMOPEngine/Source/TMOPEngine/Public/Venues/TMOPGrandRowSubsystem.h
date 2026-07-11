#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Venues/TMOPGrandRowComponent.h"
#include "TMOPGrandRowSubsystem.generated.h"

class UTMOPGrandRowComponent;

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPGrandRowSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Rows")
    bool RegisterRow(UTMOPGrandRowComponent* Row);

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Rows")
    bool UnregisterRow(UTMOPGrandRowComponent* Row);

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Rows")
    UTMOPGrandRowComponent* FindRow(FName RowId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Rows")
    UTMOPGrandRowComponent* FindRowForSeat(FName SeatId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Rows")
    int32 DiscoverRowsInWorld();

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Rows")
    int32 GetRowCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Rows")
    FTransform ResolveSeatAisleTransform(FName SeatId, ETMOPGrandAisleSide PreferredSide,
        bool& bFound) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Rows")
    bool ValidateLayout(TArray<FString>& OutErrors) const;

private:
    TMap<FName, TWeakObjectPtr<UTMOPGrandRowComponent>> Rows;
};
