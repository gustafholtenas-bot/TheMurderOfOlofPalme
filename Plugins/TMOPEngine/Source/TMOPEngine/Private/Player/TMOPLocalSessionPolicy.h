#pragma once
#include <cmath>

namespace TMOPLocalSessionPolicy
{
inline bool IsValidPlayerCount(int Count) { return Count >= 1 && Count <= 4; }
inline bool CanRunClock(bool RunRequested, bool AwaitingDecision, bool HasPause, bool WorldPaused)
{ return RunRequested && !AwaitingDecision && !HasPause && !WorldPaused; }
struct SpawnOffset { float X; float Y; };
inline SpawnOffset CandidateOffset(int Attempt)
{
    if (Attempt <= 0) return {0, 0};
    const int Ring = 1 + (Attempt - 1) / 16;
    const float Angle = ((Attempt - 1) % 16) * 6.28318530718f / 16.0f;
    return {Ring * 150.0f * std::cos(Angle), Ring * 150.0f * std::sin(Angle)};
}
}
