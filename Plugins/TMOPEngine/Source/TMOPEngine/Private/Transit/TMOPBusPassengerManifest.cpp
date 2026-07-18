#include "Transit/TMOPBusPassengerManifest.h"

bool UTMOPBusPassengerManifest::ValidateManifest(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    TSet<FName> PassengerIds;
    for (int32 Index = 0; Index < Journeys.Num(); ++Index)
    {
        const FTMOPBusPassengerJourney& Journey = Journeys[Index];
        const FString Prefix = FString::Printf(TEXT("Journey %d"), Index);
        if (Journey.PassengerEntityId.IsNone()) OutErrors.Add(Prefix + TEXT(": PassengerEntityId is missing."));
        if (Journey.BoardingStopId.IsNone()) OutErrors.Add(Prefix + TEXT(": BoardingStopId is missing."));
        if (Journey.AlightingStopId.IsNone()) OutErrors.Add(Prefix + TEXT(": AlightingStopId is missing."));
        if (Journey.BoardingStopId == Journey.AlightingStopId && !Journey.BoardingStopId.IsNone())
            OutErrors.Add(Prefix + TEXT(": boarding and alighting stops are identical."));
        if (Journey.Placement == ETMOPBusPassengerPlacement::AssignedSeat && Journey.AssignedSeatId.IsNone())
            OutErrors.Add(Prefix + TEXT(": AssignedSeat placement requires AssignedSeatId."));
        if (PassengerIds.Contains(Journey.PassengerEntityId) && !Journey.PassengerEntityId.IsNone())
            OutErrors.Add(Prefix + TEXT(": duplicate PassengerEntityId in this manifest."));
        PassengerIds.Add(Journey.PassengerEntityId);
    }
    return OutErrors.IsEmpty();
}
