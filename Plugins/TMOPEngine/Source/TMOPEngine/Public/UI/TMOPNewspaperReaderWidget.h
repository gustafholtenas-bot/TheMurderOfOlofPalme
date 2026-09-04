#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPNewspaperReaderWidget.generated.h"

class ATMOPPlayerCharacter;
class UTexture2D;
class UTMOPNewspaperItemDefinition;

/** Native full-screen reader for scanned newspaper pages. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPNewspaperReaderWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeReader(ATMOPPlayerCharacter* InPlayerCharacter);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Newspaper")
    bool OpenNewspaper(UTMOPNewspaperItemDefinition* InNewspaper);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Newspaper")
    void CloseReader();

    /** Called by the player after restoring input and simulation state. */
    void DismissReader();

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Newspaper")
    bool NextPage();

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Newspaper")
    bool PreviousPage();

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Newspaper")
    bool GoToPage(int32 PageIndex);

    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Newspaper")
    void SetZoom(float NewZoom);

    UFUNCTION(BlueprintPure, Category="TMOP|UI|Newspaper")
    int32 GetCurrentPageIndex() const { return CurrentPageIndex; }

    UFUNCTION(BlueprintPure, Category="TMOP|UI|Newspaper")
    int32 GetPageCount() const;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnPreviewKeyDown(
        const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnKeyDown(
        const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
    virtual FReply NativeOnMouseWheel(
        const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
    FReply HandlePreviousClicked();
    FReply HandleNextClicked();
    FReply HandleZoomOutClicked();
    FReply HandleZoomInClicked();
    FReply HandleCloseClicked();
    FReply PanPage(float HorizontalDirection, float VerticalDirection);
    FReply HandleReaderKey(const FKey& Key);
    void RefreshPage();

    UPROPERTY(Transient)
    TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPNewspaperItemDefinition> Newspaper;

    int32 CurrentPageIndex = 0;
    float Zoom = 1.0f;
    TSharedPtr<class STextBlock> TitleText;
    TSharedPtr<class STextBlock> PageNumberText;
    TSharedPtr<class STextBlock> PageLabelText;
};
