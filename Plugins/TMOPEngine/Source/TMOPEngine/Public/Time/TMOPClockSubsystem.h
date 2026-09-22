#pragma once

#include "CoreMinimal.h"
#include "Containers/Ticker.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Time/TMOPTime.h"
#include "TMOPClockSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPSecondChangedSignature,
    FTMOPTime,
    NewTime);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPLoopRestartedSignature,
    int32,
    NewLoopNumber,
    FTMOPTime,
    RestartTime);

/** Fired once when the scenario reaches its configured end time. */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPLoopEndedSignature,
    int32,
    FinishedLoopNumber,
    FTMOPTime,
    EndTime);

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPClockSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UTMOPClockSubsystem();

    // Set only by the historical playback owner. Ordinary clock setters route
    // through its transaction; listeners see a completed world, never half a seek.
    bool bAuthoritativePlayback = false;
    bool bRecordingAuthoritativeBake = false;
    void CommitHistoricalTime(FTMOPTime Time);
    void PublishHistoricalTime();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Clock")
    FTMOPSecondChangedSignature OnSecondChanged;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Clock")
    FTMOPLoopRestartedSignature OnLoopRestarted;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Clock")
    FTMOPLoopEndedSignature OnLoopEnded;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    FTMOPTime GetCurrentTime() const;

    /** Simulation time including the fractional second between clock ticks. */
    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    double GetCurrentTimeSecondsExact() const
    {
        return static_cast<double>(CurrentTimeSeconds) + FractionalSeconds;
    }

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    FTMOPTime GetLoopStartTime() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    FTMOPTime GetLoopEndTime() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    int32 GetLoopNumber() const { return LoopNumber; }

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    bool IsClockRunning() const;
    /** Run intent before temporary menu/world pause gates, for time reconstruction. */
    bool IsClockRunRequested() const { return bClockRunning; }

    /** Idempotent, owner-scoped pause: releasing one menu cannot resume another. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Clock|Pause")
    void RequestPause(UObject* Owner, FName Reason, bool bPauseWorld = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Clock|Pause")
    void ReleasePause(UObject* Owner, FName Reason);

    UFUNCTION(BlueprintCallable, Category="TMOP|Clock|Pause")
    void ReleaseAllPauses(UObject* Owner);

    UFUNCTION(BlueprintPure, Category="TMOP|Clock|Pause")
    bool HasPauseRequests() const;

    /** True after 23:45 until the player chooses how to continue. */
    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    bool IsAwaitingLoopDecision() const { return bAwaitingLoopDecision; }

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    float GetTimeScale() const { return TimeScale; }

    UFUNCTION(BlueprintCallable, Category = "TMOP|Clock")
    void StartClock();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Clock")
    void PauseClock();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Clock")
    void RestartLoop();

    UFUNCTION(BlueprintCallable, Category = "TMOP|Clock")
    void SetCurrentTime(FTMOPTime NewTime);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Clock")
    bool SetLoopRange(FTMOPTime NewStartTime, FTMOPTime NewEndTime);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Clock")
    void SetTimeScale(float NewTimeScale);

private:
    struct FPauseRequest
    {
        TWeakObjectPtr<UObject> Owner;
        FName Reason;
        bool bPauseWorld = true;
    };
    TArray<FPauseRequest> PauseRequests;
    TWeakObjectPtr<UWorld> PausedWorld;
    bool bOwnsWorldPause = false;
    void UpdatePauseState();
    bool TickClock(float DeltaSeconds);
    void AdvanceOneSecond();
    void ReachLoopEnd();

    FTSTicker::FDelegateHandle TickerHandle;

    int32 CurrentTimeSeconds = 0;
    int32 LoopStartSeconds = 0;
    int32 LoopEndSeconds = 0;
    int32 LoopNumber = 1;

    double FractionalSeconds = 0.0;
    float TimeScale = 1.0f;
    bool bClockRunning = true;
    bool bAwaitingLoopDecision = false;
    bool bRestartInProgress = false;
    int32 LastPublishedHistoricalSecond = INDEX_NONE;
};
