#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPVerticalTransport.generated.h"

class ATMOPHistoricalAnchor;
class USceneComponent;

UENUM(BlueprintType)
enum class ETMOPVerticalTransportType : uint8
{
    Elevator,
    Escalator
};

UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPVerticalTransport : public AActor
{
    GENERATED_BODY()

public:
    ATMOPVerticalTransport();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vertical Transport")
    ETMOPVerticalTransportType TransportType = ETMOPVerticalTransportType::Elevator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vertical Transport")
    FName LowerAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vertical Transport")
    FName UpperAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vertical Transport", meta=(ClampMin="0.1"))
    float TravelDurationSeconds = 8.0f;

    /** Elevator arrival/door delay. Escalators normally use zero. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vertical Transport", meta=(ClampMin="0.0"))
    float BoardingDelaySeconds = 1.5f;

    /** Optional lift cabin or escalator step root moved together with the passenger. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Vertical Transport")
    TObjectPtr<USceneComponent> MovingVisual;

    UFUNCTION(BlueprintPure, Category="TMOP|Vertical Transport")
    bool Connects(FName FromAnchorId, FName ToAnchorId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Vertical Transport")
    bool RequestTransport(AActor* Passenger, FName FromAnchorId, FName ToAnchorId);

    UFUNCTION(BlueprintPure, Category="TMOP|Vertical Transport")
    bool IsTransporting(const AActor* Passenger) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Vertical Transport")
    static ATMOPVerticalTransport* FindTransport(UObject* WorldContextObject, FName FromAnchorId, FName ToAnchorId);

private:
    UPROPERTY(VisibleAnywhere)
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(Transient)
    TObjectPtr<AActor> ActivePassenger;

    FVector StartLocation = FVector::ZeroVector;
    FVector EndLocation = FVector::ZeroVector;
    float ElapsedSeconds = 0.0f;
    bool bTransporting = false;
};
