#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPHUDTimelineWidget.generated.h"

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHUDTimelineMarker
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timeline")
    FText Label;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Timeline")
    FName SharedEventId;
};

UCLASS()
class TMOPENGINE_API UTMOPHUDTimelineWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    TArray<FTMOPHUDTimelineMarker> Markers;
    void SetTimelineInputEnabled(bool bEnabled);
    bool IsTimelineInputEnabled() const { return bTimelineInputEnabled; }
    bool bTimelineInputEnabled = false;
    TSharedPtr<SWidget> TimelineSlate;
    virtual void NativeDestruct() override;
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
};
