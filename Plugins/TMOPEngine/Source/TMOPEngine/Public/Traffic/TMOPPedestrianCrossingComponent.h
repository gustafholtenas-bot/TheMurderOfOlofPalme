#pragma once
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "TMOPPedestrianCrossingComponent.generated.h"

/** X runs across the road, Y spans the zebra width. Place box ends at the two curbs.
 * People already inside may finish after green ends. Box is query geometry, not a wall. */
UCLASS(ClassGroup=(TMOP), BlueprintType, meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPedestrianCrossingComponent : public UBoxComponent
{
    GENERATED_BODY()
public:
    UTMOPPedestrianCrossingComponent();
    virtual void OnRegister() override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Crossing") FName CrossingId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Crossing") FName IntersectionId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Crossing") FName SignalGroupId = NAME_None;
    /** Vehicle groups which must yield while a pedestrian is physically in this box. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Crossing") TArray<FName> YieldingVehicleGroups;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Crossing", meta=(ClampMin="1"))
    float ClearanceWalkingSpeedCmPerSecond = 100.0f;
    bool ContainsPedestrian(const FVector& Position) const;
    bool CrossesEntrance(const FVector& From, const FVector& To, float MarginCm = 0.0f) const;
};
