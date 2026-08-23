#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPPlayerMovementAudioComponent.generated.h"

class ATMOPAudioDirector;
class ATMOPVehicleBase;
class UPrimitiveComponent;

/** First-person/local feedback sounds not supplied by animation notifies. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerMovementAudioComponent final
    : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerMovementAudioComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player")
    FName JumpAudioId = TEXT("PLAYER_JUMP");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player")
    FName LandAudioId = TEXT("PLAYER_LAND");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player")
    FName RunBreathAudioId = TEXT("PLAYER_BREATH_RUN");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player")
    FName BodyImpactAudioId = TEXT("PLAYER_BODY_IMPACT");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player")
    FName EnterVehicleAudioId = TEXT("PLAYER_ENTER_VEHICLE");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player|Breathing",
        meta=(ClampMin="0.25", Units="s"))
    float RunningBreathIntervalSeconds = 2.2f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player|Breathing",
        meta=(ClampMin="1.0", Units="cm/s"))
    float RunningBreathMinimumSpeedCmPerSecond = 430.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player|Impact",
        meta=(ClampMin="1.0", Units="cm/s"))
    float MinimumImpactSpeedCmPerSecond = 240.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Audio|Player|Impact",
        meta=(ClampMin="0.0", Units="s"))
    float ImpactCooldownSeconds = 0.3f;

private:
    UFUNCTION()
    void HandleOwnerHit(AActor* SelfActor, AActor* OtherActor,
        FVector NormalImpulse, const FHitResult& Hit);
    UFUNCTION()
    void HandleVehicleSessionStarted(ATMOPVehicleBase* Vehicle, bool bDriver);

    ATMOPAudioDirector* FindAudioDirector() const;
    void PlayLocal(FName AudioId, float VolumeMultiplier = 1.0f) const;

    bool bWasFalling = false;
    float BreathElapsedSeconds = 0.0f;
    float ImpactCooldownRemainingSeconds = 0.0f;
    float PreviousVerticalVelocityCmPerSecond = 0.0f;
};
