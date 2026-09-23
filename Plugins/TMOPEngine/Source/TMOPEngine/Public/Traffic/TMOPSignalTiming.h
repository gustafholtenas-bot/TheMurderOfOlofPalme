#pragma once
#include <cmath>
#include <vector>

// Engine-independent timing core: identical result regardless of frame rate/seek path.
namespace TMOPSignalTiming
{
    struct Result { int Index = -1; double Remaining = 0; };
    inline Result Evaluate(const std::vector<double>& Durations, double Time,
        double Epoch, double Offset, int Initial)
    {
        if (Durations.empty() || !std::isfinite(Time) || !std::isfinite(Epoch) ||
            !std::isfinite(Offset) || Initial < 0 || Initial >= int(Durations.size())) return {};
        double Total = 0, Prefix = 0;
        for (int I = 0; I < int(Durations.size()); ++I)
        {
            const double D = Durations[I];
            if (!std::isfinite(D) || D <= 0) return {};
            Total += D;
            if (I < Initial) Prefix += D;
        }
        if (!std::isfinite(Total)) return {};
        double Position = std::fmod(Time - Epoch + Offset + Prefix, Total);
        if (Position < 0) Position += Total;
        for (int I = 0; I < int(Durations.size()); ++I)
        {
            if (Position < Durations[I]) return {I, Durations[I] - Position};
            Position -= Durations[I];
        }
        return {0, Durations[0]};
    }
}
