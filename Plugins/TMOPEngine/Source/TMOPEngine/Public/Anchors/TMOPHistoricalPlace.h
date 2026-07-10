#pragma once

#include "CoreMinimal.h"
#include "Anchors/TMOPAnchorTypes.h"
#include "Entities/TMOPWorldEntity.h"
#include "TMOPHistoricalPlace.generated.h"

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPHistoricalPlace : public ATMOPWorldEntity
{
    GENERATED_BODY()

public:
    ATMOPHistoricalPlace();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Place")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Place")
    ETMOPPlaceCategory PlaceCategory = ETMOPPlaceCategory::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Place")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Place")
    TArray<FTMOPSourceReference> Sources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Place")
    FString Notes;
};
