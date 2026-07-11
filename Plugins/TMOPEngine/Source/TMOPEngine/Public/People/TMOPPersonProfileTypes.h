#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "TMOPPersonProfileTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPPersonGender : uint8
{
    Unknown,
    Female,
    Male,
    OtherOrUnspecified
};

UENUM(BlueprintType)
enum class ETMOPHairColor : uint8
{
    Unknown,
    Blond,
    Brown,
    Dark,
    Black,
    Grey,
    Red,
    White,
    Bald
};

UENUM(BlueprintType)
enum class ETMOPFacialHairType : uint8
{
    Unknown,
    None,
    Mustache,
    Beard,
    MustacheAndBeard,
    Stubble
};

UENUM(BlueprintType)
enum class ETMOPHeadwearType : uint8
{
    Unknown,
    None,
    Cap,
    KnitCap,
    FurHat,
    BrimmedHat,
    Hood,
    Helmet,
    Other
};

UENUM(BlueprintType)
enum class ETMOPBodyBuild : uint8
{
    Unknown,
    Thin,
    Slim,
    Average,
    Athletic,
    Strong,
    Heavy
};

UENUM(BlueprintType)
enum class ETMOPOuterwearType : uint8
{
    Unknown,
    None,
    Jacket,
    Coat,
    LeatherJacket,
    SailingJacket,
    Parka,
    Uniform,
    Other
};

/** One evidence-preserving description slot. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAppearanceSlot
{
    GENERATED_BODY()

    /** Exact wording from the source. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    FString OriginalText;

    /** Stable filter value, e.g. DarkBlueLongCoat. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    FName NormalizedValue = NAME_None;

    /** Additional flexible filters that do not require a C++ rebuild. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    TArray<FName> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    FString SourceReference;
};

/** One row in DT_TMOP_People. Row Name should equal EntityId. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonProfileRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FName EntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FText FullName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FText FirstName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FText LastName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    ETMOPPersonGender Gender = ETMOPPersonGender::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FString Nationality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FString Occupation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History")
    FString HistoricalAddress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History")
    int32 BirthYear = 0;

    /** Age at the simulated event, not current age. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History")
    int32 AgeAtEvent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance",
        meta=(ClampMin="0.0"))
    float HeightCentimeters = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPBodyBuild BodyBuildCategory = ETMOPBodyBuild::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPHairColor HairColorCategory = ETMOPHairColor::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPHeadwearType HeadwearCategory = ETMOPHeadwearType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPFacialHairType FacialHairCategory = ETMOPFacialHairType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPOuterwearType OuterwearCategory = ETMOPOuterwearType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot Hair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot Headwear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot BeardOrMustache;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot FaceShape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot Nose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot JacketOrCoat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot ShirtOrSweater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot Trousers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot Shoes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Accessories")
    FTMOPAppearanceSlot Scarf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Accessories")
    FTMOPAppearanceSlot Glasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot OtherCharacteristics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Source")
    FString GeneralSourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Source")
    FString Notes;
};
