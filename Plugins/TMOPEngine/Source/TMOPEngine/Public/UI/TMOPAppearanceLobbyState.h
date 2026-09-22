#pragma once

/** Engine-independent ready barrier. A new edit invalidates that player's vote. */
class FTMOPAppearanceLobbyState
{
public:
    bool Begin(int InCount)
    {
        Cancel();
        if (InCount < 1 || InCount > 4) return false;
        Count = InCount;
        return true;
    }
    void Cancel()
    {
        Count = 0;
        ++Generation;
        for (int I=0; I<4; ++I) { Ready[I] = false; Revisions[I] = 0; }
    }
    bool Edit(int Slot)
    {
        if (!Contains(Slot)) return false;
        Ready[Slot] = false;
        ++Revisions[Slot];
        return true;
    }
    bool Confirm(int Slot, unsigned Revision)
    {
        if (!Contains(Slot) || Revisions[Slot] != Revision) return false;
        Ready[Slot] = true;
        return true;
    }
    bool Contains(int Slot) const { return Slot >= 0 && Slot < Count; }
    bool IsReady(int Slot) const { return Contains(Slot) && Ready[Slot]; }
    bool AllReady() const
    {
        if (Count < 1 || Count > 4) return false;
        for (int I=0; I<Count; ++I) if (!Ready[I]) return false;
        return true;
    }
    int GetCount() const { return Count; }
    unsigned GetGeneration() const { return Generation; }
    unsigned GetRevision(int Slot) const { return Contains(Slot) ? Revisions[Slot] : 0; }
private:
    int Count = 0;
    unsigned Generation = 0;
    bool Ready[4] = {};
    unsigned Revisions[4] = {};
};
