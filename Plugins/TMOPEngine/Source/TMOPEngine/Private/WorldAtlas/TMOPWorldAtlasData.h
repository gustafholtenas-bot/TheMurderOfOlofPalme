#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

struct FTMOPAtlasSource
{
    FString Title, Url, Published, Document;
};

struct FTMOPAtlasParticipant
{
    FString Id, Label, Flag; // Explicit national flag ID; empty for non-state actors.
    double Latitude = 0, Longitude = 0;
    FVector2D Offset = FVector2D::ZeroVector;
};
struct FTMOPAtlasLink
{
    FString From, To, Kind; // opposition, support, violence
};

struct FTMOPAtlasOffice
{
    FString Id, Parent, Label, Note, Relation; // group or reports_to; a forest is allowed.
    TArray<FString> Sources;
};

struct FTMOPAtlasEntry
{
    FString Id, Kind, From, To, Country;
    double Latitude = 0, Longitude = 0;
    // Screen-space separation; geographical position remains unchanged.
    FVector2D MarkerOffset = FVector2D::ZeroVector;
    bool bMarker = true;
    bool bLaterOnly = false;
    TMap<FString, TSharedPtr<FJsonObject>> Texts;
    TArray<FString> Related, Route;
    TArray<FTMOPAtlasSource> Sources;
    TArray<FTMOPAtlasParticipant> Participants;
    TArray<FTMOPAtlasLink> Links;
    TArray<FTMOPAtlasOffice> Hierarchy;

    FText Text(const FString& Field) const;
    bool Visible(bool bLater, bool bNearby = false) const;
};

struct FTMOPWorldAtlasData
{
    TArray<FTMOPAtlasEntry> Entries;
    TArray<TArray<FVector>> Coastlines;
    FString Error;

    bool Load();
    const FTMOPAtlasEntry* Find(const FString& Id) const;
};
