#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPInspectableComponent.generated.h"

class USphereComponent;

/** Shared target/reading behavior for resident directories and authored information. */
UCLASS(Abstract, ClassGroup=(TMOP))
class TMOPENGINE_API UTMOPInspectableComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UTMOPInspectableComponent();

    /** Local reading point, in centimetres. Anchor/actor transforms are not changed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection",
        meta=(MakeEditWidget="true"))
    FVector InteractionOffset = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Inspection",
        meta=(ClampMin="5.0", ClampMax="100.0", Units="cm"))
    float InteractionRadiusCm = 45.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection")
    bool bInteractionEnabled = true;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual bool HasReadableContent() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionTitle() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionText() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionCategory() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionSource() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionAction() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    FVector GetInteractionLocation() const;

    bool IsVisibleFrom(const FVector& ViewLocation, const AActor* Viewer) const;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<USphereComponent> InteractionShape;
};
