#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TMOPLocalizationLibrary.generated.h"

/** Bridge for Blueprint UI that displays legacy table strings. User notes/names bypass it. */
UCLASS()
class TMOPENGINE_API UTMOPLocalizationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** Table is the asset name (DT_TMOP_...), Row its exported Name, Field e.g. Body. */
    UFUNCTION(BlueprintPure, Category="TMOP|Language")
    static FText LocalizeTableText(const FString& Table, FName Row, const FString& Field, const FText& Source);

    UFUNCTION(BlueprintPure, Category="TMOP|Language")
    static FText LocalizeTableString(const FString& Table, FName Row, const FString& Field, const FString& Source);

    UFUNCTION(BlueprintPure, Category="TMOP|Language")
    static FText LocalizeText(const FText& Source);

    UFUNCTION(BlueprintPure, Category="TMOP|Language")
    static FText LocalizeString(const FString& Source);
};
