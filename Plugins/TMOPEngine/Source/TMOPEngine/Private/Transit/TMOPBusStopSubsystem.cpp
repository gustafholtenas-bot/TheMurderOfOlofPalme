#include "Transit/TMOPBusStopSubsystem.h"

#include "EngineUtils.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Transit/TMOPBusStopComponent.h"

void UTMOPBusStopSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Stops.Reset();
}

void UTMOPBusStopSubsystem::Deinitialize()
{
    Stops.Reset();
    Super::Deinitialize();
}

bool UTMOPBusStopSubsystem::RegisterStop(UTMOPBusStopComponent* Stop)
{
    if (!IsValid(Stop) || Stop->StopId.IsNone()) return false;
    if (TWeakObjectPtr<UTMOPBusStopComponent>* Existing = Stops.Find(Stop->StopId))
    {
        if (Existing->IsValid())
        {
            if (Existing->Get() == Stop) return true;
            UE_LOG(LogTemp, Error, TEXT("TMOP duplicate bus StopId '%s'."), *Stop->StopId.ToString());
            return false;
        }
        Stops.Remove(Stop->StopId);
    }
    Stops.Add(Stop->StopId, Stop);
    return true;
}

bool UTMOPBusStopSubsystem::UnregisterStop(UTMOPBusStopComponent* Stop)
{
    if (Stop == nullptr || Stop->StopId.IsNone()) return false;
    TWeakObjectPtr<UTMOPBusStopComponent>* Existing = Stops.Find(Stop->StopId);
    if (Existing == nullptr || Existing->Get() != Stop) return false;
    Stops.Remove(Stop->StopId);
    return true;
}

UTMOPBusStopComponent* UTMOPBusStopSubsystem::FindStop(const FName StopId) const
{
    const TWeakObjectPtr<UTMOPBusStopComponent>* Found = Stops.Find(StopId);
    return Found != nullptr ? Found->Get() : nullptr;
}

int32 UTMOPBusStopSubsystem::DiscoverStopsInWorld()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    for (auto It = Stops.CreateIterator(); It; ++It) if (!It.Value().IsValid()) It.RemoveCurrent();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPBusStopComponent*> Components;
        It->GetComponents<UTMOPBusStopComponent>(Components);
        for (UTMOPBusStopComponent* Stop : Components) RegisterStop(Stop);
    }
    int32 Count = 0;
    for (const TPair<FName, TWeakObjectPtr<UTMOPBusStopComponent>>& Pair : Stops)
        if (Pair.Value.IsValid()) ++Count;
    return Count;
}

bool UTMOPBusStopSubsystem::ValidateStops(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    for (const TPair<FName, TWeakObjectPtr<UTMOPBusStopComponent>>& Pair : Stops)
    {
        UTMOPBusStopComponent* Stop = Pair.Value.Get();
        if (!IsValid(Stop)) continue;
        TArray<FString> Errors;
        Stop->ValidateStop(Errors);
        for (const FString& Error : Errors)
            OutErrors.Add(FString::Printf(TEXT("Stop '%s': %s"), *Pair.Key.ToString(), *Error));
        if (Network == nullptr || !IsValid(Network->FindLane(Stop->LaneId)))
            OutErrors.Add(FString::Printf(TEXT("Stop '%s' references a missing lane."), *Pair.Key.ToString()));
    }
    return OutErrors.IsEmpty();
}
