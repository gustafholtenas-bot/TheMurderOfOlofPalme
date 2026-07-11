#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPGrandFilmBehaviorComponent.generated.h"

/** Per-person choice for when this spectator stands up. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPGrandFilmBehaviorComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPGrandFilmBehaviorComponent();

    /** True: stand when the film has fully ended. False: stand when credits begin. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film")
    bool bStandAtFilmEnd = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Film")
    bool bHasStoodThisLoop = false;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film")
    bool StandFromAssignedSeat();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film")
    void ResetForLoop();
};
