#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPBusStopSubsystem.generated.h"

class UTMOPBusStopComponent;

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPBusStopSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    bool RegisterStop(UTMOPBusStopComponent* Stop);
    bool UnregisterStop(UTMOPBusStopComponent* Stop);

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Stop")
    UTMOPBusStopComponent* FindStop(FName StopId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Stop")
    int32 DiscoverStopsInWorld();

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Stop")
    bool ValidateStops(TArray<FString>& OutErrors) const;

private:
    TMap<FName, TWeakObjectPtr<UTMOPBusStopComponent>> Stops;
};
