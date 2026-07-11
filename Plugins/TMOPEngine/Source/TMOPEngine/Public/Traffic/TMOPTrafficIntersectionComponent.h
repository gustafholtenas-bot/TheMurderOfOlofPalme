#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPTrafficIntersectionComponent.generated.h"

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPTrafficIntersectionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPTrafficIntersectionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Intersection")
    FName IntersectionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Intersection")
    TArray<FName> IncomingLaneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Intersection")
    TArray<FName> OutgoingLaneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic|Intersection")
    bool bSignalControlled = false;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic|Intersection")
    bool ValidateIntersection(TArray<FString>& OutErrors) const;
};
