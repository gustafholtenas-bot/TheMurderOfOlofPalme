#pragma once

#include "CoreMinimal.h"
#include "Inventory/TMOPItemDefinition.h"
#include "TMOPNewspaperItemDefinition.generated.h"

class UTexture2D;

UENUM(BlueprintType)
enum class ETMOPNewspaperPublication : uint8
{
    Arbetet,
    Aftonbladet,
    DagensNyheter,
    Expressen,
    SvenskaDagbladet,
    GoteborgsPosten,
    DagensIndustri,
    Other
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPNewspaperPage
{
    GENERATED_BODY()

    /** One scanned page. Soft references ensure that only opened pages consume memory. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper")
    TSoftObjectPtr<UTexture2D> PageImage;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper",
        meta=(ClampMin="1"))
    int32 PrintedPageNumber = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper")
    FText PageLabel;
};

/** Inventory item containing an ordered set of scanned newspaper pages. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPNewspaperItemDefinition : public UTMOPItemDefinition
{
    GENERATED_BODY()

public:
    UTMOPNewspaperItemDefinition();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper|Identity")
    ETMOPNewspaperPublication Publication = ETMOPNewspaperPublication::Other;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper|Identity")
    FText EditionName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper|Identity")
    FString PublicationDate = TEXT("1986-02-28");

    /** Pages in reading order, normally beginning with the front page. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper|Pages")
    TArray<FTMOPNewspaperPage> Pages;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper|Reading")
    bool bPauseSimulationWhileReading = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Newspaper|Reading",
        meta=(ClampMin="0.25", ClampMax="4.0"))
    float InitialZoom = 1.0f;

};
