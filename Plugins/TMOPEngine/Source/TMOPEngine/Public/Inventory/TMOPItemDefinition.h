#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TMOPItemDefinition.generated.h"

class UStaticMesh;
class UTexture2D;

UENUM(BlueprintType)
enum class ETMOPItemType : uint8
{
    Camera,
    Flashlight,
    Radio,
    Notebook,
    Map,
    Pistol,
    BaseballBat,
    Cigarette,
    Other
};

/** Data-only description shared by inventory, HUD, hand visual and future item actions. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Identity")
    FName ItemId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Identity",
        meta=(MultiLine="true"))
    FText Description;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item")
    ETMOPItemType ItemType = ETMOPItemType::Other;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Visual")
    TObjectPtr<UTexture2D> Icon;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Visual")
    TObjectPtr<UStaticMesh> RightHandMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Visual")
    FName RightHandSocket = TEXT("hand_rSocket");

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Visual")
    FTransform RightHandTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Inventory",
        meta=(ClampMin="1"))
    int32 MaximumStack = 1;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Inventory")
    bool bCanDrop = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Inventory")
    bool bCanEquip = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Item|Inventory")
    bool bOpensMenuInsteadOfEquipping = false;

    virtual FPrimaryAssetId GetPrimaryAssetId() const override
    {
        return FPrimaryAssetId(TEXT("TMOPItem"), ItemId);
    }
};
