#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Venues/TMOPGrandRowComponent.h"
#include "TMOPCinemaSeatExitComponent.generated.h"

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPCinemaSeatExitComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPCinemaSeatExitComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Seat Exit")
    FName RowId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Seat Exit")
    FName RowExitRouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Seat Exit")
    FName AuditoriumExitRouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Seat Exit")
    ETMOPGrandAisleSide PreferredAisle = ETMOPGrandAisleSide::Automatic;
};
