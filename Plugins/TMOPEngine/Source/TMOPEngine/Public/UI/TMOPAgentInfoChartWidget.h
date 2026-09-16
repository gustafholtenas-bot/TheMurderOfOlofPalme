#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "People/TMOPPersonProfileTypes.h"
#include "Styling/SlateBrush.h"
#include "TMOPAgentInfoChartWidget.generated.h"

class ATMOPPlayerCharacter;
class SBorder;
class SImage;
class STextBlock;
class SScrollBox;

/** Full-screen research card opened when the player interacts with a person. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API UTMOPAgentInfoChartWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void InitializeAgentInfo(ATMOPPlayerCharacter* InPlayerCharacter);
    void ShowAgentInfo(const FTMOPPersonProfileRow& Profile,
        const FText& TimelineSummary, bool bPoliceInterviewed,
        FName InspectedEntityId);
    void HideAgentInfo();

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual FReply NativeOnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;

private:
    FReply HandleCloseClicked();
    void RefreshVisibility();
    void RefreshEvidenceGallery(const FTMOPPersonProfileRow& Profile);
    FText BuildObserverSummary(FName InspectedEntityId) const;

    TWeakObjectPtr<ATMOPPlayerCharacter> PlayerCharacter;
    TSharedPtr<SBorder> MainPanel;
    TSharedPtr<SScrollBox> ScrollBox;
    TSharedPtr<STextBlock> NameText;
    TSharedPtr<STextBlock> IdentityText;
    TSharedPtr<STextBlock> InterviewStatusText;
    TSharedPtr<STextBlock> TimelineText;
    TSharedPtr<STextBlock> ObservationText;
    TSharedPtr<STextBlock> ObserversText;
    TSharedPtr<STextBlock> PostMurderEventsText;
    TSharedPtr<STextBlock> SourceText;
    TSharedPtr<SScrollBox> EvidenceGallery;
    TSharedPtr<STextBlock> EvidenceGalleryPlaceholder;
    TArray<TSharedPtr<FSlateBrush>> EvidenceImageBrushes;
    bool bChartVisible = false;
};
