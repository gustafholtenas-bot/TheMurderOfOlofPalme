#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Styling/SlateColor.h"
#include "UI/TMOPTypographyTypes.h"
#include "TMOPTypographyDirector.generated.h"

class STextBlock;
class UDataTable;
class UTextBlock;
class UTextRenderComponent;

/** Central typography source for all TMOP native and Blueprint interfaces. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPTypographyDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPTypographyDirector();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Typography")
    TObjectPtr<UDataTable> TypographyTable;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Typography")
    bool bAutomaticallyStyleBlueprintWidgets = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Typography",
        meta=(ClampMin="0.1", Units="s"))
    float BlueprintRefreshIntervalSeconds = 0.5f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Typography")
    TMap<FName, FName> WidgetNameStyleOverrides;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Typography|Menu Colors")
    FTMOPMenuColorPalette MenuColors;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|Typography|Style Reference")
    TArray<FTMOPTypographyUsageReference> StyleUsageReference;

    UFUNCTION(BlueprintCallable, Category="TMOP|Typography")
    bool ApplyTypographyStyle(UTextBlock* TextBlock, FName StyleId) const;
    UFUNCTION(BlueprintCallable, Category="TMOP|Typography")
    bool ApplyWorldTextStyle(UTextRenderComponent* TextRender, FName StyleId) const;
    UFUNCTION(BlueprintPure, Category="TMOP|Typography")
    bool GetTypographyStyle(FName StyleId, FTMOPTypographyStyleRow& OutStyle) const;
    UFUNCTION(BlueprintCallable, Category="TMOP|Typography")
    void RefreshAllBlueprintText();

    static const ATMOPTypographyDirector* Find(const UObject* WorldContext);
    static FSlateFontInfo ResolveFont(const UObject* WorldContext, FName StyleId,
        const FSlateFontInfo& Fallback);
    static FSlateColor ResolveColor(const UObject* WorldContext, FName StyleId,
        const FLinearColor& Fallback);
    static FTMOPMenuColorPalette ResolveMenuColors(const UObject* WorldContext);
    static void ApplySlateStyle(const UObject* WorldContext,
        const TSharedPtr<STextBlock>& TextBlock, FName StyleId);

private:
    const FTMOPTypographyStyleRow* FindExactStyle(FName StyleId) const;
    FName InferStyleId(const UTextBlock* TextBlock) const;
    float RefreshAccumulator = 0.0f;
};
