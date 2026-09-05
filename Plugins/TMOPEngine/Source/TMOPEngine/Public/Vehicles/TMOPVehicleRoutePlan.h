#pragma once

#include "CoreMinimal.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"

class UWorld;
struct FHitResult;

/** Immutable plan shared by the editor, playback, seek and validation. Distances
 * include only the travelled parts of lanes and the approach to the anchor. */
struct TMOPENGINE_API FTMOPVehicleRoutePlan
{
    TArray<FName> LaneIds;
    float StartDistanceCm = 0.0f;
    float EndDistanceCm = 0.0f;
    FName StartAnchorId;
    FName EndAnchorId;
    bool bAnchorManeuver = false;
    bool bHasDestination = false;
    FTransform Destination = FTransform::Identity;
    TArray<FTransform> Anchors;
    TArray<FTransform> Samples;
    TArray<double> SampleDistances;
    TArray<double> ViaDistances;
    double LengthCm = 0.0;

    void AddSample(const FTransform& Pose);
    FTransform Sample(double DistanceCm) const;
};

namespace TMOPVehicleRoute
{
    TMOPENGINE_API bool IsDriving(ETMOPHistoricalVehicleAction Action);
    TMOPENGINE_API bool IsStop(ETMOPHistoricalVehicleAction Action);
    TMOPENGINE_API bool HasPlacement(ETMOPHistoricalVehicleAction Action);
    TMOPENGINE_API int32 CompletionDelay(const FTMOPHistoricalVehicleTimelineEntry& Entry);
    TMOPENGINE_API FName Driver(const FTMOPHistoricalVehicleRow& Row,
        const FTMOPHistoricalVehicleTimelineEntry& Entry);

    /** RebuildManual is used only by the explicit Recalculate command. An
     * invalid stored manual route fails instead of silently taking another road. */
    TMOPENGINE_API bool Build(UWorld* World, const FTMOPHistoricalVehicleRow& Row,
        int32 Index, FTMOPVehicleRoutePlan& Out, FString& Failure,
        bool bRebuildManual = false);
    TMOPENGINE_API void BuildManeuver(const TArray<FTransform>& Anchors,
        float Strength, bool bReverse, ETMOPVehicleManeuverTurn Turn,
        float RadiusCm, FTMOPVehicleRoutePlan& Out);
    TMOPENGINE_API double DistanceAtTime(const FTMOPVehicleRoutePlan& Plan,
        double Alpha, bool bStopAtViaAnchors = false);
    TMOPENGINE_API FString Fingerprint(const FTMOPHistoricalVehicleRow& Row,
        const FTMOPVehicleRoutePlan& Plan, int32 Departure, int32 Arrival);
    TMOPENGINE_API FName UniqueEntryId(const FTMOPHistoricalVehicleRow& Row,
        const FString& Base);
    /** Non-mutating, vehicle-sized world-static sweep for authored maneuvers. */
    TMOPENGINE_API bool FindObstacle(UWorld* World,
        const FTMOPVehicleRoutePlan& Plan, FVector HalfExtent,
        FHitResult& Hit, const AActor* IgnoreActor = nullptr);
}
