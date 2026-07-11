#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Vehicles/TMOPVehicleCatalogTypes.h"
#include "TMOPVehicleAppearanceData.generated.h"

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPVehicleAppearanceData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    FName AppearanceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    ETMOPVehicleCategory Category = ETMOPVehicleCategory::PassengerCar;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    TObjectPtr<UMaterialInterface> BodyMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    ETMOPVehicleGlassType GlassType = ETMOPVehicleGlassType::Standard;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    TObjectPtr<UMaterialInterface> GlassMaterial;

    /** Police, ambulance, taxi or other livery material. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    TObjectPtr<UMaterialInterface> LiveryMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    TArray<FTMOPRoofAccessoryVisual> RoofAccessories;
};
