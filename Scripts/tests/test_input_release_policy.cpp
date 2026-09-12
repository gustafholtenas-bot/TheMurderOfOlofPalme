#include "Player/TMOPInputReleasePolicy.h"
#include <cassert>

int main()
{
    TMOPInputReleasePolicy::TState<int> State;
    // P2 holding movement must never block P1's newly pressed pause or movement.
    State.Hold(1, 10);
    assert(State.IsSuppressed(1, 10, true));
    assert(!State.IsSuppressed(0, 10, true));
    assert(!State.IsSuppressed(0, 20, true));
    assert(!State.IsSuppressed(1, 20, true));
    // Controller button identifiers are equal across devices, but owners differ.
    State.Hold(2, 30);
    assert(!State.IsSuppressed(3, 30, true));
    assert(State.IsSuppressed(2, 30, false));
    assert(!State.IsSuppressed(2, 30, true));
    // Physical release clears only that key, without waiting for every other key.
    State.Hold(1, 11);
    State.ReleaseKey(10);
    assert(!State.IsSuppressed(1, 10, true));
    assert(State.IsSuppressed(1, 11, true));
    State.Hold(1, 11);
    State.ReleaseKey(11);
    assert(!State.IsSuppressed(1, 11, true));
    State.Hold(0, 40);
    State.Reset();
    assert(!State.IsSuppressed(0, 40, true));
}
