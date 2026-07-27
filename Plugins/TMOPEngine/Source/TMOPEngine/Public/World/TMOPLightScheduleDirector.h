#pragma once

#include "CoreMinimal.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "GameFramework/Actor.h"
#include "Time/TMOPTime.h"
#include "TMOPLightScheduleDirector.generated.h"

class ULightComponent;

UENUM(BlueprintType)
enum class ETMOPLightScheduleAction : uint8
{
    TurnOn UMETA(DisplayName="Turn On"),
    TurnOff UMETA(DisplayName="Turn Off"),
    Toggle,
    SetIntensity UMETA(DisplayName="Set Intensity")
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPLightScheduleEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule")
    FName EntryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Time")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Time")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative",
            DisplayName="Shared Event ID"))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative || TimingMode==ETMOPEventTimingMode::RelativeToPreviousEntry",
            DisplayName="Offset Seconds"))
    int32 OffsetSeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule")
    ETMOPLightScheduleAction Action = ETMOPLightScheduleAction::TurnOn;

    /** Explicit level actors containing one or more Light Components. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Targets")
    TArray<TSoftObjectPtr<AActor>> TargetLightActors;

    /** Also affects every loaded actor carrying this Actor Tag. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Targets")
    FName TargetActorTag = NAME_None;

    /** Used by Set Intensity, or as the Turn On intensity when enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Light",
        meta=(ClampMin="0.0"))
    float TargetIntensity = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Light",
        meta=(EditCondition="Action==ETMOPLightScheduleAction::TurnOn"))
    bool bOverrideTurnOnIntensity = false;

    /** Zero changes immediately. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Light",
        meta=(ClampMin="0.0", Units="s"))
    float FadeSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule|Source")
    FString Notes;
};

/** Drives selected level lights from the TMOP clock and Shared Events. */
UCLASS(Blueprintable)
class TMOPENGINE_API ATMOPLightScheduleDirector : public AActor
{
    GENERATED_BODY()

public:
    ATMOPLightScheduleDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Light Schedule",
        meta=(TitleProperty="EntryId"))
    TArray<FTMOPLightScheduleEntry> ScheduledEntries;

    UFUNCTION(BlueprintCallable, Category="TMOP|Light Schedule")
    void RestartScheduleAtCurrentTime();

private:
    struct FLightTransition
    {
        TWeakObjectPtr<ULightComponent> Light;
        float StartIntensity = 0.0f;
        float TargetIntensity = 0.0f;
        float Elapsed = 0.0f;
        float Duration = 0.0f;
        bool bTurnOffAtEnd = false;
    };

    void EvaluateSchedule(int32 CurrentSecond, bool bCatchUp);
    bool ResolveEntrySecond(
        const FTMOPLightScheduleEntry& Entry, int32& OutSecond) const;
    void ApplyEntry(const FTMOPLightScheduleEntry& Entry, bool bCatchUp);
    void GatherTargetLights(
        const FTMOPLightScheduleEntry& Entry,
        TArray<ULightComponent*>& OutLights);
    void UpdateTransitions(float DeltaSeconds);
    void RestoreBaseline();

    int32 NextEntryIndex = 0;
    int32 LastResolvedEntrySecond = INDEX_NONE;
    int32 LastEvaluatedSecond = INDEX_NONE;
    TMap<TWeakObjectPtr<ULightComponent>, float> OriginalIntensities;
    TMap<TWeakObjectPtr<ULightComponent>, bool> OriginalVisibility;
    TArray<FLightTransition> ActiveTransitions;
};
