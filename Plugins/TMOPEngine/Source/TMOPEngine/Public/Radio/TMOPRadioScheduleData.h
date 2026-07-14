#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TMOPRadioScheduleData.generated.h"

class USoundBase;

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRadioProgramSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    FName SegmentId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    FText DisplayName;

    /** Seconds since midnight. Example 23:15:00 = 83700. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio",
        meta=(ClampMin="0", ClampMax="86399"))
    int32 StartSecondOfDay = 82800;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio",
        meta=(ClampMin="0", ClampMax="86400"))
    int32 EndSecondOfDay = 86400;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    TObjectPtr<USoundBase> Audio;

    /** Offset into Audio that corresponds to StartSecondOfDay. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio",
        meta=(ClampMin="0.0"))
    float AudioStartOffsetSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    bool bLoopAudioWithinSegment = false;

    bool ContainsSecond(int32 SecondOfDay) const
    {
        return SecondOfDay >= StartSecondOfDay && SecondOfDay < EndSecondOfDay;
    }
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPRadioChannel
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    FName ChannelId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    FText FrequencyLabel;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    bool bPoliceFrequency = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    TArray<FTMOPRadioProgramSegment> Segments;
};

/** All radio channels and time-synchronised broadcasts for one scenario. */
UCLASS(BlueprintType)
class TMOPENGINE_API UTMOPRadioScheduleData : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Radio")
    TArray<FTMOPRadioChannel> Channels;

    UFUNCTION(BlueprintPure, Category="TMOP|Radio")
    int32 FindChannelIndex(FName ChannelId) const;

    const FTMOPRadioProgramSegment* FindSegment(int32 ChannelIndex,
        int32 SecondOfDay) const;
};
