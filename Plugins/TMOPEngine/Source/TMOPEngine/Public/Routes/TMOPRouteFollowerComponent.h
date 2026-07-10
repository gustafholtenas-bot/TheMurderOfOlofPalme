#pragma once

#include "CoreMinimal.h"
#include "Actions/TMOPActionExecutorComponent.h"
#include "Components/ActorComponent.h"
#include "Routes/TMOPRouteTypes.h"
#include "TMOPRouteFollowerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(
    FTMOPRouteStateChangedSignature,
    FName,
    RouteId,
    ETMOPRouteExecutionState,
    State,
    int32,
    CurrentStepIndex);

UCLASS(
    ClassGroup = (TMOP),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPRouteFollowerComponent final
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPRouteFollowerComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Routes")
    FTMOPRouteStateChangedSignature OnRouteStateChanged;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Routes")
    bool StartRouteById(FName RouteId);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Routes")
    bool StartRoute(
        const FTMOPHistoricalRouteDefinition& Route);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Routes")
    void CancelRoute();

    UFUNCTION(BlueprintPure, Category = "TMOP|Routes")
    bool IsFollowingRoute() const
    {
        return Runtime.State == ETMOPRouteExecutionState::Executing;
    }

    UFUNCTION(BlueprintPure, Category = "TMOP|Routes")
    FTMOPRouteRuntime GetRouteRuntime() const
    {
        return Runtime;
    }

    UFUNCTION(BlueprintPure, Category = "TMOP|Routes")
    FTMOPHistoricalRouteDefinition GetCurrentRoute() const
    {
        return CurrentRoute;
    }

private:
    UFUNCTION()
    void HandleActionExecutionChanged(
        FName EntryId,
        ETMOPScheduleActionType ActionType,
        ETMOPActionExecutionState State);

    bool BeginCurrentStep();
    void AdvanceRoute();
    void FinishRoute(ETMOPRouteExecutionState FinalState);
    FName MakeActionEntryId(int32 StepIndex) const;

    UPROPERTY(Transient)
    FTMOPHistoricalRouteDefinition CurrentRoute;

    UPROPERTY(Transient)
    FTMOPRouteRuntime Runtime;

    float DefaultArrivalRadius = 75.0f;
};
