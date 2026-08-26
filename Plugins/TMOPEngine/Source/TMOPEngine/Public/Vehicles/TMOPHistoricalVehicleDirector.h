#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Vehicles/TMOPHistoricalVehicleTypes.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "TMOPHistoricalVehicleDirector.generated.h"

class ATMOPVehicleBase;
class UDataTable;

/**
 * Loads DT_TMOP_HistoricalVehicles, reuses matching vehicles already placed in
 * the level, and spawns enabled historical vehicles at their initial transform.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPHistoricalVehicleDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPHistoricalVehicleDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles")
    TObjectPtr<UDataTable> HistoricalVehicleTable;

    /** Used when a row does not provide VehicleClass. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    TSubclassOf<ATMOPVehicleBase> DefaultVehicleClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bSpawnVehiclesAutomatically = true;

    /** Only rows enabled for simulation are considered for scheduled spawning. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bRespectRowSpawnFlags = true;

    /** Reuse level actors whose VehicleId matches a DataTable row. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning")
    bool bReusePlacedVehicles = true;

    /** Boundary vehicles appear only this many seconds before first driving. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="0", Units="s"))
    int32 EntrySpawnLeadSeconds = 10;

    /** Do not spawn a vehicle on top of another vehicle at an entry anchor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="50.0", Units="cm"))
    float EntrySpawnClearanceRadiusCm = 300.0f;

    /** Number of deterministic queue positions searched behind an occupied entry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="1", ClampMax="8"))
    int32 EntrySpawnQueueSlots = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="100.0", Units="cm"))
    float EntrySpawnQueueSpacingCm = 450.0f;

    /** Boundary-staged vehicles ignore vehicle collision until they have
     * physically cleared the shared entry area. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="100.0", Units="cm"))
    float EntryCollisionReleaseDistanceCm = 900.0f;

    /** Minimum driving time before boundary collision may be restored. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicles|Spawning",
        meta=(ClampMin="0.0", Units="s"))
    float EntryCollisionReleaseDelaySeconds = 2.0f;

    /** A completed Stop/Park may align by at most this distance. Larger
     * corrections are reported and left in place instead of visibly
     * teleporting a vehicle across the street or back along its route. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicles|Parking",
        meta=(ClampMin="0.0", Units="cm"))
    float TimedParkingAlignmentToleranceCm = 125.0f;

    /** Compatibility switch for old tables that intentionally used distant
     * Stop/Park entries as teleports. Keep false for historical simulation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicles|Parking")
    bool bAllowDistantTimedParkingTeleport = false;

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles")
    int32 InitializeHistoricalVehicles();

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles")
    int32 SpawnEnabledVehicles();

    /** Explicit staging/debug action which ignores bSpawnInSimulation. */
    UFUNCTION(CallInEditor, BlueprintCallable, Category="TMOP|Historical Vehicles|Debug")
    void SpawnAllVehiclesForStaging();

    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles|Validation")
    bool ValidateHistoricalVehicleTable(TArray<FString>& OutErrors) const;

    UFUNCTION(BlueprintPure, Category="TMOP|Historical Vehicles")
    ATMOPVehicleBase* FindHistoricalVehicle(FName VehicleId) const;

    /**
     * Starts a parked historical vehicle on a deterministic lane route.
     * The requesting person must occupy its driver seat.
     */
    UFUNCTION(BlueprintCallable, Category="TMOP|Historical Vehicles|Driving")
    bool BeginDrivingVehicle(
        FName VehicleId,
        FName DriverEntityId,
        const TArray<FName>& OrderedLaneIds,
        const TArray<FName>& PassAnchorIds,
        ETMOPVehicleRouteMode RouteMode =
            ETMOPVehicleRouteMode::ManualLaneRoute,
        FName DestinationAnchorId = NAME_None,
        float StartDistanceAlongFirstLaneCm = 0.0f);

    /** Returns the most recent detailed reason BeginDrivingVehicle rejected
     * this vehicle. The diagnostic is cleared when a new attempt succeeds. */
    bool GetLastDrivingFailure(
        FName VehicleId,
        FString& OutFailureCode,
        FString& OutFailureDetails) const;

private:
    struct FHistoricalVehicleRuntime
    {
        FName RowName = NAME_None;
        FTMOPHistoricalVehicleRow Profile;
        TWeakObjectPtr<ATMOPVehicleBase> Vehicle;
        bool bSpawnedByDirector = false;
        bool bDeferredPlacedVehicle = false;
        /** Last Spawn/InitialPlacement/Despawn entry whose lifecycle state was
         * applied. Keeping the entry index (instead of a permanent despawn
         * flag) allows one vehicle row to leave and later return repeatedly. */
        int32 LastAppliedLifecycleEntryIndex = INDEX_NONE;
        bool bBoundaryCollisionSuppressed = false;
        bool bBoundaryVehicleHasStartedDriving = false;
        float BoundaryDrivingSeconds = 0.0f;
        FVector BoundaryDrivingStartLocation = FVector::ZeroVector;
        TSet<FName> AppliedPlacementEntryIds;
        int32 InitialSpawnSecond = INDEX_NONE;
    };

    void DiscoverPlacedVehicles();
    int32 SpawnDueVehicles(int32 CurrentSecond);
    void DespawnDueVehicles(int32 CurrentSecond);
    void ApplyDueVehiclePlacements(int32 CurrentSecond);
    int32 GetInitialSpawnSecond(
        const FTMOPHistoricalVehicleRow& Profile) const;
    void ApplyDeferredPlacedVehicleState(int32 CurrentSecond);
    ATMOPVehicleBase* SpawnVehicle(FHistoricalVehicleRuntime& Runtime,
        const FTransform* SpawnTransformOverride = nullptr);
    FTransform GetInitialTransform(const FTMOPHistoricalVehicleRow& Profile) const;
    bool FindClearInitialSpawnTransform(
        const FTMOPHistoricalVehicleRow& Profile,
        FTransform& OutTransform) const;
    bool FindClearBoundarySpawnTransform(
        const FTransform& BaseTransform,
        FTransform& OutTransform) const;
    bool ResolveTimelinePlacementTransform(
        const FTMOPHistoricalVehicleTimelineEntry& Entry,
        FTransform& OutTransform) const;
    bool ResolveTimelineEntrySecond(
        const FTMOPHistoricalVehicleTimelineEntry& Entry,
        int32& OutSecond) const;
    bool IsBoundaryEntryVehicle(
        const FTMOPHistoricalVehicleRow& Profile) const;
    void SuppressBoundaryEntryCollision(
        FHistoricalVehicleRuntime& Runtime,
        bool bForceBoundaryEntry = false);
    void UpdateBoundaryEntryCollision(float DeltaSeconds);
    bool IsVehicleClearForCollisionRestore(
        const ATMOPVehicleBase* Vehicle) const;
    bool ShouldSpawn(const FTMOPHistoricalVehicleRow& Profile, bool bIgnoreRowFlag) const;
    void RegisterVehicle(ATMOPVehicleBase* Vehicle) const;
    void UnregisterVehicle(ATMOPVehicleBase* Vehicle) const;
    int32 SpawnVehicles(bool bIgnoreRowFlags);
    const FTMOPHistoricalVehicleTimelineEntry* FindDrivingEntry(
        const FTMOPHistoricalVehicleRow& Profile,
        FName DriverEntityId) const;
    bool ReportDrivingFailure(
        FName VehicleId,
        const FString& FailureCode,
        const FString& FailureDetails);

    TMap<FName, FHistoricalVehicleRuntime> RuntimeVehicles;
    TMap<FName, FString> LastDrivingFailureCodes;
    TMap<FName, FString> LastDrivingFailureDetails;
    int32 LastEvaluatedSecond = INDEX_NONE;
};
