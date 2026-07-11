#pragma once

#include "CoreMinimal.h"
#include "TMOPVehicleCatalogTypes.generated.h"

class UMaterialInterface;
class UStaticMesh;

UENUM(BlueprintType)
enum class ETMOPVehicleCategory : uint8
{
    PassengerCar,
    Taxi,
    Police,
    Ambulance,
    Bus,
    Van,
    Truck,
    Other
};

UENUM(BlueprintType)
enum class ETMOPVehicleGlassType : uint8
{
    Standard,
    DarkTinted,
    Custom
};

UENUM(BlueprintType)
enum class ETMOPRoofAccessoryType : uint8
{
    None,
    TaxiSign,
    PoliceBeacon,
    PoliceLightbar,
    AmbulanceBeacon,
    Custom
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPVehicleWheelSetup
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model")
    FTransform FrontLeft = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model")
    FTransform FrontRight = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model")
    FTransform RearLeft = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model")
    FTransform RearRight = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model",
        meta=(ClampMin="1.0"))
    float WheelRadiusCm = 32.0f;

    /** Local rotation axis. Y is common for wheels imported facing Unreal X-forward. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Model")
    FVector RotationAxis = FVector(0.0f, 1.0f, 0.0f);
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRoofAccessoryVisual
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    ETMOPRoofAccessoryType Type = ETMOPRoofAccessoryType::None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    TObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    FTransform LocalTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Vehicle Appearance")
    TObjectPtr<UMaterialInterface> Material;
};
