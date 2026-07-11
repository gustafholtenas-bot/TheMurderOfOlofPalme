#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPCinemaSeatExitComponent.generated.h"

/**
 * Optional routing metadata for a cinema chair actor.
 * Add beside TMOPCinemaSeatComponent when this seat needs a specific row/aisle
 * route. Empty values fall back to the Grand director defaults.
 */
UCLASS(
    ClassGroup = (TMOP),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPCinemaSeatExitComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPCinemaSeatExitComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat Exit")
    FName RowExitRouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat Exit")
    FName AuditoriumExitRouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat Exit")
    bool bPreferLeftAisle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat Exit")
    bool bPreferRightAisle = false;
};
