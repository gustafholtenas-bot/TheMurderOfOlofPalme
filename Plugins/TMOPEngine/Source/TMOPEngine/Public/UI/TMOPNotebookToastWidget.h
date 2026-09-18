#pragma once
#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TMOPNotebookToastWidget.generated.h"

class STextBlock;

UCLASS()
class TMOPENGINE_API UTMOPNotebookToastWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    void ShowObservation(const FText& Name);
protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeTick(const FGeometry& Geometry, float DeltaSeconds) override;
    virtual void ReleaseSlateResources(bool bReleaseChildren) override;
private:
    TSharedPtr<STextBlock> Message;
    double HideAt = 0.0;
};
