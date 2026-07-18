#pragma once

#include "CoreMinimal.h"
#include "Agents/TMOPAgentTypes.h"
#include "Engine/DataAsset.h"
#include "TMOPBusPassengerManifest.generated.h"

UENUM(BlueprintType)
enum class ETMOPBusPassengerPlacement : uint8
{
    FirstAvailableSeat,
    AssignedSeat,
    Standing
};

/** One source-backed journey. No runtime passenger is ever generated from this record. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPBusPassengerJourney
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger")
    FName PassengerEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger")
    FName BoardingStopId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger")
    FName AlightingStopId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger")
    ETMOPBusPassengerPlacement Placement = ETMOPBusPassengerPlacement::FirstAvailableSeat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger",
        meta=(EditCondition="Placement==ETMOPBusPassengerPlacement::AssignedSeat"))
    FName AssignedSeatId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passenger|Source")
    FString Notes;
};

/** Explicit, deterministic passenger list for one scheduled bus run. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPBusPassengerManifest : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Manifest")
    FName ManifestId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Manifest")
    FName DriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Bus Manifest")
    TArray<FTMOPBusPassengerJourney> Journeys;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Manifest")
    bool ValidateManifest(TArray<FString>& OutErrors) const;
};
