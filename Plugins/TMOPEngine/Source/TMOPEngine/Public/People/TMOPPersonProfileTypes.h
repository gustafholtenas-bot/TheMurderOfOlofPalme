#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "TMOPPersonProfileTypes.generated.h"

class ATMOPHistoricalAgent;
class UTexture2D;
class USoundBase;

UENUM(BlueprintType)
enum class ETMOPPersonLocationType : uint8
{
    Unknown,
    NotPresent,
    Anchor,
    WorldTransform,
    VenueSeat,
    VehicleSeat,
    BusSeat,
    StandingInVehicle
};

UENUM(BlueprintType)
enum class ETMOPPersonTimelineAction : uint8
{
    InitialPlacement,
    Spawn,
    Despawn,
    MoveToAnchor,
    Wait,
    SitDown,
    StandUp,
    EnterVehicle,
    ExitVehicle,
    ChangeActivity,
    ChangeLifeState,
    Interact,
    Custom,
    /** Start the target vehicle on its ordered traffic-lane route. */
    BeginDriving,
    /** Create GroupDefinition at this point in the person's timeline. */
    CreateGroup,
    /** Add this person to TargetGroupId. */
    JoinGroup,
    /** Remove this person from TargetGroupId (or their current runtime group). */
    LeaveGroup,
    /** Replace TargetGroupId with SplitGroupDefinitions. */
    SplitGroup,
    /** Dissolve TargetGroupId. */
    DissolveGroup,
    /** Change TargetGroupId's runtime leader to NewGroupLeaderEntityId. */
    SetGroupLeader
};

/** Controls whether a historical timeline entry affects the running simulation. */
UENUM(BlueprintType)
enum class ETMOPPersonTimelineUsage : uint8
{
    /** Existing/default behaviour: execute this entry in the simulation. */
    Simulation UMETA(DisplayName="Simulation"),
    /** Store this entry for chronology, alibi and research only. */
    DocumentationOnly UMETA(DisplayName="Documentation Only"),
    /** Keep the documentation marker and execute it when its world references exist. */
    DocumentationAndSimulation UMETA(DisplayName="Documentation + Simulation")
};

/** Describes whether an anchor is already required in the map or may be added later. */
UENUM(BlueprintType)
enum class ETMOPAnchorReferenceMode : uint8
{
    RequiredInWorld UMETA(DisplayName="Required In World"),
    PlannedFuture UMETA(DisplayName="Planned / Future Anchor")
};

/** One chronological, source-backed state or action for a person. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonTimelineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline",
        meta=(DisplayName="Entry ID"))
    FName EntryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline")
    ETMOPPersonTimelineAction Action = ETMOPPersonTimelineAction::InitialPlacement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline",
        meta=(ToolTip="Documentation Only entries never spawn or move a runtime person. Existing imported entries default to Simulation."))
    ETMOPPersonTimelineUsage Usage = ETMOPPersonTimelineUsage::Simulation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    /**
     * Absolute uses Time. Relative to Shared Event uses SharedEventId.
     * Relative to Previous Entry uses the resolved time of the array item
     * immediately above this one. Both relative modes add EventOffsetSeconds.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    /** Central event used by Relative timing, for example a shared rendezvous. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative"))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative || TimingMode==ETMOPEventTimingMode::RelativeToPreviousEntry",
            DisplayName="Offset Seconds"))
    int32 EventOffsetSeconds = 0;

    /** For MoveToAnchor, calculate departure backwards so arrival matches the resolved time. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::MoveToAnchor"))
    bool bTimeIsArrival = false;

    /** Zero uses the person's movement profile. Units are centimetres per second. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(ClampMin="0.0", EditCondition="bTimeIsArrival"))
    float TravelSpeedOverrideCmPerSecond = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    ETMOPPersonLocationType LocationType = ETMOPPersonLocationType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetAnchorId = NAME_None;

    /**
     * Planned Future permits this ID before an anchor actor exists. A runtime-enabled
     * entry is ignored while it is missing and becomes executable when an Unreal
     * anchor with the same ID is later added to the world.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    ETMOPAnchorReferenceMode AnchorReferenceMode =
        ETMOPAnchorReferenceMode::RequiredInWorld;

    /** Human-readable off-map/future location, for example "Polishuset". */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location",
        meta=(EditCondition="AnchorReferenceMode==ETMOPAnchorReferenceMode::PlannedFuture"))
    FText PlannedAnchorDisplayName;

    /** Placement/address guidance retained until the actual Unreal anchor is built. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location",
        meta=(EditCondition="AnchorReferenceMode==ETMOPAnchorReferenceMode::PlannedFuture", MultiLine="true"))
    FString PlannedAnchorNotes;

    /**
     * Optional anchors that must be visited in order before TargetAnchorId.
     * These are route waypoints, not separate timeline events.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::MoveToAnchor || Action==ETMOPPersonTimelineAction::BeginDriving",
            DisplayName="Pass Anchors On The Way",
            ToolTip="Ordered pedestrian or vehicle waypoints visited before the final target/destination anchor. Automatic vehicle routing calculates lanes between them."))
    TArray<FName> PassAnchorIds;

    /** Vehicle ID, bus Run ID, venue ID, or another target entity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetSeatId = NAME_None;

    /** Optional stop constraint for boarding or alighting a bus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetStopId = NAME_None;

    /**
     * Optional driving route for BeginDriving. When empty, the route is read
     * from the target vehicle's BeginDriving/EnterTrafficRoute timeline entry.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::BeginDriving",
            DisplayName="Ordered Lane IDs"))
    TArray<FName> OrderedLaneIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::BeginDriving"))
    ETMOPVehicleRouteMode VehicleRouteMode =
        ETMOPVehicleRouteMode::ManualLaneRoute;

    /** Destination used by Automatic To Anchor and Manual Then Automatic. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::BeginDriving && VehicleRouteMode!=ETMOPVehicleRouteMode::ManualLaneRoute",
            DisplayName="Destination Anchor ID"))
    FName DrivingDestinationAnchorId = NAME_None;

    /** Distance from the beginning of the first lane at which driving starts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::BeginDriving",
            ClampMin="0.0", DisplayName="Start Distance Along First Lane (cm)"))
    float VehicleStartDistanceAlongFirstLaneCm = 0.0f;

    /** Runtime group affected by Join/Leave/Split/Dissolve/Set Group Leader. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Group",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::JoinGroup || Action==ETMOPPersonTimelineAction::LeaveGroup || Action==ETMOPPersonTimelineAction::SplitGroup || Action==ETMOPPersonTimelineAction::DissolveGroup || Action==ETMOPPersonTimelineAction::SetGroupLeader",
            DisplayName="Target Group ID"))
    FName TargetGroupId = NAME_None;

    /** Complete runtime group definition used by Create Group. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Group",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::CreateGroup"))
    FTMOPGroupDefinition GroupDefinition;

    /** Child groups used by Split Group. Every source member must occur exactly once. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Group",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::SplitGroup"))
    TArray<FTMOPGroupDefinition> SplitGroupDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Group",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::SetGroupLeader",
            DisplayName="New Leader Entity ID"))
    FName NewGroupLeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FTransform WorldTransform = FTransform::Identity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|State")
    ETMOPAgentActivityState ActivityState = ETMOPAgentActivityState::Idle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|State")
    ETMOPAgentLifeState LifeState = ETMOPAgentLifeState::Alive;

    /** Catch-up may place the person directly when play begins after this entry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline")
    bool bTeleportDuringCatchUp = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Source")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Source")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Source")
    FString Notes;
};

/** Dialogue used by the normal talk interaction on either side of the shot. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonDialog
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Dialog",
        meta=(MultiLine="true", DisplayName="Before Shot"))
    FText BeforeShot;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Dialog",
        meta=(MultiLine="true", DisplayName="After Shot"))
    FText AfterShot;
};

UENUM(BlueprintType)
enum class ETMOPSpeechTimingMode : uint8
{
    Absolute UMETA(DisplayName="Absolute"),
    RelativeToSharedEvent UMETA(DisplayName="Relative to Shared Event"),
    RelativeToPreviousLine UMETA(DisplayName="Relative to Previous Line")
};

/** A source-backed line spoken automatically at a resolved simulation time. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPTimedSpeechLine
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(DisplayName="Line ID"))
    FName LineId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech")
    ETMOPSpeechTimingMode TimingMode = ETMOPSpeechTimingMode::Absolute;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(EditCondition="TimingMode==ETMOPSpeechTimingMode::RelativeToSharedEvent",
            DisplayName="Shared Event ID"))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(EditCondition="TimingMode!=ETMOPSpeechTimingMode::Absolute",
            DisplayName="Offset Seconds"))
    int32 OffsetSeconds = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(MultiLine="true", DisplayName="Spoken Text"))
    FText Text;

    /** Optional imported Sound Wave (MP3/WAV) played with this line. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(DisplayName="Voice Over"))
    TSoftObjectPtr<USoundBase> VoiceOver;

    /**
     * Zero calculates a readable duration. Voice-over length is always
     * respected, even when this override is shorter.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(ClampMin="0.0", Units="s", DisplayName="Display Duration Override"))
    float DisplayDurationOverrideSeconds = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech")
    ETMOPHistoricalConfidence Confidence =
        ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech")
    FString SourceReference;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech")
    FString Notes;
};

UENUM(BlueprintType)
enum class ETMOPPersonGender : uint8
{
    Unknown,
    Female,
    Male,
    OtherOrUnspecified
};

UENUM(BlueprintType)
enum class ETMOPHairColor : uint8
{
    Unknown,
    Blond,
    Brown,
    Dark,
    Black,
    Grey,
    Red,
    White,
    Bald
};

UENUM(BlueprintType)
enum class ETMOPFacialHairType : uint8
{
    Unknown,
    None,
    Mustache,
    Beard,
    MustacheAndBeard,
    Stubble
};

UENUM(BlueprintType)
enum class ETMOPHeadwearType : uint8
{
    Unknown,
    None,
    Cap,
    KnitCap,
    FurHat,
    BrimmedHat,
    Hood,
    Helmet,
    Other
};

UENUM(BlueprintType)
enum class ETMOPBodyBuild : uint8
{
    Unknown,
    Thin,
    Slim,
    Average,
    Athletic,
    Strong,
    Heavy
};

UENUM(BlueprintType)
enum class ETMOPOuterwearType : uint8
{
    Unknown,
    None,
    Jacket,
    Coat,
    LeatherJacket,
    SailingJacket,
    Parka,
    Uniform,
    Other
};

/** One evidence-preserving description slot. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAppearanceSlot
{
    GENERATED_BODY()

    /** Exact wording from the source. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    FString OriginalText;

    /** Stable filter value, e.g. DarkBlueLongCoat. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    FName NormalizedValue = NAME_None;

    /** Additional flexible filters that do not require a C++ rebuild. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    TArray<FName> Tags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPHistoricalConfidence Confidence = ETMOPHistoricalConfidence::Documented;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    FString SourceReference;
};

/** One row in DT_TMOP_People. Row Name should equal EntityId. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonProfileRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FName EntityId = NAME_None;

    /** Stable sorting category, normally derived from the person's Blender collection. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FName CategoryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FText FullName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FText FirstName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FText LastName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    ETMOPPersonGender Gender = ETMOPPersonGender::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FString Nationality;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Identity")
    FString Occupation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History")
    FString HistoricalAddress;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History")
    int32 BirthYear = 0;

    /** Age at the simulated event, not current age. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History")
    int32 AgeAtEvent = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|History",
        meta=(ClampMin="0.0"))
    float HeightCentimeters = 0.0f;

    /** Optional source photograph shown in the TMOP People Editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Reference",
        meta=(DisplayName="Reference Image"))
    TSoftObjectPtr<UTexture2D> ReferenceImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Source")
    FString GeneralSourceReference;

    /** The investigation lead/file reference where this person occurs. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Investigation",
        meta=(DisplayName="Uppslag"))
    FString Uppslag;

    /** Empty uses the director's Default Agent Class. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    TSubclassOf<ATMOPHistoricalAgent> AgentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    bool bSpawnInSimulation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    FTMOPMovementProfile MovementProfile;

    /** Vehicles that are source-backed as belonging to, carrying, or otherwise involving this person. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Vehicle")
    TArray<FName> AssociatedVehicleIds;

    /** Existing generic group system membership. Leave GroupId empty for an individual. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Group")
    FName SocialGroupId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Group")
    FName GroupLeaderEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Group")
    ETMOPGroupFormation GroupFormation = ETMOPGroupFormation::SideBySide;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Group",
        meta=(ClampMin="30.0"))
    float GroupFormationSpacingCm = 110.0f;

    /** Followers only need their initial placement; the leader's moves drive the group. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Group")
    bool bFollowGroupLeaderSchedule = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Dialog")
    FTMOPPersonDialog Dialog;

    /** Timed lines shown above this person's head and optionally voiced. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Automatic Speech",
        meta=(TitleProperty="LineId"))
    TArray<FTMOPTimedSpeechLine> AutomaticSpeech;

    /** Slot 0 is the initial marker. It may occur later than 23:00. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline")
    TArray<FTMOPPersonTimelineEntry> Timeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPBodyBuild BodyBuildCategory = ETMOPBodyBuild::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPHairColor HairColorCategory = ETMOPHairColor::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPHeadwearType HeadwearCategory = ETMOPHeadwearType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPFacialHairType FacialHairCategory = ETMOPFacialHairType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance")
    ETMOPOuterwearType OuterwearCategory = ETMOPOuterwearType::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot Hair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot Headwear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot BeardOrMustache;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot FaceShape;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot Nose;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot BodyBuild;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot JacketOrCoat;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot ShirtOrSweater;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot Trousers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Clothing")
    FTMOPAppearanceSlot Shoes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Accessories")
    FTMOPAppearanceSlot Scarf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Accessories")
    FTMOPAppearanceSlot Glasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Other")
    FTMOPAppearanceSlot OtherCharacteristics;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Notes")
    FString Notes;
};
