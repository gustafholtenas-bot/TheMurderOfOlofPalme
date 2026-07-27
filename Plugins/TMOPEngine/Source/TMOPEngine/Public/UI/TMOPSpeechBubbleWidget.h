#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPSpeechBubbleWidget.generated.h"

/** Screen-space speech bubble shown above an NPC. It intentionally has no name. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPSpeechBubbleWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Speech")
    void SetSpeechText(const FText& NewText);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    TSharedPtr<class STextBlock> SpeechText;
};
