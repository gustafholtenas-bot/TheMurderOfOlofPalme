#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/** Display text uses the same source-matching fallback as the world atlas. */
struct FTMOPMarketText
{
    TSharedPtr<FJsonObject> Values;
    FText Resolve(const FString& Id, const FString& Field) const;
};

struct FTMOPMarketSource
{
    FString Id, Title, Published, URL;
};

struct FTMOPMarketCitation
{
    FString SourceId, Pages;
};

struct FTMOPMarketEntry
{
    FString Id, Region, Kind, Status;
    FTMOPMarketText Market, Name, Notes;
    TOptional<double> Before, After;
    int32 Decimals = 2;
    TArray<FTMOPMarketCitation> Citations;

    /** An unavailable value never becomes zero. A zero base cannot yield a percentage. */
    bool Change(double& Points, double& Percent) const;
};

struct FTMOPStockMarketData
{
    FString BeforeDate, AfterDate, Error;
    FTMOPMarketText Method;
    TArray<FTMOPMarketSource> Sources;
    TArray<TSharedPtr<FTMOPMarketEntry>> Entries;

    bool Load();
    bool Parse(const FString& Json);
    const FTMOPMarketSource* FindSource(const FString& Id) const;
};
