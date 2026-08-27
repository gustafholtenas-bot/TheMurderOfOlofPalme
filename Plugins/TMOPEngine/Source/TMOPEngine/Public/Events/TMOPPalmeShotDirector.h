#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "TMOPPalmeShotDirector.generated.h"

class ATMOPHistoricalAgent;
class UAnimSequence;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class UStaticMesh;
class UStaticMeshComponent;

/** Synchronizes the two-character Palme shot animation and its ballistic FX. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPalmeShotDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPPalmeShotDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Identity")
    FName ShotEventId = TEXT("PALME_SHOT_1");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Identity")
    FName OlofEntityId = TEXT("OLOF_PALME");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Identity")
    FName KillerEntityId = TEXT("THE_KILLER");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Animation")
    TObjectPtr<UAnimSequence> OlofShotAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Animation")
    TObjectPtr<UAnimSequence> KillerShotAnimation;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Animation")
    float SequenceDurationSeconds = 5.333333f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Animation")
    float FirstShotTimeSeconds = 1.7f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Animation")
    float SecondShotTimeSeconds = 4.6f;

    /** Optional exact start anchors. Empty keeps each timeline position. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Placement")
    FName OlofStartAnchorId = TEXT("Mordplatsen");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Placement")
    FName KillerStartAnchorId = TEXT("Dekorimaingang");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Placement")
    FName KillerMuzzleSocket = TEXT("hand_r");

    /** First bullet: muzzle -> wall, then optional ricochet continuation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Ballistics")
    FName FirstShotWallImpactAnchorId = TEXT("PALME_SHOT_WALL_IMPACT");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Ballistics")
    FName FirstShotRicochetEndAnchorId = TEXT("PALME_SHOT_RICOCHET_END");
    /** Second bullet: muzzle -> snow bank. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Ballistics")
    FName SecondShotSnowImpactAnchorId = TEXT("PALME_SHOT_SNOW_IMPACT");
    /** Disable if the snow-bank round should be shot one instead. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Ballistics")
    bool bFirstShotUsesWallRoute = true;

    /** Niagara beam reads User.Start and User.End in world space. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|FX")
    TObjectPtr<UNiagaraSystem> BulletLightTrailEffect;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|FX")
    TObjectPtr<UNiagaraSystem> MuzzleSmokeEffect;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|FX")
    TObjectPtr<UNiagaraSystem> WallImpactEffect;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|FX")
    TObjectPtr<UNiagaraSystem> SnowImpactEffect;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|FX")
    TObjectPtr<USoundBase> FirstShotSound;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|FX")
    TObjectPtr<USoundBase> SecondShotSound;

    /** Optional blood pool revealed when Olof reaches the final grounded frame. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Blood")
    TObjectPtr<UStaticMesh> BloodPoolMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Blood")
    FName BloodPoolAnchorId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Blood")
    FTransform BloodPoolLocalTransform = FTransform(
        FRotator::ZeroRotator, FVector(0.0f, 0.0f, 1.0f), FVector::OneVector);

    /** Experimental dramatic mode, evaluated four simulation seconds before shot one. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Experimental Slow Motion")
    bool bEnableProximitySlowMotion = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Experimental Slow Motion",
        meta=(ClampMin="0.0", Units="m"))
    float SlowMotionActivationRadiusMeters = 70.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Experimental Slow Motion",
        meta=(ClampMin="0.01", ClampMax="1.0"))
    float SlowMotionFactor = 0.25f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Experimental Slow Motion",
        meta=(ClampMin="0.0", Units="s"))
    float SlowMotionLeadSeconds = 4.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Palme Shot|Experimental Slow Motion",
        meta=(ClampMin="0.0", Units="s"))
    float SlowMotionEndSecondsAfterFirstShot = 6.0f;

private:
    UFUNCTION()
    void HandleHistoricalEventTriggered(FName EventId, FTMOPTime TriggerTime);
    UFUNCTION()
    void HandleHistoricalEventsReset(int32 LoopNumber);
    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);
    void StartSequence();
    void FireFirstShot();
    void FireSecondShot();
    void FinishSequence();
    void ResetSequence();
    ATMOPHistoricalAgent* FindAgent(FName EntityId) const;
    FVector ResolveAnchorLocation(FName AnchorId, const FVector& Fallback) const;
    FVector GetMuzzleLocation() const;
    void SpawnTrail(const FVector& Start, const FVector& End);
    void FireWallRound(USoundBase* Sound);
    void FireSnowRound(USoundBase* Sound);
    void SpawnEffect(UNiagaraSystem* Effect, const FVector& Location,
        const FVector& Direction = FVector::ForwardVector);
    void TryActivateProximitySlowMotion(const FTMOPTime& NewTime,
        const FTMOPTime& ShotTime);
    void RestoreSlowMotion();
    void ShowBloodPool();

    TWeakObjectPtr<ATMOPHistoricalAgent> OlofAgent;
    TWeakObjectPtr<ATMOPHistoricalAgent> KillerAgent;
    float SequenceTime = 0.0f;
    bool bSequenceActive = false;
    bool bFirstShotFired = false;
    bool bSecondShotFired = false;
    bool bSlowMotionActive = false;
    bool bSlowMotionEvaluated = false;
    float SavedTimeScale = 1.0f;
    float SavedGlobalTimeDilation = 1.0f;
    int32 SlowMotionEndTimeSeconds = INDEX_NONE;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMeshComponent> BloodPoolComponent;
};
