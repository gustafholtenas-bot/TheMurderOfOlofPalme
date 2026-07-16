#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPLaneSplineActor.generated.h"

class USplineComponent;

UCLASS(BlueprintType)
class THEMURDEROFOLOFPALME_API ATMOPLaneSplineActor : public AActor
{
    GENERATED_BODY()

public:
    ATMOPLaneSplineActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Lane")
    TObjectPtr<USplineComponent> LaneSpline;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") FName LaneID;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") FName RoadID;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") FString Direction;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") int32 LaneIndexFromRight = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") bool bLegalForNormalTraffic = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") bool bIsCrossing = false;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") FName FromLaneID;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") FName ToLaneID;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane") FString TurnType;

    void SetLanePoints(const TArray<FVector>& WorldPoints);
};
