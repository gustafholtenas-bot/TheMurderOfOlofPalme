#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TMOPNavigationPolicyLibrary.generated.h"

class UNavigationQueryFilter;

UCLASS()
class TMOPENGINE_API UTMOPNavigationPolicyLibrary final
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "TMOP|Navigation")
    static bool PolicyNormallyAvoidsRoad(
        ETMOPMovementPolicy Policy);

    UFUNCTION(BlueprintPure, Category = "TMOP|Navigation")
    static bool PolicyAllowsHistoricalRoadOverride(
        ETMOPMovementPolicy Policy);

    UFUNCTION(BlueprintPure, Category = "TMOP|Navigation")
    static float GetSuggestedSurfaceCost(
        ETMOPRouteSurfacePreference SurfacePreference);

    UFUNCTION(BlueprintPure, Category = "TMOP|Navigation")
    static TSubclassOf<UNavigationQueryFilter> GetFilterClassForPolicy(
        ETMOPMovementPolicy Policy);
};
