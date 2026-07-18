#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPBusStopComponent.generated.h"

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPBusStopComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPBusStopComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop")
    FName StopId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop")
    FText StopName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop")
    FName LaneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop",
        meta=(ClampMin="0.0"))
    float DistanceAlongLane = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop")
    TArray<FName> ServedRouteIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop",
        meta=(ClampMin="0.0"))
    float MinimumDwellSeconds = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop",
        meta=(ClampMin="0.0"))
    float MaximumDwellSeconds = 20.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop",
        meta=(ClampMin="0.0"))
    float StopBufferCm = 80.0f;

    /** Historical agents must be within this radius to board this bus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop|Passengers",
        meta=(ClampMin="25.0"))
    float PassengerWaitingRadiusCm = 350.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop|Passengers")
    FVector BoardingLocalOffset = FVector(0.0f, 100.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Stop|Passengers")
    FVector AlightingLocalOffset = FVector(100.0f, 140.0f, 0.0f);

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Stop|Passengers")
    FVector GetBoardingWorldLocation() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Stop|Passengers")
    FVector GetAlightingWorldLocation() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Stop")
    bool ServesRoute(FName RouteId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Stop")
    bool ValidateStop(TArray<FString>& OutErrors) const;
};
