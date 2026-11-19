#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPSnowCurb.generated.h"

class USplineComponent;
class UHierarchicalInstancedStaticMeshComponent;
class UStaticMesh;

/** Editor-generated, spline-aligned snow clumps. Instances share mesh assets. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPSnowCurb : public AActor
{
    GENERATED_BODY()

public:
    ATMOPSnowCurb();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Snow Curb")
    TObjectPtr<USplineComponent> CurbSpline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Meshes")
    TArray<TObjectPtr<UStaticMesh>> SnowMeshes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="10.0", Units="cm"))
    float SpacingCm = 125.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="0.0", Units="cm"))
    float AlongJitterCm = 18.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="0.0", Units="cm"))
    float SideJitterCm = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement", meta=(ClampMin="0.0", ClampMax="180.0"))
    float YawJitterDegrees = 12.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    FVector2D UniformScaleRange = FVector2D(0.85f, 1.2f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    float HeightOffsetCm = 0.0f;

    /** If meshes are modeled lengthwise along Y, use 90 degrees. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    float MeshYawOffsetDegrees = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    int32 RandomSeed = 1986;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Performance", meta=(ClampMin="0.0", Units="cm"))
    float EndCullDistanceCm = 12000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Snow Curb|Placement")
    bool bGenerateInEditor = true;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Snow Curb")
    void RebuildSnow();

    UFUNCTION(CallInEditor, BlueprintCallable, Category="Snow Curb")
    void ClearSnow();

    virtual void OnConstruction(const FTransform& Transform) override;

private:
    UPROPERTY(VisibleAnywhere, Category="Snow Curb|Generated")
    TArray<TObjectPtr<UHierarchicalInstancedStaticMeshComponent>> SnowHISMComponents;
};
