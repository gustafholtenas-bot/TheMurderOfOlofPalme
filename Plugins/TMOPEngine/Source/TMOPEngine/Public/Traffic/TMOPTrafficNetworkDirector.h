#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPTrafficNetworkDirector.generated.h"

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTrafficNetworkDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTrafficNetworkDirector();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic")
    bool bValidateOnBeginPlay = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool ValidateCompleteTrafficNetwork(TArray<FString>& OutErrors) const;
};
