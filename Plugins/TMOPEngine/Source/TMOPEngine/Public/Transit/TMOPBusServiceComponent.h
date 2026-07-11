#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPBusServiceComponent.generated.h"

class UTMOPBusRouteData;
class UTMOPBusStopComponent;
class UTMOPTrafficVehicleMovementComponent;

UENUM(BlueprintType)
enum class ETMOPBusServiceState : uint8
{
    Uninitialized,
    DrivingToStop,
    Dwelling,
    RouteComplete,
    InvalidRoute
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPBusStopEventSignature, FName, StopId, int32, StopIndex);

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPBusServiceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPBusServiceComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service")
    TObjectPtr<UTMOPBusRouteData> RouteData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service")
    FName ServiceRunId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service")
    int32 DwellRandomSeed = 43053;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Service")
    int32 CurrentStopIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Service")
    ETMOPBusServiceState ServiceState = ETMOPBusServiceState::Uninitialized;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Service")
    bool bDoorsOpen = false;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Bus Service|Events")
    FTMOPBusStopEventSignature OnArrivedAtStop;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Bus Service|Events")
    FTMOPBusStopEventSignature OnDepartedStop;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Service")
    bool InitializeService();

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Service")
    void OpenDoors();

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Service")
    void CloseDoors();

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Service")
    UTMOPBusStopComponent* GetCurrentTargetStop() const;

private:
    void UpdateApproachToStop();
    void BeginDwell(UTMOPBusStopComponent* Stop);
    void FinishDwell(UTMOPBusStopComponent* Stop);
    FName GetStopConstraintId(const UTMOPBusStopComponent* Stop) const;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPTrafficVehicleMovementComponent> Movement;

    float RemainingDwellSeconds = 0.0f;
    FRandomStream DwellRandom;
};
