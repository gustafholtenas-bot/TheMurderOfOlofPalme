#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPCinemaSeatSubsystem.generated.h"

class UTMOPCinemaSeatComponent;

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPCinemaSeatSubsystem final
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seats")
    bool RegisterSeat(UTMOPCinemaSeatComponent* Seat);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seats")
    bool UnregisterSeat(UTMOPCinemaSeatComponent* Seat);

    UFUNCTION(BlueprintPure, Category = "TMOP|Seats")
    UTMOPCinemaSeatComponent* FindSeat(FName SeatId) const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seats")
    int32 DiscoverSeatsInWorld();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seats")
    int32 RemoveInvalidSeats();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seats")
    bool ValidateSeatRegistry(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Seats")
    int32 GetSeatCount() const;

private:
    TMap<FName, TWeakObjectPtr<UTMOPCinemaSeatComponent>> Seats;
};
