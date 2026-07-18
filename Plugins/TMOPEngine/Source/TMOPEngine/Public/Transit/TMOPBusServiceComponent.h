#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPBusServiceComponent.generated.h"

class UTMOPBusRouteData;
class UTMOPBusStopComponent;
class UTMOPTrafficVehicleMovementComponent;
class USceneComponent;

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

    /** Automatically discovers mesh components whose names contain "door". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service|Doors")
    bool bAnimateDoorsAutomatically = true;

    /** Optional exact component names. Empty means auto-discover by "door" in the name. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service|Doors")
    TArray<FName> DoorComponentNames;

    /** Local movement from each door's closed position to its open position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service|Doors")
    FVector DoorOpenOffset = FVector(15.0f, 25.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Service|Doors",
        meta=(ClampMin="0.05"))
    float DoorAnimationSeconds = 0.7f;

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
    void DiscoverDoorComponents();
    void UpdateDoorAnimation(float DeltaTime);
    void BeginDwell(UTMOPBusStopComponent* Stop);
    void FinishDwell(UTMOPBusStopComponent* Stop);
    FName GetStopConstraintId(const UTMOPBusStopComponent* Stop) const;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPTrafficVehicleMovementComponent> Movement;

    UPROPERTY(Transient)
    TArray<TObjectPtr<USceneComponent>> DoorComponents;

    TArray<FVector> DoorClosedLocations;

    float RemainingDwellSeconds = 0.0f;
    float DoorAnimationAlpha = 0.0f;
    float RemainingDepartureDelaySeconds = 0.0f;
    bool bDeparturePending = false;
    bool bWaitingForStopRegistration = false;
    FName LastDiagnosedStopId = NAME_None;
    FRandomStream DwellRandom;
};
