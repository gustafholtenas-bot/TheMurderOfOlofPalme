#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "Vehicles/TMOPVehicleCatalogTypes.h"
#include "UI/TMOPEntityLabelTypes.h"
#include "TMOPHistoricalVehicleTypes.generated.h"

class AActor;
class UTMOPVehicleModelData;

UENUM(BlueprintType)
enum class ETMOPHistoricalVehicleAction : uint8
{
    InitialPlacement,
    Spawn,
    Despawn,
    BeginDriving,
    Stop,
    Park,
    EnterTrafficRoute,
    ExitTrafficRoute,
    Custom,
    EmergencySirenOn,
    EmergencySirenOff,
    /** Hide the vehicle and its seated occupants, move it to a new
     * placement, then reveal it after OffscreenTransferDurationSeconds. */
    OffscreenTransfer UMETA(DisplayName="Offscreen Transfer")
};

UENUM(BlueprintType)
enum class ETMOPHistoricalVehiclePlacementMode : uint8
{
    WorldTransform,
    Anchor
};

UENUM(BlueprintType)
enum class ETMOPVehicleManeuverTurn : uint8
{
    Automatic,
    Left,
    Right
};

/** Reusable driving behaviour; Custom Speed Override always has priority. */
UENUM(BlueprintType)
enum class ETMOPVehicleDrivingPreset : uint8
{
    AutomaticFromTimeline UMETA(DisplayName="Automatic From Timeline"),
    Parking UMETA(DisplayName="Parking / Crawl"),
    SlowCity UMETA(DisplayName="Slow City Traffic"),
    NormalCity UMETA(DisplayName="Normal City Traffic"),
    Fast UMETA(DisplayName="Fast"),
    Emergency UMETA(DisplayName="Emergency"),
    Fleeing UMETA(DisplayName="Fleeing")
};

/** One source-backed state or action in a historical vehicle's schedule. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalVehicleTimelineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    FName EntryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    ETMOPHistoricalVehicleAction Action = ETMOPHistoricalVehicleAction::InitialPlacement;

    /** Friendly description such as "Grand to crime scene". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            DisplayName="Route Segment Name", EditConditionHides))
    FText RouteSegmentName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    /**
     * Absolute uses Time. Relative to Shared Event resolves SharedEventId.
     * Relative to Previous Entry uses the resolved time of the timeline item
     * immediately above this one. Both relative modes add EventOffsetSeconds.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Time",
        meta=(ValidEnumValues="Absolute,Relative,RelativeToPreviousEntry"))
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative", EditConditionHides))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative || TimingMode==ETMOPEventTimingMode::RelativeToPreviousEntry",
            DisplayName="Offset Seconds", EditConditionHides))
    int32 EventOffsetSeconds = 0;

    /**
     * For a driving entry, the main Time fields describe planned arrival.
     * Departure can be set explicitly below or inherited from the preceding
     * timeline entry when Set Departure Time On This Row is disabled.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Time",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            DisplayName="Time Is Arrival", EditConditionHides))
    bool bTimeIsArrival = false;

    /**
     * When Time Is Arrival is enabled, expose a separate departure time on
     * this same driving row. When disabled, the preceding entry (or an
     * Offscreen Transfer's reveal time) remains the departure.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Departure Time",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && bTimeIsArrival",
            DisplayName="Set Departure Time On This Row", EditConditionHides))
    bool bUseExplicitDepartureTime = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Departure Time",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && bTimeIsArrival && bUseExplicitDepartureTime",
            ValidEnumValues="Absolute,Relative,RelativeToPreviousEntry",
            DisplayName="Departure Timing Mode", EditConditionHides))
    ETMOPEventTimingMode DepartureTimingMode =
        ETMOPEventTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Departure Time",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && bTimeIsArrival && bUseExplicitDepartureTime && DepartureTimingMode==ETMOPEventTimingMode::Absolute",
            DisplayName="Departure Time", EditConditionHides))
    FTMOPTime DepartureTime = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Departure Time",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && bTimeIsArrival && bUseExplicitDepartureTime && DepartureTimingMode==ETMOPEventTimingMode::Relative",
            DisplayName="Departure Shared Event ID", EditConditionHides))
    FName DepartureSharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Departure Time",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && bTimeIsArrival && bUseExplicitDepartureTime && (DepartureTimingMode==ETMOPEventTimingMode::Relative || DepartureTimingMode==ETMOPEventTimingMode::RelativeToPreviousEntry)",
            DisplayName="Departure Offset Seconds", EditConditionHides))
    int32 DepartureOffsetSeconds = 0;

    /** Zero derives cruise speed from route distance and departure/arrival. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            ClampMin="0.0", Units="km/h", DisplayName="Cruise Speed Override", EditConditionHides))
    float CruiseSpeedOverrideKmh = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            DisplayName="Driving Preset", EditConditionHides))
    ETMOPVehicleDrivingPreset DrivingPreset =
        ETMOPVehicleDrivingPreset::AutomaticFromTimeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline")
    FTransform WorldTransform = FTransform::Identity;

    /** Select how a placement, stop or park entry positions the vehicle. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Placement",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::InitialPlacement || Action==ETMOPHistoricalVehicleAction::Spawn || Action==ETMOPHistoricalVehicleAction::Stop || Action==ETMOPHistoricalVehicleAction::Park || Action==ETMOPHistoricalVehicleAction::OffscreenTransfer", EditConditionHides))
    ETMOPHistoricalVehiclePlacementMode PlacementMode =
        ETMOPHistoricalVehiclePlacementMode::WorldTransform;

    /** Anchor used when Placement Mode is Anchor, for example LarsKnubbBil. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Placement",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::InitialPlacement || Action==ETMOPHistoricalVehicleAction::Spawn || Action==ETMOPHistoricalVehicleAction::Stop || Action==ETMOPHistoricalVehicleAction::Park || Action==ETMOPHistoricalVehicleAction::OffscreenTransfer) && PlacementMode==ETMOPHistoricalVehiclePlacementMode::Anchor",
            DisplayName="Placement Anchor ID", EditConditionHides))
    FName PlacementAnchorId = NAME_None;

    /**
     * Optional local adjustment relative to the anchor. Use this when the
     * vehicle mesh origin or forward direction differs from the anchor.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Placement",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::InitialPlacement || Action==ETMOPHistoricalVehicleAction::Spawn || Action==ETMOPHistoricalVehicleAction::Stop || Action==ETMOPHistoricalVehicleAction::Park || Action==ETMOPHistoricalVehicleAction::OffscreenTransfer) && PlacementMode==ETMOPHistoricalVehiclePlacementMode::Anchor",
            DisplayName="Anchor Local Offset", EditConditionHides))
    FTransform AnchorLocalOffset = FTransform::Identity;

    /** How long the vehicle remains hidden after an Offscreen Transfer.
     * The same vehicle actor and all current seat assignments are retained. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Offscreen Transfer",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::OffscreenTransfer",
            ClampMin="0", Units="s", DisplayName="Transfer Duration Seconds", EditConditionHides))
    int32 OffscreenTransferDurationSeconds = 60;

    /** Opt in to a stop duration. The next arrival-timed drive can inherit its
     * departure from the end of this stop. Existing tables keep their times. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Stop",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::Stop || Action==ETMOPHistoricalVehicleAction::Park || Action==ETMOPHistoricalVehicleAction::ExitTrafficRoute", EditConditionHides))
    bool bUseStopDuration = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Stop",
        meta=(EditCondition="bUseStopDuration", EditConditionHides, ClampMin="0", Units="s"))
    int32 StopDurationSeconds = 10;

    /** Lane route used after this entry, when one has been reconstructed. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="VehicleRouteMode!=ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides, AdvancedDisplay))
    TArray<FName> OrderedLaneIds;

    /** Retained for old JSON/assets. All vehicle driving rows now start from
     * their own timeline, including rows serialized with this value false. */
    UPROPERTY(BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(DeprecatedProperty, DeprecationMessage="Vehicle timeline routes now always start automatically."))
    bool bAutoStartFromVehicleTimeline = true;

    /** Route interpretation copied from legacy person-driven vehicle entries. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute", EditConditionHides))
    ETMOPVehicleRouteMode VehicleRouteMode =
        ETMOPVehicleRouteMode::ManualLaneRoute;

    /** Curvature of an Anchor Maneuver. Zero is nearly straight; one gives
     * start/end anchor rotations strong influence over the path. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Anchor Maneuver",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && VehicleRouteMode==ETMOPVehicleRouteMode::AnchorManeuver",
            ClampMin="0.0", ClampMax="2.0", DisplayName="Curve Strength", EditConditionHides))
    float AnchorManeuverCurveStrength = 0.5f;

    /** Move backwards while retaining the rotations authored on the anchors.
     * Useful for reversing into a parking space. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Anchor Maneuver",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && VehicleRouteMode==ETMOPVehicleRouteMode::AnchorManeuver",
            DisplayName="Reverse Maneuver", EditConditionHides))
    bool bAnchorManeuverReverse = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Anchor Maneuver",
        meta=(EditCondition="VehicleRouteMode==ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides))
    ETMOPVehicleManeuverTurn AnchorManeuverTurn = ETMOPVehicleManeuverTurn::Automatic;

    /** Controls curve handles, not a guaranteed vehicle turning radius. Zero uses Curve Strength. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Anchor Maneuver",
        meta=(EditCondition="VehicleRouteMode==ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides, ClampMin="0", Units="cm", DisplayName="Turn Handle Radius"))
    float AnchorManeuverRadiusCm = 0.0f;

    /** Pass through via anchors by default. Enable to reach zero speed at each one. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Timeline|Anchor Maneuver",
        meta=(EditCondition="VehicleRouteMode==ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides))
    bool bStopAtViaAnchors = false;

    /** Optional explicit route start. Empty uses the preceding placement. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute", EditConditionHides))
    FName RouteStartAnchorId = NAME_None;

    /** Empty resolves the closest lane to Route Start Anchor/placement. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && VehicleRouteMode!=ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides))
    FName RouteStartLaneId = NAME_None;

    /** Exact legacy start distance on the first lane. Zero resolves it from
     * the vehicle/start anchor as before. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && VehicleRouteMode!=ETMOPVehicleRouteMode::AnchorManeuver",
            ClampMin="0.0", Units="cm", EditConditionHides))
    float RouteStartDistanceAlongFirstLaneCm = 0.0f;

    /** Optional explicit destination. Empty uses the following Stop/Park. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute", EditConditionHides))
    FName RouteDestinationAnchorId = NAME_None;

    /** Empty resolves the closest lane to the destination anchor/placement. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && VehicleRouteMode!=ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides))
    FName RouteDestinationLaneId = NAME_None;

    /** Ordered anchors that automatic routing must pass before destination. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute", EditConditionHides))
    TArray<FName> RouteViaAnchorIds;

    /** Ordered lanes visited after via anchors and before destination. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Route",
        meta=(EditCondition="(Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute) && VehicleRouteMode!=ETMOPVehicleRouteMode::AnchorManeuver", EditConditionHides))
    TArray<FName> RouteViaLaneIds;

    /** Allows disabled/restricted lane connections when following this route. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Rules",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            DisplayName="Ignore One-Way / Restricted Connections", EditConditionHides))
    bool bIgnoreOneWayRestrictions = false;

    /** Traffic signals do not add stop constraints while this route is active. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Timeline|Rules",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            DisplayName="Run Red Lights", EditConditionHides))
    bool bRunRedLights = false;

    /** Retry departure until every listed driver/passenger is seated. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Occupants",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            DisplayName="Wait For Listed Occupants", EditConditionHides))
    bool bWaitForListedOccupants = false;

    /** Time reserved between reaching the car and its departure. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Occupants",
        meta=(EditCondition="Action==ETMOPHistoricalVehicleAction::BeginDriving || Action==ETMOPHistoricalVehicleAction::EnterTrafficRoute",
            ClampMin="0", ClampMax="30", Units="s",
            DisplayName="Boarding Buffer Seconds", EditConditionHides))
    int32 BoardingBufferSeconds = 4;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Occupants")
    FName DriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Occupants")
    TArray<FName> PassengerEntityIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Reconstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString Notes;
};

/** One row in DT_TMOP_HistoricalVehicles. Row Name should equal VehicleId. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPHistoricalVehicleRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    FName VehicleId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    FText DisplayName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    FName CategoryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Identity")
    ETMOPVehicleCategory VehicleCategory = ETMOPVehicleCategory::PassengerCar;

    /**
     * A fleeing vehicle never brakes or honks for pedestrians. It keeps its
     * lane speed and applies a knockdown/damage impact when it hits a pawn.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Driving")
    bool bFleeingVehicle = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Registration")
    ETMOPVehicleRegistrationStatus RegistrationStatus =
        ETMOPVehicleRegistrationStatus::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Registration")
    ETMOPVehicleRegistrationOrigin RegistrationOrigin =
        ETMOPVehicleRegistrationOrigin::Unknown;

    /** Swedish registrations use normalized ABC-123 formatting. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Registration",
        meta=(EditCondition="RegistrationStatus==ETMOPVehicleRegistrationStatus::Known", EditConditionHides))
    FString RegistrationNumber;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Registration")
    FString RegistrationNotes;

    /** Optional known model. Leave empty when the source only says "car". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Model")
    TObjectPtr<UTMOPVehicleModelData> ModelData;

    /** Apply Body Color to the model's configurable paint material. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Appearance",
        meta=(DisplayName="Override Body Color"))
    bool bOverrideBodyColor = false;

    /** Per-vehicle paint colour. Requires Override Body Color. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Appearance",
        meta=(EditCondition="bOverrideBodyColor",
            DisplayName="Body Color", EditConditionHides))
    FLinearColor BodyColor = FLinearColor::White;

    /** Optional rigid mesh attached to the model's RoofAccessorySocket. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Historical Vehicle|Appearance")
    FTMOPRoofAccessoryVisual RoofAccessory;

    /** Additional equipment. Existing Roof Accessory remains supported. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Appearance",
        meta=(TitleProperty="AccessoryId"))
    TArray<FTMOPVehicleAccessoryVisual> AdditionalAccessories;

    /** Empty uses the future historical vehicle director's default vehicle class. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Simulation")
    TSubclassOf<AActor> VehicleClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|People")
    TArray<FName> AssociatedPersonEntityIds;

    /** Owner or main source-backed person. This does not automatically mean driver. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|People")
    FName PrimaryPersonEntityId = NAME_None;

    /** Only fill this when the driver is actually supported by the source. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|People")
    FName KnownDriverEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Simulation")
    bool bSpawnInSimulation = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Simulation")
    TArray<FTMOPHistoricalVehicleTimelineEntry> Timeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Reconstructed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString SourceReference;

    /** Controls the icon above the vehicle label. Automatic marks observed
     * vehicles with the eye symbol and other source-backed vehicles as
     * documented. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    ETMOPEntityEvidenceIcon EvidenceIcon = ETMOPEntityEvidenceIcon::Automatic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Historical Vehicle|Source")
    FString Notes;
};
