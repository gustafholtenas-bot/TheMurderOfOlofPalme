#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Vehicles/TMOPVehicleCatalogTypes.h"
#include "TMOPVehicleModelData.generated.h"

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPVehicleModelData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Identity")
    FName ModelId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Identity")
    FText Manufacturer;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Identity")
    FText ModelName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Identity")
    int32 EarliestModelYear = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Identity")
    int32 LatestModelYear = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model")
    ETMOPVehicleCategory DefaultCategory = ETMOPVehicleCategory::PassengerCar;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Meshes")
    TObjectPtr<UStaticMesh> BodyMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Meshes")
    TObjectPtr<UStaticMesh> WheelMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Meshes")
    FTransform BodyLocalTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Wheels")
    FTMOPVehicleWheelSetup Wheels;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Materials")
    int32 BodyMaterialSlotIndex = 0;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Materials")
    int32 GlassMaterialSlotIndex = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Materials")
    int32 LiveryMaterialSlotIndex = 2;

    /** Base transform to which appearance roof accessory transforms are relative. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Accessories")
    FTransform RoofMountTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Dimensions")
    float VehicleLengthCm = 450.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model|Dimensions")
    float VehicleWidthCm = 180.0f;
};
