#include "Time/TMOPTimeTravelPolicy.h"
#include <cassert>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

struct Key { double Time; };

int main()
{
    using namespace TMOPTimeTravel;
    constexpr int Start = 23 * 3600, End = Start + 45 * 60;
    int Destinations = 0;
    for (int T = Start; T <= End; T += 5)
    {
        assert(Snap(T, Start, End) == T);
        assert(Snap(Snap(T + 1.9, Start, End), Start, End) == T);
        ++Destinations;
    }
    assert(Destinations == 541);
    assert(Snap(Start + 2.4999, Start, End) == Start);
    assert(Snap(Start + 2.5, Start, End) == Start + 5);
    assert(Snap(-100, Start, End) == Start);
    assert(Snap(999999, Start, End) == End);
    assert(Snap(std::numeric_limits<double>::quiet_NaN(), Start, End) == Start);
    assert(Snap(100, 3, 17) == 13); // Grid is anchored at the scenario start.

    std::vector<Key> Keys{{10}, {12}, {12.05}, {20}, {30}};
    assert(FloorKey(Keys, 9.999) == -1); // No future spawn before its start.
    assert(FloorKey(Keys, 12) == 1); // Changes apply at the boundary itself.
    assert(FloorKey(Keys, 12.049) == 1);
    assert(FloorKey(Keys, 12.05) == 2);
    assert(FloorKey(Keys, 100) == 4);
    assert(FloorKey(std::vector<Key>{}, 12) == -1);
    std::mt19937 Random(19860228);
    std::uniform_real_distribution<double> Times(0, 50);
    for (int I = 0; I < 100000; ++I)
    {
        const double T = Times(Random);
        int Expected = -1;
        for (int J = 0; J < int(Keys.size()); ++J) if (Keys[J].Time <= T) Expected = J;
        assert(FloorKey(Keys, T) == Expected);
        // Unrelated future/past lookups cannot change a repeated result.
        FloorKey(Keys, Times(Random));
        assert(FloorKey(Keys, T) == Expected);
    }

    assert(BlendAlpha(10, 20, 15, true, false) == 0.5);
    assert(BlendAlpha(10, 20, 15, false, false) == 0); // Despawn is not a fade along a path.
    assert(BlendAlpha(10, 20, 15, true, true) == 0); // Never interpolate through teleports/seat transitions.
    assert(BlendAlpha(10, 10, 15, true, false) == 0);
    assert(Crossed(10, 12, 11, false));
    assert(!Crossed(10, 12, 11, true)); // Seeking must not replay skipped gunshots.
    assert(!Crossed(12, 10, 11, false));
    assert(!Crossed(11, 12, 11, false)); // A boundary is emitted at most once.
    assert(Crossed(10, 11, 11, false));
    std::cout << "PASS: 541 destinations, 100000 randomized lookup/round-trip cases, boundaries, cuts and event suppression\n";
}
