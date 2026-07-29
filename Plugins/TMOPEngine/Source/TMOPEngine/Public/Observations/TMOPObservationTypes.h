#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "Time/TMOPTime.h"
#include "TMOPObservationTypes.generated.h"

UENUM(BlueprintType)
enum class ETMOPObservedEntityType : uint8
{
    Person,
    Vehicle,
    Unknown
};

UENUM(BlueprintType)
enum class ETMOPObservationTimingMode : uint8
{
    Absolute UMETA(DisplayName="Absolute canonical time"),
    RelativeToSharedEvent UMETA(DisplayName="Relative to shared event")
};

UENUM(BlueprintType)
enum class ETMOPObservationRuntimeState : uint8
{
    Pending,
    Observed,
    Missed,
    Invalid
};

UENUM(BlueprintType)
enum class ETMOPObservationRelationship : uint8
{
    PossibleSamePerson,
    ProbableSamePerson,
    ConfirmedSamePerson,
    PossibleSameVehicle,
    ProbableSameVehicle,
    ConfirmedSameVehicle,
    RelatedSameScene,
    Rejected
};

/**
 * One sourced witness observation. This row records the observation and checks
 * whether it occurs; it never moves either the observer or the observed actor.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation")
    FName ObservationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation")
    bool bEnabled = true;

    /** One or more people who independently share this observation window. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Entities")
    TArray<FName> ObserverEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Entities")
    FName ObservedEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Entities")
    ETMOPObservedEntityType ObservedEntityType =
        ETMOPObservedEntityType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Place")
    FName ObservationAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Timing")
    ETMOPObservationTimingMode TimingMode =
        ETMOPObservationTimingMode::Absolute;

    /** Authored fallback and resolved time for absolute observations. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Timing")
    FTMOPTime CanonicalTime;

    /**
     * Optional major historical event used only to calculate canonical time.
     * Runtime movement remains independent after the time has been resolved.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Timing")
    FName ReferenceSharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Timing")
    int32 ReferenceOffsetSeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Timing",
        meta=(ClampMin="1", Units="s"))
    int32 ObservationDurationSeconds = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Geometry",
        meta=(ClampMin="0.0", Units="cm"))
    float ObservationRadiusCm = 1800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Geometry")
    bool bRequireObserverNearAnchor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Geometry")
    bool bRequireObservedEntityNearAnchor = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Geometry")
    bool bRequiresLineOfSight = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence",
        meta=(MultiLine="true"))
    FString ObservedDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence",
        meta=(MultiLine="true"))
    FString Notes;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    FName ObservationId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    ETMOPObservationRuntimeState State =
        ETMOPObservationRuntimeState::Pending;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    FTMOPTime ResolvedCanonicalStartTime;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    FTMOPTime ResolvedCanonicalEndTime;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    bool bHasResolvedCanonicalTime = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    FName SuccessfulObserverEntityId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    float ObserverDistanceToAnchorCm = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    float ObservedDistanceToAnchorCm = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation")
    FString Diagnostic;
};

/** A hypothesis connecting two immutable source observations. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationRouteAlternative
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route")
    FName RouteId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route")
    TArray<FName> RouteAnchorIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route",
        meta=(ClampMin="0.0", Units="cm"))
    float CalculatedDistanceCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route",
        meta=(ClampMin="0.0", Units="cm/s"))
    float RequiredAverageSpeedCmPerSecond = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float ConfidenceScore = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route",
        meta=(MultiLine="true"))
    FString Notes;
};

/**
 * A hypothesis connecting two immutable observations. The main interpolated
 * route and any alternatives live here; no separate inferred-track table is
 * required.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationLinkDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    FName LinkId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    FName FromObservationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    FName ToObservationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    ETMOPObservationRelationship Relationship =
        ETMOPObservationRelationship::PossibleSamePerson;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float ConfidenceScore = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    TArray<FName> SupportingFactors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    TArray<FName> ContradictingFactors;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route")
    TArray<FName> RouteAnchorIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route",
        meta=(ClampMin="0.0", Units="cm"))
    float CalculatedDistanceCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route",
        meta=(ClampMin="0.0", Units="cm/s"))
    float RequiredAverageSpeedCmPerSecond = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Route")
    TArray<FTMOPObservationRouteAlternative> AlternativeRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    bool bPlayerCreatedHypothesis = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    bool bVisibleOnlyInInvestigationMode = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link",
        meta=(MultiLine="true"))
    FString Notes;
};
