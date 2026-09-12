#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPKillerBranchDirector.generated.h"

class ATMOPKillerGhost;
class UAnimSequence;
class UMaterialInterface;
struct FTMOPKillerBranchRuntime;

/** Keep deletion out of generated constructors, where Runtime is incomplete. */
struct TMOPENGINE_API FTMOPKillerBranchRuntimeDeleter
{
    void operator()(FTMOPKillerBranchRuntime* Value) const;
};

/** Visualizes alternative escape routes exclusively for EntityId THE_KILLER. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPKillerBranchDirector : public AActor
{
    GENERATED_BODY()
public:
    ATMOPKillerBranchDirector();
    virtual ~ATMOPKillerBranchDirector() override;
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type Reason) override;
    virtual void Tick(float DeltaSeconds) override;
#if WITH_EDITOR
    virtual bool ShouldTickIfViewportsOnly() const override { return true; }
#endif

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer")
    bool bEnabled = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer")
    FName NetworkId = TEXT("KillerEscape");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer")
    bool bDrawNetworkInEditor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Time")
    FName EscapeEventId = TEXT("Palme_shot_1");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Time", meta=(Units="s"))
    float EscapeDelaySeconds = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Movement",
        meta=(ClampMin="100", ClampMax="1000", Units="cm/s"))
    float GhostRunSpeed = 450.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Movement",
        meta=(ClampMin="0.1", ClampMax="3"))
    float AnimationPlayRate = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Appearance")
    TObjectPtr<UAnimSequence> RunAnimation;
    /** Translucent material with GhostColor (vector) and GhostOpacity (scalar). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Appearance")
    TObjectPtr<UMaterialInterface> GhostMaterial;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Appearance")
    FLinearColor GhostColor = FLinearColor(0.2f, 0.65f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Appearance",
        meta=(ClampMin="0.01", ClampMax="0.8"))
    float GhostOpacity = 0.22f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Appearance",
        meta=(ClampMin="0.05", ClampMax="3", Units="s"))
    float FadeSeconds = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Limits", meta=(ClampMin="1", ClampMax="64"))
    int32 MaxConcurrentGhosts = 24;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Limits", meta=(ClampMin="1", ClampMax="16"))
    int32 MaxBranchDepth = 8;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Limits", meta=(ClampMin="1", ClampMax="2048"))
    int32 MaxScheduledLegs = 512;

    /** Re-projects authored links to navigation; call after changing the network in Play. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Killer")
    bool RebuildNetwork();
    UFUNCTION(BlueprintCallable, Category="TMOP|Killer")
    void ResetBranches();
    UFUNCTION(CallInEditor, Category="TMOP|Killer")
    void ValidateNetwork();
    UFUNCTION(BlueprintPure, Category="TMOP|Killer")
    int32 GetActiveGhostCount() const { return Ghosts.Num(); }

private:
    TUniquePtr<FTMOPKillerBranchRuntime, FTMOPKillerBranchRuntimeDeleter> Runtime;
    UPROPERTY(Transient)
    TObjectPtr<ATMOPKillerGhost> AppearanceTemplate;
    UPROPERTY(Transient)
    TMap<FString, TObjectPtr<ATMOPKillerGhost>> Ghosts;
    void ClearVisuals();
    void RebuildSchedule();
};
