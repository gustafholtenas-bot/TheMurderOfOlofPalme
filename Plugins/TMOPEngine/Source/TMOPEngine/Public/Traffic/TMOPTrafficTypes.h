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
