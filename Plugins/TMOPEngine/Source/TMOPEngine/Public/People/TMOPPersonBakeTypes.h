#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPPersonBakeTypes.generated.h"

/** One person's resolved runtime state at a sampled simulation time. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBakedPersonState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FName EntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FTransform WorldTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    ETMOPAgentActivityState ActivityState = ETMOPAgentActivityState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    ETMOPAgentLifeState LifeState = ETMOPAgentLifeState::Alive;

    /** Individual navigation target. Group movement is saved separately. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    bool bHasMoveTarget = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FVector MoveTarget = FVector::ZeroVector;
};

/** A moving group's shared destination, required to resume formations after a seek. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBakedGroupState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FName GroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    ETMOPGroupState State = ETMOPGroupState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    float AcceptanceRadius = 100.0f;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonBakeFrame
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FTMOPTime Time;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    TArray<FTMOPBakedPersonState> People;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    TArray<FTMOPBakedGroupState> Groups;
};

/** JSON-serializable result from one standard simulation pass. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonBakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    int32 FormatVersion = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FTMOPTime ScenarioStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FTMOPTime ScenarioEndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    int32 SampleIntervalSeconds = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    TArray<FTMOPPersonBakeFrame> Frames;
};
