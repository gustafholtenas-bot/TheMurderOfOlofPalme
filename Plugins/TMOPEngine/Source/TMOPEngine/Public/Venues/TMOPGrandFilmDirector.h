#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "TMOPGrandFilmDirector.generated.h"

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

/** Resolves the uncertain film ending and tells spectators when to stand. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandFilmDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandFilmDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

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

private:
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    void TriggerStandingGroup(bool bAtFilmEnd);
    void ResetBehaviors();

    bool bCreditsTriggered = false;
    bool bFilmEndTriggered = false;
};
