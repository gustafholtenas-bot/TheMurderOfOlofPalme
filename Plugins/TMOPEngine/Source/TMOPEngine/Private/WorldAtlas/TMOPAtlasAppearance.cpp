#include "WorldAtlas/TMOPAtlasAppearance.h"
#include "WorldAtlas/TMOPGlobeMath.h"
#include "Localization/TMOPLocalization.h"
#include "Engine/Texture2D.h"
#include "ImageUtils.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Internationalization.h"
#include "Internationalization/Culture.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
bool ReadAppearanceJson(const FString& Path, TSharedPtr<FJsonObject>& Root)
{
    FString Json;
    return FFileHelper::LoadFileToString(Json, *Path) &&
        FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) && Root.IsValid();
}
bool ReadPoint(const TSharedPtr<FJsonValue>& Value, FVector& Out)
{
    const TArray<TSharedPtr<FJsonValue>>* XY = nullptr;
    double Lon = 0, Lat = 0;
    if (!Value.IsValid() || !Value->TryGetArray(XY) || XY->Num() != 2 ||
        !(*XY)[0]->TryGetNumber(Lon) || !(*XY)[1]->TryGetNumber(Lat) ||
        !FMath::IsFinite(Lon) || !FMath::IsFinite(Lat) || FMath::Abs(Lon) > 180 || FMath::Abs(Lat) > 90) return false;
    Out = TMOPGlobe::Unit(Lat, Lon);
    return true;
}
}

FLinearColor FTMOPAtlasAppearance::BlocColor(const FString& Bloc)
{
    if (Bloc == TEXT("west")) return FLinearColor(.045f, .24f, .62f);
    if (Bloc == TEXT("east")) return FLinearColor(.63f, .075f, .065f);
    return FLinearColor(.25f, .29f, .30f);
}

FText FTMOPAtlasAppearance::Alignment(const FString& CountryId) const
{
    const auto* Text = Alignments.Find(CountryId);
    if (!Text) return FText::GetEmpty();
    const FText Override = FTMOPLocalization::TableText(TEXT("TMOP_WorldAtlasAlignments"), CountryId, TEXT("description"), Text->Swedish);
    if (Override.ToString() != Text->Swedish) return Override;
    const bool bEnglish = FInternationalization::Get().GetCurrentLanguage()->GetTwoLetterISOLanguageName() == TEXT("en");
    return FText::FromString(bEnglish && !Text->English.IsEmpty() && Text->EnglishSource == Text->Swedish ? Text->English : Text->Swedish);
}

void FTMOPAtlasAppearance::AddReferencedObjects(FReferenceCollector& Collector)
{
    Collector.AddReferencedObject(FlagTexture);
}

bool FTMOPAtlasAppearance::Load()
{
    Lands.Reset(); Flags.Reset(); Alignments.Reset(); FlagTexture = nullptr; Error.Reset();
    const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("TMOPEngine"));
    if (!Plugin) { Error = TEXT("TMOPEngine plugin not found"); return false; }
    const FString Directory = Plugin->GetContentDir() / TEXT("WorldAtlas");
    TSharedPtr<FJsonObject> Root;
    auto Fail = [this](const TCHAR* Message) { Error = Message; return false; };
    TMap<FString, FString> Blocs;
    const TArray<TSharedPtr<FJsonValue>>* Rows = nullptr;
    FString Date;
    double Version = 0;
    if (!ReadAppearanceJson(Directory / TEXT("alignments_1986.json"), Root) ||
        !Root->TryGetNumberField(TEXT("schema"), Version) || Version != 1 ||
        !Root->TryGetStringField(TEXT("reference_date"), Date) || Date != TEXT("1986-02-28") ||
        !Root->TryGetArrayField(TEXT("countries"), Rows) || Rows->Num() > 512)
        return Fail(TEXT("Invalid alignments_1986.json"));
    for (const auto& Value : *Rows)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        FString Id, Bloc;
        FAlignmentText Text;
        if (!Value->TryGetObject(Obj) || !(*Obj)->TryGetStringField(TEXT("id"), Id) || Id.IsEmpty() || Blocs.Contains(Id) ||
            !(*Obj)->TryGetStringField(TEXT("bloc"), Bloc) ||
            (Bloc != TEXT("west") && Bloc != TEXT("east") && Bloc != TEXT("other")) ||
            !(*Obj)->TryGetStringField(TEXT("sv"), Text.Swedish) || !(*Obj)->TryGetStringField(TEXT("en"), Text.English))
            return Fail(TEXT("Invalid alignment record"));
        (*Obj)->TryGetStringField(TEXT("en_source"), Text.EnglishSource);
        Blocs.Add(Id, Bloc); Alignments.Add(Id, MoveTemp(Text));
    }
    if (!ReadAppearanceJson(Directory / TEXT("land_1986.json"), Root) ||
        !Root->TryGetNumberField(TEXT("schema"), Version) || Version != 1 ||
        !Root->TryGetStringField(TEXT("reference_date"), Date) || Date != TEXT("1986-02-28") ||
        !Root->TryGetArrayField(TEXT("countries"), Rows) || Rows->Num() > 512)
        return Fail(TEXT("Invalid land_1986.json"));
    TArray<FTMOPAtlasLand> LoadedLands;
    TSet<FString> LandIds;
    int32 TotalPoints = 0, TotalIndices = 0;
    for (const auto& Value : *Rows)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        const TArray<TSharedPtr<FJsonValue>> *Vertices = nullptr, *Indices = nullptr, *Borders = nullptr;
        FTMOPAtlasLand Land;
        if (!Value->TryGetObject(Obj) || !(*Obj)->TryGetStringField(TEXT("id"), Land.Id) ||
            LandIds.Contains(Land.Id) || !Blocs.Contains(Land.Id) ||
            !(*Obj)->TryGetArrayField(TEXT("vertices"), Vertices) ||
            !(*Obj)->TryGetArrayField(TEXT("indices"), Indices) || Indices->Num() % 3 ||
            !(*Obj)->TryGetArrayField(TEXT("borders"), Borders)) return Fail(TEXT("Invalid land mesh"));
        TotalPoints += Vertices->Num(); TotalIndices += Indices->Num();
        if (TotalPoints > 400000 || TotalIndices > 1200000 || Borders->Num() > 10000) return Fail(TEXT("Land mesh too large"));
        Land.Bloc = Blocs[Land.Id]; LandIds.Add(Land.Id);
        for (const auto& V : *Vertices)
        {
            FVector P;
            if (!ReadPoint(V, P)) return Fail(TEXT("Invalid land coordinate"));
            Land.Vertices.Add(P);
        }
        for (const auto& I : *Indices)
        {
            double Index = 0;
            if (!I->TryGetNumber(Index) || !FMath::IsFinite(Index) || Index < 0 || Index >= Land.Vertices.Num() || Index != FMath::FloorToDouble(Index))
                return Fail(TEXT("Invalid land triangle index"));
            Land.Indices.Add(int32(Index));
        }
        for (const auto& B : *Borders)
        {
            const TArray<TSharedPtr<FJsonValue>>* Points = nullptr;
            if (!B->TryGetArray(Points) || Points->Num() < 2) return Fail(TEXT("Invalid land border"));
            TotalPoints += Points->Num();
            if (TotalPoints > 400000) return Fail(TEXT("Land borders too large"));
            TArray<FVector> Line;
            for (const auto& V : *Points)
            {
                FVector P;
                if (!ReadPoint(V, P)) return Fail(TEXT("Invalid border coordinate"));
                Line.Add(P);
            }
            Land.Borders.Add(MoveTemp(Line));
        }
        LoadedLands.Add(MoveTemp(Land));
    }
    Lands = MoveTemp(LoadedLands);
    // Flags can fail independently: land and ordinary country markers remain usable.
    TArray<uint8> Png;
    if (!ReadAppearanceJson(Directory / TEXT("flags_1986.json"), Root) ||
        !Root->TryGetNumberField(TEXT("schema"), Version) || Version != 1 ||
        !Root->TryGetStringField(TEXT("reference_date"), Date) || Date != TEXT("1986-02-28") ||
        !Root->TryGetArrayField(TEXT("flags"), Rows) || Rows->Num() > 512 ||
        !FFileHelper::LoadFileToArray(Png, *(Directory / TEXT("flags_1986.png"))) || Png.Num() > 4 * 1024 * 1024)
        return Fail(TEXT("Cannot read flags_1986.json/png"));
    FlagTexture = FImageUtils::ImportBufferAsTexture2D(Png);
    if (!FlagTexture) return Fail(TEXT("Cannot decode flags_1986.png"));
    TMap<FString, FSlateBrush> LoadedFlags;
    for (const auto& Value : *Rows)
    {
        const TSharedPtr<FJsonObject>* Obj = nullptr;
        const TArray<TSharedPtr<FJsonValue>>* Rect = nullptr;
        FString Id;
        if (!Value->TryGetObject(Obj) || !(*Obj)->TryGetStringField(TEXT("id"), Id) || Id.IsEmpty() || LoadedFlags.Contains(Id) ||
            !(*Obj)->TryGetArrayField(TEXT("rect"), Rect) || Rect->Num() != 4) return Fail(TEXT("Invalid flag record"));
        double R[4];
        for (int32 I = 0; I < 4; ++I) if (!(*Rect)[I]->TryGetNumber(R[I]) || !FMath::IsFinite(R[I])) return Fail(TEXT("Invalid flag rectangle"));
        const double W = FlagTexture->GetSizeX(), H = FlagTexture->GetSizeY();
        if (R[0] < 0 || R[1] < 0 || R[2] < 1 || R[3] < 1 || R[0] + R[2] > W || R[1] + R[3] > H)
            return Fail(TEXT("Flag rectangle outside texture"));
        FSlateBrush Brush;
        Brush.SetResourceObject(FlagTexture);
        Brush.ImageSize = FVector2D(24, 24 * R[3] / R[2]);
        Brush.DrawAs = ESlateBrushDrawType::Image;
        Brush.SetUVRegion(FBox2f(FVector2f(R[0] / W, R[1] / H), FVector2f((R[0] + R[2]) / W, (R[1] + R[3]) / H)));
        LoadedFlags.Add(Id, MoveTemp(Brush));
    }
    Flags = MoveTemp(LoadedFlags);
    return true;
}
