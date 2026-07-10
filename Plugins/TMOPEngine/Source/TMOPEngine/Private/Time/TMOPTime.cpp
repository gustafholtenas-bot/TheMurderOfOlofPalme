#include "Time/TMOPTime.h"

namespace
{
    constexpr int32 SecondsPerMinute = 60;
    constexpr int32 SecondsPerHour = 60 * SecondsPerMinute;
    constexpr int32 SecondsPerDay = 24 * SecondsPerHour;

    int32 NormalizeSeconds(const int32 InSeconds)
    {
        return ((InSeconds % SecondsPerDay) + SecondsPerDay) % SecondsPerDay;
    }
}

FTMOPTime::FTMOPTime(const int32 InHour, const int32 InMinute, const int32 InSecond)
{
    const FTMOPTime Normalized = FromSecondsFromMidnight(
        InHour * SecondsPerHour + InMinute * SecondsPerMinute + InSecond);

    Hour = Normalized.Hour;
    Minute = Normalized.Minute;
    Second = Normalized.Second;
}

int32 FTMOPTime::ToSecondsFromMidnight() const
{
    return NormalizeSeconds(Hour * SecondsPerHour + Minute * SecondsPerMinute + Second);
}

FTMOPTime FTMOPTime::FromSecondsFromMidnight(const int32 InSeconds)
{
    const int32 Normalized = NormalizeSeconds(InSeconds);

    FTMOPTime Result;
    Result.Hour = Normalized / SecondsPerHour;
    Result.Minute = (Normalized % SecondsPerHour) / SecondsPerMinute;
    Result.Second = Normalized % SecondsPerMinute;
    return Result;
}

FString FTMOPTime::ToDisplayString() const
{
    return FString::Printf(TEXT("%02d:%02d:%02d"), Hour, Minute, Second);
}

bool FTMOPTime::operator==(const FTMOPTime& Other) const
{
    return ToSecondsFromMidnight() == Other.ToSecondsFromMidnight();
}

bool FTMOPTime::operator!=(const FTMOPTime& Other) const
{
    return !(*this == Other);
}
