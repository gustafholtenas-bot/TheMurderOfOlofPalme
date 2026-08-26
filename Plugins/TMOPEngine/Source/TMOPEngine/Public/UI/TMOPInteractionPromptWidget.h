#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPInteractionPromptWidget.generated.h"

/** Small native interaction prompt shown near the lower screen centre. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPInteractionPromptWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Interaction")
    void SetTargetInformation(const FText& Title, const FText& Details);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Interaction")
    void SetPromptText(const FText& NewText);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    void RefreshTargetVisibility();

    FText TargetTitle;
    FText TargetDetails;
    FText PromptText;
    TSharedPtr<class STextBlock> TargetReticleWidget;
    TSharedPtr<class STextBlock> TargetTitleWidget;
    TSharedPtr<class STextBlock> TargetDetailsWidget;
    TSharedPtr<class SBorder> TargetInformationPanel;
    TSharedPtr<class SBorder> PromptPanel;
    TSharedPtr<class STextBlock> PromptTextWidget;
};
