#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "TMOPMainMenuIntroTypes.generated.h"

class ACameraActor;
class UTexture2D;

UENUM(BlueprintType)
enum class ETMOPIntroRouteSource : uint8
{
    AutomaticFromAnchors UMETA(DisplayName="Automatic From Anchors"),
    VehicleEditorTimeline UMETA(DisplayName="Vehicle Editor Timeline")
};

/** Shared presentation settings for the title cards shown during the intro. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPIntroTextPresentationSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Typewriter")
    bool bUseTypewriter = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Typewriter")
    bool bTypewriterHeading = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Typewriter",
        meta=(EditCondition="bUseTypewriter", ClampMin="1.0", ClampMax="120.0"))
    float CharactersPerSecond = 32.0f;

    /** Typography table style, with the fields below acting as explicit overrides. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heading")
    FName HeadingStyleId = TEXT("IntroCardHeading");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heading",
        meta=(AllowedClasses="/Script/Engine.Font,/Script/Engine.FontFace"))
    TObjectPtr<UObject> HeadingFontAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heading")
    FName HeadingTypeface = TEXT("Bold");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heading",
        meta=(ClampMin="6", ClampMax="96"))
    int32 HeadingFontSize = 24;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Heading")
    FLinearColor HeadingColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body")
    FName BodyStyleId = TEXT("IntroCardBody");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body",
        meta=(AllowedClasses="/Script/Engine.Font,/Script/Engine.FontFace"))
    TObjectPtr<UObject> BodyFontAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body")
    FName BodyTypeface = TEXT("Regular");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body",
        meta=(ClampMin="6", ClampMax="96"))
    int32 BodyFontSize = 17;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body")
    FLinearColor BodyColor = FLinearColor::White;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPIntroPresentationCard : public FTableRowBase
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName CardId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float StartSeconds = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float EndSeconds = 5.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FText Heading;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine=true)) FText Body;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UTexture2D> Image;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPIntroCameraShot
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float StartSeconds = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0.0", Units="s")) float BlendSeconds = 1.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TObjectPtr<ACameraActor> PlacedCamera = nullptr;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bFollowIntroVehicle = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bFollowIntroVehicle")) FTransform VehicleRelativeTransform = FTransform(FRotator(-12.0f, 180.0f, 0.0f), FVector(-650.0f, 0.0f, 260.0f));
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(EditCondition="bFollowIntroVehicle")) bool bLookAtVehicle = true;

    /** Adds subtle deterministic camera drift without moving the source camera actor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handheld")
    bool bEnableHandheldMotion = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handheld",
        meta=(EditCondition="bEnableHandheldMotion", ClampMin="0.01", ClampMax="10.0", Units="Hz"))
    float HandheldFrequencyHz = 0.75f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handheld",
        meta=(EditCondition="bEnableHandheldMotion", Units="cm"))
    FVector HandheldLocationAmplitude = FVector(2.5f, 3.0f, 2.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handheld",
        meta=(EditCondition="bEnableHandheldMotion"))
    FRotator HandheldRotationAmplitude = FRotator(0.35f, 0.55f, 0.25f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Handheld",
        meta=(EditCondition="bEnableHandheldMotion"))
    int32 HandheldSeed = 17;
};
