#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TMOPAddressRegistryTypes.generated.h"

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAddressResident
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString ResidentId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString ArchivalFullName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString InGameDisplayName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    bool bAllowArchivalNameInGame = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString BirthDateIso;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString BirthPlace;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString RelationshipToPrimaryResident;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString RelationshipConfidence;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAddressHousehold
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString HouseholdId;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString FloorLabel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    int32 FloorNumber = -1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString ApartmentLabel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString DoorbellLabel;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString DoorbellLabelConfidence;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName DoorAnchorId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    TArray<FTMOPAddressResident> Residents;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString Notes;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    bool bConfirmedFamily = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString FamilySurname;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAddressRegistryRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName AddressId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString StreetName;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    int32 StreetNumber = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString EntranceSuffix;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString RegistrySearchText;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString RegistryAddressQualifier;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString City;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName BuildingAnchorId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName EntranceAnchorId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName DoorbellActorTag = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString SourceTitle;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString SourceSnapshotDateIso;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString SourceReference;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    TArray<FTMOPAddressHousehold> Households;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FString Notes;
};

namespace TMOPAddressDisplay
{
    TMOPENGINE_API FString Resident(const FTMOPAddressResident& Person);
    TMOPENGINE_API FString Household(const FTMOPAddressHousehold& Home);
    TMOPENGINE_API FString Directory(const FTMOPAddressRegistryRow& Row);
}

