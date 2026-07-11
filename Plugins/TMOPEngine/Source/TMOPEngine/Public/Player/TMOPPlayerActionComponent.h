#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPPlayerActionComponent.generated.h"

class ACharacter;
class UTMOPAnimationStateComponent;

UENUM(BlueprintType)
enum class ETMOPPlayerAction : uint8
{
    None,
    Interact,
    Inspect,
    Talk,
    PhoneOrRadio,
    Smoke,
    LookAround,
    Sit,
    Stand,
    EnterVehicle,
    ExitVehicle,
    Punch,
    Kick,
    SwingWeapon,
    AimGun,
    ShootGun
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPPlayerActionSignature, ETMOPPlayerAction, Action, AActor*, Target);

UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerActionComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerActionComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Action")
    ETMOPPlayerAction CurrentAction = ETMOPPlayerAction::None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Action")
    TObjectPtr<AActor> CurrentTarget;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Player Action")
    bool bMovementBlocked = false;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Player Action|Events")
    FTMOPPlayerActionSignature OnActionStarted;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Player Action|Events")
    FTMOPPlayerActionSignature OnActionCompleted;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Player Action|Events")
    FTMOPPlayerActionSignature OnActionCancelled;

    UFUNCTION(BlueprintPure, Category="TMOP|Player Action")
    bool CanStartAction(ETMOPPlayerAction Action) const;

    /** Negative duration keeps the action active until cancelled/completed manually. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Player Action")
    bool StartAction(ETMOPPlayerAction Action, AActor* Target,
        float DurationSeconds = 0.75f, bool bBlockMovement = true);

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Action")
    bool CompleteCurrentAction();

    UFUNCTION(BlueprintCallable, Category="TMOP|Player Action")
    bool CancelCurrentAction();

private:
    void ApplyAnimationForAction(ETMOPPlayerAction Action);
    void ClearActionAnimation(ETMOPPlayerAction Action);
    void SetMovementBlocked(bool bBlocked);

    UPROPERTY(Transient)
    TObjectPtr<ACharacter> CharacterOwner;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPAnimationStateComponent> AnimationState;

    float RemainingActionSeconds = 0.0f;
};
