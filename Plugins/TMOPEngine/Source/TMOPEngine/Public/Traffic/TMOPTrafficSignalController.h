#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Traffic/TMOPTrafficSignalTypes.h"
#include "TMOPTrafficSignalController.generated.h"
class UTMOPTrafficSignalComponent;

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTrafficSignalController : public AActor
{
    GENERATED_BODY()
public:
    ATMOPTrafficSignalController();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    FName IntersectionId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    TArray<FTMOPTrafficSignalGroup> Groups;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    TArray<FTMOPTrafficSignalConflict> Conflicts;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    TArray<FTMOPTrafficSignalPhase> Phases;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bCycleAutomatically = true;
    /** Re-evaluate from the simulation clock, including historical playback. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bVisualClockOnly = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    int32 InitialPhaseIndex = 0;
    /** Seconds since midnight. Shared epoch permits green-wave offsets. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    double ProgramEpochSeconds = 82800.0;
    /** Positive values advance this intersection in its cycle. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    double CycleOffsetSeconds = 0.0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Builder")
    TArray<FTMOPTrafficSignalStage> Stages;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Builder")
    bool bStockholm1986 = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Builder", meta=(ClampMin="0.1"))
    float AmberSeconds = 3.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Builder", meta=(ClampMin="0.1"))
    float RedAmberSeconds = 1.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic Signal")
    int32 CurrentPhaseIndex = INDEX_NONE;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic Signal")
    double PhaseEndSecond = 0.0;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic Signal")
    bool bProgramValid = false;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Traffic Signal|Builder")
    void BuildProtectedProgram();
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool SetPhase(int32 NewPhaseIndex);
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    void ForceGroupState(FName SignalGroupId, ETMOPTrafficSignalState NewState);
    UFUNCTION(BlueprintPure, Category="TMOP|Traffic Signal")
    ETMOPTrafficSignalState GetGroupState(FName SignalGroupId, bool& bFound) const;
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool ValidateController(TArray<FString>& OutErrors) const;
    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Traffic Signal")
    void RefreshSignalHeads();
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    void EvaluateAtTime(double Time);
    UFUNCTION(BlueprintPure, Category="TMOP|Traffic Signal")
    float GetRemainingSeconds(double Time) const;
    bool HasGroup(FName Id) const;
    bool IsPedestrianGroup(FName Id) const;
    FString ConfigurationSignature() const;
    TArray<FTMOPSignalGroupState> CapturePlaybackSignals() const;
    void RestorePlaybackSignals(const TArray<FTMOPSignalGroupState>& States);
    void RestorePlaybackPhase(int32 Index, double EndSecond);
private:
    void ApplyCurrentPhase();
    void PublishSignals();
    void SetAllRed();
    TMap<FName, ETMOPTrafficSignalState> RuntimeStates;
    TArray<TWeakObjectPtr<UTMOPTrafficSignalComponent>> SignalHeads;
};
