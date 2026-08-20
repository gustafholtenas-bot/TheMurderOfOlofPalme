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
