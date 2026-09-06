#pragma once
#include "CoreMinimal.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "TMOPVehicleEditorObjects.generated.h"

UCLASS(Transient)
class UTMOPVehicleDetailsObject : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="TMOP", meta=(ShowOnlyInnerProperties))
    FTMOPHistoricalVehicleRow Data;
};

UCLASS(Transient)
class UTMOPVehicleEntryDetailsObject : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="TMOP", meta=(ShowOnlyInnerProperties))
    FTMOPHistoricalVehicleTimelineEntry Data;
};

UCLASS(Transient)
class UTMOPVehicleAccessoryDetailsObject : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="TMOP", meta=(ShowOnlyInnerProperties))
    FTMOPVehicleAccessoryVisual Data;
};

UCLASS(Transient)
class UTMOPVehicleRoofDetailsObject : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category="TMOP", meta=(ShowOnlyInnerProperties))
    FTMOPRoofAccessoryVisual Data;
};

