#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPKillerBranchPoint.generated.h"

class UBillboardComponent;

/** Authored corner of the killer's possible escape network. Place at ground level. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPKillerBranchPoint : public AActor
{
    GENERATED_BODY()
public:
    ATMOPKillerBranchPoint();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer")
    FName NetworkId = TEXT("KillerEscape");

    /** Adjacent corners only. Navigation supplies the walkable path between them. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer")
    TArray<TObjectPtr<ATMOPKillerBranchPoint>> NextCorners;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Original")
    bool bBranchWhenOriginalPasses = false;

    /** These directions are excluded when spawning alternatives from the original. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Original")
    TObjectPtr<ATMOPKillerBranchPoint> OriginalNextCorner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Original")
    TObjectPtr<ATMOPKillerBranchPoint> OriginalPreviousCorner;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Original",
        meta=(ClampMin="10", ClampMax="300", Units="cm"))
    float ArrivalRadiusCm = 90.0f;

    /** Optional authored arrival time, supporting direct seeks before the original was observed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Time")
    bool bUseSharedEventArrival = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Time",
        meta=(EditCondition="bUseSharedEventArrival"))
    FName ArrivalEventId = TEXT("Palme_shot_1");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Killer|Time",
        meta=(EditCondition="bUseSharedEventArrival", Units="s"))
    float ArrivalEventOffsetSeconds = 0.0f;

    UPROPERTY(VisibleAnywhere, Category="TMOP|Killer")
    TObjectPtr<UBillboardComponent> EditorIcon;
};
