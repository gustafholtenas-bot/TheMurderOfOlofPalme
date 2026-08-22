#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Audio/TMOPAudioTypes.h"
#include "TMOPVehicleAudioComponent.generated.h"

class UAudioComponent;

UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleAudioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPVehicleAudioComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Vehicle") FName AudioProfileId = TEXT("VOLVO_240");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Vehicle") bool bEngineEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Vehicle") bool bEmergencySirenEnabled = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Vehicle", meta=(ClampMin="0.0")) float HardStartAccelerationThreshold = 500.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlayHorn();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlayDoorOpen();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlayDoorClose();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlayDoorCycle();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlaySkid();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlayTireBurst();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void PlayHardStart();
    UFUNCTION(BlueprintCallable, Category="TMOP|Audio|Vehicle") void SetEmergencySirenEnabled(bool bEnabled);

private:
    void RefreshProfile();
    void PlayOneShot(FName AudioId, float Volume = 1.0f);
    void StopLoops();
    FTMOPVehicleAudioProfileRow Profile;
    TWeakObjectPtr<UAudioComponent> EngineLoop;
    TWeakObjectPtr<UAudioComponent> DrivingLoop;
    TWeakObjectPtr<UAudioComponent> SirenLoop;
    FVector PreviousLocation = FVector::ZeroVector;
    float PreviousSpeed = 0.0f;
    bool bWasMoving = false;
};
