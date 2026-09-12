#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPLoopEndWidget.generated.h"

class ATMOPPlayerCharacter;
class SButton;

/** Non-dismissible choice shown when the scenario clock reaches 23:45. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPLoopEndWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeLoopEnd(ATMOPPlayerCharacter* InPlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Loop End")
    void SetMenuVisible(bool bVisible);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry& Geometry,
        const FKeyEvent& KeyEvent) override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
    FReply HandleReplayClicked();
    FReply HandleMainMenuClicked();
    FReply HandleQuitClicked();

    UPROPERTY(Transient)
    TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;

    TSharedPtr<SButton> ReplayButton;
};
