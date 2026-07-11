#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Groups/TMOPGroupTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPGrandSimulationDirector.generated.h"

UENUM(BlueprintType)
enum class ETMOPGrandTimelineActionType : uint8
{
    CreateGroup,
    MergeGroups,
    StartConversation,
    EndConversation,
    MoveGroup,
    SplitGroup,
    StopGroup
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandTimelineAction
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline")
    FName ActionId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline")
    ETMOPGrandTimelineActionType ActionType = ETMOPGrandTimelineActionType::CreateGroup;

    /** If false, EarliestTime and LatestTime resolve an uncertain execution time. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Time")
    bool bUseExactTime = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Time")
    FTMOPTime ExactTime = FTMOPTime(23, 10, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Time",
        meta=(EditCondition="!bUseExactTime"))
    FTMOPTime EarliestTime = FTMOPTime(23, 10, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Time",
        meta=(EditCondition="!bUseExactTime"))
    FTMOPTime LatestTime = FTMOPTime(23, 15, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group")
    FName GroupId = NAME_None;

    /** Used by CreateGroup. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group")
    FTMOPGroupDefinition GroupDefinition;

    /** Used by MergeGroups. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group")
    TArray<FName> SourceGroupIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group")
    FName LeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group")
    ETMOPGroupFormation Formation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group",
        meta=(ClampMin="30.0"))
    float FormationSpacing = 110.0f;

    /** Used by SplitGroup. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Group")
    TArray<FTMOPGroupDefinition> SplitDefinitions;

    /** Used by StartConversation. Negative maximum means no automatic end. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Conversation")
    float MinimumConversationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Conversation")
    float MaximumConversationSeconds = -1.0f;

    /** Used by MoveGroup. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Movement")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Timeline|Movement",
        meta=(ClampMin="20.0"))
    float AcceptanceRadius = 100.0f;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPGrandResolvedTimelineAction
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Timeline")
    FName ActionId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Timeline")
    FTMOPTime ResolvedTime;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Grand Timeline")
    bool bExecuted = false;

    int32 SourceIndex = INDEX_NONE;
};

/** Owns Grand's clock-driven lifecycle across all auditoriums. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPGrandSimulationDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPGrandSimulationDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Simulation")
    int32 SimulationSeed = 19860228;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Simulation")
    bool bResetWhenTimeMovesBackwards = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Simulation")
    TArray<FTMOPGrandTimelineAction> TimelineActions;

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Simulation")
    void ResetFullGrandSimulation();

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Simulation")
    bool ResolveTimeline(int32 LoopNumber);

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Simulation")
    void EvaluateTimeline(FTMOPTime CurrentTime);

    UFUNCTION(BlueprintPure, Category="TMOP|Grand Simulation")
    TArray<FTMOPGrandResolvedTimelineAction> GetResolvedTimeline() const { return ResolvedActions; }

    UFUNCTION(BlueprintCallable, Category="TMOP|Grand Simulation")
    bool ValidateTimeline(TArray<FString>& OutErrors) const;

private:
    UFUNCTION()
    void InitializeSimulation();

    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool ExecuteAction(const FTMOPGrandTimelineAction& Action, int32 ActionIndex);

    UPROPERTY(Transient)
    TArray<FTMOPGrandResolvedTimelineAction> ResolvedActions;

    int32 CurrentLoopNumber = 1;
    int32 LastEvaluatedSeconds = INDEX_NONE;
    bool bIsResetting = false;
};
