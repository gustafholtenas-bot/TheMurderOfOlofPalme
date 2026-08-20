#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Time/TMOPTime.h"
#include "Agents/TMOPAgentTypes.h"
#include "TMOPRecordedCallTypes.generated.h"

class USoundWave;

/** What kind of recorded communication this row represents. */
UENUM(BlueprintType)
enum class ETMOPRecordedCallType : uint8
{
    Unknown,
    LAC,
    PoliceEmergencyCall,
    PoliceRadio,
    TaxiRadio,
    Telephone,
    Interview,
    Other
};

/** Whether the audio itself is currently available to the project. */
UENUM(BlueprintType)
enum class ETMOPRecordingAvailability : uint8
{
    Unknown,
    ReferencedNotAcquired UMETA(DisplayName="Referenced, not acquired"),
    AudioAvailable UMETA(DisplayName="Audio available"),
    TranscriptOnly UMETA(DisplayName="Transcript only"),
    MissingOrDestroyed UMETA(DisplayName="Missing or destroyed")
};

/** One time-coded speaker turn within a recording. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRecordedCallSpeechSegment
{
    GENERATED_BODY()

    /** Stable identifier within this recording. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FName SegmentId = NAME_None;

    /** Seconds from the beginning of the imported audio asset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call",
        meta=(ClampMin="0.0", Units="s"))
    float AudioStartOffsetSeconds = 0.0f;

    /** End position in the imported audio asset. Zero means not yet measured. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call",
        meta=(ClampMin="0.0", Units="s"))
    float AudioEndOffsetSeconds = 0.0f;

    /** EntityId from DT_TMOP_People. NAME_None is allowed for an unidentified voice. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FName SpeakerEntityId = NAME_None;

    /** People directly addressed by this speaker turn. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    TArray<FName> ListenerEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call",
        meta=(MultiLine="true"))
    FString Transcript;

    /** False when Transcript is a summary or cautious normalisation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    bool bVerbatimTranscript = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call",
        meta=(MultiLine="true"))
    FString Notes;
};

/** A recording, tape or selected clip from a historical call or radio exchange. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRecordedCallRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FName RecordingId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    ETMOPRecordedCallType CallType = ETMOPRecordedCallType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    ETMOPRecordingAvailability Availability = ETMOPRecordingAvailability::Unknown;

    /** MP3/WAV imported into Unreal as a SoundWave asset. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    TSoftObjectPtr<USoundWave> AudioFile;

    /** ISO date, for example 1986-02-28 or 2020-05-25. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FString RecordingDateISO;

    /** True when HistoricalStartTime is an exact documented time. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call|Historical Time")
    bool bHistoricalStartTimeKnown = false;

    /** Historical time corresponding to AudioClipStartOffsetSeconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call|Historical Time",
        meta=(EditCondition="bHistoricalStartTimeKnown", EditConditionHides))
    FTMOPTime HistoricalStartTime;

    /** Optional uncertainty window when the exact historical start is unknown. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call|Historical Time")
    FTMOPTime EarliestHistoricalStartTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call|Historical Time")
    FTMOPTime LatestHistoricalStartTime;

    /** Seek position in the imported audio where this database clip begins. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call|Audio",
        meta=(ClampMin="0.0", Units="s"))
    float AudioClipStartOffsetSeconds = 0.0f;

    /** Zero plays to the end of the file. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call|Audio",
        meta=(ClampMin="0.0", Units="s"))
    float AudioClipEndOffsetSeconds = 0.0f;

    /** Every identified or unidentified DT_TMOP_People role heard in the clip. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    TArray<FName> ParticipantEntityIds;

    /** Optional link to a central DT_TMOP_HistoricalEvents row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FName HistoricalEventId = NAME_None;

    /** Uppslag IDs that document the recording, call or transcript. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    TArray<FName> UppslagIds;

    /** Original tape/reel/file designation such as a LAC tape number. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FString ArchiveRecordingId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FString ChannelOrLine;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    TArray<FTMOPRecordedCallSpeechSegment> SpeechSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Recorded Call",
        meta=(MultiLine="true"))
    FString Notes;
};
