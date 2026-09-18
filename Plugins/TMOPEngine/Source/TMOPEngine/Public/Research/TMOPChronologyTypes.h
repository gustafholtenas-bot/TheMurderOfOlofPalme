#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TMOPChronologyTypes.generated.h"

/** Importable editorial chronology. Zero date parts mean unknown, not January 1. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPChronologyRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology", meta=(MultiLine="true"))
    FText Body;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    int32 Year = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology", meta=(ClampMin="0", ClampMax="12"))
    int32 Month = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology", meta=(ClampMin="0", ClampMax="31"))
    int32 Day = 0;
    /** Knowledge page: before/after the murder refers to the claim's event, not publication. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    bool bBeforeMurder = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    int32 SortOrder = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    FText DateNote;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    FText EvidenceStatus;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology", meta=(MultiLine="true"))
    FText Source;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    FString SourceUrl;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Chronology")
    bool bPublished = true;
};
