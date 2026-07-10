#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPWorldEntity.generated.h"

class UTMOPWorldEntityComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPWorldEntity : public AActor
{
    GENERATED_BODY()

public:
    ATMOPWorldEntity();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Entity")
    TObjectPtr<UTMOPWorldEntityComponent> EntityIdentity;
};
