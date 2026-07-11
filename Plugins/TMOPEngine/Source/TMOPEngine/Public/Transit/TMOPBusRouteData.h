#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TMOPBusRouteData.generated.h"

/** Create DA_BUS_43 and DA_BUS_53 from this class. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPBusRouteData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Route")
    FName RouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Route")
    FText PublicLineNumber;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Route")
    FText DestinationDisplay;

    /** Exact traffic lane sequence for this direction/run. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Route")
    TArray<FName> OrderedLaneIds;

    /** Stops in visit order. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Route")
    TArray<FName> OrderedStopIds;
};
