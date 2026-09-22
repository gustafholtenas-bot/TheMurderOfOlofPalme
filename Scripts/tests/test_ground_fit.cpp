#include "Vehicles/TMOPGroundFitMath.h"
#include <cassert>
#include <algorithm>
#include <limits>
#include <random>
#include <vector>
#include <iostream>

using namespace TMOPGroundFit;
static bool Near(double A, double B, double E = 1e-8) { return std::abs(A-B) < E; }
int main()
{
    Plane P;
    Point Flat[] = {{140,-75,0},{140,75,0},{-140,-75,0},{-140,75,0}};
    assert(Fit(Flat, 4, P)); assert(Near(P.X,0) && Near(P.Y,0) && Near(P.Height,0));
    // Both surfaces have vertical normals, but wheel HEIGHTS must produce a body roll.
    Flat[1].Z = Flat[3].Z = 20;
    assert(Fit(Flat,4,P)); assert(Near(P.X,0) && Near(P.Y,20.0/150) && Near(P.Height,10));
    // A single wheel on a kerb needs independent suspension residuals, not four coplanar wheels.
    Flat[3].Z = 0;
    assert(Fit(Flat,4,P));
    for (const auto& W : Flat) assert(Near(std::abs(W.Z-(P.X*W.X+P.Y*W.Y+P.Height)),5));
    Point Line[] = {{0,0,0},{10,0,0},{20,0,0}};
    assert(!Fit(Line,3,P)); assert(!Fit(Flat,2,P));
    Line[0].Z = std::numeric_limits<double>::quiet_NaN(); assert(!Fit(Line,3,P));
    std::mt19937 Rng(19860228);
    std::uniform_real_distribution<double> Slope(-.3,.3), Origin(-1000000,1000000);
    for (int I = 0; I < 50000; ++I)
    {
        const double A=Slope(Rng), B=Slope(Rng), C=Origin(Rng), OX=Origin(Rng), OY=Origin(Rng);
        // Three-axle bus; translated world coordinates and permuted contact order.
        std::vector<Point> Wheels;
        for (double X : {-400.0,-250.0,350.0}) for (double Y : {-110.0,110.0})
            Wheels.push_back({X+OX,Y+OY,A*(X+OX)+B*(Y+OY)+C});
        assert(Fit(Wheels.data(),Wheels.size(),P));
        assert(Near(P.X,A) && Near(P.Y,B) && Near(P.Height,C,.001));
        const Plane First = P;
        std::shuffle(Wheels.begin(),Wheels.end(),Rng);
        assert(Fit(Wheels.data(),Wheels.size(),P));
        assert(Near(P.X,First.X) && Near(P.Y,First.Y) && Near(P.Height,First.Height,.001));
    }
    std::cout << "PASS: flat, kerb, one-wheel suspension, degenerate/NaN rejection, 50000 three-axle slope/order/translation cases\n";
}
