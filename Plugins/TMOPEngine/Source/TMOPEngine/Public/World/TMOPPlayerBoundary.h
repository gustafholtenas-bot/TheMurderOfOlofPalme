#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPPlayerBoundary.generated.h"

class UArrowComponent;
class UBoxComponent;
class UMaterialInterface;
class UNiagaraComponent;
class UNiagaraSystem;
class USceneComponent;
class UStaticMeshComponent;

/**
 * One-sided playable-area boundary. It constrains only the local player while
 * NPCs, observed people and autonomous traffic remain completely unaffected.
 * Place segments with local +X (the green arrow) pointing into the playable area.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPPlayerBoundary : public AActor
{
    GENERATED_BODY()

public:
    ATMOPPlayerBoundary();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Boundary")
    TObjectPtr<USceneComponent> BoundaryRoot;

    /** Editor guide only. It has no collision and never affects navigation. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Boundary")
    TObjectPtr<UBoxComponent> BoundaryGuide;

    /** Opaque fallback wall or custom fog-material carrier. Never collides. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Boundary|Visual")
    TObjectPtr<UStaticMeshComponent> FogWallMesh;

    /** Optional local fog particles. They never affect collision or navigation. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Boundary|Visual")
    TObjectPtr<UNiagaraComponent> FogEffect;

    /** Green arrow points toward the side where the player is allowed. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Boundary")
    TObjectPtr<UArrowComponent> AllowedSideArrow;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary",
        meta=(ClampMin="100.0", Units="cm"))
    float HalfLengthCm = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary",
        meta=(ClampMin="1.0", Units="cm"))
    float HalfThicknessCm = 100.0f;

    /** Actor Z is ground level; the boundary extends upward by twice this value. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary",
        meta=(ClampMin="100.0", Units="cm"))
    float HalfHeightCm = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary")
    bool bEnablePlayerBlocking = true;

    /** Also prevents the vehicle containing the player from leaving the map. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary")
    bool bBlockVehicleContainingPlayer = true;

    /** Extra space retained between the controlled actor and the visible wall. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary",
        meta=(ClampMin="0.0", Units="cm"))
    float PushBackPaddingCm = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary|Visual")
    bool bShowFogWall = true;

    /**
     * Assign an opaque/dithered fog material here. Without one, Unreal's opaque
     * default material is intentionally used so the player cannot see through.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary|Visual")
    TObjectPtr<UMaterialInterface> FogMaterial;

    /** Optional Niagara fog system layered over the non-see-through wall. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary|Visual")
    TObjectPtr<UNiagaraSystem> FogNiagaraSystem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Boundary|Visual")
    FVector FogEffectScale = FVector(5.0f, 50.0f, 10.0f);

private:
    AActor* ResolveControlledActor() const;
    float ResolveClearanceCm(const AActor* ControlledActor) const;
    void ConstrainPlayerToAllowedSide();
    void RefreshComponents();
};
