#pragma once

#include "CoreMinimal.h"
#include "UI/TMOPMenuSaveGame.h"

class ATMOPPlayerCharacter;
class UWorld;

/** Lightweight information used to build save/load lists without exposing payload details. */
struct TMOPENGINE_API FTMOPSaveSlotInfo
{
    FString SlotName;
    FString DisplayName;
    FString LocationName;
    FString MapName;
    FString SavedAtText;
    FTMOPTime GameTime;
    ETMOPMenuSaveKind SaveKind = ETMOPMenuSaveKind::Manual;
    bool bLegacySave = false;
};

/** Shared save operations used by both the main menu and pause menu. */
class TMOPENGINE_API FTMOPSaveGameService
{
public:
    static FString MakeManualSlotName(const FString& Prefix, int32 Index);
    static FString FindFirstFreeManualSlot(const FString& Prefix, int32 SlotCount);
    static TArray<FTMOPSaveSlotInfo> FindSaveSlots(
        const FString& ManualPrefix, int32 ManualSlotCount,
        const FString& LegacyQuickSlot);
    static bool SavePlayer(UWorld* World, ATMOPPlayerCharacter* Player,
        const FString& SlotName, const FString& DisplayName,
        ETMOPMenuSaveKind SaveKind, FText& OutStatus);
    static bool LoadPlayer(UWorld* World, ATMOPPlayerCharacter* Player,
        const FString& SlotName, FText& OutStatus);
};
