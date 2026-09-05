#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPAddressComponent.generated.h"

class UDataTable;
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPAddressComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTMOPAddressComponent();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    TObjectPtr<UDataTable> Registry;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName RowName;
    // Use this function from an interaction widget; no archival birth details are returned.
    UFUNCTION(BlueprintPure, Category="Address")
    FText GetResidentDirectory() const;
};
