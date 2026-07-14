#include "UI/TMOPPlayerHUDDataComponent.h"

#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Engine/GameInstance.h"
#include "Radio/TMOPPlayerRadioComponent.h"
#include "Time/TMOPClockSubsystem.h"

UTMOPPlayerHUDDataComponent::UTMOPPlayerHUDDataComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
    SimulationTimeText = FText::FromString(TEXT("23:00:00"));
}

void UTMOPPlayerHUDDataComponent::BeginPlay()
{
    Super::BeginPlay();
    Inventory = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryComponent>() : nullptr;
    if (IsValid(Inventory.Get()))
    {
        Inventory->OnEquippedItemChanged.AddDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleEquippedItemChanged);
        HandleEquippedItemChanged(nullptr, Inventory->EquippedItem.Get());
    }

    Radio = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPPlayerRadioComponent>() : nullptr;
    if (IsValid(Radio.Get()))
    {
        Radio->OnRadioStateChanged.AddDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleRadioStateChanged);
        const FTMOPRadioChannel Channel = Radio->GetCurrentChannel();
        HandleRadioStateChanged(Radio->bRadioOn, Channel.ChannelId,
            Channel.DisplayName, Radio->GetCurrentProgramName());
    }

    UGameInstance* GameInstance = GetWorld() != nullptr
        ? GetWorld()->GetGameInstance() : nullptr;
    Clock = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (IsValid(Clock.Get()))
    {
        Clock->OnSecondChanged.AddDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleClockSecondChanged);
        HandleClockSecondChanged(Clock->GetCurrentTime());
    }
}

void UTMOPPlayerHUDDataComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (IsValid(Clock.Get()))
        Clock->OnSecondChanged.RemoveDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleClockSecondChanged);
    if (IsValid(Radio.Get()))
        Radio->OnRadioStateChanged.RemoveDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleRadioStateChanged);
    if (IsValid(Inventory.Get()))
        Inventory->OnEquippedItemChanged.RemoveDynamic(this,
            &UTMOPPlayerHUDDataComponent::HandleEquippedItemChanged);
    Super::EndPlay(EndPlayReason);
}

void UTMOPPlayerHUDDataComponent::HandleClockSecondChanged(const FTMOPTime NewTime)
{
    SetSimulationTime(NewTime.Hour, NewTime.Minute, NewTime.Second);
    if (IsValid(Radio.Get()))
        Radio->SetSimulationTime(NewTime.Hour, NewTime.Minute, NewTime.Second);
}

void UTMOPPlayerHUDDataComponent::HandleRadioStateChanged(const bool bNewRadioOn,
    const FName ChannelId, const FText ChannelName, const FText ProgramName)
{
    bRadioOn = bNewRadioOn;
    RadioChannelId = ChannelId;
    RadioChannelName = ChannelName;
    RadioProgramName = ProgramName;
    OnHUDDataChanged.Broadcast();
}

void UTMOPPlayerHUDDataComponent::SetSimulationTime(const int32 Hour,
    const int32 Minute, const int32 Second)
{
    SimulationTimeText = FText::FromString(FString::Printf(TEXT("%02d:%02d:%02d"),
        FMath::Clamp(Hour, 0, 23), FMath::Clamp(Minute, 0, 59),
        FMath::Clamp(Second, 0, 59)));
    OnHUDDataChanged.Broadcast();
}

void UTMOPPlayerHUDDataComponent::SetMapUnlocked(const bool bUnlocked)
{
    if (bMapUnlocked == bUnlocked) return;
    bMapUnlocked = bUnlocked;
    OnHUDDataChanged.Broadcast();
}

void UTMOPPlayerHUDDataComponent::HandleEquippedItemChanged(
    UTMOPItemDefinition* PreviousItem, UTMOPItemDefinition* NewItem)
{
    EquippedItemName = IsValid(NewItem) ? NewItem->DisplayName : FText::GetEmpty();
    EquippedItemIcon = IsValid(NewItem) ? NewItem->Icon : nullptr;
    OnHUDDataChanged.Broadcast();
}
