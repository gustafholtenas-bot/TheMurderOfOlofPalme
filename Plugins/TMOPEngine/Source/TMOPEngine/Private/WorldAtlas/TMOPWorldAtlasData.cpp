#include "WorldAtlas/TMOPWorldAtlasData.h"
#include "WorldAtlas/TMOPGlobeMath.h"
#include "Localization/TMOPLocalization.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FText FTMOPAtlasEntry::Text(const FString& Field) const
{
    const TSharedPtr<FJsonObject>* Value = Texts.Find(Field);
    if (!Value || !Value->IsValid()) return FText::GetEmpty();
    FString Swedish;
    (*Value)->TryGetStringField(TEXT("sv"), Swedish);
    const FText Override = FTMOPLocalization::TableText(TEXT("TMOP_WorldAtlas"), Id, Field, Swedish);
    if (Override.ToString() != Swedish) return Override;
    const FString Language = FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName();
    FString Translation, Source;
    if (Language != TEXT("sv") && (*Value)->TryGetStringField(Language, Translation) &&
        (*Value)->TryGetStringField(Language + TEXT("_source"), Source) && Source == Swedish && !Translation.IsEmpty())
        return FText::FromString(Translation);
    return FText::FromString(Swedish);
}

bool FTMOPAtlasEntry::Visible(bool bLater, bool bNearby) const
{
    if (bLaterOnly && !bLater) return false;
    if (Kind == TEXT("conflict"))
    {
        // Inclusive ±90 days; independent of the later-1986 flow/event switch.
        const FString Begin = bNearby ? TEXT("1985-11-30") : TEXT("1986-02-28");
        const FString End = bNearby ? TEXT("1986-05-29") : TEXT("1986-02-28");
        return (From.IsEmpty() || From <= End) && (To.IsEmpty() || To >= Begin);
    }
    // Flows/events can be historical context.
    const FString Cutoff = Kind == TEXT("conflict") || !bLater ? TEXT("1986-02-28") : TEXT("1986-12-31");
    if (!From.IsEmpty() && From > Cutoff) return false;
    return To.IsEmpty() || Kind == TEXT("arms") || Kind == TEXT("funds") || Kind == TEXT("event") || To >= Cutoff;
}

const FTMOPAtlasEntry* FTMOPWorldAtlasData::Find(const FString& Id) const
{
    return Entries.FindByPredicate([&Id](const FTMOPAtlasEntry& E) { return E.Id == Id; });
}

bool FTMOPWorldAtlasData::Load()
{
    Entries.Reset(); Coastlines.Reset(); Error.Reset();
    auto Fail = [this](const FString& Message) { Error = Message; Entries.Reset(); Coastlines.Reset(); return false; };
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("TMOPEngine"));
    if (!Plugin) return Fail(TEXT("TMOPEngine plugin not found"));
    const FString Directory = Plugin->GetContentDir() / TEXT("WorldAtlas");
    auto Read = [](const FString& Path, TSharedPtr<FJsonObject>& Root)
    {
        FString Json;
        return FFileHelper::LoadFileToString(Json, *Path) &&
            FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) && Root.IsValid();
    };
    TSharedPtr<FJsonObject> Root;
    if (!Read(Directory / TEXT("world.json"), Root)) return Fail(TEXT("Cannot read WorldAtlas/world.json"));
    double Version = 0;
    if (!Root->TryGetNumberField(TEXT("schema"), Version) || Version != 1) return Fail(TEXT("Unsupported atlas schema"));
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Root->TryGetArrayField(TEXT("entries"), Rows) || Rows->Num() > 2048) return Fail(TEXT("Invalid entries"));
    TSet<FString> Ids;
    const TSet<FString> Kinds = {TEXT("region"), TEXT("country"), TEXT("actor"), TEXT("group"), TEXT("conflict"), TEXT("arms"), TEXT("funds"), TEXT("event")};
    auto DateOK = [](const FString& Date)
    {
        if (Date.IsEmpty()) return true;
        if (Date.Len() != 10 || Date[4] != '-' || Date[7] != '-') return false;
        FDateTime Parsed;
        return FDateTime::ParseIso8601(*(Date + TEXT("T00:00:00Z")), Parsed);
    };
    for (const auto& Value : *Rows)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        if (!Value->TryGetObject(Obj)) return Fail(TEXT("Entry must be an object"));
        FTMOPAtlasEntry E;
        (*Obj)->TryGetStringField(TEXT("id"), E.Id);
        (*Obj)->TryGetStringField(TEXT("kind"), E.Kind);
        (*Obj)->TryGetStringField(TEXT("country"), E.Country);
        (*Obj)->TryGetStringField(TEXT("from"), E.From);
        (*Obj)->TryGetStringField(TEXT("to"), E.To);
        (*Obj)->TryGetBoolField(TEXT("marker"), E.bMarker);
        (*Obj)->TryGetBoolField(TEXT("later_only"), E.bLaterOnly);
        if (E.Id.IsEmpty() || Ids.Contains(E.Id) || !Kinds.Contains(E.Kind) || !DateOK(E.From) || !DateOK(E.To) ||
            (!E.From.IsEmpty() && !E.To.IsEmpty() && E.To < E.From)) return Fail(TEXT("Invalid ID, kind or dates: ") + E.Id);
        if (!(*Obj)->TryGetNumberField(TEXT("lat"), E.Latitude) || !(*Obj)->TryGetNumberField(TEXT("lon"), E.Longitude) ||
            !FMath::IsFinite(E.Latitude) || !FMath::IsFinite(E.Longitude) || FMath::Abs(E.Latitude) > 90 || FMath::Abs(E.Longitude) > 180)
            return Fail(TEXT("Invalid latitude/longitude: ") + E.Id);
        if ((*Obj)->HasField(TEXT("marker_offset")))
        {
            const TArray<TSharedPtr<FJsonValue>>* Offset = nullptr;
            double X = 0, Y = 0;
            if (!(*Obj)->TryGetArrayField(TEXT("marker_offset"), Offset) || Offset->Num() != 2 ||
                !(*Offset)[0]->TryGetNumber(X) || !(*Offset)[1]->TryGetNumber(Y) ||
                !FMath::IsFinite(X) || !FMath::IsFinite(Y) || FMath::Abs(X) > 80 || FMath::Abs(Y) > 80)
                return Fail(TEXT("Invalid marker offset: ") + E.Id);
            E.MarkerOffset = FVector2D(X, Y);
        }
        const TSharedPtr<FJsonObject>* Texts = nullptr;
        if (!(*Obj)->TryGetObjectField(TEXT("text"), Texts)) return Fail(TEXT("Missing text: ") + E.Id);
        for (const auto& Pair : (*Texts)->Values)
        {
            const TSharedPtr<FJsonObject>* Localized = nullptr;
            FString Swedish;
            if (!Pair.Value->TryGetObject(Localized) || !(*Localized)->TryGetStringField(TEXT("sv"), Swedish))
                return Fail(TEXT("Invalid localized field: ") + E.Id + TEXT(".") + FString(Pair.Key));
            E.Texts.Add(FString(Pair.Key), *Localized);
        }
        if (!E.Texts.Contains(TEXT("title"))) return Fail(TEXT("Missing title: ") + E.Id);
        (*Obj)->TryGetStringArrayField(TEXT("related"), E.Related);
        (*Obj)->TryGetStringArrayField(TEXT("route"), E.Route);
        const TArray<TSharedPtr<FJsonValue>>* Sources = nullptr;
        if ((*Obj)->TryGetArrayField(TEXT("sources"), Sources)) for (const auto& Source : *Sources)
        {
            const TSharedPtr<FJsonObject>* S = nullptr;
            if (!Source->TryGetObject(S)) return Fail(TEXT("Invalid source: ") + E.Id);
            FTMOPAtlasSource Item;
            (*S)->TryGetStringField(TEXT("title"), Item.Title);
            (*S)->TryGetStringField(TEXT("url"), Item.Url);
            (*S)->TryGetStringField(TEXT("published"), Item.Published);
            (*S)->TryGetStringField(TEXT("document"), Item.Document);
            if (Item.Title.IsEmpty() || (Item.Url.IsEmpty() ? Item.Document.IsEmpty() : !Item.Url.StartsWith(TEXT("https://")))) return Fail(TEXT("Invalid source URL: ") + E.Id);
            E.Sources.Add(MoveTemp(Item));
        }
        const TArray<TSharedPtr<FJsonValue>>* Participants = nullptr;
        TSet<FString> ParticipantIds;
        if ((*Obj)->HasField(TEXT("participants")))
        {
            if (E.Kind != TEXT("conflict") || !(*Obj)->TryGetArrayField(TEXT("participants"), Participants) || Participants->Num() > 32)
                return Fail(TEXT("Invalid participants: ") + E.Id);
            for (const auto& V : *Participants)
            {
                const TSharedPtr<FJsonObject>* P = nullptr;
                FTMOPAtlasParticipant Item;
                const TArray<TSharedPtr<FJsonValue>>* Offset = nullptr;
                double X = 0, Y = 0;
                if (!V->TryGetObject(P) || !(*P)->TryGetStringField(TEXT("id"), Item.Id) || Item.Id.IsEmpty() || ParticipantIds.Contains(Item.Id) ||
                    !(*P)->TryGetStringField(TEXT("label"), Item.Label) || !E.Texts.Contains(Item.Label) ||
                    !(*P)->TryGetNumberField(TEXT("lat"), Item.Latitude) || !(*P)->TryGetNumberField(TEXT("lon"), Item.Longitude) ||
                    !FMath::IsFinite(Item.Latitude) || !FMath::IsFinite(Item.Longitude) || FMath::Abs(Item.Latitude) > 90 || FMath::Abs(Item.Longitude) > 180 ||
                    !(*P)->TryGetArrayField(TEXT("offset"), Offset) || Offset->Num() != 2 ||
                    !(*Offset)[0]->TryGetNumber(X) || !(*Offset)[1]->TryGetNumber(Y) ||
                    !FMath::IsFinite(X) || !FMath::IsFinite(Y) || FMath::Abs(X) > 80 || FMath::Abs(Y) > 80)
                    return Fail(TEXT("Invalid participant: ") + E.Id);
                Item.Offset = FVector2D(X, Y);
                if ((*P)->HasField(TEXT("flag")) &&
                    (!(*P)->TryGetStringField(TEXT("flag"), Item.Flag) || Item.Flag.Len() != 2 ||
                     Item.Flag[0] < 'a' || Item.Flag[0] > 'z' || Item.Flag[1] < 'a' || Item.Flag[1] > 'z'))
                    return Fail(TEXT("Invalid participant flag: ") + E.Id);
                ParticipantIds.Add(Item.Id); E.Participants.Add(MoveTemp(Item));
            }
        }
        const TArray<TSharedPtr<FJsonValue>>* Links = nullptr;
        TSet<FString> LinkIds;
        if ((*Obj)->HasField(TEXT("links")))
        {
            if (E.Kind != TEXT("conflict") || !(*Obj)->TryGetArrayField(TEXT("links"), Links) || Links->Num() > 64)
                return Fail(TEXT("Invalid conflict links: ") + E.Id);
            for (const auto& V : *Links)
            {
                const TSharedPtr<FJsonObject>* P = nullptr;
                FTMOPAtlasLink Item;
                if (!V->TryGetObject(P) || !(*P)->TryGetStringField(TEXT("from"), Item.From) || !(*P)->TryGetStringField(TEXT("to"), Item.To) ||
                    !(*P)->TryGetStringField(TEXT("kind"), Item.Kind) || !ParticipantIds.Contains(Item.From) || !ParticipantIds.Contains(Item.To) || Item.From == Item.To ||
                    (Item.Kind != TEXT("opposition") && Item.Kind != TEXT("support") && Item.Kind != TEXT("violence")))
                    return Fail(TEXT("Invalid conflict link: ") + E.Id);
                const FString Key = Item.From + TEXT("|") + Item.To + TEXT("|") + Item.Kind;
                if (LinkIds.Contains(Key)) return Fail(TEXT("Duplicate conflict link: ") + E.Id);
                LinkIds.Add(Key); E.Links.Add(MoveTemp(Item));
            }
        }
        if (E.Kind == TEXT("conflict") && (E.From.IsEmpty() || E.To.IsEmpty() || E.Participants.Num() < 2 || E.Links.IsEmpty() || E.Sources.IsEmpty()))
            return Fail(TEXT("Incomplete dated conflict: ") + E.Id);
        if ((*Obj)->HasField(TEXT("hierarchy")))
        {
            const TSharedPtr<FJsonObject>* H = nullptr;
            const TArray<TSharedPtr<FJsonValue>>* Nodes = nullptr;
            FString AsOf;
            if (!(*Obj)->TryGetObjectField(TEXT("hierarchy"), H) ||
                !(*H)->TryGetStringField(TEXT("as_of"), AsOf) || AsOf != TEXT("1986-02-28") ||
                !(*H)->TryGetArrayField(TEXT("nodes"), Nodes) || Nodes->IsEmpty() || Nodes->Num() > 512)
                return Fail(TEXT("Invalid hierarchy: ") + E.Id);
            TSet<FString> OfficeIds;
            for (const auto& V : *Nodes)
            {
                const TSharedPtr<FJsonObject>* N = nullptr;
                FTMOPAtlasOffice Item;
                if (!V->TryGetObject(N) || !(*N)->TryGetStringField(TEXT("id"), Item.Id) || Item.Id.IsEmpty() || OfficeIds.Contains(Item.Id) ||
                    !(*N)->TryGetStringField(TEXT("parent"), Item.Parent) || !(*N)->TryGetStringField(TEXT("label"), Item.Label) || !E.Texts.Contains(Item.Label) ||
                    !(*N)->TryGetStringField(TEXT("relation"), Item.Relation) || (Item.Relation != TEXT("group") && Item.Relation != TEXT("reports_to")))
                    return Fail(TEXT("Invalid hierarchy node: ") + E.Id);
                if ((*N)->HasField(TEXT("note")) && (!(*N)->TryGetStringField(TEXT("note"), Item.Note) || !E.Texts.Contains(Item.Note)))
                    return Fail(TEXT("Invalid hierarchy note: ") + E.Id);
                if ((*N)->HasField(TEXT("sources")) && !(*N)->TryGetStringArrayField(TEXT("sources"), Item.Sources))
                    return Fail(TEXT("Invalid hierarchy sources: ") + E.Id);
                for (const FString& Url : Item.Sources) if (!Url.StartsWith(TEXT("https://")))
                    return Fail(TEXT("Invalid hierarchy source URL: ") + E.Id);
                if (Item.Relation == TEXT("reports_to") && (Item.Parent.IsEmpty() || Item.Sources.IsEmpty()))
                    return Fail(TEXT("Reporting relationship requires parent and source: ") + E.Id);
                OfficeIds.Add(Item.Id); E.Hierarchy.Add(MoveTemp(Item));
            }
            // Validate every ancestry chain before the recursive Slate tree is built.
            for (const auto& Node : E.Hierarchy)
            {
                TSet<FString> Seen;
                const FTMOPAtlasOffice* Current = &Node;
                while (Current)
                {
                    if (Seen.Contains(Current->Id) || Seen.Num() >= 32)
                        return Fail(TEXT("Cyclic or excessively deep hierarchy: ") + E.Id);
                    Seen.Add(Current->Id);
                    if (Current->Parent.IsEmpty()) break;
                    const FString Parent = Current->Parent;
                    Current = E.Hierarchy.FindByPredicate([&Parent](const auto& Candidate) { return Candidate.Id == Parent; });
                    if (!Current) return Fail(TEXT("Unknown hierarchy parent: ") + E.Id + TEXT(".") + Parent);
                }
            }
        }
        Ids.Add(E.Id); Entries.Add(MoveTemp(E));
    }
    for (const FTMOPAtlasEntry& E : Entries)
    {
        if (E.Kind == TEXT("actor"))
        {
            const FTMOPAtlasEntry* Parent = Find(E.Country);
            if (!Parent || Parent->Kind != TEXT("country")) return Fail(TEXT("Actor requires a country: ") + E.Id);
        }
        for (const FString& Ref : E.Related) if (!Find(Ref)) return Fail(TEXT("Unknown related ID: ") + Ref);
        for (const FString& Ref : E.Route) if (!Find(Ref)) return Fail(TEXT("Unknown route ID: ") + Ref);
        if (!E.Route.IsEmpty() && (E.Route.Num() < 2 || (E.Kind != TEXT("arms") && E.Kind != TEXT("funds"))))
            return Fail(TEXT("Invalid route: ") + E.Id);
    }
    // Optional map geometry: menu remains usable when custom deployments omit it.
    if (Read(Directory / TEXT("coastlines.json"), Root))
    {
        const TArray<TSharedPtr<FJsonValue>>* Lines = nullptr;
        if (Root->TryGetArrayField(TEXT("lines"), Lines)) for (const auto& Line : *Lines)
        {
            const TArray<TSharedPtr<FJsonValue>>* Points = nullptr;
            if (!Line->TryGetArray(Points)) continue;
            TArray<FVector> Curve;
            for (const auto& Point : *Points)
            {
                const TArray<TSharedPtr<FJsonValue>>* XY = nullptr;
                double Lon, Lat;
                if (Point->TryGetArray(XY) && XY->Num() == 2 && (*XY)[0]->TryGetNumber(Lon) && (*XY)[1]->TryGetNumber(Lat) &&
                    FMath::IsFinite(Lon) && FMath::IsFinite(Lat) && FMath::Abs(Lon) <= 180 && FMath::Abs(Lat) <= 90)
                    Curve.Add(TMOPGlobe::Unit(Lat, Lon));
            }
            if (Curve.Num() > 1) Coastlines.Add(MoveTemp(Curve));
        }
    }
    return true;
}
