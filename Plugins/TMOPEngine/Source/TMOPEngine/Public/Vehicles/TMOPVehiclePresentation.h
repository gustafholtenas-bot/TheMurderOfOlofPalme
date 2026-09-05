#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPVehicleCatalogTypes.h"

class ATMOPVehicleBase;
class USceneComponent;
class UMeshComponent;
struct FTMOPHistoricalVehicleRow;

/** Authoritative visual assembly, used by runtime and isolated editor previews. */
namespace TMOPVehiclePresentation
{
    TMOPENGINE_API UClass* ResolveClass(const FTMOPHistoricalVehicleRow& Profile, UClass* DefaultClass);
    TMOPENGINE_API UMeshComponent* ResolveBodyMesh(ATMOPVehicleBase* Vehicle);
    TMOPENGINE_API bool ApplyProfile(ATMOPVehicleBase* Vehicle, const FTMOPHistoricalVehicleRow& Profile,
        TArray<FString>* OutWarnings = nullptr);
    TMOPENGINE_API bool Attach(USceneComponent* Part, UMeshComponent* Body, USceneComponent* RoofMount,
        FName Socket, const FTransform& Offset, FString& OutWarning);
    TMOPENGINE_API void BuildAccessories(ATMOPVehicleBase* Vehicle, UMeshComponent* Body,
        const TArray<FTMOPVehicleAccessoryVisual>& Entries, TArray<FString>& OutWarnings);
}
