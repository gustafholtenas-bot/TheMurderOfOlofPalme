#pragma once

#include "CoreMinimal.h"
#include "Observations/TMOPNotebookTypes.h"

class AActor;
class UWorld;
struct FTMOPPersonProfileRow;

/** Builds a self-contained presentation snapshot for an already eligible observation. */
struct TMOPENGINE_API FTMOPNotebookPresentation
{
    static void Populate(FTMOPNotebookObservation& Entry, UWorld* World,
        AActor* Actor = nullptr, const FTMOPPersonProfileRow* Profile = nullptr, bool bCaptureModel = true, bool bLocalizeView = false);
    /** Rebuild a translated display copy; never write translated prose into the save snapshot. */
    static FTMOPNotebookObservation LocalizedView(const FTMOPNotebookObservation& Source, UWorld* World);
    static void CollectLocations(FTMOPNotebookObservation& Entry, UWorld* World);
    static void RecordPlayerSighting(FTMOPNotebookObservation& Entry, UWorld* World, FVector Position, int32 Second);
    static bool CaptureModel(AActor* Actor, ETMOPNotebookEntityKind Kind, TArray<uint8>& OutPng);
};
