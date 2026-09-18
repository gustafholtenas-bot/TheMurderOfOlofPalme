#pragma once

#include "CoreMinimal.h"
#include "Observations/TMOPNotebookTypes.h"
#include "Research/TMOPTheoryTypes.h"
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

USTRUCT()
struct FTMOPLocalPlayerSave
{
    GENERATED_BODY()
    UPROPERTY(SaveGame) FTransform Transform = FTransform::Identity;
    UPROPERTY(SaveGame) FRotator ViewRotation = FRotator::ZeroRotator;
    UPROPERTY(SaveGame) TArray<FSoftObjectPath> InventoryItemPaths;
    UPROPERTY(SaveGame) TArray<int32> InventoryQuantities;
    UPROPERTY(SaveGame) FSoftObjectPath EquippedItemPath;
    UPROPERTY(SaveGame) TArray<FName> DiscoveredEvidenceIds;
    UPROPERTY(SaveGame) TArray<FTMOPNotebookObservation> NotebookObservations;
    UPROPERTY(SaveGame) TArray<FTMOPTheoryTree> TheoryTrees;
    UPROPERTY(SaveGame) FGuid ActiveTheoryTreeId;
    UPROPERTY(SaveGame) FName RadioChannelId;
    UPROPERTY(SaveGame) bool bRadioOn = false;
};

/** Compact player/menu save. The simulation world is reconstructed at SavedTime. */
UCLASS()
class TMOPENGINE_API UTMOPMenuSaveGame : public USaveGame
{
    GENERATED_BODY()
public:
    /** Increment when the serialized payload changes incompatibly. */
    UPROPERTY(SaveGame) int32 SaveFormatVersion = 1;
    /** Version 3 adds all local players; legacy fields below remain readable. */
    UPROPERTY(SaveGame) TArray<FTMOPLocalPlayerSave> LocalPlayers;
    UPROPERTY(SaveGame) bool bKeyboardForPlayerOne = true;
    UPROPERTY(SaveGame) bool bSharedKeyboardForPlayerTwo = false;
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
    UPROPERTY(SaveGame) TArray<FTMOPNotebookObservation> NotebookObservations;
    UPROPERTY(SaveGame) TArray<FTMOPTheoryTree> TheoryTrees;
    UPROPERTY(SaveGame) FGuid ActiveTheoryTreeId;
};
