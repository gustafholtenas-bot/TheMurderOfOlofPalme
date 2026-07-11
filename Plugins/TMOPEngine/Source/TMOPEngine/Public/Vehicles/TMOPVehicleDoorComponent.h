#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPVehicleDoorComponent.generated.h"

UENUM(BlueprintType)
enum class ETMOPVehicleDoorSide : uint8
{
    Left,
    Right
};

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleDoorComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPVehicleDoorComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Door")
    FName DoorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Door")
    FName SeatId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Door")
    ETMOPVehicleDoorSide DoorSide = ETMOPVehicleDoorSide::Left;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Door")
    bool bEnabled = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Door")
    FVector ApproachLocalOffset = FVector(0.0f, 90.0f, 0.0f);

    UFUNCTION(BlueprintPure, Category="TMOP|Vehicle Door")
    FTransform GetApproachWorldTransform() const;
};
