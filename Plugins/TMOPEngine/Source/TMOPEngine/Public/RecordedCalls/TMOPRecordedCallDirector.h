#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RecordedCalls/TMOPRecordedCallTypes.h"
#include "TMOPRecordedCallDirector.generated.h"

class UAudioComponent;
class UDataTable;
class UTMOPClockSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FTMOPRecordedCallPlaybackSignature, FName, RecordingId);

/**
 * Plays archival calls as a non-spatial, scenario-time-synchronised monitor.
 * Place one actor in the level and assign DT_TMOP_RecordedCalls.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPRecordedCallDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPRecordedCallDirector();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Recorded Calls")
    TObjectPtr<UDataTable> RecordedCallTable = nullptr;

    /** Starts monitoring the historical clock immediately on BeginPlay. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls")
    bool bAutoStart = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls|Filter")
    bool bPlayLAC = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls|Filter")
    bool bPlayPoliceRadio = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls|Filter")
    bool bPlayOtherTypes = false;

    /** Safety limit for overlapping original tape channels. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls",
        meta=(ClampMin="1", ClampMax="16"))
    int32 MaximumConcurrentRecordings = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls",
        meta=(ClampMin="0.0", ClampMax="2.0"))
    float Volume = 0.85f;

    /** Audio follows clock speed up to this value, then pauses until normal speed returns. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Calls",
        meta=(ClampMin="1.0", ClampMax="4.0"))
    float MaximumAudibleTimeScale = 4.0f;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Recorded Calls|Events")
    FTMOPRecordedCallPlaybackSignature OnRecordingStarted;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Recorded Calls|Events")
    FTMOPRecordedCallPlaybackSignature OnRecordingFinished;

    UFUNCTION(BlueprintCallable, Category="TMOP|Recorded Calls")
    bool ReloadRecordings();

    UFUNCTION(BlueprintCallable, Category="TMOP|Recorded Calls")
    void SetPlaybackEnabled(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="TMOP|Recorded Calls")
    void SynchronizeToClock();

    UFUNCTION(BlueprintPure, Category="TMOP|Recorded Calls")
    bool IsPlaybackEnabled() const { return bPlaybackEnabled; }

    UFUNCTION(BlueprintPure, Category="TMOP|Recorded Calls")
    TArray<FName> GetActiveRecordingIds() const;

    /** Current names/transcript for the HUD. Returns false outside a speech segment. */
    bool GetActiveSubtitle(FName RecordingId, FName& OutSegmentId,
        FText& OutLeftSpeaker, FText& OutRightSpeaker,
        FText& OutTranscript, bool& bOutRadioStyle) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Recorded Calls")
    bool ValidateRecordings(TArray<FString>& OutErrors) const;

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool PassesFilter(const FTMOPRecordedCallRow& Row) const;
    bool IsRowActiveAt(const FTMOPRecordedCallRow& Row, int32 SecondOfDay) const;
    float CalculateAudioOffset(const FTMOPRecordedCallRow& Row, int32 SecondOfDay) const;
    UAudioComponent* AcquireAudioComponent(FName RecordingId);
    void StartRecording(const FTMOPRecordedCallRow& Row, int32 SecondOfDay);
    void StopRecording(FName RecordingId);
    void StopAllRecordings();

    UPROPERTY(Transient)
    TObjectPtr<UTMOPClockSubsystem> Clock = nullptr;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UAudioComponent>> AudioPool;

    UPROPERTY(Transient)
    TArray<FTMOPRecordedCallRow> LoadedRows;

    TMap<FName, int32> ActiveComponentByRecording;
    bool bPlaybackEnabled = false;
    bool bComponentsPaused = false;
    int32 LastClockSecond = INDEX_NONE;
};
