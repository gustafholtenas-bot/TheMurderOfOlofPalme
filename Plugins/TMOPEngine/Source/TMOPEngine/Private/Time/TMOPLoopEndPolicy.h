#pragma once

namespace TMOPLoopEndPolicy
{
/** The configured end second is part of the loop and must be shown before stopping. */
constexpr bool HasReachedEnd(const int CurrentSecond, const int EndSecond)
{
    return CurrentSecond >= EndSecond;
}
}
