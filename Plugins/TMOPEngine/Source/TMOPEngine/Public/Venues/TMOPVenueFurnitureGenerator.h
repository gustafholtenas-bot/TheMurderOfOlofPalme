#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPVenueFurnitureGenerator.generated.h"

class ATMOPHistoricalAnchor;
class AStaticMeshActor;
class UStaticMesh;
struct FTMOPVenueFurnitureSeatRecord;

/**
 * One-click editor utility that turns restaurant/pub seat anchors into visible
 * furniture. Grand cinema anchors are deliberately and unconditionally ignored.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPVenueFurnitureGenerator : public AActor
{
    GENERATED_BODY()

public:
    ATMOPVenueFurnitureGenerator();

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Meshes")
    TObjectPtr<UStaticMesh> ChairMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Meshes")
    TObjectPtr<UStaticMesh> BarStoolMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Meshes")
    TObjectPtr<UStaticMesh> TableMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Meshes")
    TObjectPtr<UStaticMesh> BarCounterMesh;

    /** Applied relative to every table-seat anchor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Placement")
    FTransform ChairLocalOffset = FTransform::Identity;

    /** Applied relative to every bar-seat anchor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Placement")
    FTransform BarStoolLocalOffset = FTransform::Identity;

    /** Applied after calculating the centre of each table group. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Placement")
    FTransform TableLocalOffset = FTransform::Identity;

    /** Applied after calculating the bar counter transform. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Placement")
    FTransform BarCounterLocalOffset = FTransform::Identity;

    /** Signed distance from the average bar-seat position towards their facing direction. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Bar", meta=(Units="cm"))
    float BarCounterForwardOffsetCm = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Bar", meta=(ClampMin="0.0", Units="cm"))
    float BarCounterEndPaddingCm = 50.0f;

    /** Scale the counter mesh on local X so it covers the complete row of stools. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Bar")
    bool bAutoScaleBarCounterLength = true;

    /** Generated static meshes receive normal collision. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Venue Furniture|Collision")
    bool bEnableFurnitureCollision = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Venue Furniture|Result")
    int32 LastSeatAnchorCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Venue Furniture|Result")
    int32 LastChairCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Venue Furniture|Result")
    int32 LastTableCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Venue Furniture|Result")
    int32 LastBarCounterCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Venue Furniture|Result")
    int32 LastSkippedGrandCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly,
        Category="TMOP|Venue Furniture|Result")
    int32 LastErrorCount = 0;

    UFUNCTION(CallInEditor, BlueprintCallable,
        Category="TMOP|Venue Furniture")
    void GenerateOrUpdateVenueFurniture();

private:
    bool ParseSeatAnchor(ATMOPHistoricalAnchor* Anchor,
        FTMOPVenueFurnitureSeatRecord& OutRecord) const;
    bool IsGrandAnchor(const ATMOPHistoricalAnchor* Anchor) const;
    AStaticMeshActor* FindGeneratedActor(FName FurnitureKey) const;
    AStaticMeshActor* CreateOrUpdateMeshActor(FName FurnitureKey,
        const FString& Label, UStaticMesh* Mesh,
        const FTransform& WorldTransform, const FString& VenueKey);
};
