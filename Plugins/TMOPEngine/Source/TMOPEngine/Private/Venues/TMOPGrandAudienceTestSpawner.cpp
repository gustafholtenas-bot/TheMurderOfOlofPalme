#include "Venues/TMOPGrandAudienceTestSpawner.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Engine/World.h"
#include "Venues/TMOPCinemaSeatComponent.h"
#include "Venues/TMOPCinemaSeatSubsystem.h"

ATMOPGrandAudienceTestSpawner::ATMOPGrandAudienceTestSpawner()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ATMOPGrandAudienceTestSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bSpawnAutomatically)
    {
        SpawnTestAudience();
    }
}

int32 ATMOPGrandAudienceTestSpawner::SpawnTestAudience()
{
    if (AgentClass == nullptr || GetWorld() == nullptr)
    {
        return 0;
    }

    UGameInstance* GameInstance = GetGameInstance();
    UTMOPCinemaSeatSubsystem* SeatSubsystem =
        GameInstance != nullptr
            ? GameInstance->GetSubsystem<UTMOPCinemaSeatSubsystem>()
            : nullptr;

    if (SeatSubsystem == nullptr)
    {
        return 0;
    }

    SeatSubsystem->DiscoverSeatsInWorld();

    const int32 Count = FMath::Min(
        FMath::Min(MaximumAgents, 10),
        SeatIds.Num());

    int32 SpawnedCount = 0;

    for (int32 Index = 0; Index < Count; ++Index)
    {
        UTMOPCinemaSeatComponent* Seat =
            SeatSubsystem->FindSeat(SeatIds[Index]);

        if (!IsValid(Seat) || Seat->IsOccupied())
        {
            continue;
        }

        const FTransform SpawnTransform =
            Seat->GetApproachWorldTransform();

        ATMOPHistoricalAgent* Agent =
            GetWorld()->SpawnActor<ATMOPHistoricalAgent>(
                AgentClass,
                SpawnTransform);

        if (!IsValid(Agent))
        {
            continue;
        }

        Agent->InitialSeatAssignment.VenueId = VenueId;
        Agent->InitialSeatAssignment.AuditoriumId = AuditoriumId;
        Agent->InitialSeatAssignment.SeatId = SeatIds[Index];
        Agent->InitialSeatAssignment.RowNumber = Seat->RowNumber;
        Agent->InitialSeatAssignment.SeatNumber = Seat->SeatNumber;
        Agent->InitialSeatAssignment.bStartsSeated = true;

        if (Seat->SeatAgent(Agent))
        {
            ++SpawnedCount;
        }
        else
        {
            Agent->Destroy();
        }
    }

    UE_LOG(
        LogTemp,
        Log,
        TEXT("TMOP Grand test audience spawned: %d"),
        SpawnedCount);

    return SpawnedCount;
}
