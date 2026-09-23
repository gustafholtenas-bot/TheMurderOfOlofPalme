#include "Traffic/TMOPTrafficSignalController.h"
#include "Traffic/TMOPTrafficSignalComponent.h"
#include "Traffic/TMOPSignalTiming.h"
#include "Time/TMOPClockSubsystem.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "Components/SceneComponent.h"
#include "JsonObjectConverter.h"

namespace
{
    bool Released(ETMOPTrafficSignalState S)
    {
        return S == ETMOPTrafficSignalState::Green || S == ETMOPTrafficSignalState::GreenYellow ||
            S == ETMOPTrafficSignalState::Yellow || S == ETMOPTrafficSignalState::RedYellow;
    }
}
ATMOPTrafficSignalController::ATMOPTrafficSignalController()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
    SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
}
void ATMOPTrafficSignalController::BeginPlay()
{
    Super::BeginPlay();
    RefreshSignalHeads();
    TArray<FString> Errors;
    bProgramValid = ValidateController(Errors);
    for (const FString& Error : Errors)
        UE_LOG(LogTemp, Error, TEXT("TMOP signal %s: %s"), *IntersectionId.ToString(), *Error);
    if (GetGameInstance())
        if (auto* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
            EvaluateAtTime(Clock->GetCurrentTimeSecondsExact());
}
void ATMOPTrafficSignalController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bVisualClockOnly && Tags.Contains(TEXT("TMOP_AuthoritativeHistory"))) return;
    if (GetGameInstance())
        if (auto* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
            if (bVisualClockOnly || !Clock->bAuthoritativePlayback) EvaluateAtTime(Clock->GetCurrentTimeSecondsExact());
}
bool ATMOPTrafficSignalController::HasGroup(FName Id) const
{
    return !Id.IsNone() && Groups.ContainsByPredicate([Id](const auto& G){ return G.GroupId == Id; });
}
bool ATMOPTrafficSignalController::IsPedestrianGroup(FName Id) const
{
    const auto* Group = Groups.FindByPredicate([Id](const auto& G){ return G.GroupId == Id; });
    return Group && Group->bPedestrian;
}
void ATMOPTrafficSignalController::RefreshSignalHeads()
{
    SignalHeads.Reset();
    if (!GetWorld()) return;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        TArray<UTMOPTrafficSignalComponent*> Heads;
        It->GetComponents(Heads);
        for (auto* Head : Heads)
            if (IsValid(Head) && (Head->IntersectionId == IntersectionId ||
                (Head->IntersectionId.IsNone() && Head->GetOwner() == this)))
                SignalHeads.Add(Head);
    }
    PublishSignals();
}
void ATMOPTrafficSignalController::PublishSignals()
{
    for (const auto& Weak : SignalHeads)
        if (auto* Head = Weak.Get())
        {
            bool Found = false;
            const auto State = GetGroupState(Head->SignalGroupId, Found);
            Head->ApplySignalState(Found ? State : ETMOPTrafficSignalState::Red);
        }
}
void ATMOPTrafficSignalController::SetAllRed()
{
    RuntimeStates.Reset();
    for (const auto& G : Groups) RuntimeStates.Add(G.GroupId, ETMOPTrafficSignalState::Red);
    CurrentPhaseIndex = INDEX_NONE;
    PublishSignals();
}
void ATMOPTrafficSignalController::EvaluateAtTime(double Time)
{
    if (!bProgramValid) { SetAllRed(); return; }
    if (!bCycleAutomatically)
    {
        if (CurrentPhaseIndex == INDEX_NONE) SetPhase(InitialPhaseIndex);
        return;
    }
    std::vector<double> Durations;
    Durations.reserve(Phases.Num());
    for (const auto& Phase : Phases) Durations.push_back(Phase.DurationSeconds);
    const auto Result = TMOPSignalTiming::Evaluate(Durations, Time,
        ProgramEpochSeconds, CycleOffsetSeconds, InitialPhaseIndex);
    if (Result.Index < 0) { SetAllRed(); return; }
    PhaseEndSecond = Time + Result.Remaining;
    if (CurrentPhaseIndex != Result.Index)
    {
        CurrentPhaseIndex = Result.Index;
        ApplyCurrentPhase();
    }
}
bool ATMOPTrafficSignalController::SetPhase(int32 Index)
{
    TArray<FString> Errors;
    bProgramValid = ValidateController(Errors);
    if (!bProgramValid || !Phases.IsValidIndex(Index)) { SetAllRed(); return false; }
    CurrentPhaseIndex = Index;
    double Time = ProgramEpochSeconds;
    if (GetGameInstance())
        if (auto* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>()) Time = Clock->GetCurrentTimeSecondsExact();
    PhaseEndSecond = Time + Phases[Index].DurationSeconds;
    ApplyCurrentPhase();
    return true;
}
void ATMOPTrafficSignalController::ApplyCurrentPhase()
{
    RuntimeStates.Reset();
    for (const auto& G : Groups) RuntimeStates.Add(G.GroupId, ETMOPTrafficSignalState::Red);
    if (Phases.IsValidIndex(CurrentPhaseIndex))
        for (const auto& State : Phases[CurrentPhaseIndex].GroupStates)
            RuntimeStates.Add(State.SignalGroupId, State.State);
    PublishSignals();
}
void ATMOPTrafficSignalController::ForceGroupState(FName Id, ETMOPTrafficSignalState State)
{
    if (!HasGroup(Id)) return;
    if (Released(State))
        for (const auto& Conflict : Conflicts)
        {
            const FName Other = Conflict.GroupA == Id ? Conflict.GroupB : Conflict.GroupB == Id ? Conflict.GroupA : NAME_None;
            bool Found = false;
            if (!Other.IsNone() && Released(GetGroupState(Other, Found)))
            {
                UE_LOG(LogTemp, Error, TEXT("TMOP rejected conflicting forced signal in %s"), *IntersectionId.ToString());
                return;
            }
        }
    if (IsPedestrianGroup(Id) && State != ETMOPTrafficSignalState::Red && State != ETMOPTrafficSignalState::Green) return;
    RuntimeStates.Add(Id, State);
    PublishSignals();
}
ETMOPTrafficSignalState ATMOPTrafficSignalController::GetGroupState(FName Id, bool& Found) const
{
    const auto* State = RuntimeStates.Find(Id);
    Found = State != nullptr;
    return State ? *State : ETMOPTrafficSignalState::Red;
}
float ATMOPTrafficSignalController::GetRemainingSeconds(double Time) const
{
    return float(FMath::Max(0.0, PhaseEndSecond - Time));
}
void ATMOPTrafficSignalController::BuildProtectedProgram()
{
    Phases.Reset();
    auto AddPhase = [this](const TArray<FName>& Active, ETMOPTrafficSignalState VehicleState,
        ETMOPTrafficSignalState PedestrianState, float Duration)
    {
        FTMOPTrafficSignalPhase Phase;
        Phase.DurationSeconds = Duration;
        for (const auto& G : Groups)
        {
            FTMOPSignalGroupState S;
            S.SignalGroupId = G.GroupId;
            S.State = Active.Contains(G.GroupId) ? (G.bPedestrian ? PedestrianState : VehicleState) : ETMOPTrafficSignalState::Red;
            Phase.GroupStates.Add(S);
        }
        Phases.Add(Phase);
    };
    // Each stage starts with red/amber; pedestrians stay red during preparation.
    for (const auto& Stage : Stages)
    {
        AddPhase(Stage.GreenGroups, ETMOPTrafficSignalState::RedYellow, ETMOPTrafficSignalState::Red, RedAmberSeconds);
        AddPhase(Stage.GreenGroups, ETMOPTrafficSignalState::Green, ETMOPTrafficSignalState::Green, Stage.GreenSeconds);
        AddPhase(Stage.GreenGroups, bStockholm1986 ? ETMOPTrafficSignalState::GreenYellow : ETMOPTrafficSignalState::Yellow,
            ETMOPTrafficSignalState::Red, AmberSeconds);
        AddPhase(TArray<FName>(), ETMOPTrafficSignalState::Red, ETMOPTrafficSignalState::Red, Stage.ClearanceSeconds);
    }
    CurrentPhaseIndex = INDEX_NONE;
    TArray<FString> Errors;
    bProgramValid = ValidateController(Errors);
    for (const auto& Error : Errors) UE_LOG(LogTemp, Error, TEXT("TMOP signal builder: %s"), *Error);
}
bool ATMOPTrafficSignalController::ValidateController(TArray<FString>& Errors) const
{
    Errors.Reset();
    if (IntersectionId.IsNone()) Errors.Add(TEXT("IntersectionId is missing."));
    if (Groups.IsEmpty()) Errors.Add(TEXT("No explicit signal groups. Configure Groups before baking."));
    if (Phases.IsEmpty()) Errors.Add(TEXT("No signal phases."));
    if (!Phases.IsValidIndex(InitialPhaseIndex)) Errors.Add(TEXT("Invalid InitialPhaseIndex."));
    if (!FMath::IsFinite(ProgramEpochSeconds) || !FMath::IsFinite(CycleOffsetSeconds)) Errors.Add(TEXT("Invalid program clock."));
    TSet<FName> Ids;
    for (const auto& G : Groups)
    {
        if (G.GroupId.IsNone() || Ids.Contains(G.GroupId)) Errors.Add(TEXT("Missing/duplicate GroupId."));
        Ids.Add(G.GroupId);
    }
    for (const auto& C : Conflicts)
        if (C.GroupA == C.GroupB || !Ids.Contains(C.GroupA) || !Ids.Contains(C.GroupB))
            Errors.Add(TEXT("Invalid conflict pair."));
    for (const auto& Stage : Stages)
        for (FName Id : Stage.GreenGroups)
            if (!Ids.Contains(Id)) Errors.Add(TEXT("Stage references an unknown group."));
    for (int32 I = 0; I < Phases.Num(); ++I)
    {
        const auto& P = Phases[I];
        if (!FMath::IsFinite(P.DurationSeconds) || P.DurationSeconds <= 0) Errors.Add(TEXT("Invalid phase duration."));
        TMap<FName, ETMOPTrafficSignalState> States;
        for (const auto& S : P.GroupStates)
        {
            if (!Ids.Contains(S.SignalGroupId) || States.Contains(S.SignalGroupId)) Errors.Add(TEXT("Unknown/duplicate phase group."));
            if (IsPedestrianGroup(S.SignalGroupId) && S.State != ETMOPTrafficSignalState::Red && S.State != ETMOPTrafficSignalState::Green)
                Errors.Add(TEXT("Pedestrian groups support only red/green."));
            if (S.State == ETMOPTrafficSignalState::Disabled || uint8(S.State) > uint8(ETMOPTrafficSignalState::GreenYellow))
                Errors.Add(TEXT("Disabled/unknown is not a safe phase state."));
            States.Add(S.SignalGroupId, S.State);
        }
        for (const auto& C : Conflicts)
            if (Released(States.FindRef(C.GroupA)) && Released(States.FindRef(C.GroupB)))
                Errors.Add(FString::Printf(TEXT("Conflicting movements in phase %d."), I));
    }
    // Require a full all-red phase between incompatible releases, including cycle wrap.
    for (const auto& C : Conflicts)
    {
        FName Last = NAME_None;
        for (int32 I = 0; I < Phases.Num() * 2; ++I)
        {
            const auto& P = Phases[I % Phases.Num()];
            bool A = false, B = false;
            for (const auto& S : P.GroupStates)
            {
                if (S.SignalGroupId == C.GroupA) A = Released(S.State);
                if (S.SignalGroupId == C.GroupB) B = Released(S.State);
            }
            const FName Now = A ? C.GroupA : B ? C.GroupB : NAME_None;
            if (!Now.IsNone() && !Last.IsNone() && Last != Now)
                Errors.Add(TEXT("Conflicting groups change without an all-red clearance phase."));
            Last = Now;
        }
    }
    return Errors.IsEmpty();
}
TArray<FTMOPSignalGroupState> ATMOPTrafficSignalController::CapturePlaybackSignals() const
{
    TArray<FTMOPSignalGroupState> Result;
    for (const auto& Pair : RuntimeStates)
    {
        FTMOPSignalGroupState S; S.SignalGroupId = Pair.Key; S.State = Pair.Value; Result.Add(S);
    }
    Result.Sort([](const auto& A, const auto& B){ return A.SignalGroupId.LexicalLess(B.SignalGroupId); });
    return Result;
}
void ATMOPTrafficSignalController::RestorePlaybackSignals(const TArray<FTMOPSignalGroupState>& States)
{
    if (bVisualClockOnly)
    {
        if (GetGameInstance())
            if (auto* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
                EvaluateAtTime(Clock->GetCurrentTimeSecondsExact());
        return;
    }
    RuntimeStates.Reset();
    for (const auto& G : Groups) RuntimeStates.Add(G.GroupId, ETMOPTrafficSignalState::Red);
    for (const auto& S : States) RuntimeStates.Add(S.SignalGroupId, S.State);
    // Atomic restore: do not validate a partial mixture of the old and new phase.
    if (SignalHeads.IsEmpty()) RefreshSignalHeads();
    PublishSignals();
}
void ATMOPTrafficSignalController::RestorePlaybackPhase(int32 Index, double EndSecond)
{
    if (bVisualClockOnly) return;
    CurrentPhaseIndex = Index;
    PhaseEndSecond = EndSecond;
}
FString ATMOPTrafficSignalController::ConfigurationSignature() const
{
    FString Result = FString::Printf(TEXT("%s|%.9f|%.9f|%d|%d|%d"), *IntersectionId.ToString(),
        ProgramEpochSeconds, CycleOffsetSeconds, InitialPhaseIndex, bCycleAutomatically, bVisualClockOnly);
    auto Append = [&Result](const auto& Entries)
    {
        for (const auto& Entry : Entries) { FString Json; FJsonObjectConverter::UStructToJsonObjectString(Entry, Json); Result += Json; }
    };
    Append(Groups); Append(Conflicts); Append(Phases);
    return Result;
}
