#include "Audio/TMOPVenueAudioEmitter.h"

#include "Anchors/TMOPAnchorSubsystem.h"
#include "Anchors/TMOPHistoricalAnchor.h"
#include "Audio/TMOPAudioDirector.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/DataTable.h"
#include "EngineUtils.h"
#include "Time/TMOPClockSubsystem.h"

ATMOPVenueAudioEmitter::ATMOPVenueAudioEmitter()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.2f;
    AudioOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("AudioOrigin"));
    SetRootComponent(AudioOrigin);
}

void ATMOPVenueAudioEmitter::BeginPlay()
{
    Super::BeginPlay();
    RefreshFromVenueTable();
    if (bSnapToPlacementAnchorOnBeginPlay && !VenueProfile.PlacementAnchorId.IsNone())
    {
        UTMOPAnchorSubsystem* Anchors = GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPAnchorSubsystem>() : nullptr;
        if (Anchors != nullptr)
            if (ATMOPHistoricalAnchor* Anchor = Anchors->FindAnchor(VenueProfile.PlacementAnchorId))
                SetActorLocation(Anchor->GetActorLocation());
    }
}

void ATMOPVenueAudioEmitter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopAllSlots();
    Super::EndPlay(EndPlayReason);
}

void ATMOPVenueAudioEmitter::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UTMOPClockSubsystem* Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;
    if (Clock == nullptr) return;
    const int32 CurrentSecond = Clock->GetCurrentTime().ToSecondsFromMidnight();
    if (LastEvaluatedSecond != INDEX_NONE && CurrentSecond < LastEvaluatedSecond)
        StopAllSlots();
    if (CurrentSecond != LastEvaluatedSecond)
    {
        EvaluateSlots(CurrentSecond);
        LastEvaluatedSecond = CurrentSecond;
    }
}

void ATMOPVenueAudioEmitter::RefreshFromVenueTable()
{
    if (VenueAudioTable == nullptr || VenueId.IsNone()) return;
    if (const FTMOPVenueAudioRow* Row =
        VenueAudioTable->FindRow<FTMOPVenueAudioRow>(VenueId, TEXT("TMOP Venue Audio"), false))
        VenueProfile = *Row;
}

void ATMOPVenueAudioEmitter::EvaluateSlots(const int32 CurrentSecond)
{
    for (const FTMOPVenueAudioSlot& Slot : VenueProfile.AudioSlots)
    {
        if (!Slot.bEnabled || Slot.SlotId.IsNone()) continue;
        const bool bActive = CurrentSecond >= Slot.StartSecond &&
            (Slot.EndSecond < 0 || CurrentSecond < Slot.EndSecond);
        UAudioComponent* Existing = ActiveSlots.FindRef(Slot.SlotId).Get();
        if (!bActive)
        {
            if (IsValid(Existing)) Existing->FadeOut(Slot.FadeOutSeconds, 0.0f);
            ActiveSlots.Remove(Slot.SlotId);
            continue;
        }
        if (IsValid(Existing) && Existing->IsPlaying()) continue;
        for (TActorIterator<ATMOPAudioDirector> It(GetWorld()); It; ++It)
        {
            UAudioComponent* Started = It->PlayAttachedById(
                Slot.AudioId, GetRootComponent(), Slot.VolumeMultiplier);
            if (IsValid(Started))
            {
                Started->FadeIn(Slot.FadeInSeconds, Started->VolumeMultiplier, 0.0f);
                ActiveSlots.Add(Slot.SlotId, Started);
            }
            break;
        }
    }
}

void ATMOPVenueAudioEmitter::StopAllSlots()
{
    for (TPair<FName, TWeakObjectPtr<UAudioComponent>>& Pair : ActiveSlots)
        if (UAudioComponent* Audio = Pair.Value.Get()) Audio->Stop();
    ActiveSlots.Reset();
    LastEvaluatedSecond = INDEX_NONE;
}
