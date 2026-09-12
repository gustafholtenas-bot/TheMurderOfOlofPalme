#pragma once

#include <algorithm>
#include <cmath>
#include <queue>
#include <string>
#include <vector>

// Engine-independent schedule: the renderer samples it using the simulation clock.
namespace TMOPKillerBranches
{
struct Edge { int From = -1; int To = -1; double Duration = 0; };
struct Seed { int Node = -1; int OriginalNext = -1; int OriginalPrevious = -1; double Time = 0; };
struct Leg
{
    std::string Key;
    int EdgeIndex = -1;
    int Depth = 0;
    double Start = 0, End = 0, RootTime = 0;
    bool Terminal = true;
};
struct Schedule { std::vector<Leg> Legs; int Pruned = 0; };
inline bool Active(const Leg& Leg, double Now) { return Now >= Leg.Start && Now < Leg.End; }

inline Schedule Build(const std::vector<Edge>& Edges, const std::vector<Seed>& Seeds,
    int MaxConcurrent, int MaxDepth, int MaxLegs)
{
    Schedule Result;
    if (MaxConcurrent < 1 || MaxDepth < 1 || MaxLegs < 1) return Result;
    struct Pending { Leg Value; std::vector<int> Visited; int Parent = -1; };
    const auto Later = [](const Pending& A, const Pending& B)
    { return A.Value.Start == B.Value.Start ? A.Value.Key > B.Value.Key : A.Value.Start > B.Value.Start; };
    std::priority_queue<Pending, std::vector<Pending>, decltype(Later)> Queue(Later);
    const auto Enqueue = [&](int Node, int Skip, double Time, double RootTime,
        int Depth, const std::string& Key, const std::vector<int>& Visited, int Parent)
    {
        std::vector<int> SeenTargets;
        for (int I = 0; I < static_cast<int>(Edges.size()); ++I)
        {
            const auto& E = Edges[I];
            if (E.From != Node || E.To < 0 || E.To == Skip || E.To == Node ||
                !std::isfinite(E.Duration) || E.Duration <= 0 ||
                std::find(Visited.begin(), Visited.end(), E.To) != Visited.end() ||
                std::find(SeenTargets.begin(), SeenTargets.end(), E.To) != SeenTargets.end()) continue;
            SeenTargets.push_back(E.To);
            auto NextVisited = Visited;
            NextVisited.push_back(E.To);
            Queue.push({{Key + "/" + std::to_string(E.To), I, Depth,
                Time, Time + E.Duration, RootTime, true}, NextVisited, Parent});
        }
    };
    for (const auto& S : Seeds)
        if (S.Node >= 0 && std::isfinite(S.Time))
            Enqueue(S.Node, S.OriginalNext, S.Time, S.Time, 1, std::to_string(S.Node),
                {S.OriginalPrevious, S.Node}, -1);

    while (!Queue.empty())
    {
        Pending P = Queue.top();
        Queue.pop();
        // Starts are chronological. Count half-open intervals; an arriving parent frees its slot.
        const int ActiveCount = static_cast<int>(std::count_if(Result.Legs.begin(), Result.Legs.end(),
            [&](const Leg& L) { return Active(L, P.Value.Start); }));
        if (ActiveCount >= MaxConcurrent || static_cast<int>(Result.Legs.size()) >= MaxLegs)
        {
            ++Result.Pruned;
            continue;
        }
        const int Index = static_cast<int>(Result.Legs.size());
        Result.Legs.push_back(P.Value);
        if (P.Parent >= 0) Result.Legs[P.Parent].Terminal = false;
        if (P.Value.Depth < MaxDepth)
            Enqueue(Edges[P.Value.EdgeIndex].To, -1, P.Value.End, P.Value.RootTime,
                P.Value.Depth + 1, P.Value.Key, P.Visited, Index);
    }
    return Result;
}
}
