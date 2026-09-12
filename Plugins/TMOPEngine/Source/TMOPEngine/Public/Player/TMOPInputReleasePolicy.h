#pragma once

#include <algorithm>
#include <array>
#include <vector>

namespace TMOPInputReleasePolicy
{
/** A held key blocks only itself, for its owning player, until released. */
template <typename Key>
class TState
{
public:
    void Hold(int Player, const Key& Value)
    {
        if (Player < 0 || Player >= 4) return;
        auto& Keys = Players[Player];
        if (std::find(Keys.begin(), Keys.end(), Value) == Keys.end()) Keys.push_back(Value);
    }

    bool IsSuppressed(int Player, const Key& Value, bool StillDown)
    {
        if (Player < 0 || Player >= 4) return false;
        auto& Keys = Players[Player];
        const auto Found = std::find(Keys.begin(), Keys.end(), Value);
        if (Found == Keys.end()) return false;
        if (!StillDown) Keys.erase(Found);
        return true;
    }

    void ReleaseKey(const Key& Value)
    {
        for (auto& Keys : Players)
            Keys.erase(std::remove(Keys.begin(), Keys.end(), Value), Keys.end());
    }

    void Reset() { for (auto& Keys : Players) Keys.clear(); }

private:
    std::array<std::vector<Key>, 4> Players;
};
}
