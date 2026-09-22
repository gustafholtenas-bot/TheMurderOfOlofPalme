#include "UI/TMOPAppearanceLobbyState.h"
#include <cassert>
#include <iostream>

int main()
{
    FTMOPAppearanceLobbyState State;
    assert(!State.AllReady());
    assert(!State.Begin(0));
    assert(!State.Begin(5));
    for (int Count=1; Count<=4; ++Count)
    {
        assert(State.Begin(Count));
        assert(!State.Confirm(-1, 0));
        assert(!State.Confirm(Count, 0));
        for (int Slot=0; Slot<Count; ++Slot)
        {
            assert(!State.AllReady());
            assert(State.Confirm(Slot, State.GetRevision(Slot)));
            assert(State.IsReady(Slot));
        }
        assert(State.AllReady());
        assert(State.Confirm(0, State.GetRevision(0))); // duplicate vote is harmless
        const unsigned OldRevision = State.GetRevision(0);
        State.Edit(0);
        assert(!State.AllReady());
        assert(!State.Confirm(0, OldRevision)); // cannot confirm a stale UI snapshot
        assert(State.Confirm(0, State.GetRevision(0)));
        assert(State.AllReady());
        for (int Slot=1; Slot<Count; ++Slot) assert(State.IsReady(Slot));
        const unsigned OldGeneration = State.GetGeneration();
        State.Cancel();
        assert(State.GetGeneration() != OldGeneration);
        assert(!State.AllReady());
        assert(!State.Confirm(0, 0));
        State.Begin(Count);
        assert(!State.AllReady()); // previous lobby's ready votes never carry over
    }
    State.Begin(4);
    for (int I=0; I<4; ++I) State.Confirm(I, 0);
    State.Begin(2);
    assert(!State.AllReady());
    assert(!State.IsReady(2));
    assert(!State.Edit(3));
    std::cout << "PASS: 1-4 players, all-ready barrier, stale edits, cancellation, count changes, invalid slots\n";
}
