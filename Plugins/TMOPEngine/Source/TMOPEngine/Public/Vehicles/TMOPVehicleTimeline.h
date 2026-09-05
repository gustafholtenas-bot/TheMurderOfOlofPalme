#pragma once
#include "Vehicles/TMOPVehicleRoutePlan.h"

/** Event lookup differs between an editor table and live shared events; all
 * arithmetic and interval boundaries are otherwise identical. */
namespace TMOPVehicleTimeline
{
    using FEventResolver = TFunctionRef<bool(FName, int32&)>;
    inline bool ResolveEntry(const FTMOPHistoricalVehicleRow& Row, int32 Index,
        FEventResolver EventTime, int32& Result)
    {
        if (!Row.Timeline.IsValidIndex(Index)) return false;
        const auto& Entry = Row.Timeline[Index];
        if (Entry.TimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
        {
            if (!ResolveEntry(Row, Index - 1, EventTime, Result)) return false;
            if (TMOPVehicleRoute::IsDriving(Entry.Action) && !Entry.bTimeIsArrival &&
                Row.Timeline[Index-1].bUseStopDuration)
                Result += TMOPVehicleRoute::CompletionDelay(Row.Timeline[Index-1]);
            Result += Entry.EventOffsetSeconds;
        }
        else if (Entry.TimingMode == ETMOPEventTimingMode::Relative)
        {
            if (!EventTime(Entry.SharedEventId, Result)) return false;
            Result += Entry.EventOffsetSeconds;
        }
        else Result = Entry.Time.ToSecondsFromMidnight();
        return Result >= 0 && Result < 86400;
    }
    inline bool ResolveDeparture(const FTMOPHistoricalVehicleRow& Row, int32 Index,
        FEventResolver EventTime, int32& Result)
    {
        if (!Row.Timeline.IsValidIndex(Index) || !TMOPVehicleRoute::IsDriving(Row.Timeline[Index].Action)) return false;
        const auto& Entry = Row.Timeline[Index];
        if (!Entry.bTimeIsArrival) return ResolveEntry(Row, Index, EventTime, Result);
        if (!Entry.bUseExplicitDepartureTime ||
            Entry.DepartureTimingMode == ETMOPEventTimingMode::RelativeToPreviousEntry)
        {
            if (!ResolveEntry(Row, Index-1, EventTime, Result)) return false;
            Result += TMOPVehicleRoute::CompletionDelay(Row.Timeline[Index-1]);
            if (Entry.bUseExplicitDepartureTime) Result += Entry.DepartureOffsetSeconds;
        }
        else if (Entry.DepartureTimingMode == ETMOPEventTimingMode::Relative)
        {
            if (!EventTime(Entry.DepartureSharedEventId, Result)) return false;
            Result += Entry.DepartureOffsetSeconds;
        }
        else Result = Entry.DepartureTime.ToSecondsFromMidnight();
        return Result >= 0 && Result < 86400;
    }
    inline bool ResolveWindow(const FTMOPHistoricalVehicleRow& Row, int32 Index,
        FEventResolver EventTime, int32& Departure, int32& Arrival)
    {
        if (!ResolveDeparture(Row, Index, EventTime, Departure)) return false;
        if (Row.Timeline[Index].bTimeIsArrival)
            return ResolveEntry(Row, Index, EventTime, Arrival) && Arrival > Departure;
        for (int32 Next = Index + 1; Next < Row.Timeline.Num(); ++Next)
        {
            if (TMOPVehicleRoute::IsDriving(Row.Timeline[Next].Action)) break;
            if (TMOPVehicleRoute::IsStop(Row.Timeline[Next].Action))
                return ResolveEntry(Row, Next, EventTime, Arrival) && Arrival > Departure;
        }
        return false;
    }
}
