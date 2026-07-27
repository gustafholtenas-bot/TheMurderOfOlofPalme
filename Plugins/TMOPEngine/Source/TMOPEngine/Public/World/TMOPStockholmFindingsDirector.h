#pragma once

#include "CoreMinimal.h"
#include "World/TMOPTimedPropDirector.h"
#include "TMOPStockholmFindingsDirector.generated.h"

/**
 * Ready-filled inner-city finding schedule generated from Fynd.kmz.
 * Place this actor in the level; Scheduled Entries remains editable in Details.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPStockholmFindingsDirector
    : public ATMOPTimedPropDirector
{
    GENERATED_BODY()

public:
    ATMOPStockholmFindingsDirector();
};

