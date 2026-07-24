#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Agents/TMOPAgentTypes.h"
#include "Events/TMOPHistoricalEventTypes.h"
#include "Groups/TMOPGroupTypes.h"
#include "TMOPPersonProfileTypes.generated.h"

class ATMOPHistoricalAgent;

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
    Custom
};

/** One chronological, source-backed state or action for a person. */
USTRUCT(BlueprintType)
struct TMOPENGINE_API FTMOPPersonTimelineEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline")
    FName EntryId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline")
    ETMOPPersonTimelineAction Action = ETMOPPersonTimelineAction::InitialPlacement;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time")
    FTMOPTime Time = FTMOPTime(23, 0, 0);

    /** Absolute uses Time. Relative uses SharedEventId plus EventOffsetSeconds. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time")
    ETMOPEventTimingMode TimingMode = ETMOPEventTimingMode::Absolute;

    /** Central event used by Relative timing, for example a shared rendezvous. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative"))
    FName SharedEventId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Time",
        meta=(EditCondition="TimingMode==ETMOPEventTimingMode::Relative"))
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

    /** Vehicle ID, bus Run ID, venue ID, or another target entity. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetEntityId = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetSeatId = NAME_None;

    /** Optional stop constraint for boarding or alighting a bus. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Timeline|Location")
    FName TargetStopId = NAME_None;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Appearance",
        meta=(ClampMin="0.0"))
    float HeightCentimeters = 0.0f;

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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Description")
    FTMOPAppearanceSlot OtherCharacteristics;

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

    /** Slot 0 is the initial marker. It may occur later than 23:00. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Simulation")
    TArray<FTMOPPersonTimelineEntry> Timeline;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="TMOP|Person|Source")
    FString Notes;
};
