#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPGrandFoyerDirector.generated.h"

class ATMOPHistoricalAgent;
class UTMOPGrandFoyerPointComponent;

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandFoyerRule
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    FName EntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    bool bWaitInFoyer = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer",
        meta=(EditCondition="bWaitInFoyer"))
    FName WaitingPointId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer",
        meta=(ClampMin="0.0", EditCondition="bWaitInFoyer"))
    float WaitDurationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    FName BuildingExitId = NAME_None;
};

UENUM(BlueprintType)
enum class ETMOPGrandFoyerState : uint8
{
    MovingToWaitingPoint,
    Waiting,
    QueuedForBuildingExit,
    MovingToBuildingExit,
    LeftGrand,
    Failed
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandFoyerStatus
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Foyer")
    TObjectPtr<ATMOPHistoricalAgent> Agent;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Foyer")
    FName WaitingPointId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Foyer")
    FName BuildingExitId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Foyer")
    ETMOPGrandFoyerState State = ETMOPGrandFoyerState::QueuedForBuildingExit;

    float RemainingWaitSeconds = 0.0f;
};

/** Moves agents from the auditorium through the foyer and out of Grand. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandFoyerDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandFoyerDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    TArray<FTMOPGrandFoyerRule> PersonRules;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    FName DefaultBuildingExitId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer",
        meta=(ClampMin="1", ClampMax="10"))
    int32 MaximumSimultaneousPerBuildingExit = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer",
        meta=(ClampMin="0.05"))
    float ScanIntervalSeconds = 0.20f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Foyer")
    int32 DiscoverFoyerPoints();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Foyer")
    bool ValidateFoyer(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Foyer")
    void ResetFoyer();

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Foyer")
    int32 GetLeftGrandCount() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Foyer")
    TArray<FTMOPGrandFoyerStatus> GetFoyerStatuses() const { return Statuses; }

private:
    void ImportAgentsFromAuditorium();
    void UpdateStatuses(float DeltaSeconds);
    void StartExitQueues();
    const FTMOPGrandFoyerRule* FindRule(FName EntityId) const;
    UTMOPGrandFoyerPointComponent* FindPoint(FName PointId) const;
    UTMOPGrandFoyerPointComponent* ChooseBuildingExit(const ATMOPHistoricalAgent* Agent,
        FName PreferredId) const;
    bool MoveAgentToPoint(ATMOPHistoricalAgent* Agent, UTMOPGrandFoyerPointComponent* Point);
    int32 MovingToExitCount(FName ExitId) const;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UTMOPGrandFoyerPointComponent>> Points;

    UPROPERTY(Transient)
    TArray<FTMOPGrandFoyerStatus> Statuses;

    float ScanAccumulator = 0.0f;
};
