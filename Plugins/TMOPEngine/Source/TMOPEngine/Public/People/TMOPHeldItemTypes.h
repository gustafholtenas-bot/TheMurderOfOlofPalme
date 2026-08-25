#pragma once

#include "CoreMinimal.h"
#include "Time/TMOPTime.h"
#include "TMOPHeldItemTypes.generated.h"

class UStaticMesh;

UENUM(BlueprintType)
enum class ETMOPHeldItemPose : uint8
{
    None,
    Relaxed,
    WalkieTalkie,
    Pistol,
    Revolver,
    Briefcase,
    Package,
    PlasticBag,
    Custom
};

UENUM(BlueprintType)
enum class ETMOPItemAttachmentPoint : uint8
{
    Custom,
    LeftHand,
    RightHand,
    Chest,
    Back,
    LeftShoulder,
    RightShoulder,
    LeftHip,
    RightHip
};

/** One prop attached to an NPC hand. Transform is relative to the selected socket. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHeldItemDefinition
{
    GENERATED_BODY()

    /** Stable ID can be filled before the final mesh asset exists. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    TSoftObjectPtr<UStaticMesh> Mesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    ETMOPItemAttachmentPoint AttachmentPoint = ETMOPItemAttachmentPoint::Custom;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    FName SocketName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    FTransform RelativeTransform = FTransform::Identity;

    /** Read by the animation blueprint to select hand/finger and upper-body pose. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    ETMOPHeldItemPose GripPose = ETMOPHeldItemPose::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item")
    bool bVisible = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item|Time")
    bool bUseVisibilityWindow = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item|Time",
        meta=(EditCondition="bUseVisibilityWindow"))
    FTMOPTime VisibleFrom = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Item|Time",
        meta=(EditCondition="bUseVisibilityWindow"))
    FTMOPTime VisibleUntil = FTMOPTime(23, 45, 0);
};
