#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Observations/TMOPNotebookTypes.h"
#include "TMOPTheoryTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPTheoryNodeKind : uint8 { Person, Vehicle, Note };

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTheoryNode
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FGuid Id;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") ETMOPTheoryNodeKind Kind = ETMOPTheoryNodeKind::Person;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") bool bShooter = false;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") bool bHeading = false;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FString Role;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FVector2D Size = FVector2D(80, 104);
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FName EntityId;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FString Title;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FString Notes;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FVector2D Position = FVector2D::ZeroVector;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTheoryLink
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FGuid Id;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FGuid From;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FGuid To;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FString Label;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTheoryTree
{
    GENERATED_BODY()
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FGuid Id;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FString Title;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") TArray<FTMOPTheoryNode> Nodes;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") int32 TemplateIndex = 0;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") TArray<FTMOPTheoryLink> Links;
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") FVector2D Pan = FVector2D(20, 20);
    UPROPERTY(SaveGame, BlueprintReadOnly, Category="Theory") float Zoom = 0.65f;
};

UENUM(BlueprintType)
enum class ETMOPTheoryInformationTrack : uint8
{
    LoneGunman UMETA(DisplayName="Ensam gärningsman"),
    Conspiracy UMETA(DisplayName="Konspiration")
};

/** Author sourced information in a Data Table using this row type. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTheoryInformationRow : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Theory information") ETMOPTheoryInformationTrack Track = ETMOPTheoryInformationTrack::Conspiracy;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Theory information") int32 SortOrder = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Theory information") FText Title;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Theory information", meta=(MultiLine="true")) FText Body;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Theory information", meta=(MultiLine="true")) FText Source;
};

namespace TMOPTheory
{
    TMOPENGINE_API FString TemplateName(int32 Index);
    TMOPENGINE_API TArray<FTMOPTheoryInformationRow> DefaultInformation();
    TMOPENGINE_API FTMOPTheoryTree CreateTemplate(int32 Index);
    TMOPENGINE_API FGuid AddNode(FTMOPTheoryTree& Tree, ETMOPTheoryNodeKind Kind,
        const FVector2D& Position, const FString& Title);
    TMOPENGINE_API bool RemoveNode(FTMOPTheoryTree& Tree, FGuid Id);
    TMOPENGINE_API bool AddLink(FTMOPTheoryTree& Tree, FGuid From, FGuid To);
    TMOPENGINE_API bool AssignObservation(FTMOPTheoryTree& Tree, FGuid NodeId,
        FName EntityId, const TArray<FTMOPNotebookObservation>& Observations);
    TMOPENGINE_API void SynchronizeShooter(FTMOPTheoryTree& Tree,
        const TArray<FTMOPNotebookObservation>& Observations);
}
