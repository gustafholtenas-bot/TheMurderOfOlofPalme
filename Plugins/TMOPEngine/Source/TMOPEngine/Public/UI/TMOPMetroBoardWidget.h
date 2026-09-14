#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPMetroBoardWidget.generated.h"

/** Native screen-space arrival board shown above a metro entrance. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPMetroBoardWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetBoard(const FText& Station, const FText& Arrivals,
        const FText& Disclaimer);

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;

private:
    TSharedPtr<class STextBlock> StationText;
    TSharedPtr<class STextBlock> ArrivalText;
    TSharedPtr<class STextBlock> DisclaimerText;
    FText PendingStation;
    FText PendingArrivals;
    FText PendingDisclaimer;
};

