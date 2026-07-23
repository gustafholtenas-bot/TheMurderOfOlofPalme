#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Schedules/TMOPScheduleTypes.h"
#include "TMOPActionExecutorComponent.generated.h"

class ATMOPHistoricalAgent;

UENUM(BlueprintType)
enum class ETMOPActionExecutionState : uint8
{
    Idle,
    Executing,
    WaitingForArrival,
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

    UFUNCTION(BlueprintPure, Category = "TMOP|Actions")
    ETMOPActionExecutionState GetExecutionState() const
    {
        return ExecutionState;
    }

    UFUNCTION(BlueprintPure, Category = "TMOP|Actions")
    bool IsExecutingAction() const
    {
        return ExecutionState == ETMOPActionExecutionState::Executing ||
            ExecutionState == ETMOPActionExecutionState::WaitingForArrival;
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

private:
    UFUNCTION()
    void HandleScheduleEntryReady(
        FName AgentId,
        FTMOPScheduleEntry Entry,
        FTMOPTime TriggerTime);

    bool ExecuteImmediateAction(const FTMOPScheduleEntry& Entry);
    bool BeginMoveToAnchor(const FTMOPScheduleEntry& Entry);
    void CompleteCurrentAction(bool bSuccessful);
    FName GetOwnerEntityId() const;
    ATMOPHistoricalAgent* GetHistoricalAgent() const;

    UPROPERTY(Transient)
    FTMOPScheduleEntry CurrentEntry;

    UPROPERTY(Transient)
    FVector CurrentTargetLocation = FVector::ZeroVector;

    ETMOPActionExecutionState ExecutionState =
        ETMOPActionExecutionState::Idle;

    bool bHasCurrentEntry = false;
    bool bRestoredFromBake = false;
};
