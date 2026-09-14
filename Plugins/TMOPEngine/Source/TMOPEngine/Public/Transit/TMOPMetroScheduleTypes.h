#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Time/TMOPTime.h"
#include "TMOPMetroScheduleTypes.generated.h"

/** One documented train arrival at one station. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPMetroArrivalRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FName StationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FText StationName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FName Direction = TEXT("Northbound");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    int32 Line = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FText Destination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FTMOPTime ArrivalTime;

    /** False means that the arrival occurs just after the 23:45 loop end. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    bool bWithinSimulationWindow = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Metro")
    FString SourceNote;
};

