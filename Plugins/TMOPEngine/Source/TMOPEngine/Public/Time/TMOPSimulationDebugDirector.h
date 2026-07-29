#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "People/TMOPPersonBakeTypes.h"
#include "TMOPSimulationDebugDirector.generated.h"

class ATMOPGroupDirector;
class ATMOPHistoricalVehicleDirector;
class ATMOPObservationDirector;
class ATMOPPersonRegistryDirector;
class UTMOPClockSubsystem;

/**
 * Place one instance in the test level.
 *
 * Number keys seek through 23:00-23:40 in five-minute steps.
 * Shift+1 moves 30 seconds backwards; Shift+2 moves 30 seconds forwards.
 * Shift+3 restores 1x speed; every Shift+4 press doubles clock speed.
 * B starts/stops and saves a deterministic World Bake.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPSimulationDebugDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPSimulationDebugDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Debug")
    bool bEnableTimeShortcutKeys = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bLoadExistingBakeOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bApplyBakeAfterTimeJump = true;

    /** Set by the editor Bake button; the next Play session performs the bake. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bBakeOnNextBeginPlay = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake",
        meta=(ClampMin="1", ClampMax="60"))
    int32 BakeSampleIntervalSeconds = 5;

    /** Keep at 1 for the authoritative bake. Higher values are preview-only. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake",
        meta=(ClampMin="1.0", ClampMax="32.0"))
    float BakeTimeScale = 1.0f;

    /** Stored under Saved/TMOP/Bakes. The .json extension is optional. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FString BakeFileName = TEXT("TMOP_World_2300_2345");

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Time")
    bool JumpToSimulationTime(FTMOPTime TargetTime);

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Time")
    void SetSimulationTimeScale(float NewTimeScale);

    /** In editor this arms the next Play; during Play it starts immediately. */
    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|World Bake",
        meta=(DisplayName="Bake Entire Simulation"))
    void BakeEntireSimulation();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|World Bake",
        meta=(DisplayName="Cancel Bake"))
    void CancelWorldBake();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|World Bake",
        meta=(DisplayName="Clear World Bake"))
    void ClearWorldBake();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|World Bake",
        meta=(DisplayName="Validate World Bake"))
    void ValidateWorldBake();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|World Bake",
        meta=(DisplayName="Load World Bake"))
    void LoadWorldBakeFromDisk();

    /** Backward-compatible API; starts the complete World Bake. */
    UFUNCTION(BlueprintCallable, Category="TMOP|World Bake")
    bool StartPersonBakeRecording();

    UFUNCTION(BlueprintCallable, Category="TMOP|World Bake")
    bool StopPersonBakeRecordingAndSave();

    UFUNCTION(BlueprintCallable, Category="TMOP|World Bake")
    bool SavePersonBake() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|World Bake")
    bool LoadPersonBake();

    UFUNCTION(BlueprintCallable, Category="TMOP|World Bake")
    bool ApplyPersonBakeAtTime(FTMOPTime TargetTime);

    UFUNCTION(BlueprintPure, Category="TMOP|World Bake")
    bool IsRecordingPersonBake() const { return bRecordingBake; }

    UFUNCTION(BlueprintPure, Category="TMOP|World Bake")
    FString GetResolvedBakePath() const;

private:
    void CaptureResolvedSharedEvents();
    void CaptureBakeFrame(const FTMOPTime& Time);
    void FinishBakeAfterLoop();
    void RestoreDerivedScheduledSystems();
    void ApplyBakedVehicles(const FTMOPPersonBakeFrame& Frame);
    void ApplyBakedLights(const FTMOPPersonBakeFrame& Frame);
    FString BuildSourceSignature() const;
    bool ValidateLoadedBake(TArray<FString>& OutErrors) const;
    bool SaveWorldBakeAtomically() const;
    const FTMOPPersonBakeFrame* FindNearestBakeFrame(const FTMOPTime& Time) const;
    UTMOPClockSubsystem* GetClock() const;
    ATMOPPersonRegistryDirector* FindPersonDirector() const;
    ATMOPGroupDirector* FindGroupDirector() const;
    ATMOPHistoricalVehicleDirector* FindVehicleDirector() const;
    ATMOPObservationDirector* FindObservationDirector() const;
    void HandleDigit(int32 Digit);

    void DebugKey1();
    void DebugKey2();
    void DebugKey3();
    void DebugKey4();
    void DebugKey5();
    void DebugKey6();
    void DebugKey7();
    void DebugKey8();
    void DebugKey9();
    void DebugBakeKey();

    UPROPERTY(Transient)
    FTMOPWorldBakeData BakeData;

    bool bRecordingBake = false;
    int32 LastRecordedSecond = INDEX_NONE;
};
