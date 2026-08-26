#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "People/TMOPPersonProfileTypes.h"
#include "TMOPAgentInfoChartWidget.generated.h"

class ATMOPPlayerCharacter;
class SBorder;
class STextBlock;

/** Full-screen research card opened when the player interacts with a person. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPAgentInfoChartWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeAgentInfo(ATMOPPlayerCharacter* InPlayerCharacter);
    void ShowAgentInfo(const FTMOPPersonProfileRow& Profile,
        const FText& TimelineSummary, bool bPoliceInterviewed);
    void HideAgentInfo();

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    FReply HandleCloseClicked();
    void RefreshVisibility();

    TWeakObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;
    TSharedPtr<SBorder> MainPanel;
    TSharedPtr<STextBlock> NameText;
    TSharedPtr<STextBlock> IdentityText;
    TSharedPtr<STextBlock> InterviewStatusText;
    TSharedPtr<STextBlock> TimelineText;
    TSharedPtr<STextBlock> ObservationText;
    TSharedPtr<STextBlock> SourceText;
    bool bChartVisible = false;
};
