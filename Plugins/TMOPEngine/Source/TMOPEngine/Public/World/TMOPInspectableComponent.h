#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TMOPInspectableComponent.generated.h"

class USphereComponent;
class UTextRenderComponent;
class UWorld;

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

    /** Show a persistent in-world information marker above this reading point. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection|Indicator")
    bool bShowWorldIndicator = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection|Indicator",
        meta=(MakeEditWidget="true"))
    FVector WorldIndicatorOffset = FVector(0.0f, 0.0f, 45.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection|Indicator",
        meta=(ClampMin="8.0", ClampMax="100.0", Units="cm"))
    float WorldIndicatorSize = 30.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection|Indicator",
        meta=(ClampMin="100.0", Units="cm"))
    float WorldIndicatorMaxDistanceCm = 2500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection|Indicator")
    FColor WorldIndicatorColor = FColor(100, 210, 245);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Inspection|Indicator")
    FText WorldIndicatorText = FText::FromString(TEXT("i"));

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual bool HasReadableContent() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionTitle() const;

    virtual FText GetWorldIndicatorTextAt(const FVector& ViewLocation) const;
    virtual float GetWorldIndicatorSizeAt(const FVector& ViewLocation) const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionText() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionCategory() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionSource() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FText GetInspectionAction() const;

    UFUNCTION(BlueprintPure, Category="Inspection")
    virtual FVector GetInteractionLocation() const;

    UFUNCTION(BlueprintPure, Category="Inspection|Indicator")
    FVector GetWorldIndicatorLocation() const;

    UFUNCTION(BlueprintPure, Category="Inspection|Indicator")
    bool ShouldShowWorldIndicatorAt(const FVector& ViewLocation) const;

    /** Returns all live address/information components in this PIE/game world. */
    static void GetActiveInWorld(const UWorld* World,
        TArray<UTMOPInspectableComponent*>& OutComponents);

    bool IsVisibleFrom(const FVector& ViewLocation, const AActor* Viewer) const;

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
        FActorComponentTickFunction* ThisTickFunction) override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<USphereComponent> InteractionShape;

    UPROPERTY(Transient)
    TObjectPtr<UTextRenderComponent> WorldIndicator;
};

