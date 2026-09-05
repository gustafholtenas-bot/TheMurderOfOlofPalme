#pragma once

#include "CoreMinimal.h"
#include "TMOPTrafficTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPTrafficLaneType : uint8
{
    General,
    BusAllowed,
    BusOnly,
    EmergencyOnly
};

UENUM(BlueprintType)
enum class ETMOPTrafficTurnType : uint8
{
    Straight,
    Left,
    Right,
    UTurn
};

UENUM(BlueprintType)
enum class ETMOPVehicleRouteMode : uint8
{
    /** Use Ordered Lane IDs exactly as entered. */
    ManualLaneRoute,
    /** Find a connected lane route from the vehicle to Destination Anchor ID. */
    AutomaticToAnchor,
    /** Follow Ordered Lane IDs first, then calculate the rest to the anchor. */
    ManualThenAutomatic,
    /** Move directly between anchors on a smooth authored curve, without lanes. */
    AnchorManeuver UMETA(DisplayName="Anchor Maneuver (No Lanes)")
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPLaneConnection
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic")
    FName TargetLaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic")
    ETMOPTrafficTurnType TurnType = ETMOPTrafficTurnType::Straight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic")
    bool bAllowed = true;
};
