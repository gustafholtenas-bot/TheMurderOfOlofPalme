#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TMOPAddressEditorLibrary.generated.h"

class ATMOPHistoricalAnchor;
class UDataTable;

/** Persistent, undoable component attachment for the address installer script. */
UCLASS()
class TMOPENGINEEDITOR_API UTMOPAddressEditorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** Validate and replace the rows of the existing registry asset without changing its identity. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Editor|Address")
    static FString ReplaceAddressRegistryJson(UDataTable* Registry, const FString& Json,
        bool bDryRun = false);

    /** Empty result means success. Dry run performs all checks without modifying anything. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Editor|Address")
    static FString BindAddressAnchor(ATMOPHistoricalAnchor* Anchor, UDataTable* Registry,
        FName RowName, bool bDryRun = false);
};
