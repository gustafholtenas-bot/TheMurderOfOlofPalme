#include "Traffic/TMOPTrafficSignalDirector.h"

#include "EngineUtils.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Traffic/TMOPTrafficSignalController.h"
#include "Traffic/TMOPTrafficStopLineComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"

ATMOPTrafficSignalDirector::ATMOPTrafficSignalDirector()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPTrafficSignalDirector::BeginPlay()
{
    Super::BeginPlay();
    DiscoverSignalSystem();
}

void ATMOPTrafficSignalDirector::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    UpdateAccumulator += DeltaSeconds;
    if (UpdateAccumulator >= UpdateIntervalSeconds)
    {
        UpdateAccumulator = 0.0f;
        UpdateVehicleConstraints();
    }
}

int32 ATMOPTrafficSignalDirector::DiscoverSignalSystem()
{
    StopLines.Reset();
    Controllers.Reset();
    UWorld* World = GetWorld();
    if (World == nullptr) return 0;
    for (TActorIterator<ATMOPTrafficSignalController> It(World); It; ++It) Controllers.Add(*It);
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficStopLineComponent*> Components;
        It->GetComponents<UTMOPTrafficStopLineComponent>(Components);
        for (UTMOPTrafficStopLineComponent* StopLine : Components) if (IsValid(StopLine)) StopLines.Add(StopLine);
    }
    return StopLines.Num();
}

void ATMOPTrafficSignalDirector::UpdateVehicleConstraints()
{
    UWorld* World = GetWorld();
    if (World == nullptr) return;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPTrafficVehicleMovementComponent*> Vehicles;
        It->GetComponents<UTMOPTrafficVehicleMovementComponent>(Vehicles);
        for (UTMOPTrafficVehicleMovementComponent* Vehicle : Vehicles)
        {
            if (!IsValid(Vehicle)) continue;
            for (UTMOPTrafficStopLineComponent* StopLine : StopLines)
            {
                if (!IsValid(StopLine)) continue;
                const FName ConstraintId(*FString::Printf(TEXT("SIGNAL_%s"), *StopLine->StopLineId.ToString()));
                if (StopLine->LaneId != Vehicle->CurrentLaneId ||
                    Vehicle->DistanceAlongLane > StopLine->DistanceAlongLane + 50.0f)
                {
                    Vehicle->ClearNamedStopConstraint(ConstraintId);
                    continue;
                }
                ATMOPTrafficSignalController* Controller = FindControllerForGroup(StopLine->SignalGroupId);
                bool bFound = false;
                const ETMOPTrafficSignalState State = IsValid(Controller)
                    ? Controller->GetGroupState(StopLine->SignalGroupId, bFound)
                    : ETMOPTrafficSignalState::Disabled;
                const bool bMustStop = bFound && (State == ETMOPTrafficSignalState::Red ||
                    State == ETMOPTrafficSignalState::RedYellow ||
                    (bStopOnYellow && State == ETMOPTrafficSignalState::Yellow));
                if (bMustStop)
                    Vehicle->SetNamedStopConstraint(ConstraintId,
                        FMath::Max(0.0f, StopLine->DistanceAlongLane - StopLine->StopBufferCm));
                else Vehicle->ClearNamedStopConstraint(ConstraintId);
            }
        }
    }
}

ATMOPTrafficSignalController* ATMOPTrafficSignalDirector::FindControllerForGroup(
    const FName SignalGroupId) const
{
    for (ATMOPTrafficSignalController* Controller : Controllers)
    {
        if (!IsValid(Controller)) continue;
        bool bFound = false;
        Controller->GetGroupState(SignalGroupId, bFound);
        if (bFound) return Controller;
    }
    return nullptr;
}

bool ATMOPTrafficSignalDirector::ValidateSignalSystem(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    UTMOPTrafficNetworkSubsystem* Network = GetGameInstance() != nullptr
        ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    TSet<FName> StopLineIds;
    for (UTMOPTrafficStopLineComponent* StopLine : StopLines)
    {
        if (!IsValid(StopLine)) continue;
        TArray<FString> Errors;
        StopLine->ValidateStopLine(Errors);
        for (const FString& Error : Errors)
            OutErrors.Add(FString::Printf(TEXT("Stop line '%s': %s"), *StopLine->StopLineId.ToString(), *Error));
        if (StopLineIds.Contains(StopLine->StopLineId))
            OutErrors.Add(FString::Printf(TEXT("Duplicate StopLineId '%s'."), *StopLine->StopLineId.ToString()));
        StopLineIds.Add(StopLine->StopLineId);
        if (Network == nullptr || !IsValid(Network->FindLane(StopLine->LaneId)))
            OutErrors.Add(FString::Printf(TEXT("Stop line '%s' references a missing lane."), *StopLine->StopLineId.ToString()));
        if (!IsValid(FindControllerForGroup(StopLine->SignalGroupId)))
            OutErrors.Add(FString::Printf(TEXT("Stop line '%s' has no signal controller."), *StopLine->StopLineId.ToString()));
    }
    for (ATMOPTrafficSignalController* Controller : Controllers)
    {
        if (!IsValid(Controller)) continue;
        TArray<FString> Errors;
        Controller->ValidateController(Errors);
        OutErrors.Append(Errors);
    }
    return OutErrors.IsEmpty();
}
