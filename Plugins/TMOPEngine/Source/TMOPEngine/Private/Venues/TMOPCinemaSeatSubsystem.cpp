#include "Venues/TMOPCinemaSeatSubsystem.h"

#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Venues/TMOPCinemaSeatComponent.h"

void UTMOPCinemaSeatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Seats.Reset();
}

void UTMOPCinemaSeatSubsystem::Deinitialize()
{
    Seats.Reset();
    Super::Deinitialize();
}

bool UTMOPCinemaSeatSubsystem::RegisterSeat(UTMOPCinemaSeatComponent* Seat)
{
    if (!IsValid(Seat) || Seat->SeatId.IsNone())
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP rejected a cinema seat without a SeatId."));
        return false;
    }

    if (TWeakObjectPtr<UTMOPCinemaSeatComponent>* Existing = Seats.Find(Seat->SeatId))
    {
        if (Existing->IsValid())
        {
            if (Existing->Get() == Seat)
            {
                return true;
            }

            UE_LOG(LogTemp, Error,
                TEXT("TMOP duplicate cinema SeatId '%s': '%s' conflicts with '%s'."),
                *Seat->SeatId.ToString(), *Seat->GetPathName(),
                *Existing->Get()->GetPathName());
            return false;
        }

        Seats.Remove(Seat->SeatId);
    }

    Seats.Add(Seat->SeatId, Seat);
    return true;
}

bool UTMOPCinemaSeatSubsystem::UnregisterSeat(UTMOPCinemaSeatComponent* Seat)
{
    if (Seat == nullptr || Seat->SeatId.IsNone())
    {
        return false;
    }

    TWeakObjectPtr<UTMOPCinemaSeatComponent>* Existing = Seats.Find(Seat->SeatId);
    if (Existing == nullptr || Existing->Get() != Seat)
    {
        return false;
    }

    Seats.Remove(Seat->SeatId);
    return true;
}

UTMOPCinemaSeatComponent* UTMOPCinemaSeatSubsystem::FindSeat(const FName SeatId) const
{
    const TWeakObjectPtr<UTMOPCinemaSeatComponent>* Found = Seats.Find(SeatId);
    return Found != nullptr ? Found->Get() : nullptr;
}

int32 UTMOPCinemaSeatSubsystem::DiscoverSeatsInWorld()
{
    UWorld* World = GetWorld();
    if (World == nullptr)
    {
        return 0;
    }

    RemoveInvalidSeats();
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        TArray<UTMOPCinemaSeatComponent*> ActorSeats;
        It->GetComponents<UTMOPCinemaSeatComponent>(ActorSeats);
        for (UTMOPCinemaSeatComponent* Seat : ActorSeats)
        {
            RegisterSeat(Seat);
        }
    }

    return GetSeatCount();
}

int32 UTMOPCinemaSeatSubsystem::RemoveInvalidSeats()
{
    int32 Removed = 0;
    for (auto It = Seats.CreateIterator(); It; ++It)
    {
        if (!It.Value().IsValid())
        {
            It.RemoveCurrent();
            ++Removed;
        }
    }
    return Removed;
}

bool UTMOPCinemaSeatSubsystem::ValidateSeatRegistry(TArray<FString>& OutErrors) const
{
    OutErrors.Reset();
    for (const TPair<FName, TWeakObjectPtr<UTMOPCinemaSeatComponent>>& Pair : Seats)
    {
        const UTMOPCinemaSeatComponent* Seat = Pair.Value.Get();
        if (!IsValid(Seat))
        {
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' has an invalid component."), *Pair.Key.ToString()));
            continue;
        }
        if (Seat->VenueId.IsNone())
        {
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' has no VenueId."), *Pair.Key.ToString()));
        }
        if (Seat->AuditoriumId.IsNone())
        {
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' has no AuditoriumId."), *Pair.Key.ToString()));
        }
        if (Seat->RowNumber < 0 || Seat->SeatNumber < 0)
        {
            OutErrors.Add(FString::Printf(TEXT("Seat '%s' needs valid row and seat numbers."), *Pair.Key.ToString()));
        }
    }
    return OutErrors.IsEmpty();
}

int32 UTMOPCinemaSeatSubsystem::GetSeatCount() const
{
    int32 Count = 0;
    for (const TPair<FName, TWeakObjectPtr<UTMOPCinemaSeatComponent>>& Pair : Seats)
    {
        Count += Pair.Value.IsValid() ? 1 : 0;
    }
    return Count;
}
