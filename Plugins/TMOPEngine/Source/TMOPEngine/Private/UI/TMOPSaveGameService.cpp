#include "UI/TMOPSaveGameService.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Anchors/TMOPHistoricalPlace.h"
#include "EngineUtils.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Kismet/GameplayStatics.h"
#include "Player/TMOPPlayerCharacter.h"
#include "Time/TMOPClockSubsystem.h"
#include "Time/TMOPSimulationDebugDirector.h"

namespace
{
FString ResolveLocationName(UWorld* World, const FVector& PlayerLocation)
{
    if (!IsValid(World)) return TEXT("Okänd plats");
    ATMOPHistoricalAnchor* Nearest = nullptr;
    double NearestDistanceSquared = TNumericLimits<double>::Max();
    for (TActorIterator<ATMOPHistoricalAnchor> It(World); It; ++It)
    {
        if (!IsValid(*It) || !It->bShowOnMap) continue;
        const double DistanceSquared = FVector::DistSquared2D(
            PlayerLocation, It->GetActorLocation());
        if (DistanceSquared < NearestDistanceSquared)
        {
            NearestDistanceSquared = DistanceSquared;
            Nearest = *It;
        }
    }
    if (!IsValid(Nearest)) return TEXT("Okänd plats");
    if (!Nearest->ParentPlaceId.IsNone())
        if (UGameInstance* GameInstance = World->GetGameInstance())
            if (UTMOPAnchorSubsystem* Anchors =
                GameInstance->GetSubsystem<UTMOPAnchorSubsystem>())
                if (ATMOPHistoricalPlace* Place =
                    Anchors->FindPlace(Nearest->ParentPlaceId))
                    if (!Place->DisplayName.IsEmpty())
                        return Place->DisplayName.ToString();
    return !Nearest->DisplayName.IsEmpty()
        ? Nearest->DisplayName.ToString() : Nearest->GetAnchorId().ToString();
}

FString FriendlySavedAt(const FString& Iso8601)
{
    FDateTime DateTime;
    return FDateTime::ParseIso8601(*Iso8601, DateTime)
        ? DateTime.ToString(TEXT("%Y-%m-%d  %H:%M")) : Iso8601;
}

FTMOPSaveSlotInfo MakeSlotInfo(const FString& SlotName,
    const UTMOPMenuSaveGame& Save)
{
    FTMOPSaveSlotInfo Info;
    Info.SlotName = SlotName;
    Info.DisplayName = Save.SlotDisplayName.IsEmpty()
        ? (SlotName == TEXT("TMOP_QuickSave") ? TEXT("Äldre quicksave") : SlotName)
        : Save.SlotDisplayName;
    Info.LocationName = Save.LocationDisplayName.IsEmpty()
        ? TEXT("Okänd plats") : Save.LocationDisplayName;
    Info.MapName = Save.MapDisplayName;
    Info.SavedAtText = FriendlySavedAt(Save.SavedAtIso8601);
    Info.GameTime = Save.SavedTime;
    Info.SaveKind = Save.SaveKind;
    Info.bLegacySave = Save.SaveFormatVersion < 2;
    return Info;
}
}

FString FTMOPSaveGameService::MakeManualSlotName(
    const FString& Prefix, const int32 Index)
{
    return FString::Printf(TEXT("%s%02d"), *Prefix, Index);
}

FString FTMOPSaveGameService::FindFirstFreeManualSlot(
    const FString& Prefix, const int32 SlotCount)
{
    for (int32 Index = 1; Index <= FMath::Max(1, SlotCount); ++Index)
    {
        const FString Slot = MakeManualSlotName(Prefix, Index);
        if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) return Slot;
    }
    return FString();
}

TArray<FTMOPSaveSlotInfo> FTMOPSaveGameService::FindSaveSlots(
    const FString& ManualPrefix, const int32 ManualSlotCount,
    const FString& LegacyQuickSlot)
{
    TArray<FTMOPSaveSlotInfo> Result;
    auto AddSlot = [&Result](const FString& Slot)
    {
        if (!UGameplayStatics::DoesSaveGameExist(Slot, 0)) return;
        if (UTMOPMenuSaveGame* Save = Cast<UTMOPMenuSaveGame>(
            UGameplayStatics::LoadGameFromSlot(Slot, 0)))
            Result.Add(MakeSlotInfo(Slot, *Save));
    };
    for (int32 Index = 1; Index <= FMath::Max(1, ManualSlotCount); ++Index)
        AddSlot(MakeManualSlotName(ManualPrefix, Index));
    if (!LegacyQuickSlot.IsEmpty() &&
        !LegacyQuickSlot.StartsWith(ManualPrefix)) AddSlot(LegacyQuickSlot);
    Result.Sort([](const FTMOPSaveSlotInfo& Left, const FTMOPSaveSlotInfo& Right)
    {
        return Left.SavedAtText > Right.SavedAtText;
    });
    return Result;
}

bool FTMOPSaveGameService::SavePlayer(UWorld* World,
    ATMOPPlayerCharacter* Player, const FString& SlotName,
    const FString& DisplayName, const ETMOPMenuSaveKind SaveKind,
    FText& OutStatus)
{
    if (!IsValid(World) || !IsValid(Player) || SlotName.IsEmpty())
    {
        OutStatus = NSLOCTEXT("TMOP", "SaveInvalidTarget", "Kunde inte skapa sparningen.");
        return false;
    }
    UTMOPMenuSaveGame* Save = Cast<UTMOPMenuSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UTMOPMenuSaveGame::StaticClass()));
    if (!IsValid(Save)) return false;
    Save->SaveFormatVersion = 2;
    Save->SlotDisplayName = DisplayName;
    Save->LocationDisplayName = ResolveLocationName(World, Player->GetActorLocation());
    Save->MapDisplayName = UGameplayStatics::GetCurrentLevelName(World, true);
    Save->SavedAtIso8601 = FDateTime::Now().ToIso8601();
    Save->SaveKind = SaveKind;
    Save->PlayerTransform = Player->GetActorTransform();
    if (UTMOPClockSubsystem* Clock = Player->GetGameInstance()
        ? Player->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr)
        Save->SavedTime = Clock->GetCurrentTime();
    if (IsValid(Player->Inventory))
    {
        for (const FTMOPInventoryEntry& Entry : Player->Inventory->Items)
            if (IsValid(Entry.Item))
            {
                Save->InventoryItemPaths.Add(FSoftObjectPath(Entry.Item->GetPathName()));
                Save->InventoryQuantities.Add(Entry.Quantity);
            }
        if (IsValid(Player->Inventory->EquippedItem))
            Save->EquippedItemPath =
                FSoftObjectPath(Player->Inventory->EquippedItem->GetPathName());
    }
    Save->DiscoveredEvidenceIds = Player->DiscoveredEvidenceIds;
    const bool bSaved = UGameplayStatics::SaveGameToSlot(Save, SlotName, 0);
    OutStatus = bSaved
        ? NSLOCTEXT("TMOP", "SaveSuccess", "Spelet sparades.")
        : NSLOCTEXT("TMOP", "SaveFailed", "Kunde inte spara spelet.");
    return bSaved;
}

bool FTMOPSaveGameService::LoadPlayer(UWorld* World,
    ATMOPPlayerCharacter* Player, const FString& SlotName, FText& OutStatus)
{
    UTMOPMenuSaveGame* Save = Cast<UTMOPMenuSaveGame>(
        UGameplayStatics::LoadGameFromSlot(SlotName, 0));
    if (!IsValid(Save) || !IsValid(Player) || !IsValid(World))
    {
        OutStatus = NSLOCTEXT("TMOP", "NoSave", "Ingen giltig sparfil hittades.");
        return false;
    }
    bool bTimeLoaded = false;
    for (TActorIterator<ATMOPSimulationDebugDirector> It(World); It; ++It)
    {
        bTimeLoaded = It->JumpToSimulationTime(Save->SavedTime);
        break;
    }
    if (!bTimeLoaded)
    {
        OutStatus = NSLOCTEXT("TMOP", "LoadNeedsDirector",
            "Laddning kräver TMOPSimulationDebugDirector i nivån.");
        return false;
    }
    Player->SetActorTransform(Save->PlayerTransform, false, nullptr,
        ETeleportType::TeleportPhysics);
    if (IsValid(Player->Inventory))
    {
        const TArray<FTMOPInventoryEntry> Existing = Player->Inventory->Items;
        for (const FTMOPInventoryEntry& Entry : Existing)
            if (IsValid(Entry.Item))
                Player->Inventory->RemoveItem(Entry.Item, Entry.Quantity);
        for (int32 Index = 0; Index < Save->InventoryItemPaths.Num(); ++Index)
            if (UTMOPItemDefinition* Item = Cast<UTMOPItemDefinition>(
                Save->InventoryItemPaths[Index].TryLoad()))
                Player->Inventory->AddItem(Item,
                    Save->InventoryQuantities.IsValidIndex(Index)
                        ? Save->InventoryQuantities[Index] : 1);
        if (UTMOPItemDefinition* Equipped = Cast<UTMOPItemDefinition>(
            Save->EquippedItemPath.TryLoad()))
            Player->Inventory->EquipItem(Equipped);
    }
    Player->DiscoveredEvidenceIds = Save->DiscoveredEvidenceIds;
    OutStatus = NSLOCTEXT("TMOP", "LoadSuccess", "Spelet laddades.");
    return true;
}
