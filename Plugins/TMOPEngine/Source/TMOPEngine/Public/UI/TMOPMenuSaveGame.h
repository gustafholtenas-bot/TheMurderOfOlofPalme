#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Time/TMOPTime.h"
#include "TMOPMenuSaveGame.generated.h"

UENUM(BlueprintType)
enum class ETMOPMenuSaveKind : uint8
{
    Manual UMETA(DisplayName="Manual Save"),
    Quick UMETA(DisplayName="Quick Save"),
    Auto UMETA(DisplayName="Auto Save")
};

/** Compact player/menu save. The simulation world is reconstructed at SavedTime. */
UCLASS()
class TMOPENGINE_API UTMOPMenuSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    /** Increment when the serialized payload changes incompatibly. */
    UPROPERTY(SaveGame) int32 SaveFormatVersion = 1;
    UPROPERTY(SaveGame) FString SlotDisplayName;
    UPROPERTY(SaveGame) FString LocationDisplayName;
    UPROPERTY(SaveGame) FString MapDisplayName;
    UPROPERTY(SaveGame) FString SavedAtIso8601;
    UPROPERTY(SaveGame) ETMOPMenuSaveKind SaveKind = ETMOPMenuSaveKind::Manual;
    UPROPERTY(SaveGame) FTransform PlayerTransform = FTransform::Identity;
    UPROPERTY(SaveGame) FTMOPTime SavedTime = FTMOPTime(23, 0, 0);
    UPROPERTY(SaveGame) TArray<FSoftObjectPath> InventoryItemPaths;
    UPROPERTY(SaveGame) TArray<int32> InventoryQuantities;
    UPROPERTY(SaveGame) FSoftObjectPath EquippedItemPath;
    UPROPERTY(SaveGame) TArray<FName> DiscoveredEvidenceIds;
};
