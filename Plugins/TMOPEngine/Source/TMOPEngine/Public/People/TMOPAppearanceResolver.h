#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "People/TMOPAppearanceAssetTypes.h"
#include "TMOPAppearanceResolver.generated.h"

class UDataTable;

/** Deterministically converts evidence and overrides into runtime asset choices. */
UCLASS()
class TMOPENGINE_API UTMOPAppearanceResolver final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="TMOP|Appearance")
    static bool ResolveAppearance(const FTMOPPersonProfileRow& Profile,
        UDataTable* AssetCatalog, FTMOPResolvedAppearance& OutAppearance);

    UFUNCTION(BlueprintPure, Category="TMOP|Appearance")
    static bool IsAppearanceSlotKnown(const FTMOPAppearanceSlot& Slot);

    /** Converts Swedish/English free-text evidence to catalog tags. */
    UFUNCTION(BlueprintPure, Category="TMOP|Appearance")
    static TArray<FName> GetNormalizedEvidenceTags(
        const FTMOPAppearanceSlot& Slot);

private:
    static FTMOPResolvedAppearancePart ResolvePart(
        const FTMOPPersonProfileRow& Profile,
        UDataTable* AssetCatalog,
        ETMOPAppearancePartType PartType,
        const FTMOPAppearancePartChoice& Override,
        const TArray<FTMOPAppearanceSlot>& Evidence,
        FName UnknownCatalogId,
        bool bKnownAbsent,
        FRandomStream& Random,
        TArray<FString>& Diagnostics);
};
