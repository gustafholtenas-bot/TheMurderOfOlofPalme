#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPLaneNetworkActor.generated.h"

class USceneComponent;
class USplineComponent;

USTRUCT(BlueprintType)
struct FTMOPLaneRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName LaneID;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName RoadID;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString Direction;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) int32 LaneIndexFromRight = 1;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bLegalForNormalTraffic = true;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) bool bIsCrossing = false;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName FromLaneID;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName ToLaneID;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FString TurnType;
    UPROPERTY(Transient, BlueprintReadOnly) TObjectPtr<USplineComponent> Spline = nullptr;
};

UCLASS(BlueprintType)
class THEMURDEROFOLOFPALME_API ATMOPLaneNetworkActor : public AActor
{
    GENERATED_BODY()

public:
    ATMOPLaneNetworkActor();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Traffic")
    FString JsonPath = TEXT("Traffic/TMOP_LaneNetwork.json");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Traffic")
    bool bLoadOnBeginPlay = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Traffic")
    bool bDrawSplines = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Traffic")
    TArray<FTMOPLaneRuntimeData> Lanes;

    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic")
    bool LoadLaneNetwork();

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    bool FindLane(FName LaneID, FTMOPLaneRuntimeData& OutLane) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Traffic")
    TArray<FName> GetNextLaneIDs(FName LaneID, bool bAllowIllegalTraffic = false) const;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY() TObjectPtr<USceneComponent> SceneRoot;
    TMap<FName, int32> LaneIndexByID;
    TMultiMap<FName, FName> NextLanesByID;
    void ClearGeneratedSplines();
};
