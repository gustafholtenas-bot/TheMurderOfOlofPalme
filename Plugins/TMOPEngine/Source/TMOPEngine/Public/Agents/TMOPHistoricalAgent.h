#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "GameFramework/Character.h"
#include "TMOPHistoricalAgent.generated.h"

class UTMOPActionExecutorComponent;
class UTMOPRouteFollowerComponent;
class UTMOPWorldEntityComponent;
class UTextRenderComponent;
class UWidgetComponent;
class UAudioComponent;
class USoundBase;
class AAIController;

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
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent")
    TObjectPtr<UTMOPWorldEntityComponent> EntityIdentity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent")
    TObjectPtr<UTMOPActionExecutorComponent> ActionExecutor;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent")
    TObjectPtr<UTMOPRouteFollowerComponent> RouteFollower;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Identity")
    FText DisplayName;

    /** World-space name shown above the person. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent|Debug")
    TObjectPtr<UTextRenderComponent> NameLabel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Debug")
    bool bShowNameLabel = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Debug",
        meta = (ClampMin = "0.0", Units = "cm"))
    float NameLabelHeightCm = 125.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Debug",
        meta = (ClampMin = "1.0"))
    float NameLabelWorldSize = 24.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Debug")
    FColor NameLabelColor = FColor::White;

    /** Name-free speech bubble for timed historical quotes. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "TMOP|Agent|Speech")
    TObjectPtr<UWidgetComponent> SpeechBubble;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Speech",
        meta=(ClampMin="80.0", Units="cm"))
    float SpeechBubbleHeightCm = 205.0f;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Speech")
    float ShowAutomaticSpeech(
        const FText& Text, USoundBase* VoiceOver = nullptr,
        float DisplayDurationOverrideSeconds = 0.0f);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Speech")
    void HideAutomaticSpeech();

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Speech")
    bool IsAutomaticSpeechVisible() const
    {
        return AutomaticSpeechSecondsRemaining > 0.0f;
    }

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

    /** Refresh after DisplayName or EntityId changes. */
    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Debug")
    void RefreshNameLabel();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Debug")
    void SetNameLabelVisible(bool bVisible);

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Movement")
    float GetDesiredMovementSpeed() const;

    /** Look at an actor for a limited time. A negative duration keeps focus until cleared. */
    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Social")
    void SetSocialFocus(AActor* Target, float DurationSeconds = -1.0f,
        bool bUseTalkingOverlay = false);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Social")
    void ClearSocialFocus();

    /** Locks focus and talking on the player until the dialogue GUI closes. */
    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Social")
    void BeginDialogueFocus(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Social")
    void EndDialogueFocus();

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Social")
    bool IsDialogueFocused() const { return bDialogueFocusLocked; }

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Social")
    AActor* GetSocialFocusTarget() const { return SocialFocusTarget.Get(); }

    /** Smoothed yaw consumed by ABP_TMOPAgent for neck/spine aim. */
    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Agent|Social")
    float SocialLookYaw = 0.0f;

    /** Smoothed pitch consumed by ABP_TMOPAgent for neck/spine aim. */
    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Agent|Social")
    float SocialLookPitch = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Agent|Social")
    float SocialLookAlpha = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Social",
        meta = (ClampMin = "0.0", ClampMax = "80.0", Units = "deg"))
    float MaximumSocialLookYaw = 55.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Social",
        meta = (ClampMin = "0.0", ClampMax = "45.0", Units = "deg"))
    float MaximumSocialLookPitch = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Social",
        meta = (ClampMin = "0.1"))
    float SocialLookInterpolationSpeed = 4.5f;

    /**
     * Gives immediate visible feedback before the AnimBP neck/spine nodes are wired.
     * Only the skeletal mesh is turned, never the movement capsule.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Social")
    bool bUseSubtleMeshTurnForSocialLook = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Social",
        meta = (ClampMin = "0.0", ClampMax = "25.0", Units = "deg"))
    float MaximumSocialMeshTurnYaw = 12.0f;

    /** Enables automatic recovery when dense pedestrian traffic blocks this agent. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck")
    bool bEnableAutomaticUnstuck = true;

    /** Capsule radius used by historical pedestrians in normal crowds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "20.0", ClampMax = "42.0", Units = "cm"))
    float PedestrianCapsuleRadiusCm = 28.0f;

    /** Repath after being almost stationary for this long. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "0.5", Units = "s"))
    float RepathAfterSeconds = 2.0f;

    /** Try a projected point to either side after this long. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "1.0", Units = "s"))
    float SideStepAfterSeconds = 4.0f;

    /** Temporarily shrink the capsule after this long. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "1.0", Units = "s"))
    float SqueezeAfterSeconds = 7.0f;

    /** Attempt a short, swept NavMesh relocation after this long. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "2.0", Units = "s"))
    float FailsafeAfterSeconds = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "40.0", Units = "cm"))
    float SideStepDistanceCm = 85.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement|Unstuck",
        meta = (ClampMin = "20.0", Units = "cm"))
    float FailsafeAdvanceCm = 100.0f;

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|State")
    bool CanMove() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Agent|Venue")
    bool HasInitialSeatAssignment() const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Agent|Venue")
    bool ApplyInitialSeatAssignment();

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

private:
    void UpdateSocialFocus(float DeltaSeconds);
    void UpdateAutomaticSpeech(float DeltaSeconds);
    void UpdateAutomaticUnstuck(float DeltaSeconds);
    void ResetAutomaticUnstuck(bool bRestoreCapsule);
    void ReissueMove(AAIController* Controller, const FVector& Destination);
    bool TrySideStep(AAIController* Controller, const FVector& Destination);
    bool TryFailsafeAdvance(const FVector& Destination);

    FVector LastUnstuckLocation = FVector::ZeroVector;
    FVector SavedMoveDestination = FVector::ZeroVector;
    float StationarySeconds = 0.0f;
    float SideStepReturnSeconds = 0.0f;
    float OriginalCapsuleRadiusCm = 0.0f;
    bool bUnstuckInitialized = false;
    bool bRepathAttempted = false;
    bool bSideStepAttempted = false;
    bool bSqueezeActive = false;
    bool bFailsafeAttempted = false;
    bool bReturningFromSideStep = false;

    TWeakObjectPtr<AActor> SocialFocusTarget;
    float SocialFocusSecondsRemaining = 0.0f;
    bool bSocialFocusHasNoAutomaticEnd = false;
    bool bSocialFocusUsesTalkingOverlay = false;
    bool bDialogueFocusLocked = false;
    bool bDialogueReturnRotationSaved = false;
    FRotator DialogueReturnRotation = FRotator::ZeroRotator;
    float AutomaticSpeechSecondsRemaining = 0.0f;
    bool bAutomaticSpeechUsesTalkingOverlay = false;
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> ActiveSpeechAudio;
    FQuat BaseMeshRelativeRotation = FQuat::Identity;
};
