#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Time/TMOPTime.h"
#include "TMOPPlayerHUDDataComponent.generated.h"

class UTexture2D;
class UTMOPClockSubsystem;
class UTMOPPlayerRadioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTMOPHUDDataChangedSignature);

/** Lightweight data source for the future HUD widget. */
UCLASS(ClassGroup=(TMOP), BlueprintType, Blueprintable,
    meta=(BlueprintSpawnableComponent))
class TMOPENGINE_API UTMOPPlayerHUDDataComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTMOPPlayerHUDDataComponent();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    FText SimulationTimeText;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    FText EquippedItemName;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    TObjectPtr<UTexture2D> EquippedItemIcon;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD")
    bool bMapUnlocked = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD|Radio")
    bool bRadioOn = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD|Radio")
    FName RadioChannelId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD|Radio")
    FText RadioChannelName;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|HUD|Radio")
    FText RadioProgramName;

    UPROPERTY(BlueprintAssignable, Category="TMOP|HUD")
    FTMOPHUDDataChangedSignature OnHUDDataChanged;

    /** Manual override remains available; BeginPlay now binds to the central clock. */
    UFUNCTION(BlueprintCallable, Category="TMOP|HUD")
    void SetSimulationTime(int32 Hour, int32 Minute, int32 Second);

    UFUNCTION(BlueprintCallable, Category="TMOP|HUD")
    void SetMapUnlocked(bool bUnlocked);

private:
    UFUNCTION()
    void HandleClockSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleRadioStateChanged(bool bNewRadioOn, FName ChannelId,
        FText ChannelName, FText ProgramName);

    UFUNCTION()
    void HandleEquippedItemChanged(UTMOPItemDefinition* PreviousItem,
        UTMOPItemDefinition* NewItem);

    UPROPERTY(Transient)
    TObjectPtr<UTMOPInventoryComponent> Inventory;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPClockSubsystem> Clock;

    UPROPERTY(Transient)
    TObjectPtr<UTMOPPlayerRadioComponent> Radio;
};
