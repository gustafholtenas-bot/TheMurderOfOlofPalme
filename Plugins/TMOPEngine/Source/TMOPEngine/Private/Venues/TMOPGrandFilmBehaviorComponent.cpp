#include "Venues/TMOPGrandFilmBehaviorComponent.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Engine/GameInstance.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

UTMOPGrandFilmBehaviorComponent::UTMOPGrandFilmBehaviorComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

bool UTMOPGrandFilmBehaviorComponent::StandFromAssignedSeat()
{
    if (bHasStoodThisLoop) return false;
    ATMOPHistoricalAgent* Agent = Cast<ATMOPHistoricalAgent>(GetOwner());
    if (!IsValid(Agent) || Agent->InitialSeatAssignment.SeatId.IsNone()) return false;
    UGameInstance* GameInstance = GetWorld() != nullptr ? GetWorld()->GetGameInstance() : nullptr;
    UTMOPCinemaSeatSubsystem* Seats = GameInstance != nullptr
        ? GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>() : nullptr;
    UTMOPCinemaSeatComponent* Seat = Seats != nullptr
        ? Seats->FindSeat(Agent->InitialSeatAssignment.SeatId) : nullptr;
    if (!IsValid(Seat) || Seat->GetOccupyingAgent() != Agent) return false;
    bHasStoodThisLoop = Seat->StandAgent(Agent);
    return bHasStoodThisLoop;
}

void UTMOPGrandFilmBehaviorComponent::ResetForLoop()
{
    bHasStoodThisLoop = false;
}
