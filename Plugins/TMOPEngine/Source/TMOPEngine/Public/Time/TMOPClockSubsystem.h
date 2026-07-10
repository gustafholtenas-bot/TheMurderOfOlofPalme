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

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPClockSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UTMOPClockSubsystem();

    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Clock")
    FTMOPSecondChangedSignature OnSecondChanged;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Clock")
    FTMOPLoopRestartedSignature OnLoopRestarted;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    FTMOPTime GetCurrentTime() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    FTMOPTime GetLoopStartTime() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    FTMOPTime GetLoopEndTime() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    int32 GetLoopNumber() const { return LoopNumber; }

    UFUNCTION(BlueprintPure, Category = "TMOP|Clock")
    bool IsClockRunning() const { return bClockRunning; }

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
    bool TickClock(float DeltaSeconds);
    void AdvanceOneSecond();

    FDelegateHandle TickerHandle;

    int32 CurrentTimeSeconds = 0;
    int32 LoopStartSeconds = 0;
    int32 LoopEndSeconds = 0;
    int32 LoopNumber = 1;

    double FractionalSeconds = 0.0;
    float TimeScale = 1.0f;
    bool bClockRunning = true;
};
