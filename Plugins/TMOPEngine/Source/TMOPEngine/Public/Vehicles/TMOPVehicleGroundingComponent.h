#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPVehicleGroundingComponent.generated.h"

class USceneComponent;

USTRUCT(BlueprintType)
struct FTMOPGroundWheel
{
    GENERATED_BODY()
    /** Wheel centre at rest, relative to the actor/collision root (not VisualRoot). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground") FVector LocalCenter = FVector::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground", meta=(ClampMin="1")) float RadiusCm = 32;
    /** Optional separate wheel component. Empty permits a rigid one-piece model. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Ground") FName WheelComponentName;
};

struct FTMOPGroundSolution
{
    FTransform Pose;
    TArray<FVector> WheelCenters;
    TArray<FName> WheelNames;
    bool bApproximate = false;
    bool bSettled = true;
    FString Error;
};

/** Shared, kinematic road support. Runs after movement and before history capture. */
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPVehicleGroundingComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTMOPVehicleGroundingComponent();
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground") bool bEnabled = true;
    /** Any number of wheels/axles. Empty uses configured wheels, then a box footprint approximation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground") TArray<FTMOPGroundWheel> Wheels;
    /** Empty accepts static upward-facing collision. For production use e.g. TMOP_Ground on road actors/components. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground") FName RequiredGroundTag;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground") bool bTraceComplex = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="1")) float SearchUpCm = 300;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="1")) float SearchDownCm = 400;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="1")) float MaxCorrectionCm = 300;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="0", ClampMax="60")) float MaxSlopeDegrees = 25;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="0")) float SuspensionTravelCm = 22;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="0")) float TireClearanceCm = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground", meta=(ClampMin="0.1")) float TiltResponse = 10;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Ground") bool bDrawContacts = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Ground") bool bGroundValid = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Ground") bool bUsingApproximateFootprint = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Ground") FString GroundStatus;

    /** Pure query: player movement can sweep the corrected candidate before committing it. */
    bool Solve(const FTransform& Requested, FTMOPGroundSolution& Out, float DeltaTime = 0) const;
    void ApplyWheels(const FTMOPGroundSolution& Solution);
    /** Only after a collision-tested move reached Solution.Pose. Avoids querying again that frame. */
    void AcceptSupportedPose(const FTMOPGroundSolution& Solution);
    void UpdateGroundContact(float DeltaTime);
    UFUNCTION(BlueprintCallable, CallInEditor, Category="TMOP|Ground") void ValidateGroundContact();
    UFUNCTION(BlueprintCallable, CallInEditor, Category="TMOP|Ground") void SnapToGround();
    UFUNCTION(BlueprintCallable, Category="TMOP|Ground") void InvalidateGroundCache();
private:
    bool BuildWheels(TArray<FTMOPGroundWheel>& Out, bool& bApproximate) const;
    bool TraceGround(const FVector& Center, float ExpectedGroundZ, FHitResult& Out) const;
    bool bHaveCache = false;
    FTransform LastRequested;
    FTransform LastApplied;
    FTMOPGroundSolution Cached;
    double LastFailureLogTime = -100;
    uint64 AcceptedFrame = MAX_uint64;
};
