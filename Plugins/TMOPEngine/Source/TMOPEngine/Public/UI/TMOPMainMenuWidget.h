#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "TMOPMainMenuWidget.generated.h"

class ATMOPMainMenuIntroDirector;
class SImage;
class STextBlock;
class UTexture2D;

UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPMainMenuWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void InitializeMainMenu(ATMOPMainMenuIntroDirector* InDirector,
        UTexture2D* InLogo);
    void SetMenuMode(bool bShowMenu);
    void SetIntroCard(const FText& Heading, const FText& Body,
        UTexture2D* Image, bool bVisible);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    FReply StartClicked();
    FReply LoadClicked();
    FReply SettingsClicked();
    FReply QuitClicked();
    TWeakObjectPtr<ATMOPMainMenuIntroDirector> Director;
    UPROPERTY(Transient) TObjectPtr<UTexture2D> LogoTexture;
    UPROPERTY(Transient) TObjectPtr<UTexture2D> CardTexture;
    FSlateBrush LogoBrush;
    FSlateBrush CardImageBrush;
    TSharedPtr<class SVerticalBox> MenuPanel;
    TSharedPtr<class SBorder> IntroPanel;
    TSharedPtr<STextBlock> IntroHeading;
    TSharedPtr<STextBlock> IntroBody;
    TSharedPtr<SImage> IntroImage;
};
