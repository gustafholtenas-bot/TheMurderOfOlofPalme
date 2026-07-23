#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Vehicles/TMOPVehicleCatalogTypes.h"
#include "TMOPHistoricalVehicleTypes.generated.h"

class AActor;
class UTMOPVehicleModelData;

UENUM(BlueprintType)
enum class ETMOPHistoricalVehicleAction : uint8
{
    InitialPlacement,
    Spawn,
    Despawn,
    BeginDriving,
    Stop,
    Park,
    EnterTrafficRoute,
    ExitTrafficRoute,
    Custom
};

/** One source-backed state or action in a historical vehicle's schedule. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalVehicleTimelineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    FName EntryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    ETMOPHistoricalVehicleAction Action = ETMOPHistoricalVehicleAction::InitialPlacement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    FTransform WorldTransform = FTransform::Identity;

    /** Lane route used after this entry, when one has been reconstructed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    TArray<FName> OrderedLaneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Occupants")
    FName DriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Occupants")
    TArray<FName> PassengerEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Reconstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString Notes;
};

/** One row in DT_TMOP_HistoricalVehicles. Row Name should equal VehicleId. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalVehicleRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    FName CategoryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    ETMOPVehicleCategory VehicleCategory = ETMOPVehicleCategory::PassengerCar;

    /** Optional known model. Leave empty when the source only says "car". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Model")
    TObjectPtr<UTMOPVehicleModelData> ModelData;

    /** Empty uses the future historical vehicle director's default vehicle class. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Simulation")
    TSubclassOf<AActor> VehicleClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|People")
    TArray<FName> AssociatedPersonEntityIds;

    /** Owner or main source-backed person. This does not automatically mean driver. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|People")
    FName PrimaryPersonEntityId = NAME_None;

    /** Only fill this when the driver is actually supported by the source. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|People")
    FName KnownDriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Simulation")
    bool bSpawnInSimulation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Simulation")
    TArray<FTMOPHistoricalVehicleTimelineEntry> Timeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Reconstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString Notes;
};
