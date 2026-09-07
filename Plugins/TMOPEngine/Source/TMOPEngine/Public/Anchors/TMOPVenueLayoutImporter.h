#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPVenueLayoutImporter.generated.h"

class ATMOPHistoricalAnchor;

/** Creates reusable venue-position anchors relative to existing *_inside anchors. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPVenueLayoutImporter : public AActor
{
    GENERATED_BODY()

public:
    ATMOPVenueLayoutImporter();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import")
    FString JsonFilePath = TEXT("TMOP/Data/TMOP_VENUE_LAYOUT_ANCHORS.json");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import")
    TSubclassOf<ATMOPHistoricalAnchor> AnchorClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import")
    bool bUpdateExistingAnchors = true;

    /** Creates real reservable seats for TableSeat and BarSeat layout rows. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import|Seats")
    bool bCreateSeatComponents = true;

    /** Preserve hand-adjusted seat components on subsequent imports. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import|Seats")
    bool bUpdateExistingSeatAlignment = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import|Seats", meta=(Units="cm"))
    float ChairSeatHeightCm = 37.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Venue Layout Import|Seats", meta=(Units="cm"))
    float BarSeatHeightCm = 91.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Venue Layout Import")
    int32 LastCreatedCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Venue Layout Import")
    int32 LastUpdatedCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Venue Layout Import")
    int32 LastErrorCount = 0;

    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Venue Layout Import")
    void ImportOrUpdateVenueLayoutAnchors();

    UFUNCTION(BlueprintPure, Category="TMOP|Venue Layout Import")
    FString GetResolvedJsonPath() const;

private:
    ATMOPHistoricalAnchor* FindExistingAnchor(FName AnchorId) const;
};
