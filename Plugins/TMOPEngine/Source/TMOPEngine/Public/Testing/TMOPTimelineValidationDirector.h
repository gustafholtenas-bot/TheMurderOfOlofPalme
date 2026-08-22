#pragma once

#include "CoreMinimal.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "GameFramework/Actor.h"
#include "TMOPTimelineValidationDirector.generated.h"

UENUM(BlueprintType)
enum class ETMOPTimelineValidationSeverity : uint8
{
    Passed,
    Warning,
    Error
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTimelineValidationRecord
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName EntityId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName EntryId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString Event;
    UPROPERTY(BlueprintReadOnly) FName TargetAnchorId = NAME_None;
    UPROPERTY(BlueprintReadOnly) int32 PlannedSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) int32 ActualSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) float TimeDeviationSeconds = 0.0f;
    UPROPERTY(BlueprintReadOnly) float DistanceToTargetCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FVector ActualLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) ETMOPTimelineValidationSeverity Severity =
        ETMOPTimelineValidationSeverity::Passed;
    UPROPERTY(BlueprintReadOnly) FString Message;
};

/**
 * Observes every spawned historical agent without changing the simulation.
 * Records actual action starts, arrivals/failures and stationary navigation,
 * then exports CSV and JSON reports to Saved/TMOP/Validation.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTimelineValidationDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTimelineValidationDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation")
    bool bStartAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation",
        meta=(ClampMin="0.05", Units="s"))
    float SampleIntervalSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation",
        meta=(ClampMin="1.0", Units="cm"))
    float StationaryDistanceCm = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation",
        meta=(ClampMin="1.0", Units="s"))
    float StuckAfterSeconds = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation",
        meta=(ClampMin="1.0", Units="cm"))
    float ArrivalWarningDistanceCm = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation",
        meta=(ClampMin="0", Units="s"))
    int32 TimingWarningSeconds = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation",
        meta=(ClampMin="0", Units="s"))
    int32 TimingErrorSeconds = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation")
    bool bExportOnEndPlay = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Validation")
    TArray<FTMOPTimelineValidationRecord> Records;

    UFUNCTION(BlueprintCallable, Category="TMOP|Validation")
    void StartValidation();

    UFUNCTION(BlueprintCallable, Category="TMOP|Validation")
    void StopValidation(bool bExportReports = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Validation")
    bool ExportReports();

    UFUNCTION(BlueprintPure, Category="TMOP|Validation")
    int32 GetTrackedAgentCount() const { return TrackedAgents.Num(); }

private:
    struct FTrackedAgent
    {
        TWeakObjectPtr<class ATMOPHistoricalAgent> Agent;
        TWeakObjectPtr<UTMOPActionExecutorComponent> Executor;
        FVector LastLocation = FVector::ZeroVector;
        float StationarySeconds = 0.0f;
        bool bStuckReportedForCurrentMove = false;
        FName ActiveEntryId = NAME_None;
    };

    void DiscoverAgents();
    void SampleAgents(float DeltaSeconds);
    void AddRecord(const FTMOPTimelineValidationRecord& Record);
    int32 GetSimulationSecond() const;
    FName GetEntityId(const UTMOPActionExecutorComponent* Executor) const;

    void HandleActionValidation(
        UTMOPActionExecutorComponent* Executor,
        const FTMOPScheduleEntry& Entry,
        FTMOPTime ScheduledTime,
        ETMOPActionExecutionState State);

    TMap<FName, FTrackedAgent> TrackedAgents;
    float SampleAccumulator = 0.0f;
    bool bValidationActive = false;
};

