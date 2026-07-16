#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPLaneAnchorComponent.generated.h"

class ATMOPLaneSplineActor;

/** Add this component to a bus-stop or other traffic anchor Blueprint. */
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class THEMURDEROFOLOFPALME_API UTMOPLaneAnchorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPLaneAnchorComponent();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane Reference")
    FName LaneID;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane Reference")
    float DistanceAlongLaneCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Lane Reference")
    TObjectPtr<ATMOPLaneSplineActor> LaneActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Lane Reference")
    bool bSnapOwnerToLane = false;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Lane Reference")
    void AttachToNearestLane();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Lane Reference")
    void RefreshFromLaneID();

    UFUNCTION(BlueprintPure, Category="TMOP|Lane Reference")
    FVector GetLaneWorldLocation() const;
};
