#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Items/TMOPInteractable.h"
#include "TMOPWorldItem.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UTextRenderComponent;
class UTMOPInventoryComponent;
class UTMOPItemDefinition;

/** Generic item representation placed or dropped in the world. */
UCLASS(BlueprintType, Blueprintable)
class TMOPENGINE_API ATMOPWorldItem : public AActor, public ITMOPInteractable
{
    GENERATED_BODY()

public:
    ATMOPWorldItem();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnConstruction(const FTransform& Transform) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|World Item")
    TObjectPtr<USphereComponent> InteractionCollision;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|World Item")
    TObjectPtr<UStaticMeshComponent> ItemMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="TMOP|World Item|Prompt")
    TObjectPtr<UTextRenderComponent> WorldPrompt;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Item|Prompt")
    float PromptMinimumDistanceCm = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Item|Prompt")
    float PromptMaximumDistanceCm = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Item|Prompt")
    float PromptHeightCm = 75.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|World Item")
    TObjectPtr<UTMOPItemDefinition> ItemDefinition;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|World Item",
        meta=(ClampMin="1"))
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|World Item")
    bool bUseDefinitionMesh = true;

    UFUNCTION(BlueprintCallable, Category="TMOP|World Item")
    void ConfigureWorldItem(UTMOPItemDefinition* NewItem, int32 NewQuantity = 1);

    UFUNCTION(BlueprintCallable, Category="TMOP|World Item")
    bool TryPickup(UTMOPInventoryComponent* TargetInventory);

    virtual bool Interact_Implementation(AActor* Interactor) override;
    virtual FText GetInteractionText_Implementation() const override;

private:
    void RefreshVisual();
    void UpdateWorldPrompt();
};
