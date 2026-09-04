#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "TMOPGrandFilmDirector.generated.h"

class UMediaPlayer;
class UMediaSoundComponent;
class UMediaSource;
class USceneComponent;
class UTMOPClockSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTMOPGrandFilmTimeSignature, FTMOPTime, EventTime);

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandStandingRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film")
    FName EntityId = NAME_None;

    /** True: film end. False: beginning of credits. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film")
    bool bStandAtFilmEnd = true;
};

/** Plays Grand's film against the scenario clock and tells spectators when to stand. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandFilmDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandFilmDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    /** Media Player also used by the Media Texture on the cinema screen. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback")
    TObjectPtr<UMediaPlayer> FilmMediaPlayer = nullptr;

    /** Usually a File Media Source pointing at Content/Movies. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback")
    TObjectPtr<UMediaSource> FilmMediaSource = nullptr;

    /** Spatial film audio. Move this inherited component to the screen. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Grand Film|Playback")
    TObjectPtr<UMediaSoundComponent> FilmSoundComponent = nullptr;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Grand Film|Playback")
    TObjectPtr<USceneComponent> SceneRoot = nullptr;

    /** Open Film Media Source automatically when play begins. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback")
    bool bAutoOpenFilm = true;

    /**
     * Recommended for a complete film or ending clip. The last frame is
     * aligned with Resolved Film End, including after loop randomisation.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback")
    bool bAlignMediaEndToResolvedFilmEnd = true;

    /** Used when Align Media End is disabled: game time at media time 00:00. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback",
        meta=(EditCondition="!bAlignMediaEndToResolvedFilmEnd"))
    FTMOPTime MediaStartGameTime = FTMOPTime(21, 30, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback",
        meta=(ClampMin="0.0", ClampMax="2.0"))
    float FilmVolume = 1.0f;

    /** Above this clock speed the movie pauses and seeks instead of racing. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback",
        meta=(ClampMin="1.0", ClampMax="8.0"))
    float MaximumRealtimePlaybackScale = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Grand Film|Playback",
        meta=(ClampMin="0.1", ClampMax="10.0", Units="s"))
    float ResyncThresholdSeconds = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film|Uncertainty")
    FTMOPTime EarliestFilmEnd = FTMOPTime(23, 5, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film|Uncertainty")
    FTMOPTime LatestFilmEnd = FTMOPTime(23, 10, 0);

    /** Length of the credits. Credits begin this many seconds before film end. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film|Credits",
        meta=(ClampMin="0", UIMin="0", UIMax="900"))
    int32 CreditsDurationSeconds = 300;

    /** Same seed and loop number always produce the same resolved ending. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film|Uncertainty")
    int32 SimulationSeed = 19860228;

    /** Per-person overrides, matched against the agent's EntityId. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film|People")
    TArray<FTMOPGrandStandingRule> StandingRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Film")
    bool bResolveAgainOnLoopRestart = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Film")
    FTMOPTime ResolvedFilmEnd;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Film")
    FTMOPTime ResolvedCreditsStart;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Grand Film|Events")
    FTMOPGrandFilmTimeSignature OnCreditsStarted;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Grand Film|Events")
    FTMOPGrandFilmTimeSignature OnFilmEnded;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film")
    bool ResolveFilmTimes(int32 LoopNumber);

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film")
    void EvaluateAtTime(FTMOPTime CurrentTime);

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film|Playback")
    bool OpenFilm();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film|Playback")
    void SynchronizeFilmToClock();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Film|Playback")
    void StopFilm();

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    UFUNCTION()
    void HandleMediaOpened(FString OpenedUrl);

    UFUNCTION()
    void HandleMediaOpenFailed(FString FailedUrl);

    void TriggerStandingGroup(bool bAtFilmEnd);
    void ResetBehaviors();
    double ResolveMediaStartSecond() const;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPClockSubsystem> Clock = nullptr;

    bool bCreditsTriggered = false;
    bool bFilmEndTriggered = false;
    bool bFilmMediaOpen = false;
    int32 LastSynchronizedClockSecond = INDEX_NONE;
};
