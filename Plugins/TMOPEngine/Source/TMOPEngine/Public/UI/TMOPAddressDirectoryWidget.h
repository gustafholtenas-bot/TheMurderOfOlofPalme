#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPAddressDirectoryWidget.generated.h"

class ATMOPPlayerCharacter;
class STextBlock;
class SScrollBox;

/** Shared native reader. Class name retained for existing address integrations. */
UCLASS()
class TMOPENGINE_API UTMOPAddressDirectoryWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void InitializeDirectory(ATMOPPlayerCharacter* InPlayer);
    void ShowDirectory(const FText& Address, const FText& Residents);
    void ShowInformation(const FText& Title, const FText& Body,
        const FText& Category, const FText& Source);
    void HideDirectory();

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry,
        const FKeyEvent& KeyEvent) override;

private:
    FReply HandleClose();
    TWeakObjectPtr<ATMOPPlayerCharacter> Player;
    TSharedPtr<STextBlock> TitleText;
    TSharedPtr<STextBlock> DirectoryText;
    TSharedPtr<STextBlock> CategoryText;
    TSharedPtr<STextBlock> SourceText;
    TSharedPtr<SScrollBox> ScrollBox;
    FText AddressText;
    FText ResidentsText;
    FText ReadingCategory;
    FText ReadingSource;
};
