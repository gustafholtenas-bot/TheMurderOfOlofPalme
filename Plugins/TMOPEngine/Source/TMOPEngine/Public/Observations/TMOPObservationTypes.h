#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "Time/TMOPTime.h"
#include "Traffic/TMOPTrafficTypes.h"
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

/** How a multi-observation hypothesis affects the running world. */
UENUM(BlueprintType)
enum class ETMOPObservationTrackSimulationMode : uint8
{
    /** Evidence/research only. */
    Disabled,
    /** Build and validate the track without moving an actor. */
    ValidateOnly,
    /** Move an existing ObservedUnknown person/vehicle along the inferred route. */
    InterpolateExistingActor
};

/** How the free time between two observations is converted into movement. */
UENUM(BlueprintType)
enum class ETMOPObservationTravelTimingMode : uint8
{
    /** Move continuously during the complete interval between observations. */
    UseEntireAvailableWindow UMETA(DisplayName="Use entire available window"),
    /** Wait at the old observation, then travel at the preferred speed and arrive exactly on time. */
    ArriveAtPreferredSpeed UMETA(DisplayName="Wait, then arrive at preferred speed")
};

/** Physical behaviour used between two consecutive observations. */
UENUM(BlueprintType)
enum class ETMOPObservationSegmentMovementMode : uint8
{
    /** Infer walking or lane driving from entity type and authored route data. */
    Automatic,
    Walk,
    Run,
    Sprint,
    VehicleLaneRoute UMETA(DisplayName="Vehicle lane route"),
    VehicleDirectInterpolation UMETA(DisplayName="Vehicle direct interpolation (no lanes)"),
    WalkToVehicleAndBoard UMETA(DisplayName="Walk to vehicle and board"),
    RunToVehicleAndBoard UMETA(DisplayName="Run to vehicle and board"),
    RideInVehicle UMETA(DisplayName="Remain seated / ride in vehicle"),
    ExitVehicleThenWalk UMETA(DisplayName="Exit vehicle, then walk"),
    ExitVehicleThenRun UMETA(DisplayName="Exit vehicle, then run"),
    HoldPosition UMETA(DisplayName="Hold position")
};

UENUM(BlueprintType)
enum class ETMOPObservationTrackRuntimeState : uint8
{
    Unresolved,
    WaitingForFirstObservation,
    AtObservation,
    Interpolating,
    Completed,
    Invalid
};

/** One comparable feature in a witness's description of the observed person. */
UENUM(BlueprintType)
enum class ETMOPSignalementTraitType : uint8
{
    SexOrGenderImpression,
    Age,
    Height,
    BodyBuild,
    HairColor,
    HairStyle,
    FacialHair,
    FaceShape,
    FacialFeature,
    SkinToneOrComplexion,
    Headwear,
    Outerwear,
    UpperBodyClothing,
    Trousers,
    Footwear,
    Accessory,
    CarriedObject,
    GaitOrMovement,
    VoiceOrAccent,
    Other
};

/** Source wording plus normalized values; exclusions preserve negative evidence. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservedSignalementTrait
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    ETMOPSignalementTraitType TraitType = ETMOPSignalementTraitType::Other;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(MultiLine="true"))
    FString OriginalText;

    /** Stable lowercase tags such as dark_blue_coat, knit_cap or broad_shoulders. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    TArray<FName> NormalizedValues;

    /** Explicit source negatives such as no_hat or not_blue_jacket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    TArray<FName> ExplicitlyExcludedValues;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    FString SourceReference;
};

/** A signalement remains attributed to one witness instead of being flattened across sources. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationWitnessSignalement
{
    GENERATED_BODY()

    /** Empty is allowed only for a genuinely collective/unknown witness account. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    FName ObserverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(ClampMin="0", ClampMax="120"))
    int32 EstimatedAgeMinimum = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(ClampMin="0", ClampMax="120"))
    int32 EstimatedAgeMaximum = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(ClampMin="0.0", ClampMax="230.0", Units="cm"))
    float EstimatedHeightMinimumCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(ClampMin="0.0", ClampMax="230.0", Units="cm"))
    float EstimatedHeightMaximumCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(TitleProperty="TraitType"))
    TArray<FTMOPObservedSignalementTrait> Traits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(MultiLine="true"))
    FString OriginalSummary;

    /** Lighting, distance, viewing angle, duration and other reliability limits. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(MultiLine="true"))
    FString ObservationConditions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    FString SourceReference;
};

USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPSignalementComparison
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation|Signalement")
    bool bHasComparableEvidence = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation|Signalement",
        meta=(ClampMin="0.0", ClampMax="1.0"))
    float CompatibilityScore = 0.5f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation|Signalement")
    TArray<FString> SupportingTraits;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation|Signalement")
    TArray<FString> ContradictingTraits;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation|Signalement")
    FString Summary;
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

    /**
     * Allows a reconstructed/legacy observation whose original observer has
     * not yet been identified. Normal sourced observations should keep this
     * false and provide at least one ObserverEntityId.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Entities")
    bool bAllowUnattributedObservation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Evidence",
        meta=(MultiLine="true"))
    FString ObservedDescription;

    /** Per-witness appearance evidence used by ObservationLink matching. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement",
        meta=(TitleProperty="ObserverEntityId"))
    TArray<FTMOPObservationWitnessSignalement> WitnessSignalements;

    /** Set after the cited documents have been checked for signalement details. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    bool bSignalementSourceReviewed = false;

    /** Distinguishes a reviewed source with no usable description from missing data. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation|Signalement")
    bool bNoFurtherSignalementInSource = false;

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

/** Authored route between two members of a multi-observation track. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationTrackSegment
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track")
    FName FromObservationId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track")
    FName ToObservationId = NAME_None;

    /** Optional anchors visited between the two immutable observation anchors. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track")
    TArray<FName> RouteAnchorIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track")
    TArray<FTMOPObservationRouteAlternative> AlternativeRoutes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Movement")
    ETMOPObservationSegmentMovementMode MovementMode =
        ETMOPObservationSegmentMovementMode::Automatic;

    /** Vehicle boarded, ridden in, exited from, or driven on this leg. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    FName VehicleEntityId = NAME_None;

    /** Exact seat when known; empty selects the first available passenger seat. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    FName VehicleSeatId = NAME_None;

    /** Finish the approach before the next observation so boarding can complete visibly. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle",
        meta=(ClampMin="0.0", Units="s"))
    float BoardingLeadSeconds = 0.0f;

    /** Refuse boarding beyond this distance instead of teleporting the person. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle",
        meta=(ClampMin="0.0", Units="cm"))
    float MaximumBoardingDistanceCm = 600.0f;

    /** Required when this leg starts the linked vehicle through the traffic system. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    FName DriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    TArray<FName> OrderedLaneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    TArray<FName> VehiclePassAnchorIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    ETMOPVehicleRouteMode VehicleRouteMode =
        ETMOPVehicleRouteMode::ManualLaneRoute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    FName VehicleDestinationAnchorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle",
        meta=(ClampMin="0.0", Units="cm"))
    float VehicleStartDistanceAlongFirstLaneCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle",
        meta=(ClampMin="0.0", Units="km/h"))
    float VehicleCruiseSpeedKmh = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    bool bIgnoreOneWayRestrictions = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Vehicle")
    bool bRunRedLights = false;

    /** Overrides the link-level preferred speed for this one leg. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Track|Movement",
        meta=(ClampMin="0.0", Units="cm/s"))
    float PreferredTravelSpeedCmPerSecond = 0.0f;

    /** Zero lets the director calculate the polyline distance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track",
        meta=(ClampMin="0.0", Units="cm"))
    float AuthoredDistanceCm = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track",
        meta=(MultiLine="true"))
    FString Notes;
};

/** Runtime diagnostic for one resolved multi-observation hypothesis. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationTrackRuntime
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FName LinkId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FName LinkedEntityId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    ETMOPObservationTrackRuntimeState State =
        ETMOPObservationTrackRuntimeState::Unresolved;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    TArray<FName> OrderedObservationIds;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    int32 CurrentSegmentIndex = INDEX_NONE;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FName CurrentFromObservationId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FName CurrentToObservationId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    float SegmentAlpha = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    float CurrentRequiredSpeedCmPerSecond = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    ETMOPObservationSegmentMovementMode CurrentMovementMode =
        ETMOPObservationSegmentMovementMode::Automatic;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FName CurrentVehicleEntityId = NAME_None;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    bool bPlaybackActorSeated = false;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    float MaximumRequiredSpeedCmPerSecond = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    bool bPhysicallyPlausible = true;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FVector InferredLocation = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="TMOP|Observation Link|Runtime")
    FString Diagnostic;
};

/**
 * A hypothesis clustering any number of immutable source observations.
 * ObservationIds is the authoritative member array. The legacy From/To fields
 * remain as an automatic two-member migration path for old DataTables.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPObservationLinkDefinition : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    FName LinkId = NAME_None;

    /** All observations believed to describe the same entity or phenomenon. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link",
        meta=(DisplayName="Observation IDs"))
    TArray<FName> ObservationIds;

    /**
     * Existing known witness/person/vehicle or an OBSERVED_UNKNOWN entity that
     * represents the whole cluster. Empty means the link is research-only.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    FName LinkedEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    ETMOPObservedEntityType LinkedEntityType =
        ETMOPObservedEntityType::Unknown;

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

    /**
     * Validate every consecutive pair against observation-specific witness
     * signalements. Disabled by default so legacy links without reviewed
     * descriptions remain usable.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Signalement")
    bool bRequireSignalementCompatibility = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Observation Link|Signalement",
        meta=(ClampMin="0.0", ClampMax="1.0",
            EditCondition="bRequireSignalementCompatibility"))
    float MinimumSignalementCompatibility = 0.35f;

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

    /** Optional per-leg routes. Missing legs use direct anchor interpolation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Track")
    TArray<FTMOPObservationTrackSegment> TrackSegments;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation")
    ETMOPObservationTrackSimulationMode SimulationMode =
        ETMOPObservationTrackSimulationMode::ValidateOnly;

    /**
     * Preferred mode for sparse sightings. It avoids making an actor crawl
     * slowly for the entire gap merely because the next sighting is later.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation")
    ETMOPObservationTravelTimingMode TravelTimingMode =
        ETMOPObservationTravelTimingMode::ArriveAtPreferredSpeed;

    /** Zero selects a movement-mode default (140 walk, 350 run, 600 sprint, 1200 vehicle cm/s). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation",
        meta=(ClampMin="0.0", Units="cm/s"))
    float PreferredTravelSpeedCmPerSecond = 0.0f;

    /** When no route anchors are authored, people follow the navigation path. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation")
    bool bUseNavigationPathForPeople = true;

    /** Evidence actors should normally be non-blocking ghost representations. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation")
    bool bDisableCollisionWhileInterpolating = true;

    /** Zero selects 800 cm/s for people and 5000 cm/s for vehicles. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation",
        meta=(ClampMin="0.0", Units="cm/s"))
    float MaximumPlausibleSpeedCmPerSecond = 0.0f;

    /** Never move a confirmed known witness unless this is deliberately enabled. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link|Simulation")
    bool bAllowMovementOfKnownEntity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    bool bPlayerCreatedHypothesis = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link")
    bool bVisibleOnlyInInvestigationMode = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Observation Link",
        meta=(MultiLine="true"))
    FString Notes;
};
