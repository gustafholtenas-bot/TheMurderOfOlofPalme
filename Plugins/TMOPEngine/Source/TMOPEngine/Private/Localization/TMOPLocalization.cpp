#include "Localization/TMOPLocalization.h"

#include "Dom/JsonObject.h"
#include "UObject/UnrealType.h"
#include "Dom/JsonValue.h"
#include "Internationalization/Text.h"
#include "Internationalization/PolyglotTextData.h"
#include "Internationalization/TextLocalizationManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/SecureHash.h"
#include "HAL/FileManager.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace
{
    // Only explicitly exported source strings get aliases. Names, IDs and player-authored
    // notes are not catalogued by the exporter and therefore pass through unchanged.
    TMap<FString, FText> SourceTexts;
    struct FTableEntry
    {
        FString Source, Translation;
        FText Text;
        TArray<FString> NativeIds;
    };
    TMap<FString, FTableEntry> TableTexts;
    TMap<FString, FString> NativeTableKeys;
    TSet<FString> TableSources;
    bool bPacksLoaded = false;
    bool IsTextField(const FString& Table, const FString& Name, const FString& Path)
    {
        static const TSet<FString> Fields = {
#include "TMOPTableTextFields.inl"
        };
        const bool bTimelineNote = Table == TEXT("DT_TMOP_People") &&
            Name == TEXT("Notes") && Path.StartsWith(TEXT("Timeline[")) &&
            Path.EndsWith(TEXT("].Notes")) && Path.Find(TEXT("].")) == Path.Len() - 7;
        return bTimelineNote || Fields.Contains(Name) || (Name == TEXT("DisplayName") &&
            !Table.Contains(TEXT("People")) && !Table.Contains(TEXT("Address")));
    }

    void RebuildTableAliases()
    {
        for (const FString& Source : TableSources) SourceTexts.Remove(Source);
        TableSources.Reset();
        NativeTableKeys.Reset();
        TSet<FString> Conflicts, NativeConflicts;
        TMap<FString, FString> Translations;
        for (const auto& Pair : TableTexts)
        {
            const FTableEntry& Entry = Pair.Value;
            TableSources.Add(Entry.Source);
            const FString* Previous = Translations.Find(Entry.Source);
            if (Entry.Translation.IsEmpty() || (Previous && *Previous != Entry.Translation))
                Conflicts.Add(Entry.Source);
            Translations.Add(Entry.Source, Entry.Translation);
            for (const FString& Id : Entry.NativeIds)
            {
                if (const FString* Existing = NativeTableKeys.Find(Id))
                    if (*Existing != Pair.Key) NativeConflicts.Add(Id);
                NativeTableKeys.Add(Id, Pair.Key);
            }
        }
        for (const FString& Id : NativeConflicts) NativeTableKeys.Remove(Id);
        for (const auto& Pair : TableTexts)
        {
            SourceTexts.Remove(Pair.Value.Source);
        }
        for (const auto& Pair : TableTexts)
            if (!Conflicts.Contains(Pair.Value.Source))
                SourceTexts.Add(Pair.Value.Source, Pair.Value.Text);
    }

    TArray<FString> FormatArguments(const FString& Value)
    {
        TArray<FString> Arguments;
        for (int32 I = 0; I < Value.Len(); ++I)
        {
            if (Value[I] == TEXT('`')) { ++I; continue; }
            if (Value[I] != TEXT('{')) continue;
            const int32 Start = I++;
            while (I < Value.Len() && Value[I] != TEXT('}')) ++I;
            Arguments.Add(Value.Mid(Start, I - Start + 1));
        }
        Arguments.Sort();
        return Arguments;
    }
}

FString FTMOPLocalization::SourceKey(const FString& Source)
{
    // Hash exact UTF-8 bytes (no trimming/normalization), identical to the export tool.
    const FTCHARToUTF8 Utf8(*Source);
    uint8 Digest[FSHA1::DigestSize];
    FSHA1::HashBuffer(Utf8.Get(), Utf8.Length(), Digest);
    return BytesToHex(Digest, FSHA1::DigestSize).ToLower();
}

FString FTMOPLocalization::TableKey(const FString& Table, const FString& Row, const FString& Field)
{
    const auto Escape = [](FString Value)
    {
        return Value.Replace(TEXT("%"), TEXT("%25")).Replace(TEXT("/"), TEXT("%2F"));
    };
    return Escape(Table) + TEXT("/") + Escape(Row) + TEXT("/") + Escape(Field);
}

FText FTMOPLocalization::TableText(const FString& Table, const FString& Row,
    const FString& Field, const FString& Source)
{
    if (const FTableEntry* Entry = TableTexts.Find(TableKey(Table, Row, Field)))
        if (Entry->Source == Source && !Entry->Translation.IsEmpty()) return Entry->Text;
    // Explicit context must never borrow a different row's translation.
    return FText::ChangeKey(TEXT("TMOP.TableFallback"), TableKey(Table, Row, Field) + TEXT("/") + SourceKey(Source), FText::FromString(Source));
}

FText FTMOPLocalization::TableText(const FString& Table, const FString& Row,
    const FString& Field, const FText& Source)
{
    const FString* Native = FTextInspector::GetSourceString(Source);
    return TableText(Table, Row, Field, Native ? *Native : Source.ToString());
}

void FTMOPLocalization::LocalizeRowView(const FString& Table, const FString& Row,
    UScriptStruct* Type, void* Copy)
{
    if (!Type || !Copy || TableTexts.IsEmpty()) return;
    TFunction<void(FProperty*, void*, const FString&)> Visit;
    Visit = [&](FProperty* Property, void* Value, const FString& Path)
    {
        const FString Key = TableKey(Table, Row, Path);
        if (FTextProperty* TextProperty = CastField<FTextProperty>(Property))
        {
            if (IsTextField(Table, Property->GetName(), Path) && TableTexts.Contains(Key))
                TextProperty->SetPropertyValue(Value, TableText(Table, Row, Path, TextProperty->GetPropertyValue(Value)));
        }
        else if (FStrProperty* StringProperty = CastField<FStrProperty>(Property))
        {
            if (IsTextField(Table, Property->GetName(), Path) && TableTexts.Contains(Key))
                StringProperty->SetPropertyValue(Value, TableText(Table, Row, Path, StringProperty->GetPropertyValue(Value)).ToString());
        }
        else if (FStructProperty* Struct = CastField<FStructProperty>(Property))
        {
            for (TFieldIterator<FProperty> It(Struct->Struct); It; ++It)
                Visit(*It, It->ContainerPtrToValuePtr<void>(Value), Path + TEXT(".") + It->GetName());
        }
        else if (FArrayProperty* Array = CastField<FArrayProperty>(Property))
        {
            FScriptArrayHelper Helper(Array, Value);
            FString Identity;
            TArray<FString> Tokens;
            if (FStructProperty* Item = CastField<FStructProperty>(Array->Inner))
            {
                for (const TCHAR* Candidate : {TEXT("LocalizationId"), TEXT("SegmentId"), TEXT("EntryId")})
                {
                    FProperty* Id = Item->Struct->FindPropertyByName(FName(Candidate));
                    if (!Id) continue;
                    TSet<FString> Seen;
                    Tokens.Reset();
                    for (int32 Index = 0; Index < Helper.Num(); ++Index)
                    {
                        void* IdValue = Id->ContainerPtrToValuePtr<void>(Helper.GetRawPtr(Index));
                        FString Token;
                        if (FNameProperty* Name = CastField<FNameProperty>(Id)) Token = Name->GetPropertyValue(IdValue).ToString();
                        else if (FStrProperty* String = CastField<FStrProperty>(Id)) Token = String->GetPropertyValue(IdValue);
                        if (Token.IsEmpty() || Token == TEXT("None") || Seen.Contains(Token)) break;
                        Seen.Add(Token);
                        Tokens.Add(Token.Replace(TEXT("%"), TEXT("%25")).Replace(TEXT("["), TEXT("%5B")).Replace(TEXT("]"), TEXT("%5D")));
                    }
                    if (Tokens.Num() == Helper.Num()) { Identity = Candidate; break; }
                }
            }
            for (int32 Index = 0; Index < Helper.Num(); ++Index)
            {
                const FString Token = Identity.IsEmpty() ? FString::FromInt(Index)
                    : TEXT("@") + Identity + TEXT("=") + Tokens[Index];
                Visit(Array->Inner, Helper.GetRawPtr(Index), Path + TEXT("[") + Token + TEXT("]"));
            }
        }
    };
    for (TFieldIterator<FProperty> It(Type); It; ++It)
        Visit(*It, It->ContainerPtrToValuePtr<void>(Copy), It->GetName());
}

FText FTMOPLocalization::Text(const FString& Source)
{
    if (const FText* Localized = SourceTexts.Find(Source)) return *Localized;
    return FText::FromString(Source);
}

FText FTMOPLocalization::Text(const FText& Source)
{
    // Native NSLOCTEXT/StringTable identities take precedence over the legacy-string bridge.
    const TOptional<FString> Namespace = FTextInspector::GetNamespace(Source);
    if (Namespace.IsSet() && (Namespace.GetValue() == TEXT("TMOP") || Namespace.GetValue() == TEXT("TMOP.Table") || Namespace.GetValue() == TEXT("TMOP.TableFallback"))) return Source;
    const TOptional<FString> Key = FTextInspector::GetKey(Source);
    if (Namespace.IsSet() && Key.IsSet())
    {
        if (const FString* StableKey = NativeTableKeys.Find(Namespace.GetValue() + TEXT("\n") + Key.GetValue()))
        {
            const FTableEntry& Entry = TableTexts.FindChecked(*StableKey);
            const FString* Original = FTextInspector::GetSourceString(Source);
            if (Original && *Original == Entry.Source && !Entry.Translation.IsEmpty()) return Entry.Text;
            return FText::AsCultureInvariant(Original ? *Original : Source.ToString());
        }
    }
    const FString* Native = FTextInspector::GetSourceString(Source);
    if (Native)
        if (const FText* Localized = SourceTexts.Find(*Native)) return *Localized;
    return Source;
}

uint32 FTMOPLocalization::GetRevision()
{
    return FTextLocalizationManager::Get().GetTextRevision();
}

void FTMOPLocalization::LoadLanguagePacks()
{
    // Packs are immutable for a process lifetime. Restart to install/update one; changing
    // the active language remains immediate. This avoids stale entries after removing a pack.
    if (bPacksLoaded) return;
    bPacksLoaded = true;
    LoadPack(FPaths::ProjectContentDir() / TEXT("Localization/TMOP/en.json"), TEXT("en"));
    LoadPack(FPaths::ProjectSavedDir() / TEXT("LanguagePacks/en.json"), TEXT("en"));
}

bool FTMOPLocalization::LoadPack(const FString& Filename, const FString& Language)
{
    const int64 Size = IFileManager::Get().FileSize(*Filename);
    if (Size < 0) return true; // Optional file.
    if (Size > 32 * 1024 * 1024)
    {
        UE_LOG(LogTemp, Warning, TEXT("TMOP language pack exceeds 32 MiB: %s"), *Filename);
        return false;
    }
    FString Json;
    TSharedPtr<FJsonObject> Root;
    if (!FFileHelper::LoadFileToString(Json, *Filename) ||
        !FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Json), Root) || !Root.IsValid())
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid TMOP language pack JSON: %s"), *Filename);
        return false;
    }
    double Version = 0;
    FString Culture;
    const TArray<TSharedPtr<FJsonValue>>* Entries = nullptr;
    if (!Root->TryGetNumberField(TEXT("schema_version"), Version) || (Version != 1 && Version != 2) ||
        !Root->TryGetStringField(TEXT("language"), Culture) || Culture != Language ||
        !Root->TryGetArrayField(TEXT("entries"), Entries) || Entries->Num() > 100000)
    {
        UE_LOG(LogTemp, Warning, TEXT("Unsupported TMOP language pack schema/culture: %s"), *Filename);
        return false;
    }

    // Validate the entire file before applying anything. A malformed override cannot
    // leave half a pack installed; the shipped text stays usable.
    struct FEntry { FString Namespace, Key, Source, Translation; TArray<FString> NativeIds; };
    TArray<FEntry> Validated;
    TSet<FString> Identities;
    for (const TSharedPtr<FJsonValue>& Value : *Entries)
    {
        const TSharedPtr<FJsonObject>* Object = nullptr;
        FEntry Entry;
        if (!Value.IsValid() || !Value->TryGetObject(Object) || !Object->IsValid() ||
            !(*Object)->TryGetStringField(TEXT("namespace"), Entry.Namespace) ||
            !(*Object)->TryGetStringField(TEXT("key"), Entry.Key) ||
            !(*Object)->TryGetStringField(TEXT("source"), Entry.Source) ||
            !(*Object)->TryGetStringField(TEXT("translation"), Entry.Translation) ||
            (Entry.Namespace != TEXT("TMOP") && Entry.Namespace != TEXT("TMOP.Source") && Entry.Namespace != TEXT("TMOP.Table")) ||
            Entry.Key.IsEmpty() || Entry.Source.IsEmpty() ||
            Entry.Source.Len() > 1000000 || Entry.Translation.Len() > 1000000 ||
            (Entry.Namespace == TEXT("TMOP.Source") && Entry.Key != SourceKey(Entry.Source)) ||
            (!Entry.Translation.IsEmpty() && FormatArguments(Entry.Source) != FormatArguments(Entry.Translation)))
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid entry in TMOP language pack: %s"), *Filename);
            return false;
        }
        if (Version == 2 || Entry.Namespace == TEXT("TMOP.Table"))
        {
            FString Hash, Status;
            if (!(*Object)->TryGetStringField(TEXT("source_hash"), Hash) || Hash != SourceKey(Entry.Source) ||
                !(*Object)->TryGetStringField(TEXT("status"), Status) ||
                (Status != TEXT("untranslated") && Status != TEXT("draft") && Status != TEXT("reviewed") && Status != TEXT("needs_review")) ||
                (Status == TEXT("reviewed") && Entry.Translation.IsEmpty()))
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid entry in TMOP language pack: %s"), *Filename);
                return false;
            }
            if (Status != TEXT("reviewed")) Entry.Translation.Empty();
            if (Entry.Namespace == TEXT("TMOP.Table"))
            {
                FString Table, Row, Field;
                if (!(*Object)->TryGetStringField(TEXT("table"), Table) || Table.IsEmpty() ||
                    !(*Object)->TryGetStringField(TEXT("row"), Row) || Row.IsEmpty() ||
                    !(*Object)->TryGetStringField(TEXT("field"), Field) || Field.IsEmpty() ||
                    Entry.Key != TableKey(Table, Row, Field))
                {
                    UE_LOG(LogTemp, Warning, TEXT("Invalid entry in TMOP language pack: %s"), *Filename);
                    return false;
                }
            }
            const TArray<TSharedPtr<FJsonValue>>* NativeIds = nullptr;
            if ((*Object)->HasField(TEXT("native_ids")))
            {
                if (!(*Object)->TryGetArrayField(TEXT("native_ids"), NativeIds)) return false;
                for (const TSharedPtr<FJsonValue>& Alias : *NativeIds)
                {
                    const TSharedPtr<FJsonObject>* AliasObject = nullptr;
                    FString Namespace, Key;
                    if (!Alias.IsValid() || !Alias->TryGetObject(AliasObject) || !AliasObject->IsValid() ||
                        !(*AliasObject)->TryGetStringField(TEXT("namespace"), Namespace) || Namespace.IsEmpty() ||
                        !(*AliasObject)->TryGetStringField(TEXT("key"), Key) || Key.IsEmpty()) return false;
                    Entry.NativeIds.Add(Namespace + TEXT("\n") + Key);
                }
            }
        }
        const FString Identity = Entry.Namespace + TEXT("\n") + Entry.Key;
        if (Identities.Contains(Identity))
        {
            UE_LOG(LogTemp, Warning, TEXT("Duplicate key in TMOP language pack: %s"), *Filename);
            return false;
        }
        Identities.Add(Identity);
        Validated.Add(MoveTemp(Entry));
    }
    for (const FEntry& Entry : Validated)
    {
        if (Entry.Namespace == TEXT("TMOP.Table"))
        {
            FTableEntry TableEntry;
            TableEntry.Source = Entry.Source;
            TableEntry.Translation = Entry.Translation;
            TableEntry.NativeIds = Entry.NativeIds;
            TableEntry.Text = FText::AsCultureInvariant(Entry.Source);
            if (!Entry.Translation.IsEmpty())
            {
                FPolyglotTextData Data(ELocalizedTextSourceCategory::Game,
                    Entry.Namespace, Entry.Key, Entry.Source, TEXT("sv"));
                Data.AddLocalizedString(Language, Entry.Translation);
                FTextLocalizationManager::Get().RegisterPolyglotTextData(Data, true);
                TableEntry.Text = Data.GetText();
            }
            // Empty/draft override deliberately disables this row, even over a shipped pack.
            TableTexts.Add(Entry.Key, MoveTemp(TableEntry));
            continue;
        }
        if (Entry.Translation.IsEmpty()) continue;
        FPolyglotTextData Data(ELocalizedTextSourceCategory::Game,
            Entry.Namespace, Entry.Key, Entry.Source, TEXT("sv"));
        Data.AddLocalizedString(Language, Entry.Translation);
        FTextLocalizationManager::Get().RegisterPolyglotTextData(Data, true);
        if (Entry.Namespace == TEXT("TMOP.Source"))
            SourceTexts.Add(Entry.Source, Data.GetText());
    }
    RebuildTableAliases();
    UE_LOG(LogTemp, Display, TEXT("Loaded TMOP language pack: %s"), *Filename);
    return true;
}
