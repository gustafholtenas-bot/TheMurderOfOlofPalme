#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Traffic/TMOPTrafficSignalTypes.h"
#include "TMOPTrafficSignalComponent.generated.h"
class UMeshComponent;
class UMaterialInstanceDynamic;

/** One independently controlled head, on any actor in the level. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable, meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPTrafficSignalComponent : public USceneComponent
{
    GENERATED_BODY()
public:
    UTMOPTrafficSignalComponent();
    virtual void BeginPlay() override;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    FName IntersectionId = NAME_None;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal")
    FName SignalGroupId = NAME_None;
    UPROPERTY(BlueprintReadOnly, Category="TMOP|Traffic Signal")
    ETMOPTrafficSignalState CurrentState = ETMOPTrafficSignalState::Red;
    /** Select explicitly if the owner contains more than one mesh. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Materials")
    TObjectPtr<UMeshComponent> TargetMesh;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Materials")
    bool bPedestrian = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Materials")
    FName RedSlot = TEXT("Vehicle_Red");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Materials")
    FName YellowSlot = TEXT("Vehicle_Yellow");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Materials")
    FName GreenSlot = TEXT("Vehicle_Green");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Traffic Signal|Materials")
    FName LampParameter = TEXT("LampOn");
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    bool ValidateMaterials(TArray<FString>& OutErrors) const;
    UMeshComponent* ResolveMesh() const;
    FName GetLampSlot(int32 Lamp) const;
    UFUNCTION(BlueprintCallable, Category="TMOP|Traffic Signal")
    void ApplySignalState(ETMOPTrafficSignalState NewState);
    UFUNCTION(BlueprintImplementableEvent, Category="TMOP|Traffic Signal")
    void OnSignalVisualChanged(ETMOPTrafficSignalState NewState);
private:
    UPROPERTY(Transient) TArray<TObjectPtr<UMaterialInstanceDynamic>> LampMaterials;
};
