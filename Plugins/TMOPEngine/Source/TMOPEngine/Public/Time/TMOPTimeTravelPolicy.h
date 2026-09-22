#pragma once

// Engine-independent rules shared by the HUD, clock and replay tests.
#include <algorithm>
#include <cmath>
#include <cstddef>

namespace TMOPTimeTravel
{
constexpr int SeekStepSeconds = 5;
constexpr double RecordingStepSeconds = 0.05;
constexpr int FormatVersion = 3;

inline int Snap(double Requested, int Start, int End)
{
    if (!std::isfinite(Requested)) return Start;
    const int Last = Start + std::max(0, (End - Start) / SeekStepSeconds) * SeekStepSeconds;
    const double Clamped = std::max(double(Start), std::min(double(Last), Requested));
    return std::min(Last, Start + int(std::floor((Clamped - Start) / SeekStepSeconds + 0.5)) * SeekStepSeconds);
}

// Right-continuous: a change at T is already in effect when seeking to T.
// Returns -1 before the first key; never leaks a future spawn/state backwards.
template<class TimeAt> int FloorKeyIndex(int Count, TimeAt GetTime, double Time)
{
    int Low = 0, High = Count;
    while (Low < High)
    {
        const int Mid = Low + (High - Low) / 2;
        if (GetTime(Mid) <= Time) Low = Mid + 1;
        else High = Mid;
    }
    return Low - 1;
}
template<class Keys> int FloorKey(const Keys& Values, double Time)
{
    return FloorKeyIndex(int(Values.size()), [&](int I) { return Values[I].Time; }, Time);
}
inline double BlendAlpha(double A, double B, double Time, bool NextPresent, bool NextCut)
{
    if (!NextPresent || NextCut || B <= A) return 0;
    return std::max(0.0, std::min(1.0, (Time - A) / (B - A)));
}
inline bool Crossed(double Previous, double Current, double EventTime, bool Seeking)
{
    return !Seeking && Current >= Previous && Previous < EventTime && EventTime <= Current;
}
}
