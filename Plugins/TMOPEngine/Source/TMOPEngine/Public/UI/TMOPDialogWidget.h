#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "TMOPDialogWidget.generated.h"

class ATMOPPlayerCharacter;
class ATMOPRecordedCallDirector;
class UTexture2D;

/** Native dialogue panel used when the player talks to a historical person. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPDialogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UTMOPDialogWidget(const FObjectInitializer& ObjectInitializer);
    void InitializeDialog(ATMOPPlayerCharacter* InPlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Dialog")
    void ShowDialog(const FText& Speaker, const FText& Dialog);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Dialog")
    void HideDialog();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Communication")
    float TypewriterCharactersPerSecond = 38.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Communication")
    TObjectPtr<UTexture2D> RadioIcon;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    FReply HandleCloseClicked();
    void RefreshVisibility();
    void RefreshRadioSubtitle();
    void AdvanceTypewriter(float DeltaTime);

    UPROPERTY(Transient)
    TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;

    TSharedPtr<class STextBlock> SpeakerText;
    TSharedPtr<class STextBlock> DialogText;
    TSharedPtr<class SBorder> DialogPanel;
    TSharedPtr<class SBorder> RadioPanel;
    TSharedPtr<class STextBlock> RadioLeftSpeakerText;
    TSharedPtr<class STextBlock> RadioRightSpeakerText;
    TSharedPtr<class STextBlock> RadioSubtitleText;
    FSlateBrush RadioIconBrush;
    TWeakObjectPtr<ATMOPRecordedCallDirector> RecordedCallDirector;
    FString FullDialogString;
    FString FullRadioString;
    FName ActiveRadioRecordingId = NAME_None;
    FName ActiveRadioSegmentId = NAME_None;
    float DialogRevealCharacters = 0.0f;
    float RadioRevealCharacters = 0.0f;
    bool bDialogVisible = false;
    bool bRadioVisible = false;
};
