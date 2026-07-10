#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPAnchorAutoRegistrationComponent.generated.h"

UCLASS(
    ClassGroup = (TMOP),
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPAnchorAutoRegistrationComponent final
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPAnchorAutoRegistrationComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
