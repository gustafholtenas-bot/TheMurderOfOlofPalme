#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TMOPPersonNameLibrary.generated.h"

/** Presentation only; never modifies archival names or entity IDs. */
UCLASS()
class TMOPENGINE_API UTMOPPersonNameLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category="TMOP|Person|Display")
    static FText FormatPersonName(const FText& FullName, const FText& FirstName, const FText& LastName);

    UFUNCTION(BlueprintPure, Category="TMOP|Person|Display")
    static FText FormatUnstructuredPersonName(const FText& Name);

    UFUNCTION(BlueprintPure, Category="TMOP|Person|Display")
    static FText GetSurnameInitial(const FText& Surname);
};
