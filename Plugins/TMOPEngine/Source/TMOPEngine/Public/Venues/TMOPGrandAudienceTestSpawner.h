#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPGrandAudienceTestSpawner.generated.h"

class ATMOPHistoricalAgent;

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandAudienceEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    FName EntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    FName SeatId = NAME_None;

    /** Optional override. Falls back to the spawner AgentClass. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    TSubclassOf<ATMOPHistoricalAgent> AgentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    FString SourceReference;
};

/** Spawns Grand's opening state: all configured spectators already seated. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandAudienceTestSpawner : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandAudienceTestSpawner();
    virtual void BeginPlay() override;

    /** Default class for named and anonymous spectators. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    TSubclassOf<ATMOPHistoricalAgent> AgentClass;

    /** Named/documented people with stable identities and assigned seats. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    TArray<FTMOPGrandAudienceEntry> AudienceEntries;

    /** Anonymous spectators. Kept as SeatIds for compatibility with v0.0.26. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    TArray<FName> SeatIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    FName VenueId = TEXT("PLACE_GRAND");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    FName AuditoriumId = TEXT("GRAND_SALON_1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience",
        meta=(ClampMin="1", ClampMax="500"))
    int32 MaximumAgents = 250;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Audience")
    bool bSpawnAutomatically = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Audience")
    int32 SpawnTestAudience();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Audience")
    int32 ClearSpawnedAudience();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Audience")
    int32 ResetAudience();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Audience")
    bool ValidateAudienceConfiguration(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Audience")
    int32 GetSpawnedAudienceCount() const;

private:
    ATMOPHistoricalAgent* SpawnAudienceMember(
        FName EntityId,
        const FText& DisplayName,
        FName SeatId,
        TSubclassOf<ATMOPHistoricalAgent> ClassOverride,
        const FString& SourceReference);

    UPROPERTY(Transient)
    TArray<TObjectPtr<ATMOPHistoricalAgent>> SpawnedAgents;
};
