#include "People/TMOPPersonRegistrySubsystem.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Engine/DataTable.h"

void UTMOPPersonRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    ProfileTable = nullptr;
    ActiveAgents.Reset();
}

void UTMOPPersonRegistrySubsystem::Deinitialize()
{
    ActiveAgents.Reset();
    ProfileTable = nullptr;
    Super::Deinitialize();
}

bool UTMOPPersonRegistrySubsystem::ConfigureProfileTable(UDataTable* NewProfileTable)
{
    if (!IsValid(NewProfileTable) ||
        NewProfileTable->GetRowStruct() != FTMOPPersonProfileRow::StaticStruct()) return false;
    ProfileTable = NewProfileTable;
    return true;
}

bool UTMOPPersonRegistrySubsystem::GetPersonProfile(const FName EntityId,
    FTMOPPersonProfileRow& OutProfile) const
{
    if (!IsValid(ProfileTable) || EntityId.IsNone()) return false;
    const FTMOPPersonProfileRow* Row = ProfileTable->FindRow<FTMOPPersonProfileRow>(
        EntityId, TEXT("TMOP person lookup"), false);
    if (Row == nullptr) return false;
    OutProfile = *Row;
    return true;
}

bool UTMOPPersonRegistrySubsystem::DoesPersonExist(const FName EntityId) const
{
    if (!IsValid(ProfileTable) || EntityId.IsNone()) return false;
    return ProfileTable->GetRowMap().Contains(EntityId);
}

TArray<FName> UTMOPPersonRegistrySubsystem::GetAllPersonIds() const
{
    return IsValid(ProfileTable) ? ProfileTable->GetRowNames() : TArray<FName>();
}

bool UTMOPPersonRegistrySubsystem::RegisterActiveAgent(const FName EntityId,
    ATMOPHistoricalAgent* Agent)
{
    if (EntityId.IsNone() || !IsValid(Agent)) return false;
    if (TWeakObjectPtr<ATMOPHistoricalAgent>* Existing = ActiveAgents.Find(EntityId))
    {
        if (Existing->IsValid() && Existing->Get() != Agent)
        {
            UE_LOG(LogTemp, Error, TEXT("TMOP duplicate active agent EntityId '%s'."), *EntityId.ToString());
            return false;
        }
    }
    ActiveAgents.Add(EntityId, Agent);
    return true;
}

bool UTMOPPersonRegistrySubsystem::UnregisterActiveAgent(const FName EntityId,
    ATMOPHistoricalAgent* Agent)
{
    TWeakObjectPtr<ATMOPHistoricalAgent>* Existing = ActiveAgents.Find(EntityId);
    if (Existing == nullptr || Existing->Get() != Agent) return false;
    ActiveAgents.Remove(EntityId);
    return true;
}

ATMOPHistoricalAgent* UTMOPPersonRegistrySubsystem::FindActiveAgent(const FName EntityId) const
{
    const TWeakObjectPtr<ATMOPHistoricalAgent>* Found = ActiveAgents.Find(EntityId);
    return Found != nullptr ? Found->Get() : nullptr;
}

bool UTMOPPersonRegistrySubsystem::ValidateRegistry(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (!IsValid(ProfileTable))
    {
        OutErrors.Add(TEXT("No person profile DataTable is configured."));
        return false;
    }
    TSet<FName> EntityIds;
    for (const FName RowName : ProfileTable->GetRowNames())
    {
        const FTMOPPersonProfileRow* Row = ProfileTable->FindRow<FTMOPPersonProfileRow>(
            RowName, TEXT("TMOP registry validation"), false);
        if (Row == nullptr) { OutErrors.Add(FString::Printf(TEXT("Row '%s' is invalid."), *RowName.ToString())); continue; }
        if (Row->EntityId.IsNone()) OutErrors.Add(FString::Printf(TEXT("Row '%s' has no EntityId."), *RowName.ToString()));
        else if (Row->EntityId != RowName)
            OutErrors.Add(FString::Printf(TEXT("Row Name '%s' must equal EntityId '%s'."),
                *RowName.ToString(), *Row->EntityId.ToString()));
        if (EntityIds.Contains(Row->EntityId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate EntityId '%s'."), *Row->EntityId.ToString()));
        EntityIds.Add(Row->EntityId);
    }
    return OutErrors.IsEmpty();
}
