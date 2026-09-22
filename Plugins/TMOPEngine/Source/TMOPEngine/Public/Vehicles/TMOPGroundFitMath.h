#pragma once
#include <cmath>
#include <cstddef>

namespace TMOPGroundFit
{
struct Point { double X, Y, Z; };
struct Plane { double X = 0, Y = 0, Height = 0; };

// Centred least squares avoids loss of precision far from the world origin.
// Returns false for missing contacts or a collinear/degenerate wheel layout.
inline bool Fit(const Point* Points, std::size_t Count, Plane& Out)
{
    if (Count < 3) return false;
    double X = 0, Y = 0, Z = 0;
    for (std::size_t I = 0; I < Count; ++I)
    {
        if (!std::isfinite(Points[I].X) || !std::isfinite(Points[I].Y) || !std::isfinite(Points[I].Z)) return false;
        X += Points[I].X; Y += Points[I].Y; Z += Points[I].Z;
    }
    X /= Count; Y /= Count; Z /= Count;
    double XX = 0, XY = 0, YY = 0, XZ = 0, YZ = 0;
    for (std::size_t I = 0; I < Count; ++I)
    {
        const double DX = Points[I].X-X, DY = Points[I].Y-Y, DZ = Points[I].Z-Z;
        XX += DX*DX; XY += DX*DY; YY += DY*DY; XZ += DX*DZ; YZ += DY*DZ;
    }
    const double D = XX*YY-XY*XY;
    if (XX <= 1e-8 || YY <= 1e-8 || D <= 1e-8*XX*YY) return false;
    Out.X = (XZ*YY-YZ*XY)/D;
    Out.Y = (YZ*XX-XZ*XY)/D;
    Out.Height = Z-Out.X*X-Out.Y*Y;
    return std::isfinite(Out.X) && std::isfinite(Out.Y) && std::isfinite(Out.Height);
}
}
