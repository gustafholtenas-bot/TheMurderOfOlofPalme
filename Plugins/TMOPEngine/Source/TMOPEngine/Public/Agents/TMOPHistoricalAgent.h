#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "GameFramework/Character.h"
#include "TMOPHistoricalAgent.generated.h"

class UTMOPActionExecutorComponent;
class UTMOPWorldEntityComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPAgentStateChangedSignature,
    ETMOPAgentLifeState,
    OldState,
    ETMOPAgentLifeState,
    NewState);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPAgentActivityChangedSignature,
    ETMOPAgentActivityState,
    OldActivity,
    ETMOPAgentActivityState,
    NewActivity);

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPHistoricalAgent : public ACharacter
{
    GENERATED_BODY()

public:
    ATMOPHistoricalAgent();

    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent")
    TObjectPtr<UTMOPWorldEntityComponent> EntityIdentity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent")
    TObjectPtr<UTMOPActionExecutorComponent> ActionExecutor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Identity")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Identity")
    ETMOPHistoricalConfidence IdentityConfidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|State")
    ETMOPAgentLifeState LifeState = ETMOPAgentLifeState::Alive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|State")
    ETMOPAgentActivityState ActivityState =
        ETMOPAgentActivityState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    ETMOPMovementPolicy MovementPolicy =
        ETMOPMovementPolicy::NormalPedestrian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    FTMOPMovementProfile MovementProfile;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    FTMOPVenueSeatAssignment InitialSeatAssignment;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    TArray<FTMOPHistoricalRouteAnchor> InitialRouteAnchors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Group")
    FName SocialGroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Group")
    TArray<FName> KnownCompanionIds;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Agent|Events")
    FTMOPAgentStateChangedSignature OnLifeStateChanged;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Agent|Events")
    FTMOPAgentActivityChangedSignature OnActivityStateChanged;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|State")
    bool SetLifeState(ETMOPAgentLifeState NewState);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|State")
    bool SetActivityState(ETMOPAgentActivityState NewActivity);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Movement")
    void ApplyMovementSpeedForActivity();

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Movement")
    float GetDesiredMovementSpeed() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|State")
    bool CanMove() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Venue")
    bool HasInitialSeatAssignment() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Route")
    int32 GetHistoricalAnchorCount() const
    {
        return InitialRouteAnchors.Num();
    }

protected:
    virtual void HandleLifeStateChanged(
        ETMOPAgentLifeState OldState,
        ETMOPAgentLifeState NewState);

    virtual void HandleActivityStateChanged(
        ETMOPAgentActivityState OldActivity,
        ETMOPAgentActivityState NewActivity);
};
