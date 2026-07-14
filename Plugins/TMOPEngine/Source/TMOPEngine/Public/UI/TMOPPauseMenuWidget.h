#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPPauseMenuWidget.generated.h"

class APlayerController;
class ATMOPPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTMOPPauseMenuRequestSignature);

/** Native pause-menu shell. Resume/Quit work now; remaining buttons expose events. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPPauseMenuWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializePauseMenu(APlayerController* InPlayerController,
        ATMOPPlayerCharacter* InPlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Pause")
    void SetMenuVisible(bool bVisible);

    UPROPERTY(BlueprintAssignable, Category="TMOP|UI|Pause")
    FTMOPPauseMenuRequestSignature OnSettingsRequested;

    UPROPERTY(BlueprintAssignable, Category="TMOP|UI|Pause")
    FTMOPPauseMenuRequestSignature OnSaveRequested;

    UPROPERTY(BlueprintAssignable, Category="TMOP|UI|Pause")
    FTMOPPauseMenuRequestSignature OnLoadRequested;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    FReply HandleResumeClicked();
    FReply HandleSettingsClicked();
    FReply HandleSaveClicked();
    FReply HandleLoadClicked();
    FReply HandleQuitClicked();
    void SetStatus(const FText& Text);

    UPROPERTY(Transient)
    TObjectPtr<APlayerController> PlayerController;

    UPROPERTY(Transient)
    TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;

    TSharedPtr<class STextBlock> StatusText;
};
