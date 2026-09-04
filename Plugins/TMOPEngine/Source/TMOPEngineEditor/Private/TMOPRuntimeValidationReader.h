#pragma once

#include "CoreMinimal.h"

namespace TMOPRuntimeValidation
{
    /** Modification-based revision of the newest Saved/TMOP/Validation JSON. */
    uint64 GetLatestReportRevision();

    /** Builds a compact arrival result from the newest completed PIE report. */
    bool BuildArrivalBadge(
        FName EntityId,
        FName EntryId,
        FText& OutText,
        FText& OutToolTip,
        FLinearColor& OutColor);
}
