#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPLocalPlayerOverlay.generated.h"

class UTMOPInspectableComponent;

/** Player-specific clock, shared pause indicator and per-view world labels. */
UCLASS()
class TMOPENGINE_API UTMOPLocalPlayerOverlay : public UUserWidget
{
    GENERATED_BODY()
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaTime) override;
    virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& Geometry,
        const FSlateRect& CullingRect, FSlateWindowElementList& Elements, int32 LayerId,
        const FWidgetStyle& Style, bool bParentEnabled) const override;
private:
    FText GetStatus() const;
    TArray<TWeakObjectPtr<AActor>> NearbyLabels;
    TArray<TWeakObjectPtr<UTMOPInspectableComponent>> NearbyInspectables;
    float RefreshElapsed = 1.0f;
};
