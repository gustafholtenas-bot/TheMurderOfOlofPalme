#pragma once

#include "CoreMinimal.h"
#include "Misc/Attribute.h"
#include "Engine/DataTable.h"

/** Presentation-only localization. Never modifies table rows, identifiers or save data. */
class TMOPENGINE_API FTMOPLocalization
{
public:
    static FText Text(const FText& Source);
    static FText Text(const FString& Source);
    static FText Text(const TCHAR* Source) { return Text(FString(Source)); }
    static FString String(const FString& Source) { return Text(Source).ToString(); }
    static FString String(const FText& Source) { return Text(Source).ToString(); }
    /** Stable table asset name / exported row Name / field path; checks current source. */
    static FString TableKey(const FString& Table, const FString& Row, const FString& Field);
    static FText TableText(const FString& Table, const FString& Row, const FString& Field, const FString& Source);
    static FText TableText(const FString& Table, const FString& Row, const FString& Field, const FText& Source);
    /** Localize only a disposable presentation copy. Never pass a live table/save row. */
    static void LocalizeRowView(const FString& Table, const FString& Row, UScriptStruct* Type, void* Copy);
    template <typename RowType>
    static RowType RowView(const FString& Table, const FString& Row, const RowType& Source)
    {
        RowType Copy = Source;
        LocalizeRowView(Table, Row, RowType::StaticStruct(), &Copy);
        return Copy;
    }
    template <typename RowType>
    static void TableViews(const UDataTable* Table, TArray<RowType>& Storage, TArray<RowType*>& Rows)
    {
        Storage.Reset(); Rows.Reset();
        if (!IsValid(Table) || Table->GetRowStruct() != RowType::StaticStruct()) return;
        Storage.Reserve(Table->GetRowMap().Num());
        for (const auto& Pair : Table->GetRowMap())
            Storage.Add(RowView(Table->GetName(), Pair.Key.ToString(), *reinterpret_cast<const RowType*>(Pair.Value)));
        for (RowType& Row : Storage) Rows.Add(&Row);
    }
    static FString SourceKey(const FString& Source);

    template <typename... ArgTypes>
    static FText Format(const FText& Pattern, const ArgTypes&... Arguments)
    {
        return FText::Format(Text(Pattern), FormatArgument(Arguments)...);
    }

    // Preserve deferred evaluation for Slate attributes and language changes.
    template <typename CallbackType>
    static TAttribute<FText> Bind(CallbackType Callback)
    {
        return TAttribute<FText>::CreateLambda([Callback]() { return Text(Callback()); });
    }

    /** Load packaged defaults, then local overrides. Empty translations retain Swedish/built-ins. */
    static void LoadLanguagePacks();
    static uint32 GetRevision();

private:
    friend class FTMOPExternalLanguagePackTest;
    static FText FormatArgument(const FText& Value) { return Text(Value); }
    static FText FormatArgument(const FString& Value) { return Text(Value); }
    template <typename T> static const T& FormatArgument(const T& Value) { return Value; }
    static bool LoadPack(const FString& Filename, const FString& Language);
};
