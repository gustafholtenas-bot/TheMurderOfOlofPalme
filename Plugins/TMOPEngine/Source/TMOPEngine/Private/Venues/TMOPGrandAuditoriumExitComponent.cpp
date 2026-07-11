#include "Venues/TMOPGrandAuditoriumExitComponent.h"

UTMOPGrandAuditoriumExitComponent::UTMOPGrandAuditoriumExitComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPGrandAuditoriumExitComponent::ValidateExit(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (ExitId.IsNone()) OutErrors.Add(TEXT("ExitId is missing."));
    if (VenueId.IsNone()) OutErrors.Add(TEXT("VenueId is missing."));
    if (AuditoriumId.IsNone()) OutErrors.Add(TEXT("AuditoriumId is missing."));
    return OutErrors.IsEmpty();
}
