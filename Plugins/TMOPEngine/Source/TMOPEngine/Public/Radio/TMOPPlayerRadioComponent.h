#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/TMOPInventoryInputComponent.h"
#include "Radio/TMOPRadioScheduleData.h"
#include "TMOPPlayerRadioComponent.generated.h"

class UAudioComponent;
class UTMOPInventoryComponent;
class UTMOPInventoryInputComponent;
class UTMOPItemDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FTMOPRadioStateSignature,
    bool, bRadioOn, FName, ChannelId, FText, ChannelName, FText, ProgramName);

/** Player radio receiver with scenario-time synchronised programmes and police channels. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerRadioComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerRadioComponent();
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Radio")
    TObjectPtr<UTMOPRadioScheduleData> Schedule;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Radio",
        meta=(ClampMin="0.0", ClampMax="2.0"))
    float Volume = 0.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Radio")
    bool bRequireRadioItemEquipped = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Radio")
    bool bRadioOn = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Radio")
    int32 CurrentChannelIndex = 0;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Radio")
    int32 CurrentSecondOfDay = 82800;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Radio")
    FName ActiveSegmentId = NAME_None;

    UPROPERTY(BlueprintAssignable, Category="TMOP|Radio|Events")
    FTMOPRadioStateSignature OnRadioStateChanged;

    UFUNCTION(BlueprintCallable, Category="TMOP|Radio")
    void SetRadioOn(bool bEnabled);

    UFUNCTION(BlueprintCallable, Category="TMOP|Radio")
    void ToggleRadio();

    UFUNCTION(BlueprintCallable, Category="TMOP|Radio")
    bool SetChannelById(FName ChannelId);

    UFUNCTION(BlueprintCallable, Category="TMOP|Radio")
    bool CycleChannel(int32 Direction = 1);

    UFUNCTION(BlueprintCallable, Category="TMOP|Radio|Time")
    void SetSimulationSecondOfDay(int32 SecondOfDay);

    UFUNCTION(BlueprintCallable, Category="TMOP|Radio|Time")
    void SetSimulationTime(int32 Hour, int32 Minute, int32 Second);

    UFUNCTION(BlueprintPure, Category="TMOP|Radio")
    FTMOPRadioChannel GetCurrentChannel() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Radio")
    FText GetCurrentProgramName() const;

private:
    UFUNCTION()
    void HandleItemInput(UTMOPItemDefinition* Item, ETMOPItemInput Input,
        ETMOPItemInputPhase Phase);

    UFUNCTION()
    void HandleEquippedItemChanged(UTMOPItemDefinition* PreviousItem,
        UTMOPItemDefinition* NewItem);

    void RefreshBroadcast(bool bForceRestart);
    void BroadcastState();
    bool IsRadioEquipped() const;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> AudioComponent;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryComponent> Inventory;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryInputComponent> InventoryInput;
};
