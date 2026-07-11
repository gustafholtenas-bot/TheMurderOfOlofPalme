#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TMOPVehicleCatalogData.generated.h"

class UTMOPVehicleAppearanceData;
class UTMOPVehicleModelData;

/** Optional central list for browsing and validation. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPVehicleCatalogData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Catalog")
    TArray<TObjectPtr<UTMOPVehicleModelData>> Models;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Catalog")
    TArray<TObjectPtr<UTMOPVehicleAppearanceData>> AppearancePresets;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Catalog")
    UTMOPVehicleModelData* FindModel(FName ModelId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Catalog")
    UTMOPVehicleAppearanceData* FindAppearance(FName AppearanceId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vehicle Catalog")
    bool ValidateCatalog(TArray<FString>& OutErrors) const;
};
