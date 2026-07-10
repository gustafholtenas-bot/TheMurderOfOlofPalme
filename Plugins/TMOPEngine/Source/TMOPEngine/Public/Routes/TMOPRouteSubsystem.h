#pragma once

#include "CoreMinimal.h"
#include "Routes/TMOPRouteTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TMOPRouteSubsystem.generated.h"

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPRouteSubsystem final
    : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "TMOP|Routes")
    bool RegisterRoute(
        const FTMOPHistoricalRouteDefinition& Route);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Routes")
    bool RemoveRoute(FName RouteId);

    UFUNCTION(BlueprintPure, Category = "TMOP|Routes")
    bool HasRoute(FName RouteId) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Routes")
    bool TryGetRoute(
        FName RouteId,
        FTMOPHistoricalRouteDefinition& OutRoute) const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Routes")
    TArray<FName> GetRegisteredRouteIds() const;

private:
    UPROPERTY(Transient)
    TMap<FName, FTMOPHistoricalRouteDefinition> Routes;
};
