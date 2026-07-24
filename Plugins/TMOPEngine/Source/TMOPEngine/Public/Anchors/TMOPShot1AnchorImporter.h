#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPShot1AnchorImporter.generated.h"

class ATMOPHistoricalAnchor;

/** Imports TMOP_SHOT1_WITNESS_ANCHORS.json as persistent level actors. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPShot1AnchorImporter : public AActor
{
    GENERATED_BODY()

public:
    ATMOPShot1AnchorImporter();

    /**
     * Path relative to the project's Content directory. An absolute path is
     * also accepted.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Shot1 Anchor Import")
    FString JsonFilePath =
        TEXT("TMOP/Data/TMOP_SHOT1_WITNESS_ANCHORS.json");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Shot1 Anchor Import")
    TSubclassOf<ATMOPHistoricalAnchor> AnchorClass;

    /** Update an existing anchor when its ID already exists in the level. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Shot1 Anchor Import")
    bool bUpdateExistingAnchors = true;

    /** Exact witness points should normally not be projected or spread. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Shot1 Anchor Import")
    bool bUseExactPositions = true;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Shot1 Anchor Import")
    int32 LastCreatedCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Shot1 Anchor Import")
    int32 LastUpdatedCount = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Shot1 Anchor Import")
    int32 LastErrorCount = 0;

    /** Creates or updates all anchors and leaves them as normal level actors. */
    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Shot1 Anchor Import")
    void ImportOrUpdateShot1Anchors();

    UFUNCTION(BlueprintPure, Category="TMOP|Shot1 Anchor Import")
    FString GetResolvedJsonPath() const;

private:
    ATMOPHistoricalAnchor* FindExistingAnchor(FName AnchorId) const;
};
