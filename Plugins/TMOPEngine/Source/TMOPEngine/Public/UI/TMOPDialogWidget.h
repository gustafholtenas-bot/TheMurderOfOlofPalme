#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPDialogWidget.generated.h"

class ATMOPPlayerCharacter;

/** Native dialogue panel used when the player talks to a historical person. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPDialogWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeDialog(ATMOPPlayerCharacter* InPlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Dialog")
    void ShowDialog(const FText& Speaker, const FText& Dialog);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Dialog")
    void HideDialog();

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

private:
    FReply HandleCloseClicked();

    UPROPERTY(Transient)
    TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;

    TSharedPtr<class STextBlock> SpeakerText;
    TSharedPtr<class STextBlock> DialogText;
};
