#pragma once
#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

struct FTMOPFlightText
{
    TSharedPtr<FJsonObject> Values;
    FText Resolve(const FString& Id, const FString& Field) const;
};
struct FTMOPFlightCountry { FString Id; FTMOPFlightText Name; };
struct FTMOPFlightAirline
{
    FString Id, Name, Status;
    TArray<FString> Countries, Sources;
};
struct FTMOPFlightAirport
{
    FString Id, Name, Country, Timezone;
    double Latitude = 0, Longitude = 0;
    FVector Unit;
};
struct FTMOPFlightSource
{
    FString Id, Title, URL, Access, ValidFrom, ValidTo;
    FTMOPFlightText Notes;
};
struct FTMOPFlightLeg
{
    FString Id, ScheduleId, Flight, DepartureUTC, ArrivalUTC, DepartureLocal, ArrivalLocal, Page, Status;
    int32 Airline = INDEX_NONE, Origin = INDEX_NONE, Destination = INDEX_NONE, Source = INDEX_NONE;
    double Departure = 0, Arrival = 0;
    FTMOPFlightText Notes;
    bool Airborne(double Seconds) const { return Seconds >= Departure && Seconds < Arrival; }
    double Progress(double Seconds) const { return Arrival > Departure ? FMath::Clamp((Seconds - Departure) / (Arrival - Departure), 0.0, 1.0) : 0; }
};
struct FTMOPFlightData
{
    TArray<FTMOPFlightCountry> Countries;
    TArray<FTMOPFlightAirline> Airlines;
    TArray<FTMOPFlightAirport> Airports;
    TArray<FTMOPFlightSource> Sources;
    TArray<FTMOPFlightLeg> Legs;
    FTMOPFlightText Method;
    FDateTime StartUTC;
    double Duration = 172800, Anchor = 86400;
    FString Error;
    bool Load();
    bool Parse(const FString& Json);
};

/** Independent research clock per local player's atlas. Never touches game time. */
struct FTMOPFlightState
{
    TSharedPtr<FTMOPFlightData> Data;
    bool bEnabled = false, bPlaying = false, bAllRoutes = true, bOnlyAirborne = false;
    double Seconds = 0, Speed = 1800;
    FString Country, Airline, Search;
    int32 Selected = INDEX_NONE;
    uint32 FilterRevision = 0, SelectionRevision = 0, ListRevision = 0;
    TArray<int32> Filtered, Active, Routes;
    void Refilter();
    void RefreshActive();
    void SetTime(double Value);
    void Advance(double Delta);
    void Select(int32 Index);
    FText Clock() const;
    FText Label(int32 Index) const;
    const TArray<int32>& Listed() const { return bOnlyAirborne ? Active : Filtered; }
};
