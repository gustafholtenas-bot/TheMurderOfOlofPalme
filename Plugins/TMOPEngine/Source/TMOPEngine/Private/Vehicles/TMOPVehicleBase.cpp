#include "Vehicles/TMOPVehicleBase.h"

#include "Agents/TMOPHistoricalAgent.h"
#include "Vehicles/TMOPVehicleSeatComponent.h"

ATMOPVehicleBase::ATMOPVehicleBase()
{
    PrimaryActorTick.bCanEverTick = false;

    VehicleRoot =
        CreateDefaultSubobject<USceneComponent>(TEXT("VehicleRoot"));
    RootComponent = VehicleRoot;
}

TArray<UTMOPVehicleSeatComponent*>
ATMOPVehicleBase::GetVehicleSeats() const
{
    TArray<UTMOPVehicleSeatComponent*> Seats;
    GetComponents<UTMOPVehicleSeatComponent>(Seats);
    return Seats;
}

UTMOPVehicleSeatComponent*
ATMOPVehicleBase::GetDriverSeat() const
{
    for (UTMOPVehicleSeatComponent* Seat : GetVehicleSeats())
    {
        if (IsValid(Seat) &&
            Seat->SeatRole == ETMOPVehicleSeatRole::Driver)
        {
            return Seat;
        }
    }

    return nullptr;
}

bool ATMOPVehicleBase::EnterVehicle(
    ATMOPHistoricalAgent* Agent,
    const FName PreferredSeatId)
{
    for (UTMOPVehicleSeatComponent* Seat : GetVehicleSeats())
    {
        if (!IsValid(Seat) || Seat->IsOccupied())
        {
            continue;
        }

        if (!PreferredSeatId.IsNone() &&
            Seat->SeatId != PreferredSeatId)
        {
            continue;
        }

        return Seat->EnterSeat(Agent);
    }

    return false;
}

bool ATMOPVehicleBase::EnterDriverSeat(
    ATMOPHistoricalAgent* Agent)
{
    UTMOPVehicleSeatComponent* DriverSeat = GetDriverSeat();
    return IsValid(DriverSeat) &&
        DriverSeat->EnterSeat(Agent);
}

bool ATMOPVehicleBase::ExitVehicle(
    ATMOPHistoricalAgent* Agent)
{
    for (UTMOPVehicleSeatComponent* Seat : GetVehicleSeats())
    {
        if (IsValid(Seat) && Seat->GetOccupant() == Agent)
        {
            return Seat->ExitSeat(Agent);
        }
    }

    return false;
}

ATMOPHistoricalAgent* ATMOPVehicleBase::GetDriverAgent() const
{
    UTMOPVehicleSeatComponent* DriverSeat = GetDriverSeat();
    return IsValid(DriverSeat)
        ? DriverSeat->GetOccupant()
        : nullptr;
}
