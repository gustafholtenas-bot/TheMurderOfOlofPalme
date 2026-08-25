#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "People/TMOPHeldItemTypes.h"
#include "TMOPItemMeshTypes.generated.h"

class UStaticMesh;

UENUM(BlueprintType)
enum class ETMOPItemMeshCategory : uint8
{
    HeldItem UMETA(DisplayName="Held / carried item"),
    Finding UMETA(DisplayName="Historical finding"),
    Both UMETA(DisplayName="Held item and finding")
};

/** One reusable mesh definition shared by people and historical findings. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPItemMeshRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Should match the Data Table row name. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh")
    FString DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh")
    ETMOPItemMeshCategory Category = ETMOPItemMeshCategory::HeldItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh")
    TSoftObjectPtr<UStaticMesh> Mesh;

    /** Base transform applied before a person's optional per-row transform. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh|Held Item")
    FTransform DefaultAttachmentTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh|Held Item")
    ETMOPHeldItemPose DefaultGripPose = ETMOPHeldItemPose::None;

    /** Use 1,1,1 when the imported finding mesh is already real-world size. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh|Finding")
    FVector DefaultFindingScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Item Mesh",
        meta=(MultiLine="true"))
    FString Notes;
};
