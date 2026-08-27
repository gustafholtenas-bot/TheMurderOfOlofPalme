#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPPauseMenuWidget.generated.h"

class APlayerController;
class ATMOPPlayerCharacter;
class UDataTable;
class UTMOPItemDefinition;
class UTMOPNewspaperItemDefinition;
class SEditableTextBox;
class STextBlock;
class SVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTMOPPauseMenuRequestSignature);

UENUM()
enum class ETMOPPauseHubSection : uint8
{
    Inventory, Evidence, Sources, Publications, Settings, Controls, SaveLoad, Quit,
    MoveInTime
};

/** Paused main hub for inventory, research, publications and game management. */
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
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Pause|Save")
    FString SaveSlotName = TEXT("TMOP_QuickSave");
    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Pause|Save")
    bool LoadQuickSave(const FString& SlotName);
    UFUNCTION(BlueprintCallable, Category="TMOP|UI|Pause")
    void OpenSettingsPage();
    /** DT_TMOP_Uppslag_REGISTER. Assign the latest register in the HUD widget defaults. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|UI|Pause|Sources")
    TObjectPtr<UDataTable> UppslagTable;

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry,
        const FKeyEvent& InKeyEvent) override;

private:
    FReply HandleResumeClicked();
    FReply HandleSectionClicked(ETMOPPauseHubSection Section);
    FReply HandleSourceMainSectionClicked(FName MainSectionId);
    FReply HandleSourceSeriesClicked(FName SeriesId);
    FReply HandleSourceBackClicked();
    FReply HandleEquipItem(UTMOPItemDefinition* Item);
    FReply HandleOpenPublication(UTMOPNewspaperItemDefinition* Newspaper);
    FReply HandleGraphicsQuality(int32 Quality);
    FReply HandleToggleWorldLabels();
    FReply HandleToggleMinimap();
    FReply HandleToggleVSync();
    FReply HandleSaveClicked();
    FReply HandleLoadClicked();
    FReply HandleQuitClicked();
    FReply HandleMoveInTimeClicked();
    void ShowSection(ETMOPPauseHubSection Section);
    void BuildInventoryPage();
    void BuildEvidencePage();
    void BuildSourcesPage();
    void BuildPublicationsPage();
    void BuildSettingsPage();
    void BuildControlsPage();
    void BuildSaveLoadPage();
    void BuildQuitPage();
    void BuildMoveInTimePage();
    void SetStatus(const FText& Text);
    void AddHeading(const FText& Text);
    void AddBody(const FText& Text);

    UPROPERTY(Transient) TObjectPtr<APlayerController> PlayerController;
    UPROPERTY(Transient) TObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;
    TSharedPtr<SVerticalBox> ContentBox;
    TSharedPtr<STextBlock> SectionTitleText;
    TSharedPtr<STextBlock> StatusText;
    TSharedPtr<SEditableTextBox> TimeEntryBox;
    ETMOPPauseHubSection CurrentSection = ETMOPPauseHubSection::Inventory;
    FName SelectedSourceMainSection = NAME_None;
    FName SelectedSourceSeries = NAME_None;
    bool bWorldLabelsVisible = true;
};
