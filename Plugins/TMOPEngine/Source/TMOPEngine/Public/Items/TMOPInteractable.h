#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "TMOPInteractable.generated.h"

UINTERFACE(BlueprintType)
class TMOPENGINE_API UTMOPInteractable : public UInterface
{
    GENERATED_BODY()
};

class TMOPENGINE_API ITMOPInteractable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TMOP|Interaction")
    bool Interact(AActor* Interactor);

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="TMOP|Interaction")
    FText GetInteractionText() const;
};
