#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPAppearanceAssetTypes.generated.h"

class UMaterialInterface;
class USkeletalMesh;
class UStaticMesh;

/** Body areas hidden by a garment to prevent skin clipping through cloth. */
UENUM(BlueprintType, meta=(Bitflags))
enum class ETMOPBodyRegion : uint8
{
    None  = 0 UMETA(Hidden),
    Head  = 1 << 0,
    Neck  = 1 << 1,
    Torso = 1 << 2,
    Arms  = 1 << 3,
    Hands = 1 << 4,
    Hips  = 1 << 5,
    Legs  = 1 << 6,
    Feet  = 1 << 7
};
ENUM_CLASS_FLAGS(ETMOPBodyRegion)

UENUM(BlueprintType)
enum class ETMOPAppearancePartType : uint8
{
    Body,
    Face,
    Hair,
    Outerwear,
    UpperBody,
    Trousers,
    Footwear,
    Gloves,
    Headwear,
    FacialHair,
    Scarf,
    Glasses
};

/** One selectable body, face or modular clothing asset. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAppearanceAssetRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Identity")
    FName CatalogId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Identity")
    ETMOPAppearancePartType PartType = ETMOPAppearancePartType::Body;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Asset")
    TSoftObjectPtr<USkeletalMesh> Mesh;

    /** Static accessory used by Headwear. Static hats avoid skinning, leader-pose
     *  evaluation and clothing morphs. Mesh remains as a legacy fallback while
     *  existing catalog rows are migrated. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Asset")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    /** Socket used by static headwear. Missing sockets safely fall back to head. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Attachment")
    FName AttachmentSocket = TEXT("HeadwearSocket");

    /** Per-asset adjustment after snapping the static mesh to AttachmentSocket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Attachment")
    FTransform AttachmentTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Asset")
    TSoftObjectPtr<UMaterialInterface> Material;

    /** Unknown means unisex. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Compatibility")
    ETMOPPersonGender Gender = ETMOPPersonGender::Unknown;

    /** Empty means every build. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Compatibility")
    TArray<ETMOPBodyBuild> CompatibleBodyBuilds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Compatibility",
        meta=(ClampMin="0"))
    int32 MinimumAge = 0;

    /** Zero means no maximum. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Compatibility",
        meta=(ClampMin="0"))
    int32 MaximumAge = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Period")
    int32 EarliestYear = 1970;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Period")
    int32 LatestYear = 1989;

    /** Normalized evidence tags, e.g. Coat, Long, Wool, DarkBlue, Winter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Matching")
    TArray<FName> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Matching",
        meta=(ClampMin="0.01"))
    float SelectionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Fit",
        meta=(Bitmask, BitmaskEnum="/Script/TMOPEngine.ETMOPBodyRegion"))
    int32 HiddenBodyRegions = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Unknown")
    bool bObscuredFallback = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Color")
    FLinearColor DefaultPrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Appearance|Color")
    FLinearColor DefaultSecondaryColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPResolvedAppearancePart
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    ETMOPAppearancePartType PartType = ETMOPAppearancePartType::Body;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FName CatalogId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    TSoftObjectPtr<USkeletalMesh> Mesh;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    TSoftObjectPtr<UStaticMesh> StaticMesh;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FName AttachmentSocket = TEXT("HeadwearSocket");

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTransform AttachmentTransform = FTransform::Identity;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    TSoftObjectPtr<UMaterialInterface> Material;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FLinearColor SecondaryColor = FLinearColor::White;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    bool bSourceWasUnknown = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    bool bUsesObscuredFallback = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    bool bIntentionallyEmpty = false;

    /** 0 is fully defined; 1 is intentionally anonymous/featureless. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    float ObscurityAmount = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    int32 HiddenBodyRegions = 0;
};

/** Deterministic facial proportions; the final head mesh may support any subset. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPResolvedFaceMorphs
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float FaceWidth = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float JawWidth = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float JawProjection = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float CheekboneProminence = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float NoseWidth = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float NoseLength = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float BrowHeight = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float EyeSpacing = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float LipThickness = 0.0f;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance|Face") float Age = 0.0f;
};

/** Complete deterministic result consumed by the runtime component. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPResolvedAppearance
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    float HeightCentimeters = 171.5f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    ETMOPPersonGender Gender = ETMOPPersonGender::Unknown;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    ETMOPBodyBuild BodyBuild = ETMOPBodyBuild::Average;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    int32 ResolvedSeed = 1;

    /** Bespoke body/head/hair are supplied by the selected MetaHuman agent class. */
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    bool bUsesBespokeMetaHuman = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Body;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Face;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Hair;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Outerwear;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart UpperBody;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Trousers;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Footwear;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Gloves;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Headwear;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart FacialHair;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Scarf;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedAppearancePart Glasses;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    FTMOPResolvedFaceMorphs FaceMorphs;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Appearance")
    TArray<FString> Diagnostics;
};
