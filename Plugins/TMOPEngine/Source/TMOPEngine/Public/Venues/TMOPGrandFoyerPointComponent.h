#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPGrandFoyerPointComponent.generated.h"

UENUM(BlueprintType)
enum class ETMOPGrandFoyerPointType : uint8
{
    WaitingPoint,
    BuildingExit
};

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPGrandFoyerPointComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPGrandFoyerPointComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    FName PointId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer")
    ETMOPGrandFoyerPointType PointType = ETMOPGrandFoyerPointType::WaitingPoint;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Grand Foyer",
        meta=(ClampMin="10.0"))
    float AcceptanceRadius = 70.0f;
};
