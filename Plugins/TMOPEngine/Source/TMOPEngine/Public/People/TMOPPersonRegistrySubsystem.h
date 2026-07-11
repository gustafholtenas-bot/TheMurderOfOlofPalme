#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPPersonRegistrySubsystem.generated.h"

class ATMOPHistoricalAgent;
class UDataTable;

UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPPersonRegistrySubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    bool ConfigureProfileTable(UDataTable* NewProfileTable);

    UFUNCTION(BlueprintPure, Category="TMOP|People")
    UDataTable* GetProfileTable() const { return ProfileTable; }

    UFUNCTION(BlueprintPure, Category="TMOP|People")
    bool GetPersonProfile(FName EntityId, FTMOPPersonProfileRow& OutProfile) const;

    UFUNCTION(BlueprintPure, Category="TMOP|People")
    bool DoesPersonExist(FName EntityId) const;

    UFUNCTION(BlueprintPure, Category="TMOP|People")
    TArray<FName> GetAllPersonIds() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    bool RegisterActiveAgent(FName EntityId, ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    bool UnregisterActiveAgent(FName EntityId, ATMOPHistoricalAgent* Agent);

    UFUNCTION(BlueprintPure, Category="TMOP|People")
    ATMOPHistoricalAgent* FindActiveAgent(FName EntityId) const;

    UFUNCTION(BlueprintCallable, Category="TMOP|People")
    bool ValidateRegistry(TArray<FString>& OutErrors) const;

private:
    UPROPERTY(Transient)
    TObjectPtr<UDataTable> ProfileTable;

    TMap<FName, TWeakObjectPtr<ATMOPHistoricalAgent>> ActiveAgents;
};
