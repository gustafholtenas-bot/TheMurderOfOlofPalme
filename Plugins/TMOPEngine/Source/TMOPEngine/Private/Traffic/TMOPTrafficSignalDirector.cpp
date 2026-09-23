#include "Traffic/TMOPTrafficSignalDirector.h"
#include "Traffic/TMOPTrafficSignalController.h"
#include "Traffic/TMOPTrafficSignalComponent.h"
#include "Traffic/TMOPTrafficStopLineComponent.h"
#include "Traffic/TMOPPedestrianCrossingComponent.h"
#include "Traffic/TMOPTrafficVehicleMovementComponent.h"
#include "Traffic/TMOPTrafficLaneComponent.h"
#include "Traffic/TMOPTrafficNetworkSubsystem.h"
#include "Time/TMOPClockSubsystem.h"
#include "Engine/GameInstance.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "Components/MeshComponent.h"

ATMOPTrafficSignalDirector::ATMOPTrafficSignalDirector()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PrePhysics;
}
void ATMOPTrafficSignalDirector::BeginPlay()
{
    Super::BeginPlay();
    DiscoverSignalSystem();
    ValidateAndLog();
}
void ATMOPTrafficSignalDirector::EndPlay(const EEndPlayReason::Type Reason)
{
    for (const auto& Weak : Vehicles)
        if (auto* Vehicle = Weak.Get())
            for (UTMOPTrafficStopLineComponent* Stop : StopLines)
                if (IsValid(Stop)) Vehicle->ClearNamedStopConstraint(FName(*FString::Printf(TEXT("SIGNAL_%s_%s"),
                    *Stop->IntersectionId.ToString(), *Stop->StopLineId.ToString())));
    Super::EndPlay(Reason);
}
void ATMOPTrafficSignalDirector::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (GetGameInstance())
        if (auto* Clock = GetGameInstance()->GetSubsystem<UTMOPClockSubsystem>())
            if (Clock->bAuthoritativePlayback) return;
    RegistryAccumulator += DeltaSeconds;
    if (RegistryAccumulator >= RegistryRefreshSeconds)
    {
        RegistryAccumulator = 0;
        DiscoverSignalSystem();
    }
    UpdateAccumulator += DeltaSeconds;
    if (UpdateAccumulator >= UpdateIntervalSeconds)
    {
        UpdateAccumulator = 0;
        UpdateVehicleConstraints();
    }
}
int32 ATMOPTrafficSignalDirector::DiscoverSignalSystem()
{
    // Remove our previous constraints before refreshing (removed stop lines must not linger).
    for (const auto& Weak : Vehicles)
        if (auto* Vehicle = Weak.Get())
            for (UTMOPTrafficStopLineComponent* Stop : StopLines)
                if (IsValid(Stop)) Vehicle->ClearNamedStopConstraint(FName(*FString::Printf(TEXT("SIGNAL_%s_%s"),
                    *Stop->IntersectionId.ToString(), *Stop->StopLineId.ToString())));
    StopLines.Reset(); Controllers.Reset(); Crossings.Reset(); Vehicles.Reset(); Pedestrians.Reset();
    if (!GetWorld()) return 0;
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        if (auto* Controller = Cast<ATMOPTrafficSignalController>(*It))
        {
            Controllers.Add(Controller);
            AddTickPrerequisiteActor(Controller);
            Controller->RefreshSignalHeads();
        }
        TArray<UTMOPTrafficStopLineComponent*> Stops; It->GetComponents(Stops);
        for (auto* C : Stops) if (IsValid(C)) StopLines.Add(C);
        TArray<UTMOPPedestrianCrossingComponent*> Walks; It->GetComponents(Walks);
        for (auto* C : Walks) if (IsValid(C)) Crossings.Add(C);
        TArray<UTMOPTrafficVehicleMovementComponent*> Drives; It->GetComponents(Drives);
        for (auto* C : Drives) if (IsValid(C))
        {
            Vehicles.Add(C);
            C->AddTickPrerequisiteActor(this);
        }
        if (auto* Character = Cast<ACharacter>(*It))
            if (!Character->IsPlayerControlled()) Pedestrians.Add(Character);
    }
    UpdateVehicleConstraints();
    return StopLines.Num();
}
ATMOPTrafficSignalController* ATMOPTrafficSignalDirector::FindController(FName Intersection, FName Group) const
{
    ATMOPTrafficSignalController* Found = nullptr;
    for (ATMOPTrafficSignalController* C : Controllers)
        if (IsValid(C) && C->HasGroup(Group) && (Intersection.IsNone() || C->IntersectionId == Intersection))
        {
            if (Found) return nullptr; // Ambiguous IDs fail closed.
            Found = C;
        }
    return Found;
}
bool ATMOPTrafficSignalDirector::CanEnterCrossing(FName Id) const
{
    if (!bControlPedestrians) return true;
    for (UTMOPPedestrianCrossingComponent* Crossing : Crossings)
        if (IsValid(Crossing) && Crossing->CrossingId == Id)
        {
            const auto* C = FindController(Crossing->IntersectionId, Crossing->SignalGroupId);
            bool Found = false;
            return C && C->GetGroupState(Crossing->SignalGroupId, Found) == ETMOPTrafficSignalState::Green && Found;
        }
    return false;
}
bool ATMOPTrafficSignalDirector::IsPedestrianPathBlocked(const FVector& From, const FVector& To) const
{
    for (UTMOPPedestrianCrossingComponent* Crossing : Crossings)
        if (IsValid(Crossing) && Crossing->CrossesEntrance(From, To, 35) && !CanEnterCrossing(Crossing->CrossingId))
            return true;
    return false;
}
bool ATMOPTrafficSignalDirector::IsPedestrianPathBlocked(UWorld* World, const FVector& From, const FVector& To)
{
    if (!World) return false;
    for (TActorIterator<ATMOPTrafficSignalDirector> It(World); It; ++It)
        if (It->IsPedestrianPathBlocked(From, To)) return true;
    return false;
}
void ATMOPTrafficSignalDirector::UpdateVehicleConstraints()
{
    if (!bControlVehicles)
    {
        for (const auto& Weak : Vehicles)
            if (auto* Vehicle = Weak.Get())
                for (UTMOPTrafficStopLineComponent* Stop : StopLines)
                    if (IsValid(Stop)) Vehicle->ClearNamedStopConstraint(FName(*FString::Printf(TEXT("SIGNAL_%s_%s"),
                        *Stop->IntersectionId.ToString(), *Stop->StopLineId.ToString())));
        return;
    }
    // Occupancy evaluated once per crossing, not once per car.
    TSet<FString> OccupiedGroups;
    for (UTMOPPedestrianCrossingComponent* Crossing : Crossings)
    {
        if (!IsValid(Crossing)) continue;
        bool Occupied = false;
        for (const auto& Weak : Pedestrians)
            if (const auto* Pawn = Weak.Get())
                if (!Pawn->IsHidden() && !Pawn->GetAttachParentActor() &&
                    Crossing->ContainsPedestrian(Pawn->GetActorLocation())) { Occupied = true; break; }
        if (Occupied)
            for (FName Group : Crossing->YieldingVehicleGroups)
                OccupiedGroups.Add(Crossing->IntersectionId.ToString() + TEXT("/") + Group.ToString());
    }
    for (const auto& Weak : Vehicles)
    {
        auto* V = Weak.Get();
        if (!V || V->GetOwner()->Tags.Contains(TEXT("TMOP_AuthoritativeHistory"))) continue;
        for (UTMOPTrafficStopLineComponent* Stop : StopLines)
        {
            if (!IsValid(Stop)) continue;
            const FName Key(*FString::Printf(TEXT("SIGNAL_%s_%s"),
                *Stop->IntersectionId.ToString(), *Stop->StopLineId.ToString()));
            const float Front = V->DistanceAlongLane + (Stop->bAccountForVehicleLength ? V->VehicleLengthCm * 0.5f : 0.0f);
            if (V->bRunRedLights || Stop->LaneId != V->CurrentLaneId || Front > Stop->DistanceAlongLane + 5.0f)
            { V->ClearNamedStopConstraint(Key); continue; }
            const auto* C = FindController(Stop->IntersectionId, Stop->SignalGroupId);
            bool Found = false;
            const auto State = C ? C->GetGroupState(Stop->SignalGroupId, Found) : ETMOPTrafficSignalState::Red;
            const bool Amber = State == ETMOPTrafficSignalState::Yellow || State == ETMOPTrafficSignalState::GreenYellow;
            const float Braking = FMath::Square(V->CurrentSpeedCmPerSecond) /
                (2.0f * FMath::Max(1.0f, V->ServiceBrakeCmPerSecondSquared));
            const bool StopForAmber = bStopOnYellow && Amber &&
                Stop->DistanceAlongLane - Front - Stop->StopBufferCm >= Braking;
            const bool Occupied = OccupiedGroups.Contains(Stop->IntersectionId.ToString() + TEXT("/") + Stop->SignalGroupId.ToString());
            const bool MustStop = !Found || State == ETMOPTrafficSignalState::Disabled ||
                State == ETMOPTrafficSignalState::Red || State == ETMOPTrafficSignalState::RedYellow || StopForAmber || Occupied;
            if (MustStop)
            {
                const float Center = Stop->DistanceAlongLane - Stop->StopBufferCm -
                    (Stop->bAccountForVehicleLength ? V->VehicleLengthCm * 0.5f : 0.0f);
                V->SetNamedStopConstraint(Key, FMath::Max(V->DistanceAlongLane, Center));
            }
            else V->ClearNamedStopConstraint(Key);
        }
    }
}
bool ATMOPTrafficSignalDirector::IsRouteSignalControlled(UWorld* World, const TArray<FName>& LaneIds)
{
    if (!World) return false;
    for (TActorIterator<ATMOPTrafficSignalDirector> It(World); It; ++It)
        for (UTMOPTrafficStopLineComponent* Stop : It->StopLines)
            if (It->bControlVehicles && IsValid(Stop) && LaneIds.Contains(Stop->LaneId)) return true;
    return false;
}
bool ATMOPTrafficSignalDirector::ValidateSignalSystem(TArray<FString>& Errors) const
{
    Errors.Reset();
    auto* Network = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTMOPTrafficNetworkSubsystem>() : nullptr;
    TMap<FName, UTMOPTrafficLaneComponent*> EditorLanes;
    if (!Network && GetWorld())
        for (TActorIterator<AActor> It(GetWorld()); It; ++It)
        {
            TArray<UTMOPTrafficLaneComponent*> Lanes; It->GetComponents(Lanes);
            for (auto* Lane : Lanes)
            {
                if (EditorLanes.Contains(Lane->LaneId)) Errors.Add(TEXT("Duplicate lane ID."));
                EditorLanes.Add(Lane->LaneId, Lane);
            }
        }
    TSet<FName> Intersections, CrossingIds;
    for (ATMOPTrafficSignalController* C : Controllers)
    {
        if (!IsValid(C)) continue;
        if (Intersections.Contains(C->IntersectionId)) Errors.Add(TEXT("Duplicate IntersectionId."));
        Intersections.Add(C->IntersectionId);
        TArray<FString> Local; C->ValidateController(Local); Errors.Append(Local);
    }
    TSet<FString> StopIds;
    for (UTMOPTrafficStopLineComponent* Stop : StopLines)
    {
        if (!IsValid(Stop)) continue;
        TArray<FString> Local; Stop->ValidateStopLine(Local); Errors.Append(Local);
        const FString Id = Stop->IntersectionId.ToString() + TEXT("/") + Stop->StopLineId.ToString();
        if (StopIds.Contains(Id)) Errors.Add(TEXT("Duplicate stop line: ") + Id);
        StopIds.Add(Id);
        auto* C = FindController(Stop->IntersectionId, Stop->SignalGroupId);
        if (!C || C->IsPedestrianGroup(Stop->SignalGroupId)) Errors.Add(TEXT("Stop line has missing/ambiguous/non-vehicle group: ") + Id);
        auto* Lane = Network ? Network->FindLane(Stop->LaneId) : EditorLanes.FindRef(Stop->LaneId);
        if (!IsValid(Lane) || Stop->DistanceAlongLane > Lane->GetSplineLength())
            Errors.Add(TEXT("Stop line has missing lane or lies beyond lane end: ") + Id);
    }
    for (UTMOPPedestrianCrossingComponent* Crossing : Crossings)
    {
        if (!IsValid(Crossing)) continue;
        if (Crossing->CrossingId.IsNone() || CrossingIds.Contains(Crossing->CrossingId)) Errors.Add(TEXT("Missing/duplicate CrossingId."));
        CrossingIds.Add(Crossing->CrossingId);
        auto* C = FindController(Crossing->IntersectionId, Crossing->SignalGroupId);
        if (!C || !C->IsPedestrianGroup(Crossing->SignalGroupId)) { Errors.Add(TEXT("Crossing lacks pedestrian group.")); continue; }
        for (FName G : Crossing->YieldingVehicleGroups)
            if (!C->HasGroup(G) || C->IsPedestrianGroup(G)) Errors.Add(TEXT("Invalid yielding vehicle group."));
        const double Required = 2.0 * Crossing->GetScaledBoxExtent().X / FMath::Max(1.0f, Crossing->ClearanceWalkingSpeedCmPerSecond);
        // After each walk-green ends, every declared yielding vehicle group needs clearance.
        for (int32 I = 0; I < C->Phases.Num(); ++I)
        {
            auto State = [C](int32 Phase, FName Id)
            {
                for (const auto& S : C->Phases[Phase].GroupStates) if (S.SignalGroupId == Id) return S.State;
                return ETMOPTrafficSignalState::Red;
            };
            if (State(I, Crossing->SignalGroupId) != ETMOPTrafficSignalState::Green ||
                State((I+1)%C->Phases.Num(), Crossing->SignalGroupId) == ETMOPTrafficSignalState::Green) continue;
            for (FName G : Crossing->YieldingVehicleGroups)
            {
                double Available = 0;
                for (int32 Step = 1; Step <= C->Phases.Num(); ++Step)
                {
                    const int32 J = (I + Step) % C->Phases.Num();
                    if (State(J,G) != ETMOPTrafficSignalState::Red) break;
                    Available += C->Phases[J].DurationSeconds;
                }
                if (Available < Required) Errors.Add(TEXT("Insufficient pedestrian clearance: ") + Crossing->CrossingId.ToString());
            }
        }
    }
    TSet<FString> UsedSlots;
    if (GetWorld()) for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        TArray<UTMOPTrafficSignalComponent*> Heads; It->GetComponents(Heads);
        for (auto* Head : Heads)
        {
            auto* C = FindController(Head->IntersectionId, Head->SignalGroupId);
            if (!C || C->IsPedestrianGroup(Head->SignalGroupId) != Head->bPedestrian) Errors.Add(TEXT("Signal head has invalid group/type: ") + Head->GetPathName());
            TArray<FString> Local; Head->ValidateMaterials(Local); Errors.Append(Local);
            if (auto* Mesh = Head->ResolveMesh())
                for (int32 Lamp = 0; Lamp < 3; ++Lamp)
                {
                    if (Head->bPedestrian && Lamp == 1) continue;
                    const FString Slot = Mesh->GetPathName() + TEXT("/") + Head->GetLampSlot(Lamp).ToString();
                    if (UsedSlots.Contains(Slot)) Errors.Add(TEXT("Two heads control the same mesh slot: ") + Slot);
                    UsedSlots.Add(Slot);
                }
        }
    }
    return Errors.IsEmpty();
}
void ATMOPTrafficSignalDirector::ValidateAndLog()
{
    DiscoverSignalSystem();
    TArray<FString> Errors;
    ValidateSignalSystem(Errors);
    for (const auto& Error : Errors) UE_LOG(LogTemp, Error, TEXT("TMOP signals: %s"), *Error);
}
FString ATMOPTrafficSignalDirector::ConfigurationSignature(UWorld* World)
{
    TArray<FString> Entries;
    if (!World) return TEXT("SIGNALS2");
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (const auto* D = Cast<ATMOPTrafficSignalDirector>(*It))
            Entries.Add(FString::Printf(TEXT("DIRECTOR:%d:%.6f:%d:%d"), D->bStopOnYellow, D->UpdateIntervalSeconds, D->bControlVehicles, D->bControlPedestrians));
        if (const auto* C = Cast<ATMOPTrafficSignalController>(*It)) Entries.Add(C->ConfigurationSignature());
        TArray<UTMOPTrafficStopLineComponent*> Stops; It->GetComponents(Stops);
        for (const auto* S : Stops)
            Entries.Add(FString::Printf(TEXT("STOP:%s:%s:%s:%s:%.4f:%.4f:%d"), *S->IntersectionId.ToString(),
                *S->StopLineId.ToString(), *S->LaneId.ToString(), *S->SignalGroupId.ToString(),
                S->DistanceAlongLane, S->StopBufferCm, S->bAccountForVehicleLength));
        TArray<UTMOPPedestrianCrossingComponent*> Walks; It->GetComponents(Walks);
        for (const auto* C : Walks)
        {
            FString Entry = TEXT("WALK:") + C->CrossingId.ToString() + C->IntersectionId.ToString() + C->SignalGroupId.ToString() +
                C->GetComponentTransform().ToString() + C->GetUnscaledBoxExtent().ToString() +
                FString::SanitizeFloat(C->ClearanceWalkingSpeedCmPerSecond);
            for (FName G : C->YieldingVehicleGroups) Entry += TEXT("|") + G.ToString();
            Entries.Add(Entry);
        }
        TArray<UTMOPTrafficSignalComponent*> Heads; It->GetComponents(Heads);
        for (auto* H : Heads)
            Entries.Add(TEXT("HEAD:") + H->IntersectionId.ToString() + TEXT("/") + H->SignalGroupId.ToString() +
                TEXT("/") + H->GetLampSlot(0).ToString() + TEXT("/") + H->GetLampSlot(1).ToString() +
                TEXT("/") + H->GetLampSlot(2).ToString());
    }
    Entries.Sort();
    return TEXT("SIGNALS2|") + FString::Join(Entries, TEXT("|"));
}
