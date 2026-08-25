#pragma once

#include "CoreMinimal.h"
#include "TMOPEntityLabelTypes.generated.h"

/** Source/evidence symbol displayed above a historical person or vehicle. */
UENUM(BlueprintType)
enum class ETMOPEntityEvidenceIcon : uint8
{
    /** Infer from observed category and source text. */
    Automatic,
    /** The entity has its own police interview. */
    PoliceInterview,
    /** Documented by another witness, media, social media, or another source. */
    OtherDocumentation,
    /** A person or vehicle observed in the reconstruction. */
    Observed
};

