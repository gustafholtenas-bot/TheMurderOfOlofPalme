#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "TMOPRouteTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPRouteExecutionState : uint8
{
    Idle,
    Executing,
    Completed,
    Failed,
    Cancelled
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRouteStep
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FName StepId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FName TargetAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    ETMOPAgentActivityState ActivityState =
        ETMOPAgentActivityState::Walking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    ETMOPRouteSurfacePreference SurfacePreference =
        ETMOPRouteSurfacePreference::SidewalkPreferred;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    bool bHardHistoricalPath = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    float ArrivalRadiusOverride = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FString SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalRouteDefinition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FName RouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    ETMOPMovementPolicy MovementPolicy =
        ETMOPMovementPolicy::NormalPedestrian;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    TArray<FTMOPRouteStep> Steps;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    bool bLoopRoute = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FString SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Route")
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRouteRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Route")
    FName RouteId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Route")
    ETMOPRouteExecutionState State =
        ETMOPRouteExecutionState::Idle;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Route")
    int32 CurrentStepIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category = "TMOP|Route")
    int32 StartedLoopNumber = 0;
};
