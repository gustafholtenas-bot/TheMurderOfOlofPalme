#include "TMOPAddressEditorLibrary.h"

#include "Addresses/TMOPAddressComponent.h"
#include "Addresses/TMOPAddressRegistryTypes.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "ScopedTransaction.h"

FString UTMOPAddressEditorLibrary::ReplaceAddressRegistryJson(UDataTable* Registry,
    const FString& Json, const bool bDryRun)
{
    if (!GEditor || GEditor->PlayWorld)
        return TEXT("Öppna banan i editorn och stoppa Play före import.");
    if (!IsValid(Registry) || Registry->GetRowStruct() != FTMOPAddressRegistryRow::StaticStruct())
        return TEXT("Fel tabell eller radtyp: välj TMOPAddressRegistryRow.");
    if (Json.TrimStartAndEnd().IsEmpty()) return TEXT("Adressfilen är tom.");

    UDataTable* Temp = NewObject<UDataTable>(GetTransientPackage());
    Temp->RowStruct = FTMOPAddressRegistryRow::StaticStruct();
    Temp->bIgnoreMissingFields = true;
    const TArray<FString> Errors = Temp->CreateTableFromJSONString(Json);
    if (!Errors.IsEmpty() || Temp->GetRowNames().IsEmpty())
        return TEXT("Adressimporten är ogiltig: ") + FString::Join(Errors, TEXT("; "));

    TSet<FName> AddressIds;
    for (const FName Name : Temp->GetRowNames())
    {
        const auto* Row = Temp->FindRow<FTMOPAddressRegistryRow>(Name, TEXT("Validate address import"));
        if (!Row || Row->AddressId.IsNone()) return TEXT("En adressrad saknar AddressId.");
        if (AddressIds.Contains(Row->AddressId))
            return TEXT("Flera adressrader delar AddressId: ") + Row->AddressId.ToString();
        AddressIds.Add(Row->AddressId);
    }
    if (bDryRun) return FString();

    const FScopedTransaction Transaction(
        NSLOCTEXT("TMOP", "ReplaceAddressRegistryJson", "Uppdatera TMOP-adressregister"));
    Registry->Modify();
    Registry->EmptyTable();
    for (const FName Name : Temp->GetRowNames())
        Registry->AddRow(Name, *Temp->FindRow<FTMOPAddressRegistryRow>(Name, TEXT("Copy address import")));
    Registry->MarkPackageDirty();
    return FString();
}

FString UTMOPAddressEditorLibrary::BindAddressAnchor(ATMOPHistoricalAnchor* Anchor,
    UDataTable* Registry, FName RowName, bool bDryRun)
{
    if (!GEditor || GEditor->PlayWorld || !IsValid(Anchor) || !Anchor->GetWorld() ||
        Anchor->GetWorld()->WorldType != EWorldType::Editor)
        return TEXT("Öppna banan i editorn och stoppa Play före koppling.");
    if (!IsValid(Registry) || Registry->GetRowStruct() != FTMOPAddressRegistryRow::StaticStruct())
        return TEXT("Fel tabell eller radtyp: välj TMOPAddressRegistryRow.");
    auto* Row = Registry->FindRow<FTMOPAddressRegistryRow>(RowName, TEXT("Bind address"), false);
    if (!Row || Row->AddressId.IsNone()) return TEXT("Adressraden saknas eller saknar AddressId.");
    const FName AnchorId = Anchor->GetAnchorId();
    if (AnchorId.IsNone()) return TEXT("Ankaret saknar ett stabilt AnchorId.");
    if (!Row->EntranceAnchorId.IsNone() && Row->EntranceAnchorId != AnchorId)
        return TEXT("Adressen har redan ett annat EntranceAnchorId.");
    if (Row->EntranceAnchorId.IsNone() && !Row->BuildingAnchorId.IsNone() &&
        Row->BuildingAnchorId != AnchorId)
        return TEXT("Adressen har ett annat explicit BuildingAnchorId.");

    TArray<UTMOPAddressComponent*> Components;
    Anchor->GetComponents<UTMOPAddressComponent>(Components);
    if (Components.Num() > 1) return TEXT("Ankaret har flera adresskomponenter; rätta dubbletten först.");
    UTMOPAddressComponent* Existing = Components.IsEmpty() ? nullptr : Components[0];
    if (Existing && ((Existing->Registry && Existing->Registry != Registry) ||
        (!Existing->RowName.IsNone() && Existing->RowName != RowName)))
        return TEXT("Ankaret har redan en annan adresskoppling.");
    for (TActorIterator<AActor> It(Anchor->GetWorld()); It; ++It)
    {
        if (*It == Anchor) continue;
        if (auto* OtherAnchor = Cast<ATMOPHistoricalAnchor>(*It))
            if (OtherAnchor->GetAnchorId() == AnchorId)
                return TEXT("Flera ankare delar samma AnchorId.");
        TArray<UTMOPAddressComponent*> Others;
        It->GetComponents<UTMOPAddressComponent>(Others);
        for (const auto* Other : Others)
            if (Other->Registry == Registry && Other->RowName == RowName)
                return TEXT("Adressen har redan en komponent på en annan actor.");
    }
    for (FName OtherName : Registry->GetRowNames())
    {
        if (OtherName == RowName) continue;
        const auto* Other = Registry->FindRow<FTMOPAddressRegistryRow>(OtherName, TEXT("Check address links"));
        if (Other->AddressId == Row->AddressId)
            return TEXT("Flera tabellrader delar samma AddressId.");
        if (Other->EntranceAnchorId == AnchorId || Other->BuildingAnchorId == AnchorId)
            return TEXT("Ankaret används redan av en annan adressrad.");
    }
    if (bDryRun) return FString();
    // Re-running preserves hand-adjusted interaction offsets and adds no duplicates.
    if (Existing && Existing->Registry == Registry && Existing->RowName == RowName &&
        Row->EntranceAnchorId == AnchorId && Anchor->GetActorEnableCollision()) return FString();
    const FScopedTransaction Transaction(NSLOCTEXT("TMOP", "BindAddressAnchor", "Koppla adressankare"));
    Anchor->Modify();
    Registry->Modify();
    // Actors with collision globally disabled cannot be found by the runtime query shape.
    Anchor->SetActorEnableCollision(true);
    if (!Existing)
    {
        Existing = NewObject<UTMOPAddressComponent>(Anchor, NAME_None, RF_Transactional);
        Anchor->AddInstanceComponent(Existing);
        Existing->RegisterComponent();
    }
    Existing->SetFlags(RF_Transactional);
    Existing->Modify();
    Existing->Registry = Registry;
    Existing->RowName = RowName;
    Row->EntranceAnchorId = AnchorId;
    Anchor->MarkPackageDirty();
    Registry->MarkPackageDirty();
    return FString();
}
