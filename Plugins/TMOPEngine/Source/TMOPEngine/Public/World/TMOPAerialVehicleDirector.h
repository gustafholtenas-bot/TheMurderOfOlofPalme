#pragma once

#include "CoreMinimal.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "TMOPAerialVehicleDirector.generated.h"

class USplineComponent;

UENUM(BlueprintType)
enum class ETMOPAerialScheduleAction : uint8
{
    SpawnFlight UMETA(DisplayName="Spawn Flight"),
    Despawn
};

UENUM(BlueprintType)
enum class ETMOPAerialEndBehavior : uint8
{
    DespawnAtEnd UMETA(DisplayName="Despawn At End"),
    HoldAtEnd UMETA(DisplayName="Hold At End"),
    Loop
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAerialScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial")
    FName EntryId = NAME_None;

    /** Spawn and Despawn entries controlling the same aircraft use this ID. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial")
    FName InstanceId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial")
    ETMOPAerialScheduleAction Action =
        ETMOPAerialScheduleAction::SpawnFlight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Time")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Time")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative",
            DisplayName="Shared Event ID"))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative || TimingMode==ETMOPEventTimingMode::RelativeToPreviousEntry",
            DisplayName="Offset Seconds"))
    int32 OffsetSeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight"))
    TSubclassOf<AActor> AircraftClass;

    /** Level actor containing the Spline Component used as the flight path. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight"))
    TSoftObjectPtr<AActor> SplineActor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight",
            ClampMin="0.0", Units="km/h"))
    float SpeedKmh = 180.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight",
            ClampMin="0.0", Units="cm"))
    float StartDistanceCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight"))
    bool bReverseDirection = false;

    /** Corrects aircraft assets whose local forward axis is not +X. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight"))
    FRotator RotationOffset = FRotator::ZeroRotator;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Flight",
        meta=(EditCondition="Action==ETMOPAerialScheduleAction::SpawnFlight"))
    ETMOPAerialEndBehavior EndBehavior =
        ETMOPAerialEndBehavior::DespawnAtEnd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial|Source")
    FString Notes;
};

/**
 * Spawns aircraft on scheduled TMOP times and moves them along level splines.
 * Scheduled entries must be stored in chronological order.
 */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPAerialVehicleDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPAerialVehicleDirector();
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Aerial",
        meta=(TitleProperty="EntryId"))
    TArray<FTMOPAerialScheduleEntry> ScheduledFlights;

    UFUNCTION(BlueprintCallable, Category="TMOP|Aerial")
    void RestartScheduleAtCurrentTime();

    UFUNCTION(BlueprintPure, Category="TMOP|Aerial")
    AActor* FindAircraft(FName InstanceId) const;

private:
    struct FActiveFlight
    {
        TWeakObjectPtr<AActor> Aircraft;
        TWeakObjectPtr<USplineComponent> Spline;
        float DistanceCm = 0.0f;
        float SpeedCmPerSecond = 0.0f;
        bool bReverseDirection = false;
        FRotator RotationOffset = FRotator::ZeroRotator;
        ETMOPAerialEndBehavior EndBehavior =
            ETMOPAerialEndBehavior::DespawnAtEnd;
    };

    void EvaluateSchedule(int32 CurrentSecond, bool bCatchUp);
    bool ResolveEntrySecond(
        const FTMOPAerialScheduleEntry& Entry, int32& OutSecond) const;
    bool ApplyEntry(
        const FTMOPAerialScheduleEntry& Entry,
        int32 ResolvedSecond,
        int32 CurrentSecond);
    bool SpawnFlight(
        const FTMOPAerialScheduleEntry& Entry,
        int32 ResolvedSecond,
        int32 CurrentSecond);
    void DespawnFlight(FName InstanceId);
    void UpdateFlights(float SimulationDeltaSeconds);
    bool ApplyFlightTransform(FActiveFlight& Flight);
    void DestroyAllAircraft();

    int32 NextEntryIndex = 0;
    int32 LastResolvedEntrySecond = INDEX_NONE;
    int32 LastEvaluatedSecond = INDEX_NONE;
    TMap<FName, FActiveFlight> ActiveFlights;
};
