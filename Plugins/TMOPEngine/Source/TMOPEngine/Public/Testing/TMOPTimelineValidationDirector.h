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
    UPROPERTY(BlueprintReadOnly) FName ExpectedShotAnchorId = NAME_None;
    UPROPERTY(BlueprintReadOnly) float DistanceToExpectedShotAnchorCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) bool bAtExpectedShotAnchor = false;
    UPROPERTY(BlueprintReadOnly) int32 ExpectedArrivalSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) float RemainingPathCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) float RequiredSpeedCmPerSecond = -1.0f;
    UPROPERTY(BlueprintReadOnly) bool bPhysicallyPossible = true;
    UPROPERTY(BlueprintReadOnly) FVector ActualLocation = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) ETMOPTimelineValidationSeverity Severity =
        ETMOPTimelineValidationSeverity::Passed;
    UPROPERTY(BlueprintReadOnly) FString Message;

    /** Extended diagnostics; old readers may safely ignore these fields. */
    UPROPERTY(BlueprintReadOnly) FString Action;
    UPROPERTY(BlueprintReadOnly) FString TimingMode;
    UPROPERTY(BlueprintReadOnly) int32 HistoricalSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) bool bScheduledAsArrival = false;
    UPROPERTY(BlueprintReadOnly) FName PreviousEntryId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName PreviousAnchorId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName TargetEntityId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName TargetSeatId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString FailureCode;
    UPROPERTY(BlueprintReadOnly) FString FailureDetails;
    UPROPERTY(BlueprintReadOnly) int32 RetryCount = 0;
    UPROPERTY(BlueprintReadOnly) float RetryDurationSeconds = 0.0f;
    UPROPERTY(BlueprintReadOnly) FName ActualVehicleId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName ActualSeatId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString AttachedParentName;
    UPROPERTY(BlueprintReadOnly) FString VehicleTrafficState;
    UPROPERTY(BlueprintReadOnly) FName VehicleLaneId = NAME_None;
    UPROPERTY(BlueprintReadOnly) float VehicleSpeedCmPerSecond = -1.0f;
    UPROPERTY(BlueprintReadOnly) int32 VehicleRouteRemaining = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) bool bActorCollisionEnabled = true;
    UPROPERTY(BlueprintReadOnly) bool bVehicleObstacleDetectionEnabled = false;
    UPROPERTY(BlueprintReadOnly) FString BlockingActorName;
    UPROPERTY(BlueprintReadOnly) FString BlockingActorClass;
    UPROPERTY(BlueprintReadOnly) float BlockingActorDistanceCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FName GroupId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName GroupLeaderId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString GroupState;
    UPROPERTY(BlueprintReadOnly) FString GroupFormation;
    UPROPERTY(BlueprintReadOnly) float DistanceToGroupLeaderCm = -1.0f;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAgentValidationSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName EntityId = NAME_None;
    UPROPERTY(BlueprintReadOnly) int32 SampleSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) FString Reason;
    UPROPERTY(BlueprintReadOnly) FVector Location = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FVector Velocity = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) FString ActivityState;
    UPROPERTY(BlueprintReadOnly) FString LifeState;
    UPROPERTY(BlueprintReadOnly) FName ActiveEntryId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName TargetAnchorId = NAME_None;
    UPROPERTY(BlueprintReadOnly) float DistanceToTargetCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FName ExpectedShotAnchorId = NAME_None;
    UPROPERTY(BlueprintReadOnly) float DistanceToExpectedShotAnchorCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) bool bAtExpectedShotAnchor = false;
    UPROPERTY(BlueprintReadOnly) float StationarySeconds = 0.0f;
    UPROPERTY(BlueprintReadOnly) FName VehicleId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName SeatId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString AttachedParentName;
    UPROPERTY(BlueprintReadOnly) FName GroupId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FName GroupLeaderId = NAME_None;
    UPROPERTY(BlueprintReadOnly) FString GroupState;
    UPROPERTY(BlueprintReadOnly) FString GroupFormation;
    UPROPERTY(BlueprintReadOnly) float DistanceToGroupLeaderCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) bool bCollisionEnabled = true;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPVehicleValidationSnapshot
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) FName VehicleId = NAME_None;
    UPROPERTY(BlueprintReadOnly) int32 SampleSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) FString Reason;
    UPROPERTY(BlueprintReadOnly) FVector Location = FVector::ZeroVector;
    UPROPERTY(BlueprintReadOnly) float YawDegrees = 0.0f;
    UPROPERTY(BlueprintReadOnly) float SpeedCmPerSecond = -1.0f;
    UPROPERTY(BlueprintReadOnly) FString TrafficState;
    UPROPERTY(BlueprintReadOnly) FName CurrentLaneId = NAME_None;
    UPROPERTY(BlueprintReadOnly) float DistanceAlongLaneCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FName NextLaneId = NAME_None;
    UPROPERTY(BlueprintReadOnly) int32 RemainingLaneCount = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) float StationarySeconds = 0.0f;
    UPROPERTY(BlueprintReadOnly) bool bCollisionEnabled = true;
    UPROPERTY(BlueprintReadOnly) bool bObstacleDetectionEnabled = false;
    UPROPERTY(BlueprintReadOnly) bool bHasStopConstraint = false;
    UPROPERTY(BlueprintReadOnly) float RemainingStopConstraintCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FName PlannedStopAnchorId = NAME_None;
    UPROPERTY(BlueprintReadOnly) int32 PlannedStopSecond = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly) float DistanceToPlannedStopCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FString BlockingActorName;
    UPROPERTY(BlueprintReadOnly) FString BlockingActorClass;
    UPROPERTY(BlueprintReadOnly) float BlockingActorDistanceCm = -1.0f;
    UPROPERTY(BlueprintReadOnly) FString OccupiedSeats;
    UPROPERTY(BlueprintReadOnly) FString NearbyVehicleIds;
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
    float StuckAfterSeconds = 16.0f;

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

    /** Simulation-time interval for detailed runtime snapshots. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced",
        meta=(ClampMin="1", Units="s"))
    int32 SnapshotIntervalSeconds = 5;

    /** Capture every agent periodically. Disable for very large crowds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced")
    bool bCaptureAllAgentsPeriodically = false;

    /** When false, active, grouped and vehicle-bound agents are still captured. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced")
    bool bCaptureImportantAgentsPeriodically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced")
    bool bCaptureAllVehiclesPeriodically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced")
    bool bCaptureAllAgentsAtShot = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced")
    FTMOPTime ShotSnapshotTime = FTMOPTime(23, 21, 30);

    /** Repeated identical failures are summarized instead of written every second. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced",
        meta=(ClampMin="1", Units="s"))
    int32 FailureRepeatReportIntervalSeconds = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Validation|Advanced",
        meta=(ClampMin="100.0", Units="cm"))
    float NearbyVehicleRadiusCm = 600.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Validation")
    TArray<FTMOPTimelineValidationRecord> Records;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Validation")
    TArray<FTMOPAgentValidationSnapshot> AgentSnapshots;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Validation")
    TArray<FTMOPVehicleValidationSnapshot> VehicleSnapshots;

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
        FName ActiveTargetAnchorId = NAME_None;
        int32 ActivePlannedSecond = INDEX_NONE;
        bool bRegistryManagedMove = false;
        FName LastTimelineEntryId = NAME_None;
        FName LastTimelineAnchorId = NAME_None;
        FName LastFailureEntryId = NAME_None;
        FString LastFailureCode;
        int32 FirstFailureSecond = INDEX_NONE;
        int32 LastFailureRecordSecond = INDEX_NONE;
        int32 FailureAttemptCount = 0;
        bool bMissingReported = false;
    };

    struct FTrackedVehicle
    {
        TWeakObjectPtr<class ATMOPVehicleBase> Vehicle;
        FVector LastLocation = FVector::ZeroVector;
        float StationarySeconds = 0.0f;
        float MaximumStationarySeconds = 0.0f;
        bool bMissingReported = false;
    };

    void DiscoverAgents();
    void DiscoverVehicles();
    void SampleAgents(float DeltaSeconds);
    void SampleVehicles(float DeltaSeconds);
    void CaptureSnapshots(int32 SampleSecond, const FString& Reason,
        bool bForceAllAgents);
    void CaptureAgentSnapshot(FName EntityId, const FTrackedAgent& Tracked,
        int32 SampleSecond, const FString& Reason);
    void CaptureVehicleSnapshot(FName VehicleId, const FTrackedVehicle& Tracked,
        int32 SampleSecond, const FString& Reason);
    bool ShouldCapturePeriodicAgent(const FTrackedAgent& Tracked) const;
    void PopulateRecordRuntimeDiagnostics(
        FTMOPTimelineValidationRecord& Record,
        const FTrackedAgent* Tracked) const;
    void DiagnosePersonFailure(
        const struct FTMOPPersonTimelineEntry& Entry,
        const FTrackedAgent* Tracked,
        FString& OutCode,
        FString& OutDetails) const;
    class ATMOPVehicleBase* FindVehicle(FName VehicleId) const;
    bool FindVehicleSeatForAgent(const class ATMOPVehicleBase* Vehicle,
        const class ATMOPHistoricalAgent* Agent, FName& OutSeatId) const;
    bool FindNextVehicleStop(FName VehicleId, int32 CurrentSecond,
        FName& OutAnchorId, int32& OutStopSecond) const;
    bool FindExpectedShotAnchor(FName EntityId, FName& OutAnchorId) const;
    void AddRecord(const FTMOPTimelineValidationRecord& Record);
    int32 GetSimulationSecond() const;
    FName GetEntityId(const UTMOPActionExecutorComponent* Executor) const;

    void HandleActionValidation(
        UTMOPActionExecutorComponent* Executor,
        const FTMOPScheduleEntry& Entry,
        FTMOPTime ScheduledTime,
        ETMOPActionExecutionState State);

    void HandlePersonTimelineApplied(
        FName EntityId,
        const struct FTMOPPersonTimelineEntry& Entry,
        int32 ResolvedSecond,
        bool bSuccessful,
        bool bCatchUp);

    TMap<FName, FTrackedAgent> TrackedAgents;
    TMap<FName, FTrackedVehicle> TrackedVehicles;
    TWeakObjectPtr<class ATMOPPersonRegistryDirector> PeopleDirector;
    TWeakObjectPtr<class ATMOPGroupDirector> GroupDirector;
    TWeakObjectPtr<class ATMOPHistoricalVehicleDirector> VehicleDirector;
    float SampleAccumulator = 0.0f;
    int32 ValidationStartSecond = INDEX_NONE;
    int32 NextSnapshotSecond = INDEX_NONE;
    bool bShotSnapshotCaptured = false;
    bool bValidationActive = false;
};
