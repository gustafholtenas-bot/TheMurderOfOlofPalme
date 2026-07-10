#include "Venues/TMOPCinemaSeatSubsystem.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Venues/TMOPCinemaSeatComponent.h"

void UTMOPCinemaSeatSubsystem::Initialize(
    FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Seats.Reset();
}

void UTMOPCinemaSeatSubsystem::Deinitialize()
{
    Seats.Reset();
    Super::Deinitialize();
}

bool UTMOPCinemaSeatSubsystem::RegisterSeat(
    UTMOPCinemaSeatComponent* Seat)
{
    if (!IsValid(Seat) || Seat->SeatId.IsNone())
    {
        return false;
    }

    if (const TWeakObjectPtr<UTMOPCinemaSeatComponent>* Existing =
        Seats.Find(Seat->SeatId))
    {
        if (Existing->IsValid() && Existing->Get() != Seat)
        {
            UE_LOG(
                LogTemp,
                Warning,
                TEXT("TMOP rejected duplicate cinema seat ID '%s'."),
                *Seat->SeatId.ToString());
            return false;
        }
    }

    Seats.Add(Seat->SeatId, Seat);
    return true;
}

bool UTMOPCinemaSeatSubsystem::UnregisterSeat(
    const FName SeatId)
{
    return Seats.Remove(SeatId) > 0;
}

UTMOPCinemaSeatComponent* UTMOPCinemaSeatSubsystem::FindSeat(
    const FName SeatId) const
{
    if (const TWeakObjectPtr<UTMOPCinemaSeatComponent>* Found =
        Seats.Find(SeatId))
    {
        return Found->Get();
    }

    return nullptr;
}

int32 UTMOPCinemaSeatSubsystem::DiscoverSeatsInWorld()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return 0;
    }

    int32 RegisteredCount = 0;

    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPCinemaSeatComponent*> ActorSeats;
        It->GetComponents<UTMOPCinemaSeatComponent>(ActorSeats);

        for (UTMOPCinemaSeatComponent* Seat : ActorSeats)
        {
            RegisteredCount += RegisterSeat(Seat) ? 1 : 0;
        }
    }

    return RegisteredCount;
}
