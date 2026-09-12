#include "UI/TMOPSaveGameService.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerVehicleSessionComponent.h"
#include "Radio/TMOPPlayerRadioComponent.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Anchors/TMOPHistoricalPlace.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
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
    if (UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(Player) != 0)
    {
        OutStatus = NSLOCTEXT("TMOP", "SavePartyLeader", "Spelare 1 sparar och laddar för hela gruppen.");
        return false;
    }
    const auto Party = UTMOPLocalMultiplayerSubsystem::GetPlayers(World);
    for (ATMOPPlayerCharacter* Member : Party)
        if (Member->VehicleSession && Member->VehicleSession->IsInVehicle())
        {
            OutStatus = NSLOCTEXT("TMOP", "SaveExitVehicles", "Kliv ur fordonen före sparning. Spelarstyrda fordonslägen ingår ännu inte i sparformatet.");
            return false;
        }
    UTMOPMenuSaveGame* Save = Cast<UTMOPMenuSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UTMOPMenuSaveGame::StaticClass()));
    if (!IsValid(Save)) return false;
    Save->SaveFormatVersion = 3;
    auto* Session = Player->GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
    Save->bKeyboardForPlayerOne = Session->UsesKeyboardForPlayerOne();
    Save->bSharedKeyboardForPlayerTwo = Session->UsesSharedKeyboardForPlayerTwo();
    for (ATMOPPlayerCharacter* Member : Party)
    {
        auto& State = Save->LocalPlayers.AddDefaulted_GetRef();
        State.Transform = Member->GetActorTransform();
        if (const auto* PC = Cast<APlayerController>(Member->GetController()))
            State.ViewRotation = PC->GetControlRotation();
        State.DiscoveredEvidenceIds = Member->DiscoveredEvidenceIds;
        if (Member->Inventory)
        {
            for (const FTMOPInventoryEntry& Entry : Member->Inventory->Items)
                if (IsValid(Entry.Item))
                {
                    State.InventoryItemPaths.Add(FSoftObjectPath(Entry.Item->GetPathName()));
                    State.InventoryQuantities.Add(Entry.Quantity);
                }
            if (Member->Inventory->EquippedItem)
                State.EquippedItemPath = FSoftObjectPath(Member->Inventory->EquippedItem->GetPathName());
        }
        if (Member->Radio)
        {
            State.RadioChannelId = Member->Radio->GetCurrentChannel().ChannelId;
            State.bRadioOn = Member->Radio->bRadioOn;
        }
    }
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
    if (UTMOPLocalMultiplayerSubsystem::GetPlayerSlot(Player) != 0)
    {
        OutStatus = NSLOCTEXT("TMOP", "LoadPartyLeader", "Spelare 1 sparar och laddar för hela gruppen.");
        return false;
    }
    if (Save->SaveFormatVersion > 3 || (Save->SaveFormatVersion >= 3 &&
        (Save->LocalPlayers.IsEmpty() || Save->LocalPlayers.Num() > 4)))
    {
        OutStatus = NSLOCTEXT("TMOP", "BadLocalSave", "Sparfilens spelarantal eller format stöds inte.");
        return false;
    }
    if (!Save->MapDisplayName.IsEmpty() &&
        Save->MapDisplayName != UGameplayStatics::GetCurrentLevelName(World, true))
    {
        OutStatus = NSLOCTEXT("TMOP", "SaveWrongMap", "Öppna nivån som sparningen skapades i innan du laddar den.");
        return false;
    }
    ATMOPSimulationDebugDirector* DebugDirector = nullptr;
    for (TActorIterator<ATMOPSimulationDebugDirector> It(World); It; ++It) { DebugDirector = *It; break; }
    if (!DebugDirector)
    {
        OutStatus = NSLOCTEXT("TMOP", "PartyLoadDirector", "Laddning kräver TMOPSimulationDebugDirector i nivån.");
        return false;
    }
    auto* Session = Player->GetGameInstance()->GetSubsystem<UTMOPLocalMultiplayerSubsystem>();
    const int32 Count = Save->SaveFormatVersion >= 3 ? Save->LocalPlayers.Num() : 1;
    auto* Clock = Player->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>();
    const int32 SavedSecond = Save->SavedTime.ToSecondsFromMidnight();
    bool bValidState = Clock && Session && SavedSecond >= Clock->GetLoopStartTime().ToSecondsFromMidnight()
        && SavedSecond <= Clock->GetLoopEndTime().ToSecondsFromMidnight();
    if (Save->SaveFormatVersion >= 3)
        for (const auto& State : Save->LocalPlayers)
            bValidState &= !State.Transform.ContainsNaN() && !State.ViewRotation.ContainsNaN();
    else bValidState &= !Save->PlayerTransform.ContainsNaN();
    if (!bValidState)
    {
        OutStatus = NSLOCTEXT("TMOP", "BadPartySaveState", "Sparfilen innehåller en ogiltig tid eller position.");
        return false;
    }
    Clock->RequestPause(Session, TEXT("LoadGame"));
    // Detach everyone before the historical vehicle director reconstructs the world.
    for (ATMOPPlayerCharacter* Member : UTMOPLocalMultiplayerSubsystem::GetPlayers(World))
        if (Member->VehicleSession) Member->VehicleSession->ExitVehicle();
    Session->ConfigureSession(Count, Save->bKeyboardForPlayerOne,
        Save->bSharedKeyboardForPlayerTwo);
    if (!Session->EnsurePlayerCount(Count, OutStatus))
    {
        Clock->ReleasePause(Session, TEXT("LoadGame"));
        return false;
    }
    const bool bTimeLoaded = DebugDirector->JumpToSimulationTime(Save->SavedTime);
    if (!bTimeLoaded)
    {
        OutStatus = NSLOCTEXT("TMOP", "LoadNeedsDirector",
            "Laddning kräver TMOPSimulationDebugDirector i nivån.");
        Clock->ReleasePause(Session, TEXT("LoadGame"));
        return false;
    }
    Session->CloseAllPlayerMenus();
    if (Save->SaveFormatVersion >= 3)
    {
        const auto Members = UTMOPLocalMultiplayerSubsystem::GetPlayers(World);
        for (int32 Slot = 0; Slot < Members.Num(); ++Slot)
        {
            ATMOPPlayerCharacter* Member = Members[Slot];
            const auto& State = Save->LocalPlayers[Slot];
            Member->SetActorTransform(State.Transform, false, nullptr, ETeleportType::TeleportPhysics);
            Member->GetCharacterMovement()->StopMovementImmediately();
            if (auto* PC = Cast<APlayerController>(Member->GetController()))
            {
                PC->SetControlRotation(State.ViewRotation);
                PC->SetViewTarget(Member);
            }
            Member->DiscoveredEvidenceIds = State.DiscoveredEvidenceIds;
            if (Member->Inventory)
            {
                const auto Existing = Member->Inventory->Items;
                for (const FTMOPInventoryEntry& Entry : Existing)
                    if (Entry.Item) Member->Inventory->RemoveItem(Entry.Item, Entry.Quantity);
                for (int32 Index = 0; Index < State.InventoryItemPaths.Num(); ++Index)
                    if (auto* Item = Cast<UTMOPItemDefinition>(State.InventoryItemPaths[Index].TryLoad()))
                        Member->Inventory->AddItem(Item, State.InventoryQuantities.IsValidIndex(Index)
                            ? FMath::Max(1, State.InventoryQuantities[Index]) : 1);
                if (auto* Item = Cast<UTMOPItemDefinition>(State.EquippedItemPath.TryLoad())) Member->Inventory->EquipItem(Item);
            }
            if (Member->Radio)
            {
                Member->Radio->SetSimulationSecondOfDay(Save->SavedTime.ToSecondsFromMidnight());
                Member->Radio->SetChannelById(State.RadioChannelId);
                Member->Radio->SetRadioOn(State.bRadioOn);
            }
        }
        Session->AdoptLoadedSession();
        Clock->ReleasePause(Session, TEXT("LoadGame"));
        Clock->StartClock();
        OutStatus = NSLOCTEXT("TMOP", "PartyLoadSuccess", "Hela den lokala spelomgången laddades.");
        return true;
    }
    Player->SetActorTransform(Save->PlayerTransform, false, nullptr,
        ETeleportType::TeleportPhysics);
    Player->GetCharacterMovement()->StopMovementImmediately();
    if (auto* PC = Cast<APlayerController>(Player->GetController()))
    {
        PC->SetControlRotation(Save->PlayerTransform.Rotator());
        PC->SetViewTarget(Player);
    }
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
    Session->AdoptLoadedSession();
    Clock->ReleasePause(Session, TEXT("LoadGame"));
    Clock->StartClock();
    OutStatus = NSLOCTEXT("TMOP", "LoadSuccess", "Spelet laddades.");
    return true;
}
