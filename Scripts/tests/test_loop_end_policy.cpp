#include <cassert>
#include "Time/TMOPLoopEndPolicy.h"

int main()
{
    constexpr int End = 23 * 60 * 60 + 45 * 60;
    static_assert(!TMOPLoopEndPolicy::HasReachedEnd(End - 1, End));
    static_assert(TMOPLoopEndPolicy::HasReachedEnd(End, End));
    static_assert(TMOPLoopEndPolicy::HasReachedEnd(End + 8, End));
    assert(TMOPLoopEndPolicy::HasReachedEnd(End, End));
    return 0;
}
