#include "../../Plugins/TMOPEngine/Source/TMOPEngine/Private/Player/TMOPLocalSessionPolicy.h"
#include <cassert>
#include <iostream>
#include <vector>

int main()
{
    using namespace TMOPLocalSessionPolicy;
    for (int Count = -10; Count <= 10; ++Count)
        assert(IsValidPlayerCount(Count) == (Count >= 1 && Count <= 4));
    for (int Flags = 0; Flags < 16; ++Flags)
        assert(CanRunClock(Flags & 1, Flags & 2, Flags & 4, Flags & 8) == (Flags == 1));
    std::vector<SpawnOffset> Candidates;
    for (int Attempt = 0; Attempt < 65; ++Attempt)
    {
        const auto Position = CandidateOffset(Attempt);
        assert(std::isfinite(Position.X) && std::isfinite(Position.Y));
        assert(std::hypot(Position.X, Position.Y) <= 600.01f);
        for (const auto& Earlier : Candidates)
            assert(std::hypot(Position.X - Earlier.X, Position.Y - Earlier.Y) > 1.0f);
        Candidates.push_back(Position);
    }
    assert(Candidates.front().X == 0 && Candidates.front().Y == 0);
    // Four normal capsules can choose distinct positions without exceeding 6 m.
    std::vector<SpawnOffset> Chosen;
    for (const auto& Candidate : Candidates)
    {
        bool Clear = true;
        for (const auto& Earlier : Chosen)
            Clear &= std::hypot(Candidate.X - Earlier.X, Candidate.Y - Earlier.Y) >= 114.0f;
        if (Clear) Chosen.push_back(Candidate);
        if (Chosen.size() == 4) break;
    }
    assert(Chosen.size() == 4);
    std::cout << "PASS: player bounds, 16 clock gates, 65 spawn candidates, four-player spacing\n";
}
