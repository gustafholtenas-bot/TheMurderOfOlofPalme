#pragma once
#include "CoreMinimal.h"
#include "World/TMOPInspectableComponent.h"
#include "TMOPAddressComponent.generated.h"

class UDataTable;
struct FTMOPAddressRegistryRow;
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPAddressComponent : public UTMOPInspectableComponent
{
    GENERATED_BODY()
public:
    UTMOPAddressComponent();
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    TObjectPtr<UDataTable> Registry;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Address")
    FName RowName;

    UFUNCTION(BlueprintPure, Category="Address")
    bool HasValidAddress() const;

    UFUNCTION(BlueprintPure, Category="Address")
    FText GetAddressTitle() const;

    // Use this function from an interaction widget; no archival birth details are returned.
    UFUNCTION(BlueprintPure, Category="Address")
    FText GetResidentDirectory() const;

    virtual bool HasReadableContent() const override;
    virtual FText GetInspectionTitle() const override;
    virtual FText GetInspectionText() const override;
    virtual FText GetInspectionCategory() const override;
    virtual FText GetInspectionAction() const override;

private:
    const FTMOPAddressRegistryRow* FindAddress() const;
};
