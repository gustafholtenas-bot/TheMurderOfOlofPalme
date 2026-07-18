#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPBusPassengerComponent.generated.h"

class ACharacter;
class ATMOPHistoricalAgent;
class UBoxComponent;
class UTMOPBusStopComponent;
class UTMOPTrafficVehicleMovementComponent;
class UTMOPVehicleSeatComponent;
class UTMOPBusPassengerManifest;

UENUM(BlueprintType)
enum class ETMOPBusPassengerJourneyState : uint8
{
    WaitingToBoard,
    Aboard,
    Completed,
    Error
};

/** Carries free-moving characters with a spline-driven bus and manages seats/driver. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPBusPassengerComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPBusPassengerComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    /** Optional Box Collision component placed visually in the bus Blueprint. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Interior")
    FName PassengerVolumeComponentName = TEXT("PassengerVolume");

    /** Optional Box Collision across the door opening. Departure waits while a character overlaps it. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Interior")
    FName DoorwayVolumeComponentName = TEXT("DoorwayVolume");

    /** Fallback centre and half-size when no named Box Collision exists. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Interior")
    FVector InteriorVolumeCenter = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Interior")
    FVector InteriorVolumeExtent = FVector(500.0f, 115.0f, 120.0f);

    /** Extra margin before a carried character is considered outside the bus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Interior",
        meta=(ClampMin="0.0"))
    float ExitMarginCm = 35.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Driver")
    bool bSpawnDriverAutomatically = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Driver")
    TSubclassOf<ACharacter> DriverClass;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Passengers|Driver")
    TObjectPtr<ACharacter> SpawnedDriver;

    /** Assigned explicitly by the scheduled run. Empty means no documented NPC passengers. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Manifest")
    TObjectPtr<UTMOPBusPassengerManifest> PassengerManifest;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Bus Passengers|Manifest")
    FName ServiceRunId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Bus Passengers|Seats",
        meta=(ClampMin="25.0"))
    float PlayerSeatInteractionRadiusCm = 175.0f;

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Passengers")
    bool IsCharacterAboard(const ACharacter* Character) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Passengers")
    void RegisterFreePassenger(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Passengers")
    void UnregisterFreePassenger(ACharacter* Character);

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Passengers|Seats")
    UTMOPVehicleSeatComponent* FindAvailablePassengerSeat(FName PreferredSeatId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Passengers|Seats")
    bool SeatCharacter(ACharacter* Character, FName PreferredSeatId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Passengers|Seats")
    bool UnseatCharacter(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Passengers|Seats")
    bool ToggleNearestSeat(ACharacter* Character);

    UFUNCTION(BlueprintCallable, Category="TMOP|Bus Passengers|Manifest")
    bool InitializePassengerManifest(UTMOPBusPassengerManifest* Manifest, FName RunId);

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Passengers|Manifest")
    TArray<ETMOPBusPassengerJourneyState> GetJourneyStates() const { return JourneyStates; }

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Passengers|Interior")
    UBoxComponent* GetPassengerVolume() const { return PassengerVolume; }

    UFUNCTION(BlueprintPure, Category="TMOP|Bus Passengers|Interior")
    bool IsDoorwayClear() const;

private:
    void CreatePassengerVolume();
    void DiscoverNewPassengers();
    void CarryRegisteredPassengers(const FTransform& PreviousBusTransform,
        const FTransform& CurrentBusTransform);
    void SpawnDriver();
    void AssignManifestDriver();
    void ProcessPassengerManifest();
    ATMOPHistoricalAgent* ResolveHistoricalAgent(FName EntityId) const;
    bool BoardManifestPassenger(int32 JourneyIndex, UTMOPBusStopComponent* Stop);
    bool AlightManifestPassenger(int32 JourneyIndex, UTMOPBusStopComponent* Stop);

    UPROPERTY(Transient)
    TObjectPtr<UBoxComponent> PassengerVolume;

    UPROPERTY(Transient)
    TObjectPtr<UBoxComponent> DoorwayVolume;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPTrafficVehicleMovementComponent> VehicleMovement;

    UPROPERTY(Transient)
    TArray<TObjectPtr<ACharacter>> FreePassengers;

    UPROPERTY(Transient)
    TArray<ETMOPBusPassengerJourneyState> JourneyStates;

    UPROPERTY(Transient)
    TObjectPtr<ATMOPHistoricalAgent> ManifestDriver;

    TSet<int32> MissingAgentWarnings;

    FTransform PreviousOwnerTransform;
    bool bPreviousTransformInitialized = false;
};
