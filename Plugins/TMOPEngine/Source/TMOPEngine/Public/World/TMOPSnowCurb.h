#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPSnowCurb.generated.h"

class USplineComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

/** Places non-colliding snow meshes along an editable spline.
 * Each distinct mesh uses one native HISM component, sharing draw calls.
 * Components are created in the constructor, never in OnConstruction.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPSnowCurb : public AActor
{
    GENERATED_BODY()

public:
    /** Up to MaxMeshTypes different snow meshes per actor. */
    static constexpr int32 MaxMeshTypes = 8;

    ATMOPSnowCurb();

    /** Select this component in the editor to edit spline points. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Snow Curb|Spline")
    TObjectPtr<USplineComponent> CurbSpline;

    /** Assign 1-8 meshes. Empty slots are ignored. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Meshes")
    TArray<TObjectPtr<UStaticMesh>> SnowMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="10.0", UIMin="50.0", Units="cm"))
    float SpacingCm = 125.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="0.0", Units="cm"))
    float AlongJitterCm = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="0.0", Units="cm"))
    float SideJitterCm = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="0.0", ClampMax="180.0", Units="deg"))
    float YawJitterDegrees = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    FVector2D UniformScaleRange = FVector2D(0.85f, 1.2f);

    /** Adjust for the mesh pivot or sidewalk surface. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(Units="cm"))
    float HeightOffsetCm = 0.0f;

    /** Set to 90 if source meshes run along local Y rather than local X. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(Units="deg"))
    float MeshYawOffsetDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    int32 RandomSeed = 1986;

    /** Safety limit for accidentally dense or very long splines. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Performance", meta=(ClampMin="1", UIMin="100"))
    int32 MaxInstances = 10000;

    /** 0 disables culling. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Performance", meta=(ClampMin="0.0", Units="cm"))
    float EndCullDistanceCm = 12000.0f;

    /** Disabling clears instances on the next construction update. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    bool bGenerateInEditor = true;

    /** Shows whether the last rebuild actually created any snow instances. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient, Category="Snow Curb|Debug")
    int32 GeneratedInstanceCount = 0;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Snow Curb")
    void RebuildSnow();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Snow Curb")
    void ClearSnow();

    virtual void OnConstruction(const FTransform& Transform) override;

private:
    /** Stable native components, not dynamically generated construction objects. */
    UPROPERTY(VisibleAnywhere, Category="Snow Curb|Generated")
    TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> SnowHISMComponents;
};
