#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "Observations/TMOPObservationTypes.h"
#include "Time/TMOPTime.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "TMOPPersonBakeTypes.generated.h"

/** Resolved Shared Event time/state used by every relative timeline. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBakedEventState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FTMOPHistoricalEventRuntime Runtime;
};

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FName> MemberEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName LeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    ETMOPGroupState State = ETMOPGroupState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    ETMOPGroupFormation Formation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    float RemainingConversationSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bConversationHasNoAutomaticEnd = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    FVector TargetLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bake")
    float AcceptanceRadius = 100.0f;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBakedVehicleOccupant
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName SeatId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName PersonEntityId = NAME_None;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBakedVehicleState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FTransform WorldTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FVector LinearVelocity = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bHasTrafficMovement = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName CurrentLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    float DistanceAlongLaneCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    float SpeedCmPerSecond = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    ETMOPTrafficVehicleState TrafficState =
        ETMOPTrafficVehicleState::Uninitialized;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bDrivingEnabled = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FName> PlannedLaneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FTMOPBakedVehicleOccupant> Occupants;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBakedLightState
{
    GENERATED_BODY()

    /** Stable owner/component names, without PIE package prefixes. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName OwnerActorName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FName ComponentName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    bool bVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    float Intensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FLinearColor LightColor = FLinearColor::White;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FTMOPBakedVehicleState> Vehicles;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FTMOPObservationRuntime> Observations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FTMOPBakedLightState> Lights;
};

/** JSON-serializable deterministic result from one complete simulation pass. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPWorldBakeData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    int32 FormatVersion = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FString LevelPackageName;

    /** Changes when the configured source tables/schedules change. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FString SourceSignature;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FString CreatedUtc;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FTMOPTime ScenarioStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    FTMOPTime ScenarioEndTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    int32 SampleIntervalSeconds = 5;

    /** Captured before movement so every relative system uses the same times. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FTMOPBakedEventState> SharedEvents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Bake")
    TArray<FTMOPPersonBakeFrame> Frames;
};
