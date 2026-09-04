#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Styling/SlateBrush.h"
#include "UI/TMOPMainMenuIntroTypes.h"
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
    void ConfigureIntroText(const FTMOPIntroTextPresentationSettings& InSettings);
    void SetMenuMode(bool bShowMenu);
    void SetLoadMenuMode(bool bShowLoadMenu);
    void SetLoadStatus(const FText& Status);
    void SetIntroControlsVisible(bool bVisible);
    void SetIntroCard(const FText& Heading, const FText& Body,
        UTexture2D* Image, bool bVisible);
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual TSharedRef<SWidget> RebuildWidget() override;
private:
    FReply StartClicked();
    FReply LoadClicked();
    FReply LoadSlotClicked(FString SlotName);
    FReply LoadBackClicked();
    FReply SettingsClicked();
    FReply QuitClicked();
    FReply SkipIntroClicked();
    TWeakObjectPtr<ATMOPMainMenuIntroDirector> Director;
    UPROPERTY(Transient) TObjectPtr<UTexture2D> LogoTexture;
    UPROPERTY(Transient) TObjectPtr<UTexture2D> CardTexture;
    FSlateBrush LogoBrush;
    FSlateBrush CardImageBrush;
    TSharedPtr<class SVerticalBox> MenuPanel;
    TSharedPtr<class SVerticalBox> LoadPanel;
    TSharedPtr<class SVerticalBox> LoadListBox;
    TSharedPtr<STextBlock> LoadStatusText;
    TSharedPtr<class SBorder> IntroPanel;
    TSharedPtr<class SBox> IntroImageBox;
    TSharedPtr<class SButton> IntroSkipButton;
    TSharedPtr<STextBlock> IntroHeading;
    TSharedPtr<STextBlock> IntroBody;
    TSharedPtr<SImage> IntroImage;
    FTMOPIntroTextPresentationSettings IntroTextSettings;
    FString FullIntroHeading;
    FString FullIntroBody;
    float TypewriterCharacterAccumulator = 0.0f;
    int32 RevealedHeadingCharacters = 0;
    int32 RevealedBodyCharacters = 0;
    bool bIntroCardVisible = false;
    void ApplyIntroTextStyle();
    void ResetTypewriter();
    void RebuildLoadList();
};
