#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "TMOPCinemaSeatComponent.generated.h"

class ATMOPHistoricalAgent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPCinemaSeatAgentSignature,
    FName,
    SeatId,
    ATMOPHistoricalAgent*,
    Agent);

UCLASS(
    ClassGroup = (TMOP),
    BlueprintType,
    Blueprintable,
    meta = (BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPCinemaSeatComponent : public USceneComponent
{
    GENERATED_BODY()

public:
    UTMOPCinemaSeatComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(
        const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Identity")
    FName SeatId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Identity")
    FName VenueId = TEXT("PLACE_GRAND");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Identity")
    FName AuditoriumId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Identity")
    int32 RowNumber = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Identity")
    int32 SeatNumber = INDEX_NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Alignment",
        meta = (ClampMin = "0.0"))
    float ApproachDistance = 65.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Alignment")
    float ApproachVerticalOffset = -45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Alignment")
    FVector SeatedLocalOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Alignment")
    FRotator SeatedRotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Alignment")
    bool bUseManualApproachTransform = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Alignment",
        meta = (EditCondition = "bUseManualApproachTransform"))
    FTransform ManualApproachTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Seat|Behavior")
    bool bAttachAgentWhileSeated = true;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Seat|Events")
    FTMOPCinemaSeatAgentSignature OnAgentSeated;

    UPROPERTY(BlueprintAssignable, Category = "TMOP|Seat|Events")
    FTMOPCinemaSeatAgentSignature OnAgentStoodUp;

    UFUNCTION(BlueprintPure, Category = "TMOP|Seat")
    FTransform GetSeatWorldTransform() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Seat")
    FTransform GetApproachWorldTransform() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Seat")
    bool IsOccupied() const;

    UFUNCTION(BlueprintPure, Category = "TMOP|Seat")
    ATMOPHistoricalAgent* GetOccupyingAgent() const;

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seat")
    bool ReserveSeat(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seat")
    bool ReleaseSeat(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seat")
    bool SeatAgent(ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category = "TMOP|Seat")
    bool StandAgent(ATMOPHistoricalAgent* Agent);

private:
    UPROPERTY(Transient)
    TObjectPtr<ATMOPHistoricalAgent> OccupyingAgent;
};
