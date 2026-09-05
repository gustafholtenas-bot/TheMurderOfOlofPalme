#pragma once

// Engine-independent primitives used by the real route builder and the
// standalone regression test. Vector must support vector arithmetic.
namespace TMOPVehicleRouteMath
{
    template<class Vector> struct TCubicSample
    {
        Vector Position;
        Vector Tangent;
    };
    template<class Vector>
    TCubicSample<Vector> Cubic(const Vector& P0, const Vector& P1,
        const Vector& P2, const Vector& P3, const Vector& Bias, double T)
    {
        const double U = 1.0 - T;
        return {
            P0*U*U*U + P1*3.0*U*U*T + P2*3.0*U*T*T + P3*T*T*T + Bias*(16.0*T*T*U*U),
            (P1-P0)*3.0*U*U + (P2-P1)*6.0*U*T + (P3-P2)*3.0*T*T + Bias*(32.0*T*U*(1.0-2.0*T))
        };
    }
    inline double SmoothProgress(double Alpha)
    {
        if (Alpha <= 0.0) return 0.0;
        if (Alpha >= 1.0) return 1.0;
        return Alpha*Alpha*(3.0-2.0*Alpha);
    }
    inline bool IsActive(double Now, double Departure, double Arrival)
    {
        return Arrival > Departure && Now >= Departure && Now < Arrival;
    }
}
