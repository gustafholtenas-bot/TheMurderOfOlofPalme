#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPGrandAudienceTestSpawner.generated.h"

class ATMOPHistoricalAgent;

/**
 * Spawns a small Grand test audience from a list of Seat IDs.
 * Intended for the first 3-10 agent vertical slice.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandAudienceTestSpawner : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandAudienceTestSpawner();

    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Grand Test")
    TSubclassOf<ATMOPHistoricalAgent> AgentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Grand Test")
    TArray<FName> SeatIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Grand Test")
    FName VenueId = TEXT("PLACE_GRAND");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Grand Test")
    FName AuditoriumId = TEXT("GRAND_SALON_1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Grand Test",
        meta = (ClampMin = "1", ClampMax = "10"))
    int32 MaximumAgents = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Grand Test")
    bool bSpawnAutomatically = true;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Grand Test")
    int32 SpawnTestAudience();
};
