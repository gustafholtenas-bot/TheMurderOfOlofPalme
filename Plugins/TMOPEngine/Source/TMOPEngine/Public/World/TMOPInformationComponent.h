#pragma once

#include "CoreMinimal.h"
#include "Time/TMOPTime.h"
#include "World/TMOPInspectableComponent.h"
#include "TMOPInformationComponent.generated.h"

/** Authored information on any actor. No address row, anchor or DataTable is required. */
UCLASS(ClassGroup=(TMOP), meta=(BlueprintSpawnableComponent, DisplayName="TMOP Information"))
class TMOPENGINE_API UTMOPInformationComponent : public UTMOPInspectableComponent
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information", meta=(DisplayPriority="0"))
    FText Title;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information",
        meta=(MultiLine="true", DisplayPriority="1"))
    FText Description;

    /** Optional heading, e.g. Företag, Händelse, Plats or Pågår nu. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information", meta=(DisplayPriority="2"))
    FText CategoryLabel;

    /** Optional source text shown below the description; not treated as executable markup. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information",
        meta=(MultiLine="true", DisplayPriority="3"))
    FText SourceReference;

    /** Off by default: historical background can be read throughout the whole loop. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information|Time")
    bool bUseTimeWindow = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information|Time",
        meta=(EditCondition="bUseTimeWindow", EditConditionHides))
    FTMOPTime VisibleFrom = FTMOPTime(23, 0, 0);

    /** Exclusive end. A window crossing midnight is supported; equal times disable it. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Information|Time",
        meta=(EditCondition="bUseTimeWindow", EditConditionHides))
    FTMOPTime VisibleUntil = FTMOPTime(23, 45, 0);

    UFUNCTION(BlueprintPure, Category="Information|Time")
    bool IsAvailableAtTime(FTMOPTime Time) const;

    virtual bool HasReadableContent() const override;
    virtual FText GetInspectionTitle() const override;
    virtual FText GetInspectionText() const override;
    virtual FText GetInspectionCategory() const override;
    virtual FText GetInspectionSource() const override;
};
