#pragma once

#include "CoreMinimal.h"

namespace TMOPAutomaticSpeech
{
/**
 * Consume elapsed lines independently of array order. Shared-event changes may
 * move a later array entry before an earlier one; an unresolved event must not
 * block unrelated lines. Preserve the existing latest-due-line policy on seeks.
 * Reset ConsumedIndices when the simulation loop is initialized again.
 */
inline int32 ConsumeDueLines(const TArray<int32>& ResolvedSeconds,
    TSet<int32>& ConsumedIndices, const int32 CurrentSecond,
    const int32 PreviousSecond)
{
    int32 LatestIndex = INDEX_NONE;
    int32 LatestSecond = INDEX_NONE;
    for (int32 Index = 0; Index < ResolvedSeconds.Num(); ++Index)
    {
        const int32 LineSecond = ResolvedSeconds[Index];
        if (ConsumedIndices.Contains(Index) || LineSecond == INDEX_NONE ||
            LineSecond > CurrentSecond)
            continue;
        ConsumedIndices.Add(Index);
        if (LineSecond > PreviousSecond &&
            (LatestIndex == INDEX_NONE || LineSecond >= LatestSecond))
        {
            LatestIndex = Index;
            LatestSecond = LineSecond;
        }
    }
    return LatestIndex;
}
}
