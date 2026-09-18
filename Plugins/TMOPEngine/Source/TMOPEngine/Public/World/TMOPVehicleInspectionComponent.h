#pragma once

#include "CoreMinimal.h"
#include "World/TMOPInspectableComponent.h"
#include "TMOPVehicleInspectionComponent.generated.h"

/** Read-only dossier for the green observed vehicles; collected on closing it. */
UCLASS(ClassGroup=(TMOP))
class TMOPENGINE_API UTMOPVehicleInspectionComponent : public UTMOPInspectableComponent
{
    GENERATED_BODY()
public:
    UTMOPVehicleInspectionComponent();
    static FVector GetVehicleAimPoint(const AActor* Vehicle);
    virtual FVector GetInteractionLocation() const override;
    virtual bool HasReadableContent() const override;
    virtual FText GetInspectionTitle() const override;
    virtual FText GetInspectionText() const override;
    virtual FText GetInspectionCategory() const override;
    virtual FText GetInspectionSource() const override;
    virtual FText GetInspectionAction() const override;
};
