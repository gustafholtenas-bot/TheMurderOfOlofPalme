#include "Traffic/TMOPTrafficNetworkSubsystem.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Traffic/TMOPTrafficLaneComponent.h"

void UTMOPTrafficNetworkSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Lanes.Reset();
}

void UTMOPTrafficNetworkSubsystem::Deinitialize()
{
    Lanes.Reset();
    Super::Deinitialize();
}

bool UTMOPTrafficNetworkSubsystem::RegisterLane(UTMOPTrafficLaneComponent* Lane)
{
    if (!IsValid(Lane) || Lane->LaneId.IsNone()) return false;
    if (TWeakObjectPtr<UTMOPTrafficLaneComponent>* Existing = Lanes.Find(Lane->LaneId))
    {
        if (Existing->IsValid())
        {
            if (Existing->Get() == Lane) return true;
            UE_LOG(LogTemp, Error, TEXT("TMOP duplicate traffic LaneId '%s'."), *Lane->LaneId.ToString());
            return false;
        }
        Lanes.Remove(Lane->LaneId);
    }
    Lanes.Add(Lane->LaneId, Lane);
    return true;
}

bool UTMOPTrafficNetworkSubsystem::UnregisterLane(UTMOPTrafficLaneComponent* Lane)
{
    if (Lane == nullptr || Lane->LaneId.IsNone()) return false;
    TWeakObjectPtr<UTMOPTrafficLaneComponent>* Existing = Lanes.Find(Lane->LaneId);
    if (Existing == nullptr || Existing->Get() != Lane) return false;
    Lanes.Remove(Lane->LaneId);
    return true;
}

UTMOPTrafficLaneComponent* UTMOPTrafficNetworkSubsystem::FindLane(const FName LaneId) const
{
    const TWeakObjectPtr<UTMOPTrafficLaneComponent>* Found = Lanes.Find(LaneId);
    return Found != nullptr ? Found->Get() : nullptr;
}

int32 UTMOPTrafficNetworkSubsystem::DiscoverLanesInWorld()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    for (auto It = Lanes.CreateIterator(); It; ++It) if (!It.Value().IsValid()) It.RemoveCurrent();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficLaneComponent*> Components;
        It->GetComponents<UTMOPTrafficLaneComponent>(Components);
        for (UTMOPTrafficLaneComponent* Lane : Components) RegisterLane(Lane);
    }
    return GetAllLaneIds().Num();
}

TArray<FName> UTMOPTrafficNetworkSubsystem::GetAllLaneIds() const
{
    TArray<FName> Result;
    for (const TPair<FName, TWeakObjectPtr<UTMOPTrafficLaneComponent>>& Pair : Lanes)
        if (Pair.Value.IsValid()) Result.Add(Pair.Key);
    Result.Sort([](const FName A, const FName B) { return A.LexicalLess(B); });
    return Result;
}

bool UTMOPTrafficNetworkSubsystem::ValidateNetwork(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    for (const TPair<FName, TWeakObjectPtr<UTMOPTrafficLaneComponent>>& Pair : Lanes)
    {
        UTMOPTrafficLaneComponent* Lane = Pair.Value.Get();
        if (!IsValid(Lane)) { OutErrors.Add(FString::Printf(TEXT("Lane '%s' is invalid."), *Pair.Key.ToString())); continue; }
        TArray<FString> Errors;
        Lane->ValidateLane(Errors);
        for (const FString& Error : Errors)
            OutErrors.Add(FString::Printf(TEXT("Lane '%s': %s"), *Pair.Key.ToString(), *Error));
        for (const FTMOPLaneConnection& Connection : Lane->NextLanes)
            if (Connection.bAllowed && !IsValid(FindLane(Connection.TargetLaneId)))
                OutErrors.Add(FString::Printf(TEXT("Lane '%s' references missing next lane '%s'."),
                    *Pair.Key.ToString(), *Connection.TargetLaneId.ToString()));
        if (!Lane->LeftNeighborLaneId.IsNone() && !IsValid(FindLane(Lane->LeftNeighborLaneId)))
            OutErrors.Add(FString::Printf(TEXT("Lane '%s' has missing left neighbor."), *Pair.Key.ToString()));
        if (!Lane->RightNeighborLaneId.IsNone() && !IsValid(FindLane(Lane->RightNeighborLaneId)))
            OutErrors.Add(FString::Printf(TEXT("Lane '%s' has missing right neighbor."), *Pair.Key.ToString()));
    }
    return OutErrors.IsEmpty();
}

bool UTMOPTrafficNetworkSubsystem::FindNearestLane(
    const FVector WorldLocation,
    FName& OutLaneId,
    float& OutDistanceAlongLaneCm) const
{
    OutLaneId = NAME_None;
    OutDistanceAlongLaneCm = 0.0f;
    double BestDistanceSquared = TNumericLimits<double>::Max();

    for (const TPair<FName, TWeakObjectPtr<UTMOPTrafficLaneComponent>>& Pair : Lanes)
    {
        UTMOPTrafficLaneComponent* Lane = Pair.Value.Get();
        if (!IsValid(Lane))
        {
            continue;
        }
        const float InputKey =
            Lane->FindInputKeyClosestToWorldLocation(WorldLocation);
        const FVector Closest =
            Lane->GetLocationAtSplineInputKey(InputKey, ESplineCoordinateSpace::World);
        const double DistanceSquared =
            FVector::DistSquared(WorldLocation, Closest);
        if (DistanceSquared < BestDistanceSquared ||
            (FMath::IsNearlyEqual(DistanceSquared, BestDistanceSquared) &&
                (OutLaneId.IsNone() || Pair.Key.LexicalLess(OutLaneId))))
        {
            BestDistanceSquared = DistanceSquared;
            OutLaneId = Pair.Key;
            OutDistanceAlongLaneCm =
                Lane->GetDistanceAlongSplineAtSplineInputKey(InputKey);
        }
    }
    return !OutLaneId.IsNone();
}

bool UTMOPTrafficNetworkSubsystem::FindLaneRoute(
    const FName StartLaneId,
    const FName DestinationLaneId,
    TArray<FName>& OutOrderedLaneIds) const
{
    OutOrderedLaneIds.Reset();
    if (!IsValid(FindLane(StartLaneId)) ||
        !IsValid(FindLane(DestinationLaneId)))
    {
        return false;
    }

    TMap<FName, double> Distances;
    TMap<FName, FName> Previous;
    TSet<FName> Unvisited;
    for (const FName LaneId : GetAllLaneIds())
    {
        Distances.Add(LaneId, TNumericLimits<double>::Max());
        Unvisited.Add(LaneId);
    }
    Distances.FindOrAdd(StartLaneId) = 0.0;

    while (!Unvisited.IsEmpty())
    {
        FName Current = NAME_None;
        double CurrentDistance = TNumericLimits<double>::Max();
        for (const FName Candidate : Unvisited)
        {
            const double CandidateDistance =
                Distances.FindRef(Candidate);
            if (CandidateDistance < CurrentDistance ||
                (FMath::IsNearlyEqual(CandidateDistance, CurrentDistance) &&
                    (Current.IsNone() || Candidate.LexicalLess(Current))))
            {
                Current = Candidate;
                CurrentDistance = CandidateDistance;
            }
        }
        if (Current.IsNone() ||
            CurrentDistance == TNumericLimits<double>::Max())
        {
            break;
        }
        Unvisited.Remove(Current);
        if (Current == DestinationLaneId)
        {
            break;
        }

        const UTMOPTrafficLaneComponent* Lane = FindLane(Current);
        if (!IsValid(Lane))
        {
            continue;
        }
        TArray<FTMOPLaneConnection> Connections = Lane->NextLanes;
        Connections.Sort([](const FTMOPLaneConnection& A,
            const FTMOPLaneConnection& B)
        {
            return A.TargetLaneId.LexicalLess(B.TargetLaneId);
        });
        for (const FTMOPLaneConnection& Connection : Connections)
        {
            if (!Connection.bAllowed ||
                !Unvisited.Contains(Connection.TargetLaneId))
            {
                continue;
            }
            const UTMOPTrafficLaneComponent* NextLane =
                FindLane(Connection.TargetLaneId);
            if (!IsValid(NextLane))
            {
                continue;
            }
            const double CandidateDistance =
                CurrentDistance + FMath::Max(1.0f, NextLane->GetSplineLength());
            double& KnownDistance =
                Distances.FindOrAdd(Connection.TargetLaneId);
            if (CandidateDistance < KnownDistance)
            {
                KnownDistance = CandidateDistance;
                Previous.Add(Connection.TargetLaneId, Current);
            }
        }
    }

    if (StartLaneId != DestinationLaneId &&
        !Previous.Contains(DestinationLaneId))
    {
        return false;
    }
    for (FName LaneId = DestinationLaneId; !LaneId.IsNone();)
    {
        OutOrderedLaneIds.Insert(LaneId, 0);
        if (LaneId == StartLaneId)
        {
            return true;
        }
        const FName* Parent = Previous.Find(LaneId);
        LaneId = Parent != nullptr ? *Parent : NAME_None;
    }
    OutOrderedLaneIds.Reset();
    return false;
}

bool UTMOPTrafficNetworkSubsystem::FindNearestReachableLane(
    const FVector WorldLocation,
    const FName StartLaneId,
    FName& OutLaneId,
    float& OutDistanceAlongLaneCm,
    TArray<FName>& OutOrderedLaneIds,
    const int32 MaximumCandidates) const
{
    struct FCandidate
    {
        FName LaneId = NAME_None;
        float DistanceAlongCm = 0.0f;
        double DistanceSquared = 0.0;
    };

    OutLaneId = NAME_None;
    OutDistanceAlongLaneCm = 0.0f;
    OutOrderedLaneIds.Reset();
    if (!IsValid(FindLane(StartLaneId))) return false;

    TArray<FCandidate> Candidates;
    for (const FName LaneId : GetAllLaneIds())
    {
        UTMOPTrafficLaneComponent* Lane = FindLane(LaneId);
        if (!IsValid(Lane)) continue;
        const float InputKey =
            Lane->FindInputKeyClosestToWorldLocation(WorldLocation);
        const FVector Closest = Lane->GetLocationAtSplineInputKey(
            InputKey, ESplineCoordinateSpace::World);
        FCandidate& Candidate = Candidates.AddDefaulted_GetRef();
        Candidate.LaneId = LaneId;
        Candidate.DistanceAlongCm =
            Lane->GetDistanceAlongSplineAtSplineInputKey(InputKey);
        Candidate.DistanceSquared =
            FVector::DistSquared(WorldLocation, Closest);
    }
    Candidates.Sort([](const FCandidate& A, const FCandidate& B)
    {
        return A.DistanceSquared < B.DistanceSquared ||
            (FMath::IsNearlyEqual(A.DistanceSquared, B.DistanceSquared) &&
                A.LaneId.LexicalLess(B.LaneId));
    });

    const int32 CandidateCount = FMath::Min(
        Candidates.Num(), FMath::Max(1, MaximumCandidates));
    for (int32 Index = 0; Index < CandidateCount; ++Index)
    {
        TArray<FName> Route;
        if (!FindLaneRoute(StartLaneId, Candidates[Index].LaneId, Route))
            continue;
        OutLaneId = Candidates[Index].LaneId;
        OutDistanceAlongLaneCm = Candidates[Index].DistanceAlongCm;
        OutOrderedLaneIds = MoveTemp(Route);
        return true;
    }
    return false;
}
