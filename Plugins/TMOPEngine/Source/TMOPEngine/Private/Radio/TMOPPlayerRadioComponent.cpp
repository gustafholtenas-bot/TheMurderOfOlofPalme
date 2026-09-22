#include "Radio/TMOPPlayerRadioComponent.h"
#include "Time/TMOPClockSubsystem.h"
#include "Engine/GameInstance.h"

#include "Components/AudioComponent.h"
#include "Inventory/TMOPInventoryComponent.h"
#include "Inventory/TMOPItemDefinition.h"
#include "Sound/SoundBase.h"
#include "Player/TMOPLocalMultiplayerSubsystem.h"
#include "Player/TMOPPlayerCharacter.h"

UTMOPPlayerRadioComponent::UTMOPPlayerRadioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UTMOPPlayerRadioComponent::BeginPlay()
{
    Super::BeginPlay();
    Inventory = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryComponent>() : nullptr;
    InventoryInput = GetOwner() != nullptr
        ? GetOwner()->FindComponentByClass<UTMOPInventoryInputComponent>() : nullptr;
    if (GetOwner() != nullptr)
    {
        AudioComponent = NewObject<UAudioComponent>(GetOwner(), TEXT("TMOPRadioAudio"));
        if (IsValid(AudioComponent.Get()))
        {
            AudioComponent->bAutoActivate = false;
            AudioComponent->bAllowSpatialization = false;
            AudioComponent->SetVolumeMultiplier(Volume);
            AudioComponent->RegisterComponent();
        }
    }
    if (IsValid(InventoryInput.Get()))
        InventoryInput->OnEquippedItemInput.AddDynamic(this,
            &UTMOPPlayerRadioComponent::HandleItemInput);
    if (IsValid(Inventory.Get()))
        Inventory->OnEquippedItemChanged.AddDynamic(this,
            &UTMOPPlayerRadioComponent::HandleEquippedItemChanged);
}

bool UTMOPPlayerRadioComponent::IsRadioEquipped() const
{
    const UTMOPItemDefinition* Item = IsValid(Inventory.Get())
        ? Inventory->EquippedItem.Get() : nullptr;
    return IsValid(Item) && Item->ItemType == ETMOPItemType::Radio;
}

void UTMOPPlayerRadioComponent::SelectSharedOutput(const bool bPreferThisReceiver)
{
    TArray<UTMOPPlayerRadioComponent*> Receivers;
    for (auto* Player : UTMOPLocalMultiplayerSubsystem::GetPlayers(this))
        if (IsValid(Player) && !Player->IsActorBeingDestroyed())
            if (auto* Radio = Player->FindComponentByClass<UTMOPPlayerRadioComponent>())
                Receivers.Add(Radio);
    UTMOPPlayerRadioComponent* Audible = bPreferThisReceiver && bRadioOn ? this : nullptr;
    if (!Audible)
        for (auto* Radio : Receivers)
            if (Radio->bRadioOn && !Radio->bSharedOutputMuted) { Audible = Radio; break; }
    if (!Audible)
        for (auto* Radio : Receivers)
            if (Radio->bRadioOn) { Audible = Radio; break; }
    // Stop every previous receiver before starting the selected one.
    for (auto* Radio : Receivers)
    {
        Radio->bSharedOutputMuted = Radio->bRadioOn && Radio != Audible;
        if (Radio != Audible && Radio->AudioComponent) Radio->AudioComponent->Stop();
    }
    for (auto* Radio : Receivers) Radio->RefreshBroadcast(Radio == Audible);
    if (!Receivers.Contains(this)) RefreshBroadcast(true);
}

void UTMOPPlayerRadioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    bRadioOn = false;
    if (AudioComponent) AudioComponent->Stop();
    SelectSharedOutput(false);
    Super::EndPlay(EndPlayReason);
}

void UTMOPPlayerRadioComponent::SetRadioOn(const bool bEnabled)
{
    const bool bNewValue = bEnabled && (!bRequireRadioItemEquipped || IsRadioEquipped());
    if (bRadioOn == bNewValue) return;
    bRadioOn = bNewValue;
    SelectSharedOutput(bRadioOn);
}

void UTMOPPlayerRadioComponent::ToggleRadio()
{
    SetRadioOn(!bRadioOn);
}

bool UTMOPPlayerRadioComponent::SetChannelById(const FName ChannelId)
{
    if (!IsValid(Schedule.Get())) return false;
    const int32 Index = Schedule->FindChannelIndex(ChannelId);
    if (Index == INDEX_NONE) return false;
    CurrentChannelIndex = Index;
    SelectSharedOutput(bRadioOn);
    return true;
}

bool UTMOPPlayerRadioComponent::CycleChannel(const int32 Direction)
{
    if (!IsValid(Schedule.Get()) || Schedule->Channels.IsEmpty()) return false;
    const int32 Step = Direction >= 0 ? 1 : -1;
    CurrentChannelIndex = (CurrentChannelIndex + Step + Schedule->Channels.Num()) %
        Schedule->Channels.Num();
    SelectSharedOutput(bRadioOn);
    return true;
}

void UTMOPPlayerRadioComponent::SetSimulationSecondOfDay(const int32 SecondOfDay)
{
    const int32 Normalized = ((SecondOfDay % 86400) + 86400) % 86400;
    if (CurrentSecondOfDay == Normalized) return;
    const FName PreviousSegment = ActiveSegmentId;
    CurrentSecondOfDay = Normalized;
    const FTMOPRadioProgramSegment* Segment = IsValid(Schedule.Get())
        ? Schedule->FindSegment(CurrentChannelIndex, CurrentSecondOfDay) : nullptr;
    const FName NewSegment = Segment != nullptr ? Segment->SegmentId : NAME_None;
    RefreshBroadcast(NewSegment != PreviousSegment);
}

void UTMOPPlayerRadioComponent::SetSimulationTime(const int32 Hour,
    const int32 Minute, const int32 Second)
{
    SetSimulationSecondOfDay(FMath::Clamp(Hour, 0, 23) * 3600 +
        FMath::Clamp(Minute, 0, 59) * 60 + FMath::Clamp(Second, 0, 59));
}

void UTMOPPlayerRadioComponent::RefreshBroadcast(const bool bForceRestart)
{
    if (!IsValid(AudioComponent.Get())) return;
    if (!bRadioOn || bSharedOutputMuted || !IsValid(Schedule.Get()) ||
        !Schedule->Channels.IsValidIndex(CurrentChannelIndex))
    {
        AudioComponent->Stop();
        ActiveSegmentId = NAME_None;
        BroadcastState();
        return;
    }
    const FTMOPRadioProgramSegment* Segment =
        Schedule->FindSegment(CurrentChannelIndex, CurrentSecondOfDay);
    if (Segment == nullptr || !IsValid(Segment->Audio.Get()))
    {
        AudioComponent->Stop();
        ActiveSegmentId = NAME_None;
        BroadcastState();
        return;
    }
    if (bForceRestart || ActiveSegmentId != Segment->SegmentId ||
        AudioComponent->Sound != Segment->Audio.Get())
    {
        ActiveSegmentId = Segment->SegmentId;
        AudioComponent->SetSound(Segment->Audio.Get());
        AudioComponent->SetVolumeMultiplier(Volume);
        float StartOffset = Segment->AudioStartOffsetSeconds +
            static_cast<float>(CurrentSecondOfDay - Segment->StartSecondOfDay);
        const float Duration = Segment->Audio->GetDuration();
        if (Segment->bLoopAudioWithinSegment && Duration > KINDA_SMALL_NUMBER)
            StartOffset = FMath::Fmod(StartOffset, Duration);
        AudioComponent->Play(FMath::Max(0.0f, StartOffset));
    }
    BroadcastState();
}

FTMOPRadioChannel UTMOPPlayerRadioComponent::GetCurrentChannel() const
{
    return IsValid(Schedule.Get()) && Schedule->Channels.IsValidIndex(CurrentChannelIndex)
        ? Schedule->Channels[CurrentChannelIndex] : FTMOPRadioChannel();
}

FText UTMOPPlayerRadioComponent::GetCurrentProgramName() const
{
    const FTMOPRadioProgramSegment* Segment = IsValid(Schedule.Get())
        ? Schedule->FindSegment(CurrentChannelIndex, CurrentSecondOfDay) : nullptr;
    const FText Name = Segment != nullptr ? Segment->DisplayName : FText::GetEmpty();
    return bSharedOutputMuted ? FText::Format(NSLOCTEXT("TMOP", "SharedRadioOutput",
        "{0} (annan spelares radio hörs – byt kanal för att lyssna här)"), Name) : Name;
}

void UTMOPPlayerRadioComponent::BroadcastState()
{
    const FTMOPRadioChannel Channel = GetCurrentChannel();
    OnRadioStateChanged.Broadcast(bRadioOn, Channel.ChannelId,
        Channel.DisplayName, GetCurrentProgramName());
}

void UTMOPPlayerRadioComponent::HandleItemInput(UTMOPItemDefinition* Item,
    const ETMOPItemInput Input, const ETMOPItemInputPhase Phase)
{
    if (!IsValid(Item) || Item->ItemType != ETMOPItemType::Radio ||
        Phase != ETMOPItemInputPhase::Started) return;
    if (Input == ETMOPItemInput::Primary) ToggleRadio();
    else if (Input == ETMOPItemInput::Secondary) CycleChannel(1);
    else if (Input == ETMOPItemInput::Alternate) CycleChannel(-1);
}

void UTMOPPlayerRadioComponent::HandleEquippedItemChanged(
    UTMOPItemDefinition* PreviousItem, UTMOPItemDefinition* NewItem)
{
    if (bRequireRadioItemEquipped &&
        (!IsValid(NewItem) || NewItem->ItemType != ETMOPItemType::Radio))
        SetRadioOn(false);
}

void UTMOPPlayerRadioComponent::RestoreAfterTimeSeek()
{
    if (GetWorld() && GetWorld()->GetGameInstance())
        CurrentSecondOfDay = GetWorld()->GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()->GetCurrentTime().ToSecondsFromMidnight();
    RefreshBroadcast(true);
}
