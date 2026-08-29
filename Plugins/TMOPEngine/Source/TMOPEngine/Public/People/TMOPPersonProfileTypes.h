#pragma once

#include "CoreMinimal.h"
#include "People/TMOPHeldItemTypes.h"
#include "Engine/DataTable.h"
#include "UI/TMOPEntityLabelTypes.h"
#include "Agents/TMOPAgentTypes.h"
#include "Agents/TMOPHistoricalAgent.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "Traffic/TMOPTrafficTypes.h"
#include "TMOPPersonProfileTypes.generated.h"

class UTexture2D;
class USoundBase;
class USkeletalMesh;
class UMaterialInterface;

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
enum class ETMOPAnchorOffsetSpace : uint8
{
    AnchorLocal UMETA(DisplayName="Anchor Local"),
    World UMETA(DisplayName="World")
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
    SetGroupLeader,
    /** Play one person-specific animation sequence through an Anim Blueprint slot. */
    PlayUniqueAnimation UMETA(DisplayName="Play Unique Animation"),
    /** Stop the unique animation currently playing in Animation Slot Name. */
    StopUniqueAnimation UMETA(DisplayName="Stop Unique Animation"),
    /** Keep looking at a selected anchor or moving person without talking. */
    LookAtAnchor UMETA(DisplayName="Look At")
};

/** Target used by talking, interacting and authored Look At actions. */
UENUM(BlueprintType)
enum class ETMOPConversationTargetMode : uint8
{
    /** Preserve older rows by using whichever target ID is already filled. */
    Automatic UMETA(DisplayName="Automatic / Existing Target"),
    SpecificPerson UMETA(DisplayName="Specific Person"),
    Group UMETA(DisplayName="Group"),
    Anchor UMETA(DisplayName="Anchor")
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
     * Physical destination offset from TargetAnchorId, in centimetres.
     * Applied to placement and the final destination of Move To Anchor; pass
     * anchors and Look At Anchor remain at their authored positions.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Location",
        meta=(Units="cm", DisplayName="Anchor Offset",
            EditCondition="LocationType==ETMOPPersonLocationType::Anchor && (Action==ETMOPPersonTimelineAction::InitialPlacement || Action==ETMOPPersonTimelineAction::Spawn || Action==ETMOPPersonTimelineAction::MoveToAnchor)"))
    FVector AnchorOffsetCm = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Location",
        meta=(DisplayName="Anchor Offset Space",
            EditCondition="LocationType==ETMOPPersonLocationType::Anchor && (Action==ETMOPPersonTimelineAction::InitialPlacement || Action==ETMOPPersonTimelineAction::Spawn || Action==ETMOPPersonTimelineAction::MoveToAnchor)"))
    ETMOPAnchorOffsetSpace AnchorOffsetSpace =
        ETMOPAnchorOffsetSpace::AnchorLocal;

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

    /**
     * Exact destination/parking transform. Automatic modes calculate the
     * complete route to it. Manual Lane Route uses it for the smooth final
     * approach from the last supplied lane.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::BeginDriving",
            DisplayName="Destination Anchor ID"))
    FName DrivingDestinationAnchorId = NAME_None;

    /** Distance from the beginning of the first lane at which driving starts. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Driving",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::BeginDriving",
            ClampMin="0.0", DisplayName="Start Distance Along First Lane (cm)"))
    float VehicleStartDistanceAlongFirstLaneCm = 0.0f;

    /** Runtime group affected by Join/Leave/Split/Dissolve/Set Group Leader. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Group",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::JoinGroup || Action==ETMOPPersonTimelineAction::LeaveGroup || Action==ETMOPPersonTimelineAction::SplitGroup || Action==ETMOPPersonTimelineAction::DissolveGroup || Action==ETMOPPersonTimelineAction::SetGroupLeader || ((Action==ETMOPPersonTimelineAction::Interact || Action==ETMOPPersonTimelineAction::PlayUniqueAnimation || Action==ETMOPPersonTimelineAction::LookAtAnchor) && ConversationTargetMode==ETMOPConversationTargetMode::Group)",
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

    /**
     * Used by Interact, a talking Play Unique Animation and Look At. Selecting
     * an anchor/person reference in the People Editor changes this mode
     * automatically. A person target is tracked continuously while moving.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Conversation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::Interact || Action==ETMOPPersonTimelineAction::PlayUniqueAnimation || Action==ETMOPPersonTimelineAction::LookAtAnchor",
            DisplayName="Target Type"))
    ETMOPConversationTargetMode ConversationTargetMode =
        ETMOPConversationTargetMode::Automatic;

    /**
     * Person-specific sequence used for actions such as kneeling, CPR or a
     * sourced gesture. The Animation Blueprint must contain a slot with the
     * matching AnimationSlotName. Existing locomotion continues to use the
     * ordinary TMOP movement state machine.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Animation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::PlayUniqueAnimation",
            DisplayName="Animation Asset",
            AllowedClasses="/Script/Engine.AnimSequenceBase"))
    TSoftObjectPtr<UObject> AnimationAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Animation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::PlayUniqueAnimation || Action==ETMOPPersonTimelineAction::StopUniqueAnimation",
            DisplayName="Animation Slot Name"))
    FName AnimationSlotName = FName(TEXT("DefaultSlot"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Animation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::PlayUniqueAnimation",
            ClampMin="0.01", DisplayName="Animation Play Rate"))
    float AnimationPlayRate = 1.0f;

    /** One plays once. Zero keeps looping until Stop Unique Animation. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Animation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::PlayUniqueAnimation",
            ClampMin="0", DisplayName="Animation Loop Count"))
    int32 AnimationLoopCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Animation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::PlayUniqueAnimation",
            ClampMin="0.0", Units="s", DisplayName="Animation Blend In"))
    float AnimationBlendInSeconds = 0.15f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite,
        Category="TMOP|Person|Timeline|Animation",
        meta=(EditCondition="Action==ETMOPPersonTimelineAction::PlayUniqueAnimation || Action==ETMOPPersonTimelineAction::StopUniqueAnimation",
            ClampMin="0.0", Units="s", DisplayName="Animation Blend Out"))
    float AnimationBlendOutSeconds = 0.20f;

    /** Catch-up may place the person directly when play begins after this entry. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline")
    bool bTeleportDuringCatchUp = true;

    /**
     * A precise historical deadline may replace an older movement which is
     * still active when this entry becomes due. No teleport is performed: the
     * previous navigation request is cancelled and this entry starts from the
     * person's actual position. Leave disabled for ordinary schedules.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(DisplayName="Supersede Active Movement When Due"))
    bool bSupersedeActiveMovementWhenDue = false;

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

/** How the runtime appearance system should build this person. */
UENUM(BlueprintType)
enum class ETMOPAppearanceGenerationMode : uint8
{
    /** Derive all known parts from the source-backed description and vary only unknown parts. */
    AutomaticFromEvidence UMETA(DisplayName="Automatic From Evidence"),
    /** Prefer the explicit visual choices in AppearanceProfile. */
    Manual UMETA(DisplayName="Manual Overrides"),
    /** A bespoke MetaHuman supplies the head/body; modular clothing may still be used. */
    MetaHuman UMETA(DisplayName="MetaHuman")
};

/** Rendering treatment used when a visual fact is not known from the sources. */
UENUM(BlueprintType)
enum class ETMOPUnknownAppearanceStyle : uint8
{
    /** Soft, low-detail and visibly indistinct without hiding the person. */
    Obscured UMETA(DisplayName="Obscured / Blurred"),
    /** Neutral placeholder asset, intended mainly for development. */
    Neutral UMETA(DisplayName="Neutral Placeholder"),
    /** Do not render this modular part. */
    Hidden UMETA(DisplayName="Hidden")
};

/** One runtime mesh/material choice. Empty fields mean automatic selection. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAppearancePartChoice
{
    GENERATED_BODY()

    /** Stable catalog ID, e.g. COAT_WOOL_1986_03 or UNKNOWN_TROUSERS_OBSCURED. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    FName CatalogId = NAME_None;

    /** Optional direct asset override for bespoke people. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    TSoftObjectPtr<USkeletalMesh> MeshOverride;

    /** Optional direct material override. Unknown parts normally use the shared obscured material. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    TSoftObjectPtr<UMaterialInterface> MaterialOverride;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    FLinearColor PrimaryColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    FLinearColor SecondaryColor = FLinearColor::White;
};

/**
 * Runtime visualisation settings. Source evidence remains in the existing
 * description fields; this profile only controls how that evidence is rendered.
 */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPAppearanceProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    ETMOPAppearanceGenerationMode GenerationMode =
        ETMOPAppearanceGenerationMode::AutomaticFromEvidence;

    /** Zero derives a stable seed from EntityId, giving the same person on every run. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance",
        meta=(ClampMin="0"))
    int32 AppearanceSeed = 0;

    /** Zero uses the source-backed height, then the Swedish 1986 fallback. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="0.0", ClampMax="205.0", Units="cm"))
    float HeightOverrideCentimeters = 0.0f;

    /** Unknown uses BodyBuildCategory and finally Average. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body")
    ETMOPBodyBuild BodyBuildOverride = ETMOPBodyBuild::Unknown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="-1.0", ClampMax="1.0"))
    float BodyWeightMorph = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="-1.0", ClampMax="1.0"))
    float MuscularityMorph = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="0.75", ClampMax="1.25"))
    float HeadScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="0.75", ClampMax="1.25"))
    float ShoulderScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="0.75", ClampMax="1.25"))
    float TorsoLengthScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="0.75", ClampMax="1.25"))
    float ArmLengthScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Body",
        meta=(ClampMin="0.75", ClampMax="1.25"))
    float LegLengthScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Color")
    FName SkinToneId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Color")
    FName EyeColorId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Face;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Hair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice FacialHair;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Outerwear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice UpperBody;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Trousers;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Footwear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Gloves;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Headwear;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Scarf;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Parts")
    FTMOPAppearancePartChoice Glasses;

    /** Applied independently to every unknown face or clothing part. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Unknown")
    ETMOPUnknownAppearanceStyle UnknownPartStyle =
        ETMOPUnknownAppearanceStyle::Obscured;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Unknown")
    FName UnknownFaceCatalogId = TEXT("UNKNOWN_FACE_OBSCURED");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Unknown")
    FName UnknownOuterwearCatalogId = TEXT("UNKNOWN_OUTERWEAR_OBSCURED");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Unknown")
    FName UnknownUpperBodyCatalogId = TEXT("UNKNOWN_UPPER_BODY_OBSCURED");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Unknown")
    FName UnknownTrousersCatalogId = TEXT("UNKNOWN_TROUSERS_OBSCURED");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance|Unknown")
    FName UnknownFootwearCatalogId = TEXT("UNKNOWN_FOOTWEAR_OBSCURED");
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

    /** Rendering profile; historical/source-backed descriptions remain below. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Visual Appearance")
    FTMOPAppearanceProfile AppearanceProfile;

    /** Resolved simulation height, including explicit override and 1986 fallback. */
    float GetResolvedHeightCentimeters() const
    {
        if (AppearanceProfile.HeightOverrideCentimeters > 0.0f)
            return FMath::Clamp(AppearanceProfile.HeightOverrideCentimeters, 120.0f, 205.0f);
        if (HeightCentimeters > 0.0f)
            return FMath::Clamp(HeightCentimeters, 120.0f, 205.0f);
        // Practical adult-population defaults for the Stockholm 1986 scenario.
        if (Gender == ETMOPPersonGender::Male) return 178.0f;
        if (Gender == ETMOPPersonGender::Female) return 165.0f;
        return 171.5f;
    }

    ETMOPBodyBuild GetResolvedBodyBuild() const
    {
        if (AppearanceProfile.BodyBuildOverride != ETMOPBodyBuild::Unknown)
            return AppearanceProfile.BodyBuildOverride;
        return BodyBuildCategory != ETMOPBodyBuild::Unknown
            ? BodyBuildCategory : ETMOPBodyBuild::Average;
    }

    /** Optional source photograph shown in the TMOP People Editor. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Reference",
        meta=(DisplayName="Reference Image"))
    TSoftObjectPtr<UTexture2D> ReferenceImage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Source")
    FString GeneralSourceReference;

    /** Controls the icon above the in-world name. Automatic uses observed
     * categories first, then source wording, and otherwise assumes an own
     * police interview when a source document exists. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Source")
    ETMOPEntityEvidenceIcon EvidenceIcon = ETMOPEntityEvidenceIcon::Automatic;

    /** The investigation lead/file reference where this person occurs. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Investigation",
        meta=(DisplayName="Uppslag"))
    FString Uppslag;

    /** Explicit status shown in the in-game agent information chart. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Agent Info")
    bool bPoliceInterviewed = false;

    /** Optional source-edited first-person account. Empty generates a summary from Timeline. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Agent Info",
        meta=(MultiLine="true", DisplayName="Timeline Summary"))
    FText AgentTimelineSummary;

    /** Source-preserving summary of this person's observations or testimony. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Agent Info",
        meta=(MultiLine="true"))
    FText ObservationSummary;

    /** Documents supporting the information displayed in the chart. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Agent Info")
    FString AgentInfoSourceReference;

    /** Empty uses the director's Default Agent Class. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    TSubclassOf<ATMOPHistoricalAgent> AgentClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    bool bSpawnInSimulation = true;

    /** Explicitly includes this person in the editor's Main Characters filter. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation",
        meta=(DisplayName="Main Character"))
    bool bMainCharacter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    FTMOPMovementProfile MovementProfile;

    /** Vehicles that are source-backed as belonging to, carrying, or otherwise involving this person. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Vehicle")
    TArray<FName> AssociatedVehicleIds;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Items")
    FTMOPHeldItemDefinition LeftHandItem;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Items")
    FTMOPHeldItemDefinition RightHandItem;

    /** Shoulder bags, backpacks, chest radios, hip props and other body attachments. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Held Items",
        meta=(TitleProperty="ItemId"))
    TArray<FTMOPHeldItemDefinition> AdditionalCarriedItems;

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
