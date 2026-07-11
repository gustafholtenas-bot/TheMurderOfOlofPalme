#include "Vehicles/TMOPVehicleCatalogData.h"

#include "Vehicles/TMOPVehicleAppearanceData.h"
#include "Vehicles/TMOPVehicleModelData.h"

UTMOPVehicleModelData* UTMOPVehicleCatalogData::FindModel(const FName ModelId) const
{
    for (UTMOPVehicleModelData* Model : Models)
        if (IsValid(Model) && Model->ModelId == ModelId) return Model;
    return nullptr;
}

UTMOPVehicleAppearanceData* UTMOPVehicleCatalogData::FindAppearance(const FName AppearanceId) const
{
    for (UTMOPVehicleAppearanceData* Appearance : AppearancePresets)
        if (IsValid(Appearance) && Appearance->AppearanceId == AppearanceId) return Appearance;
    return nullptr;
}

bool UTMOPVehicleCatalogData::ValidateCatalog(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TSet<FName> ModelIds;
    for (UTMOPVehicleModelData* Model : Models)
    {
        if (!IsValid(Model)) { OutErrors.Add(TEXT("Catalog contains an invalid model asset.")); continue; }
        if (Model->ModelId.IsNone() || ModelIds.Contains(Model->ModelId))
            OutErrors.Add(TEXT("A vehicle model has missing or duplicate ModelId."));
        ModelIds.Add(Model->ModelId);
        if (!IsValid(Model->BodyMesh)) OutErrors.Add(FString::Printf(TEXT("Model '%s' has no BodyMesh."), *Model->ModelId.ToString()));
        if (!IsValid(Model->WheelMesh)) OutErrors.Add(FString::Printf(TEXT("Model '%s' has no WheelMesh."), *Model->ModelId.ToString()));
    }
    TSet<FName> AppearanceIds;
    for (UTMOPVehicleAppearanceData* Appearance : AppearancePresets)
    {
        if (!IsValid(Appearance)) { OutErrors.Add(TEXT("Catalog contains an invalid appearance asset.")); continue; }
        if (Appearance->AppearanceId.IsNone() || AppearanceIds.Contains(Appearance->AppearanceId))
            OutErrors.Add(TEXT("An appearance preset has missing or duplicate AppearanceId."));
        AppearanceIds.Add(Appearance->AppearanceId);
        if (!IsValid(Appearance->BodyMaterial))
            OutErrors.Add(FString::Printf(TEXT("Appearance '%s' has no BodyMaterial."), *Appearance->AppearanceId.ToString()));
    }
    return OutErrors.IsEmpty();
}
