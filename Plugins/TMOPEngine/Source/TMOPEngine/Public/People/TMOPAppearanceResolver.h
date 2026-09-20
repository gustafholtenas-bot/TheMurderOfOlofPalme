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

    /** Standard modular head used when no bespoke Face choice is assigned.
     *  The nearest of 18/30/45/65 is selected for male/female profiles.
     *  Unknown or unspecified gender returns NAME_None. */
    UFUNCTION(BlueprintPure, Category="TMOP|Appearance|Face")
    static FName GetStandardFaceCatalogId(
        ETMOPPersonGender Gender, int32 AgeAtEvent);

    UFUNCTION(BlueprintPure, Category="TMOP|Appearance|Hair")
    static FName GetHairMaterialKey(const FTMOPAppearanceSlot& Evidence,
        ETMOPHairColor Category);

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

