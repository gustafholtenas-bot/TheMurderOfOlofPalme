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
    UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<UObject> FontAsset;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FName Typeface = TEXT("Regular");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="1", ClampMax="200")) int32 Size = 16;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor Color = FLinearColor::White;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector2D ShadowOffset = FVector2D::ZeroVector;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor ShadowColor = FLinearColor::Transparent;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(ClampMin="0", ClampMax="10")) int32 OutlineSize = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite) FLinearColor OutlineColor = FLinearColor::Black;
};
