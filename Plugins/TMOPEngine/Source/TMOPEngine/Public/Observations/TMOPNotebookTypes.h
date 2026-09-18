#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"
#include "TMOPNotebookTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPNotebookCategory : uint8
{
    Automatic UMETA(DisplayName="Automatisk"),
    Shooter UMETA(DisplayName="1. Skytten"),
    HighlySuspicious UMETA(DisplayName="2. Högt suspekta personer"),
    WalkieTalkie UMETA(DisplayName="3. Män med walkie-talkies"),
    OtherSuspicious UMETA(DisplayName="4. Andra suspekta personer"),
    LessSuspicious UMETA(DisplayName="5. Mindre suspekta personer")
};

UENUM(BlueprintType)
enum class ETMOPNotebookEntityKind : uint8
{
    Person UMETA(DisplayName="Person"),
    Vehicle UMETA(DisplayName="Fordon")
};

UENUM(BlueprintType)
enum class ETMOPNotebookVehicleSuspicion : uint8
{
    LessSuspicious UMETA(DisplayName="Mindre misstänkta"),
    HighlySuspicious UMETA(DisplayName="Mycket misstänkta")
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPNotebookEvidenceImage
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FSoftObjectPath ImagePath;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FText Caption;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FText Source;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPNotebookObservation
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FName EntityId;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") ETMOPNotebookEntityKind Kind = ETMOPNotebookEntityKind::Person;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FText DisplayName;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FText Summary;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") ETMOPNotebookCategory Category = ETMOPNotebookCategory::OtherSuspicious;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") int32 DiscoveredSecond = 0;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") FText Signalement;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") TArray<FText> ObserverNames;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") TArray<FTMOPNotebookEvidenceImage> EvidenceImages;
    /** Compressed model snapshot, independent of whether the original actor still exists. */
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") TArray<uint8> ModelPreviewPng;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Notebook") ETMOPNotebookVehicleSuspicion VehicleSuspicion = ETMOPNotebookVehicleSuspicion::LessSuspicious;
    UPROPERTY(SaveGame) int32 PresentationVersion = 0;
};

namespace TMOPNotebook
{
    inline bool IsVehicleEligible(const FString& EntityId, const FString& Category)
    {
        return Category.ToUpper().StartsWith(TEXT("OBSERVED_")) ||
            EntityId.ToUpper().StartsWith(TEXT("OBSERVED_"));
    }
    // Shared with green name labels. Ordinary witnesses/police are never
    // eligible simply because they run, carry a radio, or have an override.
    inline bool IsEligible(const FString& EntityId, const FString& Category)
    {
        const FString Id = EntityId.ToUpper();
        const FString Kind = Category.ToUpper();
        if (Id.EndsWith(TEXT("_PALME"))) return false;
        if (Kind.StartsWith(TEXT("OBSERVED_")) || Id.StartsWith(TEXT("OBSERVED_")))
            return true;
        if (Kind == TEXT("POLICE") || Kind == TEXT("POLIS")) return false;
        return Kind == TEXT("SUSPECT") || Id == TEXT("THE_KILLER");
    }

    inline ETMOPNotebookCategory ResolveCategory(ETMOPNotebookCategory Authored,
        bool bShooter, bool bFleeing, bool bWalkieTalkie)
    {
        if (bShooter) return ETMOPNotebookCategory::Shooter;
        if (Authored != ETMOPNotebookCategory::Automatic) return Authored;
        if (bFleeing) return ETMOPNotebookCategory::HighlySuspicious;
        if (bWalkieTalkie) return ETMOPNotebookCategory::WalkieTalkie;
        return ETMOPNotebookCategory::OtherSuspicious;
    }

    inline bool AddUnique(TArray<FTMOPNotebookObservation>& Entries,
        const FTMOPNotebookObservation& Entry)
    {
        if (Entry.EntityId.IsNone() || Entries.ContainsByPredicate(
            [&Entry](const FTMOPNotebookObservation& Existing)
            { return Existing.EntityId == Entry.EntityId && Existing.Kind == Entry.Kind; })) return false;
        Entries.Add(Entry);
        return true;
    }
}
