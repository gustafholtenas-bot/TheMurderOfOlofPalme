#include "Traffic/TMOPTrafficNetworkDirector.h"

#include "EngineUtils.h"
#include "Traffic/TMOPTrafficIntersectionComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"

ATMOPTrafficNetworkDirector::ATMOPTrafficNetworkDirector()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPTrafficNetworkDirector::BeginPlay()
{
    Super::BeginPlay();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    if (Network != nullptr) Network->DiscoverLanesInWorld();
    if (bValidateOnBeginPlay)
    {
        TArray<FString> Errors;
        ValidateCompleteTrafficNetwork(Errors);
        for (const FString& Error : Errors) UE_LOG(LogTemp, Error, TEXT("TMOP Traffic: %s"), *Error);
    }
}

bool ATMOPTrafficNetworkDirector::ValidateCompleteTrafficNetwork(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    if (Network == nullptr) { OutErrors.Add(TEXT("Traffic network subsystem is unavailable.")); return false; }
    Network->ValidateNetwork(OutErrors);
    UWorld* World = GetWorld();
    if (World == nullptr) return false;
    TSet<FName> IntersectionIds;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficIntersectionComponent*> Components;
        It->GetComponents<UTMOPTrafficIntersectionComponent>(Components);
        for (UTMOPTrafficIntersectionComponent* Intersection : Components)
        {
            if (!IsValid(Intersection)) continue;
            TArray<FString> Errors;
            Intersection->ValidateIntersection(Errors);
            for (const FString& Error : Errors)
                OutErrors.Add(FString::Printf(TEXT("Intersection '%s': %s"),
                    *Intersection->IntersectionId.ToString(), *Error));
            if (IntersectionIds.Contains(Intersection->IntersectionId))
                OutErrors.Add(FString::Printf(TEXT("Duplicate IntersectionId '%s'."),
                    *Intersection->IntersectionId.ToString()));
            IntersectionIds.Add(Intersection->IntersectionId);
            for (const FName LaneId : Intersection->IncomingLaneIds)
                if (!IsValid(Network->FindLane(LaneId))) OutErrors.Add(TEXT("Intersection references a missing incoming lane."));
            for (const FName LaneId : Intersection->OutgoingLaneIds)
                if (!IsValid(Network->FindLane(LaneId))) OutErrors.Add(TEXT("Intersection references a missing outgoing lane."));
        }
    }
    return OutErrors.IsEmpty();
}
