#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPSpeechBubbleWidget.generated.h"

/** Code-built screen-space speech bubble shown above an NPC. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPSpeechBubbleWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Speech")
    void SetSpeechText(const FText& NewText);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Speech")
    void SetSpeakerName(const FText& NewName);

    /** Absolute typewriter position, independent of previous visits to this line. */
    void SetPlaybackElapsed(float Seconds);

    /** A deliberately brisk typewriter speed for short world-space dialogue. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Speech",
        meta=(ClampMin="1.0", ClampMax="180.0"))
    float TypewriterCharactersPerSecond = 62.0f;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    TSharedPtr<class STextBlock> SpeechText;
    TSharedPtr<class STextBlock> SpeakerNameText;
    FText PendingSpeechText;
    FText PendingSpeakerName;
    FString FullSpeechString;
    float RevealedCharacterAccumulator = 0.0f;
    int32 RevealedCharacterCount = 0;
    bool bHistoricalPlayback = false;
};
