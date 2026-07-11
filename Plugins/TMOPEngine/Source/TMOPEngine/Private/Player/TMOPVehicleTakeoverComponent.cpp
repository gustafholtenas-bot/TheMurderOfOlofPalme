#include "Player/TMOPVehicleTakeoverComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Animation/TMOPAnimationStateComponent.h"
#include "EngineUtils.h"
#include "Entities/TMOPWorldEntityComponent.h"
#include "GameFramework/Character.h"
#include "People/TMOPVehicleDispositionComponent.h"
#include "Vehicles/TMOPVehicleBase.h"
#include "Vehicles/TMOPVehicleDoorComponent.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"

UTMOPVehicleTakeoverComponent::UTMOPVehicleTakeoverComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

ATMOPVehicleBase* UTMOPVehicleTakeoverComponent::FindNearestVehicle() const
{
    const ACharacter* Player = Cast<ACharacter>(GetOwner());
    UWorld* World = GetWorld();
    if (!IsValid(Player) || World == nullptr) return nullptr;
    ATMOPVehicleBase* Best = nullptr;
    float BestDistanceSquared = FMath::Square(InteractionRangeCm);
    for (TActorIterator<ATMOPVehicleBase> It(World); It; ++It)
    {
        ATMOPVehicleBase* Vehicle = *It;
        if (!IsValid(Vehicle)) continue;
        const float DistanceSquared = FVector::DistSquared(
            Player->GetActorLocation(), Vehicle->GetActorLocation());
        if (DistanceSquared <= BestDistanceSquared)
        {
            BestDistanceSquared = DistanceSquared;
            Best = Vehicle;
        }
    }
    return Best;
}

ETMOPVehicleTakeoverResult UTMOPVehicleTakeoverComponent::TryEnterNearestVehicle(
    const bool bPreferDriverSeat)
{
    return TryEnterVehicle(FindNearestVehicle(), bPreferDriverSeat);
}

ETMOPVehicleTakeoverResult UTMOPVehicleTakeoverComponent::TryEnterVehicle(
    ATMOPVehicleBase* Vehicle, const bool bPreferDriverSeat)
{
    ACharacter* Player = Cast<ACharacter>(GetOwner());
    if (!IsValid(Player) || !IsValid(Vehicle) || IsInsideVehicle())
        return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedNoVehicle, Vehicle, nullptr);
    UTMOPVehicleSeatComponent* Seat = SelectSeat(Vehicle, bPreferDriverSeat);
    if (!IsValid(Seat))
        return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedNoSeat, Vehicle, nullptr);
    ACharacter* PreviousOccupant = Seat->GetOccupantCharacter();
    if (IsValid(PreviousOccupant))
    {
        if (Seat->SeatRole != ETMOPVehicleSeatRole::Driver)
            return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedOccupiedPassengerSeat,
                Vehicle, PreviousOccupant);
        UTMOPVehicleDispositionComponent* Disposition =
            PreviousOccupant->FindComponentByClass<UTMOPVehicleDispositionComponent>();
        if (IsValid(Disposition) && !Disposition->bCanBePulledFromVehicle)
            return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedDriverCannotBeRemoved,
                Vehicle, PreviousOccupant);
        bool bAttack = false;
        bool bFlee = false;
        if (DriverResists(PreviousOccupant, bAttack, bFlee))
        {
            if (bAttack)
                if (UTMOPAnimationStateComponent* Animation =
                    PreviousOccupant->FindComponentByClass<UTMOPAnimationStateComponent>())
                    Animation->TriggerReaction(ETMOPAnimReaction::Punch, 0.75f);
            return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedDriverResisted,
                Vehicle, PreviousOccupant);
        }
        if (!Seat->ExitCharacterSeat(PreviousOccupant))
            return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedInternal,
                Vehicle, PreviousOccupant);
        ApplyDriverRemovalReaction(PreviousOccupant, true);
    }
    if (UTMOPVehicleDoorComponent* Door = FindDoorForSeat(Vehicle, Seat->SeatId))
    {
        const FTransform Approach = Door->GetApproachWorldTransform();
        Player->SetActorLocationAndRotation(Approach.GetLocation(), Approach.Rotator(),
            false, nullptr, ETeleportType::TeleportPhysics);
    }
    if (!Seat->EnterCharacterSeat(Player))
        return ResolveAndBroadcast(ETMOPVehicleTakeoverResult::FailedInternal,
            Vehicle, PreviousOccupant);
    CurrentVehicle = Vehicle;
    CurrentSeat = Seat;
    return ResolveAndBroadcast(IsValid(PreviousOccupant)
        ? ETMOPVehicleTakeoverResult::SuccessDriverRemoved
        : ETMOPVehicleTakeoverResult::SuccessEmptySeat, Vehicle, PreviousOccupant);
}

UTMOPVehicleSeatComponent* UTMOPVehicleTakeoverComponent::SelectSeat(
    ATMOPVehicleBase* Vehicle, const bool bPreferDriverSeat) const
{
    if (!IsValid(Vehicle)) return nullptr;
    TArray<UTMOPVehicleSeatComponent*> Seats;
    Vehicle->GetComponents<UTMOPVehicleSeatComponent>(Seats);
    const ETMOPVehicleSeatRole Preferred = bPreferDriverSeat
        ? ETMOPVehicleSeatRole::Driver : ETMOPVehicleSeatRole::FrontPassenger;
    for (UTMOPVehicleSeatComponent* Seat : Seats)
        if (IsValid(Seat) && Seat->SeatRole == Preferred &&
            (!Seat->IsOccupied() || Preferred == ETMOPVehicleSeatRole::Driver)) return Seat;
    if (!bPreferDriverSeat)
        for (UTMOPVehicleSeatComponent* Seat : Seats)
            if (IsValid(Seat) && Seat->SeatRole != ETMOPVehicleSeatRole::Driver && !Seat->IsOccupied()) return Seat;
    return nullptr;
}

UTMOPVehicleDoorComponent* UTMOPVehicleTakeoverComponent::FindDoorForSeat(
    ATMOPVehicleBase* Vehicle, const FName SeatId) const
{
    TArray<UTMOPVehicleDoorComponent*> Doors;
    Vehicle->GetComponents<UTMOPVehicleDoorComponent>(Doors);
    for (UTMOPVehicleDoorComponent* Door : Doors)
        if (IsValid(Door) && Door->bEnabled && Door->SeatId == SeatId) return Door;
    return nullptr;
}

bool UTMOPVehicleTakeoverComponent::DriverResists(ACharacter* Driver,
    bool& bWillAttack, bool& bWillFlee)
{
    bWillAttack = false;
    bWillFlee = false;
    UTMOPVehicleDispositionComponent* Disposition = IsValid(Driver)
        ? Driver->FindComponentByClass<UTMOPVehicleDispositionComponent>() : nullptr;
    if (!IsValid(Disposition) || Disposition->Disposition == ETMOPVehicleDisposition::Cooperative ||
        Disposition->Disposition == ETMOPVehicleDisposition::Passive ||
        Disposition->Disposition == ETMOPVehicleDisposition::Incapacitated) return false;
    FName EntityId = NAME_None;
    if (ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(Driver))
        if (Agent->EntityIdentity != nullptr) EntityId = Agent->EntityIdentity->EntityId;
    FRandomStream Random(TakeoverSeed + GetTypeHash(EntityId) + AttemptCounter++ * 101);
    const bool bResists = Random.FRand() < FMath::Clamp(Disposition->ResistanceChance, 0.0f, 1.0f);
    if (bResists)
    {
        bWillAttack = Random.FRand() < FMath::Clamp(Disposition->AttackChanceOnResistance, 0.0f, 1.0f);
        bWillFlee = !bWillAttack && Random.FRand() <
            FMath::Clamp(Disposition->FleeChanceOnResistance, 0.0f, 1.0f);
    }
    return bResists;
}

void UTMOPVehicleTakeoverComponent::ApplyDriverRemovalReaction(ACharacter* Driver,
    const bool bViolent)
{
    if (UTMOPAnimationStateComponent* Animation = IsValid(Driver)
        ? Driver->FindComponentByClass<UTMOPAnimationStateComponent>() : nullptr)
        Animation->TriggerReaction(bViolent ? ETMOPAnimReaction::FallingFromHit
            : ETMOPAnimReaction::SurprisedOrScared, 1.0f);
}

bool UTMOPVehicleTakeoverComponent::ExitCurrentVehicle()
{
    ACharacter* Player = Cast<ACharacter>(GetOwner());
    if (!IsValid(Player) || !IsValid(CurrentSeat) ||
        !CurrentSeat->ExitCharacterSeat(Player)) return false;
    CurrentSeat = nullptr;
    CurrentVehicle = nullptr;
    return true;
}

bool UTMOPVehicleTakeoverComponent::IsDriver() const
{
    return IsValid(CurrentSeat.Get()) &&
        CurrentSeat->SeatRole == ETMOPVehicleSeatRole::Driver;
}

bool UTMOPVehicleTakeoverComponent::IsInsideVehicle() const
{
    return IsValid(CurrentVehicle.Get()) && IsValid(CurrentSeat.Get());
}

ETMOPVehicleTakeoverResult UTMOPVehicleTakeoverComponent::ResolveAndBroadcast(
    const ETMOPVehicleTakeoverResult Result, ATMOPVehicleBase* Vehicle,
    ACharacter* PreviousOccupant)
{
    OnTakeoverResolved.Broadcast(Result, Vehicle, PreviousOccupant);
    return Result;
}
