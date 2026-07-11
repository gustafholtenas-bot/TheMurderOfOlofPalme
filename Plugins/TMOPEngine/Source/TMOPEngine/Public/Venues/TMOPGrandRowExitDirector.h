#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Venues/TMOPGrandRowComponent.h"
#include "TMOPGrandRowExitDirector.generated.h"

class ATMOPHistoricalAgent;
class UTMOPGrandRowComponent;

UENUM(BlueprintType)
enum class ETMOPGrandRowExitState : uint8
{
    Queued,
    MovingToAisle,
    ReachedAisle,
    Failed
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandRowExitStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Row Exit")
    TObjectPtr<ATMOPHistoricalAgent> Agent;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Row Exit")
    FName SeatId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Row Exit")
    FName RowId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Row Exit")
    ETMOPGrandAisleSide AisleSide = ETMOPGrandAisleSide::Automatic;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Row Exit")
    ETMOPGrandRowExitState State = ETMOPGrandRowExitState::Queued;
};

/** Queues standing spectators and moves them from their row to an aisle. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandRowExitDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandRowExitDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row Exit",
        meta=(ClampMin="10.0"))
    float AisleAcceptanceRadius = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row Exit",
        meta=(ClampMin="1.0"))
    float ScanIntervalSeconds = 0.20f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row Exit")
    bool bAutoProcessStandingAgents = true;

    /** If true, left and right aisle of the same row may move one person each. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Row Exit")
    bool bAllowBothAislesInParallel = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Row Exit")
    bool QueueAgent(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Row Exit")
    void ResetRowExits();

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row Exit")
    int32 GetQueuedCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row Exit")
    int32 GetMovingCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row Exit")
    int32 GetReachedAisleCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Row Exit")
    TArray<FTMOPGrandRowExitStatus> GetExitStatuses() const { return ExitStatuses; }

private:
    void ScanForStandingAgents();
    void StartAvailableAgents();
    void UpdateMovingAgents();
    bool StartMoving(FTMOPGrandRowExitStatus& Status);
    bool IsLaneActive(FName RowId, ETMOPGrandAisleSide Side) const;
    int32 GetDistanceInSeats(const FTMOPGrandRowExitStatus& Status) const;

    UPROPERTY(Transient)
    TArray<FTMOPGrandRowExitStatus> ExitStatuses;

    float ScanAccumulator = 0.0f;
};
