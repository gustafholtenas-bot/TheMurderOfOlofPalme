#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TMOPInformationPoint.generated.h"

class UTMOPInformationComponent;
class UTextRenderComponent;

/** Place anywhere, then select Information in Details to enter the text. */
UCLASS(Blueprintable, meta=(DisplayName="TMOP Information Point"))
class TMOPENGINE_API ATMOPInformationPoint : public AActor
{
    GENERATED_BODY()
public:
    ATMOPInformationPoint();
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Information")
    TObjectPtr<UTMOPInformationComponent> Information;

#if WITH_EDITORONLY_DATA
    UPROPERTY(VisibleAnywhere, Category="Information")
    TObjectPtr<UTextRenderComponent> EditorLabel;
#endif
};
