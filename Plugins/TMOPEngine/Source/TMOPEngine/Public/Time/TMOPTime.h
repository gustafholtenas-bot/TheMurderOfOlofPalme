#pragma once

#include "CoreMinimal.h"
#include "TMOPTime.generated.h"

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTime
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Time")
    int32 Hour = 23;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Time")
    int32 Minute = 13;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TMOP|Time")
    int32 Second = 0;

    FTMOPTime() = default;
    FTMOPTime(int32 InHour, int32 InMinute, int32 InSecond);

    int32 ToSecondsFromMidnight() const;
    static FTMOPTime FromSecondsFromMidnight(int32 InSeconds);
    FString ToDisplayString() const;

    bool operator==(const FTMOPTime& Other) const;
    bool operator!=(const FTMOPTime& Other) const;
};
