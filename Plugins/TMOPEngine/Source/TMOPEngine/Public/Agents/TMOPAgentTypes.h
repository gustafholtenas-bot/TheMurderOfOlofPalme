#pragma once

#include "CoreMinimal.h"
#include "Time/TMOPTime.h"
#include "TMOPAgentTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPAgentLifeState : uint8
{
    Alive,
    Unconscious,
    Restrained,
    Arrested,
    Dead
};

UENUM(BlueprintType)
enum class ETMOPAgentActivityState : uint8
{
    Idle,
    Seated,
    Standing,
    Walking,
    FastWalking,
    Jogging,
    Running,
    Sprinting,
    RidingVehicle,
    Fleeing,
    Interacting
};

UENUM(BlueprintType)
enum class ETMOPMovementPolicy : uint8
{
    NormalPedestrian,
    CautiousPedestrian,
    FastPedestrian,
    HistoricalOverride,
    Emergency,
    Police,
    VehiclePassenger
};

UENUM(BlueprintType)
enum class ETMOPRouteSurfacePreference : uint8
{
    SidewalkPreferred,
    CrosswalkPreferred,
    RoadAllowed,
    RoadRequired,
    StairsAllowed,
    InteriorOnly
};

UENUM(BlueprintType)
enum class ETMOPHistoricalConfidence : uint8
{
    Documented,
    Reconstructed,
    Inferred,
    Speculative,
    FictionalGameplay,
    AmbientFiction
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPMovementProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float SlowWalkSpeed = 90.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float NormalWalkSpeed = 140.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float FastWalkSpeed = 190.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float JogSpeed = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float RunSpeed = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float SprintSpeed = 600.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Movement")
    float PersonalSpeedMultiplier = 1.0f;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPVenueSeatAssignment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    FName VenueId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    FName AuditoriumId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    FName SeatId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    int32 RowNumber = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    int32 SeatNumber = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    FTMOPTime InitialTime = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Venue")
    bool bStartsSeated = true;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalRouteAnchor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    FName AnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    FTMOPTime EarliestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    FTMOPTime PreferredTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    FTMOPTime LatestTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    ETMOPAgentActivityState MovementState = ETMOPAgentActivityState::Walking;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    ETMOPRouteSurfacePreference SurfacePreference =
        ETMOPRouteSurfacePreference::SidewalkPreferred;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    FString SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Agent|Route")
    bool bHardAnchor = false;
};
