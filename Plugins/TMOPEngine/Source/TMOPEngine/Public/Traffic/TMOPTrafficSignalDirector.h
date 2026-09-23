#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPTrafficSignalDirector.generated.h"
class ATMOPTrafficSignalController;
class UTMOPTrafficStopLineComponent;
class UTMOPTrafficVehicleMovementComponent;
class UTMOPPedestrianCrossingComponent;
class APawn;

/** Registry and live traffic constraints; historical playback owns the recorded result. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTrafficSignalDirector : public AActor
{
    GENERATED_BODY()
public:
    ATMOPTrafficSignalDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bStopOnYellow = true;
    /** Off by default: lights are presentation, existing timelines keep control. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bControlVehicles = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    bool bControlPedestrians = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal", meta=(ClampMin="0.02"))
    float UpdateIntervalSeconds = 0.05f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal", meta=(ClampMin="0.1"))
    float RegistryRefreshSeconds = 1.0f;
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    int32 DiscoverSignalSystem();
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool ValidateSignalSystem(TArray<FString>& OutErrors) const;
    UFUNCTION(CallInEditor, Category="TMOP|Traffic Signal")
    void ValidateAndLog();
    UFUNCTION(BlueprintPure, Category="TMOP|Traffic Signal")
    bool CanEnterCrossing(FName CrossingId) const;
    /** Entire segment check for deadline corrections; live movement supplies a short lookahead. */
    bool IsPedestrianPathBlocked(const FVector& From, const FVector& To) const;
    static bool IsPedestrianPathBlocked(UWorld* World, const FVector& From, const FVector& To);
    static FString ConfigurationSignature(UWorld* World);
    static bool IsRouteSignalControlled(UWorld* World, const TArray<FName>& LaneIds);
private:
    void UpdateVehicleConstraints();
    ATMOPTrafficSignalController* FindController(FName Intersection, FName Group) const;
    UPROPERTY(Transient) TArray<TObjectPtr<UTMOPTrafficStopLineComponent>> StopLines;
    UPROPERTY(Transient) TArray<TObjectPtr<ATMOPTrafficSignalController>> Controllers;
    UPROPERTY(Transient) TArray<TObjectPtr<UTMOPPedestrianCrossingComponent>> Crossings;
    TArray<TWeakObjectPtr<UTMOPTrafficVehicleMovementComponent>> Vehicles;
    TArray<TWeakObjectPtr<APawn>> Pedestrians;
    float UpdateAccumulator = 0;
    float RegistryAccumulator = 0;
};
