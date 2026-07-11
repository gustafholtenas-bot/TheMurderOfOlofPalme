#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPPersonRegistryDirector.generated.h"

class UDataTable;

/** Configures the central person DataTable when the level starts. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPersonRegistryDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPPersonRegistryDirector();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|People")
    TObjectPtr<UDataTable> PersonProfileTable;

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    int32 RefreshAllActiveProfiles();
};
