#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateBrush.h"
#include "UObject/GCObject.h"

class UTexture2D;

struct FTMOPAtlasLand
{
    FString Id, Bloc;
    TArray<FVector> Vertices;
    TArray<int32> Indices;
    TArray<TArray<FVector>> Borders;
};

/** Optional, offline visual assets. A bad overlay never disables historical records. */
class FTMOPAtlasAppearance final : public FGCObject
{
public:
    bool Load();
    const FSlateBrush* Flag(const FString& CountryId) const { return Flags.Find(CountryId); }
    FText Alignment(const FString& CountryId) const;
    static FLinearColor BlocColor(const FString& Bloc);
    virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
    virtual FString GetReferencerName() const override { return TEXT("FTMOPAtlasAppearance"); }

    TArray<FTMOPAtlasLand> Lands;
    FString Error;
private:
    UTexture2D* FlagTexture = nullptr;
    TMap<FString, FSlateBrush> Flags;
    struct FAlignmentText { FString Swedish, English, EnglishSource; };
    TMap<FString, FAlignmentText> Alignments;
};
