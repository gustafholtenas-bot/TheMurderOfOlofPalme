#include "RecordedCalls/TMOPRecordedCallDirector.h"

#include "Components/AudioComponent.h"
#include "Engine/DataTable.h"
#include "Sound/SoundWave.h"
#include "Time/TMOPClockSubsystem.h"
#include "People/TMOPPersonRegistrySubsystem.h"

ATMOPRecordedCallDirector::ATMOPRecordedCallDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickInterval = 0.1f;
}

void ATMOPRecordedCallDirector::BeginPlay()
{
    Super::BeginPlay();

    Clock = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>() : nullptr;

    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.AddDynamic(
            this, &ATMOPRecordedCallDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.AddDynamic(
            this, &ATMOPRecordedCallDirector::HandleLoopRestarted);
    }

    ReloadRecordings();
    SetPlaybackEnabled(bAutoStart);
}

void ATMOPRecordedCallDirector::EndPlay(
    const EEndPlayReason::Type EndPlayReason)
{
    StopAllRecordings();
    if (Clock != nullptr)
    {
        Clock->OnSecondChanged.RemoveDynamic(
            this, &ATMOPRecordedCallDirector::HandleSecondChanged);
        Clock->OnLoopRestarted.RemoveDynamic(
            this, &ATMOPRecordedCallDirector::HandleLoopRestarted);
    }
    Super::EndPlay(EndPlayReason);
}

void ATMOPRecordedCallDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (Clock == nullptr || !bPlaybackEnabled)
    {
        return;
    }

    const float TimeScale = Clock->GetTimeScale();
    const bool bShouldPause = !Clock->IsClockRunning() ||
        TimeScale <= 0.0f || TimeScale > MaximumAudibleTimeScale;

    if (bShouldPause != bComponentsPaused)
    {
        for (const TPair<FName, int32>& Pair : ActiveComponentByRecording)
        {
            if (AudioPool.IsValidIndex(Pair.Value) && AudioPool[Pair.Value] != nullptr)
            {
                AudioPool[Pair.Value]->SetPaused(bShouldPause);
            }
        }
        bComponentsPaused = bShouldPause;
    }

    if (!bShouldPause)
    {
        for (const TPair<FName, int32>& Pair : ActiveComponentByRecording)
        {
            if (AudioPool.IsValidIndex(Pair.Value) && AudioPool[Pair.Value] != nullptr)
            {
                AudioPool[Pair.Value]->SetPitchMultiplier(
                    FMath::Clamp(TimeScale, 0.01f, MaximumAudibleTimeScale));
            }
        }
    }
}

bool ATMOPRecordedCallDirector::ReloadRecordings()
{
    StopAllRecordings();
    LoadedRows.Reset();

    if (RecordedCallTable == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP recorded-call director has no table."));
        return false;
    }
    if (RecordedCallTable->GetRowStruct() != FTMOPRecordedCallRow::StaticStruct())
    {
        UE_LOG(LogTemp, Error,
            TEXT("RecordedCallTable must use TMOP Recorded Call Row."));
        return false;
    }

    static const FString Context(TEXT("TMOPRecordedCallDirector"));
    TArray<FTMOPRecordedCallRow*> Rows;
    RecordedCallTable->GetAllRows(Context, Rows);
    for (const FTMOPRecordedCallRow* Row : Rows)
    {
        if (Row != nullptr && Row->RecordingDateISO == TEXT("1986-02-28"))
        {
            LoadedRows.Add(*Row);
        }
    }

    TArray<FString> Errors;
    ValidateRecordings(Errors);
    for (const FString& Error : Errors)
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP recorded calls: %s"), *Error);
    }

    LastClockSecond = INDEX_NONE;
    if (bPlaybackEnabled)
    {
        SynchronizeToClock();
    }
    return true;
}

void ATMOPRecordedCallDirector::SetPlaybackEnabled(const bool bEnabled)
{
    bPlaybackEnabled = bEnabled;
    if (bPlaybackEnabled)
    {
        SynchronizeToClock();
    }
    else
    {
        StopAllRecordings();
    }
}

void ATMOPRecordedCallDirector::SynchronizeToClock()
{
    StopAllRecordings();
    if (!bPlaybackEnabled || Clock == nullptr)
    {
        return;
    }

    const int32 Second = Clock->GetCurrentTime().ToSecondsFromMidnight();
    LastClockSecond = Second;
    for (const FTMOPRecordedCallRow& Row : LoadedRows)
    {
        if (PassesFilter(Row) && IsRowActiveAt(Row, Second))
        {
            StartRecording(Row, Second);
        }
    }
}

void ATMOPRecordedCallDirector::HandleSecondChanged(const FTMOPTime NewTime)
{
    if (!bPlaybackEnabled)
    {
        return;
    }
    const int32 Second = NewTime.ToSecondsFromMidnight();
    if (LastClockSecond != INDEX_NONE && Second != LastClockSecond + 1)
    {
        SynchronizeToClock();
        return;
    }
    LastClockSecond = Second;

    TSet<FName> Desired;
    for (const FTMOPRecordedCallRow& Row : LoadedRows)
    {
        if (!PassesFilter(Row) || !IsRowActiveAt(Row, Second))
        {
            continue;
        }
        Desired.Add(Row.RecordingId);
        if (!ActiveComponentByRecording.Contains(Row.RecordingId))
        {
            StartRecording(Row, Second);
        }
    }

    TArray<FName> ToStop;
    ActiveComponentByRecording.GetKeys(ToStop);
    for (const FName Id : ToStop)
    {
        if (!Desired.Contains(Id))
        {
            StopRecording(Id);
        }
    }
}

void ATMOPRecordedCallDirector::HandleLoopRestarted(
    int32 NewLoopNumber, FTMOPTime RestartTime)
{
    SynchronizeToClock();
}

bool ATMOPRecordedCallDirector::PassesFilter(
    const FTMOPRecordedCallRow& Row) const
{
    if (Row.CallType == ETMOPRecordedCallType::LAC)
    {
        return bPlayLAC;
    }
    if (Row.CallType == ETMOPRecordedCallType::PoliceRadio ||
        Row.CallType == ETMOPRecordedCallType::PoliceEmergencyCall)
    {
        return bPlayPoliceRadio;
    }
    return bPlayOtherTypes;
}

bool ATMOPRecordedCallDirector::IsRowActiveAt(
    const FTMOPRecordedCallRow& Row, const int32 SecondOfDay) const
{
    const int32 Start = Row.HistoricalStartTime.ToSecondsFromMidnight();
    if (SecondOfDay < Start)
    {
        return false;
    }
    if (Row.bHistoricalEndTimeKnown)
    {
        return SecondOfDay < Row.HistoricalEndTime.ToSecondsFromMidnight();
    }

    const USoundWave* Audio = Row.AudioFile.Get();
    if (Audio == nullptr)
    {
        return SecondOfDay == Start;
    }
    const float ClipEnd = Row.AudioClipEndOffsetSeconds > 0.0f
        ? Row.AudioClipEndOffsetSeconds : Audio->GetDuration();
    const float ClipDuration = FMath::Max(
        0.0f, ClipEnd - Row.AudioClipStartOffsetSeconds);
    return static_cast<float>(SecondOfDay - Start) < ClipDuration;
}

float ATMOPRecordedCallDirector::CalculateAudioOffset(
    const FTMOPRecordedCallRow& Row, const int32 SecondOfDay) const
{
    const int32 Start = Row.HistoricalStartTime.ToSecondsFromMidnight();
    return Row.AudioClipStartOffsetSeconds +
        static_cast<float>(FMath::Max(0, SecondOfDay - Start));
}

UAudioComponent* ATMOPRecordedCallDirector::AcquireAudioComponent(
    const FName RecordingId)
{
    for (int32 Index = 0; Index < AudioPool.Num(); ++Index)
    {
        UAudioComponent* Component = AudioPool[Index];
        if (Component != nullptr && !Component->IsPlaying() &&
            !ActiveComponentByRecording.FindKey(Index))
        {
            ActiveComponentByRecording.Add(RecordingId, Index);
            return Component;
        }
    }

    if (AudioPool.Num() >= MaximumConcurrentRecordings)
    {
        return nullptr;
    }

    UAudioComponent* Component = NewObject<UAudioComponent>(
        this, *FString::Printf(TEXT("TMOPRecordedCallAudio_%d"), AudioPool.Num()));
    Component->bAutoActivate = false;
    Component->bAllowSpatialization = false;
    Component->RegisterComponent();
    const int32 Index = AudioPool.Add(Component);
    ActiveComponentByRecording.Add(RecordingId, Index);
    return Component;
}

void ATMOPRecordedCallDirector::StartRecording(
    const FTMOPRecordedCallRow& Row, const int32 SecondOfDay)
{
    if (Row.RecordingId.IsNone() || ActiveComponentByRecording.Contains(Row.RecordingId))
    {
        return;
    }

    USoundWave* Audio = Row.AudioFile.LoadSynchronous();
    if (Audio == nullptr)
    {
        return;
    }
    const float Offset = CalculateAudioOffset(Row, SecondOfDay);
    const float ClipEnd = Row.AudioClipEndOffsetSeconds > 0.0f
        ? Row.AudioClipEndOffsetSeconds : Audio->GetDuration();
    if (Offset >= ClipEnd)
    {
        return;
    }

    UAudioComponent* Component = AcquireAudioComponent(Row.RecordingId);
    if (Component == nullptr)
    {
        UE_LOG(LogTemp, Warning,
            TEXT("No free recorded-call audio component for '%s'."),
            *Row.RecordingId.ToString());
        return;
    }
    Component->SetSound(Audio);
    Component->SetVolumeMultiplier(Volume);
    const float Scale = Clock != nullptr ? Clock->GetTimeScale() : 1.0f;
    Component->SetPitchMultiplier(FMath::Clamp(Scale, 0.01f, MaximumAudibleTimeScale));
    Component->Play(Offset);
    OnRecordingStarted.Broadcast(Row.RecordingId);
}

void ATMOPRecordedCallDirector::StopRecording(const FName RecordingId)
{
    const int32* Index = ActiveComponentByRecording.Find(RecordingId);
    if (Index != nullptr && AudioPool.IsValidIndex(*Index) && AudioPool[*Index] != nullptr)
    {
        AudioPool[*Index]->Stop();
    }
    if (ActiveComponentByRecording.Remove(RecordingId) > 0)
    {
        OnRecordingFinished.Broadcast(RecordingId);
    }
}

void ATMOPRecordedCallDirector::StopAllRecordings()
{
    TArray<FName> ActiveIds;
    ActiveComponentByRecording.GetKeys(ActiveIds);
    for (const FName Id : ActiveIds)
    {
        StopRecording(Id);
    }
    bComponentsPaused = false;
}

TArray<FName> ATMOPRecordedCallDirector::GetActiveRecordingIds() const
{
    TArray<FName> Result;
    ActiveComponentByRecording.GetKeys(Result);
    return Result;
}

bool ATMOPRecordedCallDirector::GetActiveSubtitle(const FName RecordingId,
    FName& OutSegmentId, FText& OutLeftSpeaker, FText& OutRightSpeaker,
    FText& OutTranscript, bool& bOutRadioStyle) const
{
    OutSegmentId = NAME_None;
    OutLeftSpeaker = FText::GetEmpty();
    OutRightSpeaker = FText::GetEmpty();
    OutTranscript = FText::GetEmpty();
    bOutRadioStyle = false;
    if (!ActiveComponentByRecording.Contains(RecordingId) || Clock == nullptr)
        return false;
    const FTMOPRecordedCallRow* Row = LoadedRows.FindByPredicate(
        [RecordingId](const FTMOPRecordedCallRow& Candidate)
        { return Candidate.RecordingId == RecordingId; });
    if (Row == nullptr) return false;
    const float AudioSecond = CalculateAudioOffset(*Row,
        Clock->GetCurrentTime().ToSecondsFromMidnight());
    const FTMOPRecordedCallSpeechSegment* ActiveSegment = nullptr;
    for (int32 Index = 0; Index < Row->SpeechSegments.Num(); ++Index)
    {
        const FTMOPRecordedCallSpeechSegment& Segment = Row->SpeechSegments[Index];
        const float End = Segment.AudioEndOffsetSeconds > Segment.AudioStartOffsetSeconds
            ? Segment.AudioEndOffsetSeconds
            : (Row->SpeechSegments.IsValidIndex(Index + 1)
                ? Row->SpeechSegments[Index + 1].AudioStartOffsetSeconds
                : TNumericLimits<float>::Max());
        if (AudioSecond >= Segment.AudioStartOffsetSeconds && AudioSecond < End)
        {
            ActiveSegment = &Segment;
            break;
        }
    }
    if (ActiveSegment == nullptr || ActiveSegment->Transcript.IsEmpty()) return false;

    auto ResolveName = [this](const FName EntityId) -> FText
    {
        if (EntityId.IsNone()) return NSLOCTEXT("TMOP", "UnknownRadioVoice", "Okänd röst");
        UTMOPPersonRegistrySubsystem* Registry = GetGameInstance() != nullptr
            ? GetGameInstance()->GetSubsystem<UTMOPPersonRegistrySubsystem>() : nullptr;
        FTMOPPersonProfileRow Profile;
        if (Registry != nullptr && Registry->GetPersonProfile(EntityId, Profile) &&
            !Profile.FullName.IsEmpty()) return Profile.FullName;
        return FText::FromString(EntityId.ToString().Replace(TEXT("_"), TEXT(" ")));
    };

    OutSegmentId = ActiveSegment->SegmentId;
    OutLeftSpeaker = ResolveName(ActiveSegment->SpeakerEntityId);
    if (!ActiveSegment->ListenerEntityIds.IsEmpty())
        OutRightSpeaker = ResolveName(ActiveSegment->ListenerEntityIds[0]);
    else
        for (const FName Participant : Row->ParticipantEntityIds)
            if (Participant != ActiveSegment->SpeakerEntityId)
            {
                OutRightSpeaker = ResolveName(Participant);
                break;
            }
    if (OutRightSpeaker.IsEmpty()) OutRightSpeaker = Row->DisplayName;
    OutTranscript = FText::FromString(ActiveSegment->Transcript);
    bOutRadioStyle = Row->CallType == ETMOPRecordedCallType::LAC ||
        Row->CallType == ETMOPRecordedCallType::PoliceRadio ||
        Row->CallType == ETMOPRecordedCallType::PoliceEmergencyCall ||
        Row->CallType == ETMOPRecordedCallType::TaxiRadio;
    return true;
}

bool ATMOPRecordedCallDirector::ValidateRecordings(
    TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TSet<FName> Seen;
    for (const FTMOPRecordedCallRow& Row : LoadedRows)
    {
        if (Row.RecordingId.IsNone())
        {
            OutErrors.Add(TEXT("A row has no RecordingId."));
            continue;
        }
        if (Seen.Contains(Row.RecordingId))
        {
            OutErrors.Add(FString::Printf(TEXT("Duplicate RecordingId '%s'."),
                *Row.RecordingId.ToString()));
        }
        Seen.Add(Row.RecordingId);
        if (Row.RecordingDateISO != TEXT("1986-02-28"))
        {
            OutErrors.Add(FString::Printf(
                TEXT("Recording '%s' is outside 1986-02-28 and will be ignored."),
                *Row.RecordingId.ToString()));
        }
        if (Row.bHistoricalEndTimeKnown &&
            Row.HistoricalEndTime.ToSecondsFromMidnight() <=
                Row.HistoricalStartTime.ToSecondsFromMidnight())
        {
            OutErrors.Add(FString::Printf(
                TEXT("Recording '%s' has an invalid end time."),
                *Row.RecordingId.ToString()));
        }
        if (Row.AudioClipEndOffsetSeconds > 0.0f &&
            Row.AudioClipEndOffsetSeconds <= Row.AudioClipStartOffsetSeconds)
        {
            OutErrors.Add(FString::Printf(
                TEXT("Recording '%s' has an invalid audio clip range."),
                *Row.RecordingId.ToString()));
        }
    }
    return OutErrors.IsEmpty();
}
