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
