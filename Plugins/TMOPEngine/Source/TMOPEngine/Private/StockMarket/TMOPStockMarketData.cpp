#include "StockMarket/TMOPStockMarketData.h"
#include "Localization/TMOPLocalization.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

FText FTMOPMarketText::Resolve(const FString& Id, const FString& Field) const
{
    if (!Values) return FText::GetEmpty();
    FString Swedish;
    Values->TryGetStringField(TEXT("sv"), Swedish);
    const FText Override = FTMOPLocalization::TableText(TEXT("TMOP_StockMarket"), Id, Field, Swedish);
    if (Override.ToString() != Swedish) return Override;
    const FString Language = FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName();
    FString Translation, Source;
    if (Language != TEXT("sv") && Values->TryGetStringField(Language, Translation) &&
        Values->TryGetStringField(Language + TEXT("_source"), Source) && Source == Swedish && !Translation.IsEmpty())
        return FText::FromString(Translation);
    return FText::FromString(Swedish);
}

bool FTMOPMarketEntry::Change(double& Points, double& Percent) const
{
    Points = Percent = 0;
    if (!Before.IsSet() || !After.IsSet() || !FMath::IsFinite(Before.GetValue()) ||
        !FMath::IsFinite(After.GetValue()) || Before.GetValue() <= 0 || After.GetValue() < 0) return false;
    const double Delta = After.GetValue() - Before.GetValue();
    const double Rate = 100.0 * Delta / Before.GetValue();
    if (!FMath::IsFinite(Delta) || !FMath::IsFinite(Rate)) return false;
    Points = Delta; Percent = Rate;
    return true;
}

const FTMOPMarketSource* FTMOPStockMarketData::FindSource(const FString& Id) const
{
    return Sources.FindByPredicate([&Id](const FTMOPMarketSource& S) { return S.Id == Id; });
}

bool FTMOPStockMarketData::Load()
{
    Entries.Reset(); Sources.Reset(); Error.Reset(); BeforeDate.Reset(); AfterDate.Reset(); Method.Values.Reset();
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("TMOPEngine"));
    FString Json;
    if (!Plugin || !FFileHelper::LoadFileToString(Json, *(Plugin->GetContentDir() / TEXT("StockMarket/markets.json"))))
    { Error = TEXT("Cannot read StockMarket/markets.json"); return false; }
    return Parse(Json);
}

bool FTMOPStockMarketData::Parse(const FString& Json)
{
    Entries.Reset(); Sources.Reset(); Error.Reset(); BeforeDate.Reset(); AfterDate.Reset(); Method.Values.Reset();
    auto Fail = [this](const FString& Why)
    {
        Entries.Reset(); Sources.Reset(); BeforeDate.Reset(); AfterDate.Reset(); Method.Values.Reset();
        Error = Why; return false;
    };
    TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root)
        return Fail(TEXT("Invalid market JSON"));
    double Schema = 0;
    if (!Root->TryGetNumberField(TEXT("schema"), Schema) || Schema != 1)
        return Fail(TEXT("Unsupported market schema"));
    auto DateOK = [](const FString& Date)
    {
        FDateTime Parsed;
        return Date.Len() == 10 && Date[4] == '-' && Date[7] == '-' &&
            FDateTime::ParseIso8601(*(Date + TEXT("T00:00:00Z")), Parsed);
    };
    auto ReadText = [](const TSharedPtr<FJsonObject>& Object, const TCHAR* Key, FTMOPMarketText& Text)
    {
        const TSharedPtr<FJsonObject>* Localized = nullptr;
        FString Swedish;
        if (!Object->TryGetObjectField(Key, Localized) || !Localized->IsValid() ||
            !(*Localized)->TryGetStringField(TEXT("sv"), Swedish)) return false;
        Text.Values = *Localized;
        return true;
    };
    if (!Root->TryGetStringField(TEXT("before_date"), BeforeDate) || !Root->TryGetStringField(TEXT("after_date"), AfterDate) ||
        !DateOK(BeforeDate) || !DateOK(AfterDate) || BeforeDate >= AfterDate || !ReadText(Root, TEXT("method"), Method))
        return Fail(TEXT("Invalid comparison dates or method"));
    const TArray<TSharedPtr<FJsonValue>>* SourceRows = nullptr;
    if (!Root->TryGetArrayField(TEXT("sources"), SourceRows) || SourceRows->IsEmpty() || SourceRows->Num() > 256)
        return Fail(TEXT("Invalid sources"));
    TSet<FString> SourceIds;
    for (const auto& Value : *SourceRows)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        if (!Value.IsValid() || !Value->TryGetObject(Obj) || !Obj->IsValid()) return Fail(TEXT("Invalid source object"));
        FTMOPMarketSource S;
        if (!(*Obj)->TryGetStringField(TEXT("id"), S.Id) || S.Id.IsEmpty() || SourceIds.Contains(S.Id) ||
            !(*Obj)->TryGetStringField(TEXT("title"), S.Title) || S.Title.IsEmpty() ||
            !(*Obj)->TryGetStringField(TEXT("published"), S.Published) || !DateOK(S.Published) ||
            !(*Obj)->TryGetStringField(TEXT("url"), S.URL) || !S.URL.StartsWith(TEXT("https://")))
            return Fail(TEXT("Invalid source: ") + S.Id);
        SourceIds.Add(S.Id); Sources.Add(MoveTemp(S));
    }
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    if (!Root->TryGetArrayField(TEXT("entries"), Rows) || Rows->IsEmpty() || Rows->Num() > 4096)
        return Fail(TEXT("Invalid entries"));
    const TSet<FString> Regions = {TEXT("europe"), TEXT("north_america"), TEXT("asia_pacific"), TEXT("africa"), TEXT("world")};
    const TSet<FString> Kinds = {TEXT("market"), TEXT("sector"), TEXT("world")};
    const TSet<FString> Statuses = {TEXT("daily"), TEXT("close"), TEXT("provisional"), TEXT("source_conflict"), TEXT("missing")};
    TSet<FString> Ids;
    for (const auto& Value : *Rows)
    {
        const TSharedPtr<FJsonObject>* Object = nullptr;
        if (!Value.IsValid() || !Value->TryGetObject(Object) || !Object->IsValid()) return Fail(TEXT("Invalid entry object"));
        const TSharedPtr<FJsonObject>& Obj = *Object;
        auto E = MakeShared<FTMOPMarketEntry>();
        double Precision = 0;
        if (!Obj->TryGetStringField(TEXT("id"), E->Id) || E->Id.IsEmpty() || Ids.Contains(E->Id) ||
            !Obj->TryGetStringField(TEXT("region"), E->Region) || !Regions.Contains(E->Region) ||
            !Obj->TryGetStringField(TEXT("kind"), E->Kind) || !Kinds.Contains(E->Kind) ||
            !Obj->TryGetStringField(TEXT("status"), E->Status) || !Statuses.Contains(E->Status) ||
            !Obj->TryGetNumberField(TEXT("decimals"), Precision) || !FMath::IsFinite(Precision) ||
            Precision < 0 || Precision > 4 || Precision != static_cast<int32>(Precision) ||
            !ReadText(Obj, TEXT("market"), E->Market) || !ReadText(Obj, TEXT("name"), E->Name) || !ReadText(Obj, TEXT("notes"), E->Notes))
            return Fail(TEXT("Invalid market entry: ") + E->Id);
        E->Decimals = static_cast<int32>(Precision);
        auto ReadNumber = [&Obj](const TCHAR* Key, TOptional<double>& Number)
        {
            const TSharedPtr<FJsonValue>* Field = Obj->Values.Find(Key);
            if (!Field || !Field->IsValid()) return false;
            if ((*Field)->Type == EJson::Null) return true;
            double Parsed = 0;
            if ((*Field)->Type != EJson::Number || !(*Field)->TryGetNumber(Parsed) ||
                !FMath::IsFinite(Parsed) || Parsed < 0 || Parsed > 1.e12) return false;
            Number = Parsed; return true;
        };
        if (!ReadNumber(TEXT("before"), E->Before) || !ReadNumber(TEXT("after"), E->After))
            return Fail(TEXT("Invalid observation: ") + E->Id);
        if ((!E->Before.IsSet() || !E->After.IsSet()) && E->Status != TEXT("missing") && E->Status != TEXT("source_conflict"))
            return Fail(TEXT("Missing observation must be marked: ") + E->Id);
        const TArray<TSharedPtr<FJsonValue>>* Citations = nullptr;
        if (!Obj->TryGetArrayField(TEXT("citations"), Citations) || Citations->IsEmpty() || Citations->Num() > 32)
            return Fail(TEXT("Missing citations: ") + E->Id);
        for (const auto& Citation : *Citations)
        {
            const TSharedPtr<FJsonObject>* C = nullptr;
            FTMOPMarketCitation Ref;
            if (!Citation.IsValid() || !Citation->TryGetObject(C) || !C->IsValid() ||
                !(*C)->TryGetStringField(TEXT("source"), Ref.SourceId) || !SourceIds.Contains(Ref.SourceId) ||
                !(*C)->TryGetStringField(TEXT("pages"), Ref.Pages) || Ref.Pages.IsEmpty())
                return Fail(TEXT("Invalid citation: ") + E->Id);
            E->Citations.Add(MoveTemp(Ref));
        }
        Ids.Add(E->Id); Entries.Add(E);
    }
    return true;
}
