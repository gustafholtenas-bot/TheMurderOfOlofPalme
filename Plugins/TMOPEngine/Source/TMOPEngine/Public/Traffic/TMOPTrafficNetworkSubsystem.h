#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPTrafficNetworkSubsystem.generated.h"

class UTMOPTrafficLaneComponent;

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPTrafficNetworkSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool RegisterLane(UTMOPTrafficLaneComponent* Lane);

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool UnregisterLane(UTMOPTrafficLaneComponent* Lane);

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    UTMOPTrafficLaneComponent* FindLane(FName LaneId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    int32 DiscoverLanesInWorld();

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    TArray<FName> GetAllLaneIds() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool ValidateNetwork(TArray<FString>& OutErrors) const;

private:
    TMap<FName, TWeakObjectPtr<UTMOPTrafficLaneComponent>> Lanes;
};
