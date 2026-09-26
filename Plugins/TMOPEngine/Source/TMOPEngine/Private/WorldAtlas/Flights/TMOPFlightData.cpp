#include "WorldAtlas/Flights/TMOPFlightData.h"
#include "WorldAtlas/TMOPGlobeMath.h"
#include "Localization/TMOPLocalization.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FText FTMOPFlightText::Resolve(const FString& Id, const FString& Field) const
{
    if (!Values) return FText::GetEmpty();
    FString Swedish, Translation, Source;
    Values->TryGetStringField(TEXT("sv"), Swedish);
    const FText Override = FTMOPLocalization::TableText(TEXT("TMOP_Flights"), Id, Field, Swedish);
    if (Override.ToString() != Swedish) return Override;
    const FString Language = FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName();
    if (Language != TEXT("sv") && Values->TryGetStringField(Language, Translation) &&
        Values->TryGetStringField(Language + TEXT("_source"), Source) && Source == Swedish && !Translation.IsEmpty())
        return FText::FromString(Translation);
    return FText::FromString(Swedish);
}
bool FTMOPFlightData::Load()
{
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("TMOPEngine"));
    FString Json;
    if (!Plugin || !FFileHelper::LoadFileToString(Json, *(Plugin->GetContentDir() / TEXT("WorldAtlas/Flights/flights.json"))))
    { *this = FTMOPFlightData(); Error = TEXT("Cannot read WorldAtlas/Flights/flights.json"); return false; }
    return Parse(Json);
}
bool FTMOPFlightData::Parse(const FString& Json)
{
    *this = FTMOPFlightData();
    auto Fail = [this](const FString& Why) { *this = FTMOPFlightData(); Error = Why; return false; };
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root) return Fail(TEXT("Invalid flight JSON"));
    auto Str = [](const TSharedPtr<FJsonObject>& O, const TCHAR* Key, FString& Out) { return O->TryGetStringField(Key, Out); };
    auto Number = [](const TSharedPtr<FJsonObject>& O, const TCHAR* Key, double& Out)
    { const auto V = O->TryGetField(Key); return V && V->Type == EJson::Number && V->TryGetNumber(Out) && FMath::IsFinite(Out); };
    auto Text = [](const TSharedPtr<FJsonObject>& O, const TCHAR* Key, FTMOPFlightText& Out)
    {
        const TSharedPtr<FJsonObject>* T = nullptr; FString Swedish;
        if (!O->TryGetObjectField(Key, T) || !T->IsValid() || !(*T)->TryGetStringField(TEXT("sv"), Swedish)) return false;
        Out.Values = *T; return true;
    };
    auto Rows = [](const TSharedPtr<FJsonObject>& O, const TCHAR* Key, TArray<TSharedPtr<FJsonObject>>& Out, int32 Limit)
    {
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (!O->TryGetArrayField(Key, Values) || Values->Num() > Limit) return false;
        for (const auto& V : *Values)
        {
            const TSharedPtr<FJsonObject>* Obj = nullptr;
            if (!V || !V->TryGetObject(Obj) || !Obj->IsValid()) return false;
            Out.Add(*Obj);
        }
        return true;
    };
    double Schema = 0; FString Start, End, Murder; FDateTime EndTime, AnchorTime;
    if (!Number(Root, TEXT("schema"), Schema) || Schema != 1 ||
        !Number(Root, TEXT("duration_seconds"), Duration) || Duration != 172800 ||
        !Number(Root, TEXT("anchor_seconds"), Anchor) || Anchor != 86400 ||
        !Str(Root, TEXT("start_utc"), Start) || !Start.EndsWith(TEXT("Z")) || !FDateTime::ParseIso8601(*Start, StartUTC) ||
        !Str(Root, TEXT("end_utc"), End) || !End.EndsWith(TEXT("Z")) || !FDateTime::ParseIso8601(*End, EndTime) ||
        !Str(Root, TEXT("anchor_utc"), Murder) || !Murder.EndsWith(TEXT("Z")) || !FDateTime::ParseIso8601(*Murder, AnchorTime) ||
        (EndTime - StartUTC).GetTotalSeconds() != Duration || (AnchorTime - StartUTC).GetTotalSeconds() != Anchor ||
        !Text(Root, TEXT("method"), Method)) return Fail(TEXT("Invalid flight window"));
    TMap<FString, int32> CountryIds, AirlineIds, AirportIds, SourceIds;
    auto Id = [&Str](const TSharedPtr<FJsonObject>& O, FString& Value, TMap<FString, int32>& Ids, int32 Index)
    { if (!Str(O, TEXT("id"), Value) || Value.IsEmpty() || Ids.Contains(Value)) return false; Ids.Add(Value, Index); return true; };
    auto Refs = [](const TSharedPtr<FJsonObject>& O, const TCHAR* Key, TArray<FString>& Out, const TMap<FString,int32>& Known)
    {
        const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;
        if (!O->TryGetArrayField(Key, Values) || Values->Num() > 4096) return false;
        for (const auto& V : *Values) { FString Ref; if (!V || !V->TryGetString(Ref) || !Known.Contains(Ref)) return false; Out.Add(Ref); }
        return true;
    };
    TArray<TSharedPtr<FJsonObject>> CountryRows, AirlineRows, AirportRows, SourceRows, LegRows;
    if (!Rows(Root, TEXT("countries"), CountryRows, 512) || !Rows(Root, TEXT("airlines"), AirlineRows, 4096) ||
        !Rows(Root, TEXT("airports"), AirportRows, 30000) || !Rows(Root, TEXT("sources"), SourceRows, 10000) ||
        !Rows(Root, TEXT("legs"), LegRows, 200000)) return Fail(TEXT("Flight array is invalid or too large"));
    for (const auto& O : CountryRows)
    {
        FTMOPFlightCountry C;
        if (!Id(O, C.Id, CountryIds, Countries.Num()) || !Text(O, TEXT("name"), C.Name)) return Fail(TEXT("Invalid flight country"));
        Countries.Add(MoveTemp(C));
    }
    for (const auto& O : SourceRows)
    {
        FTMOPFlightSource S;
        if (!Id(O, S.Id, SourceIds, Sources.Num()) || !Str(O, TEXT("title"), S.Title) ||
            !Str(O, TEXT("url"), S.URL) || !S.URL.StartsWith(TEXT("https://")) || !Str(O, TEXT("access"), S.Access) ||
            !Text(O, TEXT("notes"), S.Notes)) return Fail(TEXT("Invalid flight source"));
        Str(O, TEXT("valid_from"), S.ValidFrom); Str(O, TEXT("valid_to"), S.ValidTo);
        Sources.Add(MoveTemp(S));
    }
    for (const auto& O : AirlineRows)
    {
        FTMOPFlightAirline A;
        if (!Id(O, A.Id, AirlineIds, Airlines.Num()) || !Str(O, TEXT("name"), A.Name) ||
            !Str(O, TEXT("research_status"), A.Status) || !Refs(O, TEXT("countries"), A.Countries, CountryIds) || A.Countries.IsEmpty() ||
            !Refs(O, TEXT("source_ids"), A.Sources, SourceIds)) return Fail(TEXT("Invalid flight airline"));
        Airlines.Add(MoveTemp(A));
    }
    for (const auto& O : AirportRows)
    {
        FTMOPFlightAirport A;
        if (!Id(O, A.Id, AirportIds, Airports.Num()) || !Str(O, TEXT("name"), A.Name) || !Str(O, TEXT("country"), A.Country) ||
            !CountryIds.Contains(A.Country) || !Str(O, TEXT("timezone"), A.Timezone) ||
            !Number(O, TEXT("lat"), A.Latitude) || FMath::Abs(A.Latitude) > 90 || !Number(O, TEXT("lon"), A.Longitude) || FMath::Abs(A.Longitude) > 180)
            return Fail(TEXT("Invalid flight airport"));
        A.Unit = TMOPGlobe::Unit(A.Latitude, A.Longitude); Airports.Add(MoveTemp(A));
    }
    TSet<FString> LegIds;
    for (const auto& O : LegRows)
    {
        FTMOPFlightLeg F; FString Airline, Origin, Destination, Source; FDateTime Dep, Arr;
        if (!Str(O, TEXT("id"), F.Id) || F.Id.IsEmpty() || LegIds.Contains(F.Id) ||
            !Str(O, TEXT("schedule"), F.ScheduleId) || F.ScheduleId.IsEmpty() ||
            !Str(O, TEXT("airline"), Airline) || !AirlineIds.Contains(Airline) ||
            !Str(O, TEXT("origin"), Origin) || !AirportIds.Contains(Origin) || !Str(O, TEXT("destination"), Destination) || !AirportIds.Contains(Destination) || Origin == Destination ||
            !Str(O, TEXT("source"), Source) || !SourceIds.Contains(Source) || !Str(O, TEXT("flight"), F.Flight) ||
            !Str(O, TEXT("status"), F.Status) || (F.Status != TEXT("scheduled") && F.Status != TEXT("confirmed")) ||
            !Str(O, TEXT("page"), F.Page) || F.Page.IsEmpty() || !Text(O, TEXT("notes"), F.Notes) ||
            !Str(O, TEXT("departure_utc"), F.DepartureUTC) || !F.DepartureUTC.EndsWith(TEXT("Z")) || !FDateTime::ParseIso8601(*F.DepartureUTC, Dep) ||
            !Str(O, TEXT("arrival_utc"), F.ArrivalUTC) || !F.ArrivalUTC.EndsWith(TEXT("Z")) || !FDateTime::ParseIso8601(*F.ArrivalUTC, Arr) ||
            !Str(O, TEXT("departure_local"), F.DepartureLocal) || !Str(O, TEXT("arrival_local"), F.ArrivalLocal) ||
            !Number(O, TEXT("departure_seconds"), F.Departure) || !Number(O, TEXT("arrival_seconds"), F.Arrival) ||
            (Dep - StartUTC).GetTotalSeconds() != F.Departure || (Arr - StartUTC).GetTotalSeconds() != F.Arrival ||
            F.Arrival <= F.Departure || F.Arrival - F.Departure > 259200 || F.Departure >= Duration || F.Arrival <= 0)
            return Fail(TEXT("Invalid flight movement: ") + F.Id);
        F.Airline = AirlineIds[Airline]; F.Origin = AirportIds[Origin]; F.Destination = AirportIds[Destination]; F.Source = SourceIds[Source];
        LegIds.Add(F.Id); Legs.Add(MoveTemp(F));
    }
    Legs.Sort([](const FTMOPFlightLeg& A, const FTMOPFlightLeg& B) { return A.Departure == B.Departure ? A.Id < B.Id : A.Departure < B.Departure; });
    return true;
}

void FTMOPFlightState::Refilter()
{
    Filtered.Reset(); Routes.Reset(); TSet<FString> RouteKeys;
    if (Data) for (int32 I = 0; I < Data->Legs.Num(); ++I)
    {
        const auto& F = Data->Legs[I]; const auto& A = Data->Airlines[F.Airline];
        if (!Country.IsEmpty() && !A.Countries.Contains(Country) && Data->Airports[F.Origin].Country != Country && Data->Airports[F.Destination].Country != Country) continue;
        if (!Airline.IsEmpty() && A.Id != Airline) continue;
        const FString Words = Label(I).ToString() + TEXT(" ") + Data->Airports[F.Origin].Name + TEXT(" ") + Data->Airports[F.Destination].Name;
        if (!Search.IsEmpty() && !Words.Contains(Search, ESearchCase::IgnoreCase)) continue;
        Filtered.Add(I);
        const FString Key = FString::FromInt(F.Origin) + TEXT("/") + FString::FromInt(F.Destination);
        if (!RouteKeys.Contains(Key)) { RouteKeys.Add(Key); Routes.Add(I); }
    }
    if (!Filtered.Contains(Selected)) { Selected = INDEX_NONE; ++SelectionRevision; }
    ++FilterRevision; ++ListRevision; RefreshActive();
}
void FTMOPFlightState::RefreshActive()
{
    TArray<int32> Next;
    if (Data) for (int32 I : Filtered)
    {
        const auto& F = Data->Legs[I];
        if (F.Departure > Seconds) break; // Dataset and filtered indices preserve departure order.
        if (F.Airborne(Seconds)) Next.Add(I);
    }
    if (Next != Active) { Active = MoveTemp(Next); if (bOnlyAirborne) ++ListRevision; }
}
void FTMOPFlightState::SetTime(double Value)
{
    if (!Data || !FMath::IsFinite(Value)) return;
    Seconds = FMath::Clamp(Value, 0.0, Data->Duration); RefreshActive();
}
void FTMOPFlightState::Advance(double Delta)
{
    if (!Data || !bEnabled || !bPlaying || !FMath::IsFinite(Delta) || Delta < 0) return;
    SetTime(Seconds + Delta * Speed);
    if (Seconds >= Data->Duration) bPlaying = false;
}
void FTMOPFlightState::Select(int32 Index)
{
    if (!Data || !Data->Legs.IsValidIndex(Index)) return;
    Selected = Index; ++SelectionRevision;
}
FText FTMOPFlightState::Clock() const
{
    if (!Data) return FText::GetEmpty();
    const FDateTime UTC = Data->StartUTC + FTimespan::FromSeconds(Seconds);
    // This fixed 1986 February window is CET throughout. Never use the host's timezone.
    return FText::FromString((UTC + FTimespan::FromHours(1)).ToString(TEXT("%Y-%m-%d %H:%M:%S")) + TEXT(" CET / ") + UTC.ToString(TEXT("%H:%M:%S")) + TEXT(" UTC"));
}
FText FTMOPFlightState::Label(int32 Index) const
{
    if (!Data || !Data->Legs.IsValidIndex(Index)) return FText::GetEmpty();
    const auto& F = Data->Legs[Index];
    return FText::FromString(Data->Airlines[F.Airline].Name + TEXT(" ") + F.Flight + TEXT(" • ") + Data->Airports[F.Origin].Id + TEXT(" → ") + Data->Airports[F.Destination].Id);
}
