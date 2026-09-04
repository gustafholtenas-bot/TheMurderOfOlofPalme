#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Fonts/SlateFontInfo.h"
#include "TMOPTypographyTypes.generated.h"

/** One centrally editable visual role used by native and Blueprint UI text. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTypographyStyleRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName StyleId = TEXT("Body");
    /** Human-readable list of the exact screens/widgets that use this row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MultiLine="true"))
    FText UsedBy;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UObject> FontAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Typeface = TEXT("Regular");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", ClampMax="200")) int32 Size = 16;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D ShadowOffset = FVector2D::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowColor = FLinearColor::Transparent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="10")) int32 OutlineSize = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor OutlineColor = FLinearColor::Black;

    /** Text Render uses legacy offline UFont assets, not Slate Composite Fonts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Text")
    bool bOverrideWorldFont = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Text")
    bool bOverrideWorldSize = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="World Text")
    bool bOverrideWorldColor = false;
};

/** Non-font colours shared by the native main and pause menus. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPMenuColorPalette
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MenuBackground =
        FLinearColor(0.008f, 0.012f, 0.022f, 0.97f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor PanelBackground =
        FLinearColor(0.025f, 0.035f, 0.055f, 0.96f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ButtonBackground =
        FLinearColor(0.18f, 0.18f, 0.18f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor MainMenuButtonText =
        FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor PauseMenuButtonText =
        FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor StatusText =
        FLinearColor(0.95f, 0.70f, 0.20f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor AccentText =
        FLinearColor(0.95f, 0.70f, 0.20f, 1.0f);
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor IntroCardBackground =
        FLinearColor(0.01f, 0.015f, 0.025f, 0.82f);
};

/** Read-only reference shown on the director so every supported style is discoverable. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTypographyUsageReference
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) FName StyleId;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta=(MultiLine="true")) FText UsedBy;
};
