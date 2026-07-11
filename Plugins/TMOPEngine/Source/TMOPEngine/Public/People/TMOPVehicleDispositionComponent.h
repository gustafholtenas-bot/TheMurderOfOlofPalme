#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPVehicleDispositionComponent.generated.h"

UENUM(BlueprintType)
enum class ETMOPVehicleDisposition : uint8
{
    Cooperative,
    Passive,
    Nervous,
    Defensive,
    Aggressive,
    Police,
    Incapacitated
};

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleDispositionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPVehicleDispositionComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Disposition")
    ETMOPVehicleDisposition Disposition = ETMOPVehicleDisposition::Passive;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Disposition")
    bool bCanBePulledFromVehicle = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Disposition",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float ResistanceChance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Disposition",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float AttackChanceOnResistance = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vehicle Disposition",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float FleeChanceOnResistance = 0.0f;
};
