#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Observations/TMOPObservationTypes.h"
#include "TMOPObservationDirector.generated.h"

class UDataTable;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(
    FTMOPObservationEvaluatedSignature,
    FName,
    ObservationId,
    ETMOPObservationRuntimeState,
    Result);

/**
 * Loads observation data, resolves canonical times and checks whether source
 * observations occur. It deliberately never changes actor movement or timing.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPObservationDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPObservationDirector();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Observations|Tables")
    TObjectPtr<UDataTable> ObservationTable = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="TMOP|Observations|Tables")
    TObjectPtr<UDataTable> ObservationLinkTable = nullptr;

    /** Inline rows override table rows with the same ID. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observations")
    TArray<FTMOPObservationDefinition> ObservationDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observations")
    TArray<FTMOPObservationLinkDefinition> ObservationLinks;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|Linked Tracks")
    bool bEnableLinkedTrackSimulation = true;

    /** Draws a red world-space beacon 1000 metres above Olof Palme. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides")
    bool bShowOlofLocationLine = true;

    /** Draws observer-to-subject lines only during each observation window. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides")
    bool bShowActiveObservationLines = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides")
    FName OlofPalmeEntityId = TEXT("OLOF_PALME");

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides",
        meta=(ClampMin="100.0", Units="cm"))
    float OlofLocationLineHeightCm = 100000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides")
    FLinearColor OlofLocationLineColor = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides")
    FLinearColor ObservationLineColor = FLinearColor(1.0f, 0.65f, 0.05f, 0.45f);

    /** Thickness of the vertical Olof beacon. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides",
        meta=(ClampMin="0.1"))
    float WorldGuideLineThickness = 3.0f;

    /** Thin observer-to-subject line, separate from the Olof beacon. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides",
        meta=(ClampMin="0.1"))
    float ObservationLineThickness = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observations|World Guides",
        meta=(ClampMin="5.0", Units="cm"))
    float ObservationArrowSizeCm = 50.0f;

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations|World Guides")
    void SetShowOlofLocationLine(bool bShow);

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations|World Guides")
    void SetShowActiveObservationLines(bool bShow);

    /** Per-user values used by the pause-menu settings, including before this
     * director has spawned in the gameplay world. */
    static bool GetSavedShowOlofLocationLine();
    static bool GetSavedShowActiveObservationLines();
    static void SaveShowOlofLocationLine(bool bShow);
    static void SaveShowActiveObservationLines(bool bShow);

    UPROPERTY(BlueprintAssignable, Category="TMOP|Observations")
    FTMOPObservationEvaluatedSignature OnObservationEvaluated;

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    int32 ReloadObservationData();

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    int32 ResolveCanonicalTimes();

    /** Sorts every link's observation array by resolved canonical time and
     * builds one interpolation segment per consecutive pair. */
    UFUNCTION(BlueprintCallable, Category="TMOP|Observations|Linked Tracks")
    int32 RebuildObservationTracks();

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    void ResetObservationRuntime();

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    bool EvaluateObservationNow(FName ObservationId);

    UFUNCTION(BlueprintPure, Category="TMOP|Observations")
    bool TryGetObservationDefinition(
        FName ObservationId,
        FTMOPObservationDefinition& OutDefinition) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Observations")
    bool TryGetObservationRuntime(
        FName ObservationId,
        FTMOPObservationRuntime& OutRuntime) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Observations")
    TArray<FTMOPObservationRuntime> GetAllObservationRuntime() const;

    UFUNCTION(BlueprintPure, Category="TMOP|Observations|Linked Tracks")
    bool TryGetObservationTrackRuntime(
        FName LinkId,
        FTMOPObservationTrackRuntime& OutRuntime) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Observations|Linked Tracks")
    TArray<FTMOPObservationTrackRuntime> GetAllObservationTrackRuntime() const;

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations|World Bake")
    int32 ApplyBakedObservationRuntime(
        const TArray<FTMOPObservationRuntime>& BakedRuntime);

    UFUNCTION(BlueprintCallable, Category="TMOP|Observations")
    bool ValidateObservationData(TArray<FString>& OutErrors) const;

private:
    struct FResolvedTrackSegment;

    UFUNCTION()
    void HandleSecondChanged(FTMOPTime NewTime);

    UFUNCTION()
    void HandleLoopRestarted(int32 NewLoopNumber, FTMOPTime RestartTime);

    bool ResolveCanonicalTime(FName ObservationId);
    bool EvaluateGeometry(
        const FTMOPObservationDefinition& Definition,
        FTMOPObservationRuntime& Runtime) const;
    AActor* FindObservedActorForObservation(
        const FTMOPObservationDefinition& Definition) const;
    bool ApplySegmentTransition(
        const FTMOPObservationLinkDefinition& Link,
        FResolvedTrackSegment& Segment,
        FTMOPObservationTrackRuntime& Runtime,
        double CurrentSecond);
    AActor* FindEntityActor(FName EntityId) const;
    TArray<FName> GetLinkObservationIds(
        const FTMOPObservationLinkDefinition& Link) const;
    void UpdateObservationTracks(double CurrentSecond);
    void DrawWorldGuideLines(int32 CurrentSecond) const;

    struct FResolvedTrackPoint
    {
        FName ObservationId = NAME_None;
        FName AnchorId = NAME_None;
        int32 StartSecond = INDEX_NONE;
        int32 EndSecond = INDEX_NONE;
        FVector Location = FVector::ZeroVector;
    };

    struct FResolvedTrackSegment
    {
        FName FromObservationId = NAME_None;
        FName ToObservationId = NAME_None;
        double TravelStartSecond = -1.0;
        double TravelEndSecond = -1.0;
        TArray<FVector> PolylinePoints;
        TArray<float> CumulativeDistancesCm;
        float DistanceCm = 0.0f;
        float RequiredSpeedCmPerSecond = 0.0f;
        ETMOPObservationSegmentMovementMode MovementMode =
            ETMOPObservationSegmentMovementMode::Automatic;
        FName VehicleEntityId = NAME_None;
        FName VehicleSeatId = NAME_None;
        float MaximumBoardingDistanceCm = 600.0f;
        FName DriverEntityId = NAME_None;
        TArray<FName> OrderedLaneIds;
        TArray<FName> VehiclePassAnchorIds;
        ETMOPVehicleRouteMode VehicleRouteMode =
            ETMOPVehicleRouteMode::ManualLaneRoute;
        FName VehicleDestinationAnchorId = NAME_None;
        float VehicleStartDistanceAlongFirstLaneCm = 0.0f;
        float VehicleCruiseSpeedKmh = 0.0f;
        bool bIgnoreOneWayRestrictions = false;
        bool bRunRedLights = false;
        bool bStartTransitionApplied = false;
        bool bEndTransitionApplied = false;
        bool bVehicleRouteStarted = false;
    };

    struct FResolvedTrack
    {
        TArray<FResolvedTrackPoint> Points;
        TArray<FResolvedTrackSegment> Segments;
        TWeakObjectPtr<AActor> ControlledActor;
        bool bCollisionSuppressedByTrack = false;
        bool bActorCollisionWasEnabled = false;
    };

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationDefinition> LoadedObservations;

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationLinkDefinition> LoadedLinks;

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationRuntime> RuntimeObservations;

    UPROPERTY(Transient)
    TMap<FName, FTMOPObservationTrackRuntime> RuntimeTracks;

    TMap<FName, FResolvedTrack> ResolvedTracks;
    TMap<FName, FName> ObservationPlaybackEntities;
    mutable TMap<FName, TWeakObjectPtr<AActor>> EntityActorCache;
};
