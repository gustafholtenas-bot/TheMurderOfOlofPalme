#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Schedules/TMOPScheduleTypes.h"
#include "TMOPActionExecutorComponent.generated.h"

class ATMOPHistoricalAgent;
class UTMOPActionExecutorComponent;
class ATMOPVerticalTransport;

UENUM(BlueprintType)
enum class ETMOPActionExecutionState : uint8
{
    Idle,
    Executing,
    WaitingForArrival,
    WaitingForVerticalTransport,
    Completed,
    Failed
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTMOPActionExecutionSignature,
    FName,
    EntryId,
    ETMOPScheduleActionType,
    ActionType,
    ETMOPActionExecutionState,
    State);

DECLARE_MULTICAST_DELEGATE_FourParams(
    FTMOPActionValidationNativeSignature,
    UTMOPActionExecutorComponent*,
    const FTMOPScheduleEntry&,
    FTMOPTime,
    ETMOPActionExecutionState);

/**
 * Converts ready schedule entries into concrete agent behavior.
 *
 * Sprint 8 supports:
 * - setting agent activity
 * - standing and sitting state
 * - waiting at an anchor
 * - moving to a historical anchor using Unreal navigation
 * - basic custom/interact events for Blueprint extension
 */
UCLASS(
    ClassGroup = (TMOP),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPActionExecutorComponent final : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPActionExecutorComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(
        float DeltaTime,
        ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Actions")
    float ArrivalRadius = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Actions")
    bool bAutoExecuteScheduleEntries = true;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Actions")
    FTMOPActionExecutionSignature OnActionExecutionChanged;

    /** Native telemetry consumed by the timeline validation director. */
    FTMOPActionValidationNativeSignature OnActionValidationEvent;

    UFUNCTION(BlueprintPure, Category = "TMOP|Actions")
    ETMOPActionExecutionState GetExecutionState() const
    {
        return ExecutionState;
    }

    UFUNCTION(BlueprintPure, Category = "TMOP|Actions")
    bool IsExecutingAction() const
    {
        return ExecutionState == ETMOPActionExecutionState::Executing ||
            ExecutionState == ETMOPActionExecutionState::WaitingForArrival ||
            ExecutionState == ETMOPActionExecutionState::WaitingForVerticalTransport;
    }

    UFUNCTION(BlueprintCallable, Category = "TMOP|Actions")
    bool ExecuteScheduleEntry(const FTMOPScheduleEntry& Entry);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Actions")
    void CancelCurrentAction();

    /** Returns the currently active individual navigation target for bake recording. */
    UFUNCTION(BlueprintPure, Category = "TMOP|Actions|Bake")
    bool TryGetActiveMoveTarget(FVector& OutTargetLocation) const;

    /** Resumes an in-progress individual walk restored from a baked frame. */
    UFUNCTION(BlueprintCallable, Category = "TMOP|Actions|Bake")
    bool RestoreBakedMoveToLocation(
        FVector TargetLocation,
        ETMOPAgentActivityState RestoredActivity);

    /**
     * Supplies the historical arrival deadline for the next MoveToAnchor.
     * Speeds are centimetres/second and are clamped to a realistic range.
     * No teleport is used when the deadline cannot be reached.
     */
    void ConfigureNextTimedMove(
        int32 ExpectedArrivalSecond,
        float MinimumSpeedCmPerSecond,
        float MaximumSpeedCmPerSecond);

    /** Current timing diagnostics consumed by timeline validation. */
    bool GetActiveMoveTimingDiagnostics(
        int32& OutExpectedArrivalSecond,
        float& OutRemainingPathCm,
        float& OutRequiredSpeedCmPerSecond,
        bool& bOutPhysicallyPossible) const;

private:
    UFUNCTION()
    void HandleScheduleEntryReady(
        FName AgentId,
        FTMOPScheduleEntry Entry,
        FTMOPTime TriggerTime);

    bool ExecuteImmediateAction(const FTMOPScheduleEntry& Entry);
    bool BeginMoveToAnchor(const FTMOPScheduleEntry& Entry);
    bool MoveToCurrentRouteAnchor();
    void UpdateTimedMovementSpeed(bool bForceUpdate = false);
    float CalculateRemainingPathLengthCm() const;
    void RestoreMovementSpeed();
    void QueueScheduleEntry(
        const FTMOPScheduleEntry& Entry,
        const FTMOPTime& TriggerTime);
    void ExecuteNextQueuedEntry();
    void CompleteCurrentAction(bool bSuccessful);
    void BroadcastExecutionState(ETMOPActionExecutionState State);
    FName GetOwnerEntityId() const;
    ATMOPHistoricalAgent* GetHistoricalAgent() const;

    UPROPERTY(Transient)
    FTMOPScheduleEntry CurrentEntry;

    UPROPERTY(Transient)
    FVector CurrentTargetLocation = FVector::ZeroVector;

    UPROPERTY(Transient)
    TArray<FName> CurrentRouteAnchorIds;

    int32 CurrentRouteAnchorIndex = INDEX_NONE;

    ETMOPActionExecutionState ExecutionState =
        ETMOPActionExecutionState::Idle;

    bool bHasCurrentEntry = false;
    FTMOPTime CurrentScheduledTime;
    bool bRestoredFromBake = false;

    int32 PendingExpectedArrivalSecond = INDEX_NONE;
    float PendingMinimumSpeedCmPerSecond = 0.0f;
    float PendingMaximumSpeedCmPerSecond = 0.0f;
    int32 ActiveExpectedArrivalSecond = INDEX_NONE;
    float ActiveMinimumSpeedCmPerSecond = 0.0f;
    float ActiveMaximumSpeedCmPerSecond = 0.0f;
    float ActiveRemainingPathCm = 0.0f;
    float ActiveRequiredSpeedCmPerSecond = 0.0f;
    bool bActiveMovePhysicallyPossible = true;
    float TimedSpeedUpdateAccumulator = 0.0f;

    /** Entries whose scheduled time passed while an earlier action was active. */
    UPROPERTY(Transient)
    TArray<FTMOPScheduleEntry> QueuedEntries;

    /** Original trigger times, kept parallel with QueuedEntries for validation. */
    UPROPERTY(Transient)
    TArray<FTMOPTime> QueuedTriggerTimes;

    UPROPERTY(Transient)
    TObjectPtr<ATMOPVerticalTransport> ActiveVerticalTransport;
};
