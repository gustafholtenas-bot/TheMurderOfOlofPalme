#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "People/TMOPPersonBakeTypes.h"
#include "TMOPSimulationDebugDirector.generated.h"

class ATMOPGroupDirector;
class ATMOPPersonRegistryDirector;
class UTMOPClockSubsystem;

/**
 * Place one instance in the test level.
 *
 * Number keys seek through 23:00-23:40 in five-minute steps.
 * Shift+1 through Shift+5 seek to 23:41-23:45.
 * B starts/stops a standard-speed person bake.
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Debug|Bake")
    bool bLoadExistingBakeOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Debug|Bake")
    bool bApplyBakeAfterTimeJump = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Debug|Bake",
        meta=(ClampMin="1", ClampMax="60"))
    int32 BakeSampleIntervalSeconds = 5;

    /** Stored under Saved/TMOP/Bakes. The .json extension is optional. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Debug|Bake")
    FString BakeFileName = TEXT("TMOP_People_2300_2345");

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Time")
    bool JumpToSimulationTime(FTMOPTime TargetTime);

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Time")
    void SetSimulationTimeScale(float NewTimeScale);

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Bake")
    bool StartPersonBakeRecording();

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Bake")
    bool StopPersonBakeRecordingAndSave();

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Bake")
    bool SavePersonBake() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Bake")
    bool LoadPersonBake();

    UFUNCTION(BlueprintCallable, Category="TMOP|Debug|Bake")
    bool ApplyPersonBakeAtTime(FTMOPTime TargetTime);

    UFUNCTION(BlueprintPure, Category="TMOP|Debug|Bake")
    bool IsRecordingPersonBake() const { return bRecordingBake; }

    UFUNCTION(BlueprintPure, Category="TMOP|Debug|Bake")
    FString GetResolvedBakePath() const;

private:
    void CaptureBakeFrame(const FTMOPTime& Time);
    void FinishBakeAfterLoop();
    const FTMOPPersonBakeFrame* FindNearestBakeFrame(const FTMOPTime& Time) const;
    UTMOPClockSubsystem* GetClock() const;
    ATMOPPersonRegistryDirector* FindPersonDirector() const;
    ATMOPGroupDirector* FindGroupDirector() const;
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
    FTMOPPersonBakeData BakeData;

    bool bRecordingBake = false;
    int32 LastRecordedSecond = INDEX_NONE;
};
