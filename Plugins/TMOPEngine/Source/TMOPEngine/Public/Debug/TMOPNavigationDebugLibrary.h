#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TMOPNavigationDebugLibrary.generated.h"

class ATMOPHistoricalAgent;

UCLASS()
class TMOPENGINE_API UTMOPNavigationDebugLibrary final
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "TMOP|Debug|Navigation")
    static FString BuildAgentNavigationReport(
        const ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Debug|Navigation",
        meta = (WorldContext = "WorldContextObject"))
    static void DrawAgentNavigationDebug(
        UObject* WorldContextObject,
        const ATMOPHistoricalAgent* Agent,
        float Duration = 5.0f);
};
