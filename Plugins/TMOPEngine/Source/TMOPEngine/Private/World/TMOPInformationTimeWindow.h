#pragma once

namespace TMOPInformationTimeWindow
{
    // Same-day clock seconds, start inclusive, end exclusive. A reversed interval
    // spans midnight. Do not silently normalize invalid times or equal endpoints.
    inline bool Contains(int Now, int Start, int End)
    {
        if (Now < 0 || Now >= 86400 || Start < 0 || Start >= 86400 ||
            End < 0 || End >= 86400 || Start == End) return false;
        return Start < End ? Now >= Start && Now < End : Now >= Start || Now < End;
    }
}
