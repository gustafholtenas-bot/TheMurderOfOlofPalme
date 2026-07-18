#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "TMOPHistoricalEventDirector.generated.h"

class UDataTable;

/**
 * Registers the project's central historical events with the game-instance
 * event subsystem. Events can be authored directly on the actor, in a DataTable,
 * or in both places.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPHistoricalEventDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPHistoricalEventDirector();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    /** Optional DataTable whose row type is TMOP Historical Event Definition. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Historical Events")
    TObjectPtr<UDataTable> EventTable = nullptr;

    /** Events authored directly on this director. These override equal table IDs. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Events")
    TArray<FTMOPHistoricalEventDefinition> EventDefinitions;

    /** Replace an event with the same EventId that another system registered earlier. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Events")
    bool bReplaceExistingDefinitions = true;

    /** Remove definitions registered by this actor when it leaves the world. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Events")
    bool bUnregisterOnEndPlay = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Events")
    int32 RegisterAllEvents();

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Events")
    void UnregisterAllEvents();

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Events")
    bool ValidateEvents(TArray<FString>& OutErrors) const;

private:
    bool RegisterDefinition(const FTMOPHistoricalEventDefinition& Definition);

    UPROPERTY(Transient)
    TArray<FName> RegisteredEventIds;
};
