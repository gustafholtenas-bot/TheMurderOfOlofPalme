#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Observations/TMOPObservationTypes.h"
#include "TMOPObservationSignalementLibrary.generated.h"

/** Conservative comparison of source-specific witness descriptions. */
UCLASS()
class TMOPENGINE_API UTMOPObservationSignalementLibrary final
    : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="TMOP|Observations|Signalement")
    static bool HasUsableSignalement(
        const FTMOPObservationDefinition& Observation);

    UFUNCTION(BlueprintPure, Category="TMOP|Observations|Signalement")
    static FTMOPSignalementComparison CompareSignalements(
        const FTMOPObservationDefinition& FirstObservation,
        const FTMOPObservationDefinition& SecondObservation);
};
