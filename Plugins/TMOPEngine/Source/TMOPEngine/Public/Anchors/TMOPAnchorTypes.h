#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "TMOPAnchorTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPAnchorCategory : uint8
{
    Unknown,
    Building,
    BuildingEntrance,
    BuildingExit,
    InteriorPoint,
    ExteriorPoint,
    Cinema,
    CinemaAuditorium,
    CinemaSeat,
    Restaurant,
    Pub,
    Hotel,
    Store,
    MetroEntrance,
    MetroPlatform,
    BusStop,
    TaxiStand,
    StreetCorner,
    Intersection,
    CrosswalkEntry,
    CrosswalkExit,
    TrafficLight,
    Bridge,
    Tunnel,
    StairBottom,
    StairTop,
    PhoneBooth,
    ATM,
    PolicePosition,
    WitnessPosition,
    VehicleSpawn,
    PlayerSpawn,
    ItemSpawn,
    RouteNode
};

UENUM(BlueprintType)
enum class ETMOPPlaceCategory : uint8
{
    Unknown,
    Building,
    Cinema,
    Restaurant,
    Pub,
    Hotel,
    Store,
    MetroStation,
    Street,
    Intersection,
    Bridge,
    Tunnel,
    Stairs,
    Park,
    Cemetery,
    District,
    Other
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPSourceReference
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Source")
    FString SourceId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Source")
    FString SourceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Source")
    FString Description;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Source")
    FString ExternalReference;
};
