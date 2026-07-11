#include "Traffic/TMOPTrafficSignalController.h"

#include "Traffic/TMOPTrafficSignalComponent.h"

ATMOPTrafficSignalController::ATMOPTrafficSignalController()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ATMOPTrafficSignalController::BeginPlay()
{
    Super::BeginPlay();
    SetPhase(FMath::Clamp(InitialPhaseIndex, 0, FMath::Max(0, Phases.Num() - 1)));
}

void ATMOPTrafficSignalController::Tick(const float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bCycleAutomatically || Phases.IsEmpty() || CurrentPhaseIndex == INDEX_NONE) return;
    RemainingPhaseSeconds -= DeltaSeconds;
    if (RemainingPhaseSeconds <= 0.0f) SetPhase((CurrentPhaseIndex + 1) % Phases.Num());
}

bool ATMOPTrafficSignalController::SetPhase(const int32 NewPhaseIndex)
{
    if (!Phases.IsValidIndex(NewPhaseIndex)) return false;
    CurrentPhaseIndex = NewPhaseIndex;
    RemainingPhaseSeconds = FMath::Max(0.1f, Phases[NewPhaseIndex].DurationSeconds);
    ApplyCurrentPhase();
    return true;
}

void ATMOPTrafficSignalController::ApplyCurrentPhase()
{
    if (!Phases.IsValidIndex(CurrentPhaseIndex)) return;
    RuntimeStates.Reset();
    for (const FTMOPSignalGroupState& State : Phases[CurrentPhaseIndex].GroupStates)
        if (!State.SignalGroupId.IsNone()) RuntimeStates.Add(State.SignalGroupId, State.State);
    TArray<UTMOPTrafficSignalComponent*> Signals;
    GetComponents<UTMOPTrafficSignalComponent>(Signals);
    for (UTMOPTrafficSignalComponent* Signal : Signals)
    {
        bool bFound = false;
        const ETMOPTrafficSignalState State = GetGroupState(Signal->SignalGroupId, bFound);
        if (bFound) Signal->ApplySignalState(State);
    }
}

void ATMOPTrafficSignalController::ForceGroupState(const FName SignalGroupId,
    const ETMOPTrafficSignalState NewState)
{
    if (SignalGroupId.IsNone()) return;
    RuntimeStates.Add(SignalGroupId, NewState);
    TArray<UTMOPTrafficSignalComponent*> Signals;
    GetComponents<UTMOPTrafficSignalComponent>(Signals);
    for (UTMOPTrafficSignalComponent* Signal : Signals)
        if (IsValid(Signal) && Signal->SignalGroupId == SignalGroupId) Signal->ApplySignalState(NewState);
}

ETMOPTrafficSignalState ATMOPTrafficSignalController::GetGroupState(const FName SignalGroupId,
    bool& bFound) const
{
    const ETMOPTrafficSignalState* Found = RuntimeStates.Find(SignalGroupId);
    bFound = Found != nullptr;
    return Found != nullptr ? *Found : ETMOPTrafficSignalState::Disabled;
}

bool ATMOPTrafficSignalController::ValidateController(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    if (IntersectionId.IsNone()) OutErrors.Add(TEXT("IntersectionId is missing."));
    if (Phases.IsEmpty()) OutErrors.Add(TEXT("No signal phases are configured."));
    for (int32 Index = 0; Index < Phases.Num(); ++Index)
    {
        if (Phases[Index].DurationSeconds <= 0.0f)
            OutErrors.Add(FString::Printf(TEXT("Phase %d has invalid duration."), Index));
        TSet<FName> Groups;
        for (const FTMOPSignalGroupState& State : Phases[Index].GroupStates)
        {
            if (State.SignalGroupId.IsNone() || Groups.Contains(State.SignalGroupId))
                OutErrors.Add(FString::Printf(TEXT("Phase %d has an invalid or duplicate signal group."), Index));
            Groups.Add(State.SignalGroupId);
        }
    }
    return OutErrors.IsEmpty();
}
