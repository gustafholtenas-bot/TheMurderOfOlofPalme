#include "../../Plugins/TMOPEngine/Source/TMOPEngine/Private/Killer/TMOPKillerBranchSchedule.h"
#include <iostream>
#include <cstdlib>
#include <limits>
#include <set>

using namespace TMOPKillerBranches;
int Checks = 0;
void Check(bool Condition, const char* Name)
{
    ++Checks;
    if (!Condition) { std::cerr << "FAIL: " << Name << '\n'; std::exit(1); }
}
int Count(const Schedule& S, double Now)
{
    return static_cast<int>(std::count_if(S.Legs.begin(), S.Legs.end(),
        [Now](const Leg& L) { return Active(L, Now); }));
}
int main()
{
    const std::vector<Edge> Graph = {{0,1,2}, {0,2,2}, {0,9,2}, {2,3,3}, {2,4,4},
        {3,5,2}, {4,5,1}, {5,0,2}, {2,0,2}, {2,3,3}};
    const std::vector<Seed> Seeds = {{0,1,9,100}};
    const auto S = Build(Graph, Seeds, 24, 8, 512);
    Check(S.Legs.size() == 5, "split twice, no original direction/backtracking/duplicate edge");
    Check(S.Legs[0].Key == "0/2", "only alternative leaves original corner");
    Check(Count(S,99.9) == 0, "no ghosts before original arrival");
    Check(Count(S,100) == 1, "first ghost appears at arrival");
    Check(Count(S,102) == 2, "parent ends exactly when two children branch");
    Check(Count(S,105.5) == 2, "branches keep independent travel times");
    Check(Count(S,107) == 0, "dead-end branches finish without cycling");
    Check(!S.Legs.front().Terminal, "parent with children does not fade at corner");
    Check(S.Legs.back().Terminal, "last leg fades out");
    Check(std::count_if(S.Legs.begin(),S.Legs.end(),[](const Leg& L)
        { return L.Key == "0/2/3/5" || L.Key == "0/2/4/5"; }) == 2,
        "separate hypotheses may reconverge without suppressing each other");
    const auto One = Build(Graph, Seeds, 1, 8, 512);
    Check(One.Pruned > 0, "population budget reports pruned alternatives");
    Check(Count(One,102) == 1, "population cap admits one continuation at the next corner");
    for (double Now = 98; Now < 110; Now += 0.125)
        if (Count(One,Now) > 1) Check(false, "population cap across whole timeline");
    const auto Shallow = Build(Graph, Seeds, 24, 1, 512);
    Check(Shallow.Legs.size()==1 && Shallow.Legs[0].Terminal, "depth cap ends branch cleanly");
    const auto Small = Build(Graph, Seeds, 24, 8, 2);
    Check(Small.Legs.size()==2 && Small.Pruned>0, "total-leg budget bounds expansion");
    const auto Disabled = Build(Graph, Seeds, 0, 8, 512);
    Check(Disabled.Legs.empty(), "disabled budget produces no actors");
    const auto Invalid = Build({{0,1,0},{0,2,-1},{0,3,std::numeric_limits<double>::quiet_NaN()},
        {0,0,1},{0,-1,1}}, {{0,-1,-1,100}},24,8,512);
    Check(Invalid.Legs.empty(), "invalid edges cannot cause infinite or negative-time routes");
    const auto Again = Build(Graph, Seeds, 24, 8, 512);
    Check(Again.Legs.size()==S.Legs.size() && Count(Again,102.5)==Count(S,102.5),
        "same graph and arrivals reproduce the same seek result");
    Check(Count(S,106)==2 && Count(S,101)==1 && Count(S,106)==2,
        "forward then backward then forward sampling does not accumulate ghosts");
    Check(Build(Graph, {},24,8,512).Legs.empty(), "loop reset clears every branch seed");
    const auto TwoRoots = Build({{0,1,4},{2,3,4}}, {{0,-1,-1,100},{2,-1,-1,102}},1,8,512);
    Check(TwoRoots.Legs.size()==1 && TwoRoots.Pruned==1, "population cap shared across original-route corners");
    std::set<std::string> Keys;
    for (const auto& L : S.Legs) Keys.insert(L.Key);
    Check(Keys.size()==S.Legs.size(), "branch keys stay unique at reconverging corners");
    std::cout << Checks << " killer-branch checks passed\n";
}
