#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Time/TMOPTime.h"
#include "TMOPMenuSaveGame.generated.h"

/** Compact player/menu save. The simulation world is reconstructed at SavedTime. */
UCLASS()
class TMOPENGINE_API UTMOPMenuSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    UPROPERTY(SaveGame) FTransform PlayerTransform = FTransform::Identity;
    UPROPERTY(SaveGame) FTMOPTime SavedTime = FTMOPTime(23, 0, 0);
    UPROPERTY(SaveGame) TArray<FSoftObjectPath> InventoryItemPaths;
    UPROPERTY(SaveGame) TArray<int32> InventoryQuantities;
    UPROPERTY(SaveGame) FSoftObjectPath EquippedItemPath;
    UPROPERTY(SaveGame) TArray<FName> DiscoveredEvidenceIds;
};
